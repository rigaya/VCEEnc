// -----------------------------------------------------------------------------------------
//     VCEEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2014-2017 rigaya
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// IABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// ------------------------------------------------------------------------------------------

#include <fcntl.h>
#include <io.h>
#include <algorithm>
#include <numeric>
#include <map>
#include <cctype>
#include <cmath>
#include <climits>
#include <memory>
#include "avcodec_reader.h"
#include "avcodec_vce_log.h"
#include "Surface.h"

#if ENABLE_AVCODEC_VCE_READER

static inline void extend_array_size(VideoFrameData *dataset) {
    static int default_capacity = 8 * 1024;
    int current_cap = dataset->capacity;
    dataset->capacity = (current_cap) ? current_cap * 2 : default_capacity;
    dataset->frame = (FramePos *)realloc(dataset->frame, dataset->capacity * sizeof(dataset->frame[0]));
    memset(dataset->frame + current_cap, 0, sizeof(dataset->frame[0]) * (dataset->capacity - current_cap));
}

CAvcodecReader::CAvcodecReader() {
    memset(&m_Demux.format, 0, sizeof(m_Demux.format));
    memset(&m_Demux.video,  0, sizeof(m_Demux.video));
    m_strReaderName = _T("avvce");
}

CAvcodecReader::~CAvcodecReader() {
    Close();
}

void CAvcodecReader::CloseThread() {
    m_Demux.thread.bAbortInput = true;
    if (m_Demux.thread.thInput.joinable()) {
        m_Demux.qVideoPkt.set_capacity(SIZE_MAX);
        m_Demux.qVideoPkt.set_keep_length(0);
        m_Demux.thread.thInput.join();
        AddMessage(VCE_LOG_DEBUG, _T("Closed Input thread.\n"));
    }
    m_Demux.thread.bAbortInput = false;
}

void CAvcodecReader::CloseFormat(AVDemuxFormat *pFormat) {
    //close video file
    if (pFormat->pFormatCtx) {
        avformat_close_input(&pFormat->pFormatCtx);
        AddMessage(VCE_LOG_DEBUG, _T("Closed avformat context.\n"));
    }
    if (m_Demux.format.pFormatOptions) {
        av_dict_free(&m_Demux.format.pFormatOptions);
    }
    memset(pFormat, 0, sizeof(pFormat[0]));
}

void CAvcodecReader::CloseVideo(AVDemuxVideo *pVideo) {
    //close parser
    if (pVideo->pParserCtx) {
        av_parser_close(pVideo->pParserCtx);
    }
    //close bitstreamfilter
    if (pVideo->pBsfcCtx) {
        av_bsf_free(&pVideo->pBsfcCtx);
    }
    
    if (pVideo->pExtradata) {
        av_free(pVideo->pExtradata);
    }

    memset(pVideo, 0, sizeof(pVideo[0]));
    pVideo->nIndex = -1;
}

void CAvcodecReader::CloseStream(AVDemuxStream *pStream) {
    if (pStream->pktSample.data) {
        av_packet_unref(&pStream->pktSample);
    }
    memset(pStream, 0, sizeof(pStream[0]));
    pStream->nIndex = -1;
}

void CAvcodecReader::Close() {
    AddMessage(VCE_LOG_DEBUG, _T("Closing...\n"));
    //リソースの解放
    CloseThread();
    m_Demux.qVideoPkt.close([](AVPacket *pkt) { av_packet_unref(pkt); });
    for (uint32_t i = 0; i < m_Demux.qStreamPktL1.size(); i++) {
        av_packet_unref(&m_Demux.qStreamPktL1[i]);
    }
    m_Demux.qStreamPktL1.clear();
    m_Demux.qStreamPktL2.close([](AVPacket *pkt) { av_packet_unref(pkt); });
    AddMessage(VCE_LOG_DEBUG, _T("Cleared Stream Packet Buffer.\n"));

    CloseFormat(&m_Demux.format);
    CloseVideo(&m_Demux.video);   AddMessage(VCE_LOG_DEBUG, _T("Closed video.\n"));
    for (int i = 0; i < (int)m_Demux.stream.size(); i++) {
        CloseStream(&m_Demux.stream[i]);
        AddMessage(VCE_LOG_DEBUG, _T("Cleared Stream #%d.\n"), i);
    }
    m_Demux.stream.clear();
    m_Demux.chapter.clear();

    m_sTrimParam.list.clear();
    m_sTrimParam.offset = 0;

    m_hevcMp42AnnexbBuffer.clear();

    //free input buffer (使用していない)
    //if (buffer) {
    //    free(buffer);
    //    buffer = nullptr;
    //}
    m_pEncSatusInfo.reset();
    if (m_sFramePosListLog.length()) {
        m_Demux.frames.printList(m_sFramePosListLog.c_str());
    }
    m_Demux.frames.clear();

    AddMessage(VCE_LOG_DEBUG, _T("Closed.\n"));
}

uint32_t CAvcodecReader::getQSVFourcc(uint32_t id) {
    for (int i = 0; i < _countof(VCE_DECODE_LIST); i++)
        if (VCE_DECODE_LIST[i].codec_id == id)
            return VCE_DECODE_LIST[i].vce_codec;
    return 0;
}

vector<int> CAvcodecReader::getStreamIndex(AVMediaType type, const vector<int> *pVidStreamIndex) {
    vector<int> streams;
    const int n_streams = m_Demux.format.pFormatCtx->nb_streams;
    for (int i = 0; i < n_streams; i++) {
        if (m_Demux.format.pFormatCtx->streams[i]->codecpar->codec_type == type) {
            streams.push_back(i);
        }
    }
    if (type == AVMEDIA_TYPE_VIDEO) {
        std::sort(streams.begin(), streams.end(), [pFormatCtx = m_Demux.format.pFormatCtx](int streamIdA, int streamIdB) {
            auto pStreamA = pFormatCtx->streams[streamIdA];
            auto pStreamB = pFormatCtx->streams[streamIdB];
            if (pStreamA->codecpar == nullptr) {
                return false;
            }
            if (pStreamB->codecpar == nullptr) {
                return true;
            }
            const int resA = pStreamA->codecpar->width * pStreamA->codecpar->height;
            const int resB = pStreamB->codecpar->width * pStreamB->codecpar->height;
            return (resA > resB);
        });
    } else if (pVidStreamIndex && pVidStreamIndex->size()) {
        auto mostNearestVidStreamId = [pFormatCtx = m_Demux.format.pFormatCtx, pVidStreamIndex](int streamId) {
            auto ret = std::make_pair(0, UINT32_MAX);
            for (uint32_t i = 0; i < pVidStreamIndex->size(); i++) {
                uint32_t diff = (uint32_t)(streamId - pFormatCtx->streams[(*pVidStreamIndex)[i]]->id);
                if (diff < ret.second) {
                    ret.second = diff;
                    ret.first = i;
                }
            }
            return ret;
        };
        std::sort(streams.begin(), streams.end(), [pFormatCtx = m_Demux.format.pFormatCtx, pVidStreamIndex, mostNearestVidStreamId](int streamIdA, int streamIdB) {
            if (pFormatCtx->streams[streamIdA]->codecpar == nullptr) {
                return false;
            }
            if (pFormatCtx->streams[streamIdB]->codecpar == nullptr) {
                return true;
            }
            auto pStreamIdA = pFormatCtx->streams[streamIdA]->id;
            auto pStreamIdB = pFormatCtx->streams[streamIdB]->id;
            auto nearestVidA = mostNearestVidStreamId(pStreamIdA);
            auto nearestVidB = mostNearestVidStreamId(pStreamIdB);
            if (nearestVidA.first == nearestVidB.first) {
                return nearestVidA.second < nearestVidB.second;
            }
            return nearestVidA.first < nearestVidB.first;
        });
    }
    return std::move(streams);
}

bool CAvcodecReader::vc1StartCodeExists(uint8_t *ptr) {
    uint32_t code = readUB32(ptr);
    return check_range_unsigned(code, 0x010A, 0x010F) || check_range_unsigned(code, 0x011B, 0x011F);
}

void CAvcodecReader::vc1FixHeader(int nLengthFix) {
    if (m_Demux.video.pStream->codecpar->codec_id == AV_CODEC_ID_WMV3) {
        m_Demux.video.nExtradataSize += nLengthFix;
        uint32_t datasize = m_Demux.video.nExtradataSize;
        vector<uint8_t> buffer(20 + datasize, 0);
        uint32_t header = 0xC5000000;
        uint32_t width = m_Demux.video.pStream->codecpar->width;
        uint32_t height = m_Demux.video.pStream->codecpar->height;
        uint8_t *dataPtr = m_Demux.video.pExtradata - nLengthFix;
        memcpy(buffer.data() +  0, &header, sizeof(header));
        memcpy(buffer.data() +  4, &datasize, sizeof(datasize));
        memcpy(buffer.data() +  8, dataPtr, datasize);
        memcpy(buffer.data() +  8 + datasize, &height, sizeof(height));
        memcpy(buffer.data() + 12 + datasize, &width, sizeof(width));
        m_Demux.video.pExtradata = (uint8_t *)av_realloc(m_Demux.video.pExtradata, sizeof(buffer) + FF_INPUT_BUFFER_PADDING_SIZE);
        m_Demux.video.nExtradataSize = (int)buffer.size();
        memcpy(m_Demux.video.pExtradata, buffer.data(), buffer.size());
    } else {
        m_Demux.video.nExtradataSize += nLengthFix;
        memmove(m_Demux.video.pExtradata, m_Demux.video.pExtradata - nLengthFix, m_Demux.video.nExtradataSize);
    }
}

void CAvcodecReader::vc1AddFrameHeader(AVPacket *pkt) {
    uint32_t size = pkt->size;
    if (m_Demux.video.pStream->codecpar->codec_id == AV_CODEC_ID_WMV3) {
        av_grow_packet(pkt, 8);
        memmove(pkt->data + 8, pkt->data, size);
        memcpy(pkt->data, &size, sizeof(size));
        memset(pkt->data + 4, 0, 4);
    } else if (!vc1StartCodeExists(pkt->data)) {
        uint32_t startCode = 0x0D010000;
        av_grow_packet(pkt, sizeof(startCode));
        memmove(pkt->data + sizeof(startCode), pkt->data, size);
        memcpy(pkt->data, &startCode, sizeof(startCode));
    }
}

AMF_RESULT CAvcodecReader::getFirstFramePosAndFrameRate(const sTrim *pTrimList, int nTrimCount) {
    AVRational fpsDecoder = m_Demux.video.pStream->avg_frame_rate;
    const bool fpsDecoderInvalid = (fpsDecoder.den == 0 || fpsDecoder.num == 0);
    //timebaseが60で割り切れない場合には、ptsが完全には割り切れない値である場合があり、より多くのフレーム数を解析する必要がある
    int maxCheckFrames = (m_Demux.format.nAnalyzeSec == 0) ? ((m_Demux.video.pStream->time_base.den >= 1000 && m_Demux.video.pStream->time_base.den % 60) ? 128 : 48) : 7200;
    int maxCheckSec = (m_Demux.format.nAnalyzeSec == 0) ? INT_MAX : m_Demux.format.nAnalyzeSec;
    AddMessage(VCE_LOG_DEBUG, _T("fps decoder invalid: %s\n"), fpsDecoderInvalid ? _T("true") : _T("false"));

    AVPacket pkt;
    av_init_packet(&pkt);

    const bool bCheckDuration = m_Demux.video.pStream->time_base.num * m_Demux.video.pStream->time_base.den > 0;
    const double timebase = (bCheckDuration) ? m_Demux.video.pStream->time_base.num / (double)m_Demux.video.pStream->time_base.den : 1.0;
    m_Demux.video.nStreamFirstKeyPts = 0;
    int i_samples = 0;
    std::vector<int> frameDurationList;
    vector<std::pair<int, int>> durationHistgram;

    for (int i_retry = 0; ; i_retry++) {
        if (i_retry) {
            //フレームレート推定がうまくいかなそうだった場合、もう少しフレームを解析してみる
            maxCheckFrames <<= 1;
            if (maxCheckSec != INT_MAX) {
                maxCheckSec <<= 1;
            }
            //ヒストグラム生成などは最初からやり直すので、一度クリアする
            durationHistgram.clear();
            frameDurationList.clear();
        }
        for (; i_samples < maxCheckFrames && !getSample(&pkt); i_samples++) {
            m_Demux.qVideoPkt.push(pkt);
            if (bCheckDuration) {
                int64_t diff = 0;
                if (pkt.dts != AV_NOPTS_VALUE && m_Demux.frames.list(0).dts != AV_NOPTS_VALUE) {
                    diff = (int)(pkt.dts - m_Demux.frames.list(0).dts);
                } else if (pkt.pts != AV_NOPTS_VALUE && m_Demux.frames.list(0).pts != AV_NOPTS_VALUE) {
                    diff = (int)(pkt.pts - m_Demux.frames.list(0).pts);
                }
                const int duration = (int)((double)diff * timebase + 0.5);
                if (duration >= maxCheckSec) {
                    break;
                }
            }
        }
#if _DEBUG && 0
        for (int i = 0; i < m_Demux.frames.frameNum(); i++) {
            fprintf(stderr, "%3d: pts:%I64d, poc:%3d, duration:%5d, duration2:%5d, repeat:%d\n",
                i, m_Demux.frames.list(i).pts, m_Demux.frames.list(i).poc,
                m_Demux.frames.list(i).duration, m_Demux.frames.list(i).duration2,
                m_Demux.frames.list(i).repeat_pict);
        }
#endif
        //ここまで集めたデータでpts, pocを確定させる
        double dEstFrameDurationByFpsDecoder = 0.0;
        if (av_isvalid_q(fpsDecoder) && av_isvalid_q(m_Demux.video.pStream->time_base)) {
            dEstFrameDurationByFpsDecoder = av_q2d(av_inv_q(fpsDecoder)) * av_q2d(av_inv_q(m_Demux.video.pStream->time_base));
        }
        m_Demux.frames.checkPtsStatus(dEstFrameDurationByFpsDecoder);

        const int nFramesToCheck = m_Demux.frames.fixedNum();
        AddMessage(VCE_LOG_DEBUG, _T("read %d packets.\n"), m_Demux.frames.frameNum());
        AddMessage(VCE_LOG_DEBUG, _T("checking %d frame samples.\n"), nFramesToCheck);

        frameDurationList.reserve(nFramesToCheck);

        for (int i = 0; i < nFramesToCheck; i++) {
#if _DEBUG && 0
            fprintf(stderr, "%3d: pts:%I64d, poc:%3d, duration:%5d, duration2:%5d, repeat:%d\n",
                i, m_Demux.frames.list(i).pts, m_Demux.frames.list(i).poc,
                m_Demux.frames.list(i).duration, m_Demux.frames.list(i).duration2,
                m_Demux.frames.list(i).repeat_pict);
#endif
            if (m_Demux.frames.list(i).poc != AVVCE_POC_INVALID) {
                int duration = m_Demux.frames.list(i).duration + m_Demux.frames.list(i).duration2;
                //RFF用の補正
                if (m_Demux.frames.list(i).repeat_pict > 1) {
                    duration = (int)(duration * 2 / (double)(m_Demux.frames.list(i).repeat_pict + 1) + 0.5);
                }
                frameDurationList.push_back(duration);
            }
        }

        //durationのヒストグラムを作成
        std::for_each(frameDurationList.begin(), frameDurationList.end(), [&durationHistgram](const int& duration) {
            auto target = std::find_if(durationHistgram.begin(), durationHistgram.end(), [duration](const std::pair<int, int>& pair) { return pair.first == duration; });
            if (target != durationHistgram.end()) {
                target->second++;
            } else {
                durationHistgram.push_back(std::make_pair(duration, 1));
            }
        });
        //多い順にソートする
        std::sort(durationHistgram.begin(), durationHistgram.end(), [](const std::pair<int, int>& pairA, const std::pair<int, int>& pairB) { return pairA.second > pairB.second; });

        AddMessage(VCE_LOG_DEBUG, _T("stream timebase %d/%d\n"), m_Demux.video.pStream->time_base.num, m_Demux.video.pStream->time_base.den);
        AddMessage(VCE_LOG_DEBUG, _T("decoder fps     %d/%d\n"), fpsDecoder.num, fpsDecoder.den);
        AddMessage(VCE_LOG_DEBUG, _T("duration histgram of %d frames\n"), durationHistgram.size());
        for (const auto& sample : durationHistgram) {
            AddMessage(VCE_LOG_DEBUG, _T("%3d [%3d frames]\n"), sample.first, sample.second);
        }

        //ここでやめてよいか判定する
        if (i_retry == 0) {
            //初回は、唯一のdurationが得られている場合を除き再解析する
            if (durationHistgram.size() <= 1) {
                break;
            }
        } else if (durationHistgram.size() <= 1 //唯一のdurationが得られている
            || durationHistgram[0].second / (double)frameDurationList.size() > 0.95 //大半がひとつのdurationである
            || std::abs(durationHistgram[0].first - durationHistgram[1].first) <= 1) { //durationのブレが貧弱なtimebaseによる丸めによるもの(mkvなど)
            break;
        }
        if (i_retry >= 4) {
            break;
        }
        //再度解析を行う場合は、音声がL2キューに入らないよう、一度fixedNumを0に戻す
        m_Demux.frames.clearPtsStatus();
    }

    //durationが0でなく、最も頻繁に出てきたもの
    auto& mostPopularDuration = durationHistgram[durationHistgram.size() > 1 && durationHistgram[0].first == 0];

    struct Rational64 {
        uint64_t num;
        uint64_t den;
    } estimatedAvgFps = { 0 }, nAvgFramerate64 = { 0 }, fpsDecoder64 = { (uint64_t)fpsDecoder.num, (uint64_t)fpsDecoder.den };
    if (mostPopularDuration.first == 0) {
        m_Demux.video.nStreamPtsInvalid |= RGY_PTS_ALL_INVALID;
    } else {
        //avgFpsとtargetFpsが近いかどうか
        auto fps_near = [](double avgFps, double targetFps) { return std::abs(1 - avgFps / targetFps) < 0.5; };
        //durationの平均を求める (ただし、先頭は信頼ならないので、cutoff分は計算に含めない)
        //std::accumulateの初期値に"(uint64_t)0"と与えることで、64bitによる計算を実行させ、桁あふれを防ぐ
        //大きすぎるtimebaseの時に必要
        double avgDuration = std::accumulate(frameDurationList.begin(), frameDurationList.end(), (uint64_t)0, [this](const uint64_t sum, const int& duration) { return sum + duration; }) / (double)(frameDurationList.size());
        double avgFps = m_Demux.video.pStream->time_base.den / (double)(avgDuration * m_Demux.video.pStream->time_base.num);
        double torrelance = (fps_near(avgFps, 25.0) || fps_near(avgFps, 50.0)) ? 0.05 : 0.0008; //25fps, 50fps近辺は基準が甘くてよい
        if (mostPopularDuration.second / (double)frameDurationList.size() > 0.95 && std::abs(1 - mostPopularDuration.first / avgDuration) < torrelance) {
            avgDuration = mostPopularDuration.first;
            AddMessage(VCE_LOG_DEBUG, _T("using popular duration...\n"));
        }
        //durationから求めた平均fpsを計算する
        const uint64_t mul = (uint64_t)ceil(1001.0 / m_Demux.video.pStream->time_base.num);
        estimatedAvgFps.num = (uint64_t)(m_Demux.video.pStream->time_base.den / avgDuration * (double)m_Demux.video.pStream->time_base.num * mul + 0.5);
        estimatedAvgFps.den = (uint64_t)m_Demux.video.pStream->time_base.num * mul;
        
        AddMessage(VCE_LOG_DEBUG, _T("fps mul:         %d\n"),    mul);
        AddMessage(VCE_LOG_DEBUG, _T("raw avgDuration: %lf\n"),   avgDuration);
        AddMessage(VCE_LOG_DEBUG, _T("estimatedAvgFps: %I64u/%I64u\n"), estimatedAvgFps.num, estimatedAvgFps.den);
    }

    if (m_Demux.video.nStreamPtsInvalid & RGY_PTS_ALL_INVALID) {
        //ptsとdurationをpkt_timebaseで適当に作成する
        nAvgFramerate64 = (fpsDecoderInvalid) ? estimatedAvgFps : fpsDecoder64;
    } else {
        if (fpsDecoderInvalid) {
            nAvgFramerate64 = estimatedAvgFps;
        } else {
            double dFpsDecoder = fpsDecoder.num / (double)fpsDecoder.den;
            double dEstimatedAvgFps = estimatedAvgFps.num / (double)estimatedAvgFps.den;
            //2フレーム分程度がもたらす誤差があっても許容する
            if (std::abs(dFpsDecoder / dEstimatedAvgFps - 1.0) < (2.0 / frameDurationList.size())) {
                AddMessage(VCE_LOG_DEBUG, _T("use decoder fps...\n"));
                nAvgFramerate64 = fpsDecoder64;
            } else {
                double dEstimatedAvgFpsCompare = estimatedAvgFps.num / (double)(estimatedAvgFps.den + ((dFpsDecoder < dEstimatedAvgFps) ? 1 : -1));
                //durationから求めた平均fpsがデコーダの出したfpsの近似値と分かれば、デコーダの出したfpsを採用する
                nAvgFramerate64 = (std::abs(dEstimatedAvgFps - dFpsDecoder) < std::abs(dEstimatedAvgFpsCompare - dFpsDecoder)) ? fpsDecoder64 : estimatedAvgFps;
            }
        }
    }
    AddMessage(VCE_LOG_DEBUG, _T("final AvgFps (raw64): %I64u/%I64u\n"), estimatedAvgFps.num, estimatedAvgFps.den);

    //フレームレートが2000fpsを超えることは考えにくいので、誤判定
    //ほかのなにか使えそうな値で代用する
    const auto codec_timebase = av_stream_get_codec_timebase(m_Demux.video.pStream);
    if (nAvgFramerate64.num / (double)nAvgFramerate64.den > 2000.0) {
        if (fpsDecoder.den > 0 && fpsDecoder.num > 0) {
            nAvgFramerate64.num = fpsDecoder.num;
            nAvgFramerate64.den = fpsDecoder.den;
        } else if (codec_timebase.den > 0
            && codec_timebase.num > 0) {
            const AVCodec *pCodec = avcodec_find_decoder(m_Demux.video.pStream->codecpar->codec_id);
            AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);
            nAvgFramerate64.num = codec_timebase.den * pCodecCtx->ticks_per_frame;
            nAvgFramerate64.den = codec_timebase.num;
            avcodec_free_context(&pCodecCtx);
        }
    }

    const uint64_t fps_gcd = vce_gcd(nAvgFramerate64.num, nAvgFramerate64.den);
    nAvgFramerate64.num /= fps_gcd;
    nAvgFramerate64.den /= fps_gcd;
    m_Demux.video.nAvgFramerate = av_make_q((int)nAvgFramerate64.num, (int)nAvgFramerate64.den);
    AddMessage(VCE_LOG_DEBUG, _T("final AvgFps (gcd): %d/%d\n"), m_Demux.video.nAvgFramerate.num, m_Demux.video.nAvgFramerate.den);

    //近似値であれば、分母1001/分母1に合わせる
    double fps = m_Demux.video.nAvgFramerate.num / (double)m_Demux.video.nAvgFramerate.den;
    double fps_n = fps * 1001;
    int fps_n_int = (int)(fps + 0.5) * 1000;
    if (std::abs(fps_n / (double)fps_n_int - 1.0) < 1e-4) {
        m_Demux.video.nAvgFramerate.num = fps_n_int;
        m_Demux.video.nAvgFramerate.den = 1001;
    } else {
        fps_n = fps * 1000;
        fps_n_int = (int)(fps + 0.5) * 1000;
        if (std::abs(fps_n / (double)fps_n_int - 1.0) < 1e-4) {
            m_Demux.video.nAvgFramerate.num = fps_n_int / 1000;
            m_Demux.video.nAvgFramerate.den = 1;
        }
    }
    AddMessage(VCE_LOG_DEBUG, _T("final AvgFps (round): %d/%d\n\n"), m_Demux.video.nAvgFramerate.num, m_Demux.video.nAvgFramerate.den);

    auto trimList = make_vector(pTrimList, nTrimCount);
    //出力時の音声・字幕解析用に1パケットコピーしておく
    if (m_Demux.qStreamPktL1.size()) { //この時点ではまだすべての音声パケットがL1にある
        if (m_Demux.qStreamPktL2.size() > 0) {
            AddMessage(VCE_LOG_ERROR, _T("qStreamPktL2 > 0, this is internal error.\n"));
            return AMF_UNEXPECTED;
        }
        for (auto streamInfo = m_Demux.stream.begin(); streamInfo != m_Demux.stream.end(); streamInfo++) {
            if (avcodec_get_type(streamInfo->pStream->codecpar->codec_id) == AVMEDIA_TYPE_AUDIO) {
                AddMessage(VCE_LOG_DEBUG, _T("checking for stream #%d\n"), streamInfo->nIndex);
                const AVPacket *pkt1 = nullptr; //最初のパケット
                const AVPacket *pkt2 = nullptr; //2番目のパケット
                //それで見つからなかったら、L1キューを探す
                for (int j = 0; j < (int)m_Demux.qStreamPktL1.size(); j++) {
                    if (m_Demux.qStreamPktL1[j].stream_index == streamInfo->nIndex) {
                        if (pkt1) {
                            pkt2 = &m_Demux.qStreamPktL1[j];
                            break;
                        }
                        pkt1 = &m_Demux.qStreamPktL1[j];
                    }
                }
                if (pkt1 != NULL) {
                    //1パケット目はたまにおかしいので、可能なら2パケット目を使用する
                    av_copy_packet(&streamInfo->pktSample, (pkt2) ? pkt2 : pkt1);
                    if (m_Demux.video.nStreamPtsInvalid & RGY_PTS_ALL_INVALID) {
                        streamInfo->nDelayOfStream = 0;
                    } else {
                        //その音声の属する動画フレーム番号
                        const int vidIndex = getVideoFrameIdx(pkt1->pts, streamInfo->pStream->time_base, 0);
                        AddMessage(VCE_LOG_DEBUG, _T("audio track %d first pts: %I64d\n"), streamInfo->nTrackId, pkt1->pts);
                        AddMessage(VCE_LOG_DEBUG, _T("      first pts videoIdx: %d\n"), vidIndex);
                        if (vidIndex >= 0) {
                            //音声の遅れているフレーム数分のdurationを足し上げる
                            int delayOfStream = (frame_inside_range(vidIndex, trimList)) ? (int)(pkt1->pts - m_Demux.frames.list(vidIndex).pts) : 0;
                            for (int iFrame = m_sTrimParam.offset; iFrame < vidIndex; iFrame++) {
                                if (frame_inside_range(iFrame, trimList)) {
                                    delayOfStream += m_Demux.frames.list(iFrame).duration;
                                }
                            }
                            streamInfo->nDelayOfStream = delayOfStream;
                            AddMessage(VCE_LOG_DEBUG, _T("audio track %d delay: %d (timebase=%d/%d)\n"),
                                streamInfo->nIndex, streamInfo->nTrackId,
                                streamInfo->nDelayOfStream, streamInfo->pStream->time_base.num, streamInfo->pStream->time_base.den);
                        }
                    }
                } else {
                    //音声の最初のサンプルを取得できていない
                    AddMessage(VCE_LOG_WARN, _T("failed to find stream #%d in preread.\n"), streamInfo->nIndex);
                    streamInfo = m_Demux.stream.erase(streamInfo) - 1;
                }
            }
        }
        if (m_Demux.stream.size() == 0) {
            //音声・字幕の最初のサンプルを取得できていないため、音声がすべてなくなってしまった
            AddMessage(VCE_LOG_ERROR, _T("failed to find audio/subtitle stream in preread.\n"));
            return AMF_UNEXPECTED;
        }
    }

    return AMF_OK;
}

#pragma warning(push)
#pragma warning(disable:4100)
AMF_RESULT CAvcodecReader::init(shared_ptr<VCELog> pLog, shared_ptr<VCEStatus> pStatus, VCEInputInfo *pInfo, amf::AMFContextPtr pContext) {

    Close();

    const AvcodecReaderPrm *input_prm = (const AvcodecReaderPrm *)pInfo->pPrivateParam;
    m_pPrintMes = pLog;
    m_pEncSatusInfo = pStatus;
    m_pContext = pContext;

    m_Demux.video.bReadVideo = input_prm->bReadVideo;
    if (input_prm->bReadVideo) {
        m_pEncSatusInfo = pStatus;
        memcpy(&m_sInputCrop, &pInfo->crop, sizeof(m_sInputCrop));
    } else {
        memset(&m_sInputCrop, 0, sizeof(m_sInputCrop));
    }

    if (!check_avcodec_dll()) {
        AddMessage(VCE_LOG_ERROR, error_mes_avcodec_dll_not_found());
        return AMF_INVALID_POINTER;
    }
    //if (!checkAvcodecLicense()) {
    //    m_strInputInfo += _T("avcodec: invalid dlls for VCEEncC.\n");
    //    return AMF_INVALID_POINTER;
    //}

    for (int i = 0; i < input_prm->nAudioSelectCount; i++) {
        tstring audioLog = strsprintf(_T("select audio track %s, codec %s"),
            (input_prm->ppAudioSelect[i]->nAudioSelect) ? strsprintf(_T("#%d"), input_prm->ppAudioSelect[i]->nAudioSelect).c_str() : _T("all"),
            input_prm->ppAudioSelect[i]->pAVAudioEncodeCodec);
        if (input_prm->ppAudioSelect[i]->pAudioExtractFormat) {
            audioLog += tstring(_T("format ")) + input_prm->ppAudioSelect[i]->pAudioExtractFormat;
        }
        if (input_prm->ppAudioSelect[i]->pAVAudioEncodeCodec != nullptr
            && 0 != _tcscmp(input_prm->ppAudioSelect[i]->pAVAudioEncodeCodec, AVVCE_CODEC_COPY)) {
            audioLog += strsprintf(_T("bitrate %d"), input_prm->ppAudioSelect[i]->nAVAudioEncodeBitrate);
        }
        if (input_prm->ppAudioSelect[i]->pAudioExtractFilename) {
            audioLog += tstring(_T("filename \"")) + input_prm->ppAudioSelect[i]->pAudioExtractFilename + tstring(_T("\""));
        }
        AddMessage(VCE_LOG_DEBUG, audioLog);
    }

    av_register_all();
    avcodec_register_all();
    avformatNetworkInit();
    av_log_set_level((m_pPrintMes->getLogLevel() == VCE_LOG_DEBUG) ?  AV_LOG_DEBUG : VCE_AV_LOG_LEVEL);
    av_vce_log_set(m_pPrintMes);

    int ret = 0;
    std::string filename_char;
    if (0 == tchar_to_string(input_prm->srcFile, filename_char, CP_UTF8)) {
        AddMessage(VCE_LOG_ERROR, _T("failed to convert filename to utf-8 characters.\n"));
        return AMF_INVALID_POINTER;
    }
    m_Demux.format.bIsPipe = (0 == strcmp(filename_char.c_str(), "-")) || filename_char.c_str() == strstr(filename_char.c_str(), R"(\\.\pipe\)");
    m_Demux.format.pFormatCtx = avformat_alloc_context();
    m_Demux.format.nAnalyzeSec = input_prm->nAnalyzeSec;
    if (m_Demux.format.nAnalyzeSec) {
        if (0 != (ret = av_opt_set_int(m_Demux.format.pFormatCtx, "probesize", m_Demux.format.nAnalyzeSec * AV_TIME_BASE, 0))) {
            AddMessage(VCE_LOG_ERROR, _T("failed to set probesize to %d sec: error %d\n"), m_Demux.format.nAnalyzeSec, ret);
        } else {
            AddMessage(VCE_LOG_DEBUG, _T("set probesize: %d sec\n"), m_Demux.format.nAnalyzeSec);
        }
    }
    if (0 == strcmp(filename_char.c_str(), "-")) {
#if defined(_WIN32) || defined(_WIN64)
        if (_setmode(_fileno(stdin), _O_BINARY) < 0) {
            AddMessage(VCE_LOG_ERROR, _T("failed to switch stdin to binary mode.\n"));
            return AMF_UNEXPECTED;
        }
#endif //#if defined(_WIN32) || defined(_WIN64)
        AddMessage(VCE_LOG_DEBUG, _T("input source set to stdin.\n"));
        filename_char = "pipe:0";
    }
    //ts向けの設定
    av_dict_set(&m_Demux.format.pFormatOptions, "scan_all_pmts", "1", 0);
    //ファイルのオープン
    if (avformat_open_input(&(m_Demux.format.pFormatCtx), filename_char.c_str(), nullptr, &m_Demux.format.pFormatOptions)) {
        AddMessage(VCE_LOG_ERROR, _T("error opening file: \"%s\"\n"), char_to_tstring(filename_char, CP_UTF8).c_str());
        return AMF_INVALID_POINTER; // Couldn't open file
    }
    AddMessage(VCE_LOG_DEBUG, _T("opened file \"%s\".\n"), char_to_tstring(filename_char, CP_UTF8).c_str());

    if (m_Demux.format.nAnalyzeSec) {
        if (0 != (ret = av_opt_set_int(m_Demux.format.pFormatCtx, "analyzeduration", m_Demux.format.nAnalyzeSec * AV_TIME_BASE, 0))) {
            AddMessage(VCE_LOG_ERROR, _T("failed to set analyzeduration to %d sec, error %d\n"), m_Demux.format.nAnalyzeSec, ret);
        } else {
            AddMessage(VCE_LOG_DEBUG, _T("set analyzeduration: %d sec\n"), m_Demux.format.nAnalyzeSec);
        }
    }

    if (avformat_find_stream_info(m_Demux.format.pFormatCtx, nullptr) < 0) {
        AddMessage(VCE_LOG_ERROR, _T("error finding stream information.\n"));
        return AMF_INVALID_POINTER; // Couldn't find stream information
    }
    AddMessage(VCE_LOG_DEBUG, _T("got stream information.\n"));
    av_dump_format(m_Demux.format.pFormatCtx, 0, filename_char.c_str(), 0);
    //dump_format(dec.pFormatCtx, 0, argv[1], 0);

    //キュー関連初期化
    //getFirstFramePosAndFrameRateで大量にパケットを突っ込む可能性があるので、この段階ではcapacityは無限大にしておく
    m_Demux.qVideoPkt.init(4096, SIZE_MAX, 4);
    m_Demux.qVideoPkt.set_keep_length(AVVCE_FRAME_MAX_REORDER);
    m_Demux.qStreamPktL2.init(4096);

    //動画ストリームを探す
    //動画ストリームは動画を処理しなかったとしても同期のため必要
    auto videoStreams = getStreamIndex(AVMEDIA_TYPE_VIDEO);
    if (videoStreams.size()) {
        if (input_prm->nVideoTrack) {
            if (videoStreams.size() < (uint32_t)std::abs(input_prm->nVideoTrack)) {
                AddMessage(VCE_LOG_ERROR, _T("track %d was selected for video, but input only contains %d video tracks.\n"), input_prm->nVideoTrack, videoStreams.size());
                return AMF_INVALID_ARG;
            } else if (input_prm->nVideoTrack < 0) {
                //逆順に並べ替え
                std::reverse(videoStreams.begin(), videoStreams.end());
            }
            m_Demux.video.nIndex = videoStreams[std::abs(input_prm->nVideoTrack)-1];
        } else if (input_prm->nVideoStreamId) {
            auto streamIndexFound = std::find_if(videoStreams.begin(), videoStreams.end(), [pFormatCtx = m_Demux.format.pFormatCtx, nSearchId = input_prm->nVideoStreamId](int nStreamIndex) {
                return (pFormatCtx->streams[nStreamIndex]->id == nSearchId);
            });
            if (streamIndexFound == videoStreams.end()) {
                AddMessage(VCE_LOG_ERROR, _T("stream id %d (0x%x) not found in video tracks.\n"), input_prm->nVideoStreamId, input_prm->nVideoStreamId);
                return AMF_INVALID_ARG;
            }
            m_Demux.video.nIndex = *streamIndexFound;
        } else {
            m_Demux.video.nIndex = videoStreams[0];
        }
        auto selectedStream = std::find(videoStreams.begin(), videoStreams.end(), m_Demux.video.nIndex);
        if (selectedStream == videoStreams.end()) {
            AddMessage(VCE_LOG_ERROR, _T("video stream lost!\n"));
            return AMF_UNEXPECTED;
        }
        //もし、選択された動画ストリームが先頭にないのなら、先頭に入れ替える
        if (selectedStream != videoStreams.begin()) {
            int nSelectedStreamIndex = *selectedStream;
            videoStreams.erase(selectedStream);
            videoStreams.insert(videoStreams.begin(), nSelectedStreamIndex);
        }
        AddMessage(VCE_LOG_DEBUG, _T("found video stream, stream idx %d\n"), m_Demux.video.nIndex);

        m_Demux.video.pStream = m_Demux.format.pFormatCtx->streams[m_Demux.video.nIndex];
    }

    //音声ストリームを探す
    if (input_prm->nReadAudio || input_prm->bReadSubtitle) {
        vector<int> mediaStreams;
        if (input_prm->nReadAudio) {
            auto audioStreams = getStreamIndex(AVMEDIA_TYPE_AUDIO, &videoStreams);
            //他のファイルから音声を読み込む場合もあるので、ここでチェックはできない
            //if (audioStreams.size() == 0) {
            //    AddMessage(VCE_LOG_ERROR, _T("--audio-encode/--audio-copy/--audio-file is set, but no audio stream found.\n"));
            //    return AMF_NOT_FOUND;
            //}
            m_Demux.format.nAudioTracks = (int)audioStreams.size();
            vector_cat(mediaStreams, audioStreams);
        }
        if (input_prm->bReadSubtitle) {
            auto subStreams = getStreamIndex(AVMEDIA_TYPE_SUBTITLE, &videoStreams);
            if (subStreams.size() == 0) {
                AddMessage(VCE_LOG_ERROR, _T("--sub-copy is set, but no subtitle stream found.\n"));
                return AMF_NOT_FOUND;
            }
            m_Demux.format.nSubtitleTracks = (int)subStreams.size();
            vector_cat(mediaStreams, subStreams);
        }
        for (int iTrack = 0; iTrack < (int)mediaStreams.size(); iTrack++) {
            const AVCodecID codecId = m_Demux.format.pFormatCtx->streams[mediaStreams[iTrack]]->codecpar->codec_id;
            bool useStream = false;
            sAudioSelect *pAudioSelect = nullptr; //トラックに対応するsAudioSelect (字幕ストリームの場合はnullptrのまま)
            if (AVMEDIA_TYPE_SUBTITLE == avcodec_get_type(codecId)) {
                //字幕の場合
                for (int i = 0; !useStream && i < input_prm->nSubtitleSelectCount; i++) {
                    if (input_prm->pSubtitleSelect[i] == 0 //特に指定なし = 全指定かどうか
                        || input_prm->pSubtitleSelect[i] == (iTrack - m_Demux.format.nAudioTracks + 1 + input_prm->nSubtitleTrackStart)) {
                        useStream = true;
                    }
                }
            } else {
                //音声の場合
                for (int i = 0; !useStream && i < input_prm->nAudioSelectCount; i++) {
                    if (input_prm->ppAudioSelect[i]->nAudioSelect == 0 //特に指定なし = 全指定かどうか
                        || input_prm->ppAudioSelect[i]->nAudioSelect == (iTrack + input_prm->nAudioTrackStart)) {
                        useStream = true;
                        pAudioSelect = input_prm->ppAudioSelect[i];
                    }
                }
            }
            if (useStream) {
                //存在するチャンネルまでのchannel_layoutのマスクを作成する
                //特に引数を指定せず--audio-channel-layoutを指定したときには、
                //pnStreamChannelsはchannelの存在しない不要なビットまで立っているのをここで修正
                if (pAudioSelect //字幕ストリームの場合は無視
                    && isSplitChannelAuto(pAudioSelect->pnStreamChannelSelect)) {
                    const uint64_t channel_layout_mask = UINT64_MAX >> (sizeof(channel_layout_mask) * 8 - m_Demux.format.pFormatCtx->streams[mediaStreams[iTrack]]->codecpar->channels);
                    for (uint32_t iSubStream = 0; iSubStream < MAX_SPLIT_CHANNELS; iSubStream++) {
                        pAudioSelect->pnStreamChannelSelect[iSubStream] &= channel_layout_mask;
                    }
                    for (uint32_t iSubStream = 0; iSubStream < MAX_SPLIT_CHANNELS; iSubStream++) {
                        pAudioSelect->pnStreamChannelOut[iSubStream] &= channel_layout_mask;
                    }
                }
                
                //必要であれば、サブストリームを追加する
                for (uint32_t iSubStream = 0; iSubStream == 0 || //初回は字幕・音声含め、かならず登録する必要がある
                    (iSubStream < MAX_SPLIT_CHANNELS //最大サブストリームの上限
                        && pAudioSelect != nullptr //字幕ではない
                        && pAudioSelect->pnStreamChannelSelect[iSubStream]); //audio-splitが指定されている
                    iSubStream++) {
                    AVDemuxStream stream = { 0 };
                    stream.nTrackId = (AVMEDIA_TYPE_SUBTITLE == avcodec_get_type(codecId))
                        ? -(iTrack - m_Demux.format.nAudioTracks + 1 + input_prm->nSubtitleTrackStart) //字幕は -1, -2, -3
                        : iTrack + input_prm->nAudioTrackStart; //音声は1, 2, 3
                    stream.nIndex = mediaStreams[iTrack];
                    stream.nSubStreamId = iSubStream;
                    stream.pStream = m_Demux.format.pFormatCtx->streams[stream.nIndex];
                    if (pAudioSelect) {
                        memcpy(stream.pnStreamChannelSelect, pAudioSelect->pnStreamChannelSelect, sizeof(stream.pnStreamChannelSelect));
                        memcpy(stream.pnStreamChannelOut,    pAudioSelect->pnStreamChannelOut,    sizeof(stream.pnStreamChannelOut));
                    }
                    m_Demux.stream.push_back(stream);
                    AddMessage(VCE_LOG_DEBUG, _T("found %s stream, stream idx %d, trackID %d.%d, %s, frame_size %d, timebase %d/%d\n"),
                        get_media_type_string(codecId).c_str(),
                        stream.nIndex, stream.nTrackId, stream.nSubStreamId, char_to_tstring(avcodec_get_name(codecId)).c_str(),
                        stream.pStream->codecpar->frame_size, stream.pStream->time_base.num, stream.pStream->time_base.den);
                }
            }
        }
        //指定されたすべての音声トラックが発見されたかを確認する
        for (int i = 0; i < input_prm->nAudioSelectCount; i++) {
            //全指定のトラック=0は無視
            if (input_prm->ppAudioSelect[i]->nAudioSelect > 0) {
                bool audioFound = false;
                for (const auto& stream : m_Demux.stream) {
                    if (stream.nTrackId == input_prm->ppAudioSelect[i]->nAudioSelect) {
                        audioFound = true;
                        break;
                    }
                }
                if (!audioFound) {
                    AddMessage(input_prm->bAudioIgnoreNoTrackError ? VCE_LOG_WARN : VCE_LOG_ERROR, _T("could not find audio track #%d\n"), input_prm->ppAudioSelect[i]->nAudioSelect);
                    if (!input_prm->bAudioIgnoreNoTrackError) {
                        return AMF_INVALID_ARG;
                    }
                }
            }
        }
    }

    if (input_prm->bReadChapter) {
        m_Demux.chapter = make_vector((const AVChapter **)m_Demux.format.pFormatCtx->chapters, m_Demux.format.pFormatCtx->nb_chapters);
    }

    //動画処理の初期化を行う
    if (input_prm->bReadVideo) {
        if (m_Demux.video.pStream == nullptr) {
            AddMessage(VCE_LOG_ERROR, _T("unable to find video stream.\n"));
            return AMF_INVALID_POINTER;
        }

        m_sFramePosListLog.clear();
        if (input_prm->pFramePosListLog) {
            m_sFramePosListLog = input_prm->pFramePosListLog;
        }

        memset(&m_inputFrameInfo, 0, sizeof(m_inputFrameInfo));

#define NO_HIGH_BIT_DEPTH_HW_DECODE 1

        struct pixfmtInfo {
            AVPixelFormat pix_fmt;
            uint16_t bit_depth;
            amf::AMF_SURFACE_FORMAT surfaceFormat;
        };

        static const pixfmtInfo pixfmtDataList[] = {
            { AV_PIX_FMT_YUV420P,      8, amf::AMF_SURFACE_NV12 },
            { AV_PIX_FMT_YUVJ420P,     8, amf::AMF_SURFACE_NV12 },
            { AV_PIX_FMT_NV12,         8, amf::AMF_SURFACE_NV12 },
            { AV_PIX_FMT_NV21,         8, amf::AMF_SURFACE_NV12 },
            { AV_PIX_FMT_YUV422P,      8, amf::AMF_SURFACE_UNKNOWN },
            { AV_PIX_FMT_YUVJ422P,     8, amf::AMF_SURFACE_UNKNOWN },
            { AV_PIX_FMT_YUYV422,      8, amf::AMF_SURFACE_YUY2 },
            { AV_PIX_FMT_UYVY422,      8, amf::AMF_SURFACE_YUY2 },
            { AV_PIX_FMT_NV16,         8, amf::AMF_SURFACE_UNKNOWN },
            { AV_PIX_FMT_YUV444P,      8, amf::AMF_SURFACE_UNKNOWN },
            { AV_PIX_FMT_YUVJ444P,     8, amf::AMF_SURFACE_UNKNOWN },
            { AV_PIX_FMT_YUV420P16LE, 16, amf::AMF_SURFACE_P010 },
            { AV_PIX_FMT_YUV420P14LE, 14, amf::AMF_SURFACE_P010 },
            { AV_PIX_FMT_YUV420P12LE, 12, amf::AMF_SURFACE_P010 },
            { AV_PIX_FMT_YUV420P10LE, 10, amf::AMF_SURFACE_P010 },
            { AV_PIX_FMT_YUV420P9LE,   9, amf::AMF_SURFACE_P010 },
            { AV_PIX_FMT_NV20LE,      10, amf::AMF_SURFACE_UNKNOWN },
            { AV_PIX_FMT_YUV422P16LE, 16, amf::AMF_SURFACE_UNKNOWN },
            { AV_PIX_FMT_YUV422P14LE, 14, amf::AMF_SURFACE_UNKNOWN },
            { AV_PIX_FMT_YUV422P12LE, 12, amf::AMF_SURFACE_UNKNOWN },
            { AV_PIX_FMT_YUV422P10LE, 10, amf::AMF_SURFACE_UNKNOWN },
            { AV_PIX_FMT_YUV444P16LE, 16, amf::AMF_SURFACE_UNKNOWN },
            { AV_PIX_FMT_YUV444P14LE, 14, amf::AMF_SURFACE_UNKNOWN },
            { AV_PIX_FMT_YUV444P12LE, 12, amf::AMF_SURFACE_UNKNOWN },
            { AV_PIX_FMT_YUV444P10LE, 10, amf::AMF_SURFACE_UNKNOWN }
        };

        const auto pixfmt = (AVPixelFormat)m_Demux.video.pStream->codecpar->format;
        const auto pixfmtData = std::find_if(pixfmtDataList, pixfmtDataList + _countof(pixfmtDataList), [pixfmt](const pixfmtInfo& tableData) {
            return tableData.pix_fmt == pixfmt;
        });
        if (pixfmtData == (pixfmtDataList + _countof(pixfmtDataList)) || pixfmtData->surfaceFormat == amf::AMF_SURFACE_UNKNOWN) {
            AddMessage(VCE_LOG_ERROR, _T("Invalid pixel format from input file.\n"));
            return AMF_FAIL;
        }

        //VCEでデコード可能かチェック
        bool bDecodecVCE = false;
        if (input_prm->nVideoDecodeSW != AV_DECODE_MODE_SW) {
            if (   0 == (m_nInputCodec = getQSVFourcc(m_Demux.video.pStream->codecpar->codec_id))
#if NO_HIGH_BIT_DEPTH_HW_DECODE
                || pixfmtData->surfaceFormat != amf::AMF_SURFACE_NV12
#endif
                //wmv3はAdvanced Profile (3)のみの対応
                || m_Demux.video.pStream->codecpar->codec_id == AV_CODEC_ID_WMV3 && m_Demux.video.pStream->codecpar->profile != 3) {
                if (input_prm->nVideoDecodeSW == AV_DECODE_MODE_HW) {
                    //avvceが指定されている場合にはエラー終了する
                    AddMessage(VCE_LOG_ERROR, _T("codec %s unable to decode by cuvid.\n"),
                        char_to_tstring(avcodec_get_name(m_Demux.video.pStream->codecpar->codec_id)).c_str());
                    return AMF_INVALID_POINTER;
                }
                m_nInputCodec = VCE_CODEC_NONE;
            } else {
                bDecodecVCE = true;
                AddMessage(VCE_LOG_DEBUG, _T("can be decoded by hw.\n"));
            }
        }
        m_strReaderName = (bDecodecVCE) ? _T("avvce") : _T("avsw");

        //HEVC入力の際に大量にメッセージが出て劇的に遅くなることがあるのを回避
        if (m_Demux.video.pStream->codecpar->codec_id == AV_CODEC_ID_HEVC) {
            AVDictionary *pDict = nullptr;
            av_dict_set_int(&pDict, "log_level_offset", AV_LOG_ERROR, 0);
            if (0 > (ret = av_opt_set_dict(m_Demux.video.pStream->codec, &pDict))) {
                AddMessage(VCE_LOG_WARN, _T("failed to set log_level_offset for HEVC codec reader.\n"));
            } else {
                AddMessage(VCE_LOG_ERROR, _T("set log_level_offset for HEVC codec reader.\n"));
            }
            av_dict_free(&pDict);
        }

        //必要ならbitstream filterを初期化
        if (m_Demux.video.pStream->codecpar->extradata && m_Demux.video.pStream->codecpar->extradata[0] == 1) {
            if (m_nInputCodec == VCE_CODEC_H264 || m_nInputCodec == VCE_CODEC_HEVC) {
                const char *filtername = nullptr;
                switch (m_nInputCodec) {
                case VCE_CODEC_H264: filtername = "h264_mp4toannexb"; break;
                case VCE_CODEC_HEVC: filtername = "hevc_mp4toannexb"; break;
                default: break;
                }
                if (filtername == nullptr) {
                    AddMessage(VCE_LOG_ERROR, _T("failed to set bitstream filter.\n"));
                    return AMF_FAIL;
                }
                auto filter = av_bsf_get_by_name(filtername);
                if (filter == nullptr) {
                    AddMessage(VCE_LOG_ERROR, _T("failed to find %s.\n"), char_to_tstring(filtername).c_str());
                    return AMF_NOT_FOUND;
                }
                if (0 > (ret = av_bsf_alloc(filter, &m_Demux.video.pBsfcCtx))) {
                    AddMessage(VCE_LOG_ERROR, _T("failed to allocate memory for %s: %s.\n"), char_to_tstring(filter->name).c_str(), qsv_av_err2str(ret).c_str());
                    return AMF_OUT_OF_MEMORY;
                }
                if (0 > (ret = avcodec_parameters_copy(m_Demux.video.pBsfcCtx->par_in, m_Demux.video.pStream->codecpar))) {
                    AddMessage(VCE_LOG_ERROR, _T("failed to copy parameter for %s: %s.\n"), char_to_tstring(filter->name).c_str(), qsv_av_err2str(ret).c_str());
                    return AMF_FAIL;
                }
                m_Demux.video.pBsfcCtx->time_base_in = m_Demux.video.pStream->time_base;
                if (0 > (ret = av_bsf_init(m_Demux.video.pBsfcCtx))) {
                    AddMessage(VCE_LOG_ERROR, _T("failed to init %s: %s.\n"), char_to_tstring(filter->name).c_str(), qsv_av_err2str(ret).c_str());
                    return AMF_FAIL;
                }
                AddMessage(VCE_LOG_DEBUG, _T("initialized %s filter.\n"), char_to_tstring(filter->name).c_str());
            }
        } else if (bDecodecVCE
            && (m_nInputCodec != VCE_CODEC_VP8 && m_nInputCodec != VCE_CODEC_VP9)
            && m_Demux.video.pStream->codecpar->extradata == NULL && m_Demux.video.pStream->codecpar->extradata_size == 0) {
            AddMessage(VCE_LOG_ERROR, _T("video header not extracted by libavcodec.\n"));
            return AMF_NOT_SUPPORTED;
        }
        AddMessage(VCE_LOG_DEBUG, _T("start predecode.\n"));

        AMF_RESULT sts = AMF_OK;
        sBitstream bitstream = { 0 };
        if (AMF_OK != (sts = GetHeader(&bitstream))) {
            AddMessage(VCE_LOG_ERROR, _T("failed to get header.\n"));
            return sts;
        }
        if (input_prm->fSeekSec > 0.0f) {
            AVPacket firstpkt;
            getSample(&firstpkt); //現在のtimestampを取得する
            const auto seek_time = av_rescale_q(1, av_d2q((double)input_prm->fSeekSec, 1<<24), m_Demux.video.pStream->time_base);
            int seek_ret = av_seek_frame(m_Demux.format.pFormatCtx, m_Demux.video.nIndex, firstpkt.pts + seek_time, 0);
            if (0 > seek_ret) {
                seek_ret = av_seek_frame(m_Demux.format.pFormatCtx, m_Demux.video.nIndex, firstpkt.pts + seek_time, AVSEEK_FLAG_ANY);
            }
            av_packet_unref(&firstpkt);
            if (0 > seek_ret) {
                AddMessage(VCE_LOG_ERROR, _T("failed to seek %s.\n"), print_time(input_prm->fSeekSec).c_str());
                return AMF_NOT_SUPPORTED;
            }
            //seekのために行ったgetSampleの結果は破棄する
            m_Demux.frames.clear();
        }

        //parserはseek後に初期化すること
        m_Demux.video.pParserCtx = av_parser_init(m_Demux.video.pStream->codecpar->codec_id);
        if (m_Demux.video.pParserCtx) {
            m_Demux.video.pParserCtx->flags |= PARSER_FLAG_COMPLETE_FRAMES;
            if (nullptr == (m_Demux.video.pCodecCtxParser = avcodec_alloc_context3(avcodec_find_decoder(m_Demux.video.pStream->codecpar->codec_id)))) {
                AddMessage(VCE_LOG_ERROR, _T("failed to allocate context for parser: %s.\n"), qsv_av_err2str(ret).c_str());
                return AMF_FAIL;
            }
            if (0 > (ret = avcodec_parameters_to_context(m_Demux.video.pCodecCtxParser, m_Demux.video.pStream->codecpar))) {
                AddMessage(VCE_LOG_ERROR, _T("failed to set codec param to context for parser: %s.\n"), qsv_av_err2str(ret).c_str());
                return AMF_FAIL;
            }
            m_Demux.video.pCodecCtxParser->time_base = av_stream_get_codec_timebase(m_Demux.video.pStream);
            m_Demux.video.pCodecCtxParser->pkt_timebase = m_Demux.video.pStream->time_base;
            AddMessage(VCE_LOG_ERROR, _T("initialized %s codec context for parser: time_base: %d/%d, pkt_timebase: %d/%d.\n"),
                char_to_tstring(avcodec_get_name(m_Demux.video.pStream->codecpar->codec_id)).c_str(),
                m_Demux.video.pCodecCtxParser->time_base.num, m_Demux.video.pCodecCtxParser->time_base.den,
                m_Demux.video.pCodecCtxParser->pkt_timebase.num, m_Demux.video.pCodecCtxParser->pkt_timebase.den);
        } else if (bDecodecVCE) {
            AddMessage(VCE_LOG_ERROR, _T("failed to init parser for %s.\n"), char_to_tstring(avcodec_get_name(m_Demux.video.pStream->codecpar->codec_id)).c_str());
            return AMF_FAIL;
        }

        if (AMF_OK != (sts = getFirstFramePosAndFrameRate(input_prm->pTrimList, input_prm->nTrimCount))) {
            AddMessage(VCE_LOG_ERROR, _T("failed to get first frame position.\n"));
            return sts;
        }

        m_sTrimParam.list = make_vector(input_prm->pTrimList, input_prm->nTrimCount);
        //キーフレームに到達するまでQSVではフレームが出てこない
        //そのぶんのずれを記録しておき、Trim値などに補正をかける
        if (m_sTrimParam.offset) {
            for (int i = (int)m_sTrimParam.list.size() - 1; i >= 0; i--) {
                if (m_sTrimParam.list[i].fin - m_sTrimParam.offset < 0) {
                    m_sTrimParam.list.erase(m_sTrimParam.list.begin() + i);
                } else {
                    m_sTrimParam.list[i].start = (std::max)(0, m_sTrimParam.list[i].start - m_sTrimParam.offset);
                    if (m_sTrimParam.list[i].fin != TRIM_MAX) {
                        m_sTrimParam.list[i].fin = (std::max)(0, m_sTrimParam.list[i].fin - m_sTrimParam.offset);
                    }
                }
            }
            //ずれが存在し、範囲指定がない場合はダミーの全域指定を追加する
            //これにより、自動的に音声側との同期がとれるようになる
            if (m_sTrimParam.list.size() == 0) {
                m_sTrimParam.list.push_back({ 0, TRIM_MAX });
            }
            AddMessage(VCE_LOG_DEBUG, _T("adjust trim by offset %d.\n"), m_sTrimParam.offset);
        }

        //あらかじめfpsが指定されていればそれを採用する
        if (input_prm->nVideoAvgFramerate.first * input_prm->nVideoAvgFramerate.second > 0) {
            m_Demux.video.nAvgFramerate.num = input_prm->nVideoAvgFramerate.first;
            m_Demux.video.nAvgFramerate.den = input_prm->nVideoAvgFramerate.second;
        }
        //getFirstFramePosAndFrameRateをもとにfpsを決定
        m_inputFrameInfo.fps.num = m_Demux.video.nAvgFramerate.num;
        m_inputFrameInfo.fps.den = m_Demux.video.nAvgFramerate.den;

        const auto aspectRatio = m_Demux.video.pStream->sample_aspect_ratio;
        const bool bAspectRatioUnknown = aspectRatio.num * aspectRatio.den <= 0;


        if (!bDecodecVCE) {
            if (nullptr == (m_Demux.video.pCodecDecode = avcodec_find_decoder(m_Demux.video.pStream->codecpar->codec_id))) {
                AddMessage(VCE_LOG_ERROR, errorMesForCodec(_T("Failed to find decoder"), m_Demux.video.pStream->codecpar->codec_id).c_str());
                return AMF_NOT_SUPPORTED;
            }
            if (nullptr == (m_Demux.video.pCodecCtxDecode = avcodec_alloc_context3(m_Demux.video.pCodecDecode))) {
                AddMessage(VCE_LOG_ERROR, errorMesForCodec(_T("Failed to allocate decoder"), m_Demux.video.pStream->codecpar->codec_id).c_str());
                return AMF_FAIL;
            }
            cpu_info_t cpu_info;
            if (get_cpu_info(&cpu_info)) {
                AVDictionary *pDict = nullptr;
                av_dict_set_int(&pDict, "threads", cpu_info.logical_cores, 0);
                if (0 > (ret = av_opt_set_dict(m_Demux.video.pCodecCtxDecode, &pDict))) {
                    AddMessage(VCE_LOG_ERROR, _T("Failed to set threads for decode (codec: %s): %s\n"),
                        char_to_tstring(avcodec_get_name(m_Demux.video.pStream->codecpar->codec_id)).c_str(), qsv_av_err2str(ret).c_str());
                    return AMF_FAIL;
                }
                av_dict_free(&pDict);
            }
            m_Demux.video.pCodecCtxDecode->time_base = av_stream_get_codec_timebase(m_Demux.video.pStream);
            m_Demux.video.pCodecCtxDecode->pkt_timebase = m_Demux.video.pStream->time_base;
            if (0 > (ret = avcodec_open2(m_Demux.video.pCodecCtxDecode, m_Demux.video.pCodecDecode, nullptr))) {
                AddMessage(VCE_LOG_ERROR, _T("Failed to open decoder for %s: %s\n"), char_to_tstring(avcodec_get_name(m_Demux.video.pStream->codecpar->codec_id)).c_str(), qsv_av_err2str(ret).c_str());
                return AMF_UNEXPECTED;
            }
            const std::map<AVPixelFormat, VCE_CSP> CSP_CONV = {
                { AV_PIX_FMT_YUV420P,     VCE_CSP_YV12 },
                { AV_PIX_FMT_YUVJ420P,    VCE_CSP_YV12 },
                { AV_PIX_FMT_NV12,        VCE_CSP_NV12 },
                { AV_PIX_FMT_NV21,        VCE_CSP_NV12 },
                { AV_PIX_FMT_YUV422P,     VCE_CSP_NA },
                { AV_PIX_FMT_YUVJ422P,    VCE_CSP_NA },
                { AV_PIX_FMT_YUYV422,     VCE_CSP_YUY2 },
                { AV_PIX_FMT_UYVY422,     VCE_CSP_NA },
                { AV_PIX_FMT_NV16,        VCE_CSP_NA },
                { AV_PIX_FMT_YUV444P,     VCE_CSP_YUV444 },
                { AV_PIX_FMT_YUVJ444P,    VCE_CSP_YUV444 },
                { AV_PIX_FMT_YUV420P16LE, VCE_CSP_YV12_16 },
                { AV_PIX_FMT_YUV420P14LE, VCE_CSP_YV12_14 },
                { AV_PIX_FMT_YUV420P12LE, VCE_CSP_YV12_12 },
                { AV_PIX_FMT_YUV420P10LE, VCE_CSP_YV12_10 },
                { AV_PIX_FMT_YUV420P9LE,  VCE_CSP_YV12_09 },
                { AV_PIX_FMT_NV20LE,      VCE_CSP_NA },
                { AV_PIX_FMT_YUV422P16LE, VCE_CSP_NA },
                { AV_PIX_FMT_YUV422P14LE, VCE_CSP_NA },
                { AV_PIX_FMT_YUV422P12LE, VCE_CSP_NA },
                { AV_PIX_FMT_YUV422P10LE, VCE_CSP_NA },
                { AV_PIX_FMT_YUV444P16LE, VCE_CSP_YUV444_16 },
                { AV_PIX_FMT_YUV444P14LE, VCE_CSP_YUV444_14 },
                { AV_PIX_FMT_YUV444P12LE, VCE_CSP_YUV444_12 },
                { AV_PIX_FMT_YUV444P10LE, VCE_CSP_YUV444_10 },
                { AV_PIX_FMT_YUV444P9LE,  VCE_CSP_YUV444_09 }
            };
            auto pixCspConv = CSP_CONV.find(m_Demux.video.pCodecCtxDecode->pix_fmt);
            if (pixCspConv == CSP_CONV.end()
                || nullptr == (m_sConvert = get_convert_csp_func(pixCspConv->second, VCE_CSP_NV12, false))) {
                AddMessage(VCE_LOG_ERROR, _T("invalid colorformat.\n"));
                return AMF_INVALID_FORMAT;
            }
            if (nullptr == (m_Demux.video.pFrame = av_frame_alloc())) {
                AddMessage(VCE_LOG_ERROR, _T("Failed to allocate frame for decoder.\n"));
                return AMF_OUT_OF_MEMORY;
            }
        }

        //情報を格納
        m_inputFrameInfo.format       = (NO_HIGH_BIT_DEPTH_HW_DECODE) ? amf::AMF_SURFACE_NV12 : pixfmtData->surfaceFormat;
        m_inputFrameInfo.srcWidth     = m_Demux.video.pStream->codecpar->width;
        m_inputFrameInfo.srcHeight    = m_Demux.video.pStream->codecpar->height;
        m_inputFrameInfo.AspectRatioW = ((bAspectRatioUnknown) ? 0 : aspectRatio.num);
        m_inputFrameInfo.AspectRatioH = ((bAspectRatioUnknown) ? 0 : aspectRatio.den);
        m_inputFrameInfo.frames       = 0;
        //インタレの可能性があるときは、MFX_PICSTRUCT_UNKNOWNを返すようにする
        m_inputFrameInfo.nPicStruct   = m_Demux.frames.getPicStruct();

        const tstring codecStr = (const TCHAR *)((bDecodecVCE) ? CodecIdToStr(m_nInputCodec) : char_to_tstring(avcodec_get_name(m_Demux.video.pStream->codecpar->codec_id))).c_str();
        tstring mes = strsprintf(_T("%s: %s, %dx%d, %d/%d fps"), m_strReaderName.c_str(), codecStr.c_str(),
            m_inputFrameInfo.srcWidth, m_inputFrameInfo.srcHeight, m_inputFrameInfo.fps.num, m_inputFrameInfo.fps.den);
        if (input_prm->fSeekSec > 0.0f) {
            mes += strsprintf(_T("\n               seek: %s"), print_time(input_prm->fSeekSec).c_str());
        }
        AddMessage(VCE_LOG_DEBUG, _T("%s, sar %d:%d\n"), mes.c_str(),
            m_inputFrameInfo.AspectRatioW, m_inputFrameInfo.AspectRatioH);
        m_strInputInfo += mes;

        //スレッド関連初期化
        m_Demux.thread.pQueueInfo = nullptr;
        m_Demux.thread.bAbortInput = false;
        auto nPrmInputThread = input_prm->nInputThread;
        m_Demux.thread.nInputThread = ((nPrmInputThread == VCE_INPUT_THREAD_AUTO) | (m_Demux.video.pCodecDecode != nullptr)) ? 0 : nPrmInputThread;;
        //if (m_Demux.thread.nInputThread == VCE_INPUT_THREAD_AUTO) {
        //    m_Demux.thread.nInputThread = 0;
        //}
        if (m_Demux.thread.nInputThread) {
            m_Demux.thread.thInput = std::thread(&CAvcodecReader::ThreadFuncRead, this);
            //はじめcapacityを無限大にセットしたので、この段階で制限をかける
            //入力をスレッド化しない場合には、自動的に同期が保たれるので、ここでの制限は必要ない
            m_Demux.qVideoPkt.set_capacity(256);
        }
    } else {
        //スレッド関連初期化 (スレッドは使用しないが、pQueueInfoはnullにしておく必要がある)
        m_Demux.thread.pQueueInfo = nullptr;
        m_Demux.thread.bAbortInput = false;
        //音声との同期とかに使うので、動画の情報を格納する
        m_Demux.video.nAvgFramerate = av_make_q(input_prm->nVideoAvgFramerate.first, input_prm->nVideoAvgFramerate.second);

        if (input_prm->nTrimCount) {
            m_sTrimParam.list = vector<sTrim>(input_prm->pTrimList, input_prm->pTrimList + input_prm->nTrimCount);
        }
    
        if (m_Demux.video.pStream) {
            //動画の最初のフレームを取得しておく
            AVPacket pkt;
            av_init_packet(&pkt);
            //音声のみ処理モードでは、動画の先頭をキーフレームとする必要はなく、
            //先頭がキーフレームでなくてもframePosListに追加するようにして、trimをoffsetなしで反映できるようにする
            //そこで、bTreatFirstPacketAsKeyframe=trueにして最初のパケットを処理する
            getSample(&pkt, true);
            av_packet_unref(&pkt);

            m_Demux.frames.checkPtsStatus();
        }

        tstring mes;
        for (const auto& stream : m_Demux.stream) {
            if (mes.length()) mes += _T(", ");
            tstring codec_name = char_to_tstring(avcodec_get_name(stream.pStream->codecpar->codec_id));
            mes += codec_name;
            AddMessage(VCE_LOG_DEBUG, _T("avcodec %s: %s from %s\n"),
                get_media_type_string(stream.pStream->codecpar->codec_id).c_str(), codec_name.c_str(), input_prm->srcFile);
        }
        m_strInputInfo += _T("avcodec audio: ") + mes;
    }

    return AMF_OK;
}
#pragma warning(pop)

vector<const AVChapter *> CAvcodecReader::GetChapterList() {
    return m_Demux.chapter;
}

int CAvcodecReader::GetSubtitleTrackCount() {
    return m_Demux.format.nSubtitleTracks;
}

int CAvcodecReader::GetAudioTrackCount() {
    return m_Demux.format.nAudioTracks;
}

int64_t CAvcodecReader::GetVideoFirstKeyPts() {
    return m_Demux.video.nStreamFirstKeyPts;
}

FramePosList *CAvcodecReader::GetFramePosList() {
    return &m_Demux.frames;
}

int CAvcodecReader::getVideoFrameIdx(int64_t pts, AVRational timebase, int iStart) {
    const int framePosCount = m_Demux.frames.frameNum();
    const AVRational vid_pkt_timebase = (m_Demux.video.pStream) ? m_Demux.video.pStream->time_base : av_inv_q(m_Demux.video.nAvgFramerate);
    for (int i = (std::max)(0, iStart); i < framePosCount; i++) {
        //pts < demux.videoFramePts[i]であるなら、その前のフレームを返す
        if (0 > av_compare_ts(pts, timebase, m_Demux.frames.list(i).pts, vid_pkt_timebase)) {
            return i - 1;
        }
    }
    return framePosCount;
}

int64_t CAvcodecReader::convertTimebaseVidToStream(int64_t pts, const AVDemuxStream *pStream) {
    const AVRational vid_pkt_timebase = (m_Demux.video.pStream) ? m_Demux.video.pStream->time_base : av_inv_q(m_Demux.video.nAvgFramerate);
    return av_rescale_q(pts, vid_pkt_timebase, pStream->pStream->time_base);
}

bool CAvcodecReader::checkStreamPacketToAdd(const AVPacket *pkt, AVDemuxStream *pStream) {
    pStream->nLastVidIndex = getVideoFrameIdx(pkt->pts, pStream->pStream->time_base, pStream->nLastVidIndex);

    //該当フレームが-1フレーム未満なら、その音声はこの動画には含まれない
    if (pStream->nLastVidIndex < -1) {
        return false;
    }

    const auto vidFramePos = &m_Demux.frames.list((std::max)(pStream->nLastVidIndex, 0));
    const int64_t vid_fin = convertTimebaseVidToStream(vidFramePos->pts + ((pStream->nLastVidIndex >= 0) ? vidFramePos->duration : 0), pStream);

    const int64_t aud_start = pkt->pts;
    const int64_t aud_fin   = pkt->pts + pkt->duration;

    const bool frame_is_in_range = frame_inside_range(pStream->nLastVidIndex,     m_sTrimParam.list);
    const bool next_is_in_range  = frame_inside_range(pStream->nLastVidIndex + 1, m_sTrimParam.list);

    bool result = true; //動画に含まれる音声かどうか

    if (frame_is_in_range) {
        if (aud_fin < vid_fin || next_is_in_range) {
            ; //完全に動画フレームの範囲内か、次のフレームも範囲内なら、その音声パケットは含まれる
        //              vid_fin
        //動画 <-----------|
        //音声      |-----------|
        //     aud_start     aud_fin
        } else if (pkt->duration / 2 > (aud_fin - vid_fin + pStream->nExtractErrExcess)) {
            //はみ出した領域が少ないなら、その音声パケットは含まれる
            pStream->nExtractErrExcess += aud_fin - vid_fin;
        } else {
            //はみ出した領域が多いなら、その音声パケットは含まれない
            pStream->nExtractErrExcess -= vid_fin - aud_start;
            result = false;
        }
    } else if (next_is_in_range && aud_fin > vid_fin) {
        //             vid_fin
        //動画             |------------>
        //音声      |-----------|
        //     aud_start     aud_fin
        if (pkt->duration / 2 > (vid_fin - aud_start + pStream->nExtractErrExcess)) {
            pStream->nExtractErrExcess += vid_fin - aud_start;
        } else {
            pStream->nExtractErrExcess -= aud_fin - vid_fin;
            result = false;
        }
    } else {
        result = false;
    }
    return result;
}

AVDemuxStream *CAvcodecReader::getPacketStreamData(const AVPacket *pkt) {
    int streamIndex = pkt->stream_index;
    for (int i = 0; i < (int)m_Demux.stream.size(); i++) {
        if (m_Demux.stream[i].nIndex == streamIndex) {
            return &m_Demux.stream[i];
        }
    }
    return NULL;
}

int CAvcodecReader::getSample(AVPacket *pkt, bool bTreatFirstPacketAsKeyframe) {
    av_init_packet(pkt);
    int i_samples = 0;
    while (av_read_frame(m_Demux.format.pFormatCtx, pkt) >= 0
        //trimからわかるフレーム数の上限値よりfixedNumがある程度の量の処理を進めたら読み込みを打ち切る
        && m_Demux.frames.fixedNum() - TRIM_OVERREAD_FRAMES < getVideoTrimMaxFramIdx()) {
        if (pkt->stream_index == m_Demux.video.nIndex) {
            if (m_Demux.video.pBsfcCtx) {
                auto ret = av_bsf_send_packet(m_Demux.video.pBsfcCtx, pkt);
                if (ret < 0) {
                    av_packet_unref(pkt);
                    AddMessage(VCE_LOG_ERROR, _T("failed to send packet to %s bitstream filter: %s.\n"), char_to_tstring(m_Demux.video.pBsfcCtx->filter->name).c_str(), qsv_av_err2str(ret).c_str());
                    return 1;
                }
                ret = av_bsf_receive_packet(m_Demux.video.pBsfcCtx, pkt);
                if (ret == AVERROR(EAGAIN)) {
                    continue; //もっとpacketを送らないとダメ
                } else if (ret < 0 && ret != AVERROR_EOF) {
                    AddMessage(VCE_LOG_ERROR, _T("failed to run %s bitstream filter: %s.\n"), char_to_tstring(m_Demux.video.pBsfcCtx->filter->name).c_str(), qsv_av_err2str(ret).c_str());
                    return 1;
                }
            }
            if (m_nInputCodec == VCE_CODEC_VC1) {
                vc1AddFrameHeader(pkt);
            }
            //最初のキーフレームを取得するまではスキップする
            //スキップした枚数はi_samplesでカウントし、trim時に同期を適切にとるため、m_sTrimParam.offsetに格納する
            //  ただし、bTreatFirstPacketAsKeyframeが指定されている場合には、キーフレームでなくてもframePosListへの追加を許可する
            //  このモードは、対象の入力ファイルから--audio-sourceなどで音声のみ拾ってくる場合に使用する
            if (!bTreatFirstPacketAsKeyframe && !m_Demux.video.bGotFirstKeyframe && !(pkt->flags & AV_PKT_FLAG_KEY)) {
                av_packet_unref(pkt);
                i_samples++;
                continue;
            } else {
                if (!m_Demux.video.bGotFirstKeyframe) {
                    //ここに入った場合は、必ず最初のキーフレーム
                    m_Demux.video.nStreamFirstKeyPts = pkt->pts;
                    m_Demux.video.bGotFirstKeyframe = true;
                    //キーフレームに到達するまでQSVではフレームが出てこない
                    //そのため、getSampleでも最初のキーフレームを取得するまでパケットを出力しない
                    //だが、これが原因でtrimの値とずれを生じてしまう
                    //そこで、そのぶんのずれを記録しておき、Trim値などに補正をかける
                    m_sTrimParam.offset = i_samples;
                }
                FramePos pos = { 0 };
                pos.pts = pkt->pts;
                pos.dts = pkt->dts;
                pos.duration = (int)pkt->duration;
                pos.duration2 = 0;
                pos.poc = AVVCE_POC_INVALID;
                pos.flags = (uint8_t)pkt->flags;
                if (m_Demux.video.pParserCtx) {
                    if (m_Demux.video.pBsfcCtx) {
                        std::swap(m_Demux.video.pExtradata, m_Demux.video.pCodecCtxParser->extradata);
                        std::swap(m_Demux.video.nExtradataSize, m_Demux.video.pCodecCtxParser->extradata_size);
                    }
                    uint8_t *dummy = nullptr;
                    int dummy_size = 0;
                    av_parser_parse2(m_Demux.video.pParserCtx, m_Demux.video.pCodecCtxParser, &dummy, &dummy_size, pkt->data, pkt->size, pkt->pts, pkt->dts, pkt->pos);
                    if (m_Demux.video.pBsfcCtx) {
                        std::swap(m_Demux.video.pExtradata, m_Demux.video.pCodecCtxParser->extradata);
                        std::swap(m_Demux.video.nExtradataSize, m_Demux.video.pCodecCtxParser->extradata_size);
                    }
                    pos.pict_type = (uint8_t)(std::max)(m_Demux.video.pParserCtx->pict_type, 0);
                    switch (m_Demux.video.pParserCtx->picture_structure) {
                    //フィールドとして符号化されている
                    case AV_PICTURE_STRUCTURE_TOP_FIELD:    pos.pic_struct = RGY_PICSTRUCT_FIELD_TOP; break;
                    case AV_PICTURE_STRUCTURE_BOTTOM_FIELD: pos.pic_struct = RGY_PICSTRUCT_FIELD_BOTTOM; break;
                    //フレームとして符号化されている
                    default:
                        switch (m_Demux.video.pParserCtx->field_order) {
                        case AV_FIELD_TT:
                        case AV_FIELD_TB: pos.pic_struct = RGY_PICSTRUCT_FRAME_TFF; break;
                        case AV_FIELD_BT:
                        case AV_FIELD_BB: pos.pic_struct = RGY_PICSTRUCT_FRAME_BFF; break;
                        default:          pos.pic_struct = RGY_PICSTRUCT_FRAME;     break;
                        }
                    }
                    pos.repeat_pict = (uint8_t)m_Demux.video.pParserCtx->repeat_pict;
                }
                m_Demux.frames.add(pos);
            }
            //ptsの確定したところまで、音声を出力する
            CheckAndMoveStreamPacketList();
            return 0;
        }
        if (getPacketStreamData(pkt) != NULL) {
            //音声/字幕パケットはひとまずすべてバッファに格納する
            m_Demux.qStreamPktL1.push_back(*pkt);
        } else {
            av_packet_unref(pkt);
        }
    }
    //ファイルの終わりに到達
    pkt->data = nullptr;
    pkt->size = 0;
    //動画の終端を表す最後のptsを挿入する
    int64_t videoFinPts = 0;
    const int nFrameNum = m_Demux.frames.frameNum();
    if (m_Demux.video.nStreamPtsInvalid & RGY_PTS_ALL_INVALID) {
        videoFinPts = nFrameNum * m_Demux.frames.list(0).duration;
    } else if (nFrameNum) {
        const FramePos *lastFrame = &m_Demux.frames.list(nFrameNum - 1);
        videoFinPts = lastFrame->pts + lastFrame->duration;
    }
    //もし選択範囲が手動で決定されていないのなら、音声を最大限取得する
    if (m_sTrimParam.list.size() == 0 || m_sTrimParam.list.back().fin == TRIM_MAX) {
        for (uint32_t i = 0; i < m_Demux.qStreamPktL2.size(); i++) {
            videoFinPts = (std::max)(videoFinPts, m_Demux.qStreamPktL2[i].data.pts);
        }
        for (uint32_t i = 0; i < m_Demux.qStreamPktL1.size(); i++) {
            videoFinPts = (std::max)(videoFinPts, m_Demux.qStreamPktL1[i].pts);
        }
    }
    //最後のフレーム情報をセットし、m_Demux.framesの内部状態を終了状態に移行する
    m_Demux.frames.fin(framePos(videoFinPts, videoFinPts, 0), m_Demux.format.pFormatCtx->duration);
    //映像キューのサイズ維持制限を解除する → パイプラインに最後まで読み取らせる
    m_Demux.qVideoPkt.set_keep_length(0);
    //音声をすべて出力する
    //m_Demux.frames.finをしたので、ここで実行すれば、qAudioPktL1のデータがすべてqAudioPktL2に移される
    CheckAndMoveStreamPacketList();
    //音声のみ読み込みの場合はm_pEncSatusInfoはnullptrなので、nullチェックを行う
    if (m_pEncSatusInfo) {
        m_pEncSatusInfo->UpdateDisplay(0, 100.0);
    }
    return 1;
}

AMF_RESULT CAvcodecReader::setToBitstream(sBitstream *bitstream, AVPacket *pkt) {
    AMF_RESULT sts = AMF_OK;
    if (pkt->data) {
        sts = bitstreamAppend(bitstream, pkt->data, pkt->size) ? AMF_OUT_OF_MEMORY : AMF_OK;
        bitstream->TimeStamp = 0;
    } else {
        sts = AMF_EOF;
    }
    return sts;
}

//動画ストリームの1フレーム分のデータをbitstreamに追加する (リーダー側のデータは消す)
AMF_RESULT CAvcodecReader::GetNextBitstream(amf::AMFData **ppData) {
    AVPacket pkt;
    if (!m_Demux.thread.thInput.joinable() //入力スレッドがなければ、自分で読み込む
        && m_Demux.qVideoPkt.get_keep_length() > 0) { //keep_length == 0なら読み込みは終了していて、これ以上読み込む必要はない
        if (0 == getSample(&pkt)) {
            m_Demux.qVideoPkt.push(pkt);
        }
    }

    bool bGetPacket = false;
    for (int i = 0; false == (bGetPacket = m_Demux.qVideoPkt.front_copy_and_pop_no_lock(&pkt, (m_Demux.thread.pQueueInfo) ? &m_Demux.thread.pQueueInfo->usage_vid_in : nullptr)) && m_Demux.qVideoPkt.size() > 0; i++) {
        m_Demux.qVideoPkt.wait_for_push();
    }
    AMF_RESULT sts = AMF_EOF;
    if (bGetPacket && pkt.size > 0) {
        amf::AMFBufferPtr pictureBuffer;
        AMF_RESULT ar = m_pContext->AllocBuffer(amf::AMF_MEMORY_HOST, pkt.size, &pictureBuffer);
        if (ar != AMF_OK) {
            AddMessage(VCE_LOG_ERROR, _T("Failed to allocate memory for input stream.\n"));
            return ar;
        }
        memcpy(pictureBuffer->GetNative(), pkt.data, pkt.size);
        amf_pts frameDuration = av_rescale_q(pkt.duration, m_Demux.video.pStream->time_base, av_make_q(1, AMF_SECOND)); // In 100 NanoSeconds
        pictureBuffer->SetDuration(frameDuration);
        pictureBuffer->SetPts(pkt.pts);
        *ppData = pictureBuffer.Detach();

        av_packet_unref(&pkt);
        m_Demux.video.nSampleGetCount++;
    }
    return sts;
}

AMF_RESULT CAvcodecReader::QueryOutput(amf::AMFData **ppData) {
    AMF_RESULT res = AMF_OK;
    if (m_Demux.video.pCodecDecode) {
        //動画のデコードを行う
        amf::AMFSurfacePtr pSurface;
        res = m_pContext->AllocSurface(amf::AMF_MEMORY_HOST, m_inputFrameInfo.format,
            m_inputFrameInfo.srcWidth - m_inputFrameInfo.crop.left - m_inputFrameInfo.crop.right,
            m_inputFrameInfo.srcHeight - m_inputFrameInfo.crop.bottom - m_inputFrameInfo.crop.up,
            &pSurface);
        if (res != AMF_OK) {
            AddMessage(VCE_LOG_ERROR, _T("AMFContext::AllocSurface(amf::AMF_MEMORY_HOST) failed.\n"));
            return res;
        }
        int got_frame = 0;
        while (!got_frame) {
            AVPacket pkt;
            av_init_packet(&pkt);
            if (!m_Demux.thread.thInput.joinable() //入力スレッドがなければ、自分で読み込む
                && m_Demux.qVideoPkt.get_keep_length() > 0) { //keep_length == 0なら読み込みは終了していて、これ以上読み込む必要はない
                if (0 == getSample(&pkt)) {
                    m_Demux.qVideoPkt.push(pkt);
                }
            }

            bool bGetPacket = false;
            for (int i = 0; false == (bGetPacket = m_Demux.qVideoPkt.front_copy_no_lock(&pkt)) && m_Demux.qVideoPkt.size() > 0; i++) {
                m_Demux.qVideoPkt.wait_for_push();
            }
            if (!bGetPacket) {
                //flushするためのパケット
                pkt.data = nullptr;
                pkt.size = 0;
            }
            int ret = avcodec_send_packet(m_Demux.video.pCodecCtxDecode, &pkt);
            //AVERROR(EAGAIN) -> パケットを送る前に受け取る必要がある
            //パケットが受け取られていないのでpopしない
            if (ret != AVERROR(EAGAIN)) {
                m_Demux.qVideoPkt.pop();
                av_packet_unref(&pkt);
            }
            if (ret == AVERROR_EOF) { //これ以上パケットを送れない
                AddMessage(VCE_LOG_ERROR, _T("failed to send packet to video decoder, already flushed: %s.\n"), qsv_av_err2str(ret).c_str());
                return AMF_FAIL;
            }
            if (ret < 0 && ret != AVERROR(EAGAIN)) {
                AddMessage(VCE_LOG_ERROR, _T("failed to send packet to video decoder: %s.\n"), qsv_av_err2str(ret).c_str());
                return AMF_FAIL;
            }
            ret = avcodec_receive_frame(m_Demux.video.pCodecCtxDecode, m_Demux.video.pFrame);
            if (ret == AVERROR(EAGAIN)) { //もっとパケットを送る必要がある
                continue;
            }
            if (ret == AVERROR_EOF) {
                //最後まで読み込んだ
                return AMF_EOF;
            }
            if (ret < 0) {
                AddMessage(VCE_LOG_ERROR, _T("failed to receive frame from video decoder: %s.\n"), qsv_av_err2str(ret).c_str());
                return AMF_FAIL;
            }
            got_frame = TRUE;
        }
        //フレームデータをコピー
        const auto plane = pSurface->GetPlaneAt(0);
        const int dst_stride = plane->GetHPitch();
        const int dst_height = plane->GetVPitch();

        void *dst_array[3];
        dst_array[0] = plane->GetNative();
        dst_array[1] = (uint8_t *)dst_array[0] + dst_stride * dst_height;
        dst_array[2] = (uint8_t *)dst_array[1] + dst_stride * dst_height; //YUV444出力時

        m_sConvert->func[!!m_Demux.video.pFrame->interlaced_frame](dst_array, (const void **)m_Demux.video.pFrame->data, m_inputFrameInfo.srcWidth, m_Demux.video.pFrame->linesize[0], m_Demux.video.pFrame->linesize[1], dst_stride, m_inputFrameInfo.srcHeight, dst_height, m_inputFrameInfo.crop.c);
        if (got_frame) {
            av_frame_unref(m_Demux.video.pFrame);
        }
        *ppData = pSurface.Detach();
    } else {
        if (m_Demux.qVideoPkt.size() == 0) {
            //m_Demux.qVideoPkt.size() == 0となるのは、最後まで読み込んだときか、中断した時しかありえない
            return AMF_EOF;
        }
        res = GetNextBitstream(ppData);
    }
    //進捗のみ表示
    m_pEncSatusInfo->m_nInputFrames++;
    double progressPercent = 0.0;
    if (m_Demux.format.pFormatCtx->duration) {
        progressPercent = m_Demux.frames.duration() * (m_Demux.video.pStream->time_base.num / (double)m_Demux.video.pStream->time_base.den) / (m_Demux.format.pFormatCtx->duration * (1.0 / (double)AV_TIME_BASE)) * 100.0;
    }
    m_pEncSatusInfo->UpdateDisplay(0, progressPercent);
    return res;
}

//動画ストリームの1フレーム分のデータをbitstreamに追加する (リーダー側のデータは残す)
AMF_RESULT CAvcodecReader::GetNextBitstreamNoDelete(amf::AMFData **ppData) {
    AVPacket pkt;
    if (!m_Demux.thread.thInput.joinable() //入力スレッドがなければ、自分で読み込む
        && m_Demux.qVideoPkt.get_keep_length() > 0) { //keep_length == 0なら読み込みは終了していて、これ以上読み込む必要はない
        if (0 == getSample(&pkt)) {
            m_Demux.qVideoPkt.push(pkt);
        }
    }

    bool bGetPacket = false;
    for (int i = 0; false == (bGetPacket = m_Demux.qVideoPkt.front_copy_no_lock(&pkt, (m_Demux.thread.pQueueInfo) ? &m_Demux.thread.pQueueInfo->usage_vid_in : nullptr)) && m_Demux.qVideoPkt.size() > 0; i++) {
        m_Demux.qVideoPkt.wait_for_push();
    }
    AMF_RESULT sts = AMF_EOF;
    if (bGetPacket && pkt.size > 0) {
        amf::AMFBufferPtr pictureBuffer;
        AMF_RESULT ar = m_pContext->AllocBuffer(amf::AMF_MEMORY_HOST, pkt.size, &pictureBuffer);
        if (ar != AMF_OK) {
            AddMessage(VCE_LOG_ERROR, _T("Failed to allocate memory for input stream.\n"));
            return ar;
        }
        memcpy(pictureBuffer->GetNative(), pkt.data, pkt.size);
        amf_pts frameDuration = av_rescale_q(pkt.duration, m_Demux.video.pStream->time_base, av_make_q(1, AMF_SECOND)); // In 100 NanoSeconds
        pictureBuffer->SetDuration(frameDuration);
        pictureBuffer->SetPts(pkt.pts);
        *ppData = pictureBuffer.Detach();
    }
    return sts;
}

void CAvcodecReader::GetAudioDataPacketsWhenNoVideoRead() {
    m_Demux.video.nSampleGetCount++;

    AVPacket pkt;
    av_init_packet(&pkt);
    if (m_Demux.video.pStream) {
        //動画に映像がある場合、getSampleを呼んで1フレーム分の音声データをm_Demux.qStreamPktL1に取得する
        //同時に映像フレームをロードし、ロードしたptsデータを突っ込む
        if (!getSample(&pkt)) {
            //動画データ自体は不要なので解放
            av_packet_unref(&pkt);
            CheckAndMoveStreamPacketList();
        }
        return;
    } else {
        const double vidEstDurationSec = m_Demux.video.nSampleGetCount * (double)m_Demux.video.nAvgFramerate.den / (double)m_Demux.video.nAvgFramerate.num; //1フレームの時間(秒)
        //動画に映像がない場合、
        //およそ1フレーム分のパケットを取得する
        while (av_read_frame(m_Demux.format.pFormatCtx, &pkt) >= 0) {
            if (m_Demux.format.pFormatCtx->streams[pkt.stream_index]->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
                av_packet_unref(&pkt);
            } else {
                AVDemuxStream *pStream = getPacketStreamData(&pkt);
                if (checkStreamPacketToAdd(&pkt, pStream)) {
                    m_Demux.qStreamPktL1.push_back(pkt);
                } else {
                    av_packet_unref(&pkt); //Writer側に渡さないパケットはここで開放する
                }

                //最初のパケットは参照用にコピーしておく
                if (pStream->pktSample.data == nullptr) {
                    av_copy_packet(&pStream->pktSample, &pkt);
                }
                uint64_t pktt = (pkt.pts == AV_NOPTS_VALUE) ? pkt.dts : pkt.pts;
                uint64_t pkt_dist = pktt - pStream->pktSample.pts;
                //1フレーム分のサンプルを取得したら終了
                if (pkt_dist * (double)pStream->pStream->time_base.num / (double)pStream->pStream->time_base.den > vidEstDurationSec) {
                    //およそ1フレーム分のパケットを設定する
                    int64_t pts = m_Demux.video.nSampleGetCount;
                    m_Demux.frames.add(framePos(pts, pts, 1, 0, m_Demux.video.nSampleGetCount, AV_PKT_FLAG_KEY));
                    if (m_Demux.frames.getStreamPtsStatus() == RGY_PTS_UNKNOWN) {
                        m_Demux.frames.checkPtsStatus();
                    }
                    CheckAndMoveStreamPacketList();
                    return;
                }
            }
        }
        //読み込みが終了
        int64_t pts = m_Demux.video.nSampleGetCount;
        m_Demux.frames.fin(framePos(pts, pts, 1, 0, m_Demux.video.nSampleGetCount, AV_PKT_FLAG_KEY), m_Demux.video.nSampleGetCount);
    }
}

const AVDictionary *CAvcodecReader::GetInputFormatMetadata() {
    return m_Demux.format.pFormatCtx->metadata;
}

const AVStream *CAvcodecReader::GetInputVideoStream() {
    return m_Demux.video.pStream;
}

//qStreamPktL1をチェックし、framePosListから必要な音声パケットかどうかを判定し、
//必要ならqStreamPktL2に移し、不要ならパケットを開放する
void CAvcodecReader::CheckAndMoveStreamPacketList() {
    if (m_Demux.frames.fixedNum() == 0) {
        return;
    }
    //出力するパケットを選択する
    const AVRational vid_pkt_timebase = (m_Demux.video.pStream) ? m_Demux.video.pStream->time_base : av_inv_q(m_Demux.video.nAvgFramerate);
    while (!m_Demux.qStreamPktL1.empty()) {
        auto pkt = m_Demux.qStreamPktL1.front();
        AVDemuxStream *pStream = getPacketStreamData(&pkt);
        //音声のptsが映像の終わりのptsを行きすぎたらやめる
        if (0 < av_compare_ts(pkt.pts, pStream->pStream->time_base, m_Demux.frames.list(m_Demux.frames.fixedNum()).pts, vid_pkt_timebase)) {
            break;
        }
        if (checkStreamPacketToAdd(&pkt, pStream)) {
            pkt.flags = (pkt.flags & 0xffff) | (pStream->nTrackId << 16); //flagsの上位16bitには、trackIdへのポインタを格納しておく
            m_Demux.qStreamPktL2.push(pkt); //Writer側に渡したパケットはWriter側で開放する
        } else {
            av_packet_unref(&pkt); //Writer側に渡さないパケットはここで開放する
        }
        m_Demux.qStreamPktL1.pop_front();
    }
}

vector<AVPacket> CAvcodecReader::GetStreamDataPackets() {
    if (!m_Demux.video.bReadVideo) {
        GetAudioDataPacketsWhenNoVideoRead();
    }

    //出力するパケットを選択する
    vector<AVPacket> packets;
    AVPacket pkt;
    while (m_Demux.qStreamPktL2.front_copy_and_pop_no_lock(&pkt, (m_Demux.thread.pQueueInfo) ? &m_Demux.thread.pQueueInfo->usage_aud_in : nullptr)) {
        packets.push_back(pkt);
    }
    return std::move(packets);
}

vector<AVDemuxStream> CAvcodecReader::GetInputStreamInfo() {
    return vector<AVDemuxStream>(m_Demux.stream.begin(), m_Demux.stream.end());
}

AMF_RESULT CAvcodecReader::GetHeader(sBitstream *bitstream) {
    if (bitstream == nullptr)
        return AMF_INVALID_POINTER;
    if (bitstream->Data == nullptr)
        if (0 != bitstreamInit(bitstream, AVCODEC_READER_INPUT_BUF_SIZE))
            return AMF_OUT_OF_MEMORY;

    if (m_Demux.video.pExtradata == nullptr) {
        m_Demux.video.nExtradataSize = m_Demux.video.pStream->codecpar->extradata_size;
        //ここでav_mallocを使用しないと正常に動作しない
        m_Demux.video.pExtradata = (uint8_t *)av_malloc(m_Demux.video.pStream->codecpar->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
        //ヘッダのデータをコピーしておく
        memcpy(m_Demux.video.pExtradata, m_Demux.video.pStream->codecpar->extradata, m_Demux.video.nExtradataSize);
        memset(m_Demux.video.pExtradata + m_Demux.video.nExtradataSize, 0, FF_INPUT_BUFFER_PADDING_SIZE);

        if (m_Demux.video.pBsfcCtx && m_Demux.video.pExtradata[0] == 1) {
            int ret = 0;
            auto pBsf = av_bsf_get_by_name(m_Demux.video.pBsfcCtx->filter->name);
            if (pBsf == nullptr) {
                AddMessage(VCE_LOG_ERROR, _T("failed find %s.\n"), char_to_tstring(m_Demux.video.pBsfcCtx->filter->name).c_str());
                return AMF_FAIL;
            }
            AVBSFContext *pBsfCtx = nullptr;
            if (0 > (ret = av_bsf_alloc(pBsf, &pBsfCtx))) {
                AddMessage(VCE_LOG_ERROR, _T("failed alloc memory for %s: %s.\n"), char_to_tstring(pBsf->name).c_str(), qsv_av_err2str(ret).c_str());
                return AMF_OUT_OF_MEMORY;
            }
            if (0 > (ret = avcodec_parameters_copy(pBsfCtx->par_in, m_Demux.video.pStream->codecpar))) {
                AddMessage(VCE_LOG_ERROR, _T("failed alloc get param for %s: %s.\n"), char_to_tstring(pBsf->name).c_str(), qsv_av_err2str(ret).c_str());
                return AMF_FAIL;
            }
            if (0 > (ret = av_bsf_init(pBsfCtx))) {
                AddMessage(VCE_LOG_ERROR, _T("failed init %s: %s.\n"), char_to_tstring(pBsf->name).c_str(), qsv_av_err2str(ret).c_str());
                return AMF_FAIL;
            }
            uint8_t H264_IDR[] = { 0x00, 0x00, 0x00, 0x01, 0x65 };
            uint8_t HEVC_IDR[] = { 0x00, 0x00, 0x00, 0x01, 19<<1 };
            AVPacket pkt = { 0 };
            av_init_packet(&pkt);
            switch (m_Demux.video.pStream->codecpar->codec_id) {
            case AV_CODEC_ID_H264: pkt.data = H264_IDR; pkt.size = sizeof(H264_IDR); break;
            case AV_CODEC_ID_HEVC: pkt.data = HEVC_IDR; pkt.size = sizeof(HEVC_IDR); break;
            default: break;
            }
            if (pkt.data == nullptr) {
                AddMessage(VCE_LOG_ERROR, _T("invalid codec to run %s.\n"), char_to_tstring(pBsf->name).c_str());
                return AMF_FAIL;
            }
            for (AVPacket *inpkt = &pkt; 0 == av_bsf_send_packet(pBsfCtx, inpkt); inpkt = nullptr) {
                ret = av_bsf_receive_packet(pBsfCtx, &pkt);
                if (ret == 0)
                    break;
                if (ret != AVERROR(EAGAIN) && !(inpkt && ret == AVERROR_EOF)) {
                    AddMessage(VCE_LOG_ERROR, _T("failed to run %s.\n"), char_to_tstring(pBsf->name).c_str());
                    return AMF_FAIL;
                }
            }
            av_bsf_free(&pBsfCtx);
            //VCEのHEVCデコーダはヘッダにSPS/PPS/VPS以外が入っているとエラー終了してしまう
            if (m_Demux.video.pStream->codecpar->codec_id == AV_CODEC_ID_HEVC) {
                const auto nal_units = parse_nal_unit_hevc(pkt.data, pkt.size);
                pkt.size = 0;
                for (auto nal_unit : nal_units) {
                    if (   nal_unit.type == NALU_HEVC_PPS
                        || nal_unit.type == NALU_HEVC_SPS
                        || nal_unit.type == NALU_HEVC_VPS) {
                        memcpy(pkt.data + pkt.size, nal_unit.ptr, nal_unit.size);
                        pkt.size += nal_unit.size;
                    }
                }
            }
            if (m_Demux.video.nExtradataSize < pkt.size) {
                m_Demux.video.pExtradata = (uint8_t *)av_realloc(m_Demux.video.pExtradata, m_Demux.video.pStream->codecpar->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
            }
            memcpy(m_Demux.video.pExtradata, pkt.data, pkt.size);
            m_Demux.video.nExtradataSize = pkt.size;
            av_packet_unref(&pkt);
        } else if (m_nInputCodec == VCE_CODEC_VC1) {
            int lengthFix = (0 == strcmp(m_Demux.format.pFormatCtx->iformat->name, "mpegts")) ? 0 : -1;
            vc1FixHeader(lengthFix);
        }
    }

    memcpy(bitstream->Data, m_Demux.video.pExtradata, m_Demux.video.nExtradataSize);
    bitstream->DataLength = m_Demux.video.nExtradataSize;
    return AMF_OK;
}

HANDLE CAvcodecReader::getThreadHandleInput() {
#if defined(WIN32) || defined(WIN64)
    return m_Demux.thread.thInput.native_handle();
#else
    return NULL;
#endif //#if defined(WIN32) || defined(WIN64)
}

AMF_RESULT CAvcodecReader::ThreadFuncRead() {
    while (!m_Demux.thread.bAbortInput) {
        AVPacket pkt;
        if (getSample(&pkt)) {
            break;
        }
        m_Demux.qVideoPkt.push(pkt);
    }
    return AMF_OK;
}

#endif //ENABLE_AVCODEC_VCE_READER
