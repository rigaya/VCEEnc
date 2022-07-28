﻿// -----------------------------------------------------------------------------------------
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
#include "rgy_filter_tweak.h"
#include "rgy_filter_transform.h"
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
#include "VideoConverter.h"
#include "Factory.h"

#include "h264_level.h"
#include "hevc_level.h"

void VCECore::PrintMes(RGYLogLevel log_level, const TCHAR *format, ...) {
    if (m_pLog.get() == nullptr || log_level < m_pLog->getLogLevel(RGY_LOGT_CORE)) {
        return;
    }

    va_list args;
    va_start(args, format);

    int len = _vsctprintf(format, args) + 1; // _vscprintf doesn't count terminating '\0'
    vector<TCHAR> buffer(len, 0);
    _vstprintf_s(buffer.data(), len, format, args);
    va_end(args);

    m_pLog->write(log_level, RGY_LOGT_CORE, buffer.data());
}

VCECore::VCECore() :
    m_encCodec(RGY_CODEC_UNKNOWN),
    m_pLog(),
    m_bTimerPeriodTuning(true),
#if ENABLE_AVSW_READER
    m_keyOnChapter(false),
    m_keyFile(),
    m_Chapters(),
#endif
    m_timecode(),
    m_hdr10plusCopy(false),
    m_hdr10plus(),
    m_hdrsei(),
    m_trimParam(),
    m_poolPkt(),
    m_poolFrame(),
    m_pFileReader(),
    m_AudioReaders(),
    m_pFileWriter(),
    m_pFileWriterListAudio(),
    m_pStatus(),
    m_pPerfMonitor(),
    m_queueFrameData(),
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
    m_dll(),
    m_pFactory(nullptr),
    m_pDebug(nullptr),
    m_pTrace(nullptr),
    m_tracer(),
    m_AMFRuntimeVersion(0),
    m_vpFilters(),
    m_pLastFilterParam(),
    m_ssim(),
    m_state(RGY_STATE_STOPPED),
    m_pTrimParam(nullptr),
    m_pDecoder(),
    m_pEncoder(),
    m_pConverter(),
    m_thDecoder(),
    m_thOutput(),
    m_params(),
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

    m_ssim.reset();

    m_pTrimParam = nullptr;

    if (m_pEncoder != nullptr) {
        PrintMes(RGY_LOG_DEBUG, _T("Closing Encoder...\n"));
        m_pEncoder->Terminate();
        m_pEncoder = nullptr;
        PrintMes(RGY_LOG_DEBUG, _T("Closed Encoder.\n"));
    }

    if (m_pConverter != nullptr) {
        PrintMes(RGY_LOG_DEBUG, _T("Closing Converter...\n"));
        m_pConverter->Terminate();
        m_pConverter = nullptr;
        PrintMes(RGY_LOG_DEBUG, _T("Closed Converter.\n"));
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
    m_queueFrameData.clear([](RGYFrameData **data) { if (*data) delete *data; });
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

RGY_ERR VCECore::initLog(RGYLogLevel loglevel) {
    m_pLog.reset(new RGYLog(nullptr, loglevel));
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
    case RGY_CODEC_HEVC: return inputParam->outputDepth;
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
    m_outputTimebase = m_inputFps.inv() * rgy_rational<int>(1, 4);
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
#endif // #if ENABLE_AVSW_READER
    if (inputParam->common.dynamicHdr10plusJson.length() > 0) {
        m_hdr10plus = initDynamicHDR10Plus(inputParam->common.dynamicHdr10plusJson, m_pLog);
        if (!m_hdr10plus) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to initialize hdr10plus reader.\n"));
            return RGY_ERR_INVALID_PARAM;
        }
    } else if (inputParam->common.hdr10plusMetadataCopy) {
        m_hdr10plusCopy = true;
    }
#if ENABLE_DOVI_METADATA_OPTIONS
    if (inputParam->common.doviRpuFile.length() > 0) {
        m_dovirpu = std::make_unique<DOVIRpu>();
        if (m_dovirpu->init(inputParam->common.doviRpuFile.c_str()) != 0) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to open dovi rpu \"%s\".\n"), inputParam->common.doviRpuFile.c_str());
            return NV_ENC_ERR_GENERIC;
        }
    }
#endif // #if ENABLE_DOVI_METADATA_OPTIONS
    return RGY_ERR_NONE;
#else
    return RGY_ERR_UNSUPPORTED;
#endif //#if ENABLE_RAW_READER
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
    prm->nQPMax = clamp(prm->nQPMax, 0, 51);
    prm->nQPMin = clamp(prm->nQPMin, 0, 51);
    prm->nQPI   = clamp(prm->nQPI,   0, 51);
    prm->nQPP   = clamp(prm->nQPP,   0, 51);
    prm->nQPB   = clamp(prm->nQPB,   0, 51);

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
        m_trimParam, m_outputTimebase, m_Chapters, m_hdrsei.get(), nullptr, nullptr, false, false,
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

    PrintMes(RGY_LOG_DEBUG, _T("initialize decoder: %dx%d, %s.\n"),
        prm->input.srcWidth, prm->input.srcHeight,
        wstring_to_tstring(m_pTrace->SurfaceGetFormatName(csp_rgy_to_enc(prm->input.csp))).c_str());
    if (AMF_OK != (res = m_pDecoder->Init(csp_rgy_to_enc(prm->input.csp), prm->input.srcWidth, prm->input.srcHeight))) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to init decoder: %s\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("Initialized decoder\n"));
    return RGY_ERR_NONE;
#else
    return RGY_ERR_NONE;
#endif
}
#pragma warning(pop)

RGY_ERR VCECore::initConverter(VCEParam *prm) {
#if 0 //現状使用していない
    const auto formatOut = csp_rgy_to_enc(GetEncoderCSP(prm));
    if (prm->input.dstWidth == prm->input.srcWidth
        && prm->input.dstHeight == prm->input.srcHeight
        && csp_rgy_to_enc(prm->input.csp) == formatOut) {
        PrintMes(RGY_LOG_DEBUG, _T("converter not required.\n"));
        return RGY_ERR_NONE;
    }
    auto res = m_pFactory->CreateComponent(m_dev->context(), AMFVideoConverter, &m_pConverter);
    if (res != AMF_OK) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to create converter context: %s\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("created converter context.\n"));

    res = m_pConverter->SetProperty(AMF_VIDEO_CONVERTER_MEMORY_TYPE, amf::AMF_MEMORY_OPENCL);
    res = m_pConverter->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_FORMAT, formatOut);
    res = m_pConverter->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_SIZE, AMFConstructSize(prm->input.dstWidth, prm->input.dstHeight));
    res = m_pConverter->SetProperty(AMF_VIDEO_CONVERTER_SCALE, AMF_VIDEO_CONVERTER_SCALE_BICUBIC);
    PrintMes(RGY_LOG_DEBUG, _T("initialize converter by mem type %s, format out %s, output size %dx%x.\n"),
        wstring_to_tstring(m_pTrace->GetMemoryTypeName(amf::AMF_MEMORY_OPENCL)).c_str(),
        wstring_to_tstring(m_pTrace->SurfaceGetFormatName(formatOut)).c_str(),
        prm->input.dstWidth, prm->input.dstHeight);
    if (AMF_OK != (res = m_pConverter->Init(formatOut, prm->input.srcWidth, prm->input.srcHeight))) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to init converter: %s\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("initialized converter.\n"));
#else
    UNREFERENCED_PARAMETER(prm);
#endif
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

    //リサイザの出力すべきサイズ
    int resizeWidth  = croppedWidth;
    int resizeHeight = croppedHeight;
    m_encWidth = resizeWidth;
    m_encHeight = resizeHeight;
    if (inputParam->vpp.pad.enable) {
        m_encWidth  += inputParam->vpp.pad.right + inputParam->vpp.pad.left;
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
    bool resizeRequired = false;
    if (croppedWidth != resizeWidth || croppedHeight != resizeHeight) {
        resizeRequired = true;
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
    //if (inputParam->vpp.yadif.enable) deinterlacer++;
    if (deinterlacer >= 2) {
        PrintMes(RGY_LOG_ERROR, _T("Activating 2 or more deinterlacer is not supported.\n"));
        return RGY_ERR_UNSUPPORTED;
    }

    //VUI情報
    auto VuiFiltered = inputParam->input.vui;

    //フィルタが必要
    if (resizeRequired
        || cropRequired
        || inputParam->vpp.delogo.enable
        || inputParam->vpp.colorspace.enable
        || inputParam->vpp.afs.enable
        || inputParam->vpp.nnedi.enable
        || inputParam->vpp.decimate.enable
        || inputParam->vpp.mpdecimate.enable
        || inputParam->vpp.pad.enable
        || inputParam->vpp.convolution3d.enable
        || inputParam->vpp.knn.enable
        || inputParam->vpp.pmd.enable
        || inputParam->vpp.smooth.enable
        || inputParam->vpp.subburn.size() > 0
        || inputParam->vpp.unsharp.enable
        || inputParam->vpp.edgelevel.enable
        || inputParam->vpp.warpsharp.enable
        || inputParam->vpp.tweak.enable
        || inputParam->vpp.transform.enable
        || inputParam->vpp.deband.enable) {
        if (!m_dev->cl()) {
            PrintMes(RGY_LOG_ERROR, _T("OpenCL disabled, filtering not supported!\n"));
            return RGY_ERR_UNSUPPORTED;
        }
        //swデコードならGPUに上げる必要がある
        if (m_pFileReader->getInputCodec() == RGY_CODEC_UNKNOWN) {
            amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
            unique_ptr<RGYFilter> filterCrop(new RGYFilterCspCrop(m_dev->cl()));
            shared_ptr<RGYFilterParamCrop> param(new RGYFilterParamCrop());
            param->frameIn = inputFrame;
            param->frameOut.csp = param->frameIn.csp;
            param->frameOut.mem_type = RGY_MEM_TYPE_GPU;
            param->baseFps = m_encFps;
            param->bOutOverwrite = false;
            auto sts = filterCrop->init(param, m_pLog);
            if (sts != RGY_ERR_NONE) {
                return sts;
            }
            //フィルタチェーンに追加
            m_vpFilters.push_back(std::move(filterCrop));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        const auto encCsp = GetEncoderCSP(inputParam);
        if (encCsp == RGY_CSP_NA) {
            PrintMes(RGY_LOG_ERROR, _T("Unknown Error in GetEncoderCSP().\n"));
            return RGY_ERR_UNSUPPORTED;
        }
        auto filterCsp = encCsp;
        switch (filterCsp) {
        case RGY_CSP_NV12: filterCsp = RGY_CSP_YV12; break;
        case RGY_CSP_P010: filterCsp = RGY_CSP_YV12_16; break;
        default: break;
        }
        if (inputParam->vpp.afs.enable && RGY_CSP_CHROMA_FORMAT[inputFrame.csp] == RGY_CHROMAFMT_YUV444) {
            filterCsp = (RGY_CSP_BIT_DEPTH[inputFrame.csp] > 8) ? RGY_CSP_YUV444_16 : RGY_CSP_YUV444;
        }
        //colorspace
        if (inputParam->vpp.colorspace.enable) {
            amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
            unique_ptr<RGYFilterColorspace> filter(new RGYFilterColorspace(m_dev->cl()));
            shared_ptr<RGYFilterParamColorspace> param(new RGYFilterParamColorspace());
            param->colorspace = inputParam->vpp.colorspace;
            param->encCsp = encCsp;
            param->VuiIn = VuiFiltered;
            param->frameIn = inputFrame;
            param->frameOut = inputFrame;
            param->baseFps = m_encFps;
            auto sts = filter->init(param, m_pLog);
            if (sts != RGY_ERR_NONE) {
                return sts;
            }
            VuiFiltered = filter->VuiOut();
            //フィルタチェーンに追加
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        if (filterCsp != inputFrame.csp
            || cropRequired) { //cropが必要ならただちに適用する
            amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
            unique_ptr<RGYFilter> filterCrop(new RGYFilterCspCrop(m_dev->cl()));
            shared_ptr<RGYFilterParamCrop> param(new RGYFilterParamCrop());
            param->frameIn = inputFrame;
            param->frameOut.csp = encCsp;
            switch (param->frameOut.csp) {
            case RGY_CSP_NV12:
                param->frameOut.csp = RGY_CSP_YV12;
                break;
            case RGY_CSP_P010:
                param->frameOut.csp = RGY_CSP_YV12_16;
                break;
            default:
                break;
            }
            if (cropRequired) {
                param->crop = inputParam->input.crop;
            }
            param->baseFps = m_encFps;
            param->frameOut.mem_type = RGY_MEM_TYPE_GPU;
            param->bOutOverwrite = false;
            auto sts = filterCrop->init(param, m_pLog);
            if (sts != RGY_ERR_NONE) {
                return sts;
            }
            //フィルタチェーンに追加
            m_vpFilters.push_back(std::move(filterCrop));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //delogo
        if (inputParam->vpp.delogo.enable) {
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
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //afs
        if (inputParam->vpp.afs.enable) {
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
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //nnedi
        if (inputParam->vpp.nnedi.enable) {
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
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //decimate
        if (inputParam->vpp.decimate.enable) {
            amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
            unique_ptr<RGYFilter> filter(new RGYFilterDecimate(m_dev->cl()));
            shared_ptr<RGYFilterParamDecimate> param(new RGYFilterParamDecimate());
            param->decimate = inputParam->vpp.decimate;
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
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //mpdecimate
        if (inputParam->vpp.mpdecimate.enable) {
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
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //回転
        if (inputParam->vpp.transform.enable) {
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
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //ノイズ除去 (convolution3d)
        if (inputParam->vpp.convolution3d.enable) {
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
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //smooth
        if (inputParam->vpp.smooth.enable) {
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
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //knn
        if (inputParam->vpp.knn.enable) {
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
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //pmd
        if (inputParam->vpp.pmd.enable) {
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
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //字幕焼きこみ
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
                m_vpFilters.push_back(std::move(filter));
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
        //リサイズ
        if (resizeRequired) {
            amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
            unique_ptr<RGYFilter> filterResize(new RGYFilterResize(m_dev->cl()));
            shared_ptr<RGYFilterParamResize> param(new RGYFilterParamResize());
            param->interp = (inputParam->vpp.resize_algo != RGY_VPP_RESIZE_AUTO) ? inputParam->vpp.resize_algo : RGY_VPP_RESIZE_SPLINE36;
            param->frameIn = inputFrame;
            param->frameOut = inputFrame;
            param->frameOut.width = resizeWidth;
            param->frameOut.height = resizeHeight;
            param->baseFps = m_encFps;
            param->bOutOverwrite = false;
            auto sts = filterResize->init(param, m_pLog);
            if (sts != RGY_ERR_NONE) {
                return sts;
            }
            //フィルタチェーンに追加
            m_vpFilters.push_back(std::move(filterResize));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //unsharp
        if (inputParam->vpp.unsharp.enable) {
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
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //edgelevel
        if (inputParam->vpp.edgelevel.enable) {
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
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //warpsharp
        if (inputParam->vpp.warpsharp.enable) {
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
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //tweak
        if (inputParam->vpp.tweak.enable) {
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
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //deband
        if (inputParam->vpp.deband.enable) {
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
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        //padding
        if (inputParam->vpp.pad.enable) {
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
            m_vpFilters.push_back(std::move(filter));
            //パラメータ情報を更新
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
    }
    //最後のフィルタ
    if (m_dev->cl()) {
        //もし入力がCPUメモリで色空間が違うなら、一度そのままGPUに転送する必要がある
        if (inputFrame.mem_type == RGY_MEM_TYPE_CPU && inputFrame.csp != GetEncoderCSP(inputParam)) {
            amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
            unique_ptr<RGYFilter> filterCrop(new RGYFilterCspCrop(m_dev->cl()));
            shared_ptr<RGYFilterParamCrop> param(new RGYFilterParamCrop());
            param->frameIn = inputFrame;
            param->frameOut.csp = param->frameIn.csp;
            param->frameOut.mem_type = RGY_MEM_TYPE_GPU_IMAGE;
            param->baseFps = m_encFps;
            param->bOutOverwrite = false;
            auto sts = filterCrop->init(param, m_pLog);
            if (sts != RGY_ERR_NONE) {
                return sts;
            }
            m_vpFilters.push_back(std::move(filterCrop));
            m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
            //入力フレーム情報を更新
            inputFrame = param->frameOut;
            m_encFps = param->baseFps;
        }
        amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
        unique_ptr<RGYFilter> filterCrop(new RGYFilterCspCrop(m_dev->cl()));
        shared_ptr<RGYFilterParamCrop> param(new RGYFilterParamCrop());
        param->frameIn = inputFrame;
        param->frameOut.csp = GetEncoderCSP(inputParam);
        //インタレ保持であれば、CPU側にフレームを戻す必要がある
        //色空間が同じなら、ここでやってしまう
        param->frameOut.mem_type = RGY_MEM_TYPE_GPU_IMAGE;
        param->baseFps = m_encFps;
        param->bOutOverwrite = false;
        auto sts = filterCrop->init(param, m_pLog);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        m_vpFilters.push_back(std::move(filterCrop));
        m_pLastFilterParam = std::dynamic_pointer_cast<RGYFilterParam>(param);
        //入力フレーム情報を更新
        inputFrame = param->frameOut;
        m_encFps = param->baseFps;
    }
    m_picStruct = inputFrame.picstruct;
    m_encVUI = inputParam->common.out_vui;
    m_encVUI.apply_auto(VuiFiltered, m_encHeight);
    m_encVUI.descriptpresent =
        get_cx_value(list_colormatrix, _T("undef")) != (int)m_encVUI.matrix
        || get_cx_value(list_colorprim, _T("undef")) != (int)m_encVUI.colorprim
        || get_cx_value(list_transfer, _T("undef")) != (int)m_encVUI.transfer;

    if (inputParam->vpp.checkPerformance) {
        for (auto& filter : m_vpFilters) {
            filter->setCheckPerformance(inputParam->vpp.checkPerformance);
        }
    }
    return RGY_ERR_NONE;
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
            if (prm->rateControl == AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_QUALITY_VBR) {
                PrintMes(RGY_LOG_ERROR, _T("QVBR mode is not supported on HEVC encoding.\n"));
                return RGY_ERR_UNSUPPORTED;
            }
        } else {
            PrintMes(RGY_LOG_WARN, _T("Unsupported codec.\n"));
            return RGY_ERR_UNSUPPORTED;
        }

        if (prm->pa.enable) {
            bool preAnalysisSupported = false;
            encoderCaps->GetProperty(AMF_PARAM_CAP_PRE_ANALYSIS(prm->codec), &preAnalysisSupported);
            if (!preAnalysisSupported) {
                PrintMes(RGY_LOG_WARN, _T("Pre-analysis is not supported on this device, disabled.\n"));
                prm->pa.enable = false;
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
            && !is_avail_hevc_high_tier(prm->codecParam[prm->codec].nLevel)) {
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
                level = calc_h264_auto_level(m_encWidth, m_encHeight, prm->nRefFrames, false,
                    m_encFps.n(), m_encFps.d(), profile, max_bitrate_kbps, vbv_bufsize_kbps);
                //なんかLevel4.0以上でないと設定に失敗する場合がある
                level = std::max(level, 40);
            }
            get_h264_vbv_value(&max_bitrate_kbps, &vbv_bufsize_kbps, level, profile);
        } else if (prm->codec == RGY_CODEC_HEVC) {
            const bool high_tier = prm->codecParam[prm->codec].nTier == AMF_VIDEO_ENCODER_HEVC_TIER_HIGH;
            if (level == 0) {
                level = calc_hevc_auto_level(m_encWidth, m_encHeight, prm->nRefFrames,
                    m_encFps.n(), m_encFps.d(), high_tier, max_bitrate_kbps);
            }
            max_bitrate_kbps = get_hevc_max_bitrate(level, high_tier);
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
    if (m_sar.is_valid()) {
        m_params.SetParam(AMF_PARAM_ASPECT_RATIO(prm->codec), AMFConstructRatio(m_sar.n(), m_sar.d()));
    }
    m_params.SetParam(AMF_PARAM_USAGE(prm->codec),          (amf_int64)get_encoder_usage(prm->codec));
    m_params.SetParam(AMF_PARAM_PROFILE(prm->codec),        (amf_int64)prm->codecParam[prm->codec].nProfile);
    m_params.SetParam(AMF_PARAM_PROFILE_LEVEL(prm->codec),  (amf_int64)prm->codecParam[prm->codec].nLevel);
    m_params.SetParam(AMF_PARAM_QUALITY_PRESET(prm->codec), (amf_int64)prm->qualityPreset);
    m_params.SetParam(AMF_PARAM_QP_I(prm->codec),           (amf_int64)prm->nQPI);
    m_params.SetParam(AMF_PARAM_QP_P(prm->codec),           (amf_int64)prm->nQPP);
    m_params.SetParam(AMF_PARAM_TARGET_BITRATE(prm->codec), (amf_int64)prm->nBitrate * 1000);
    m_params.SetParam(AMF_PARAM_PEAK_BITRATE(prm->codec),   (amf_int64)prm->nMaxBitrate * 1000);
    m_params.SetParam(AMF_PARAM_MAX_NUM_REFRAMES(prm->codec), (amf_int64)prm->nRefFrames);
    m_params.SetParam(AMF_PARAM_MAX_LTR_FRAMES(prm->codec), (amf_int64)prm->nLTRFrames);
    m_params.SetParam(AMF_PARAM_RATE_CONTROL_SKIP_FRAME_ENABLE(prm->codec),  prm->bEnableSkipFrame);
    m_params.SetParam(AMF_PARAM_RATE_CONTROL_METHOD(prm->codec),             (amf_int64)prm->rateControl);
    m_params.SetParam(AMF_PARAM_VBV_BUFFER_SIZE(prm->codec),                 (amf_int64)prm->nVBVBufferSize * 1000);
    m_params.SetParam(AMF_PARAM_INITIAL_VBV_BUFFER_FULLNESS(prm->codec),     (amf_int64)prm->nInitialVBVPercent);
    m_params.SetParam(AMF_PARAM_LOWLATENCY_MODE(prm->codec), prm->ctrl.lowLatency);

    m_params.SetParam(AMF_PARAM_ENFORCE_HRD(prm->codec),        prm->bEnforceHRD != 0);
    m_params.SetParam(AMF_PARAM_FILLER_DATA_ENABLE(prm->codec), prm->bFiller != 0);
    if (prm->bVBAQ) m_params.SetParam(AMF_PARAM_ENABLE_VBAQ(prm->codec), true);
    m_params.SetParam(AMF_PARAM_SLICES_PER_FRAME(prm->codec),               (amf_int64)prm->nSlices);
    m_params.SetParam(AMF_PARAM_GOP_SIZE(prm->codec),                       (amf_int64)nGOPLen);

    m_params.SetParam(AMF_PARAM_PREENCODE_ENABLE(prm->codec), prm->pe);
    if (prm->pa.enable &&
        (   prm->rateControl != get_codec_vbr(prm->codec)
         && prm->rateControl != get_codec_vbr_lat(prm->codec)
         && prm->rateControl != get_codec_qvbr(prm->codec))) {
        PrintMes(RGY_LOG_WARN, _T("Pre analysis is currently supported only with VBR/QVBR mode.\n"));
        PrintMes(RGY_LOG_WARN, _T("Currenlty %s mode is selected, so pre analysis will be disabled.\n"), get_cx_desc(get_rc_method(prm->codec), prm->rateControl));
        prm->pa.enable = false;
    }
    if (prm->pa.enable) {
        m_params.SetParam(AMF_PARAM_PRE_ANALYSIS_ENABLE(prm->codec), prm->pa.enable);
        m_params.SetParam(AMF_PA_SCENE_CHANGE_DETECTION_ENABLE, prm->pa.sc);
        if (prm->pa.sc) m_params.SetParam(AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY, (amf_int64)prm->pa.scSensitivity);
        m_params.SetParam(AMF_PA_STATIC_SCENE_DETECTION_ENABLE, prm->pa.ss);
        if (prm->pa.ss)  m_params.SetParam(AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY, (amf_int64)prm->pa.ssSensitivity);
        m_params.SetParam(AMF_PA_ACTIVITY_TYPE, (amf_int64)prm->pa.activityType);
        if (prm->pa.initQPSC > AMF_PA_INITQPSC_AUTO) m_params.SetParam(AMF_PA_INITIAL_QP_AFTER_SCENE_CHANGE, (amf_int64)prm->pa.initQPSC); //設定しなければ自動
        m_params.SetParam(AMF_PA_MAX_QP_BEFORE_FORCE_SKIP, (amf_int64)prm->pa.maxQPBeforeForceSkip);
        m_params.SetParam(AMF_PA_CAQ_STRENGTH, (amf_int64)prm->pa.CAQStrength);
    }

    //m_params.SetParam(AMF_PARAM_INPUT_COLOR_PROFILE(prm->codec),           AMF_VIDEO_CONVERTER_COLOR_PROFILE_UNKNOWN);
    //m_params.SetParam(AMF_PARAM_INPUT_TRANSFER_CHARACTERISTIC(prm->codec), AMF_COLOR_TRANSFER_CHARACTERISTIC_UNDEFINED);
    //m_params.SetParam(AMF_PARAM_INPUT_COLOR_PRIMARIES(prm->codec),         AMF_COLOR_PRIMARIES_UNDEFINED);

    //m_params.SetParam(AMF_PARAM_OUTPUT_COLOR_PROFILE(prm->codec),           AMF_VIDEO_CONVERTER_COLOR_PROFILE_UNKNOWN);
    //m_params.SetParam(AMF_PARAM_OUTPUT_TRANSFER_CHARACTERISTIC(prm->codec), AMF_COLOR_TRANSFER_CHARACTERISTIC_UNDEFINED);
    //m_params.SetParam(AMF_PARAM_OUTPUT_COLOR_PRIMARIES(prm->codec),         AMF_COLOR_PRIMARIES_UNDEFINED);

    //m_params.SetParam(AMF_PARAM_END_OF_SEQUENCE(prm->codec),                false);
    m_params.SetParam(AMF_PARAM_INSERT_AUD(prm->codec),                     false);
    if (prm->codec == RGY_CODEC_H264) {
        //m_params.SetParam(AMF_PARAM_RATE_CONTROL_PREANALYSIS_ENABLE(prm->codec), (prm->preAnalysis) ? AMF_VIDEO_ENCODER_PREENCODE_ENABLED : AMF_VIDEO_ENCODER_PREENCODE_DISABLED);

        m_params.SetParam(AMF_VIDEO_ENCODER_QVBR_QUALITY_LEVEL, (amf_int64)prm->qvbrLevel);

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

        m_params.SetParam(AMF_VIDEO_ENCODER_MIN_QP,                                (amf_int64)prm->nQPMin);
        m_params.SetParam(AMF_VIDEO_ENCODER_MAX_QP,                                (amf_int64)prm->nQPMax);

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
        m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_COLOR_BIT_DEPTH,                 (amf_int64)prm->outputDepth);
        //m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_NOMINAL_RANGE,                   (amf_int64)(m_encVUI.colorrange == RGY_COLORRANGE_FULL ? AMF_VIDEO_ENCODER_HEVC_NOMINAL_RANGE_FULL : AMF_VIDEO_ENCODER_HEVC_NOMINAL_RANGE_STUDIO));

        m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_MIN_QP_I,                        (amf_int64)prm->nQPMin);
        m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_MAX_QP_I,                        (amf_int64)prm->nQPMax);
        m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_MIN_QP_P,                        (amf_int64)prm->nQPMin);
        m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_MAX_QP_P,                        (amf_int64)prm->nQPMax);

        m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_DE_BLOCKING_FILTER_DISABLE,      !prm->bDeblockFilter);

        m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_INSERT_HEADER,                   true);
    } else {
        PrintMes(RGY_LOG_ERROR, _T("Unsupported codec.\n"));
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

RGY_ERR VCECore::initAMFFactory() {
    m_dll = std::unique_ptr<std::remove_pointer_t<HMODULE>, module_deleter>(RGY_LOAD_LIBRARY(wstring_to_tstring(AMF_DLL_NAME).c_str()));
    if (!m_dll) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to load %s.\n"), wstring_to_tstring(AMF_DLL_NAME).c_str());
        return RGY_ERR_NOT_FOUND;
    }
    AMFInit_Fn initFun = (AMFInit_Fn)RGY_GET_PROC_ADDRESS(m_dll.get(), AMF_INIT_FUNCTION_NAME);
    if (initFun == NULL) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to load %s.\n"), AMF_INIT_FUNCTION_NAME);
        return RGY_ERR_NOT_FOUND;
    }
    AMF_RESULT res = initFun(AMF_FULL_VERSION, &m_pFactory);
    if (res != AMF_OK) {
        return err_to_rgy(res);
    }
    AMFQueryVersion_Fn versionFun = (AMFQueryVersion_Fn)RGY_GET_PROC_ADDRESS(m_dll.get(), AMF_QUERY_VERSION_FUNCTION_NAME);
    if (versionFun == NULL) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to load %s.\n"), AMF_QUERY_VERSION_FUNCTION_NAME);
        return RGY_ERR_NOT_FOUND;
    }
    res = versionFun(&m_AMFRuntimeVersion);
    if (res != AMF_OK) {
        return err_to_rgy(res);
    }
    m_pFactory->GetTrace(&m_pTrace);
    m_pFactory->GetDebug(&m_pDebug);
    PrintMes(RGY_LOG_DEBUG, _T("Loaded %s.\n"), wstring_to_tstring(AMF_DLL_NAME).c_str());
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::initTracer(int log_level) {
    m_pTrace->EnableWriter(AMF_TRACE_WRITER_DEBUG_OUTPUT, log_level < RGY_LOG_INFO);
    if (log_level < RGY_LOG_INFO)
        m_pTrace->SetWriterLevel(AMF_TRACE_WRITER_DEBUG_OUTPUT, loglevel_rgy_to_enc(log_level));
    m_pTrace->EnableWriter(AMF_TRACE_WRITER_CONSOLE, false);
    m_pTrace->SetGlobalLevel(loglevel_rgy_to_enc(log_level));

    m_tracer.init(m_pLog);
    m_pTrace->RegisterWriter(L"RGYLOGWriter", &m_tracer, log_level < RGY_LOG_INFO);
    m_pTrace->SetWriterLevel(L"RGYLOGWriter", loglevel_rgy_to_enc(log_level));
    return RGY_ERR_NONE;
}

std::vector<std::unique_ptr<VCEDevice>> VCECore::createDeviceList(bool interopD3d9, bool interopD3d11, bool interopVulkan, bool enableOpenCL, bool enableVppPerfMonitor) {
    std::vector<std::unique_ptr<VCEDevice>> devs;
#if ENABLE_D3D11
    const int adapterCount = DeviceDX11::adapterCount();
#elif ENABLE_VULKAN
    int adapterCount = 1;
    if (VULKAN_DEFAULT_DEVICE_ONLY == 0) {
        auto devVk = std::make_unique<DeviceVulkan>();
        adapterCount = devVk->adapterCount();
        devVk.reset(); // VCEDevice::init()を呼ぶ前に開放しないとなぜか処理がうまく進まない
    }
#else
    RGYOpenCL cl(m_pLog);
    auto platforms = cl.getPlatforms("AMD");
    const int adapterCount = std::accumulate(platforms.begin(), platforms.end(), 0, [](int acc, std::shared_ptr<RGYOpenCLPlatform>& p) {
        if (p->createDeviceList(CL_DEVICE_TYPE_GPU) == RGY_ERR_NONE) {
            acc += (int)p->devs().size();
        }
        return acc;
    });
    PrintMes(RGY_LOG_ERROR, _T("adapterCount %d.\n"), adapterCount);
#endif
    for (int i = 0; i < adapterCount; i++) {
        auto dev = std::make_unique<VCEDevice>(m_pLog, m_pFactory, m_pTrace);
        if (dev->init(i, interopD3d9, interopD3d11, interopVulkan, enableOpenCL, enableVppPerfMonitor) == RGY_ERR_NONE) {
            devs.push_back(std::move(dev));
        }
    }
    return devs;
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
                && !prm->vpp.nnedi.enable;
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

        if (prm->input.type == RGY_INPUT_FMT_AVHW) {
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
            RGYGPUCounterWinEntries(counters).filter_type(L"codec").sum(), //vce
            RGYGPUCounterWinEntries(counters).filter_type(L"encode").sum());
        auto gpu_utilization = std::max(std::max(std::max(
            RGYGPUCounterWinEntries(counters).filter_type(L"cuda").sum(), //nvenc
            RGYGPUCounterWinEntries(counters).filter_type(L"compute").sum()), //vce-opencl
            RGYGPUCounterWinEntries(counters).filter_type(L"3d").sum()), //qsv
            RGYGPUCounterWinEntries(counters).filter_type(L"videoprocessing").sum());
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
        param->frameIn = m_pLastFilterParam->frameOut;
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
        m_ssim = std::move(filterSsim);
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
        prm->nBframes = 0;
        if (prm->rateControl == get_cx_value(get_rc_method(prm->codec), _T("VBR"))) {
            prm->rateControl = get_cx_value(get_rc_method(prm->codec), _T("VBR_LAT"));
        }
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

    if (RGY_ERR_NONE != (ret = initConverter(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initFilters(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initEncoder(prm))) {
        return ret;
    }

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

    {
        const auto& threadParam = prm->ctrl.threadParams.get(RGYThreadType::MAIN);
        threadParam.apply(GetCurrentThread());
        PrintMes(RGY_LOG_DEBUG, _T("Set main thread param: %s.\n"), threadParam.desc().c_str());
    }

    return ret;
}

RGY_ERR VCECore::run_decode() {
    m_queueFrameData.init(256);
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

            for (auto& frameData : bitstream.getFrameDataList()) {
                if (frameData->dataType() == RGY_FRAME_DATA_HDR10PLUS) {
                    auto ptr = dynamic_cast<RGYFrameDataHDR10plus*>(frameData);
                    if (ptr) {
                        m_queueFrameData.push(new RGYFrameDataHDR10plus(*ptr));
                    }
                }
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
                case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_P: output.setFrametype(RGY_FRAMETYPE_P); break;
                case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_B: output.setFrametype(RGY_FRAMETYPE_B); break;
                case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_I: output.setFrametype(RGY_FRAMETYPE_I); break;
                case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_IDR:
                default:
                    output.setFrametype(RGY_FRAMETYPE_IDR); break;
                }
            }
            uint32_t value32 = 0;
            if (buffer->GetProperty(AMF_PARAM_STATISTIC_AVERAGE_QP(m_encCodec), &value32) == AMF_OK) {
                output.setAvgQP(value32);
            }
            if (buffer->HasProperty(AMF_VIDEO_ENCODER_HEVC_INPUT_HDR_METADATA)) {
                amf::AMFVariant hdr10MetadataBuffer;
                if (buffer->GetProperty(AMF_VIDEO_ENCODER_HEVC_INPUT_HDR_METADATA, &hdr10MetadataBuffer) == AMF_OK) {
                    hdr10MetadataBuffer.Clear();
                }
            }
            if (m_ssim) {
                if (!m_ssim->decodeStarted()) {
                    m_ssim->initDecode(&output);
                }
                m_ssim->addBitstream(&output);
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

RGY_ERR VCECore::run() {
    m_pStatus->SetStart();
    m_state = RGY_STATE_RUNNING;
    const auto VCE_TIMEBASE = rgy_rational<int>(1, AMF_SECOND);
    const bool vpp_rff = false;
    const bool vpp_afs_rff_aware = false;
    std::map<int, shared_ptr<RGYOutputAvcodec>> pWriterForAudioStreams;
    if (m_pFileWriterListAudio.size()) {
        //streamのindexから必要なwriteへのポインタを返すテーブルを作成
        for (auto pWriter : m_pFileWriterListAudio) {
            auto pAVCodecWriter = std::dynamic_pointer_cast<RGYOutputAvcodec>(pWriter);
            if (pAVCodecWriter) {
                auto trackIdList = pAVCodecWriter->GetStreamTrackIdList();
                for (auto trackID : trackIdList) {
                    pWriterForAudioStreams[trackID] = pAVCodecWriter;
                }
            }
        }
    }

    //streamのtrackIdからパケットを送信するvppフィルタへのポインタを返すテーブルを作成
    std::map<int, RGYFilter *> pFilterForStreams;
    for (uint32_t ifilter = 0; ifilter < m_vpFilters.size(); ifilter++) {
        const auto targetTrackId = m_vpFilters[ifilter]->targetTrackIdx();
        if (targetTrackId != 0) {
            pFilterForStreams[targetTrackId] = m_vpFilters[ifilter].get();
        }
    }
    if (m_pDecoder != nullptr) {
        auto res = run_decode();
        if (res != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("failed to start input threads.\n"), get_err_mes(res));
            return res;
        }
    }
    auto res = run_output();
    if (res != RGY_ERR_NONE) {
        PrintMes(RGY_LOG_ERROR, _T("failed to start output threads.\n"), get_err_mes(res));
        return res;
    }
    if (m_pPerfMonitor) {
        HANDLE thDecode = NULL;
        HANDLE thOutput = NULL;
        HANDLE thInput = NULL;
        HANDLE thAudProc = NULL;
        HANDLE thAudEnc = NULL;
#if !THREAD_DEC_USE_FUTURE
        thDecode = (HANDLE)(m_thDecoder.native_handle());
#endif
        auto pAVCodecReader = std::dynamic_pointer_cast<RGYInputAvcodec>(m_pFileReader);
        if (pAVCodecReader != nullptr) {
            thInput = pAVCodecReader->getThreadHandleInput();
        }
        auto pAVCodecWriter = std::dynamic_pointer_cast<RGYOutputAvcodec>(m_pFileWriter);
        if (pAVCodecWriter != nullptr) {
            thOutput = pAVCodecWriter->getThreadHandleOutput();
            thAudProc = pAVCodecWriter->getThreadHandleAudProcess();
            thAudEnc = pAVCodecWriter->getThreadHandleAudEncode();
        }
        m_pPerfMonitor->SetThreadHandles(thDecode, thInput, thOutput, thAudProc, thAudEnc);
    }

    auto run_send_streams = [this, &pWriterForAudioStreams, &pFilterForStreams](int inputFrames) {
#if ENABLE_SM_READER
        RGYInputSM *pReaderSM = dynamic_cast<RGYInputSM *>(m_pFileReader.get());
        const int droppedInAviutl = (pReaderSM != nullptr) ? pReaderSM->droppedFrames() : 0;
#else
        const int droppedInAviutl = 0;
#endif

        vector<AVPacket*> packetList = m_pFileReader->GetStreamDataPackets(inputFrames + droppedInAviutl);

        //音声ファイルリーダーからのトラックを結合する
        for (const auto &reader : m_AudioReaders) {
            vector_cat(packetList, reader->GetStreamDataPackets(inputFrames + droppedInAviutl));
        }
        //パケットを各Writerに分配する
        for (uint32_t i = 0; i < packetList.size(); i++) {
            const int nTrackId = packetList[i]->flags >> 16;
            const bool sendToFilter = pFilterForStreams.count(nTrackId) > 0;
            const bool sendToWriter = pWriterForAudioStreams.count(nTrackId) > 0;
            AVPacket *pkt = packetList[i];
            if (sendToFilter) {
                AVPacket *pktToFilter = nullptr;
                if (sendToWriter) {
                    pktToFilter = av_packet_clone(pkt);
                } else {
                    std::swap(pktToFilter, pkt);
                }
                auto err = pFilterForStreams[nTrackId]->addStreamPacket(pktToFilter);
                if (err != RGY_ERR_NONE) {
                    return err;
                }
            }
            if (sendToWriter) {
                auto pWriter = pWriterForAudioStreams[nTrackId];
                if (pWriter == nullptr) {
                    PrintMes(RGY_LOG_ERROR, _T("Invalid writer found for %s track #%d\n"), char_to_tstring(trackMediaTypeStr(nTrackId)).c_str(), trackID(nTrackId));
                    return RGY_ERR_NOT_FOUND;
                }
                auto err = pWriter->WriteNextPacket(pkt);
                if (err != RGY_ERR_NONE) {
                    return err;
                }
                pkt = nullptr;
            }
            if (pkt != nullptr) {
                PrintMes(RGY_LOG_ERROR, _T("Failed to find writer for %s track #%d\n"), char_to_tstring(trackMediaTypeStr(nTrackId)).c_str(), trackID(nTrackId));
                return RGY_ERR_NOT_FOUND;
            }
        }
        return RGY_ERR_NONE;
    };

    auto getHDR10plusMetadata = [this](int64_t timestamp) {
        std::shared_ptr<RGYFrameData> frameData;
        RGYFrameData *frameDataPtr = nullptr;
        while (m_queueFrameData.front_copy_no_lock(&frameDataPtr)) {
            auto ptr = dynamic_cast<RGYFrameDataHDR10plus*>(frameDataPtr);
            if (ptr && ptr->timestamp() < timestamp) {
                m_queueFrameData.pop();
                delete frameDataPtr;
            } else {
                break;
            }
        }
        size_t queueSize = m_queueFrameData.size();
        for (uint32_t i = 0; i < queueSize; i++) {
            if (m_queueFrameData.copy(&frameDataPtr, i, &queueSize)) {
                auto ptr = dynamic_cast<RGYFrameDataHDR10plus*>(frameDataPtr);
                if (ptr && ptr->timestamp() == timestamp) {
                    frameData = std::make_shared<RGYFrameDataHDR10plus>(*ptr);
                    break;
                }
            }
        }
        return frameData;
    };

    const AVStream *pStreamIn = nullptr;
    RGYInputAvcodec *pReader = dynamic_cast<RGYInputAvcodec *>(m_pFileReader.get());
    if (pReader != nullptr) {
        pStreamIn = pReader->GetInputVideoStream();
    }
    const auto srcTimebase = (pStreamIn) ? to_rgy(pStreamIn->time_base) : m_pFileReader->getInputTimebase();;

    int64_t outFirstPts = AV_NOPTS_VALUE; //入力のptsに対する補正 (スケール: m_outputTimebase)
    int64_t lastTrimFramePts = AV_NOPTS_VALUE; //直前のtrimで落とされたフレームのpts, trimで落とされてない場合はAV_NOPTS_VALUE (スケール: m_outputTimebase)
    int64_t outEstimatedPts = 0; //固定fpsを仮定した時のfps (スケール: m_outputTimebase)
    const int64_t outFrameDuration = std::max<int64_t>(1, rational_rescale(1, m_inputFps.inv(), m_outputTimebase));
    uint32_t inputFramePosIdx = UINT32_MAX;
    auto check_pts = [&](unique_ptr<RGYFrame>& inFrame) {
        vector<unique_ptr<RGYFrame>> outFrames;
        int64_t outPtsSource = outEstimatedPts;
        int64_t outDuration = outFrameDuration; //入力fpsに従ったduration
#if ENABLE_AVSW_READER
        if ((srcTimebase.n() > 0 && srcTimebase.is_valid())
            && ((m_nAVSyncMode & (RGY_AVSYNC_VFR | RGY_AVSYNC_FORCE_CFR)) || vpp_rff || vpp_afs_rff_aware)) {
            //CFR仮定ではなく、オリジナルの時間を見る
            outPtsSource = rational_rescale(inFrame->timestamp(), srcTimebase, m_outputTimebase);
        }
        if (outFirstPts == AV_NOPTS_VALUE) {
            outFirstPts = outPtsSource; //最初のpts
        }
        //最初のptsを0に修正
        outPtsSource -= outFirstPts;

        if ((m_nAVSyncMode & RGY_AVSYNC_VFR) || vpp_rff || vpp_afs_rff_aware) {
            if (vpp_rff || vpp_afs_rff_aware) {
                if (std::abs(outPtsSource - outEstimatedPts) >= 32 * outFrameDuration) {
                    //timestampに一定以上の差があればそれを無視する
                    outFirstPts += (outPtsSource - outEstimatedPts); //今後の位置合わせのための補正
                    outPtsSource = outEstimatedPts;
                }
                auto ptsDiff = outPtsSource - outEstimatedPts;
                if (ptsDiff <= std::min<int64_t>(-1, -1 * outFrameDuration * 7 / 8)) {
                    //間引きが必要
                    return outFrames;
                }
            }
            if (pStreamIn) {
                //cuvidデコード時は、timebaseの分子はかならず1なので、pStreamIn->time_baseとズレているかもしれないのでオリジナルを計算
                const auto orig_pts = rational_rescale(inFrame->timestamp(), srcTimebase, to_rgy(pStreamIn->time_base));
                //ptsからフレーム情報を取得する
                const auto framePos = pReader->GetFramePosList()->findpts(orig_pts, &inputFramePosIdx);
                if (framePos.poc != FRAMEPOS_POC_INVALID && framePos.duration > 0) {
                    //有効な値ならオリジナルのdurationを使用する
                    outDuration = rational_rescale(framePos.duration, to_rgy(pStreamIn->time_base), m_outputTimebase);
                }
            }
        }
        if (m_nAVSyncMode & RGY_AVSYNC_FORCE_CFR) {
            if (std::abs(outPtsSource - outEstimatedPts) >= CHECK_PTS_MAX_INSERT_FRAMES * outFrameDuration) {
                //timestampに一定以上の差があればそれを無視する
                outFirstPts += (outPtsSource - outEstimatedPts); //今後の位置合わせのための補正
                outPtsSource = outEstimatedPts;
                PrintMes(RGY_LOG_WARN, _T("Big Gap was found between 2 frames, avsync might be corrupted.\n"));
            }
            auto ptsDiff = outPtsSource - outEstimatedPts;
            if (ptsDiff <= std::min<int64_t>(-1, -1 * outFrameDuration * 7 / 8)) {
                //間引きが必要
                return outFrames;
            }
            while (ptsDiff >= std::max<int64_t>(1, outFrameDuration * 7 / 8)) {
                //水増しが必要
                auto newFrame = inFrame->createCopy();
                newFrame->setTimestamp(outPtsSource);
                newFrame->setDuration(outDuration);
                outFrames.push_back(std::move(newFrame));
                outEstimatedPts += outFrameDuration;
                ptsDiff = outPtsSource - outEstimatedPts;
            }
            outPtsSource = outEstimatedPts;
        }
#endif //#if ENABLE_AVSW_READER
        //次のフレームのptsの予想
        outEstimatedPts += outDuration;
        inFrame->setTimestamp(outPtsSource);
        inFrame->setDuration(outDuration);
        outFrames.push_back(std::move(inFrame));
        return outFrames;
    };

    auto filter_frame = [&](unique_ptr<RGYFrame> &inframe, deque<unique_ptr<RGYFrame>> &dqEncFrames, bool &bDrain) {

        deque<std::pair<RGYFrameInfo, uint32_t>> filterframes;

        bool skipFilters = false;
        if (bDrain) {
            filterframes.push_back(std::make_pair(RGYFrameInfo(), 0u));
        } else {
            const auto inframeInfo = inframe->getInfo();
            if (m_vpFilters.size() == 0) {
                skipFilters = true;
            } else {
                auto &lastFilter = m_vpFilters[m_vpFilters.size() - 1];
                if (typeid(*lastFilter.get()) == typeid(RGYFilterCspCrop)
                    && m_vpFilters.size() == 1
                    && lastFilter->GetFilterParam()->frameOut.csp == inframeInfo.csp
                    && m_encWidth == inframeInfo.width
                    && m_encHeight == inframeInfo.height
                    && !m_ssim) {
                    skipFilters = true;
                }
            }
            const auto& inAmf = inframe->amf();
            if (!skipFilters
                && inframeInfo.mem_type != RGY_MEM_TYPE_CPU
                && inAmf
                && inAmf->GetMemoryType() != amf::AMF_MEMORY_OPENCL) {
                amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
#if 0
                auto ar = inAmf->Interop(amf::AMF_MEMORY_OPENCL);
#else
#if 1
                //dummyのCPUへのメモリコピーを行う
                //こうしないとデコーダからの出力をOpenCLに渡したときに、フレームが壊れる(フレーム順序が入れ替わってガクガクする)
                amf::AMFDataPtr data;
                inAmf->Duplicate(amf::AMF_MEMORY_HOST, &data);
#else
                auto frameinfo = inframe->info();
                amf::AMFSurfacePtr pSurface;
                auto ar = m_dev->context()->AllocSurface(amf::AMF_MEMORY_HOST, csp_rgy_to_enc(frameinfo.csp),
                    16, 16, &pSurface);
                auto ar2 = inAmf->CopySurfaceRegion(pSurface, 0, 0, 0, 0, 16, 16);
#endif
                auto ar = inAmf->Convert(amf::AMF_MEMORY_OPENCL);
#endif
                if (ar != AMF_OK) {
                    PrintMes(RGY_LOG_ERROR, _T("Failed to convert plane: %s.\n"), get_err_mes(err_to_rgy(ar)));
                    return err_to_rgy(ar);
                }
            }
            filterframes.push_back(std::make_pair(inframe->getInfo(), 0u));
        }

        while (filterframes.size() > 0 || bDrain) {
            //フィルタリングするならここ
            for (int ifilter = filterframes.front().second; ifilter < (int)m_vpFilters.size() - 1; ifilter++) {
                amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
                int nOutFrames = 0;
                RGYFrameInfo *outInfo[16] = { 0 };
                auto sts_filter = m_vpFilters[ifilter]->filter(&filterframes.front().first, (RGYFrameInfo **)&outInfo, &nOutFrames);
                if (sts_filter != RGY_ERR_NONE) {
                    PrintMes(RGY_LOG_ERROR, _T("Error while running filter \"%s\".\n"), m_vpFilters[ifilter]->name().c_str());
                    return sts_filter;
                }
                if (nOutFrames == 0) {
                    if (bDrain) {
                        filterframes.front().second++;
                        continue;
                    }
                    return RGY_ERR_NONE;
                }
                filterframes.pop_front();
                bDrain = false; //途中でフレームが出てきたら、drain完了していない

                //最初に出てきたフレームは先頭に追加する
                for (int jframe = nOutFrames-1; jframe >= 0; jframe--) {
                    filterframes.push_front(std::make_pair(*outInfo[jframe], ifilter+1));
                }
            }
            if (bDrain) {
                return RGY_ERR_NONE; //最後までbDrain = trueなら、drain完了
            }
            if (skipFilters) {
                dqEncFrames.push_back(std::move(inframe));
                filterframes.pop_front();
            } else {
                //エンコードバッファにコピー
                amf::AMFContext::AMFOpenCLLocker locker(m_dev->context());
                auto &lastFilter = m_vpFilters[m_vpFilters.size()-1];
                amf::AMFSurfacePtr pSurface;
                if (m_dev->dx11interlop()) {
                    auto ar = m_dev->context()->AllocSurface(amf::AMF_MEMORY_DX11, csp_rgy_to_enc(lastFilter->GetFilterParam()->frameOut.csp),
                        m_encWidth, m_encHeight, &pSurface);
                    if (ar != AMF_OK) {
                        PrintMes(RGY_LOG_ERROR, _T("Failed to allocate surface: %s.\n"), get_err_mes(err_to_rgy(ar)));
                        return err_to_rgy(ar);
                    }
                    ar = pSurface->Interop(amf::AMF_MEMORY_OPENCL);
                    if (ar != AMF_OK) {
                        PrintMes(RGY_LOG_ERROR, _T("Failed to get interop of surface: %s.\n"), get_err_mes(err_to_rgy(ar)));
                        return err_to_rgy(ar);
                    }
                } else {
                    auto ar = m_dev->context()->AllocSurface(amf::AMF_MEMORY_OPENCL, csp_rgy_to_enc(lastFilter->GetFilterParam()->frameOut.csp),
                        m_encWidth, m_encHeight, &pSurface);
                    if (ar != AMF_OK) {
                        PrintMes(RGY_LOG_ERROR, _T("Failed to allocate surface: %s.\n"), get_err_mes(err_to_rgy(ar)));
                        return err_to_rgy(ar);
                    }
                }
                auto encSurface = std::make_unique<RGYFrame>(pSurface);
                //最後のフィルタはRGYFilterCspCropでなければならない
                if (typeid(*lastFilter.get()) != typeid(RGYFilterCspCrop)) {
                    PrintMes(RGY_LOG_ERROR, _T("Last filter setting invalid.\n"));
                    return RGY_ERR_INVALID_PARAM;
                }
                //エンコードバッファのポインタを渡す
                int nOutFrames = 0;
                auto encSurfaceInfo = encSurface->getInfo();
                RGYFrameInfo *outInfo[1];
                outInfo[0] = &encSurfaceInfo;
                auto sts_filter = lastFilter->filter(&filterframes.front().first, (RGYFrameInfo **)&outInfo, &nOutFrames);
                filterframes.pop_front();
                if (sts_filter != RGY_ERR_NONE) {
                    PrintMes(RGY_LOG_ERROR, _T("Error while running filter \"%s\".\n"), lastFilter->name().c_str());
                    return sts_filter;
                }
                if (m_ssim) {
                    int dummy = 0;
                    m_ssim->filter(&encSurfaceInfo, nullptr, &dummy);
                }
                auto err = m_dev->cl()->queue().finish();
                if (err != RGY_ERR_NONE) {
                    PrintMes(RGY_LOG_ERROR, _T("Failed to finish queue after \"%s\".\n"), lastFilter->name().c_str());
                    return sts_filter;
                }
                encSurface->setDuration(encSurfaceInfo.duration);
                encSurface->setTimestamp(encSurfaceInfo.timestamp);
                encSurface->setPicstruct(encSurfaceInfo.picstruct);
                encSurface->setInputFrameId(encSurfaceInfo.inputFrameId);
                encSurface->setFlags(encSurfaceInfo.flags);
                encSurface->setDataList(encSurfaceInfo.dataList, m_dev->context());
                dqEncFrames.push_back(std::move(encSurface));
            }
        }
        return RGY_ERR_NONE;
    };

    auto send_encoder = [this](unique_ptr<RGYFrame>& encFrame) {
        const auto inputFrameId = encFrame->inputFrameId();
        if (inputFrameId < 0) {
            PrintMes(RGY_LOG_ERROR, _T("Invalid input frame ID %d sent to encoder.\n"), inputFrameId);
            return RGY_ERR_INVALID_PARAM;
        }
        int64_t pts = encFrame->timestamp();
        int64_t duration = encFrame->duration();
        const auto frameDataList = encFrame->dataList();
        amf::AMFSurfacePtr pSurface = encFrame->detachSurface();
        //現状VCEはインタレをサポートしないので、強制的にプログレとして処理する
        pSurface->SetFrameType(amf::AMF_FRAME_PROGRESSIVE);
        //現状VCEはインタレをサポートしないので、強制的にプログレとして処理する
        //フレーム情報のほうもプログレに書き換えなければ、SubmitInputでエラーが返る
        m_params.SetParam(AMF_VIDEO_ENCODER_PICTURE_STRUCTURE, AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_TOP_FIELD);
        // apply frame-specific properties to the current frame
        m_params.Apply(pSurface, AMF_PARAM_FRAME, m_pLog.get());
        // apply dynamic properties to the encoder
        //m_params.Apply(m_pEncoder, AMF_PARAM_DYNAMIC, m_pLog.get());

        pSurface->SetProperty(RGY_PROP_TIMESTAMP, pts);
        pSurface->SetProperty(RGY_PROP_DURATION, duration);

        pSurface->SetProperty(AMF_PARAM_STATISTICS_FEEDBACK(m_encCodec), true);

        if (m_encCodec == RGY_CODEC_HEVC) {
            std::vector<uint8_t> dhdr10plus_sei;
            if (m_hdr10plus) {
                const auto data = m_hdr10plus->getData(inputFrameId);
                if (data && data->size() > 0) {
                    dhdr10plus_sei = *data;
                }
            } else if (frameDataList.size() > 0) {
                auto data = std::find_if(frameDataList.begin(), frameDataList.end(), [](const std::shared_ptr<RGYFrameData>& frameData) {
                    return frameData->dataType() == RGY_FRAME_DATA_HDR10PLUS;
                });
                if (data != frameDataList.end()) {
                    auto hdr10plus = dynamic_cast<RGYFrameDataHDR10plus *>(data->get());
                    if (hdr10plus && hdr10plus->getData().size() > 0) {
                        dhdr10plus_sei = hdr10plus->getData();
                    }
                }
            }
            if (dhdr10plus_sei.size() > 0) {
                amf::AMFBufferPtr hdr10MetadataBuffer;
                auto ar = m_dev->context()->AllocBuffer(amf::AMF_MEMORY_HOST, dhdr10plus_sei.size(), &hdr10MetadataBuffer);
                if (ar != AMF_OK) {
                    PrintMes(RGY_LOG_ERROR, _T("Failed to allocate buffer for sending hdr10plus metadata to encoder.\n"));
                    return err_to_rgy(ar);
                }
                memcpy(hdr10MetadataBuffer->GetNative(), dhdr10plus_sei.data(), dhdr10plus_sei.size());
                if ((ar = pSurface->SetProperty(AMF_VIDEO_ENCODER_HEVC_INPUT_HDR_METADATA, hdr10MetadataBuffer)) != AMF_OK) {
                    PrintMes(RGY_LOG_ERROR, _T("Failed to set hdr10plus metadata to encoder.\n"));
                    return err_to_rgy(ar);
                }
            }
        }

        if (m_timecode) {
            m_timecode->write(pts, m_outputTimebase);
        }

        auto ar = AMF_OK;
        do {
            try {
                ar = m_pEncoder->SubmitInput(pSurface);
            } catch (...) {
                PrintMes(RGY_LOG_ERROR, _T("Fatal error when submitting frame to encoder.\n"));
                return RGY_ERR_UNKNOWN;
            }
            if (ar == AMF_NEED_MORE_INPUT) {
                break;
            } else if (ar == AMF_INPUT_FULL) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            } else if (ar == AMF_REPEAT) {
                pSurface = nullptr;
            } else if (m_thOutput.wait_for(std::chrono::microseconds(0)) != std::future_status::timeout) {
                PrintMes(RGY_LOG_ERROR, _T("Error during output.\n"));
                m_state = RGY_STATE_ERROR;
                break;
            } else {
                break;
            }
        } while (m_state == RGY_STATE_RUNNING);
        return err_to_rgy(ar);
    };

    const auto inputFrameInfo = m_pFileReader->GetInputFrameInfo();
    CProcSpeedControl speedCtrl(m_nProcSpeedLimit);
    bool bInputEmpty = false;
    bool bFilterEmpty = false;
    int nInputFrame = 0;
    deque<unique_ptr<RGYFrame>> dqInFrames;
    deque<unique_ptr<RGYFrame>> dqEncFrames;
    for (; m_state == RGY_STATE_RUNNING && !bInputEmpty && !bFilterEmpty; ) {
        if ((m_pAbortByUser && *m_pAbortByUser) || stdInAbort()) {
            m_state = RGY_STATE_ABORT;
            break;
        }
        speedCtrl.wait();
        if ((res = run_send_streams(nInputFrame)) != RGY_ERR_NONE) {
            m_state = RGY_STATE_ERROR;
            break;
        }
        unique_ptr<RGYFrame> inputFrame;
        if (m_pDecoder == nullptr) {
            amf::AMFSurfacePtr pSurface;
            auto ar = m_dev->context()->AllocSurface(amf::AMF_MEMORY_HOST, csp_rgy_to_enc(inputFrameInfo.csp),
                inputFrameInfo.srcWidth - inputFrameInfo.crop.e.left - inputFrameInfo.crop.e.right,
                inputFrameInfo.srcHeight - inputFrameInfo.crop.e.bottom - inputFrameInfo.crop.e.up,
                &pSurface);
            if (ar != AMF_OK) {
                res = err_to_rgy(ar); m_state = RGY_STATE_ERROR;
                break;
            }
            pSurface->SetFrameType(frametype_rgy_to_enc(inputFrameInfo.picstruct));
            inputFrame = std::make_unique<RGYFrame>(pSurface);
            res = m_pFileReader->LoadNextFrame(inputFrame.get());
            if (res == RGY_ERR_MORE_DATA) {
                bInputEmpty = true;
                inputFrame.reset();
            } else if (res == RGY_ERR_NONE) {
                auto pAVCodecReader = std::dynamic_pointer_cast<RGYInputAvcodec>(m_pFileReader);
                if (pAVCodecReader != nullptr) {
                    const auto vid_timebase = to_rgy(pAVCodecReader->GetInputVideoStream()->time_base);
                    inputFrame->setDuration(rgy_change_scale(inputFrame->duration(), vid_timebase, srcTimebase));
                    inputFrame->setTimestamp(rgy_change_scale(inputFrame->timestamp(), vid_timebase, srcTimebase));
                }
            } else {
                PrintMes(RGY_LOG_ERROR, _T("Failed to load input frame.\n"));
                m_state = RGY_STATE_ERROR;
                break;
            }
            if (m_thOutput.wait_for(std::chrono::microseconds(0)) != std::future_status::timeout) {
                PrintMes(RGY_LOG_ERROR, _T("Error during output.\n"));
                m_state = RGY_STATE_ERROR;
                break;
            }
        } else {
            amf::AMFSurfacePtr surf;
            auto ar = AMF_REPEAT;
            auto timeS = std::chrono::system_clock::now();
            while (m_state == RGY_STATE_RUNNING) {
                amf::AMFDataPtr data;
                ar = m_pDecoder->QueryOutput(&data);
                if (ar == AMF_EOF) {
                    break;
                }
                if (ar == AMF_REPEAT) {
                    ar = AMF_OK; //これ重要...ここが欠けると最後の数フレームが欠落する
                }
                if (ar == AMF_OK && data != nullptr) {
                    surf = amf::AMFSurfacePtr(data);
                    break;
                }
                if (ar != AMF_OK) break;
                if ((std::chrono::system_clock::now() - timeS) > std::chrono::seconds(10)) {
                    PrintMes(RGY_LOG_ERROR, _T("10 sec has passed after getting last frame from decoder.\n"));
                    PrintMes(RGY_LOG_ERROR, _T("Decoder seems to have crushed.\n"));
                    ar = AMF_FAIL;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                if (m_thOutput.wait_for(std::chrono::microseconds(0)) != std::future_status::timeout) {
                    PrintMes(RGY_LOG_ERROR, _T("Error during output.\n"));
                    m_state = RGY_STATE_ERROR;
                    break;
                }
                if ((res = run_send_streams(nInputFrame)) != RGY_ERR_NONE) {
                    m_state = RGY_STATE_ERROR;
                    break;
                }
            }
            if (ar == AMF_EOF) {
                bInputEmpty = true;
            } else if (ar != AMF_OK) {
                res = err_to_rgy(ar); m_state = RGY_STATE_ERROR;
                PrintMes(RGY_LOG_ERROR, _T("Failed to load input frame.\n"));
                break;
            }
            inputFrame = std::make_unique<RGYFrame>(surf);
            if (auto frameData = getHDR10plusMetadata(inputFrame->timestamp()); frameData) {
                inputFrame->setData(frameData, m_dev->context());
            }
        }
        if (!bInputEmpty) {
            inputFrame->setInputFrameId(nInputFrame);
            //trim反映
            const auto trimSts = frame_inside_range(nInputFrame++, m_trimParam.list);
#if ENABLE_AVSW_READER
            const auto inputFramePts = rational_rescale(inputFrame->timestamp(), srcTimebase, m_outputTimebase);
            if (((m_nAVSyncMode & RGY_AVSYNC_VFR) || vpp_rff || vpp_afs_rff_aware)
                && (trimSts.second > 0) //check_pts内で最初のフレームのptsを0とするようnOutFirstPtsが設定されるので、先頭のtrim blockについてはここでは処理しない
                && (lastTrimFramePts != AV_NOPTS_VALUE)) { //前のフレームがtrimで脱落させたフレームなら
                outFirstPts += inputFramePts - lastTrimFramePts; //trimで脱落させたフレームの分の時間を加算
            }
            if (!trimSts.first) {
                lastTrimFramePts = inputFramePts; //脱落させたフレームの時間を記憶
            }
#endif
            if (!trimSts.first) {
                continue; //trimにより脱落させるフレーム
            }
            lastTrimFramePts = AV_NOPTS_VALUE;
            auto decFrames = check_pts(inputFrame);

            for (auto idf = decFrames.begin(); idf != decFrames.end(); idf++) {
                dqInFrames.push_back(std::move(*idf));
            }
        }
        while (((dqInFrames.size() || bInputEmpty) && !bFilterEmpty)) {
            const bool bDrain = (dqInFrames.size()) ? false : bInputEmpty;
            std::unique_ptr<RGYFrame> inframe;
            if (dqInFrames.size()) {
                inframe = std::move(dqInFrames.front());
            }
            bool bDrainFin = bDrain;
            RGY_ERR err = filter_frame(inframe, dqEncFrames, bDrainFin);
            if (err != RGY_ERR_NONE) {
                res = err;
                m_state = RGY_STATE_ERROR;
                PrintMes(RGY_LOG_ERROR, _T("Failed to filter frame.\n"));
                break;
            }
            bFilterEmpty = bDrainFin;
            if (!bDrain) {
                dqInFrames.pop_front();
            }
            while ((int)dqEncFrames.size() >= m_pipelineDepth) {
                auto &encframe = dqEncFrames.front();
                if ((err = send_encoder(encframe)) != RGY_ERR_NONE) {
                    res = err;
                    m_state = RGY_STATE_ERROR;
                    PrintMes(RGY_LOG_ERROR, _T("Failed to send frame to encoder.\n"));
                    break;
                }
                dqEncFrames.pop_front();
            }
        }
    }
    while (dqEncFrames.size() > 0) {
        auto &encframe = dqEncFrames.front();
        RGY_ERR err = send_encoder(encframe);
        if (err != RGY_ERR_NONE) {
            res = err;
            m_state = RGY_STATE_ERROR;
            PrintMes(RGY_LOG_ERROR, _T("Failed to send frame to encoder.\n"));
            break;
        }
        dqEncFrames.pop_front();
    }
#if THREAD_DEC_USE_FUTURE
    if (m_thDecoder.valid()) {
        while (m_thDecoder.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
#else
    if (m_thDecoder.joinable()) {
        while (RGYThreadStillActive(m_thDecoder.native_handle())) {
#endif
            if ((res = run_send_streams(nInputFrame)) != RGY_ERR_NONE) {
                m_state = RGY_STATE_ERROR;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
#if !THREAD_DEC_USE_FUTURE
        // linuxでは、これがRGYThreadStillActiveのwhile文を抜けるときに行われるため、
        // これを呼ぶとエラーになってしまう
        m_thDecoder.join();
#endif
    }
    auto ar = AMF_INPUT_FULL;
    while (ar == AMF_INPUT_FULL) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ar = m_pEncoder->Drain();
        // 出力スレッドが生存していないと、いつまでもこのループを抜けることはない
        // 出力スレッドが生存していない場合はあきらめる
        if (m_thOutput.valid()) {
            auto thsts = m_thOutput.wait_for(std::chrono::seconds(0));
            if (thsts == std::future_status::ready) {
                break;
            }
        }
    }
    if (ar != AMF_OK) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to drain encoder: %s\n"), get_err_mes(err_to_rgy(ar)));
        return err_to_rgy(ar);
    }
    PrintMes(RGY_LOG_DEBUG, _T("Flushed Encoder\n"));
    if (m_thOutput.valid()) {
        PrintMes(RGY_LOG_DEBUG, _T("Waiting for ouput thread to be finieshed...\n"));
        m_thOutput.get();
        PrintMes(RGY_LOG_DEBUG, _T("Closed output thread.\n"));
    }
    if (m_ssim) {
        PrintMes(RGY_LOG_DEBUG, _T("Flushing ssim/psnr calc.\n"));
        m_ssim->addBitstream(nullptr);
    }
    for (const auto &writer : m_pFileWriterListAudio) {
        auto pAVCodecWriter = std::dynamic_pointer_cast<RGYOutputAvcodec>(writer);
        if (pAVCodecWriter != nullptr) {
            //エンコーダなどにキャッシュされたパケットを書き出す
            pAVCodecWriter->WriteNextPacket(nullptr);
        }
    }
    m_pFileWriter->Close();
    m_pFileReader->Close();
    m_pStatus->WriteResults();
    if (m_ssim) {
        m_ssim->showResult();
    }

    //vpp-perf-monitor
    std::vector<std::pair<tstring, double>> filter_result;
    for (auto& filter : m_vpFilters) {
        auto avgtime = filter->GetAvgTimeElapsed();
        if (avgtime > 0.0) {
            filter_result.push_back({ filter->name(), avgtime });
        }
    }
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
    return (m_state == RGY_STATE_ERROR) ? res : RGY_ERR_NONE;
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

    auto getPropertyDesc = [pProperty, GetPropertyInt](const wchar_t *pName, const CX_DESC *list) {
        int64_t value = 0;
        if (pProperty->GetProperty(pName, &value) != AMF_OK) {
            return tstring(_T(""));
        }
        auto ptr = get_cx_desc(list, (int)value);
        return (ptr) ? tstring(ptr) : _T("");
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
    mes += strsprintf(_T("GPU:           %s, AMF Runtime %d.%d.%d / SDK %d.%d.%d\n"), gpu_info.c_str(),
        (int)AMF_GET_MAJOR_VERSION(m_AMFRuntimeVersion), (int)AMF_GET_MINOR_VERSION(m_AMFRuntimeVersion), (int)AMF_GET_SUBMINOR_VERSION(m_AMFRuntimeVersion),
        AMF_VERSION_MAJOR, AMF_VERSION_MINOR, AMF_VERSION_RELEASE);

    auto inputInfo = m_pFileReader->GetInputFrameInfo();
    mes += strsprintf(_T("Input Info:    %s\n"), m_pFileReader->GetInputMessage());
    if (cropEnabled(inputInfo.crop)) {
        mes += strsprintf(_T("Crop:          %d,%d,%d,%d\n"), inputInfo.crop.e.left, inputInfo.crop.e.up, inputInfo.crop.e.right, inputInfo.crop.e.bottom);
    }
    tstring vppFilterMes;
    for (const auto &filter : m_vpFilters) {
        vppFilterMes += strsprintf(_T("%s%s\n"), (vppFilterMes.length()) ? _T("               ") : _T("Vpp Filters    "), filter->GetInputMessage().c_str());
    }
    if (m_ssim) {
        vppFilterMes += _T("               ") + m_ssim->GetInputMessage() + _T("\n");
    }
    mes += vppFilterMes;
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
        mes += strsprintf(_T("CQP:           I:%d, P:%d"),
            GetPropertyInt(AMF_PARAM_QP_I(m_encCodec)),
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
        if (GetPropertyInt(AMF_PARAM_RATE_CONTROL_METHOD(m_encCodec)) == AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_QUALITY_VBR) {
            mes += strsprintf(_T("QVBR level:    %d\n"), GetPropertyInt(AMF_VIDEO_ENCODER_QVBR_QUALITY_LEVEL));
        }
        mes += strsprintf(_T("Max bitrate:   %d kbps\n"), GetPropertyInt(AMF_PARAM_PEAK_BITRATE(m_encCodec)) / 1000);
        mes += strsprintf(_T("QP:            Min: %d, Max: %d\n"),
            GetPropertyInt(AMF_PARAM_MIN_QP(m_encCodec)),
            GetPropertyInt(AMF_PARAM_MAX_QP(m_encCodec)));
    }
    mes += strsprintf(_T("VBV Bufsize:   %d kbps\n"), GetPropertyInt(AMF_PARAM_VBV_BUFFER_SIZE(m_encCodec)) / 1000);
    if (m_encCodec == RGY_CODEC_H264 && GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN)) {
        mes += strsprintf(_T("Bframes:       %d frames, b-pyramid: %s\n"),
            GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN),
            (GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN) && GetPropertyInt(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE) ? _T("on") : _T("off")));
        if (GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN)) {
            mes += strsprintf(_T("Delta QP:      Bframe: %d, RefBframe: %d\n"), GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP), GetPropertyInt(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP));
        }
    } else {
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
        pa_str += _T("activity ") + getPropertyDesc(AMF_PA_ACTIVITY_TYPE, list_pa_activity) + _T(", ");
        pa_str += _T("caq ") + getPropertyDesc(AMF_PA_CAQ_STRENGTH, list_pa_caq_strength) + _T(", ");
        mes += pa_str.substr(0, pa_str.length()-2) + _T("\n");
    } else {
        mes += _T("off\n");
    }
    mes += strsprintf(_T("Ref frames:    %d frames\n"), GetPropertyInt(AMF_PARAM_MAX_NUM_REFRAMES(m_encCodec)));
    mes += strsprintf(_T("LTR frames:    %d frames\n"), GetPropertyInt(AMF_PARAM_MAX_LTR_FRAMES(m_encCodec)));
    mes += strsprintf(_T("Motion Est:    %s\n"), get_cx_desc(list_mv_presicion, nMotionEst));
    mes += strsprintf(_T("Slices:        %d\n"), GetPropertyInt(AMF_PARAM_SLICES_PER_FRAME(m_encCodec)));
    mes += strsprintf(_T("GOP Len:       %d frames\n"), GetPropertyInt(AMF_PARAM_GOP_SIZE(m_encCodec)));
    { const auto &vui_str = m_encVUI.print_all();
    if (vui_str.length() > 0) {
        mes += strsprintf(_T("VUI:              %s\n"), vui_str.c_str());
    }
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
    if (m_hdr10plus) {
        mes += strsprintf(_T("Dynamic HDR10:     %s\n"), m_hdr10plus->inputJson().c_str());
    } else if (m_hdr10plusCopy) {
        mes += strsprintf(_T("Dynamic HDR10:     copy\n"), m_hdr10plus->inputJson().c_str());
    }
    tstring others;
    if (GetPropertyBool(AMF_PARAM_RATE_CONTROL_SKIP_FRAME_ENABLE(m_encCodec))) {
        others += _T("skip_frame ");
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
    if (m_pLog->getLogLevel(RGY_LOGT_CORE) <= RGY_LOG_DEBUG) {
        if (GetPropertyBool(AMF_PARAM_INSERT_AUD(m_encCodec))) {
            others += _T("aud ");
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
        }
    }
    if (GetPropertyBool(AMF_PARAM_LOWLATENCY_MODE(m_encCodec))) {
        others += _T("lowlatency ");
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
    if (GetPropertyBool(AMF_PARAM_ENABLE_VBAQ(m_encCodec))) {
        others += _T("vbaq ");
    }
    if (others.length() > 0) {
        mes += strsprintf(_T("Others:        %s\n"), others.c_str());
    }
    return mes;
}

void VCECore::PrintResult() {
    m_pStatus->WriteResults();
}

RGY_ERR VCEFeatures::init(int deviceId, RGYLogLevel logLevel) {
    m_core = std::make_unique<VCECore>();
    auto err = RGY_ERR_NONE;
    if (   (err = m_core->initLog(logLevel)) != RGY_ERR_NONE
        || (err = m_core->initAMFFactory()) != RGY_ERR_NONE
        || (err = m_core->initTracer(logLevel)) != RGY_ERR_NONE) {
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

bool check_if_vce_available(int deviceId, RGYLogLevel logLevel) {
    VCEFeatures vce;
    return vce.init(deviceId, logLevel) == RGY_ERR_NONE;
}

tstring check_vce_enc_features(const std::vector<RGY_CODEC> &codecs, int deviceId, RGYLogLevel logLevel) {
    VCEFeatures vce;
    if (vce.init(deviceId, logLevel) != RGY_ERR_NONE) {
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

tstring check_vce_dec_features(int deviceId, RGYLogLevel logLevel) {
    VCEFeatures vce;
    if (vce.init(deviceId, logLevel) != RGY_ERR_NONE) {
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
