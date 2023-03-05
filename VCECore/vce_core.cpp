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

#include <cmath>
#include <numeric>
#include "rgy_version.h"
#include "rgy_osdep.h"
#include "rgy_util.h"
#include "rgy_env.h"
#include "rgy_input.h"
#include "rgy_input_avi.h"
#include "rgy_input_avs.h"
#include "rgy_input_raw.h"
#include "rgy_input_vpy.h"
#include "rgy_input_sm.h"
#include "rgy_input_avcodec.h"
#include "rgy_output.h"
#include "rgy_output_avcodec.h"
#include "vce_core.h"
#include "vce_param.h"
#include "rgy_filter.h"
#include "rgy_filter_colorspace.h"
#include "rgy_filter_afs.h"
#include "rgy_filter_nnedi.h"
#include "rgy_filter_yadif.h"
#include "rgy_filter_convolution3d.h"
#include "rgy_filter_delogo.h"
#include "rgy_filter_denoise_knn.h"
#include "rgy_filter_denoise_pmd.h"
#include "rgy_filter_decimate.h"
#include "rgy_filter_mpdecimate.h"
#include "rgy_filter_smooth.h"
#include "rgy_filter_subburn.h"
#include "rgy_filter_unsharp.h"
#include "rgy_filter_edgelevel.h"
#include "rgy_filter_warpsharp.h"
#include "rgy_filter_curves.h"
#include "rgy_filter_tweak.h"
#include "rgy_filter_transform.h"
#include "rgy_filter_overlay.h"
#include "rgy_filter_deband.h"
#include "rgy_filesystem.h"
#include "rgy_version.h"
#include "rgy_bitstream.h"
#include "rgy_chapter.h"
#include "rgy_codepage.h"
#include "rgy_timecode.h"
#include "rgy_aspect_ratio.h"
#include "cpu_info.h"
#include "gpu_info.h"

#include "VideoEncoderVCE.h"
#include "VideoEncoderHEVC.h"
#include "VideoDecoderUVD.h"
#include "Factory.h"

#include "rgy_level_h264.h"
#include "rgy_level_hevc.h"
#include "rgy_level_av1.h"

VCECore::VCECore() :
    m_encCodec(RGY_CODEC_UNKNOWN),
    m_bTimerPeriodTuning(true),
#if ENABLE_AVSW_READER
    m_keyOnChapter(false),
    m_keyFile(),
    m_Chapters(),
#endif
    m_timecode(),
    m_hdr10plus(),
    m_hdr10plusMetadataCopy(false),
    m_hdrsei(),
    m_dovirpu(),
    m_encTimestamp(),
    m_trimParam(),
    m_poolPkt(),
    m_poolFrame(),
    m_pFileReader(),
    m_AudioReaders(),
    m_pFileWriter(),
    m_pFileWriterListAudio(),
    m_pStatus(),
    m_pPerfMonitor(),
    m_pipelineDepth(2),
    m_nProcSpeedLimit(0),
    m_nAVSyncMode(RGY_AVSYNC_ASSUME_CFR),
    m_inputFps(),
    m_encFps(),
    m_outputTimebase(),
    m_encWidth(0),
    m_encHeight(0),
    m_sar(),
    m_picStruct(RGY_PICSTRUCT_UNKNOWN),
    m_encVUI(),
    m_dev(),
    m_vpFilters(),
    m_pLastFilterParam(),
    m_videoQualityMetric(),
    m_state(RGY_STATE_STOPPED),
    m_pTrimParam(nullptr),
    m_pDecoder(),
    m_pEncoder(),
    m_thDecoder(),
    m_thOutput(),
    m_params(),
    m_pipelineTasks(),
    m_pAbortByUser(nullptr) {
}

VCECore::~VCECore() {
    Terminate();
}

void VCECore::Terminate() {
#if defined(_WIN32) || defined(_WIN64)
    if (m_bTimerPeriodTuning) {
        timeEndPeriod(1);
        PrintMes(RGY_LOG_DEBUG, _T("timeEndPeriod(1)\n"));
        m_bTimerPeriodTuning = false;
    }
#endif //#if defined(_WIN32) || defined(_WIN64)
    m_state = RGY_STATE_STOPPED;
    PrintMes(RGY_LOG_DEBUG, _T("Pipeline Stopped.\n"));

    m_videoQualityMetric.reset();

    m_pTrimParam = nullptr;

    m_pipelineTasks.clear();

    if (m_pEncoder != nullptr) {
        PrintMes(RGY_LOG_DEBUG, _T("Closing Encoder...\n"));
        m_pEncoder->Terminate();
        m_pEncoder = nullptr;
        PrintMes(RGY_LOG_DEBUG, _T("Closed Encoder.\n"));
    }

    if (m_pDecoder != nullptr) {
        PrintMes(RGY_LOG_DEBUG, _T("Closing Decoder...\n"));
        m_pDecoder->Terminate();
        m_pDecoder = nullptr;
        PrintMes(RGY_LOG_DEBUG, _T("Closed Decoder.\n"));
    }

    m_vpFilters.clear();
    m_pLastFilterParam.reset();
    m_dev.reset();
    m_timecode.reset();

    m_pFileWriterListAudio.clear();
    m_pFileWriter.reset();
    m_AudioReaders.clear();
    m_pFileReader.reset();
    m_Chapters.clear();
    m_keyFile.clear();
    m_hdr10plus.reset();
    m_hdrsei.reset();
    m_dovirpu.reset();
    m_encTimestamp.reset();
    m_pPerfMonitor.reset();
    m_pStatus.reset();
    m_tracer.reset();

    PrintMes(RGY_LOG_DEBUG, _T("Closing logger...\n"));
    m_pLog.reset();
    m_encCodec = RGY_CODEC_UNKNOWN;
    m_pAbortByUser = nullptr;
}

void VCECore::SetAbortFlagPointer(bool *abortFlag) {
    m_pAbortByUser = abortFlag;
}

RGY_ERR VCECore::readChapterFile(tstring chapfile) {
#if ENABLE_AVSW_READER
    ChapterRW chapter;
    auto err = chapter.read_file(chapfile.c_str(), CODE_PAGE_UNSET, 0.0);
    if (err != AUO_CHAP_ERR_NONE) {
        PrintMes(RGY_LOG_ERROR, _T("failed to %s chapter file: \"%s\".\n"), (err == AUO_CHAP_ERR_FILE_OPEN) ? _T("open") : _T("read"), chapfile.c_str());
        return RGY_ERR_UNKNOWN;
    }
    if (chapter.chapterlist().size() == 0) {
        PrintMes(RGY_LOG_ERROR, _T("no chapter found from chapter file: \"%s\".\n"), chapfile.c_str());
        return RGY_ERR_UNKNOWN;
    }
    m_Chapters.clear();
    const auto& chapter_list = chapter.chapterlist();
    tstring chap_log;
    for (size_t i = 0; i < chapter_list.size(); i++) {
        unique_ptr<AVChapter> avchap(new AVChapter);
        avchap->time_base = av_make_q(1, 1000);
        avchap->start = chapter_list[i]->get_ms();
        avchap->end = (i < chapter_list.size()-1) ? chapter_list[i+1]->get_ms() : avchap->start + 1;
        avchap->id = (int)m_Chapters.size();
        avchap->metadata = nullptr;
        av_dict_set(&avchap->metadata, "title", chapter_list[i]->name.c_str(), 0); //chapter_list[i]->nameはUTF-8になっている
        chap_log += strsprintf(_T("chapter #%02d [%d.%02d.%02d.%03d]: %s.\n"),
            avchap->id, chapter_list[i]->h, chapter_list[i]->m, chapter_list[i]->s, chapter_list[i]->ms,
            char_to_tstring(chapter_list[i]->name, CODE_PAGE_UTF8).c_str()); //chapter_list[i]->nameはUTF-8になっている
        m_Chapters.push_back(std::move(avchap));
    }
    PrintMes(RGY_LOG_DEBUG, _T("%s"), chap_log.c_str());
    return RGY_ERR_NONE;
#else
    PrintMes(RGY_LOG_ERROR, _T("chater reading unsupportted in this build"));
    return RGY_ERR_UNSUPPORTED;
#endif //#if ENABLE_AVSW_READER
}

RGY_ERR VCECore::initChapters(VCEParam *prm) {
#if ENABLE_AVSW_READER
    m_Chapters.clear();
    if (prm->common.chapterFile.length() > 0) {
        //チャプターファイルを読み込む
        auto chap_sts = readChapterFile(prm->common.chapterFile);
        if (chap_sts != RGY_ERR_NONE) {
            return chap_sts;
        }
    }
    if (m_Chapters.size() == 0) {
        auto pAVCodecReader = std::dynamic_pointer_cast<RGYInputAvcodec>(m_pFileReader);
        if (pAVCodecReader != nullptr) {
            auto chapterList = pAVCodecReader->GetChapterList();
            //入力ファイルのチャプターをコピーする
            for (uint32_t i = 0; i < chapterList.size(); i++) {
                unique_ptr<AVChapter> avchap(new AVChapter);
                *avchap = *chapterList[i];
                m_Chapters.push_back(std::move(avchap));
            }
        }
    }
    if (m_Chapters.size() > 0) {
        if (prm->common.keyOnChapter && m_trimParam.list.size() > 0) {
            PrintMes(RGY_LOG_WARN, _T("--key-on-chap not supported when using --trim.\n"));
        } else {
            m_keyOnChapter = prm->common.keyOnChapter;
        }
    }
#endif //#if ENABLE_AVSW_READER
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::initLog(VCEParam *prm) {
    m_pLog.reset(new RGYLog(prm->ctrl.logfile.c_str(), prm->ctrl.loglevel, prm->ctrl.logAddTime));
    if ((prm->ctrl.logfile.length() > 0 || prm->common.outputFilename.length() > 0) && prm->input.type != RGY_INPUT_FMT_SM) {
        m_pLog->writeFileHeader(prm->common.outputFilename.c_str());
    }
    return RGY_ERR_NONE;
}

//Power throttolingは消費電力削減に有効だが、
//fpsが高い場合やvppフィルタを使用する場合は、速度に悪影響がある場合がある
//そのあたりを適当に考慮し、throttolingのauto/onを自動的に切り替え
RGY_ERR VCECore::initPowerThrottoling(VCEParam *prm) {
    //解像度が低いほど、fpsが出やすい
    int score_resolution = 0;
    const int outputResolution = m_encWidth * m_encHeight;
    if (outputResolution <= 1024 * 576) {
        score_resolution += 4;
    } else if (outputResolution <= 1280 * 720) {
        score_resolution += 3;
    } else if (outputResolution <= 1920 * 1080) {
        score_resolution += 2;
    } else if (outputResolution <= 2560 * 1440) {
        score_resolution += 1;
    }
    const bool speedLimit = prm->ctrl.procSpeedLimit > 0 && prm->ctrl.procSpeedLimit <= 240;
    const int score = (speedLimit) ? 0 : score_resolution;

    //一定以上のスコアなら、throttolingをAuto、それ以外はthrottolingを有効にして消費電力を削減
    const int score_threshold = 3;
    const auto mode = (score >= score_threshold) ? RGYThreadPowerThrottlingMode::Auto : RGYThreadPowerThrottlingMode::Enabled;
    PrintMes(RGY_LOG_DEBUG, _T("selected mode %s : score %d: resolution %d, speed limit %s.\n"),
        rgy_thread_power_throttoling_mode_to_str(mode), score, score_resolution, speedLimit ? _T("on") : _T("off"));

    for (int i = (int)RGYThreadType::ALL + 1; i < (int)RGYThreadType::END; i++) {
        auto& target = prm->ctrl.threadParams.get((RGYThreadType)i);
        if (target.throttling == RGYThreadPowerThrottlingMode::Unset) {
            target.throttling = mode;
        }
    }
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::initPerfMonitor(VCEParam *prm) {
    const bool bLogOutput = prm->ctrl.perfMonitorSelect || prm->ctrl.perfMonitorSelectMatplot;
    tstring perfMonLog;
    if (bLogOutput) {
        perfMonLog = prm->common.outputFilename + _T("_perf.csv");
    }
    CPerfMonitorPrm perfMonitorPrm;
#if ENABLE_NVML
    perfMonitorPrm.pciBusId = selectedGpu->pciBusId.c_str();
#endif
    if (m_pPerfMonitor->init(perfMonLog.c_str(), _T(""), (bLogOutput) ? prm->ctrl.perfMonitorInterval : 1000,
        (int)prm->ctrl.perfMonitorSelect, (int)prm->ctrl.perfMonitorSelectMatplot,
#if defined(_WIN32) || defined(_WIN64)
        std::unique_ptr<void, handle_deleter>(OpenThread(SYNCHRONIZE | THREAD_QUERY_INFORMATION, false, GetCurrentThreadId()), handle_deleter()),
#else
        nullptr,
#endif
        prm->ctrl.threadParams.get(RGYThreadType::PERF_MONITOR),
        m_pLog, &perfMonitorPrm)) {
        PrintMes(RGY_LOG_WARN, _T("Failed to initialize performance monitor, disabled.\n"));
        m_pPerfMonitor.reset();
    }
    return RGY_ERR_NONE;
}

RGY_CSP VCECore::GetEncoderCSP(const VCEParam *inputParam) const {
    const int bitdepth = GetEncoderBitdepth(inputParam);
    if (bitdepth <= 0) {
        return RGY_CSP_NA;
    }
    const bool yuv444 = false;
    if (bitdepth > 8) {
        return (yuv444) ? RGY_CSP_YUV444_16 : RGY_CSP_P010;
    } else {
        return (yuv444) ? RGY_CSP_YUV444 : RGY_CSP_NV12;
    }
}

int VCECore::GetEncoderBitdepth(const VCEParam *inputParam) const {
    switch (inputParam->codec) {
    case RGY_CODEC_H264: return 8;
    case RGY_CODEC_HEVC:
    case RGY_CODEC_AV1:  return inputParam->outputDepth;
    default:
        return 0;
    }
}

RGY_ERR VCECore::initInput(VCEParam *inputParam, std::vector<std::unique_ptr<VCEDevice>> &gpuList) {
#if ENABLE_RAW_READER
    DeviceCodecCsp HWDecCodecCsp;
    for (const auto &gpu : gpuList) {
        HWDecCodecCsp.push_back(std::make_pair(gpu->id(), gpu->getHWDecCodecCsp()));
    }
    m_pStatus.reset(new EncodeStatus());

    int subburnTrackId = 0;
    for (const auto &subburn : inputParam->vpp.subburn) {
        if (subburn.trackId > 0) {
            subburnTrackId = subburn.trackId;
            break;
        }
    }

    //--input-cspの値 (raw読み込み用の入力色空間)
    //この後上書きするので、ここで保存する
    const auto inputCspOfRawReader = inputParam->input.csp;

    //入力モジュールが、エンコーダに返すべき色空間をセット
    inputParam->input.csp = GetEncoderCSP(inputParam);
    if (inputParam->input.csp == RGY_CSP_NA) {
        PrintMes(RGY_LOG_ERROR, _T("Unknown Error in GetEncoderCSP().\n"));
        return RGY_ERR_UNSUPPORTED;
    }

    m_poolPkt = std::make_unique<RGYPoolAVPacket>();
    m_poolFrame = std::make_unique<RGYPoolAVFrame>();

    const bool vpp_rff = false; // inputParam->vpp.rff;
    auto err = initReaders(m_pFileReader, m_AudioReaders, &inputParam->input, inputCspOfRawReader,
        m_pStatus, &inputParam->common, &inputParam->ctrl, HWDecCodecCsp, subburnTrackId,
        inputParam->vpp.afs.enable, vpp_rff,
        m_poolPkt.get(), m_poolFrame.get(), nullptr, m_pPerfMonitor.get(), m_pLog);
    if (err != RGY_ERR_NONE) {
        PrintMes(RGY_LOG_ERROR, _T("failed to initialize file reader(s).\n"));
        return err;
    }

    m_inputFps = rgy_rational<int>(inputParam->input.fpsN, inputParam->input.fpsD);
    m_outputTimebase = (inputParam->common.timebase.is_valid()) ? inputParam->common.timebase : m_inputFps.inv() * rgy_rational<int>(1, 4);
    if (inputParam->common.tcfileIn.length() > 0) {
        PrintMes(RGY_LOG_DEBUG, _T("Switching to VFR mode as --tcfile-in is used.\n"));
        m_nAVSyncMode |= RGY_AVSYNC_VFR;
    }
    if (m_nAVSyncMode & RGY_AVSYNC_VFR) {
        //avsync vfr時は、入力streamのtimebaseをそのまま使用する
        m_outputTimebase = m_pFileReader->getInputTimebase();
    }

    //trim情報の作成
    if (
#if ENABLE_AVSW_READER
        std::dynamic_pointer_cast<RGYInputAvcodec>(m_pFileReader) == nullptr &&
#endif
        inputParam->common.pTrimList && inputParam->common.nTrimCount > 0) {
        //avhw/avswリーダー以外は、trimは自分ではセットされないので、ここでセットする
        sTrimParam trimParam;
        trimParam.list = make_vector(inputParam->common.pTrimList, inputParam->common.nTrimCount);
        trimParam.offset = 0;
        m_pFileReader->SetTrimParam(trimParam);
    }
    //trim情報をリーダーから取得する
    m_trimParam = m_pFileReader->GetTrimParam();
    if (m_trimParam.list.size() > 0) {
        PrintMes(RGY_LOG_DEBUG, _T("Input: trim options\n"));
        for (int i = 0; i < (int)m_trimParam.list.size(); i++) {
            PrintMes(RGY_LOG_DEBUG, _T("%d-%d "), m_trimParam.list[i].start, m_trimParam.list[i].fin);
        }
        PrintMes(RGY_LOG_DEBUG, _T(" (offset: %d)\n"), m_trimParam.offset);
    }

#if ENABLE_AVSW_READER
    auto pAVCodecReader = std::dynamic_pointer_cast<RGYInputAvcodec>(m_pFileReader);
    const bool vpp_afs = inputParam->vpp.afs.enable;
    if ((m_nAVSyncMode & (RGY_AVSYNC_VFR | RGY_AVSYNC_FORCE_CFR))/* || inputParam->vpp.rff*/) {
        tstring err_target;
        if (m_nAVSyncMode & RGY_AVSYNC_VFR)       err_target += _T("avsync vfr, ");
        if (m_nAVSyncMode & RGY_AVSYNC_FORCE_CFR) err_target += _T("avsync forcecfr, ");
        if (vpp_rff)                  err_target += _T("vpp-rff, ");
        err_target = err_target.substr(0, err_target.length()-2);

        if (pAVCodecReader) {
            //timestampになんらかの問題がある場合、vpp-rffとavsync vfrは使用できない
            const auto timestamp_status = pAVCodecReader->GetFramePosList()->getStreamPtsStatus();
            if ((timestamp_status & (~RGY_PTS_NORMAL)) != 0) {

                tstring err_sts;
                if (timestamp_status & RGY_PTS_SOMETIMES_INVALID) err_sts += _T("SOMETIMES_INVALID, "); //時折、無効なptsを得る
                if (timestamp_status & RGY_PTS_HALF_INVALID)      err_sts += _T("HALF_INVALID, "); //PAFFなため、半分のフレームのptsやdtsが無効
                if (timestamp_status & RGY_PTS_ALL_INVALID)       err_sts += _T("ALL_INVALID, "); //すべてのフレームのptsやdtsが無効
                if (timestamp_status & RGY_PTS_NONKEY_INVALID)    err_sts += _T("NONKEY_INVALID, "); //キーフレーム以外のフレームのptsやdtsが無効
                if (timestamp_status & RGY_PTS_DUPLICATE)         err_sts += _T("PTS_DUPLICATE, "); //重複するpts/dtsが存在する
                if (timestamp_status & RGY_DTS_SOMETIMES_INVALID) err_sts += _T("DTS_SOMETIMES_INVALID, "); //時折、無効なdtsを得る
                err_sts = err_sts.substr(0, err_sts.length()-2);

                PrintMes(RGY_LOG_ERROR, _T("timestamp not acquired successfully from input stream, %s cannot be used. \n  [0x%x] %s\n"),
                    err_target.c_str(), (uint32_t)timestamp_status, err_sts.c_str());
                return RGY_ERR_INVALID_VIDEO_PARAM;
            }
            PrintMes(RGY_LOG_DEBUG, _T("timestamp check: 0x%x\n"), timestamp_status);
        } else if (m_outputTimebase.n() == 0 || !m_outputTimebase.is_valid()) {
            PrintMes(RGY_LOG_ERROR, _T("%s cannot be used with current reader.\n"), err_target.c_str());
            return RGY_ERR_INVALID_VIDEO_PARAM;
        }
    } else if (pAVCodecReader && ((pAVCodecReader->GetFramePosList()->getStreamPtsStatus() & (~RGY_PTS_NORMAL)) == 0)) {
        m_nAVSyncMode |= RGY_AVSYNC_VFR;
        const auto timebaseStreamIn = to_rgy(pAVCodecReader->GetInputVideoStream()->time_base);
        if ((timebaseStreamIn.inv() * m_inputFps.inv()).d() == 1 || timebaseStreamIn.n() > 1000) { //fpsを割り切れるtimebaseなら
            if (!vpp_afs && !vpp_rff) {
                m_outputTimebase = m_inputFps.inv() * rgy_rational<int>(1, 8);
            }
        }
        PrintMes(RGY_LOG_DEBUG, _T("vfr mode automatically enabled with timebase %d/%d\n"), m_outputTimebase.n(), m_outputTimebase.d());
    }
#endif
    if (inputParam->common.dynamicHdr10plusJson.length() > 0) {
        m_hdr10plus = initDynamicHDR10Plus(inputParam->common.dynamicHdr10plusJson, m_pLog);
        if (!m_hdr10plus) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to initialize hdr10plus reader.\n"));
            return RGY_ERR_UNKNOWN;
        }
    } else if (inputParam->common.hdr10plusMetadataCopy) {
        m_hdr10plusMetadataCopy = true;
        if (pAVCodecReader != nullptr) {
            const auto timestamp_status = pAVCodecReader->GetFramePosList()->getStreamPtsStatus();
            if ((timestamp_status & (~RGY_PTS_NORMAL)) != 0) {
                PrintMes(RGY_LOG_ERROR, _T("HDR10+ dynamic metadata cannot be copied from input file using avhw reader, as timestamp was not properly got from input file.\n"));
                PrintMes(RGY_LOG_ERROR, _T("Please consider using avsw reader.\n"));
                return RGY_ERR_UNSUPPORTED;
            }
        }
    }
    if (inputParam->common.doviRpuFile.length() > 0) {
        m_dovirpu = std::make_unique<DOVIRpu>();
        if (m_dovirpu->init(inputParam->common.doviRpuFile.c_str()) != 0) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to open dovi rpu \"%s\".\n"), inputParam->common.doviRpuFile.c_str());
            return RGY_ERR_FILE_OPEN;
        }
    }

    m_hdrsei = createHEVCHDRSei(inputParam->common.maxCll, inputParam->common.masterDisplay, inputParam->common.atcSei, m_pFileReader.get());
    if (!m_hdrsei) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to parse HEVC HDR10 metadata.\n"));
        return RGY_ERR_UNSUPPORTED;
    }
#endif
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::checkParam(VCEParam *prm) {
    if (prm->input.fpsN <= 0 || prm->input.fpsD <= 0) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid fps - zero or negative (%d/%d).\n"), prm->input.fpsN, prm->input.fpsD);
        return RGY_ERR_INVALID_PARAM;
    }
    rgy_reduce(prm->input.fpsN, prm->input.fpsD);
    if (prm->input.srcWidth <= 0 || prm->input.srcHeight <= 0) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid frame size - zero or negative (%dx%d).\n"), prm->input.srcWidth, prm->input.srcHeight);
        return RGY_ERR_INVALID_PARAM;
    }
    const int h_mul = (prm->input.picstruct & RGY_PICSTRUCT_INTERLACED) ? 4 : 2;
    if (prm->input.srcWidth % 2 != 0) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid input frame size - non mod2 (width: %d).\n"), prm->input.srcWidth);
        return RGY_ERR_INVALID_PARAM;
    }
    if (prm->input.srcHeight % h_mul != 0) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid input frame size - non mod%d (height: %d).\n"), h_mul, prm->input.srcHeight);
        return RGY_ERR_INVALID_PARAM;
    }
    if (prm->input.srcWidth - (prm->input.crop.e.left + prm->input.crop.e.right) < 0
        || prm->input.srcHeight - (prm->input.crop.e.bottom + prm->input.crop.e.up) < 0) {
        PrintMes(RGY_LOG_ERROR, _T("crop size is too big.\n"));
        return RGY_ERR_INVALID_PARAM;
    }
    if (prm->input.crop.e.left % 2 != 0) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid crop - non mod2 (left: %d).\n"), prm->input.crop.e.left);
        return RGY_ERR_INVALID_PARAM;
    }
    if (prm->input.crop.e.right % 2 != 0) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid crop - non mod2 (right: %d).\n"), prm->input.crop.e.right);
        return RGY_ERR_INVALID_PARAM;
    }
    if (prm->input.crop.e.bottom % 2 != 0) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid crop - non mod2 (bottom: %d).\n"), prm->input.crop.e.bottom);
        return RGY_ERR_INVALID_PARAM;
    }
    if (prm->input.crop.e.up % 2 != 0) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid crop - non mod2 (up: %d).\n"), prm->input.crop.e.up);
        return RGY_ERR_INVALID_PARAM;
    }
    if (prm->input.dstWidth % 2 != 0) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid output frame size - non mod2 (width: %d).\n"), prm->input.dstWidth);
        return RGY_ERR_INVALID_PARAM;
    }
    if (prm->input.dstHeight % h_mul != 0) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid output frame size - non mod%d (height: %d).\n"), h_mul, prm->input.dstHeight);
        return RGY_ERR_INVALID_PARAM;
    }
    if (prm->input.dstWidth < 0 && prm->input.dstHeight < 0) {
        PrintMes(RGY_LOG_ERROR, _T("Either one of output resolution must be positive value.\n"));
        return RGY_ERR_INVALID_VIDEO_PARAM;
    }
    auto outpar = std::make_pair(prm->par[0], prm->par[1]);
    if ((!prm->par[0] || !prm->par[1]) //SAR比の指定がない
        && prm->input.sar[0] && prm->input.sar[1] //入力側からSAR比を取得ずみ
        && (prm->input.dstWidth == prm->input.srcWidth && prm->input.dstHeight == prm->input.srcHeight)) {//リサイズは行われない
        outpar = std::make_pair(prm->input.sar[0], prm->input.sar[1]);
    }
    set_auto_resolution(prm->input.dstWidth, prm->input.dstHeight, outpar.first, outpar.second,
        prm->input.srcWidth, prm->input.srcHeight, prm->input.sar[0], prm->input.sar[1], 2, 2, prm->inprm.resizeResMode, prm->input.crop);

    if (prm->codec == RGY_CODEC_UNKNOWN) {
        prm->codec = RGY_CODEC_H264;
    }
    if (prm->nBframes > VCE_MAX_BFRAMES) {
        PrintMes(RGY_LOG_WARN, _T("Maximum consecutive B frames is %d.\n"), VCE_MAX_BFRAMES);
        prm->nBframes = VCE_MAX_BFRAMES;
    }
    if (prm->nGOPLen > VCE_MAX_GOP_LEN) {
        PrintMes(RGY_LOG_WARN, _T("Maximum GOP len is %d.\n"), VCE_MAX_GOP_LEN);
        prm->nGOPLen = VCE_MAX_GOP_LEN;
    }
    if (abs(prm->nDeltaQPBFrame) > VCE_MAX_B_DELTA_QP) {
        PrintMes(RGY_LOG_WARN, _T("Maximum Delta QP for Bframes is %d.\n"), VCE_MAX_B_DELTA_QP);
        prm->nDeltaQPBFrame = clamp(prm->nDeltaQPBFrame, -1 * VCE_MAX_B_DELTA_QP, VCE_MAX_B_DELTA_QP);
    }
    if (abs(prm->nDeltaQPBFrameRef) > VCE_MAX_B_DELTA_QP) {
        PrintMes(RGY_LOG_WARN, _T("Maximum Delta QP for Bframes is %d.\n"), VCE_MAX_B_DELTA_QP);
        prm->nDeltaQPBFrameRef = clamp(prm->nDeltaQPBFrameRef, -1 * VCE_MAX_B_DELTA_QP, VCE_MAX_B_DELTA_QP);
    }
    if (prm->bVBAQ && prm->codec == RGY_CODEC_HEVC && prm->rateControl == AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CONSTANT_QP) {
#ifndef VCE_AUO
        PrintMes(RGY_LOG_WARN, _T("VBAQ is not supported with CQP + HEVC encoding, disabled.\n"));
#endif
        prm->bVBAQ = 0;
    }
    if (prm->codec == RGY_CODEC_HEVC) {
        if (prm->codecParam[prm->codec].nProfile == AMF_VIDEO_ENCODER_HEVC_PROFILE_MAIN_10) {
            prm->outputDepth = 10;
        }
        //RX5500XTがmain10をサポートしないと返したりするので、ここはひとまず無効化する
        //if (prm->outputDepth == 10) {
        //    prm->codecParam[prm->codec].nProfile = AMF_VIDEO_ENCODER_HEVC_PROFILE_MAIN_10;
        //}
    }
    if (prm->nBframes > 0 && prm->codec == RGY_CODEC_HEVC) {
        PrintMes(RGY_LOG_WARN, _T("Bframes is not supported with HEVC encoding, disabled.\n"));
        prm->nBframes = 0;
        prm->bBPyramid = 0;
        prm->nDeltaQPBFrame = 0;
        prm->nDeltaQPBFrameRef = 0;
    }
    if (prm->codec == RGY_CODEC_H264) {
        if (prm->outputDepth != 8) {
            PrintMes(RGY_LOG_WARN, _T("Only 8 bitdepth is supported in H.264 encoding.\n"));
            prm->outputDepth = 8;
        }
    }
    const int maxQP = (prm->codec == RGY_CODEC_AV1) ? 255 : (prm->outputDepth > 8 ? 63 : 51);
    if (prm->nQPMin.has_value())      prm->nQPMin      = clamp(prm->nQPMin,      0, maxQP);
    if (prm->nQPMax.has_value())      prm->nQPMax      = clamp(prm->nQPMax,      0, maxQP);
    if (prm->nQPMinInter.has_value()) prm->nQPMinInter = clamp(prm->nQPMinInter, 0, maxQP);
    if (prm->nQPMaxInter.has_value()) prm->nQPMaxInter = clamp(prm->nQPMaxInter, 0, maxQP);
    prm->nQPI        = clamp(prm->nQPI,        0, maxQP);
    prm->nQPP        = clamp(prm->nQPP,        0, maxQP);
    prm->nQPB        = clamp(prm->nQPB,        0, maxQP);

    return RGY_ERR_NONE;
}

RGY_ERR VCECore::initOutput(VCEParam *inputParams) {
    m_hdrsei = createHEVCHDRSei(inputParams->common.maxCll, inputParams->common.masterDisplay, inputParams->common.atcSei, m_pFileReader.get());
    if (!m_hdrsei) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to parse HEVC HDR10 metadata.\n"));
        return RGY_ERR_INVALID_PARAM;
    }
    const auto outputVideoInfo = videooutputinfo(
        inputParams->codec,
        csp_rgy_to_enc(GetEncoderCSP(inputParams)),
        m_params,
        m_picStruct,
        m_encVUI
    );

    auto err = initWriters(m_pFileWriter, m_pFileWriterListAudio, m_pFileReader, m_AudioReaders,
        &inputParams->common, &inputParams->input, &inputParams->ctrl, outputVideoInfo,
        m_trimParam, m_outputTimebase, m_Chapters, m_hdrsei.get(), m_dovirpu.get(), m_encTimestamp.get(), false, false,
        m_poolPkt.get(), m_poolFrame.get(), m_pStatus, m_pPerfMonitor, m_pLog);
    if (err != RGY_ERR_NONE) {
        PrintMes(RGY_LOG_ERROR, _T("failed to initialize file reader(s).\n"));
        return err;
    }
    if (inputParams->common.timecode) {
        m_timecode = std::make_unique<RGYTimecode>();
        const auto tcfilename = (inputParams->common.timecodeFile.length() > 0) ? inputParams->common.timecodeFile : PathRemoveExtensionS(inputParams->common.outputFilename) + _T(".timecode.txt");
        err = m_timecode->init(tcfilename);
        if (err != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("failed to open timecode file: \"%s\".\n"), tcfilename.c_str());
            return RGY_ERR_FILE_OPEN;
        }
    }
    return RGY_ERR_NONE;
}

#pragma warning(push)
#pragma warning(disable: 4100)
RGY_ERR VCECore::initDecoder(VCEParam *prm) {
#if ENABLE_AVSW_READER
    const auto inputCodec = m_pFileReader->getInputCodec();
    if (inputCodec == RGY_CODEC_UNKNOWN) {
        PrintMes(RGY_LOG_DEBUG, _T("decoder not required.\n"));
        return RGY_ERR_NONE;
    }
    auto codec_uvd_name = codec_rgy_to_dec(inputCodec);
    if (codec_uvd_name == nullptr) {
        PrintMes(RGY_LOG_ERROR, _T("Input codec \"%s\" not supported.\n"), CodecToStr(inputCodec).c_str());
        return RGY_ERR_UNSUPPORTED;
    }
    if (prm->input.csp == RGY_CSP_P010) {
        codec_uvd_name = codec_rgy_to_dec_10bit(inputCodec);
        if (codec_uvd_name == nullptr) {
            PrintMes(RGY_LOG_ERROR, _T("\"%s\" not supported for high bit depth decoding.\n"), CodecToStr(inputCodec).c_str());
            return RGY_ERR_UNSUPPORTED;
        }
    }
    PrintMes(RGY_LOG_DEBUG, _T("decoder: use codec \"%s\".\n"), wstring_to_tstring(codec_uvd_name).c_str());
    auto res = m_pFactory->CreateComponent(m_dev->context(), codec_uvd_name, &m_pDecoder);
    if (res != AMF_OK) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to create decoder context: %s\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("created decoder context.\n"));

    //AMF_VIDEO_DECODER_SURFACE_COPYを使用すると、pre-analysis使用時などに発生するSubmitInput時のAMF_DECODER_NO_FREE_SURFACESを回避できる
    //しかし、メモリ確保エラーが発生することがある(AMF_DIRECTX_FAIL)
    //そこで、AMF_VIDEO_DECODER_SURFACE_COPYは使用せず、QueryOutput後、明示的にsurface->Duplicateを行って同様の挙動を再現する
    //m_pDecoder->SetProperty(AMF_VIDEO_DECODER_SURFACE_COPY, true);

    m_pDecoder->SetProperty(AMF_VIDEO_DECODER_ENABLE_SMART_ACCESS_VIDEO, prm->smartAccessVideo);

    //RGY_CODEC_VC1のときはAMF_TS_SORTを選択する必要がある
    const AMF_TIMESTAMP_MODE_ENUM timestamp_mode = (inputCodec == RGY_CODEC_VC1) ? AMF_TS_SORT : AMF_TS_PRESENTATION;
    if (AMF_OK != (res = m_pDecoder->SetProperty(AMF_TIMESTAMP_MODE, amf_int64(timestamp_mode)))) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to set deocder: %s\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    RGYBitstream header = RGYBitstreamInit();
    auto ret = m_pFileReader->GetHeader(&header);
    if (ret != RGY_ERR_NONE) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to get video header: %s\n"), get_err_mes(ret));
        return ret;
    }
    PrintMes(RGY_LOG_DEBUG, _T("set codec header to decoder: %d bytes.\n"), header.size());

    if (header.size() > 0) {
        amf::AMFBufferPtr buffer;
        m_dev->context()->AllocBuffer(amf::AMF_MEMORY_HOST, header.size(), &buffer);

        memcpy(buffer->GetNative(), header.data(), header.size());
        m_pDecoder->SetProperty(AMF_VIDEO_DECODER_EXTRADATA, amf::AMFVariant(buffer));
    }

    PrintMes(RGY_LOG_DEBUG, _T("initialize %s decoder: %dx%d, %s.\n"),
        CodecToStr(inputCodec).c_str(), prm->input.srcWidth, prm->input.srcHeight,
        wstring_to_tstring(m_pTrace->SurfaceGetFormatName(csp_rgy_to_enc(prm->input.csp))).c_str());
    if (AMF_OK != (res = m_pDecoder->Init(csp_rgy_to_enc(prm->input.csp), prm->input.srcWidth, prm->input.srcHeight))) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to init %s decoder (%s %dx%d): %s\n"), CodecToStr(inputCodec).c_str(),
            wstring_to_tstring(m_pTrace->SurfaceGetFormatName(csp_rgy_to_enc(prm->input.csp))).c_str(), prm->input.srcWidth, prm->input.srcHeight,
            AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("Initialized decoder.\n"));
    return RGY_ERR_NONE;
#else
    return RGY_ERR_NONE;
#endif
}
#pragma warning(pop)

#define ENABLE_VPPAMF 1

RGY_ERR VCECore::createOpenCLCopyFilterForPreVideoMetric(const VCEParam *prm) {
    std::unique_ptr<RGYFilter> filterCrop(new RGYFilterCspCrop(m_dev->cl()));
    std::shared_ptr<RGYFilterParamCrop> param(new RGYFilterParamCrop());
    param->frameOut = RGYFrameInfo(m_encWidth, m_encHeight, GetEncoderCSP(prm), GetEncoderBitdepth(prm), m_picStruct, RGY_MEM_TYPE_GPU_IMAGE);
    param->frameIn = param->frameOut;
    param->frameIn.bitdepth = RGY_CSP_BIT_DEPTH[param->frameIn.csp];
    param->baseFps = m_encFps;
    param->bOutOverwrite = false;
    auto sts = filterCrop->init(param, m_pLog);
    if (sts != RGY_ERR_NONE) {
        return sts;
    }
    //登録
    std::vector<std::unique_ptr<RGYFilter>> filters;
    filters.push_back(std::move(filterCrop));
    if (m_vpFilters.size() > 0) {
        PrintMes(RGY_LOG_ERROR, _T("Unknown error, not expected that m_vpFilters has size.\n"));
        return RGY_ERR_UNDEFINED_BEHAVIOR;
    }
    m_vpFilters.push_back(VppVilterBlock(filters));
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::initFilters(VCEParam *inputParam) {
    //hwデコーダの場合、cropを入力時に行っていない
    const bool cropRequired = cropEnabled(inputParam->input.crop)
        && m_pFileReader->getInputCodec() != RGY_CODEC_UNKNOWN;

    RGYFrameInfo inputFrame;
    inputFrame.width = inputParam->input.srcWidth;
    inputFrame.height = inputParam->input.srcHeight;
    inputFrame.csp = inputParam->input.csp;
    const int croppedWidth = inputFrame.width - inputParam->input.crop.e.left - inputParam->input.crop.e.right;
    const int croppedHeight = inputFrame.height - inputParam->input.crop.e.bottom - inputParam->input.crop.e.up;
    if (!cropRequired) {
        //入力時にcrop済み
        inputFrame.width = croppedWidth;
        inputFrame.height = croppedHeight;
    }
    if (m_pFileReader->getInputCodec() != RGY_CODEC_UNKNOWN) {
        inputFrame.mem_type = RGY_MEM_TYPE_GPU_IMAGE;
    }
    m_encFps = rgy_rational<int>(inputParam->input.fpsN, inputParam->input.fpsD);

    const bool cspConvRequired = inputFrame.csp != GetEncoderCSP(inputParam);

    //リサイザの出力すべきサイズ
    int resizeWidth = croppedWidth;
    int resizeHeight = croppedHeight;
    m_encWidth = resizeWidth;
    m_encHeight = resizeHeight;
    if (inputParam->vpp.pad.enable) {
        m_encWidth += inputParam->vpp.pad.right + inputParam->vpp.pad.left;
        m_encHeight += inputParam->vpp.pad.bottom + inputParam->vpp.pad.top;
    }

    //指定のリサイズがあればそのサイズに設定する
    if (inputParam->input.dstWidth > 0 && inputParam->input.dstHeight > 0) {
        m_encWidth = inputParam->input.dstWidth;
        m_encHeight = inputParam->input.dstHeight;
        resizeWidth = m_encWidth;
        resizeHeight = m_encHeight;
        if (inputParam->vpp.pad.enable) {
            resizeWidth -= (inputParam->vpp.pad.right + inputParam->vpp.pad.left);
            resizeHeight -= (inputParam->vpp.pad.bottom + inputParam->vpp.pad.top);
        }
    }
    RGY_VPP_RESIZE_TYPE resizeRequired = RGY_VPP_RESIZE_TYPE_NONE;
    if (croppedWidth != resizeWidth || croppedHeight != resizeHeight) {
        resizeRequired = getVppResizeType(inputParam->vpp.resize_algo);
        if (resizeRequired == RGY_VPP_RESIZE_TYPE_UNKNOWN) {
            PrintMes(RGY_LOG_ERROR, _T("Unknown resize type.\n"));
            return RGY_ERR_INVALID_VIDEO_PARAM;
        }
    }
    //picStructの設定
    //m_stPicStruct = picstruct_rgy_to_enc(inputParam->input.picstruct);
    //if (inputParam->vpp.deinterlace != cudaVideoDeinterlaceMode_Weave) {
    //    m_stPicStruct = NV_ENC_PIC_STRUCT_FRAME;
    //} else if (inputParam->vpp.afs.enable || inputParam->vpp.nnedi.enable || inputParam->vpp.yadif.enable) {
    //    m_stPicStruct = NV_ENC_PIC_STRUCT_FRAME;
    //}
    //インタレ解除の個数をチェック
    int deinterlacer = 0;
    if (inputParam->vpp.afs.enable) deinterlacer++;
    if (inputParam->vpp.nnedi.enable) deinterlacer++;
    if (inputParam->vpp.yadif.enable) deinterlacer++;
    if (deinterlacer >= 2) {
        PrintMes(RGY_LOG_ERROR, _T("Activating 2 or more deinterlacer is not supported.\n"));
        return RGY_ERR_UNSUPPORTED;
    }

    //VUI情報
    auto VuiFiltered = inputParam->input.vui;

    m_encVUI = inputParam->common.out_vui;
    m_encVUI.apply_auto(inputParam->input.vui, m_encHeight);
    m_encVUI.setDescriptPreset();

    m_vpFilters.clear();

    const auto VCE_AMF_GPU_IMAGE = RGY_MEM_TYPE_GPU_IMAGE;

    //OpenCLが使用できない場合
    if (!m_dev->cl() && cspConvRequired) {
        PrintMes(RGY_LOG_ERROR, _T("Cannot continue as OpenCL is disabled, but csp conversion required!\n"));
        return RGY_ERR_UNSUPPORTED;
    }

    std::vector<VppType> filterPipeline = InitFiltersCreateVppList(inputParam, cspConvRequired, cropRequired, resizeRequired);
    if (filterPipeline.size() == 0) {
        PrintMes(RGY_LOG_DEBUG, _T("No filters required.\n"));
        return RGY_ERR_NONE;
    }
    const auto clfilterCount = std::count_if(filterPipeline.begin(), filterPipeline.end(), [](VppType type) { return getVppFilterType(type) == VppFilterType::FILTER_OPENCL; });
    if (!m_dev->cl() && clfilterCount > 0) {
        PrintMes(RGY_LOG_ERROR, _T("Cannot continue as OpenCL is disabled, but OpenCL filter required!\n"));
        return RGY_ERR_UNSUPPORTED;
    }
    //読み込み時のcrop
    sInputCrop *inputCrop = (cropRequired) ? &inputParam->input.crop : nullptr;
    const auto resize = std::make_pair(resizeWidth, resizeHeight);

    std::vector<std::unique_ptr<RGYFilter>> vppOpenCLFilters;
    for (size_t i = 0; i < filterPipeline.size(); i++) {
        const VppFilterType ftype0 = (i >= 1)                      ? getVppFilterType(filterPipeline[i-1]) : VppFilterType::FILTER_NONE;
        const VppFilterType ftype1 =                                 getVppFilterType(filterPipeline[i+0]);
        const VppFilterType ftype2 = (i+1 < filterPipeline.size()) ? getVppFilterType(filterPipeline[i+1]) : VppFilterType::FILTER_NONE;
        if (ftype1 == VppFilterType::FILTER_AMF) {
#if ENABLE_VPPAMF
            auto [err, vppamf] = AddFilterAMF(inputFrame, filterPipeline[i], inputParam, inputCrop, resize, VuiFiltered);
            inputCrop = nullptr;
            if (err != RGY_ERR_NONE) {
                return err;
            }
            if (vppamf) {
                m_vpFilters.push_back(VppVilterBlock(vppamf));
            }
#endif
        } else if (ftype1 == VppFilterType::FILTER_OPENCL) {
            if (ftype0 != VppFilterType::FILTER_OPENCL || filterPipeline[i] == VppType::CL_CROP) { // 前のfilterがOpenCLでない場合、変換が必要
                auto filterCrop = std::make_unique<RGYFilterCspCrop>(m_dev->cl());
                shared_ptr<RGYFilterParamCrop> param(new RGYFilterParamCrop());
                param->frameIn = inputFrame;
                param->frameOut = inputFrame;
                param->frameOut.csp = GetEncoderCSP(inputParam);
                switch (param->frameOut.csp) { // OpenCLフィルタの内部形式への変換
                case RGY_CSP_NV12: param->frameOut.csp = RGY_CSP_YV12; break;
                case RGY_CSP_P010: param->frameOut.csp = RGY_CSP_YV12_16; break;
                case RGY_CSP_AYUV: param->frameOut.csp = RGY_CSP_YUV444; break;
                case RGY_CSP_Y410: param->frameOut.csp = RGY_CSP_YUV444_16; break;
                case RGY_CSP_Y416: param->frameOut.csp = RGY_CSP_YUV444_16; break;
                default:
                    break;
                }
                param->frameOut.bitdepth = RGY_CSP_BIT_DEPTH[param->frameOut.csp];
                if (inputCrop) {
                    param->crop = *inputCrop;
                    inputCrop = nullptr;
                }
                param->baseFps = m_encFps;
                param->frameIn.mem_type = VCE_AMF_GPU_IMAGE;
                param->frameOut.mem_type = RGY_MEM_TYPE_GPU;
                param->bOutOverwrite = false;
                auto sts = filterCrop->init(param, m_pLog);
                if (sts != RGY_ERR_NONE) {
                    return sts;
                }
                //入力フレーム情報を更新
                inputFrame = param->frameOut;
                m_encFps = param->baseFps;
                vppOpenCLFilters.push_back(std::move(filterCrop));
            }
            if (filterPipeline[i] != VppType::CL_CROP) {
                auto err = AddFilterOpenCL(vppOpenCLFilters, inputFrame, filterPipeline[i], inputParam, inputCrop, resize, VuiFiltered);
                if (err != RGY_ERR_NONE) {
                    return err;
                }
            }
            if (ftype2 != VppFilterType::FILTER_OPENCL) { // 次のfilterがOpenCLでない場合、変換が必要
                std::unique_ptr<RGYFilter> filterCrop(new RGYFilterCspCrop(m_dev->cl()));
                std::shared_ptr<RGYFilterParamCrop> param(new RGYFilterParamCrop());
                param->frameIn = inputFrame;
                param->frameOut = inputFrame;
                param->frameOut.csp = GetEncoderCSP(inputParam);
                param->frameOut.bitdepth = GetEncoderBitdepth(inputParam);
                param->frameIn.mem_type = RGY_MEM_TYPE_GPU;
                param->frameOut.mem_type = VCE_AMF_GPU_IMAGE;
                param->baseFps = m_encFps;
                param->bOutOverwrite = false;
                auto sts = filterCrop->init(param, m_pLog);
                if (sts != RGY_ERR_NONE) {
                    return sts;
                }
                //入力フレーム情報を更新
                inputFrame = param->frameOut;
                m_encFps = param->baseFps;
                //登録
                vppOpenCLFilters.push_back(std::move(filterCrop));
                // ブロックに追加する
                m_vpFilters.push_back(VppVilterBlock(vppOpenCLFilters));
                vppOpenCLFilters.clear();
            }
        } else {
            PrintMes(RGY_LOG_ERROR, _T("Unsupported vpp filter type.\n"));
            return RGY_ERR_UNSUPPORTED;
        }
    }

    if (inputParam->vpp.checkPerformance) {
        for (auto& block : m_vpFilters) {
            if (block.type == VppFilterType::FILTER_OPENCL) {
                for (auto& filter : block.vppcl) {
                    filter->setCheckPerformance(inputParam->vpp.checkPerformance);
                }
            }
        }
    }

    m_encWidth  = inputFrame.width;
    m_encHeight = inputFrame.height;
    m_picStruct = inputFrame.picstruct;
    return RGY_ERR_NONE;
}

std::vector<VppType> VCECore::InitFiltersCreateVppList(const VCEParam *inputParam, const bool cspConvRequired, const bool cropRequired, const RGY_VPP_RESIZE_TYPE resizeRequired) {
    std::vector<VppType> filterPipeline;
    filterPipeline.reserve((size_t)VppType::CL_MAX);

    if (cspConvRequired || cropRequired)   filterPipeline.push_back(VppType::CL_CROP);
    if (inputParam->vpp.colorspace.enable) {
#if 0
        bool requireOpenCL = inputParam->vpp.colorspace.hdr2sdr.tonemap != HDR2SDR_DISABLED || inputParam->vpp.colorspace.lut3d.table_file.length() > 0;
        if (!requireOpenCL) {
            auto currentVUI = inputParam->input.vui;
            for (size_t i = 0; i < inputParam->vpp.colorspace.convs.size(); i++) {
                auto conv_from = inputParam->vpp.colorspace.convs[i].from;
                auto conv_to = inputParam->vpp.colorspace.convs[i].to;
                if (conv_from.chromaloc != conv_to.chromaloc
                    || conv_from.colorprim != conv_to.colorprim
                    || conv_from.transfer != conv_to.transfer) {
                    requireOpenCL = true;
                } else if (conv_from.matrix != conv_to.matrix
                    && (conv_from.matrix != RGY_MATRIX_ST170_M && conv_from.matrix != RGY_MATRIX_BT709)
                    && (conv_to.matrix != RGY_MATRIX_ST170_M && conv_to.matrix != RGY_MATRIX_BT709)) {
                    requireOpenCL = true;
                }
            }
        }
        filterPipeline.push_back((requireOpenCL) ? VppType::CL_COLORSPACE : VppType::AMF_COLORSPACE);
#else
        filterPipeline.push_back(VppType::CL_COLORSPACE);
#endif
    }
    if (inputParam->vpp.delogo.enable)        filterPipeline.push_back(VppType::CL_DELOGO);
    if (inputParam->vpp.afs.enable)           filterPipeline.push_back(VppType::CL_AFS);
    if (inputParam->vpp.nnedi.enable)         filterPipeline.push_back(VppType::CL_NNEDI);
    if (inputParam->vpp.yadif.enable)         filterPipeline.push_back(VppType::CL_YADIF);
    if (inputParam->vpp.decimate.enable)      filterPipeline.push_back(VppType::CL_DECIMATE);
    if (inputParam->vpp.mpdecimate.enable)    filterPipeline.push_back(VppType::CL_MPDECIMATE);
    if (inputParam->vpp.convolution3d.enable) filterPipeline.push_back(VppType::CL_CONVOLUTION3D);
    if (inputParam->vpp.smooth.enable)        filterPipeline.push_back(VppType::CL_DENOISE_SMOOTH);
    if (inputParam->vpp.knn.enable)           filterPipeline.push_back(VppType::CL_DENOISE_KNN);
    if (inputParam->vpp.pmd.enable)           filterPipeline.push_back(VppType::CL_DENOISE_PMD);
    if (inputParam->vppamf.pp.enable)         filterPipeline.push_back(VppType::AMF_PREPROCESS);
    if (inputParam->vpp.subburn.size()>0)     filterPipeline.push_back(VppType::CL_SUBBURN);
    if (     resizeRequired == RGY_VPP_RESIZE_TYPE_OPENCL) filterPipeline.push_back(VppType::CL_RESIZE);
    else if (resizeRequired != RGY_VPP_RESIZE_TYPE_NONE)   filterPipeline.push_back(VppType::AMF_RESIZE);
    if (inputParam->vpp.unsharp.enable)    filterPipeline.push_back(VppType::CL_UNSHARP);
    if (inputParam->vpp.edgelevel.enable)  filterPipeline.push_back(VppType::CL_EDGELEVEL);
    if (inputParam->vpp.warpsharp.enable)  filterPipeline.push_back(VppType::CL_WARPSHARP);
    if (inputParam->vppamf.enhancer.enable)  filterPipeline.push_back(VppType::AMF_VQENHANCE);
    if (inputParam->vpp.transform.enable)  filterPipeline.push_back(VppType::CL_TRANSFORM);
    if (inputParam->vpp.curves.enable)     filterPipeline.push_back(VppType::CL_CURVES);
    if (inputParam->vpp.tweak.enable)      filterPipeline.push_back(VppType::CL_TWEAK);
    if (inputParam->vpp.deband.enable)     filterPipeline.push_back(VppType::CL_DEBAND);
    if (inputParam->vpp.pad.enable)        filterPipeline.push_back(VppType::CL_PAD);
    if (inputParam->vpp.overlay.size() > 0)  filterPipeline.push_back(VppType::CL_OVERLAY);

    if (filterPipeline.size() == 0) {
        return filterPipeline;
    }
    //OpenCLが使用できない場合
    if (!m_dev->cl()) {
        //置き換え
        for (auto& filter : filterPipeline) {
            if (filter == VppType::CL_RESIZE) filter = VppType::AMF_RESIZE;
        }
        //削除
        decltype(filterPipeline) newPipeline;
        for (auto& filter : filterPipeline) {
            if (getVppFilterType(filter) != VppFilterType::FILTER_OPENCL) {
                newPipeline.push_back(filter);
            }
        }
        if (filterPipeline.size() != newPipeline.size()) {
            PrintMes(RGY_LOG_WARN, _T("OpenCL disabled, OpenCL based vpp filters will be disabled!\n"));
        }
        filterPipeline = newPipeline;
    }

    // cropとresizeはmfxとopencl両方ともあるので、前後のフィルタがどちらもOpenCLだったら、そちらに合わせる
    for (size_t i = 0; i < filterPipeline.size(); i++) {
        const VppFilterType prev = (i >= 1)                        ? getVppFilterType(filterPipeline[i - 1]) : VppFilterType::FILTER_NONE;
        const VppFilterType next = (i + 1 < filterPipeline.size()) ? getVppFilterType(filterPipeline[i + 1]) : VppFilterType::FILTER_NONE;
        if (filterPipeline[i] == VppType::AMF_RESIZE) {
            if (resizeRequired == RGY_VPP_RESIZE_TYPE_AUTO // 自動以外の指定があれば、それに従うので、自動の場合のみ変更
                && m_dev->cl()
                && prev == VppFilterType::FILTER_OPENCL
                && next == VppFilterType::FILTER_OPENCL) {
                filterPipeline[i] = VppType::CL_RESIZE; // OpenCLに挟まれていたら、OpenCLのresizeを優先する
            }
        } else if (filterPipeline[i] == VppType::AMF_CONVERTER) {
            if (m_dev->cl()
                && prev == VppFilterType::FILTER_OPENCL
                && next == VppFilterType::FILTER_OPENCL) {
                filterPipeline[i] = VppType::CL_COLORSPACE; // OpenCLに挟まれていたら、OpenCLのcolorspaceを優先する
            }
        }
    }
    return filterPipeline;
}

std::tuple<RGY_ERR, std::unique_ptr<AMFFilter>> VCECore::AddFilterAMF(
    RGYFrameInfo & inputFrame, const VppType vppType, const VCEParam *inputParam, const sInputCrop *crop, const std::pair<int, int> resize, VideoVUIInfo& vuiInfo) {
    std::unique_ptr<AMFFilter> filter;
    switch (vppType) {
    case VppType::AMF_CONVERTER: {
        filter = std::make_unique<AMFFilterConverter>(m_dev->context(), m_pLog);
        auto param = std::make_shared<AMFFilterParamConverter>();
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        m_pLastFilterParam = param;
    } break;
    case VppType::AMF_PREPROCESS: {
        filter = std::make_unique<AMFFilterPreProcessing>(m_dev->context(), m_pLog);
        auto param = std::make_shared<AMFFilterParamPreProcessing>();
        param->pp = inputParam->vppamf.pp;
        param->bitrate = inputParam->nBitrate;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        m_pLastFilterParam = param;
        if (RGY_CSP_BIT_DEPTH[param->frameIn.csp] != 8) {
            PrintMes(RGY_LOG_ERROR, _T("PreProcess currently supports 8bit only.\n"));
            return { RGY_ERR_UNSUPPORTED, nullptr };
        }
        } break;
    case VppType::AMF_RESIZE: {
        filter = std::make_unique<AMFFilterHQScaler>(m_dev->context(), m_pLog);
        auto param = std::make_shared<AMFFilterParamHQScaler>();
        param->scaler = inputParam->vppamf.scaler;
        param->scaler.algorithm = resize_mode_rgy_to_enc(inputParam->vpp.resize_algo);
        if (param->scaler.algorithm < 0) {
            PrintMes(RGY_LOG_ERROR, _T("Unknown resize algorithm %s for HQ Scaler.\n"), get_cx_desc(list_vpp_resize, inputParam->vpp.resize_mode));
            return { RGY_ERR_UNSUPPORTED, nullptr };
        }
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->frameOut.width = resize.first;
        param->frameOut.height = resize.second;
        param->baseFps = m_encFps;
        m_pLastFilterParam = param;
        } break;
    case VppType::AMF_VQENHANCE: {
        filter = std::make_unique<AMFFilterVQEnhancer>(m_dev->context(), m_pLog);
        auto param = std::make_shared<AMFFilterParamVQEnhancer>();
        param->enhancer = inputParam->vppamf.enhancer;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        m_pLastFilterParam = param;
        } break;
    default:
        PrintMes(RGY_LOG_ERROR, _T("Unknown filter type.\n"));
        return { RGY_ERR_UNSUPPORTED, nullptr };
    }
    auto sts = filter->init(m_pFactory, m_pTrace, m_pLastFilterParam);
    if (sts != RGY_ERR_NONE) {
        return { sts, nullptr };
    }
    //入力フレーム情報を更新
    inputFrame = m_pLastFilterParam->frameOut;
    m_encFps = m_pLastFilterParam->baseFps;
    return { RGY_ERR_NONE, std::move(filter) };
}

RGY_ERR VCECore::AddFilterOpenCL(std::vector<std::unique_ptr<RGYFilter>>&clfilters,
        RGYFrameInfo & inputFrame, const VppType vppType, const VCEParam *inputParam, const sInputCrop *crop, const std::pair<int, int> resize, VideoVUIInfo& vuiInfo) {
    //colorspace
    if (vppType == VppType::CL_COLORSPACE) {
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilterColorspace> filter(new RGYFilterColorspace(m_dev->cl()));
        shared_ptr<RGYFilterParamColorspace> param(new RGYFilterParamColorspace());
        param->colorspace = inputParam->vpp.colorspace;
        param->encCsp = inputFrame.csp;
        param->VuiIn = vuiInfo;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        vuiInfo = filter->VuiOut();
        return RGY_ERR_NONE;
    }
    //delogo
    if (vppType == VppType::CL_DELOGO) {
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterDelogo(m_dev->cl()));
        shared_ptr < RGYFilterParamDelogo> param(new RGYFilterParamDelogo());
        param->delogo = inputParam->vpp.delogo;
        param->inputFileName = inputParam->common.inputFilename.c_str();
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = true;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //afs
    if (vppType == VppType::CL_AFS) {
        if ((inputParam->input.picstruct & (RGY_PICSTRUCT_TFF | RGY_PICSTRUCT_BFF)) == 0) {
            PrintMes(RGY_LOG_ERROR, _T("Please set input interlace field order (--interlace tff/bff) for vpp-afs.\n"));
            return RGY_ERR_INVALID_PARAM;
        }
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterAfs(m_dev->cl()));
        shared_ptr<RGYFilterParamAfs> param(new RGYFilterParamAfs());
        param->afs = inputParam->vpp.afs;
        param->afs.tb_order = (inputParam->input.picstruct & RGY_PICSTRUCT_TFF) != 0;
        if (inputParam->common.timecode && param->afs.timecode) {
            param->afs.timecode = 2;
        }
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->inFps = m_inputFps;
        param->inTimebase = m_outputTimebase;
        param->outTimebase = m_outputTimebase;
        param->baseFps = m_encFps;
        param->outFilename = inputParam->common.outputFilename;
        param->bOutOverwrite = false;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //nnedi
    if (vppType == VppType::CL_NNEDI) {
        if ((inputParam->input.picstruct & (RGY_PICSTRUCT_TFF | RGY_PICSTRUCT_BFF)) == 0) {
            PrintMes(RGY_LOG_ERROR, _T("Please set input interlace field order (--interlace tff/bff) for vpp-nnedi.\n"));
            return RGY_ERR_INVALID_PARAM;
        }
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterNnedi(m_dev->cl()));
        shared_ptr<RGYFilterParamNnedi> param(new RGYFilterParamNnedi());
        param->nnedi = inputParam->vpp.nnedi;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //yadif
    if (vppType == VppType::CL_YADIF) {
        if ((inputParam->input.picstruct & (RGY_PICSTRUCT_TFF | RGY_PICSTRUCT_BFF)) == 0) {
            PrintMes(RGY_LOG_ERROR, _T("Please set input interlace field order (--interlace tff/bff) for vpp-yadif.\n"));
            return RGY_ERR_INVALID_PARAM;
        }
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterYadif(m_dev->cl()));
        shared_ptr<RGYFilterParamYadif> param(new RGYFilterParamYadif());
        param->yadif = inputParam->vpp.yadif;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->timebase = m_outputTimebase;
        param->bOutOverwrite = false;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //decimate
    if (vppType == VppType::CL_DECIMATE) {
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterDecimate(m_dev->cl()));
        shared_ptr<RGYFilterParamDecimate> param(new RGYFilterParamDecimate());
        param->decimate = inputParam->vpp.decimate;
        param->useSeparateQueue = false; // やはりuseSeparateQueueはバグっているかもしれない
        param->outfilename = inputParam->common.outputFilename;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //mpdecimate
    if (vppType == VppType::CL_MPDECIMATE) {
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterMpdecimate(m_dev->cl()));
        shared_ptr<RGYFilterParamMpdecimate> param(new RGYFilterParamMpdecimate());
        param->mpdecimate = inputParam->vpp.mpdecimate;
        param->useSeparateQueue = false; // やはりuseSeparateQueueはバグっているかもしれない
        param->outfilename = inputParam->common.outputFilename;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //回転
    if (vppType == VppType::CL_TRANSFORM) {
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterTransform(m_dev->cl()));
        shared_ptr<RGYFilterParamTransform> param(new RGYFilterParamTransform());
        param->trans = inputParam->vpp.transform;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //ノイズ除去 (convolution3d)
    if (vppType == VppType::CL_CONVOLUTION3D) {
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterConvolution3D(m_dev->cl()));
        shared_ptr<RGYFilterParamConvolution3D> param(new RGYFilterParamConvolution3D());
        param->convolution3d = inputParam->vpp.convolution3d;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //smooth
    if (vppType == VppType::CL_DENOISE_SMOOTH) {
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterSmooth(m_dev->cl()));
        shared_ptr<RGYFilterParamSmooth> param(new RGYFilterParamSmooth());
        param->smooth = inputParam->vpp.smooth;
        param->qpTableRef = nullptr;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //knn
    if (vppType == VppType::CL_DENOISE_KNN) {
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterDenoiseKnn(m_dev->cl()));
        shared_ptr<RGYFilterParamDenoiseKnn> param(new RGYFilterParamDenoiseKnn());
        param->knn = inputParam->vpp.knn;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //pmd
    if (vppType == VppType::CL_DENOISE_PMD) {
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterDenoisePmd(m_dev->cl()));
        shared_ptr<RGYFilterParamDenoisePmd> param(new RGYFilterParamDenoisePmd());
        param->pmd = inputParam->vpp.pmd;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //字幕焼きこみ
    if (vppType == VppType::CL_SUBBURN) {
        std::vector<std::unique_ptr<RGYFilter>> filters;
        for (const auto& subburn : inputParam->vpp.subburn) {
            if (!subburn.enable)
#if ENABLE_AVSW_READER
            if (subburn.filename.length() > 0
                && m_trimParam.list.size() > 0) {
                PrintMes(RGY_LOG_ERROR, _T("--vpp-subburn with input as file cannot be used with --trim.\n"));
                return RGY_ERR_UNSUPPORTED;
            }
            amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
            unique_ptr<RGYFilter> filter(new RGYFilterSubburn(m_dev->cl()));
            shared_ptr<RGYFilterParamSubburn> param(new RGYFilterParamSubburn());
            param->subburn = subburn;

            auto pAVCodecReader = std::dynamic_pointer_cast<RGYInputAvcodec>(m_pFileReader);
            if (pAVCodecReader != nullptr) {
                param->videoInputStream = pAVCodecReader->GetInputVideoStream();
                param->videoInputFirstKeyPts = pAVCodecReader->GetVideoFirstKeyPts();
                for (const auto &stream : pAVCodecReader->GetInputStreamInfo()) {
                    if (stream.trackId == trackFullID(AVMEDIA_TYPE_SUBTITLE, param->subburn.trackId)) {
                        param->streamIn = stream;
                        break;
                    }
                }
                param->attachmentStreams = pAVCodecReader->GetInputAttachmentStreams();
            }
            param->videoInfo = m_pFileReader->GetInputFrameInfo();
            if (param->subburn.trackId != 0 && param->streamIn.stream == nullptr) {
                PrintMes(RGY_LOG_WARN, _T("Could not find subtitle track #%d, vpp-subburn for track #%d will be disabled.\n"),
                    param->subburn.trackId, param->subburn.trackId);
            } else {
                param->bOutOverwrite = true;
                param->videoOutTimebase = av_make_q(m_outputTimebase);
                param->frameIn = inputFrame;
                param->frameOut = inputFrame;
                param->baseFps = m_encFps;
                param->poolPkt = m_poolPkt.get();
                param->crop = inputParam->input.crop;
                auto sts = filter->init(param, m_pLog);
                if (sts != RGY_ERR_NONE) {
                    return sts;
                }
                //フィルタチェーンに追加
                clfilters.push_back(std::move(filter));
                //パラメータ情報を更新
                m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
                //入力フレーム情報を更新
                inputFrame = param->frameOut;
                m_encFps = param->baseFps;
            }
#else
            PrintMes(RGY_LOG_ERROR, _T("--vpp-subburn not supported in this build.\n"));
            return RGY_ERR_UNSUPPORTED;
#endif
        }
        return RGY_ERR_NONE;
    }
    //リサイズ
    if (vppType == VppType::CL_RESIZE) {
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filterResize(new RGYFilterResize(m_dev->cl()));
        shared_ptr<RGYFilterParamResize> param(new RGYFilterParamResize());
        param->interp = (inputParam->vpp.resize_algo != RGY_VPP_RESIZE_AUTO) ? inputParam->vpp.resize_algo : RGY_VPP_RESIZE_SPLINE36;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->frameOut.width = resize.first;
        param->frameOut.height = resize.second;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        auto sts = filterResize->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filterResize));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //unsharp
    if (vppType == VppType::CL_UNSHARP) {
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterUnsharp(m_dev->cl()));
        shared_ptr<RGYFilterParamUnsharp> param(new RGYFilterParamUnsharp());
        param->unsharp = inputParam->vpp.unsharp;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //warpsharp
    if (vppType == VppType::CL_WARPSHARP) {
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterWarpsharp(m_dev->cl()));
        shared_ptr<RGYFilterParamWarpsharp> param(new RGYFilterParamWarpsharp());
        param->warpsharp = inputParam->vpp.warpsharp;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //edgelevel
    if (vppType == VppType::CL_EDGELEVEL) {
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterEdgelevel(m_dev->cl()));
        shared_ptr<RGYFilterParamEdgelevel> param(new RGYFilterParamEdgelevel());
        param->edgelevel = inputParam->vpp.edgelevel;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //curves
    if (vppType == VppType::CL_CURVES) {
        unique_ptr<RGYFilter> filter(new RGYFilterCurves(m_dev->cl()));
        shared_ptr<RGYFilterParamCurves> param(new RGYFilterParamCurves());
        param->curves = inputParam->vpp.curves;
        param->vuiInfo = vuiInfo;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = true;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        //登録
        clfilters.push_back(std::move(filter));
        return RGY_ERR_NONE;
    }
    //tweak
    if (vppType == VppType::CL_TWEAK) {
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterTweak(m_dev->cl()));
        shared_ptr<RGYFilterParamTweak> param(new RGYFilterParamTweak());
        param->tweak = inputParam->vpp.tweak;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = true;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //deband
    if (vppType == VppType::CL_DEBAND) {
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterDeband(m_dev->cl()));
        shared_ptr<RGYFilterParamDeband> param(new RGYFilterParamDeband());
        param->deband = inputParam->vpp.deband;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //padding
    if (vppType == VppType::CL_PAD) {
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filter(new RGYFilterPad(m_dev->cl()));
        shared_ptr<RGYFilterParamPad> param(new RGYFilterParamPad());
        param->pad = inputParam->vpp.pad;
        param->frameIn = inputFrame;
        param->frameOut = inputFrame;
        param->frameOut.width = m_encWidth;
        param->frameOut.height = m_encHeight;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        auto sts = filter->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        //フィルタチェーンに追加
        clfilters.push_back(std::move(filter));
        //パラメータ情報を更新
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
        return RGY_ERR_NONE;
    }
    //overlay
    if (vppType == VppType::CL_OVERLAY) {
        for (const auto& overlay : inputParam->vpp.overlay) {
            unique_ptr<RGYFilter> filter(new RGYFilterOverlay(m_dev->cl()));
            shared_ptr<RGYFilterParamOverlay> param(new RGYFilterParamOverlay());
            param->overlay = overlay;
            param->frameIn = inputFrame;
            param->frameOut = inputFrame;
            param->baseFps = m_encFps;
            param->bOutOverwrite = true;
            auto sts = filter->init(param, m_pLog);
            if (sts != RGY_ERR_NONE) {
                return sts;
            }
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
            //登録
            clfilters.push_back(std::move(filter));
        }
        return RGY_ERR_NONE;
    }
    PrintMes(RGY_LOG_ERROR, _T("Unknown filter type.\n"));
    return RGY_ERR_UNSUPPORTED;
}

RGY_ERR VCECore::initEncoder(VCEParam *prm) {
    AMF_RESULT res = AMF_OK;

    m_encWidth  = (m_pLastFilterParam) ? m_pLastFilterParam->frameOut.width  : prm->input.srcWidth  - prm->input.crop.e.left - prm->input.crop.e.right;
    m_encHeight = (m_pLastFilterParam) ? m_pLastFilterParam->frameOut.height : prm->input.srcHeight - prm->input.crop.e.bottom - prm->input.crop.e.up;

    if (m_pLog->getLogLevel(RGY_LOGT_CORE) <= RGY_LOG_DEBUG) {
        TCHAR cpuInfo[256] = { 0 };
        tstring gpu_info = m_dev->getGPUInfo();
        //std::wstring deviceName = (m_deviceDX9.GetDevice() == nullptr) ? m_deviceDX11.GetDisplayDeviceName() : m_deviceDX9.GetDisplayDeviceName();
        //deviceName = str_replace(deviceName, L" (TM)", L"");
        //deviceName = str_replace(deviceName, L" (R)", L"");
        //deviceName = str_replace(deviceName, L" Series", L"");
        //deviceName = str_replace(deviceName, L" Graphics", L"");
        getCPUInfo(cpuInfo, _countof(cpuInfo));
        PrintMes(RGY_LOG_DEBUG, _T("VCEEnc    %s (%s)\n"), VER_STR_FILEVERSION_TCHAR, BUILD_ARCH_STR);
        PrintMes(RGY_LOG_DEBUG, _T("OS        %s (%s)\n"), getOSVersion().c_str(), rgy_is_64bit_os() ? _T("x64") : _T("x86"));
        PrintMes(RGY_LOG_DEBUG, _T("CPU Info  %s\n"), cpuInfo);
        PrintMes(RGY_LOG_DEBUG, _T("GPU Info  %s\n"), gpu_info.c_str());
    }

    m_encCodec = prm->codec;

    const auto encCsp = GetEncoderCSP(prm);
    if (encCsp == RGY_CSP_NA) {
        PrintMes(RGY_LOG_ERROR, _T("Unknown Error in GetEncoderCSP().\n"));
        return RGY_ERR_UNSUPPORTED;
    }
    const amf::AMF_SURFACE_FORMAT formatIn = csp_rgy_to_enc(encCsp);
    if (AMF_OK != (res = m_pFactory->CreateComponent(m_dev->context(), codec_rgy_to_enc(prm->codec), &m_pEncoder))) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to AMFCreateComponent: %s.\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("initialized Encoder component.\n"));

    amf::AMFCapsPtr encoderCaps;
    if (m_pEncoder->GetCaps(&encoderCaps) == AMF_OK) {
        //パラメータチェック
        amf::AMF_ACCELERATION_TYPE accelType = encoderCaps->GetAccelerationType();
        if (accelType != amf::AMF_ACCEL_GPU && accelType != amf::AMF_ACCEL_HARDWARE) {
            PrintMes(RGY_LOG_ERROR, _T("HW Acceleration of %s is not supported on this platform.\n"), CodecToStr(prm->codec).c_str());
            return RGY_ERR_UNSUPPORTED;
        }

        //HEVCについてはmain10が無効だと返す場合があるので、チェックしないようにする
        if (prm->codec != RGY_CODEC_HEVC || !IGNORE_HEVC_PROFILE_CAP) {
            int maxProfile = 0;
            encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_PROFILE(prm->codec), &maxProfile);
            PrintMes(RGY_LOG_DEBUG, _T("Max Profile: %s.\n"), get_cx_desc(get_profile_list(prm->codec), maxProfile));
            if (prm->codecParam[prm->codec].nProfile > maxProfile) {
                PrintMes(RGY_LOG_ERROR, _T("Max supported %s profile on this platform is %s (%s specified).\n"),
                    CodecToStr(prm->codec).c_str(),
                    get_cx_desc(get_profile_list(prm->codec), maxProfile),
                    get_cx_desc(get_profile_list(prm->codec), prm->codecParam[prm->codec].nProfile));
                return RGY_ERR_UNSUPPORTED;
            }
        }

        int maxLevel = 0;
        encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_LEVEL(prm->codec), &maxLevel);
        PrintMes(RGY_LOG_DEBUG, _T("Max Level: %d.\n"), get_cx_desc(get_level_list(prm->codec), maxLevel));
        if (prm->codecParam[prm->codec].nLevel > maxLevel) {
            PrintMes(RGY_LOG_ERROR, _T("Max supported %s Level on this platform is %s (%s specified).\n"),
                CodecToStr(prm->codec).c_str(),
                get_cx_desc(get_level_list(prm->codec), maxLevel),
                get_cx_desc(get_level_list(prm->codec), prm->codecParam[prm->codec].nLevel));
            return RGY_ERR_UNSUPPORTED;
        }

        if (prm->rateControl != get_rc_method(prm->codec)[0].value) {
            int maxBitrate = 0;
            encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_BITRATE(prm->codec), &maxBitrate);
            maxBitrate /= 1000;
            if (prm->nBitrate > maxBitrate) {
                PrintMes(RGY_LOG_WARN, _T("Max supported %s bitrate on this platform is %d (%d specified).\n"),
                    CodecToStr(prm->codec).c_str(),
                    maxBitrate, prm->nBitrate);
                prm->nBitrate = maxBitrate;
                prm->nMaxBitrate = maxBitrate;
            }
            if (prm->nMaxBitrate > maxBitrate) {
                PrintMes(RGY_LOG_WARN, _T("Max supported %s bitrate on this platform is %d (%d specified).\n"),
                    CodecToStr(prm->codec).c_str(),
                    maxBitrate, prm->nMaxBitrate);
                prm->nMaxBitrate = maxBitrate;
            }
        }
        {
            int maxRef = 0, minRef = 0;
            encoderCaps->GetProperty(AMF_PARAM_CAP_MIN_REFERENCE_FRAMES(prm->codec), &minRef);
            encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_REFERENCE_FRAMES(prm->codec), &maxRef);
            if (maxRef > 0 // maxRefが0になって適切に取得できない場合があるので、その場合はチェックしない
                && (prm->nRefFrames < minRef || maxRef < prm->nRefFrames)) {
                PrintMes(RGY_LOG_WARN, _T("%s reference frames should be in range of %d - %d (%d specified).\n"),
                    CodecToStr(prm->codec).c_str(),
                    minRef, maxRef, prm->nRefFrames);
                prm->nRefFrames = clamp(prm->nRefFrames, minRef, maxRef);
            }
        }

        const amf::AMFPropertyInfo* props = nullptr;
        m_pEncoder->GetPropertyInfo(AMF_PARAM_RATE_CONTROL_METHOD(prm->codec), &props);
        bool rateControlSupported = false;
        for (auto penum = props->pEnumDescription; !rateControlSupported && penum->name; penum++) {
            if (penum->value == prm->rateControl) rateControlSupported = true;
        }
        if (!rateControlSupported) {
            PrintMes(RGY_LOG_ERROR, _T("%s mode is not supported on this device.\n"), get_cx_desc(get_rc_method(prm->codec), prm->rateControl));
            return RGY_ERR_UNSUPPORTED;
        }

        if (prm->codec == RGY_CODEC_H264) {
            bool bBPictureSupported = false;
            encoderCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_BFRAMES, &bBPictureSupported);
            if (prm->nBframes > 0 && !bBPictureSupported) {
                PrintMes(RGY_LOG_WARN, _T("Bframes is not supported on this device, disabled.\n"));
                prm->nBframes = 0;
                prm->bBPyramid = 0;
                prm->nDeltaQPBFrame = 0;
                prm->nDeltaQPBFrameRef = 0;
            }
        } else if (prm->codec == RGY_CODEC_HEVC) {
        } else if (prm->codec == RGY_CODEC_AV1) {
        } else {
            PrintMes(RGY_LOG_WARN, _T("Unsupported codec.\n"));
            return RGY_ERR_UNSUPPORTED;
        }

        bool preAnalysisSupported = false;
        encoderCaps->GetProperty(AMF_PARAM_CAP_PRE_ANALYSIS(prm->codec), &preAnalysisSupported);
        const int encBitDepth = GetEncoderBitdepth(prm);
        if (   prm->rateControl == get_codec_hqvbr(prm->codec)
            || prm->rateControl == get_codec_hqcbr(prm->codec)) {
            if (!preAnalysisSupported || encBitDepth > 8) {
                if (prm->rateControl == get_codec_hqvbr(prm->codec)) {
                    prm->rateControl = get_codec_vbr(prm->codec);
                    PrintMes(RGY_LOG_WARN, _T("hqvbr mode switching to vbr mode, as hqvbr mode requires pre-analysis which is not supported on this device!\n"));
                } else if (prm->rateControl == get_codec_hqcbr(prm->codec)) {
                    prm->rateControl = get_codec_cbr(prm->codec);
                    PrintMes(RGY_LOG_WARN, _T("hqcbr mode switching to cbr mode, as hqcbr mode requires pre-analysis which is not supported on this device!\n"));
                }
            } else {
                PrintMes(RGY_LOG_INFO, _T("Pre-analysis will be enabled for %s mode.\n"), get_cx_desc(get_rc_method(prm->codec), prm->rateControl));
                prm->pa.enable = true;
            }
        } else if (prm->rateControl == get_codec_qvbr(prm->codec)) {
            if (!preAnalysisSupported) {
                PrintMes(RGY_LOG_ERROR, _T("qvbr mode not supported, as it requires pre-analysis which is not supported on this device!\n"));
                return RGY_ERR_UNSUPPORTED;
            }
            if (encBitDepth > 8) {
                PrintMes(RGY_LOG_ERROR, _T("qvbr mode not supported, as it requires pre-analysis which is not supported with %d bit depth encoding!\n"), encBitDepth);
                return RGY_ERR_UNSUPPORTED;
            }
            PrintMes(RGY_LOG_INFO, _T("Pre-analysis will be enabled for %s mode.\n"), get_cx_desc(get_rc_method(prm->codec), prm->rateControl));
            prm->pa.enable = true;
        }

        if (prm->pa.enable) {
            if (!preAnalysisSupported) {
                PrintMes(RGY_LOG_WARN, _T("Pre-analysis is not supported on this device, disabled.\n"));
                prm->pa.enable = false;
            }
            if (GetEncoderBitdepth(prm) > 8) {
                PrintMes(RGY_LOG_ERROR, _T("Pre analysis is limited to 8bit encoding, disabled.\n"));
                return RGY_ERR_UNSUPPORTED;
            }
        }

        amf::AMFIOCapsPtr inputCaps;
        if (encoderCaps->GetInputCaps(&inputCaps) == AMF_OK) {
            int minWidth, maxWidth;
            inputCaps->GetWidthRange(&minWidth, &maxWidth);
            if (prm->input.srcWidth < minWidth || maxWidth < prm->input.srcWidth) {
                PrintMes(RGY_LOG_ERROR, _T("Input width should be in range of %d - %d (%d specified).\n"),
                    minWidth, maxWidth, prm->input.srcWidth);
                return RGY_ERR_UNSUPPORTED;
            }

            int minHeight, maxHeight;
            inputCaps->GetHeightRange(&minHeight, &maxHeight);
            if (prm->input.srcHeight < minHeight || maxHeight < prm->input.srcHeight) {
                PrintMes(RGY_LOG_ERROR, _T("Input height should be in range of %d - %d (%d specified).\n"),
                    minHeight, maxHeight, prm->input.srcHeight);
                return RGY_ERR_UNSUPPORTED;
            }

            bool formatSupported = false;
            int numOfFormats = inputCaps->GetNumOfFormats();
            for (int i = 0; i < numOfFormats; i++) {
                amf::AMF_SURFACE_FORMAT format;
                amf_bool native = false;
                if (inputCaps->GetFormatAt(i, &format, &native) == AMF_OK) {
                    formatSupported = true;
                    break;
                }
            }
            if (!formatSupported) {
                PrintMes(RGY_LOG_ERROR, _T("Input format %s not supported on this platform.\n"), m_pTrace->SurfaceGetFormatName(formatIn));
                return RGY_ERR_UNSUPPORTED;
            }
        }

        amf::AMFIOCapsPtr outputCaps;
        if (encoderCaps->GetOutputCaps(&outputCaps) == AMF_OK) {
            int minWidth, maxWidth;
            outputCaps->GetWidthRange(&minWidth, &maxWidth);
            if (m_encWidth < minWidth || maxWidth < m_encWidth) {
                PrintMes(RGY_LOG_ERROR, _T("Output width should be in range of %d - %d (%d specified).\n"),
                    minWidth, maxWidth, m_encWidth);
                return RGY_ERR_UNSUPPORTED;
            }

            int minHeight, maxHeight;
            outputCaps->GetHeightRange(&minHeight, &maxHeight);
            if (m_encHeight < minHeight || maxHeight < m_encHeight) {
                PrintMes(RGY_LOG_ERROR, _T("Output height should be in range of %d - %d (%d specified).\n"),
                    minHeight, maxHeight, m_encHeight);
                return RGY_ERR_UNSUPPORTED;
            }

            bool formatSupported = false;
            int numOfFormats = outputCaps->GetNumOfFormats();
            for (int i = 0; i < numOfFormats; i++) {
                amf::AMF_SURFACE_FORMAT format;
                amf_bool native = false;
                if (outputCaps->GetFormatAt(i, &format, &native) == AMF_OK) {
                    formatSupported = true;
                    break;
                }
            }
            if (!formatSupported) {
                PrintMes(RGY_LOG_ERROR, _T("Output format %s not supported on this platform.\n"), m_pTrace->SurfaceGetFormatName(formatIn));
                return RGY_ERR_UNSUPPORTED;
            }
        }
    }

    if (prm->codec == RGY_CODEC_HEVC && prm->codecParam[prm->codec].nTier == AMF_VIDEO_ENCODER_HEVC_TIER_HIGH) {
        if (prm->codecParam[prm->codec].nLevel != 0
            && !is_avail_high_tier_hevc(prm->codecParam[prm->codec].nLevel)) {
            PrintMes(RGY_LOG_WARN, _T("HEVC Level %s does not support High tier, switching to Main tier.\n"), get_cx_desc(get_level_list(prm->codec), prm->codecParam[prm->codec].nLevel));
            prm->codecParam[prm->codec].nTier = AMF_VIDEO_ENCODER_HEVC_TIER_MAIN;
        }
    }

    m_params.SetParamType(VCE_PARAM_KEY_INPUT,         AMF_PARAM_COMMON, L"Input file name");
    m_params.SetParamType(VCE_PARAM_KEY_INPUT_WIDTH,   AMF_PARAM_COMMON, L"Input Frame width (integer, default = 0)");
    m_params.SetParamType(VCE_PARAM_KEY_INPUT_HEIGHT,  AMF_PARAM_COMMON, L"Input Frame height (integer, default = 0)");
    m_params.SetParamType(VCE_PARAM_KEY_OUTPUT,        AMF_PARAM_COMMON, L"Output file name");
    m_params.SetParamType(VCE_PARAM_KEY_OUTPUT_WIDTH,  AMF_PARAM_COMMON, L"Output Frame width (integer, default = 0)");
    m_params.SetParamType(VCE_PARAM_KEY_OUTPUT_HEIGHT, AMF_PARAM_COMMON, L"Output Frame height (integer, default = 0)");
    m_params.SetParamType(VCE_PARAM_KEY_ENGINE,        AMF_PARAM_COMMON, L"Specifies decoder/encoder engine type (DX9, DX11)");
    m_params.SetParamType(VCE_PARAM_KEY_ADAPTERID,     AMF_PARAM_COMMON, L"Specifies adapter ID (integer, default = 0)");
    m_params.SetParamType(VCE_PARAM_KEY_CAPABILITY,    AMF_PARAM_COMMON, L"Enable/Disable to display the device capabilities (true, false default =  false)");

    if (m_params.SetParamTypeCodec(prm->codec) != RGY_ERR_NONE) {
        PrintMes(RGY_LOG_ERROR, _T("Unknown Codec.\n"));
        return RGY_ERR_UNSUPPORTED;
    }

    m_params.SetParam(VCE_PARAM_KEY_INPUT, tchar_to_wstring(prm->common.inputFilename).c_str());
    m_params.SetParam(VCE_PARAM_KEY_OUTPUT, tchar_to_wstring(prm->common.outputFilename).c_str());
    m_params.SetParam(VCE_PARAM_KEY_ADAPTERID, 0);

    int nGOPLen = prm->nGOPLen;
    if (nGOPLen == 0) {
        nGOPLen = (int)(m_encFps.n() / (double)m_encFps.d() + 0.5) * 10;
    }
    //VCEにはlevelを自動で設定してくれる機能はないようで、"0"などとするとエラー終了してしまう。
    if (prm->codecParam[prm->codec].nLevel == 0 || prm->nMaxBitrate == 0) {
        int level = prm->codecParam[prm->codec].nLevel;
        int max_bitrate_kbps = prm->nMaxBitrate;
        int vbv_bufsize_kbps = prm->nVBVBufferSize;
        if (prm->codec == RGY_CODEC_H264) {
            const int profile = prm->codecParam[prm->codec].nProfile;
            if (level == 0) {
                level = calc_auto_level_h264(m_encWidth, m_encHeight, prm->nRefFrames, false,
                    m_encFps.n(), m_encFps.d(), profile, max_bitrate_kbps, vbv_bufsize_kbps);
                //なんかLevel4.0以上でないと設定に失敗する場合がある
                level = std::max(level, 40);
            }
            get_vbv_value_h264(&max_bitrate_kbps, &vbv_bufsize_kbps, level, profile);
        } else if (prm->codec == RGY_CODEC_HEVC) {
            const bool high_tier = prm->codecParam[prm->codec].nTier == AMF_VIDEO_ENCODER_HEVC_TIER_HIGH;
            if (level == 0) {
                level = calc_auto_level_hevc(m_encWidth, m_encHeight, prm->nRefFrames,
                    m_encFps.n(), m_encFps.d(), high_tier, max_bitrate_kbps);
            }
            max_bitrate_kbps = get_max_bitrate_hevc(level, high_tier);
            vbv_bufsize_kbps = max_bitrate_kbps;
        } else if (prm->codec == RGY_CODEC_AV1) {
            const int profile = prm->codecParam[prm->codec].nProfile;
            if (level == 0) {
                level = calc_auto_level_av1(m_encWidth, m_encHeight, prm->nRefFrames,
                    m_encFps.n(), m_encFps.d(), profile, max_bitrate_kbps, 1, 1);
            }
            max_bitrate_kbps = get_max_bitrate_av1(level, profile);
            vbv_bufsize_kbps = max_bitrate_kbps;
        } else {
            max_bitrate_kbps = VCE_DEFAULT_MAX_BITRATE;
            vbv_bufsize_kbps = VCE_DEFAULT_VBV_BUFSIZE;
        }
        if (prm->codecParam[prm->codec].nLevel == 0) {
            prm->codecParam[prm->codec].nLevel = (int16_t)level;
        }
        if (prm->nMaxBitrate == 0) {
            prm->nMaxBitrate = max_bitrate_kbps;
        }
        if (prm->nVBVBufferSize == 0) {
            prm->nVBVBufferSize = vbv_bufsize_kbps;
        }
    }

    //SAR自動設定
    auto par = std::make_pair(prm->par[0], prm->par[1]);
    if ((!prm->par[0] || !prm->par[1]) //SAR比の指定がない
        && prm->input.sar[0] && prm->input.sar[1] //入力側からSAR比を取得ずみ
        && (m_encWidth == prm->input.srcWidth && m_encHeight == prm->input.srcHeight)) {//リサイズは行われない
        par = std::make_pair(prm->input.sar[0], prm->input.sar[1]);
    }
    adjust_sar(&par.first, &par.second, m_encWidth, m_encHeight);
    m_sar = rgy_rational<int>(par.first, par.second);

    m_encVUI.descriptpresent =
           get_cx_value(list_colormatrix, _T("undef")) != (int)m_encVUI.matrix
        || get_cx_value(list_colorprim, _T("undef")) != (int)m_encVUI.colorprim
        || get_cx_value(list_transfer, _T("undef")) != (int)m_encVUI.transfer;

    m_params.SetParam(VCE_PARAM_KEY_INPUT_WIDTH, prm->input.srcWidth);
    m_params.SetParam(VCE_PARAM_KEY_INPUT_HEIGHT, prm->input.srcHeight);
    m_params.SetParam(VCE_PARAM_KEY_OUTPUT_WIDTH, m_encWidth);
    m_params.SetParam(VCE_PARAM_KEY_OUTPUT_HEIGHT, m_encHeight);
    m_params.SetParam(VCE_PARAM_KEY_CAPABILITY,    false);
    m_params.SetParam(SETFRAMEPARAMFREQ_PARAM_NAME,   0);
    m_params.SetParam(SETDYNAMICPARAMFREQ_PARAM_NAME, 0);

    m_params.SetParam(AMF_PARAM_FRAMESIZE(prm->codec),      AMFConstructSize(m_encWidth, m_encHeight));
    m_params.SetParam(AMF_PARAM_FRAMERATE(prm->codec),      AMFConstructRate(m_encFps.n(), m_encFps.d()));
    m_params.SetParam(AMF_PARAM_USAGE(prm->codec),          (amf_int64)get_encoder_usage(prm->codec));
    m_params.SetParam(AMF_PARAM_PROFILE(prm->codec),        (amf_int64)prm->codecParam[prm->codec].nProfile);
    if (prm->codecParam[prm->codec].nLevel != get_level_auto(prm->codec)) {
        m_params.SetParam(AMF_PARAM_PROFILE_LEVEL(prm->codec), (amf_int64)prm->codecParam[prm->codec].nLevel);
    }
    m_params.SetParam(AMF_PARAM_QUALITY_PRESET(prm->codec), (amf_int64)prm->qualityPreset);
    if (prm->rateControl == get_codec_qvbr(m_encCodec)) {
        m_params.SetParam(AMF_PARAM_QVBR_QUALITY_LEVEL(prm->codec), (amf_int64)prm->qvbrLevel);
    }
    m_params.SetParam(AMF_PARAM_QP_I(prm->codec), (amf_int64)prm->nQPI);
    m_params.SetParam(AMF_PARAM_QP_P(prm->codec), (amf_int64)prm->nQPP);
    if (prm->nBitrate != 0)       m_params.SetParam(AMF_PARAM_TARGET_BITRATE(prm->codec), (amf_int64)prm->nBitrate * 1000);
    if (prm->nMaxBitrate != 0)    m_params.SetParam(AMF_PARAM_PEAK_BITRATE(prm->codec),   (amf_int64)prm->nMaxBitrate * 1000);
    if (prm->nVBVBufferSize != 0) m_params.SetParam(AMF_PARAM_VBV_BUFFER_SIZE(prm->codec), (amf_int64)prm->nVBVBufferSize * 1000);
    m_params.SetParam(AMF_PARAM_INITIAL_VBV_BUFFER_FULLNESS(prm->codec),     (amf_int64)prm->nInitialVBVPercent);
    m_params.SetParam(AMF_PARAM_MAX_NUM_REFRAMES(prm->codec), (amf_int64)prm->nRefFrames);
    m_params.SetParam(AMF_PARAM_MAX_LTR_FRAMES(prm->codec), (amf_int64)prm->nLTRFrames);
    if (prm->enableSkipFrame.has_value()) {
        m_params.SetParam(AMF_PARAM_RATE_CONTROL_SKIP_FRAME_ENABLE(prm->codec), prm->enableSkipFrame.value());
    }
    m_params.SetParam(AMF_PARAM_RATE_CONTROL_METHOD(prm->codec),             (amf_int64)prm->rateControl);

    m_params.SetParam(AMF_PARAM_ENFORCE_HRD(prm->codec),        prm->bEnforceHRD != 0);
    m_params.SetParam(AMF_PARAM_FILLER_DATA_ENABLE(prm->codec), prm->bFiller != 0);
    m_params.SetParam(AMF_PARAM_GOP_SIZE(prm->codec),                       (amf_int64)nGOPLen);

    if (prm->pa.enable) {
        if (   prm->rateControl != get_codec_vbr(prm->codec)
            && prm->rateControl != get_codec_hqvbr(prm->codec)
            && prm->rateControl != get_codec_vbr_lat(prm->codec)
            && prm->rateControl != get_codec_cbr(prm->codec)
            && prm->rateControl != get_codec_hqcbr(prm->codec)
            && prm->rateControl != get_codec_qvbr(prm->codec)) {
                PrintMes(RGY_LOG_WARN, _T("Pre analysis is currently supported only with CBR/VBR/QVBR mode.\n"));
                PrintMes(RGY_LOG_WARN, _T("Currenlty %s mode is selected, so pre analysis will be disabled.\n"), get_cx_desc(get_rc_method(prm->codec), prm->rateControl));
                prm->pa.enable = false;
        }
    }
    if (prm->pa.enable) {
        m_params.SetParam(AMF_PARAM_PRE_ANALYSIS_ENABLE(prm->codec), prm->pa.enable);
        m_params.SetParam(AMF_PA_SCENE_CHANGE_DETECTION_ENABLE, prm->pa.sc);
        if (prm->pa.sc) m_params.SetParam(AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY, (amf_int64)prm->pa.scSensitivity);
        m_params.SetParam(AMF_PA_STATIC_SCENE_DETECTION_ENABLE, prm->pa.ss);
        if (prm->pa.ss) m_params.SetParam(AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY, (amf_int64)prm->pa.ssSensitivity);
        m_params.SetParam(AMF_PA_ACTIVITY_TYPE, (amf_int64)prm->pa.activityType);
        if (prm->pa.initQPSC > AMF_PA_INITQPSC_AUTO) m_params.SetParam(AMF_PA_INITIAL_QP_AFTER_SCENE_CHANGE, (amf_int64)prm->pa.initQPSC); //設定しなければ自動
        m_params.SetParam(AMF_PA_MAX_QP_BEFORE_FORCE_SKIP, (amf_int64)prm->pa.maxQPBeforeForceSkip);
        m_params.SetParam(AMF_PA_LTR_ENABLE, prm->pa.ltrEnable);
        m_params.SetParam(AMF_PA_CAQ_STRENGTH, (amf_int64)prm->pa.CAQStrength);
        m_params.SetParam(AMF_PA_PAQ_MODE, (amf_int64)prm->pa.PAQMode);
        m_params.SetParam(AMF_PA_TAQ_MODE, (amf_int64)prm->pa.TAQMode);
        m_params.SetParam(AMF_PA_HIGH_MOTION_QUALITY_BOOST_MODE, (amf_int64)prm->pa.motionQualityBoost);
        m_params.SetParam(AMF_PA_LOOKAHEAD_BUFFER_DEPTH, (amf_uint64)prm->pa.lookaheadDepth);
        if (prm->pe) {
            PrintMes(RGY_LOG_WARN, _T("Pre analysis cannot be used with pre encode, pre encode will be disabled.\n"));
            prm->pe = false;
        }
        if (prm->bVBAQ) {
            PrintMes(RGY_LOG_WARN, _T("Pre analysis cannot be used with VBAQ, VBAQ will be disabled.\n"));
            prm->bVBAQ = false;
        }
    }
    m_params.SetParam(AMF_PARAM_PREENCODE_ENABLE(prm->codec), prm->pe);

    //m_params.SetParam(AMF_PARAM_INPUT_COLOR_PROFILE(prm->codec),           AMF_VIDEO_CONVERTER_COLOR_PROFILE_UNKNOWN);
    //m_params.SetParam(AMF_PARAM_INPUT_TRANSFER_CHARACTERISTIC(prm->codec), AMF_COLOR_TRANSFER_CHARACTERISTIC_UNDEFINED);
    //m_params.SetParam(AMF_PARAM_INPUT_COLOR_PRIMARIES(prm->codec),         AMF_COLOR_PRIMARIES_UNDEFINED);

    //m_params.SetParam(AMF_PARAM_OUTPUT_COLOR_PROFILE(prm->codec),           AMF_VIDEO_CONVERTER_COLOR_PROFILE_UNKNOWN);
    //m_params.SetParam(AMF_PARAM_OUTPUT_TRANSFER_CHARACTERISTIC(prm->codec), AMF_COLOR_TRANSFER_CHARACTERISTIC_UNDEFINED);
    //m_params.SetParam(AMF_PARAM_OUTPUT_COLOR_PRIMARIES(prm->codec),         AMF_COLOR_PRIMARIES_UNDEFINED);

    m_params.SetParam(AMF_PARAM_ENABLE_SMART_ACCESS_VIDEO(prm->codec), prm->smartAccessVideo);

    if (prm->codec == RGY_CODEC_H264 || prm->codec == RGY_CODEC_HEVC) {
        if (m_sar.is_valid()) {
            m_params.SetParam(AMF_PARAM_ASPECT_RATIO(prm->codec), AMFConstructRatio(m_sar.n(), m_sar.d()));
        }
        if (prm->nSlices > 0) {
            m_params.SetParam(AMF_PARAM_SLICES_PER_FRAME(prm->codec), (amf_int64)prm->nSlices);
        }
        m_params.SetParam(AMF_PARAM_LOWLATENCY_MODE(prm->codec), prm->ctrl.lowLatency);
        if (prm->bVBAQ) m_params.SetParam(AMF_PARAM_ENABLE_VBAQ(prm->codec), true);
        //m_params.SetParam(AMF_PARAM_END_OF_SEQUENCE(prm->codec),                false);
        m_params.SetParam(AMF_PARAM_INSERT_AUD(prm->codec), false);
    }

    if (prm->codec == RGY_CODEC_HEVC || prm->codec == RGY_CODEC_AV1) {
        m_params.SetParam(AMF_PARAM_COLOR_BIT_DEPTH(prm->codec), (amf_int64)prm->outputDepth);
    }

    if (prm->codec == RGY_CODEC_H264) {
        //m_params.SetParam(AMF_PARAM_RATE_CONTROL_PREANALYSIS_ENABLE(prm->codec), (prm->preAnalysis) ? AMF_VIDEO_ENCODER_PREENCODE_ENABLED : AMF_VIDEO_ENCODER_PREENCODE_DISABLED);

        m_params.SetParam(AMF_VIDEO_ENCODER_SCANTYPE,           (amf_int64)((m_picStruct & RGY_PICSTRUCT_INTERLACED) ? AMF_VIDEO_ENCODER_SCANTYPE_INTERLACED : AMF_VIDEO_ENCODER_SCANTYPE_PROGRESSIVE));

        m_params.SetParam(AMF_VIDEO_ENCODER_B_PIC_PATTERN, (amf_int64)prm->nBframes);
        m_params.SetParam(AMF_VIDEO_ENCODER_MAX_CONSECUTIVE_BPICTURES, (amf_int64)prm->nBframes);
        m_params.SetParam(AMF_VIDEO_ENCODER_ADAPTIVE_MINIGOP, prm->adaptMiniGOP);
        if (prm->nBframes > 0) {
            m_params.SetParam(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP, (amf_int64)prm->nDeltaQPBFrame);
            m_params.SetParam(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, (amf_int64)prm->nDeltaQPBFrameRef);
            m_params.SetParam(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, prm->nBframes > 0 && !!prm->bBPyramid);
            m_params.SetParam(AMF_VIDEO_ENCODER_QP_B, (amf_int64)prm->nQPB);
        }

        if (prm->nQPMin.has_value()) {
            m_params.SetParam(AMF_VIDEO_ENCODER_MIN_QP,                                (amf_int64)prm->nQPMin.value());
        }
        if (prm->nQPMax.has_value()) {
            m_params.SetParam(AMF_VIDEO_ENCODER_MAX_QP,                                (amf_int64)prm->nQPMax.value());
        }

        //m_params.SetParam(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING,       (amf_int64)0);
        ////m_params.SetParam(AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT, false);

        m_params.SetParam(AMF_PARAM_MOTION_HALF_PIXEL(prm->codec),              !!(prm->nMotionEst & VCE_MOTION_EST_HALF));
        m_params.SetParam(AMF_PARAM_MOTION_QUARTERPIXEL(prm->codec),            !!(prm->nMotionEst & VCE_MOTION_EST_QUATER));

        //m_params.SetParam(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE,             (amf_int64)AMF_VIDEO_ENCODER_PICTURE_TYPE_NONE);
        m_params.SetParam(AMF_VIDEO_ENCODER_INSERT_SPS, false);
        m_params.SetParam(AMF_VIDEO_ENCODER_INSERT_PPS, false);
        //m_params.SetParam(AMF_VIDEO_ENCODER_PICTURE_STRUCTURE,                (amf_int64)((m_picStruct & RGY_PICSTRUCT_INTERLACED) ? AMF_VIDEO_ENCODER_SCANTYPE_INTERLACED : AMF_VIDEO_ENCODER_SCANTYPE_PROGRESSIVE);
        //m_params.SetParam(AMF_VIDEO_ENCODER_MARK_CURRENT_WITH_LTR_INDEX,    false);
        //m_params.SetParam(AMF_VIDEO_ENCODER_FORCE_LTR_REFERENCE_BITFIELD,   (amf_int64)0);

        //m_params.SetParam(AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE, AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM);
        //m_params.SetParam(AMF_VIDEO_ENCODER_OUTPUT_MARKED_LTR_INDEX, (amf_int64)-1);
        //m_params.SetParam(AMF_VIDEO_ENCODER_OUTPUT_REFERENCED_LTR_INDEX_BITFIELD, (amf_int64)0);
        if (m_encVUI.colorrange == RGY_COLORRANGE_FULL) {
            m_params.SetParam(AMF_VIDEO_ENCODER_FULL_RANGE_COLOR, true);
        }
    } else if (prm->codec == RGY_CODEC_HEVC) {
        //m_params.SetParam(AMF_PARAM_RATE_CONTROL_PREANALYSIS_ENABLE(prm->codec), prm->preAnalysis);

        m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_TIER,                            (amf_int64)prm->codecParam[prm->codec].nTier);
        //m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_NOMINAL_RANGE,                   (amf_int64)(m_encVUI.colorrange == RGY_COLORRANGE_FULL ? AMF_VIDEO_ENCODER_HEVC_NOMINAL_RANGE_FULL : AMF_VIDEO_ENCODER_HEVC_NOMINAL_RANGE_STUDIO));

        if (prm->nQPMin.has_value()) {
            m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_MIN_QP_I,                        (amf_int64)prm->nQPMin.value());
        }
        if (prm->nQPMax.has_value()) {
            m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_MAX_QP_I,                        (amf_int64)prm->nQPMax.value());
        }
        if (prm->nQPMinInter.has_value()) {
            m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_MIN_QP_P,                        (amf_int64)prm->nQPMinInter.value());
        }
        if (prm->nQPMaxInter.has_value()) {
            m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_MAX_QP_P,                        (amf_int64)prm->nQPMaxInter.value());
        }

        m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_DE_BLOCKING_FILTER_DISABLE,      !prm->bDeblockFilter);

        m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_INSERT_HEADER,                   true);
    } else if (prm->codec == RGY_CODEC_AV1) {
        if (prm->tiles > 0) {
            m_params.SetParam(AMF_VIDEO_ENCODER_AV1_TILES_PER_FRAME, (amf_int64)prm->tiles);
        }
        if (prm->temporalLayers.has_value()) {
            m_params.SetParam(AMF_VIDEO_ENCODER_AV1_MAX_NUM_TEMPORAL_LAYERS, (amf_int64)prm->temporalLayers.value());
            m_params.SetParam(AMF_VIDEO_ENCODER_AV1_NUM_TEMPORAL_LAYERS,     (amf_int64)prm->temporalLayers.value());
        }
        if (prm->cdefMode.has_value()) {
            m_params.SetParam(AMF_VIDEO_ENCODER_AV1_CDEF_MODE,           (amf_int64)prm->cdefMode.value());
        }
        if (prm->cdfUpdate.has_value()) {
            m_params.SetParam(AMF_VIDEO_ENCODER_AV1_CDF_UPDATE,          prm->cdfUpdate.value());
        }
        if (prm->cdfFrameEndUpdate.has_value()) {
            m_params.SetParam(AMF_VIDEO_ENCODER_AV1_CDF_FRAME_END_UPDATE_MODE, prm->cdfFrameEndUpdate.value() ? AMF_VIDEO_ENCODER_AV1_CDF_FRAME_END_UPDATE_MODE_ENABLE_DEFAULT : AMF_VIDEO_ENCODER_AV1_CDF_FRAME_END_UPDATE_MODE_DISABLE);
        }
        //これをいれないと、1920x1080などの解像度が正常に扱えない
        m_params.SetParam(AMF_VIDEO_ENCODER_AV1_ALIGNMENT_MODE, (amf_int64)AMF_VIDEO_ENCODER_AV1_ALIGNMENT_MODE_NO_RESTRICTIONS);
        if (prm->aqMode.has_value()) {
            m_params.SetParam(AMF_VIDEO_ENCODER_AV1_AQ_MODE,             (amf_int64)prm->aqMode.value());
        }
        if (prm->screenContentTools.has_value()) {
            if (prm->screenContentTools.value()) {
                m_params.SetParam(AMF_VIDEO_ENCODER_AV1_SCREEN_CONTENT_TOOLS, true);
                if (prm->paletteMode.has_value()) {
                    m_params.SetParam(AMF_VIDEO_ENCODER_AV1_PALETTE_MODE, prm->paletteMode.value());
                }
                if (prm->forceIntegerMV.has_value()) {
                    m_params.SetParam(AMF_VIDEO_ENCODER_AV1_FORCE_INTEGER_MV, prm->forceIntegerMV.value());
                }
            } else {
                m_params.SetParam(AMF_VIDEO_ENCODER_AV1_SCREEN_CONTENT_TOOLS, false);
            }
        }
        if (prm->nQPMin.has_value()) {
            m_params.SetParam(AMF_VIDEO_ENCODER_AV1_MIN_Q_INDEX_INTRA, (amf_int64)prm->nQPMin.value());
        }
        if (prm->nQPMax.has_value()) {
            m_params.SetParam(AMF_VIDEO_ENCODER_AV1_MAX_Q_INDEX_INTRA, (amf_int64)prm->nQPMax.value());
        }
        if (prm->nQPMinInter.has_value()) {
            m_params.SetParam(AMF_VIDEO_ENCODER_AV1_MIN_Q_INDEX_INTER, (amf_int64)prm->nQPMinInter.value());
        }
        if (prm->nQPMaxInter.has_value()) {
            m_params.SetParam(AMF_VIDEO_ENCODER_AV1_MAX_Q_INDEX_INTER, (amf_int64)prm->nQPMaxInter.value());
        }
    } else {
        PrintMes(RGY_LOG_ERROR, _T("Unsupported codec.\n"));
        return RGY_ERR_UNSUPPORTED;
    }
    // AMF_PARAM_ENCODER_USAGE は必ず設定する必要がある
    if (m_params.CountParam(AMF_PARAM_ENCODER_USAGE) == 0) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to find AMF_PARAM_ENCODER_USAGE.\n"));
        return RGY_ERR_UNSUPPORTED;
    }
    // Usage is preset that will set many parameters
    m_params.Apply(m_pEncoder, AMF_PARAM_ENCODER_USAGE, m_pLog.get());
    PrintMes(RGY_LOG_DEBUG, _T("pushed usage params.\n"));
    // override some usage parameters
    m_params.Apply(m_pEncoder, AMF_PARAM_STATIC, m_pLog.get());
    PrintMes(RGY_LOG_DEBUG, _T("pushed static params.\n"));

    m_params.Apply(m_pEncoder, AMF_PARAM_DYNAMIC, m_pLog.get());
    PrintMes(RGY_LOG_DEBUG, _T("pushed dynamic params.\n"));

    if (AMF_OK != (res = m_pEncoder->Init(formatIn, m_encWidth, m_encHeight))) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to initalize encoder: %s.\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("initalized encoder.\n"));
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::checkGPUListByEncoder(std::vector<std::unique_ptr<VCEDevice>> &gpuList, const VCEParam *prm, int deviceId) {
    const int encBitdepth = GetEncoderBitdepth(prm);
    const auto encCsp = GetEncoderCSP(prm);
    if (encCsp == RGY_CSP_NA) {
        PrintMes(RGY_LOG_ERROR, _T("Unknown Error in GetEncoderCSP().\n"));
        return RGY_ERR_UNSUPPORTED;
    }
    if (deviceId >= 0) {
        for (auto gpu = gpuList.begin(); gpu != gpuList.end(); ) {
            if ((*gpu)->id() != deviceId) {
                gpu = gpuList.erase(gpu);
                continue;
            }
            gpu++;
        }
        if (gpuList.size() == 0) {
            PrintMes(RGY_LOG_ERROR, _T("Selected device #%d not found\n"), deviceId);
            return RGY_ERR_NOT_FOUND;
        }
    }
    if (prm->ctrl.skipHWEncodeCheck) {
        return RGY_ERR_NONE;
    }
    const auto formatIn = csp_rgy_to_enc(encCsp);
    const auto formatOut = formatIn;
    //エンコーダの対応をチェック
    tstring message; //GPUチェックのメッセージ
    for (auto gpu = gpuList.begin(); gpu != gpuList.end(); ) {
        PrintMes(RGY_LOG_DEBUG, _T("Checking GPU #%d (%s) for codec %s.\n"),
            (*gpu)->id(), (*gpu)->name().c_str(), CodecToStr(prm->codec).c_str());
        //コーデックのチェック
        amf::AMFCapsPtr encoderCaps = (*gpu)->getEncCaps(prm->codec);
        if (encoderCaps == nullptr) {
            message += strsprintf(_T("GPU #%d (%s) does not support %s encoding.\n"),
                (*gpu)->id(), (*gpu)->name().c_str(), CodecToStr(prm->codec).c_str());
            gpu = gpuList.erase(gpu);
            continue;
        }

        amf::AMF_ACCELERATION_TYPE accelType = encoderCaps->GetAccelerationType();
        PrintMes(RGY_LOG_DEBUG, _T("  acceleration: %s.\n"), AccelTypeToString(accelType).c_str());
        if (accelType != amf::AMF_ACCEL_GPU && accelType != amf::AMF_ACCEL_HARDWARE) {
            message += strsprintf(_T("GPU #%d (%s) does not HW Acceleration of %s encoding.\n"),
                (*gpu)->id(), (*gpu)->name().c_str(), CodecToStr(prm->codec).c_str());
            gpu = gpuList.erase(gpu);
            continue;
        }

        //HEVCについてはmain10が無効だと返す場合があるので、チェックしないようにする
        if (prm->codec != RGY_CODEC_HEVC || !IGNORE_HEVC_PROFILE_CAP) {
            int maxProfile = 0;
            encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_PROFILE(prm->codec), &maxProfile);
            PrintMes(RGY_LOG_DEBUG, _T("  Max Profile: %s.\n"), get_cx_desc(get_profile_list(prm->codec), maxProfile));
            if (prm->codecParam[prm->codec].nProfile > maxProfile) {
                message += strsprintf(_T("GPU #%d (%s) does not support %s %s profile (max supported: %s).\n"),
                                      (*gpu)->id(), (*gpu)->name().c_str(), CodecToStr(prm->codec).c_str(),
                                      get_cx_desc(get_profile_list(prm->codec), prm->codecParam[prm->codec].nProfile),
                                      get_cx_desc(get_profile_list(prm->codec), maxProfile));
                gpu = gpuList.erase(gpu);
                continue;
            }
        }

        int maxLevel = 0;
        encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_LEVEL(prm->codec), &maxLevel);
        PrintMes(RGY_LOG_DEBUG, _T("  Max Level: %s.\n"), get_cx_desc(get_level_list(prm->codec), maxLevel));
        if (prm->codecParam[prm->codec].nLevel > maxLevel) {
            message += strsprintf(_T("GPU #%d (%s) does not support %s Level %s (max supported: %s).\n"),
                (*gpu)->id(), (*gpu)->name().c_str(), CodecToStr(prm->codec).c_str(),
                get_cx_desc(get_level_list(prm->codec), prm->codecParam[prm->codec].nLevel),
                get_cx_desc(get_level_list(prm->codec), maxLevel));
            gpu = gpuList.erase(gpu);
            continue;
        }

        if (false) { // 入力フォーマットについてはチェックしないようにする
            amf::AMFIOCapsPtr inputCaps;
            if (encoderCaps->GetInputCaps(&inputCaps) == AMF_OK) {
                bool formatSupported = false;
                int numOfFormats = inputCaps->GetNumOfFormats();
                for (int i = 0; i < numOfFormats; i++) {
                    amf::AMF_SURFACE_FORMAT format;
                    amf_bool native = false;
                    if (inputCaps->GetFormatAt(i, &format, &native) == AMF_OK && format == formatIn) {
                        formatSupported = true;
                        break;
                    }
                }
                if (!formatSupported) {
                    message += strsprintf(_T("GPU #%d (%s) [%s] does not support input format %s.\n"),
                        (*gpu)->id(), (*gpu)->name().c_str(), CodecToStr(prm->codec).c_str(),
                        m_pTrace->SurfaceGetFormatName(formatIn));
                    gpu = gpuList.erase(gpu);
                    continue;
                }
            }
        }

        amf::AMFIOCapsPtr outputCaps;
        if (encoderCaps->GetOutputCaps(&outputCaps) == AMF_OK) {
            if (false) {
                int minWidth, maxWidth;
                outputCaps->GetWidthRange(&minWidth, &maxWidth);
                if (m_encWidth < minWidth || maxWidth < m_encWidth) {
                    message += strsprintf(_T("GPU #%d (%s) [%s] does not support width %d (supported %d - %d).\n"),
                        (*gpu)->id(), (*gpu)->name().c_str(), CodecToStr(prm->codec).c_str(),
                        m_encWidth, minWidth, maxWidth);
                    gpu = gpuList.erase(gpu);
                    continue;
                }

                int minHeight, maxHeight;
                outputCaps->GetHeightRange(&minHeight, &maxHeight);
                if (m_encHeight < minHeight || maxHeight < m_encHeight) {
                    message += strsprintf(_T("GPU #%d (%s) [%s] does not support height %d (supported %d - %d).\n"),
                        (*gpu)->id(), (*gpu)->name().c_str(), CodecToStr(prm->codec).c_str(),
                        m_encHeight, minHeight, maxHeight);
                    gpu = gpuList.erase(gpu);
                    continue;
                }
            }
            if (false) { // 出力フォーマットについてはチェックしないようにする
                bool formatSupported = false;
                int numOfFormats = outputCaps->GetNumOfFormats();
                for (int i = 0; i < numOfFormats; i++) {
                    amf::AMF_SURFACE_FORMAT format;
                    amf_bool native = false;
                    if (outputCaps->GetFormatAt(i, &format, &native) == AMF_OK && format == formatOut) {
                        formatSupported = true;
                        break;
                    }
                }
                if (!formatSupported) {
                    message += strsprintf(_T("GPU #%d (%s) [%s] does not support ouput format %s.\n"),
                        (*gpu)->id(), (*gpu)->name().c_str(), CodecToStr(prm->codec).c_str(),
                        m_pTrace->SurfaceGetFormatName(formatOut));
                    gpu = gpuList.erase(gpu);
                    continue;
                }
            }
            //インタレ保持のチェック
            const bool interlacedEncoding =
                (prm->input.picstruct & RGY_PICSTRUCT_INTERLACED)
                && !prm->vpp.afs.enable
                && !prm->vpp.nnedi.enable
                && !prm->vpp.yadif.enable;
            if (interlacedEncoding
                && !outputCaps->IsInterlacedSupported()) {
                message += strsprintf(_T("GPU #%d (%s) does not support %s interlaced encoding.\n"),
                    (*gpu)->id(), (*gpu)->name().c_str(), CodecToStr(prm->codec).c_str());
                gpu = gpuList.erase(gpu);
                continue;
            }
        }
        if (encBitdepth > 8) {
            if (encBitdepth == 10) {
                bool Support10bitDepth = false;
                if (encoderCaps->GetProperty(VCEDevice::CAP_10BITDEPTH, &Support10bitDepth) != AMF_OK) {
                    Support10bitDepth = false;
                }
                if (!Support10bitDepth) {
                    message += strsprintf(_T("GPU #%d (%s) does not support 10bit depth %s decoding.\n"), (*gpu)->id(), (*gpu)->name().c_str(), CodecToStr(prm->codec).c_str());
                    gpu = gpuList.erase(gpu);
                    continue;
                }
            } else {
                PrintMes(RGY_LOG_ERROR, _T("Unsupported output bit depth: %d.\n"), encBitdepth);
                return RGY_ERR_UNSUPPORTED;
            }
        }

        { //レート制御のチェック
            amf::AMFComponentPtr encoder;
            if (m_pFactory->CreateComponent((*gpu)->context(), codec_rgy_to_enc(prm->codec), &encoder) == AMF_OK) {
                const amf::AMFPropertyInfo* props = nullptr;
                encoder->GetPropertyInfo(AMF_PARAM_RATE_CONTROL_METHOD(prm->codec), &props);
                bool rateControlSupported = false;
                for (auto penum = props->pEnumDescription; !rateControlSupported && penum->name; penum++) {
                    if (penum->value == prm->rateControl) rateControlSupported = true;
                }
                if (!rateControlSupported) {
                    message += strsprintf(_T("GPU #%d (%s) does not support %s %s mode.\n"), (*gpu)->id(), (*gpu)->name().c_str(),
                        CodecToStr(prm->codec).c_str(), get_cx_desc(get_rc_method(prm->codec), prm->rateControl));
                    gpu = gpuList.erase(gpu);
                    continue;
                }
            }
            encoder->Clear();
        }

        if (m_pFileReader->getInputCodec() != RGY_CODEC_UNKNOWN) {
            const auto dec_csp = (*gpu)->getHWDecCodecCsp();
            if (dec_csp.count(prm->input.codec) == 0) {
                message += strsprintf(_T("GPU #%d (%s) does not support %s decoding.\n"), (*gpu)->id(), (*gpu)->name().c_str(), CodecToStr(prm->input.codec).c_str());
                gpu = gpuList.erase(gpu);
                continue;
            }
            const auto targetCsp = RGY_CSP_BIT_DEPTH[prm->input.csp] > 8 ? RGY_CSP_P010 : RGY_CSP_NV12;
            const auto& cuvid_codec_csp = dec_csp.at(prm->input.codec);
            if (std::find(cuvid_codec_csp.begin(), cuvid_codec_csp.end(), targetCsp) == cuvid_codec_csp.end()) {
                message += strsprintf(_T("GPU #%d (%s) does not support %s %s decoding.\n"), (*gpu)->id(), (*gpu)->name().c_str(), CodecToStr(prm->input.codec).c_str(), RGY_CSP_NAMES[targetCsp]);
                gpu = gpuList.erase(gpu);
                continue;
            }
        }

        PrintMes(RGY_LOG_DEBUG, _T("GPU #%d (%s) available for encode.\n"), (*gpu)->id(), (*gpu)->name().c_str());
        gpu++;
    }
    PrintMes((gpuList.size() == 0) ? RGY_LOG_ERROR : RGY_LOG_DEBUG, _T("%s\n"), message.c_str());
    if (gpuList.size() == 0) {
        return RGY_ERR_UNSUPPORTED;
    }
    if (gpuList.size() == 1) {
        return RGY_ERR_NONE;
    }

    if (prm->nBframes > 0) {
        bool support_bframe = false;
        for (auto gpu = gpuList.begin(); gpu != gpuList.end(); gpu++) {
            //コーデックのチェック
            amf::AMFCapsPtr encoderCaps = (*gpu)->getEncCaps(prm->codec);
            if (encoderCaps == nullptr) {
                PrintMes(RGY_LOG_ERROR, _T("checkGPUListByEncoder: Unexpected error!.\n"));
                return RGY_ERR_UNKNOWN;
            }
            if (prm->codec == RGY_CODEC_H264) {
                bool bBPictureSupported = false;
                encoderCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_BFRAMES, &bBPictureSupported);
                support_bframe |= bBPictureSupported;
            }
        }
        //BフレームサポートのあるGPUがあれば、BフレームをサポートしないGPUは除外する
        if (support_bframe) {
            for (auto gpu = gpuList.begin(); gpu != gpuList.end(); ) {
                //コーデックのチェック
                amf::AMFCapsPtr encoderCaps = (*gpu)->getEncCaps(prm->codec);
                if (encoderCaps == nullptr) {
                    PrintMes(RGY_LOG_ERROR, _T("checkGPUListByEncoder: Unexpected error!.\n"));
                    return RGY_ERR_UNKNOWN;
                }
                if (prm->codec == RGY_CODEC_H264) {
                    bool bBPictureSupported = false;
                    encoderCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_BFRAMES, &bBPictureSupported);
                    if (!bBPictureSupported) {
                        PrintMes(RGY_LOG_DEBUG, _T("GPU #%d (%s) [%s] does not support B frame.\n"),
                            (*gpu)->id(), (*gpu)->name().c_str(), CodecToStr(prm->codec).c_str());
                        gpu = gpuList.erase(gpu);
                        continue;
                    }
                }
                gpu++;
            }
        }
    }

    if (prm->pa.enable) {
        //Pre-analysisのチェック
        bool support_preanalysis = false;
        for (auto gpu = gpuList.begin(); gpu != gpuList.end(); gpu++) {
            amf::AMFCapsPtr encoderCaps = (*gpu)->getEncCaps(prm->codec);
            if (encoderCaps == nullptr) {
                PrintMes(RGY_LOG_ERROR, _T("checkGPUListByEncoder: Unexpected error!.\n"));
                return RGY_ERR_UNKNOWN;
            }
            bool preanalysisSupported = false;
            encoderCaps->GetProperty(AMF_PARAM_CAP_PRE_ANALYSIS(prm->codec), &preanalysisSupported);
            support_preanalysis |= preanalysisSupported;
        }
        //Pre-analysisサポートのあるGPUがあれば、Pre-analysisをサポートしないGPUは除外する
        if (support_preanalysis) {
            for (auto gpu = gpuList.begin(); gpu != gpuList.end(); ) {
                amf::AMFCapsPtr encoderCaps = (*gpu)->getEncCaps(prm->codec);
                if (encoderCaps == nullptr) {
                    PrintMes(RGY_LOG_ERROR, _T("checkGPUListByEncoder: Unexpected error!.\n"));
                    return RGY_ERR_UNKNOWN;
                }
                bool preanalysisSupported = false;
                encoderCaps->GetProperty(AMF_PARAM_CAP_PRE_ANALYSIS(prm->codec), &preanalysisSupported);
                if (!preanalysisSupported) {
                    PrintMes(RGY_LOG_DEBUG, _T("GPU #%d (%s) [%s] does not support pre analysis.\n"),
                        (*gpu)->id(), (*gpu)->name().c_str(), CodecToStr(prm->codec).c_str());
                    gpu = gpuList.erase(gpu);
                    continue;
                }
                gpu++;
            }
        }
    }
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::gpuAutoSelect(std::vector<std::unique_ptr<VCEDevice>> &gpuList, const VCEParam *prm) {
    if (gpuList.size() <= 1) {
        return RGY_ERR_NONE;
    }
#if ENABLE_PERF_COUNTER
    bool counterIsIntialized = m_pPerfMonitor->isPerfCounterInitialized();
    for (int i = 0; i < 4 && !counterIsIntialized; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        counterIsIntialized = m_pPerfMonitor->isPerfCounterInitialized();
    }
    if (!counterIsIntialized) {
        return RGY_ERR_NONE;
    }
    while (!m_pPerfMonitor->isPerfCounterRefreshed()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    auto entries = m_pPerfMonitor->GetPerfCountersSystem();

    std::map<int, double> gpuscore;
    for (const auto &gpu : gpuList) {
        auto counters = RGYGPUCounterWinEntries(entries).filter_luid(gpu->luid()).get();
        auto ve_utilization = std::max(
            RGYGPUCounterWinEntries(counters).filter_type(L"codec").max(), //vce
            RGYGPUCounterWinEntries(counters).filter_type(L"encode").max());
        auto gpu_utilization = std::max(std::max(std::max(
            RGYGPUCounterWinEntries(counters).filter_type(L"cuda").max(), //nvenc
            RGYGPUCounterWinEntries(counters).filter_type(L"compute").max()), //vce-opencl
            RGYGPUCounterWinEntries(counters).filter_type(L"3d").max()), //qsv
            RGYGPUCounterWinEntries(counters).filter_type(L"videoprocessing").max());
        double core_score = 0.0;
        double cc_score = 0.0;
        double ve_score = 100.0 * (1.0 - std::pow(ve_utilization / 100.0, 1.0)) * prm->ctrl.gpuSelect.ve;
        double gpu_score = 100.0 * (1.0 - std::pow(gpu_utilization / 100.0, 1.5)) * prm->ctrl.gpuSelect.gpu;

        gpuscore[gpu->id()] = cc_score + ve_score + gpu_score + core_score;
        PrintMes(RGY_LOG_DEBUG, _T("GPU #%d (%s) score: %.1f: VE %.1f, GPU %.1f, CC %.1f, Core %.1f.\n"), gpu->id(), gpu->name().c_str(),
            gpuscore[gpu->id()], ve_score, gpu_score, cc_score, core_score);
    }
    std::sort(gpuList.begin(), gpuList.end(), [&](const std::unique_ptr<VCEDevice> &a, const std::unique_ptr<VCEDevice> &b) {
        if (gpuscore.at(a->id()) != gpuscore.at(b->id())) {
            return gpuscore.at(a->id()) > gpuscore.at(b->id());
        }
        return a->id() < b->id();
        });

    PrintMes(RGY_LOG_DEBUG, _T("GPU Priority\n"));
    for (const auto &gpu : gpuList) {
        PrintMes(RGY_LOG_DEBUG, _T("GPU #%d (%s): score %.1f\n"), gpu->id(), gpu->name().c_str(), gpuscore[gpu->id()]);
    }
#endif //#if ENABLE_PERF_COUNTER
    return RGY_ERR_NONE;
}

#pragma warning(push)
#pragma warning(disable: 4127) //C4127: 条件式が定数です。
RGY_ERR VCECore::initDevice(std::vector<std::unique_ptr<VCEDevice>> &gpuList, int deviceId) {
    if (VULKAN_DEFAULT_DEVICE_ONLY && deviceId > 0) {
        PrintMes(RGY_LOG_ERROR, _T("Currently default device is always used when using vulkan!: selected device = %d\n"), deviceId);
        return RGY_ERR_UNSUPPORTED;
    }
    auto gpu = (deviceId < 0)
        ? gpuList.begin()
        : std::find_if(gpuList.begin(), gpuList.end(), [device_id = deviceId](const std::unique_ptr<VCEDevice> &gpuinfo) {
            return gpuinfo->id() == device_id;
          });
    if (gpu == gpuList.end()) {
        PrintMes(RGY_LOG_ERROR, _T("Selected device #%d not found\n"), deviceId);
        return RGY_ERR_NOT_FOUND;
    }
    PrintMes(RGY_LOG_DEBUG, _T("InitDevice: device #%d (%s) selected.\n"), (*gpu)->id(), (*gpu)->name().c_str());
    m_dev = std::move(*gpu);
    return RGY_ERR_NONE;
}
#pragma warning(pop)

RGY_ERR VCECore::initSSIMCalc(VCEParam *prm) {
    if (prm->common.metric.enabled()) {
        if (!m_dev->cl()) {
            PrintMes(RGY_LOG_ERROR, _T("OpenCL disabled, %s calculation not supported!\n"), prm->common.metric.enabled_metric().c_str());
            return RGY_ERR_UNSUPPORTED;
        }
        const auto formatOut = csp_rgy_to_enc(GetEncoderCSP(prm));
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilterSsim> filterSsim(new RGYFilterSsim(m_dev->cl()));
        shared_ptr<RGYFilterParamSsim> param(new RGYFilterParamSsim());
        param->input = videooutputinfo(
            prm->codec,
            formatOut,
            m_params,
            m_picStruct,
            m_encVUI
        );
        param->factory = m_pFactory;
        param->trace = m_pTrace;
        param->context = m_dev->context();
        param->input.srcWidth = m_encWidth;
        param->input.srcHeight = m_encHeight;
        param->bitDepth = prm->outputDepth;
        param->frameIn = (m_pLastFilterParam) ? m_pLastFilterParam->frameOut : RGYFrameInfo(m_encWidth, m_encHeight, GetEncoderCSP(prm), GetEncoderBitdepth(prm), m_picStruct, RGY_MEM_TYPE_GPU_IMAGE);
        param->frameOut = param->frameIn;
        param->frameOut.csp = param->input.csp;
        param->frameIn.mem_type = RGY_MEM_TYPE_GPU;
        param->frameOut.mem_type = RGY_MEM_TYPE_GPU;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        param->threadParam = prm->ctrl.threadParams.get(RGYThreadType::VIDEO_QUALITY);;
        param->metric = prm->common.metric;
        auto sts = filterSsim->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        m_videoQualityMetric = std::move(filterSsim);
    }
    return RGY_ERR_NONE;
}

bool VCECore::VppAfsRffAware() const {
    //vpp-afsのrffが使用されているか
    bool vpp_afs_rff_aware = false;
    for (const auto& filter_block : m_vpFilters) {
        if (filter_block.type == VppFilterType::FILTER_OPENCL) {
            const auto vpp_afs_filter = std::find_if(filter_block.vppcl.begin(), filter_block.vppcl.end(),
                [](const unique_ptr<RGYFilter>& filter) { return typeid(*filter) == typeid(RGYFilterAfs); });
            if (vpp_afs_filter == filter_block.vppcl.end()) continue;
            auto afs_prm = reinterpret_cast<const RGYFilterParamAfs *>((*vpp_afs_filter)->GetFilterParam());
            if (afs_prm != nullptr) {
                vpp_afs_rff_aware |= afs_prm->afs.rff;
            }
        }
    }
    return vpp_afs_rff_aware;
}

RGY_ERR VCECore::initPipeline(VCEParam *prm) {
    m_pipelineTasks.clear();

    if (m_pDecoder) {
        m_pipelineTasks.push_back(std::make_unique<PipelineTaskAMFDecode>(m_pDecoder, m_dev->context(), 1, m_pFileReader.get(), m_pLog));
    } else {
        m_pipelineTasks.push_back(std::make_unique<PipelineTaskInput>(m_dev->context(), 0, m_pFileReader.get(), m_dev->cl(), m_pLog));
    }
    if (m_pFileWriterListAudio.size() > 0) {
        m_pipelineTasks.push_back(std::make_unique<PipelineTaskAudio>(m_dev->context(), m_pFileReader.get(), m_AudioReaders, m_pFileWriterListAudio, m_vpFilters, 0, m_pLog));
    }
    { // checkpts
        RGYInputAvcodec *pReader = dynamic_cast<RGYInputAvcodec *>(m_pFileReader.get());
        const int64_t outFrameDuration = std::max<int64_t>(1, rational_rescale(1, m_inputFps.inv(), m_outputTimebase)); //固定fpsを仮定した時の1フレームのduration (スケール: m_outputTimebase)
        const auto inputFrameInfo = m_pFileReader->GetInputFrameInfo();
        const auto inputFpsTimebase = rgy_rational<int>((int)inputFrameInfo.fpsD, (int)inputFrameInfo.fpsN);
        const auto srcTimebase = (m_pFileReader->getInputTimebase().n() > 0 && m_pFileReader->getInputTimebase().is_valid()) ? m_pFileReader->getInputTimebase() : inputFpsTimebase;
        if (m_trimParam.list.size() > 0) {
            m_pipelineTasks.push_back(std::make_unique<PipelineTaskTrim>(m_dev->context(), m_trimParam, m_pFileReader.get(), srcTimebase, 0, m_pLog));
        }
        m_pipelineTasks.push_back(std::make_unique<PipelineTaskCheckPTS>(m_dev->context(), srcTimebase, srcTimebase, m_outputTimebase, outFrameDuration, m_nAVSyncMode, VppAfsRffAware(), (pReader) ? pReader->GetFramePosList() : nullptr, m_pLog));
    }

    for (auto& filterBlock : m_vpFilters) {
        if (filterBlock.type == VppFilterType::FILTER_AMF) {
            m_pipelineTasks.push_back(std::make_unique<PipelineTaskAMFPreProcess>(m_dev->context(), filterBlock.vppamf, m_dev->cl(), 1, m_pLog));
        } else if (filterBlock.type == VppFilterType::FILTER_OPENCL) {
            if (!m_dev->cl()) {
                PrintMes(RGY_LOG_ERROR, _T("OpenCL not enabled, OpenCL filters cannot be used.\n"));
                return RGY_ERR_UNSUPPORTED;
            }
            m_pipelineTasks.push_back(std::make_unique<PipelineTaskOpenCL>(m_dev->context(), filterBlock.vppcl, nullptr, m_dev->cl(), 1, m_dev->dx11interlop(), m_pLog));
        } else {
            PrintMes(RGY_LOG_ERROR, _T("Unknown filter type.\n"));
            return RGY_ERR_UNSUPPORTED;
        }
    }

    if (m_videoQualityMetric) {
        int prevtask = -1;
        for (int itask = (int)m_pipelineTasks.size() - 1; itask >= 0; itask--) {
            if (!m_pipelineTasks[itask]->isPassThrough()) {
                prevtask = itask;
                break;
            }
        }
        if (m_pipelineTasks[prevtask]->taskType() == PipelineTaskType::INPUT) {
            //inputと直接つながる場合はうまく処理できなくなる(うまく同期がとれない)
            //そこで、CopyのOpenCLフィルタを挟んでその中で処理する
            auto err = createOpenCLCopyFilterForPreVideoMetric(prm);
            if (err != RGY_ERR_NONE) {
                PrintMes(RGY_LOG_ERROR, _T("Failed to join mfx vpp session: %s.\n"), get_err_mes(err));
                return err;
            } else if (m_vpFilters.size() != 1) {
                PrintMes(RGY_LOG_ERROR, _T("m_vpFilters.size() != 1.\n"));
                return RGY_ERR_UNDEFINED_BEHAVIOR;
            }
            m_pipelineTasks.push_back(std::make_unique<PipelineTaskOpenCL>(m_dev->context(), m_vpFilters.front().vppcl, m_videoQualityMetric.get(), m_dev->cl(), 1, m_dev->dx11interlop(), m_pLog));
        } else if (m_pipelineTasks[prevtask]->taskType() == PipelineTaskType::OPENCL) {
            auto taskOpenCL = dynamic_cast<PipelineTaskOpenCL*>(m_pipelineTasks[prevtask].get());
            if (taskOpenCL == nullptr) {
                PrintMes(RGY_LOG_ERROR, _T("taskOpenCL == nullptr.\n"));
                return RGY_ERR_UNDEFINED_BEHAVIOR;
            }
            taskOpenCL->setVideoQualityMetricFilter(m_videoQualityMetric.get());
        } else {
            m_pipelineTasks.push_back(std::make_unique<PipelineTaskVideoQualityMetric>(m_dev->context(), m_videoQualityMetric.get(), m_dev->cl(), 0, m_pLog));
        }
    }
    if (m_pEncoder) {
        m_pipelineTasks.push_back(std::make_unique<PipelineTaskAMFEncode>(m_pEncoder, m_encCodec, m_params, m_dev->context(), 1, m_timecode.get(), m_encTimestamp.get(), m_outputTimebase, m_hdr10plus.get(), m_hdr10plusMetadataCopy, m_pLog));
    }

    if (m_pipelineTasks.size() == 0) {
        PrintMes(RGY_LOG_DEBUG, _T("Failed to create pipeline: size = 0.\n"));
        return RGY_ERR_INVALID_OPERATION;
    }

    PrintMes(RGY_LOG_DEBUG, _T("Created pipeline.\n"));
    for (auto& p : m_pipelineTasks) {
        PrintMes(RGY_LOG_DEBUG, _T("  %s\n"), p->print().c_str());
    }
    PrintMes(RGY_LOG_DEBUG, _T("\n"));
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::allocatePiplelineFrames() {
    if (m_pipelineTasks.size() == 0) {
        PrintMes(RGY_LOG_ERROR, _T("allocFrames: pipeline not defined!\n"));
        return RGY_ERR_INVALID_CALL;
    }

    const int asyncdepth = 3;
    PrintMes(RGY_LOG_DEBUG, _T("allocFrames: m_nAsyncDepth - %d frames\n"), asyncdepth);

    PipelineTask *t0 = m_pipelineTasks[0].get();
    for (size_t ip = 1; ip < m_pipelineTasks.size(); ip++) {
        if (t0->isPassThrough()) {
            PrintMes(RGY_LOG_ERROR, _T("allocFrames: t0 cannot be path through task!\n"));
            return RGY_ERR_UNSUPPORTED;
        }
        // 次のtaskを見つける
        PipelineTask *t1 = nullptr;
        for (; ip < m_pipelineTasks.size(); ip++) {
            if (!m_pipelineTasks[ip]->isPassThrough()) { // isPassThroughがtrueなtaskはスキップ
                t1 = m_pipelineTasks[ip].get();
                break;
            }
        }
        if (t1 == nullptr) {
            PrintMes(RGY_LOG_ERROR, _T("AllocFrames: invalid pipeline, t1 not found!\n"));
            return RGY_ERR_UNSUPPORTED;
        }
        PrintMes(RGY_LOG_DEBUG, _T("AllocFrames: %s-%s\n"), t0->print().c_str(), t1->print().c_str());

        const auto t0Alloc = t0->requiredSurfOut();
        const auto t1Alloc = t1->requiredSurfIn();
        int t0RequestNumFrame = 0;
        int t1RequestNumFrame = 0;
        RGYFrameInfo allocateFrameInfo;
        bool allocateOpenCLFrame = false;
        if (t0Alloc.has_value() && t1Alloc.has_value()) {
            t0RequestNumFrame = t0Alloc.value().second;
            t1RequestNumFrame = t1Alloc.value().second;
            allocateFrameInfo = (t0->workSurfacesAllocPriority() >= t1->workSurfacesAllocPriority()) ? t0Alloc.value().first : t1Alloc.value().first;
            allocateFrameInfo.width = std::max(t0Alloc.value().first.width, t1Alloc.value().first.width);
            allocateFrameInfo.height = std::max(t0Alloc.value().first.height, t1Alloc.value().first.height);
        } else if (t0Alloc.has_value()) {
            allocateFrameInfo = t0Alloc.value().first;
            t0RequestNumFrame = t0Alloc.value().second;
        } else if (t1Alloc.has_value()) {
            allocateFrameInfo = t1Alloc.value().first;
            t1RequestNumFrame = t1Alloc.value().second;
        } else {
            PrintMes(RGY_LOG_ERROR, _T("AllocFrames: invalid pipeline: cannot get request from either t0 or t1!\n"));
            return RGY_ERR_UNSUPPORTED;
        }

        if (   (t0->taskType() == PipelineTaskType::OPENCL && !t1->isAMFTask()) // openclとraw出力がつながっているような場合
            || (t1->taskType() == PipelineTaskType::OPENCL && !t0->isAMFTask()) // inputとopenclがつながっているような場合
            ) {
            if (!m_dev->cl()) {
                PrintMes(RGY_LOG_ERROR, _T("AllocFrames: OpenCL filter not enabled.\n"));
                return RGY_ERR_UNSUPPORTED;
            }
            allocateOpenCLFrame = true; // inputとopenclがつながっているような場合
        }
        if (t0->taskType() == PipelineTaskType::OPENCL) {
            t0RequestNumFrame += 4; // 内部でフレームが増える場合に備えて
        }
        if (allocateOpenCLFrame) {
            const int requestNumFrames = std::max(1, t0RequestNumFrame + t1RequestNumFrame + asyncdepth + 1);
            PrintMes(RGY_LOG_DEBUG, _T("AllocFrames: %s-%s, type: CL, %s %dx%d, request %d frames\n"),
                t0->print().c_str(), t1->print().c_str(), RGY_CSP_NAMES[allocateFrameInfo.csp],
                allocateFrameInfo.width, allocateFrameInfo.height, requestNumFrames);
            auto sts = t0->workSurfacesAllocCL(requestNumFrames, allocateFrameInfo, m_dev->cl().get());
            if (sts != RGY_ERR_NONE) {
                PrintMes(RGY_LOG_ERROR, _T("AllocFrames:   Failed to allocate frames for %s-%s: %s."), t0->print().c_str(), t1->print().c_str(), get_err_mes(sts));
                return sts;
            }
        }
        t0 = t1;
    }
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::init(VCEParam *prm) {
    RGY_ERR ret = initLog(prm);
    if (ret != RGY_ERR_NONE) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to initalize logger: %s"), get_err_mes(ret));
        return ret;
    }

    if (const auto affinity = prm->ctrl.threadParams.get(RGYThreadType::PROCESS).affinity; affinity.mode != RGYThreadAffinityMode::ALL) {
        SetProcessAffinityMask(GetCurrentProcess(), affinity.getMask());
        PrintMes(RGY_LOG_DEBUG, _T("Set Process Affinity Mask: %s (0x%llx).\n"), affinity.to_string().c_str(), affinity.getMask());
    }
    if (const auto affinity = prm->ctrl.threadParams.get(RGYThreadType::MAIN).affinity; affinity.mode != RGYThreadAffinityMode::ALL) {
        SetThreadAffinityMask(GetCurrentThread(), affinity.getMask());
        PrintMes(RGY_LOG_DEBUG, _T("Set Main thread Affinity Mask: %s (0x%llx).\n"), affinity.to_string().c_str(), affinity.getMask());
    }
    // VCE関連の初期化前にカウンターを起動しないと、COM周りのエラーで正常に取得できなくなる場合がある
    // そのため、AMF関係の初期化前にperf counterを初期化する
    m_pPerfMonitor = std::make_unique<CPerfMonitor>();
#if ENABLE_PERF_COUNTER
    m_pPerfMonitor->runCounterThread();
#endif //#if ENABLE_PERF_COUNTER

    ret = initAMFFactory();
    if (ret != RGY_ERR_NONE) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to initalize VCE factory: %s"), get_err_mes(ret));
        return ret;
    }

    ret = initTracer(prm->ctrl.loglevel.get(RGY_LOGT_AMF));
    if (ret != RGY_ERR_NONE) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to set up AMF Tracer: %s"), get_err_mes(ret));
        return ret;
    }

    auto devList = createDeviceList(prm->interopD3d9, prm->interopD3d11, prm->interopVulkan, prm->ctrl.enableOpenCL, prm->vpp.checkPerformance);
    if (devList.size() == 0) {
        PrintMes(RGY_LOG_ERROR, _T("Could not find device to run VCE."));
        return ret;
    }

#if defined(_WIN32) || defined(_WIN64)
    if (prm->bTimerPeriodTuning) {
        m_bTimerPeriodTuning = true;
        timeBeginPeriod(1);
        PrintMes(RGY_LOG_DEBUG, _T("timeBeginPeriod(1)\n"));
    }
#endif //#if defined(_WIN32) || defined(_WIN64)
    if (prm->ctrl.lowLatency) {
        m_pipelineDepth = 1;
        PrintMes(RGY_LOG_DEBUG, _T("lowlatency mode.\n"));
    }

    if (!m_pStatus) {
        m_pStatus = std::make_shared<EncodeStatus>();
    }

    m_nAVSyncMode = prm->common.AVSyncMode;

    if (RGY_ERR_NONE != (ret = initInput(prm, devList))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = checkParam(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = checkGPUListByEncoder(devList, prm, prm->deviceID))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = gpuAutoSelect(devList, prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initDevice(devList, prm->deviceID))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initDecoder(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initFilters(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initEncoder(prm))) {
        return ret;
    }

    m_encTimestamp = std::make_unique<RGYTimestamp>();

    if (RGY_ERR_NONE != (ret = initPowerThrottoling(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initChapters(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initPerfMonitor(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initOutput(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initSSIMCalc(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initPipeline(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = allocatePiplelineFrames())) {
        return ret;
    }

    {
        const auto& threadParam = prm->ctrl.threadParams.get(RGYThreadType::MAIN);
        threadParam.apply(GetCurrentThread());
        PrintMes(RGY_LOG_DEBUG, _T("Set main thread param: %s.\n"), threadParam.desc().c_str());
    }

    return ret;
}

RGY_ERR VCECore::run_decode() {
#if THREAD_DEC_USE_FUTURE
    m_thDecoder = std::async(std::launch::async, [this]() {
#else
    m_thDecoder = std::thread([this]() {
#endif //#if THREAD_DEC_USE_FUTURE
        auto pAVCodecReader = std::dynamic_pointer_cast<RGYInputAvcodec>(m_pFileReader);
        if (pAVCodecReader == nullptr) {
            return RGY_ERR_UNKNOWN;
        }
        const auto VCE_TIMEBASE = rgy_rational<int>(1, AMF_SECOND); // In 100 NanoSeconds
        const auto inTimebase = pAVCodecReader->GetInputVideoStream()->time_base;
        RGYBitstream bitstream = RGYBitstreamInit();
        RGY_ERR sts = RGY_ERR_NONE;
        for (int i = 0; sts == RGY_ERR_NONE && m_state == RGY_STATE_RUNNING; i++) {
            if ((  (sts = m_pFileReader->LoadNextFrame(nullptr)) != RGY_ERR_NONE //進捗表示のため
                || (sts = m_pFileReader->GetNextBitstream(&bitstream)) != RGY_ERR_NONE)
                && sts != RGY_ERR_MORE_DATA) {
                m_state = RGY_STATE_ERROR;
                break;
            }

            amf::AMFBufferPtr pictureBuffer;
            if (sts == RGY_ERR_NONE) {
                auto ar = m_dev->context()->AllocBuffer(amf::AMF_MEMORY_HOST, bitstream.size(), &pictureBuffer);
                if (ar != AMF_OK) {
                    return err_to_rgy(ar);
                }
                memcpy(pictureBuffer->GetNative(), bitstream.data(), bitstream.size());

                //const auto duration = rgy_change_scale(bitstream.duration(), to_rgy(inTimebase), VCE_TIMEBASE);
                //const auto pts = rgy_change_scale(bitstream.pts(), to_rgy(inTimebase), VCE_TIMEBASE);
                pictureBuffer->SetDuration(bitstream.duration());
                pictureBuffer->SetPts(bitstream.pts());
            }
            bitstream.setSize(0);
            bitstream.setOffset(0);
            if (pictureBuffer || sts == RGY_ERR_MORE_BITSTREAM /*EOFの場合はDrainを送る*/) {
                auto ar = AMF_OK;
                do {
                    if (sts == RGY_ERR_MORE_BITSTREAM) {
                        ar = m_pDecoder->Drain();
                    } else {
                        ar = m_pDecoder->SubmitInput(pictureBuffer);
                    }
                    if (ar == AMF_NEED_MORE_INPUT) {
                        break;
                    } else if (ar == AMF_RESOLUTION_CHANGED || ar == AMF_RESOLUTION_UPDATED) {
                        PrintMes(RGY_LOG_ERROR, _T("ERROR: Resolution changed during decoding.\n"));
                        break;
                    } else if (ar == AMF_INPUT_FULL  || ar == AMF_DECODER_NO_FREE_SURFACES) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    } else if (ar == AMF_REPEAT) {
                        pictureBuffer = nullptr;
                    } else {
                        break;
                    }
                } while (m_state == RGY_STATE_RUNNING);
                if (ar != AMF_OK) {
                    m_state = RGY_STATE_ERROR;
                    return err_to_rgy(ar);
                }
            }
        }
        m_pDecoder->Drain();
        return sts;
    });
    PrintMes(RGY_LOG_DEBUG, _T("Started Decode thread.\n"));
    return RGY_ERR_NONE;
}


RGY_ERR VCECore::run2() {
    PrintMes(RGY_LOG_DEBUG, _T("Encode Thread: RunEncode2...\n"));
    if (m_pipelineTasks.size() == 0) {
        PrintMes(RGY_LOG_DEBUG, _T("Failed to create pipeline: size = 0.\n"));
        return RGY_ERR_INVALID_OPERATION;
    }

#if defined(_WIN32) || defined(_WIN64)
    TCHAR handleEvent[256];
    _stprintf_s(handleEvent, VCEENCC_ABORT_EVENT, GetCurrentProcessId());
    auto heAbort = std::unique_ptr<std::remove_pointer<HANDLE>::type, handle_deleter>((HANDLE)CreateEvent(nullptr, TRUE, FALSE, handleEvent));
    auto checkAbort = [pabort = m_pAbortByUser, &heAbort]() { return ((pabort != nullptr && *pabort) || WaitForSingleObject(heAbort.get(), 0) == WAIT_OBJECT_0) ? true : false; };
#else
    auto checkAbort = [pabort = m_pAbortByUser]() { return  (pabort != nullptr && *pabort); };
#endif
    m_pStatus->SetStart();

    CProcSpeedControl speedCtrl(m_nProcSpeedLimit);

    auto requireSync = [this](const size_t itask) {
        if (itask + 1 >= m_pipelineTasks.size()) return true; // 次が最後のタスクの時

        size_t srctask = itask;
        if (m_pipelineTasks[srctask]->isPassThrough()) {
            for (size_t prevtask = srctask - 1; prevtask >= 0; prevtask--) {
                if (!m_pipelineTasks[prevtask]->isPassThrough()) {
                    srctask = prevtask;
                    break;
                }
            }
        }
        for (size_t nexttask = itask + 1; nexttask < m_pipelineTasks.size(); nexttask++) {
            if (!m_pipelineTasks[nexttask]->isPassThrough()) {
                return m_pipelineTasks[srctask]->requireSync(m_pipelineTasks[nexttask]->taskType());
            }
        }
        return true;
    };

    RGY_ERR err = RGY_ERR_NONE;
    auto setloglevel = [](RGY_ERR err) {
        if (err == RGY_ERR_NONE || err == RGY_ERR_MORE_DATA || err == RGY_ERR_MORE_SURFACE || err == RGY_ERR_MORE_BITSTREAM) return RGY_LOG_DEBUG;
        if (err > RGY_ERR_NONE) return RGY_LOG_WARN;
        return RGY_LOG_ERROR;
    };
    struct PipelineTaskData {
        size_t task;
        std::unique_ptr<PipelineTaskOutput> data;
        PipelineTaskData(size_t t) : task(t), data() {};
        PipelineTaskData(size_t t, std::unique_ptr<PipelineTaskOutput>& d) : task(t), data(std::move(d)) {};
    };
    std::deque<PipelineTaskData> dataqueue;
    {
        auto checkContinue = [&checkAbort](RGY_ERR& err) {
            if (checkAbort() || stdInAbort()) { err = RGY_ERR_ABORTED; return false; }
            return err >= RGY_ERR_NONE || err == RGY_ERR_MORE_DATA || err == RGY_ERR_MORE_SURFACE;
        };
        while (checkContinue(err)) {
            if (dataqueue.empty()) {
                speedCtrl.wait(m_pipelineTasks.front()->outputFrames());
                dataqueue.push_back(PipelineTaskData(0)); // デコード実行用
            }
            while (!dataqueue.empty()) {
                auto d = std::move(dataqueue.front());
                dataqueue.pop_front();
                if (d.task < m_pipelineTasks.size()) {
                    err = RGY_ERR_NONE;
                    auto& task = m_pipelineTasks[d.task];
                    err = task->sendFrame(d.data);
                    if (!checkContinue(err)) {
                        PrintMes(setloglevel(err), _T("Break in task %s: %s.\n"), task->print().c_str(), get_err_mes(err));
                        break;
                    }
                    if (err == RGY_ERR_NONE) {
                        auto output = task->getOutput(requireSync(d.task));
                        if (output.size() == 0) break;
                        //出てきたものは先頭に追加していく
                        std::for_each(output.rbegin(), output.rend(), [itask = d.task, &dataqueue](auto&& o) {
                            dataqueue.push_front(PipelineTaskData(itask + 1, o));
                            });
                    }
                } else { // pipelineの最終的なデータを出力
                    if ((err = d.data->write(m_pFileWriter.get(), (m_dev->cl()) ? &m_dev->cl()->queue() : nullptr, m_videoQualityMetric.get())) != RGY_ERR_NONE) {
                        PrintMes(RGY_LOG_ERROR, _T("failed to write output: %s.\n"), get_err_mes(err));
                        break;
                    }
                }
            }
            if (dataqueue.empty()) {
                // taskを前方からひとつづつ出力が残っていないかチェック(主にcheckptsの処理のため)
                for (size_t itask = 0; itask < m_pipelineTasks.size(); itask++) {
                    auto& task = m_pipelineTasks[itask];
                    auto output = task->getOutput(requireSync(itask));
                    if (output.size() > 0) {
                        //出てきたものは先頭に追加していく
                        std::for_each(output.rbegin(), output.rend(), [itask, &dataqueue](auto&& o) {
                            dataqueue.push_front(PipelineTaskData(itask + 1, o));
                            });
                        //checkptsの処理上、でてきたフレームはすぐに後続処理に渡したいのでbreak
                        break;
                    }
                }
            }
        }
    }
    // flush
    if (err == RGY_ERR_MORE_BITSTREAM) { // 読み込みの完了を示すフラグ
        err = RGY_ERR_NONE;
        for (auto& task : m_pipelineTasks) {
            task->setOutputMaxQueueSize(0); //flushのため
        }
        auto checkContinue = [&checkAbort](RGY_ERR& err) {
            if (checkAbort()) { err = RGY_ERR_ABORTED; return false; }
            return err >= RGY_ERR_NONE || err == RGY_ERR_MORE_SURFACE;
        };
        for (size_t flushedTaskSend = 0, flushedTaskGet = 0; flushedTaskGet < m_pipelineTasks.size(); ) { // taskを前方からひとつづつflushしていく
            err = RGY_ERR_NONE;
            if (flushedTaskSend == flushedTaskGet) {
                dataqueue.push_back(PipelineTaskData(flushedTaskSend)); //flush用
            }
            while (!dataqueue.empty() && checkContinue(err)) {
                auto d = std::move(dataqueue.front());
                dataqueue.pop_front();
                if (d.task < m_pipelineTasks.size()) {
                    err = RGY_ERR_NONE;
                    auto& task = m_pipelineTasks[d.task];
                    err = task->sendFrame(d.data);
                    if (!checkContinue(err)) {
                        if (d.task == flushedTaskSend) flushedTaskSend++;
                        break;
                    }
                    auto output = task->getOutput(requireSync(d.task));
                    if (output.size() == 0) break;
                    //出てきたものは先頭に追加していく
                    std::for_each(output.rbegin(), output.rend(), [itask = d.task, &dataqueue](auto&& o) {
                        dataqueue.push_front(PipelineTaskData(itask + 1, o));
                        });
                    if (err == RGY_ERR_MORE_DATA) err = RGY_ERR_NONE; //VPPなどでsendFrameがRGY_ERR_MORE_DATAだったが、フレームが出てくる場合がある
                } else { // pipelineの最終的なデータを出力
                    if ((err = d.data->write(m_pFileWriter.get(), (m_dev->cl()) ? &m_dev->cl()->queue() : nullptr, m_videoQualityMetric.get())) != RGY_ERR_NONE) {
                        PrintMes(RGY_LOG_ERROR, _T("failed to write output: %s.\n"), get_err_mes(err));
                        break;
                    }
                }
            }
            if (dataqueue.empty()) {
                // taskを前方からひとつづつ出力が残っていないかチェック(主にcheckptsの処理のため)
                for (size_t itask = flushedTaskGet; itask < m_pipelineTasks.size(); itask++) {
                    auto& task = m_pipelineTasks[itask];
                    auto output = task->getOutput(requireSync(itask));
                    if (output.size() > 0) {
                        //出てきたものは先頭に追加していく
                        std::for_each(output.rbegin(), output.rend(), [itask, &dataqueue](auto&& o) {
                            dataqueue.push_front(PipelineTaskData(itask + 1, o));
                            });
                        //checkptsの処理上、でてきたフレームはすぐに後続処理に渡したいのでbreak
                        break;
                    } else if (itask == flushedTaskGet && flushedTaskGet < flushedTaskSend) {
                        flushedTaskGet++;
                    }
                }
            }
        }
    }

    if (m_videoQualityMetric) {
        PrintMes(RGY_LOG_DEBUG, _T("Flushing video quality metric calc.\n"));
        m_videoQualityMetric->addBitstream(nullptr);
    }

    //vpp-perf-monitor
    std::vector<std::pair<tstring, double>> filter_result;
    for (auto& block : m_vpFilters) {
        if (block.type == VppFilterType::FILTER_OPENCL) {
            for (auto& filter : block.vppcl) {
                auto avgtime = filter->GetAvgTimeElapsed();
                if (avgtime > 0.0) {
                    filter_result.push_back({ filter->name(), avgtime });
                }
            }
        }
    }
    // MFXのコンポーネントをm_pipelineTasksの解放(フレームの解放)前に実施する
    PrintMes(RGY_LOG_DEBUG, _T("Clear vpp filters...\n"));
    m_vpFilters.clear();
    PrintMes(RGY_LOG_DEBUG, _T("Closing m_pmfxDEC/ENC/VPP...\n"));

    if (m_pEncoder != nullptr) {
        PrintMes(RGY_LOG_DEBUG, _T("Closing Encoder...\n"));
        m_pEncoder->Terminate();
        m_pEncoder = nullptr;
        PrintMes(RGY_LOG_DEBUG, _T("Closed Encoder.\n"));
    }

    if (m_pDecoder != nullptr) {
        PrintMes(RGY_LOG_DEBUG, _T("Closing Decoder...\n"));
        m_pDecoder->Terminate();
        m_pDecoder = nullptr;
        PrintMes(RGY_LOG_DEBUG, _T("Closed Decoder.\n"));
    }
    //この中でフレームの解放がなされる
    PrintMes(RGY_LOG_DEBUG, _T("Clear pipeline tasks and allocated frames...\n"));
    m_pipelineTasks.clear();
    PrintMes(RGY_LOG_DEBUG, _T("Waiting for writer to finish...\n"));
    m_pFileWriter->WaitFin();
    PrintMes(RGY_LOG_DEBUG, _T("Write results...\n"));
    if (m_videoQualityMetric) {
        PrintMes(RGY_LOG_DEBUG, _T("Write video quality metric results...\n"));
        m_videoQualityMetric->showResult();
    }
    m_pStatus->WriteResults();
    if (filter_result.size()) {
        PrintMes(RGY_LOG_INFO, _T("\nVpp Filter Performance\n"));
        const auto max_len = std::accumulate(filter_result.begin(), filter_result.end(), 0u, [](uint32_t max_length, std::pair<tstring, double> info) {
            return std::max(max_length, (uint32_t)info.first.length());
            });
        for (const auto& info : filter_result) {
            tstring str = info.first + _T(":");
            for (uint32_t i = (uint32_t)info.first.length(); i < max_len; i++) {
                str += _T(" ");
            }
            PrintMes(RGY_LOG_INFO, _T("%s %8.1f us\n"), str.c_str(), info.second * 1000.0);
        }
    }
    PrintMes(RGY_LOG_DEBUG, _T("RunEncode2: finished.\n"));
    return (err == RGY_ERR_NONE || err == RGY_ERR_MORE_DATA || err == RGY_ERR_MORE_SURFACE || err == RGY_ERR_MORE_BITSTREAM || err > RGY_ERR_NONE) ? RGY_ERR_NONE : err;
}

RGY_ERR VCECore::run_output() {
    m_thOutput = std::async(std::launch::async, [this]() {
        const auto VCE_TIMEBASE = rgy_rational<int>(1, AMF_SECOND);
        while (m_state == RGY_STATE_RUNNING) {
            amf::AMFDataPtr data;
            auto ar = m_pEncoder->QueryOutput(&data);
            if (ar == AMF_REPEAT || (ar == AMF_OK && data == nullptr)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            if (ar == AMF_EOF) break;
            if (ar != AMF_OK) {
                return err_to_rgy(ar);
            }
            amf::AMFBufferPtr buffer(data);
            int64_t value = 0;
            int64_t pts = rgy_change_scale(buffer->GetPts(), VCE_TIMEBASE, m_outputTimebase);
            int64_t duration = rgy_change_scale(buffer->GetDuration(), VCE_TIMEBASE, m_outputTimebase);
            if (buffer->GetProperty(RGY_PROP_TIMESTAMP, &value) == AMF_OK) {
                pts = value;
            }
            if (buffer->GetProperty(RGY_PROP_DURATION, &value) == AMF_OK) {
                duration = value;
            }
            RGYBitstream output = RGYBitstreamInit();
            output.ref((uint8_t *)buffer->GetNative(), buffer->GetSize(), pts, 0, duration);
            if (buffer->GetProperty(AMF_PARAM_OUTPUT_DATA_TYPE(m_encCodec), &value) == AMF_OK) {
                switch ((AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM)value) {
                case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_IDR: output.setFrametype(RGY_FRAMETYPE_IDR); break;
                case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_I:   output.setFrametype(RGY_FRAMETYPE_I); break;
                case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_P:   output.setFrametype(RGY_FRAMETYPE_P); break;
                case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_B:
                default:
                    output.setFrametype((m_encCodec == RGY_CODEC_AV1) ? RGY_FRAMETYPE_P : RGY_FRAMETYPE_B); break;
                }
            }
            if (m_encCodec == RGY_CODEC_H264 || m_encCodec == RGY_CODEC_HEVC) {
                uint32_t value32 = 0;
                if (buffer->GetProperty(AMF_PARAM_STATISTIC_AVERAGE_QP(m_encCodec), &value32) == AMF_OK) {
                    output.setAvgQP(value32);
                }
            }
            if (m_videoQualityMetric) {
                if (!m_videoQualityMetric->decodeStarted()) {
                    m_videoQualityMetric->initDecode(&output);
                }
                m_videoQualityMetric->addBitstream(&output);
            }
            auto err = m_pFileWriter->WriteNextFrame(&output);
            if (err != RGY_ERR_NONE) {
                return err;
            }
        }
        return RGY_ERR_NONE;
    });
    PrintMes(RGY_LOG_DEBUG, _T("Started Output thread.\n"));
    return RGY_ERR_NONE;
}

void VCECore::PrintEncoderParam() {
    PrintMes(RGY_LOG_INFO, GetEncoderParam().c_str());
}

tstring VCECore::GetEncoderParam() {
    const amf::AMFPropertyStorage *pProperty = m_pEncoder;

    auto GetPropertyStr = [pProperty](const wchar_t *pName) {
        const wchar_t *pProp = L"";
        return (pProperty->GetPropertyWString(pName, &pProp) == AMF_OK) ? wstring_to_string(pProp) : "";
    };

    auto GetPropertyInt = [pProperty](const wchar_t *pName) {
        int64_t value = 0;
        return (pProperty->GetProperty(pName, &value) == AMF_OK) ? value : 0;
    };

    auto GetPropertyIntOptional = [pProperty](const wchar_t *pName) {
        int64_t value = 0;
        std::optional<int64_t> ret;
        if (pProperty->GetProperty(pName, &value) == AMF_OK) {
            ret = value;
        }
        return ret;
    };

    auto GetPropertyRatio = [pProperty](const wchar_t *pName) {
        AMFRatio value = AMFConstructRatio(0,0);
        pProperty->GetProperty(pName, &value);
        return value;
    };

    auto GetPropertyRate = [pProperty](const wchar_t *pName) {
        AMFRate value = AMFConstructRate(0,0);
        pProperty->GetProperty(pName, &value);
        return value;
    };

    auto GetPropertyBool = [pProperty](const wchar_t *pName) {
        bool value = false;
        return (pProperty->GetProperty(pName, &value) == AMF_OK) ? value : false;
    };

    auto GetPropertyBoolOptional = [pProperty](const wchar_t *pName) {
        bool value = false;
        std::optional<bool> ret;
        if (pProperty->GetProperty(pName, &value) == AMF_OK) {
            ret = value;
        }
        return ret;
    };
    auto GetPropertyBoolOptionalOnOffAuto = [pProperty](const wchar_t *pName) {
        bool value = false;
        if (pProperty->GetProperty(pName, &value) == AMF_OK) {
            return value ? _T("on") : _T("off");
        }
        return _T("auto");
    };

    auto getPropertyDesc = [pProperty, GetPropertyInt](const wchar_t *pName, const CX_DESC *list) {
        int64_t value = 0;
        if (pProperty->GetProperty(pName, &value) != AMF_OK) {
            return tstring(_T(""));
        }
        auto ptr = get_cx_desc(list, (int)value);
        return (ptr) ? tstring(ptr) : _T("");
    };

    auto getPropertyDescOptional = [pProperty, GetPropertyInt](const wchar_t *pName, const CX_DESC *list) {
        int64_t value = 0;
        std::optional<tstring> ret;
        if (pProperty->GetProperty(pName, &value) != AMF_OK) {
            return ret;
        }
        if (auto ptr = get_cx_desc(list, (int)value); ptr != nullptr && _tcslen(ptr) > 0) {
            ret = tstring(ptr);
        }
        return ret;
    };

    tstring mes;

    TCHAR cpu_info[256];
    getCPUInfo(cpu_info);
    tstring gpu_info = m_dev->getGPUInfo();

    uint32_t nMotionEst = 0x0;
    nMotionEst |= GetPropertyInt(AMF_PARAM_MOTION_HALF_PIXEL(m_encCodec)) ? VCE_MOTION_EST_HALF : 0;
    nMotionEst |= GetPropertyInt(AMF_PARAM_MOTION_QUARTERPIXEL(m_encCodec)) ? VCE_MOTION_EST_QUATER | VCE_MOTION_EST_HALF : 0;

    mes += strsprintf(_T("%s\n"), get_encoder_version());
#if defined(_WIN32) || defined(_WIN64)
    OSVERSIONINFOEXW osversioninfo = { 0 };
    tstring osversionstr = getOSVersion(&osversioninfo);
    mes += strsprintf(_T("OS:            %s %s (%d) [%s]\n"), osversionstr.c_str(), rgy_is_64bit_os() ? _T("x64") : _T("x86"), osversioninfo.dwBuildNumber, getACPCodepageStr().c_str());
#else
    mes += strsprintf(_T("OS:            %s %s\n"), getOSVersion().c_str(), rgy_is_64bit_os() ? _T("x64") : _T("x86"));
#endif
    mes += strsprintf(_T("CPU:           %s\n"), cpu_info);
    mes += strsprintf(_T("GPU:           %s"), gpu_info.c_str());
#if defined(_WIN32) || defined(_WIN64)
    const auto driverVersion = m_dev->getDriverVersion();
    if (driverVersion.length() > 0) {
        mes += _T(" [") + driverVersion + _T("]");
    }
#endif
    mes += _T("\n");
    mes += strsprintf(_T("AMF:           Runtime %d.%d.%d / SDK %d.%d.%d\n"),
        (int)AMF_GET_MAJOR_VERSION(m_AMFRuntimeVersion), (int)AMF_GET_MINOR_VERSION(m_AMFRuntimeVersion), (int)AMF_GET_SUBMINOR_VERSION(m_AMFRuntimeVersion),
        AMF_VERSION_MAJOR, AMF_VERSION_MINOR, AMF_VERSION_RELEASE);

    if (GetPropertyBool(AMF_PARAM_ENABLE_SMART_ACCESS_VIDEO(m_encCodec))) {
        mes += _T("Smart Access:  on\n");
    }
    auto inputInfo = m_pFileReader->GetInputFrameInfo();
    mes += strsprintf(_T("Input Info:    %s\n"), m_pFileReader->GetInputMessage());
    if (cropEnabled(inputInfo.crop)) {
        mes += strsprintf(_T("Crop:          %d,%d,%d,%d\n"), inputInfo.crop.e.left, inputInfo.crop.e.up, inputInfo.crop.e.right, inputInfo.crop.e.bottom);
    }
    if (m_vpFilters.size() > 0 || m_videoQualityMetric) {
        const TCHAR *m = _T("VPP            ");
        if (m_vpFilters.size() > 0) {
            tstring vppstr;
            for (auto& block : m_vpFilters) {
                if (block.type == VppFilterType::FILTER_AMF) {
                    vppstr += str_replace(block.vppamf->GetInputMessage(), _T("\n               "), _T("\n")) + _T("\n");
                } else if (block.type == VppFilterType::FILTER_OPENCL) {
                    for (auto& clfilter : block.vppcl) {
                        vppstr += str_replace(clfilter->GetInputMessage(), _T("\n               "), _T("\n")) + _T("\n");
                    }
                }
            }
            std::vector<TCHAR> vpp_mes(vppstr.length() + 1, _T('\0'));
            memcpy(vpp_mes.data(), vppstr.c_str(), vpp_mes.size() * sizeof(vpp_mes[0]));
            for (TCHAR *p = vpp_mes.data(), *q; (p = _tcstok_s(p, _T("\n"), &q)) != NULL; ) {
                mes += strsprintf(_T("%s%s\n"), m, p);
                m = _T("               ");
                p = NULL;
            }
        }
        if (m_videoQualityMetric) {
            mes += strsprintf(_T("%s%s\n"), m, m_videoQualityMetric->GetInputMessage().c_str());
        }
    }
    mes += strsprintf(_T("Output:        %s  %s @ Level %s%s\n"),
        CodecToStr(m_encCodec).c_str(),
        getPropertyDesc(AMF_PARAM_PROFILE(m_encCodec), get_profile_list(m_encCodec)).c_str(),
        getPropertyDesc(AMF_PARAM_PROFILE_LEVEL(m_encCodec), get_level_list(m_encCodec)).c_str(),
        (m_encCodec == RGY_CODEC_HEVC) ? (tstring(_T(" (")) + getPropertyDesc(AMF_VIDEO_ENCODER_HEVC_TIER, get_tier_list(m_encCodec)) + _T(" tier)")).c_str() : _T(""));
    const AMF_VIDEO_ENCODER_SCANTYPE_ENUM scan_type = (m_encCodec == RGY_CODEC_H264) ? (AMF_VIDEO_ENCODER_SCANTYPE_ENUM)GetPropertyInt(AMF_VIDEO_ENCODER_SCANTYPE) : AMF_VIDEO_ENCODER_SCANTYPE_PROGRESSIVE;
    AMFRatio aspectRatio = AMFConstructRatio(0,0);
    m_params.GetParam(AMF_PARAM_ASPECT_RATIO(m_encCodec), aspectRatio);
    auto frameRate = GetPropertyRate(AMF_PARAM_FRAMERATE(m_encCodec));
    int64_t outWidth = 0, outHeight = 0;
    m_params.GetParam(VCE_PARAM_KEY_OUTPUT_WIDTH, outWidth);
    m_params.GetParam(VCE_PARAM_KEY_OUTPUT_HEIGHT, outHeight);
    mes += strsprintf(_T("               %dx%d%s %d:%d %0.3ffps (%d/%dfps)\n"),
        (int)outWidth, (int)outHeight,
        scan_type == AMF_VIDEO_ENCODER_SCANTYPE_INTERLACED ? _T("i") : _T("p"),
        aspectRatio.num, aspectRatio.den,
        frameRate.num / (double)frameRate.den, frameRate.num, frameRate.den);
    if (m_pFileWriter) {
        auto mesSplitted = split(m_pFileWriter->GetOutputMessage(), _T("\n"));
        for (auto line : mesSplitted) {
            if (line.length()) {
                mes += strsprintf(_T("%s%s\n"), _T("               "), line.c_str());
            }
        }
    }
    for (auto pWriter : m_pFileWriterListAudio) {
        if (pWriter && pWriter != m_pFileWriter) {
            auto mesSplitted = split(pWriter->GetOutputMessage(), _T("\n"));
            for (auto line : mesSplitted) {
                if (line.length()) {
                    mes += strsprintf(_T("%s%s\n"), _T("               "), line.c_str());
                }
            }
        }
    }
    mes += strsprintf(_T("Quality:       %s\n"), getPropertyDesc(AMF_PARAM_QUALITY_PRESET(m_encCodec), get_quality_preset(m_encCodec)).c_str());
    if (GetPropertyInt(AMF_PARAM_RATE_CONTROL_METHOD(m_encCodec)) == get_rc_method(m_encCodec)[0].value) {
        mes += strsprintf(_T("CQP:           %s:%d, %s:%d"),
            (m_encCodec == RGY_CODEC_AV1) ? _T("Intra") : _T("I"),
            GetPropertyInt(AMF_PARAM_QP_I(m_encCodec)),
            (m_encCodec == RGY_CODEC_AV1) ? _T("Inter") : _T("P"),
            GetPropertyInt(AMF_PARAM_QP_P(m_encCodec)));
        if (m_encCodec == RGY_CODEC_H264 && GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN)) {
            mes += strsprintf(_T(", B:%d"), GetPropertyInt(AMF_VIDEO_ENCODER_QP_B));
        }
        mes += _T("\n");
    } else {
        {
            mes += strsprintf(_T("%s:           %d kbps\n"),
                getPropertyDesc(AMF_PARAM_RATE_CONTROL_METHOD(m_encCodec), get_rc_method(m_encCodec)).c_str(),
                GetPropertyInt(AMF_PARAM_TARGET_BITRATE(m_encCodec)) / 1000);
        }
        if (GetPropertyInt(AMF_PARAM_RATE_CONTROL_METHOD(m_encCodec)) == get_codec_qvbr(m_encCodec)) {
            mes += strsprintf(_T("QVBR level:    %d\n"), GetPropertyInt(AMF_PARAM_QVBR_QUALITY_LEVEL(m_encCodec)));
        }
        mes += strsprintf(_T("Max bitrate:   %d kbps\n"), GetPropertyInt(AMF_PARAM_PEAK_BITRATE(m_encCodec)) / 1000);
        const auto qpmin       = GetPropertyIntOptional(AMF_PARAM_MIN_QP(m_encCodec));
        const auto qpmin_inter = GetPropertyIntOptional(AMF_PARAM_MIN_QP_INTER(m_encCodec));
        const auto qpmax       = GetPropertyIntOptional(AMF_PARAM_MAX_QP(m_encCodec));
        const auto qpmax_inter = GetPropertyIntOptional(AMF_PARAM_MAX_QP_INTER(m_encCodec));
        auto qpminmax_str = [](const decltype(qpmin)& qpminmax) {
            return qpminmax.has_value() ? strsprintf(_T("%d"), qpminmax.value()) : _T("auto");
        };
        mes += strsprintf(_T("QP:            Min: %s:%s, Max: %s:%s\n"),
            qpminmax_str(qpmin).c_str(), qpminmax_str(qpmin_inter).c_str(),
            qpminmax_str(qpmax).c_str(), qpminmax_str(qpmax_inter).c_str());
    }
    mes += strsprintf(_T("VBV Bufsize:   %d kbps\n"), GetPropertyInt(AMF_PARAM_VBV_BUFFER_SIZE(m_encCodec)) / 1000);
    if (m_encCodec == RGY_CODEC_H264 && GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN)) {
        mes += strsprintf(_T("Bframes:       %d frames, b-pyramid: %s\n"),
            GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN),
            (GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN) && GetPropertyInt(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE) ? _T("on") : _T("off")));
        if (GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN)) {
            mes += strsprintf(_T("Delta QP:      Bframe: %d, RefBframe: %d\n"), GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP), GetPropertyInt(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP));
        }
    } else if (m_encCodec != RGY_CODEC_AV1) {
        mes += strsprintf(_T("Bframes:       0 frames\n"));
    }
    const bool pa_enable = GetPropertyBool(AMF_PARAM_PRE_ANALYSIS_ENABLE(m_encCodec));
    mes += strsprintf(_T("Pre Analysis:  "));
    if (pa_enable) {
        tstring pa_str;
        if (GetPropertyBool(AMF_PA_SCENE_CHANGE_DETECTION_ENABLE)) {
            pa_str += _T("sc ") + getPropertyDesc(AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY, list_pa_sc_sensitivity) + _T(", ");
        }
        if (GetPropertyBool(AMF_PA_STATIC_SCENE_DETECTION_ENABLE)) {
            pa_str += _T("ss ") + getPropertyDesc(AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY, list_pa_ss_sensitivity) + _T(", ");
        }
        pa_str += _T("activity ") + getPropertyDesc(AMF_PA_ACTIVITY_TYPE, list_pa_activity) + _T("\n");
        pa_str += _T("               ");
        pa_str += strsprintf(_T("lookahead %d, "), GetPropertyInt(AMF_PA_LOOKAHEAD_BUFFER_DEPTH));
        pa_str += _T("caq ") + getPropertyDesc(AMF_PA_CAQ_STRENGTH, list_pa_caq_strength) + _T(", ");
        pa_str += _T("paq ") + getPropertyDesc(AMF_PA_PAQ_MODE, list_pa_paq_mode) + _T(", ");
        pa_str += _T("taq ") + getPropertyDesc(AMF_PA_TAQ_MODE, list_pa_taq_mode) + _T(", ");
        pa_str += _T("motion-qual ") + getPropertyDesc(AMF_PA_HIGH_MOTION_QUALITY_BOOST_MODE, list_pa_motion_quality_mode) + _T(", ");
        pa_str += _T("ltr ") + tstring(GetPropertyBool(AMF_PA_LTR_ENABLE) ? _T("on") : _T("off")) + _T(", ");
        mes += pa_str.substr(0, pa_str.length()-2) + _T("\n");
    } else {
        mes += _T("off\n");
    }
    mes += strsprintf(_T("Ref frames:    %d frames\n"), GetPropertyInt(AMF_PARAM_MAX_NUM_REFRAMES(m_encCodec)));
    mes += strsprintf(_T("LTR frames:    %d frames\n"), GetPropertyInt(AMF_PARAM_MAX_LTR_FRAMES(m_encCodec)));
    mes += strsprintf(_T("Motion Est:    %s\n"), get_cx_desc(list_mv_presicion, nMotionEst));
    if (m_encCodec == RGY_CODEC_H264 || m_encCodec == RGY_CODEC_HEVC) {
        mes += strsprintf(_T("Slices:        %d\n"), GetPropertyInt(AMF_PARAM_SLICES_PER_FRAME(m_encCodec)));
    } else if (m_encCodec == RGY_CODEC_AV1) {
        mes += strsprintf(_T("Tiles:         %d\n"), GetPropertyInt(AMF_VIDEO_ENCODER_AV1_TILES_PER_FRAME));
    }
    mes += strsprintf(_T("GOP Len:       %d frames\n"), GetPropertyInt(AMF_PARAM_GOP_SIZE(m_encCodec)));
    if (m_encCodec == RGY_CODEC_AV1) {
        if (auto ivalue = GetPropertyIntOptional(AMF_VIDEO_ENCODER_AV1_NUM_TEMPORAL_LAYERS); ivalue.has_value()) {
            mes += strsprintf(_T("Temporal Lyrs: %d\n"), ivalue.value());
        }
        mes += strsprintf(_T("ScreenContent: "));
        auto sct = GetPropertyBoolOptional(AMF_VIDEO_ENCODER_AV1_SCREEN_CONTENT_TOOLS);
        if (sct.has_value()) {
            if (sct.value()) {
                tstring pa_str;
                pa_str += _T("palette-mode ") + tstring(GetPropertyBoolOptionalOnOffAuto(AMF_VIDEO_ENCODER_AV1_PALETTE_MODE)) + _T(", ");
                pa_str += _T("force-integer-mv ") + tstring(GetPropertyBoolOptionalOnOffAuto(AMF_VIDEO_ENCODER_AV1_FORCE_INTEGER_MV)) + _T(", ");
                mes += _T("on,") + pa_str.substr(0, pa_str.length() - 2) + _T("\n");
            } else {
                mes += _T("off\n");
            }
        } else {
            mes += _T("auto\n");
        }
    }
    { const auto &vui_str = m_encVUI.print_all();
    if (vui_str.length() > 0) {
        mes += strsprintf(_T("VUI:              %s\n"), vui_str.c_str());
    }
    }
    if (m_hdr10plus) {
        mes += strsprintf( _T("Dynamic HDR10     %s\n"), m_hdr10plus->inputJson().c_str());
    } else if (m_hdr10plusMetadataCopy) {
        mes += strsprintf( _T("Dynamic HDR10     copy\n"));
    }
    if (m_hdrsei) {
        const auto masterdisplay = m_hdrsei->print_masterdisplay();
        const auto maxcll = m_hdrsei->print_maxcll();
        if (masterdisplay.length() > 0) {
            const tstring tstr = char_to_tstring(masterdisplay);
            const auto splitpos = tstr.find(_T("WP("));
            if (splitpos == std::string::npos) {
                mes += strsprintf(_T("MasteringDisp: %s\n"), tstr.c_str());
            } else {
                mes += strsprintf(_T("MasteringDisp: %s\n")
                    _T("               %s\n"),
                    tstr.substr(0, splitpos - 1).c_str(), tstr.substr(splitpos).c_str());
            }
        }
        if (maxcll.length() > 0) {
            mes += strsprintf(_T("MaxCLL/MaxFALL:%s\n"), char_to_tstring(maxcll).c_str());
        }
    }
    tstring others;
    if (auto bvalue = GetPropertyBoolOptional(AMF_PARAM_RATE_CONTROL_SKIP_FRAME_ENABLE(m_encCodec)); bvalue.has_value()) {
        others += tstring(_T("skipframe:")) + (bvalue ? _T("on") : _T("off")) + _T(" ");
    }
    bool bDeblock = true;
    switch (m_encCodec) {
    case RGY_CODEC_H264:
        bDeblock = GetPropertyBool(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER);
        break;
    case RGY_CODEC_HEVC:
        bDeblock = !GetPropertyBool(AMF_VIDEO_ENCODER_HEVC_DE_BLOCKING_FILTER_DISABLE);
        break;
    default:
        break;
    }
    others += (bDeblock) ? _T("deblock ") : _T("no_deblock ");
    if (m_encCodec == RGY_CODEC_H264 || m_encCodec == RGY_CODEC_HEVC) {
        if (m_pLog->getLogLevel(RGY_LOGT_CORE) <= RGY_LOG_DEBUG) {
            if (GetPropertyBool(AMF_PARAM_INSERT_AUD(m_encCodec))) {
                others += _T("aud ");
            }
        }
        if (GetPropertyBool(AMF_PARAM_LOWLATENCY_MODE(m_encCodec))) {
            others += _T("lowlatency ");
        }
        if (GetPropertyBool(AMF_PARAM_ENABLE_VBAQ(m_encCodec))) {
            others += _T("vbaq ");
        }
    }
    if (m_encCodec == RGY_CODEC_H264) {
        if (GetPropertyBool(AMF_VIDEO_ENCODER_INSERT_SPS)) {
            others += _T("sps ");
        }
        if (GetPropertyBool(AMF_VIDEO_ENCODER_INSERT_PPS)) {
            others += _T("pps ");
        }
    } else if (m_encCodec == RGY_CODEC_HEVC) {
        if (GetPropertyBool(AMF_VIDEO_ENCODER_HEVC_INSERT_HEADER)) {
            others += _T("sps pps vps ");
        }
    } else if (m_encCodec == RGY_CODEC_AV1) {
        if (auto val = getPropertyDescOptional(AMF_VIDEO_ENCODER_AV1_AQ_MODE, list_av1_cdef_mode); val.has_value()) {
            others += _T("aq:") + val.value() + _T(" ");
        }
        if (auto val = getPropertyDescOptional(AMF_VIDEO_ENCODER_AV1_CDEF_MODE, list_av1_cdef_mode); val.has_value()) {
            others += _T("cdef:") + val.value() + _T(" ");
        }
        if (auto bvalue = GetPropertyBoolOptional(AMF_VIDEO_ENCODER_AV1_CDF_UPDATE); bvalue.has_value()) {
            others += tstring(_T("cdf-update:")) + (bvalue ? _T("on") : _T("off")) + _T(" ");
        }
        if (auto ivalue = GetPropertyIntOptional(AMF_VIDEO_ENCODER_AV1_CDF_FRAME_END_UPDATE_MODE); ivalue.has_value()) {
            others += tstring(_T("cdf-frame-end-update:")) + ((ivalue.value() != AMF_VIDEO_ENCODER_AV1_CDF_FRAME_END_UPDATE_MODE_DISABLE) ? _T("on") : _T("off")) + _T(" ");
        }
    }
    if (GetPropertyBool(AMF_PARAM_PREENCODE_ENABLE(m_encCodec))) {
        others += _T("pe ");
    }
    if (GetPropertyBool(AMF_PARAM_ENFORCE_HRD(m_encCodec))) {
        others += _T("hrd ");
    }
    if (GetPropertyBool(AMF_PARAM_FILLER_DATA_ENABLE(m_encCodec))) {
        others += _T("filler ");
    }
    if (m_encCodec == RGY_CODEC_H264) {
        if (GetPropertyBool(AMF_VIDEO_ENCODER_FULL_RANGE_COLOR)) {
            others += _T("fullrange ");
        }
        if (GetPropertyBool(AMF_VIDEO_ENCODER_ADAPTIVE_MINIGOP)) {
            others += _T("adaptMiniGOP ");
        }
    }
    if (others.length() > 0) {
        mes += strsprintf(_T("Others:        %s\n"), others.c_str());
    }
    return mes;
}

void VCECore::PrintResult() {
    m_pStatus->WriteResults();
}

RGY_ERR VCEFeatures::init(int deviceId, const RGYParamLogLevel& loglevel) {
    m_core = std::make_unique<VCECore>();
    auto err = RGY_ERR_NONE;
    if (   (err = m_core->initLogLevel(loglevel)) != RGY_ERR_NONE
        || (err = m_core->initAMFFactory()) != RGY_ERR_NONE
        || (err = m_core->initTracer(loglevel.get(RGY_LOGT_AMF))) != RGY_ERR_NONE) {
        return err;
    }

#if ENABLE_D3D11
    auto devList = m_core->createDeviceList(false, true, false, true, false);
#else
    auto devList = m_core->createDeviceList(false, false, true, true, false);
#endif
    if ((err = m_core->initDevice(devList, deviceId)) != RGY_ERR_NONE) {
        return err;
    }
    return RGY_ERR_NONE;
}

tstring VCEFeatures::checkEncFeatures(RGY_CODEC codec) {
    tstring str;
    amf::AMFCapsPtr encCaps = m_core->dev()->getEncCaps(codec);
    if (encCaps != nullptr) {
        str = m_core->dev()->QueryEncCaps(codec, encCaps);
    }
    return str;
}

tstring VCEFeatures::checkDecFeatures(RGY_CODEC codec) {
    tstring str;
    amf::AMFCapsPtr decCaps = m_core->dev()->getDecCaps(codec);
    if (decCaps != nullptr) {
        str = m_core->dev()->QueryDecCaps(codec, decCaps);
    }
    return str;
}

tstring VCEFeatures::checkFilterFeatures(const std::wstring& filter) {
    tstring str;
    amf::AMFCapsPtr decCaps = m_core->dev()->getFilterCaps(filter);
    if (decCaps != nullptr) {
        str = m_core->dev()->QueryFilterCaps(decCaps);
    }
    return str;
}

bool check_if_vce_available(int deviceId, const RGYParamLogLevel& loglevel) {
    VCEFeatures vce;
    return vce.init(deviceId, loglevel) == RGY_ERR_NONE;
}

tstring check_vce_enc_features(const std::vector<RGY_CODEC> &codecs, int deviceId, const RGYParamLogLevel& loglevel) {
    VCEFeatures vce;
    if (vce.init(deviceId, loglevel) != RGY_ERR_NONE) {
        return _T("VCE not available.\n");
    }
    tstring str = strsprintf(_T("device #%d: "), deviceId) + vce.devName() + _T("\n");
    for (const auto codec : codecs) {
        auto ret = vce.checkEncFeatures(codec);
        if (ret.length() > 0) {
            str += CodecToStr(codec) + _T(" encode features\n");
            str += ret + _T("\n");
        }
    }
    return str;
}

tstring check_vce_dec_features(int deviceId, const RGYParamLogLevel& loglevel) {
    VCEFeatures vce;
    if (vce.init(deviceId, loglevel) != RGY_ERR_NONE) {
        return _T("VCE not available.\n");
    }
    tstring str = strsprintf(_T("device #%d: "), deviceId) + vce.devName() + _T("\n");
    for (size_t i = 0; i < _countof(HW_DECODE_LIST); i++) {
        const auto codec = HW_DECODE_LIST[i].rgy_codec;
        auto ret = vce.checkDecFeatures(codec);
        if (ret.length() > 0) {
            str += CodecToStr(codec) + _T(" decode features\n");
            str += ret + _T("\n");
        }
    }
    return str;
}

tstring check_vce_filter_features(int deviceId, const RGYParamLogLevel& loglevel) {
    VCEFeatures vce;
    if (vce.init(deviceId, loglevel) != RGY_ERR_NONE) {
        return _T("VCE not available.\n");
    }
    tstring str = strsprintf(_T("device #%d: "), deviceId) + vce.devName() + _T("\n");
    for (auto& filter : { AMFVideoConverter, AMFPreProcessing, AMFHQScaler, AMFVQEnhancer, AMFPreAnalysis }) {
        auto ret = vce.checkFilterFeatures(filter);
        if (ret.length() > 0) {
            str += wstring_to_tstring(filter) + tstring(_T(" features\n"));
            str += ret + _T("\n");
        }
    }
    return str;
}
