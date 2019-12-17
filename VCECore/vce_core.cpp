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

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <cmath>
#include <tchar.h>
#include <process.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include <iostream>

#include "rgy_util.h"
#include "rgy_input.h"
#include "rgy_input_avi.h"
#include "rgy_input_avs.h"
#include "rgy_input_raw.h"
#include "rgy_input_vpy.h"
#include "rgy_input_avcodec.h"
#include "rgy_output.h"
#include "rgy_output_avcodec.h"
#include "vce_core.h"
#include "vce_param.h"
#include "vce_filter.h"
#include "vce_filter_afs.h"
#include "rgy_version.h"
#include "rgy_bitstream.h"
#include "chapter_rw.h"
#include "cpu_info.h"
#include "gpu_info.h"

#include "VideoEncoderVCE.h"
#include "VideoEncoderHEVC.h"
#include "VideoDecoderUVD.h"
#include "VideoConverter.h"
#include "Factory.h"

#include "h264_level.h"
#include "hevc_level.h"

static const amf::AMF_SURFACE_FORMAT formatOut = amf::AMF_SURFACE_NV12;

tstring VCECore::AccelTypeToString(amf::AMF_ACCELERATION_TYPE accelType) {
    tstring strValue;
    switch (accelType) {
    case amf::AMF_ACCEL_NOT_SUPPORTED:
        strValue = _T("Not supported");
        break;
    case amf::AMF_ACCEL_HARDWARE:
        strValue = _T("Hardware-accelerated");
        break;
    case amf::AMF_ACCEL_GPU:
        strValue = _T("GPU-accelerated");
        break;
    case amf::AMF_ACCEL_SOFTWARE:
        strValue = _T("Not accelerated (software)");
        break;
    }
    return strValue;
}

void VCECore::PrintMes(int log_level, const TCHAR *format, ...) {
    if (m_pLog.get() == nullptr || log_level < m_pLog->getLogLevel()) {
        return;
    }

    va_list args;
    va_start(args, format);

    int len = _vsctprintf(format, args) + 1; // _vscprintf doesn't count terminating '\0'
    vector<TCHAR> buffer(len, 0);
    _vstprintf_s(buffer.data(), len, format, args);
    va_end(args);

    m_pLog->write(log_level, buffer.data());
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
    m_trimParam(),
    m_pFileReader(),
    m_AudioReaders(),
    m_pFileWriter(),
    m_pFileWriterListAudio(),
    m_pStatus(),
    m_pPerfMonitor(),
    m_nProcSpeedLimit(0),
    m_nAVSyncMode(RGY_AVSYNC_ASSUME_CFR),
    m_inputFps(),
    m_encFps(),
    m_outputTimebase(),
    m_encWidth(0),
    m_encHeight(0),
    m_sar(),
    m_picStruct(RGY_PICSTRUCT_UNKNOWN),
    m_dll(),
    m_dx9(),
    m_dx11(),
    m_pFactory(nullptr),
    m_pDebug(nullptr),
    m_pTrace(nullptr),
    m_tracer(),
    m_AMFRuntimeVersion(0),
    m_pContext(),
    m_vpFilters(),
    m_pLastFilterParam(),
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
    if (m_bTimerPeriodTuning) {
        timeEndPeriod(1);
        PrintMes(RGY_LOG_DEBUG, _T("timeEndPeriod(1)\n"));
        m_bTimerPeriodTuning = false;
    }
    m_state = RGY_STATE_STOPPED;
    PrintMes(RGY_LOG_DEBUG, _T("Pipeline Stopped.\n"));

    m_pTrimParam = nullptr;

    if (m_pEncoder != nullptr) {
        m_pEncoder->Terminate();
        m_pEncoder = nullptr;
    }

    if (m_pConverter != nullptr) {
        m_pConverter->Terminate();
        m_pConverter = nullptr;
    }

    if (m_pDecoder != nullptr) {
        m_pDecoder->Terminate();
        m_pDecoder = nullptr;
    }

    if (m_pContext != nullptr) {
        m_pContext->Terminate();
        m_pContext = nullptr;
    }
    m_vpFilters.clear();
    m_pLastFilterParam.reset();
    m_cl.reset();
    m_dx11.Terminate();
    m_dx9.Terminate();

    m_pFileWriterListAudio.clear();
    m_pFileWriter.reset();
    m_AudioReaders.clear();
    m_pFileReader.reset();
    m_Chapters.clear();
    m_keyFile.clear();
    m_hdr10plus.reset();
    m_pPerfMonitor.reset();
    m_pStatus.reset();
    m_tracer.reset();
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
        av_dict_set(&avchap->metadata, "title", wstring_to_string(chapter_list[i]->name, CP_UTF8).c_str(), 0);
        chap_log += strsprintf(_T("chapter #%02d [%d.%02d.%02d.%03d]: %s.\n"),
            avchap->id, chapter_list[i]->h, chapter_list[i]->m, chapter_list[i]->s, chapter_list[i]->ms,
            wstring_to_tstring(chapter_list[i]->name).c_str());
        m_Chapters.push_back(std::move(avchap));
    }
    PrintMes(RGY_LOG_DEBUG, _T("%s"), chap_log.c_str());
    return RGY_ERR_NONE;
#else
    PrintMes(RGY_LOG_ERROR, _T("chater reading unsupportted in this build"));
    return RGY_ERR_UNSUPPORTED;
#endif //#if ENABLE_AVSW_READER
}

RGY_ERR VCECore::InitChapters(VCEParam *prm) {
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

RGY_ERR VCECore::initLog(int loglevel) {
    m_pLog.reset(new RGYLog(nullptr, loglevel));
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::initLog(VCEParam *prm) {
    m_pLog.reset(new RGYLog(prm->ctrl.logfile.c_str(), prm->ctrl.loglevel));
    if (prm->ctrl.logfile.length() > 0) {
        m_pLog->writeFileHeader(prm->common.outputFilename.c_str());
    }
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::initPerfMonitor(VCEParam *prm) {
    const bool bLogOutput = prm->ctrl.perfMonitorSelect || prm->ctrl.perfMonitorSelectMatplot;
    tstring perfMonLog;
    if (bLogOutput) {
        perfMonLog = prm->common.outputFilename + _T("_perf.csv");
    }
    CPerfMonitorPrm perfMonitorPrm = { 0 };
#if ENABLE_NVML
    perfMonitorPrm.pciBusId = selectedGpu->pciBusId.c_str();
#endif
    if (m_pPerfMonitor->init(perfMonLog.c_str(), _T(""), (bLogOutput) ? prm->ctrl.perfMonitorInterval : 1000,
        (int)prm->ctrl.perfMonitorSelect, (int)prm->ctrl.perfMonitorSelectMatplot,
        std::unique_ptr<void, handle_deleter>(OpenThread(SYNCHRONIZE | THREAD_QUERY_INFORMATION, false, GetCurrentThreadId()), handle_deleter()),
        m_pLog, &perfMonitorPrm)) {
        PrintMes(RGY_LOG_WARN, _T("Failed to initialize performance monitor, disabled.\n"));
        m_pPerfMonitor.reset();
    }
    return RGY_ERR_NONE;
}

RGY_CSP VCECore::GetEncoderCSP(const VCEParam *inputParam) {
    const bool highBitDepth = false;
    const bool yuv444 = false;
    if (highBitDepth) {
        return (yuv444) ? RGY_CSP_YUV444_16 : RGY_CSP_P010;
    } else {
        return (yuv444) ? RGY_CSP_YUV444 : RGY_CSP_NV12;
    }
}

RGY_ERR VCECore::initInput(VCEParam *inputParam) {
#if ENABLE_RAW_READER
    DeviceCodecCsp HWDecCodecCsp;
    HWDecCodecCsp.push_back(std::make_pair(0, getHWDecCodecCsp()));
    m_pStatus.reset(new EncodeStatus());

    int subburnTrackId = 0;
#if ENCODER_NVENC
    if (inputParam->common.nSubtitleSelectCount > 0 && inputParam->vpp.subburn.size() > 0) {
        PrintMes(RGY_LOG_ERROR, _T("--sub-copy and --vpp-subburn should not be set at the same time.\n"));
        return RGY_ERR_INVALID_PARAM;
    }
    for (const auto &subburn : inputParam->vpp.subburn) {
        if (subburn.trackId > 0) {
            subburnTrackId = subburn.trackId;
            break;
        }
    }
#endif

    const bool vpp_rff = false; // inputParam->vpp.rff;
    auto err = initReaders(m_pFileReader, m_AudioReaders, &inputParam->input,
        m_pStatus, &inputParam->common, &inputParam->ctrl, HWDecCodecCsp, subburnTrackId,
        inputParam->vpp.afs.enable, vpp_rff, m_pPerfMonitor.get(), m_pLog);
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
#endif
#if ENCODER_NVENC
    if (inputParam->common.dynamicHdr10plusJson.length() > 0) {
        m_hdr10plus = initDynamicHDR10Plus(inputParam->common.dynamicHdr10plusJson, m_pNVLog);
        if (!m_hdr10plus) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to initialize hdr10plus reader.\n"));
            return NV_ENC_ERR_GENERIC;
        }
    }
#endif
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
        prm->input.srcWidth, prm->input.srcHeight, prm->input.sar[0], prm->input.sar[1], prm->input.crop);

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
#ifndef VCE_AUO
    if (prm->vui.fullrange && prm->codec != RGY_CODEC_H264) {
        PrintMes(RGY_LOG_WARN, _T("fullrange flag is only supported with H.264 encoding, disabled.\n"));
        prm->vui.fullrange = FALSE;
    }
#endif
    if (prm->nBframes > 0 && prm->codec == RGY_CODEC_HEVC) {
        PrintMes(RGY_LOG_WARN, _T("Bframes is not supported with HEVC encoding, disabled.\n"));
        prm->nBframes = 0;
        prm->bBPyramid = 0;
        prm->nDeltaQPBFrame = 0;
        prm->nDeltaQPBFrameRef = 0;
    }
    prm->nQPMax = clamp(prm->nQPMax, 0, 51);
    prm->nQPMin = clamp(prm->nQPMin, 0, 51);
    prm->nQPI   = clamp(prm->nQPI,   0, 51);
    prm->nQPP   = clamp(prm->nQPP,   0, 51);
    prm->nQPB   = clamp(prm->nQPB,   0, 51);

    return RGY_ERR_NONE;
}

RGY_ERR VCECore::initOutput(VCEParam *inputParams) {
    const auto outputVideoInfo = videooutputinfo(
        inputParams->codec,
        formatOut,
        m_params,
        inputParams->input.picstruct,
        inputParams->vui
    );


    int subburnTrackId = 0;
#if ENCODER_NVENC
    for (const auto &subburn : inputParams->vpp.subburn) {
        if (subburn.trackId > 0) {
            subburnTrackId = subburn.trackId;
            break;
        }
    }
#endif

    auto err = initWriters(m_pFileWriter, m_pFileWriterListAudio, m_pFileReader, m_AudioReaders,
        &inputParams->common, &inputParams->input, &inputParams->ctrl, outputVideoInfo,
        m_trimParam, m_outputTimebase, m_Chapters, subburnTrackId, false, false, m_pStatus, m_pPerfMonitor, m_pLog);
    if (err != RGY_ERR_NONE) {
        PrintMes(RGY_LOG_ERROR, _T("failed to initialize file reader(s).\n"));
        return err;
    }
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::initDevice(const int deviceId, const bool interopD3d9, const bool interopD3d11) {
    if (interopD3d9) {
        auto err = m_dx9.Init(true, deviceId, false, 1280, 720, m_pLog);
        if (err != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to get directX9 device.\n"));
            return RGY_ERR_DEVICE_LOST;
        }
        auto amferr = m_pContext->InitDX9(m_dx9.GetDevice());
        if (amferr != AMF_OK) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to init AMF context by directX9.\n"));
            return err_to_rgy(amferr);
        }
    }
    if (interopD3d11) {
        auto err = m_dx11.Init(deviceId, false, m_pLog);
        if (err != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to get directX11 device.\n"));
            return RGY_ERR_DEVICE_LOST;
        }
        auto amferr = m_pContext->InitDX11(m_dx11.GetDevice());
        if (amferr != AMF_OK) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to init AMF context by directX11.\n"));
            return err_to_rgy(amferr);
        }
    }

    RGYOpenCL cl(m_pLog);
    auto platforms = cl.getPlatforms("AMD");
    if (platforms.size() == 0) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to find OpenCL platforms.\n"));
        return RGY_ERR_DEVICE_LOST;
    }
    auto& platform = platforms[0];
    if (interopD3d9) {
        if (platform->createDeviceListD3D9(CL_DEVICE_TYPE_GPU, (void *)m_dx9.GetDevice()) != CL_SUCCESS || platform->devs().size() == 0) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to find device.\n"));
            return RGY_ERR_DEVICE_LOST;
        }
    } else if (interopD3d11) {
        if (platform->createDeviceListD3D11(CL_DEVICE_TYPE_GPU, (void *)m_dx11.GetDevice()) != CL_SUCCESS || platform->devs().size() == 0) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to find device.\n"));
            return RGY_ERR_DEVICE_LOST;
        }
    } else {
        if (platform->createDeviceList(CL_DEVICE_TYPE_GPU) != CL_SUCCESS || platform->devs().size() == 0) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to find device.\n"));
            return RGY_ERR_DEVICE_LOST;
        }
    }
    auto devices = platform->devs();
    if ((int)devices.size() <= deviceId) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to device #%d.\n"), deviceId);
        return RGY_ERR_DEVICE_LOST;
    }
    platform->setDev(devices[deviceId], m_dx9.GetDevice(), m_dx11.GetDevice());

    m_cl = std::make_shared<RGYOpenCLContext>(platform, m_pLog);
    if (m_cl->createContext() != CL_SUCCESS) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to create OpenCL context.\n"));
        return RGY_ERR_UNKNOWN;
    }
    auto amferr = m_pContext->InitOpenCL(m_cl->queue().get());
    if (amferr != AMF_OK) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to init AMF context by OpenCL.\n"));
        return err_to_rgy(amferr);
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
    if (inputCodec == RGY_CODEC_HEVC && prm->input.csp == RGY_CSP_P010) {
        codec_uvd_name = AMFVideoDecoderHW_H265_MAIN10;
    }
    PrintMes(RGY_LOG_DEBUG, _T("decoder: use codec \"%s\".\n"), wstring_to_tstring(codec_uvd_name).c_str());
    auto res = m_pFactory->CreateComponent(m_pContext, codec_uvd_name, &m_pDecoder);
    if (res != RGY_ERR_NONE) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to create decoder context: %s\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("created decoder context.\n"));

    if (RGY_ERR_NONE != (res = m_pDecoder->SetProperty(AMF_TIMESTAMP_MODE, amf_int64(AMF_TS_PRESENTATION)))) {
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

    amf::AMFBufferPtr buffer;
    m_pContext->AllocBuffer(amf::AMF_MEMORY_HOST, header.size(), &buffer);

    memcpy(buffer->GetNative(), header.data(), header.size());
    m_pDecoder->SetProperty(AMF_VIDEO_DECODER_EXTRADATA, amf::AMFVariant(buffer));

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
    if (prm->input.dstWidth == prm->input.srcWidth
        && prm->input.dstHeight == prm->input.srcHeight
        && csp_rgy_to_enc(prm->input.csp) == formatOut) {
        PrintMes(RGY_LOG_DEBUG, _T("converter not required.\n"));
        return RGY_ERR_NONE;
    }
    auto res = m_pFactory->CreateComponent(m_pContext, AMFVideoConverter, &m_pConverter);
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
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::getEncCaps(RGY_CODEC codec, amf::AMFCapsPtr &encoderCaps) {
    amf::AMFComponentPtr p_encoder;
    auto ret = m_pFactory->CreateComponent(m_pContext, codec_rgy_to_enc(codec), &p_encoder);
    if (ret == AMF_OK) {
        //HEVCでのAMFComponent::GetCaps()は、AMFComponent::Init()を呼んでおかないと成功しない
        p_encoder->Init(amf::AMF_SURFACE_NV12, 1280, 720);
        ret = p_encoder->GetCaps(&encoderCaps);
    }
    return err_to_rgy(ret);
}

tstring VCECore::QueryIOCaps(amf::AMFIOCapsPtr& ioCaps) {
    tstring str;
    bool result = true;
    if (ioCaps != NULL) {
        amf_int32 minWidth, maxWidth;
        ioCaps->GetWidthRange(&minWidth, &maxWidth);
        str += strsprintf(_T("Width:       %d - %d\n"), minWidth, maxWidth);

        amf_int32 minHeight, maxHeight;
        ioCaps->GetHeightRange(&minHeight, &maxHeight);
        str += strsprintf(_T("Height:      %d - %d\n"), minHeight, maxHeight);

        amf_int32 vertAlign = ioCaps->GetVertAlign();
        str += strsprintf(_T("alignment:   %d\n"), vertAlign);

        amf_bool interlacedSupport = ioCaps->IsInterlacedSupported();
        str += strsprintf(_T("Interlace:   %s\n"), interlacedSupport ? _T("yes") : _T("no"));

        amf_int32 numOfFormats = ioCaps->GetNumOfFormats();
        str += _T("pix format:  ");
        for (amf_int32 i = 0; i < numOfFormats; i++) {
            amf::AMF_SURFACE_FORMAT format;
            amf_bool native = false;
            if (ioCaps->GetFormatAt(i, &format, &native) == AMF_OK) {
                if (i) str += _T(", ");
                str += wstring_to_tstring(m_pTrace->SurfaceGetFormatName(format)) + ((native) ? _T("(native)") : _T(""));
            }
        }
        str += _T("\n");

        if (result == true) {
            amf_int32 numOfMemTypes = ioCaps->GetNumOfMemoryTypes();
            str += _T("memory type: ");
            for (amf_int32 i = 0; i < numOfMemTypes; i++) {
                amf::AMF_MEMORY_TYPE memType;
                amf_bool native = false;
                if (ioCaps->GetMemoryTypeAt(i, &memType, &native) == AMF_OK) {
                    if (i) str += _T(", ");
                    str += wstring_to_tstring(m_pTrace->GetMemoryTypeName(memType)) + ((native) ? _T("(native)") : _T(""));
                }
            }
        }
        str += _T("\n");
    } else {
        str += _T("failed to get io capability\n");
    }
    return str;
}

tstring VCECore::QueryIOCaps(RGY_CODEC codec, amf::AMFCapsPtr& encoderCaps) {
    tstring str;
    if (encoderCaps == NULL) {
        str += _T("failed to get encoder capability\n");
    }
    amf::AMF_ACCELERATION_TYPE accelType = encoderCaps->GetAccelerationType();
    str += _T("acceleration:   ") + AccelTypeToString(accelType) + _T("\n");

    amf_uint32 maxProfile = 0;
    encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_PROFILE(codec), &maxProfile);
    str += _T("max profile:    ") + tstring(get_cx_desc(get_profile_list(codec), maxProfile)) + _T("\n");

    amf_uint32 maxLevel = 0;
    encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_LEVEL(codec), &maxLevel);
    str += _T("max level:      ") + tstring(get_cx_desc(get_level_list(codec), maxLevel)) + _T("\n");

    amf_uint32 maxBitrate = 0;
    encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_BITRATE(codec), &maxBitrate);
    str += strsprintf(_T("max bitrate:    %d kbps\n"), maxBitrate / 1000);

    amf_uint32 maxRef = 0, minRef = 0;
    encoderCaps->GetProperty(AMF_PARAM_CAP_MIN_REFERENCE_FRAMES(codec), &minRef);
    encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_REFERENCE_FRAMES(codec), &maxRef);
    str += strsprintf(_T("ref frames:     %d-%d\n"), minRef, maxRef);

    if (codec == RGY_CODEC_H264) {
        //amf_uint32 maxTemporalLayers = 0;
        //encoderCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_TEMPORAL_LAYERS, &maxTemporalLayers);
        //str += strsprintf(_T("max temp layers:  %d\n"), maxTemporalLayers);

        bool bBPictureSupported = false;
        encoderCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_BFRAMES, &bBPictureSupported);
        str += strsprintf(_T("Bframe support: %s\n"), (bBPictureSupported) ? _T("yes") : _T("no"));

        amf_uint32 NumOfHWInstances = 0;
        encoderCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_NUM_OF_HW_INSTANCES, &NumOfHWInstances);
        str += strsprintf(_T("HW instances:   %d\n"), NumOfHWInstances);
    } else if (codec == RGY_CODEC_HEVC) {
        //いまは特になし
    }

    amf_uint32 maxNumOfStreams = 0;
    encoderCaps->GetProperty(AMF_PARAM_CAP_NUM_OF_STREAMS(codec), &maxNumOfStreams);
    str += strsprintf(_T("max streams:    %d\n"), maxNumOfStreams);


    str += strsprintf(_T("\n%s encoder input:\n"), CodecToStr(codec).c_str());
    amf::AMFIOCapsPtr inputCaps;
    if (encoderCaps->GetInputCaps(&inputCaps) == AMF_OK) {
        str += QueryIOCaps(inputCaps);
    }

    str += strsprintf(_T("\n%s encoder output:\n"), CodecToStr(codec).c_str());
    amf::AMFIOCapsPtr outputCaps;
    if (encoderCaps->GetOutputCaps(&outputCaps) == AMF_OK) {
        str += QueryIOCaps(outputCaps);
    }
    return str;
}

RGY_ERR VCECore::initFilters(VCEParam *inputParam) {
    //hwデコーダの場合、cropを入力時に行っていない
    const bool cropRequired = cropEnabled(inputParam->input.crop)
        && m_pFileReader->getInputCodec() != RGY_CODEC_UNKNOWN;

    FrameInfo inputFrame = { 0 };
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
    //if (inputParam->vpp.pad.enable) {
    //    m_encWidth  += inputParam->vpp.pad.right + inputParam->vpp.pad.left;
    //    m_encHeight += inputParam->vpp.pad.bottom + inputParam->vpp.pad.top;
    //}

    //指定のリサイズがあればそのサイズに設定する
    if (inputParam->input.dstWidth > 0 && inputParam->input.dstHeight > 0) {
        m_encWidth = inputParam->input.dstWidth;
        m_encHeight = inputParam->input.dstHeight;
        resizeWidth = m_encWidth;
        resizeHeight = m_encHeight;
        //if (inputParam->vpp.pad.enable) {
        //    resizeWidth -= (inputParam->vpp.pad.right + inputParam->vpp.pad.left);
        //    resizeHeight -= (inputParam->vpp.pad.bottom + inputParam->vpp.pad.top);
        //}
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
    //if (inputParam->vpp.nnedi.enable) deinterlacer++;
    //if (inputParam->vpp.yadif.enable) deinterlacer++;
    if (deinterlacer >= 2) {
        PrintMes(RGY_LOG_ERROR, _T("Activating 2 or more deinterlacer is not supported.\n"));
        return RGY_ERR_UNSUPPORTED;
    }

    //フィルタが必要
    if (resizeRequired
        || cropRequired
        || inputParam->vpp.afs.enable) {
        //swデコードならGPUに上げる必要がある
        if (m_pFileReader->getInputCodec() == RGY_CODEC_UNKNOWN) {
            amf::AMFContext::AMFOpenCLLocker locker(m_pContext);
            unique_ptr<RGYFilter> filterCrop(new RGYFilterCspCrop(m_cl));
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
#if 0
        if (inputParam->vpp.colorspace.enable) {
            amf::AMFContext::AMFOpenCLLocker locker(m_pContext);
            unique_ptr<RGYFilter> filter(new RGYFilterColorspace(m_cl));
            shared_ptr<RGYFilterParamColorspace> param(new RGYFilterParamColorspace());
            param->colorspace = inputParam->vpp.colorspace;
            param->encCsp = encCsp;
            param->frameIn = inputFrame;
            param->frameOut = inputFrame;
            param->baseFps = m_encFps;
            NVEncCtxAutoLock(cxtlock(m_ctxLock));
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
#endif
        if (filterCsp != inputFrame.csp
            || cropRequired) { //cropが必要ならただちに適用する
            amf::AMFContext::AMFOpenCLLocker locker(m_pContext);
            unique_ptr<RGYFilter> filterCrop(new RGYFilterCspCrop(m_cl));
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
        //afs
        if (inputParam->vpp.afs.enable) {
            if ((inputParam->input.picstruct & (RGY_PICSTRUCT_TFF | RGY_PICSTRUCT_BFF)) == 0) {
                PrintMes(RGY_LOG_ERROR, _T("Please set input interlace field order (--interlace tff/bff) for vpp-afs.\n"));
                return RGY_ERR_INVALID_PARAM;
            }
            amf::AMFContext::AMFOpenCLLocker locker(m_pContext);
            unique_ptr<RGYFilter> filter(new RGYFilterAfs(m_cl));
            shared_ptr<RGYFilterParamAfs> param(new RGYFilterParamAfs());
            param->afs = inputParam->vpp.afs;
            param->afs.tb_order = (inputParam->input.picstruct & RGY_PICSTRUCT_TFF) != 0;
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
        //リサイズ
        if (resizeRequired) {
            amf::AMFContext::AMFOpenCLLocker locker(m_pContext);
            unique_ptr<RGYFilter> filterResize(new RGYFilterResize(m_cl));
            shared_ptr<RGYFilterParamResize> param(new RGYFilterParamResize());
            param->interp = (inputParam->vpp.resize != RGY_VPP_RESIZE_AUTO) ? inputParam->vpp.resize : RGY_VPP_RESIZE_SPLINE36;
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
    }
    //最後のフィルタ
    {
        //もし入力がCPUメモリで色空間が違うなら、一度そのままGPUに転送する必要がある
        if (inputFrame.mem_type == RGY_MEM_TYPE_CPU && inputFrame.csp != GetEncoderCSP(inputParam)) {
            amf::AMFContext::AMFOpenCLLocker locker(m_pContext);
            unique_ptr<RGYFilter> filterCrop(new RGYFilterCspCrop(m_cl));
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
        amf::AMFContext::AMFOpenCLLocker locker(m_pContext);
        unique_ptr<RGYFilter> filterCrop(new RGYFilterCspCrop(m_cl));
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
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::initEncoder(VCEParam *prm) {
    AMF_RESULT res = AMF_OK;

    m_encWidth  = (m_pLastFilterParam) ? m_pLastFilterParam->frameOut.width  : prm->input.srcWidth  - prm->input.crop.e.left - prm->input.crop.e.right;
    m_encHeight = (m_pLastFilterParam) ? m_pLastFilterParam->frameOut.height : prm->input.srcHeight - prm->input.crop.e.bottom - prm->input.crop.e.up;

    if (m_pLog->getLogLevel() <= RGY_LOG_DEBUG) {
        TCHAR cpuInfo[256] = { 0 };
        tstring gpu_info = getGPUInfo();
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
    const amf::AMF_SURFACE_FORMAT formatIn = amf::AMF_SURFACE_NV12;
    if (AMF_OK != (res = m_pFactory->CreateComponent(m_pContext, codec_rgy_to_enc(prm->codec), &m_pEncoder))) {
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

        int maxProfile = 0;
        encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_PROFILE(prm->codec), &maxProfile);
        PrintMes(RGY_LOG_DEBUG, _T("Max Profile: %s.\n"), get_cx_desc(get_profile_list(prm->codec), maxProfile));
        if (prm->codecParam[prm->codec].nProfile > maxProfile) {
            PrintMes(RGY_LOG_ERROR, _T("Max supported %s Level on this platform is %s (%s specified).\n"),
                CodecToStr(prm->codec).c_str(),
                get_cx_desc(get_profile_list(prm->codec), maxProfile),
                get_cx_desc(get_profile_list(prm->codec), prm->codecParam[prm->codec].nProfile));
            return RGY_ERR_UNSUPPORTED;
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
        int maxRef = 0, minRef = 0;
        encoderCaps->GetProperty(AMF_PARAM_CAP_MIN_REFERENCE_FRAMES(prm->codec), &minRef);
        encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_REFERENCE_FRAMES(prm->codec), &maxRef);
        if (prm->nRefFrames < minRef || maxRef < prm->nRefFrames) {
            PrintMes(RGY_LOG_WARN, _T("%s reference frames should be in range of %d - %d (%d specified).\n"),
                CodecToStr(prm->codec).c_str(),
                minRef, maxRef, prm->nRefFrames);
            prm->nRefFrames = clamp(prm->nRefFrames, minRef, maxRef);
        }

        if (prm->codec == RGY_CODEC_H264) {
            bool bBPictureSupported = false;
            encoderCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_BFRAMES, &bBPictureSupported);
            if (prm->nBframes > 0 && !bBPictureSupported) {
                PrintMes(RGY_LOG_WARN, _T("Bframes is not supported with HEVC encoding, disabled.\n"));
                prm->nBframes = 0;
                prm->bBPyramid = 0;
                prm->nDeltaQPBFrame = 0;
                prm->nDeltaQPBFrameRef = 0;
            }

        } else if (prm->codec == RGY_CODEC_HEVC) {
            //いまはなにもなし
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

    m_params.SetParamType(VCE_PARAM_KEY_INPUT,         AMF_PARAM_COMMON, L"Input file name");
    m_params.SetParamType(VCE_PARAM_KEY_INPUT_WIDTH,   AMF_PARAM_COMMON, L"Input Frame width (integer, default = 0)");
    m_params.SetParamType(VCE_PARAM_KEY_INPUT_HEIGHT,  AMF_PARAM_COMMON, L"Input Frame height (integer, default = 0)");
    m_params.SetParamType(VCE_PARAM_KEY_OUTPUT,        AMF_PARAM_COMMON, L"Output file name");
    m_params.SetParamType(VCE_PARAM_KEY_OUTPUT_WIDTH,  AMF_PARAM_COMMON, L"Output Frame width (integer, default = 0)");
    m_params.SetParamType(VCE_PARAM_KEY_OUTPUT_HEIGHT, AMF_PARAM_COMMON, L"Output Frame height (integer, default = 0)");
    m_params.SetParamType(VCE_PARAM_KEY_ENGINE,        AMF_PARAM_COMMON, L"Specifies decoder/encoder engine type (DX9, DX11)");
    m_params.SetParamType(VCE_PARAM_KEY_ADAPTERID,     AMF_PARAM_COMMON, L"Specifies adapter ID (integer, default = 0)");
    m_params.SetParamType(VCE_PARAM_KEY_CAPABILITY,    AMF_PARAM_COMMON, L"Enable/Disable to display the device capabilities (true, false default =  false)");

    switch (prm->codec) {
    case RGY_CODEC_H264:
        m_params.SetParamTypeAVC();
        break;
    case RGY_CODEC_HEVC:
        m_params.SetParamTypeHEVC();
        //なぜかパラメータセットに登録されていないのでここで追加。
        m_params.SetParamType(AMF_PARAM_ASPECT_RATIO(prm->codec), AMF_PARAM_STATIC, L"");
        break;
    default:
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
                level = calc_hevc_auto_level(m_encWidth, m_encHeight, //m_stEncConfig.encodeCodecConfig.hevcConfig.maxNumRefFramesInDPB,
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
    m_params.SetParam(AMF_PARAM_USAGE(prm->codec),          (amf_int64)((prm->codec == RGY_CODEC_HEVC) ? AMF_VIDEO_ENCODER_HEVC_USAGE_TRANSCONDING : AMF_VIDEO_ENCODER_USAGE_TRANSCONDING));
    m_params.SetParam(AMF_PARAM_PROFILE(prm->codec),        (amf_int64)prm->codecParam[prm->codec].nProfile);
    m_params.SetParam(AMF_PARAM_PROFILE_LEVEL(prm->codec),  (amf_int64)prm->codecParam[prm->codec].nLevel);
    m_params.SetParam(AMF_PARAM_QUALITY_PRESET(prm->codec), (amf_int64)get_quality_preset(prm->codec)[prm->qualityPreset]);
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

    m_params.SetParam(AMF_PARAM_ENFORCE_HRD(prm->codec),        prm->bEnforceHRD != 0);
    m_params.SetParam(AMF_PARAM_FILLER_DATA_ENABLE(prm->codec), prm->bFiller != 0);
    if (prm->bVBAQ) m_params.SetParam(AMF_PARAM_ENABLE_VBAQ(prm->codec), true);
    m_params.SetParam(AMF_PARAM_SLICES_PER_FRAME(prm->codec),               (amf_int64)prm->nSlices);
    m_params.SetParam(AMF_PARAM_GOP_SIZE(prm->codec),                       (amf_int64)nGOPLen);

    m_params.SetParam(AMF_PA_SCENE_CHANGE_DETECTION_ENABLE, prm->pa.sc);
    if (prm->pa.sc) m_params.SetParam(AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY, (amf_int64)prm->pa.scSensitivity);
    m_params.SetParam(AMF_PA_STATIC_SCENE_DETECTION_ENABLE, prm->pa.ss);
    if (prm->pa.ss)  m_params.SetParam(AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY, (amf_int64)prm->pa.scSensitivity);
    m_params.SetParam(AMF_PA_ACTIVITY_TYPE, (amf_int64)prm->pa.activityType);
    if (prm->pa.initQPSC > AMF_PA_INITQPSC_AUTO) m_params.SetParam(AMF_PA_INITIAL_QP_AFTER_SCENE_CHANGE, (amf_int64)prm->pa.initQPSC); //設定しなければ自動
    m_params.SetParam(AMF_PA_MAX_QP_BEFORE_FORCE_SKIP, (amf_int64)prm->pa.maxQPBeforeForceSkip);
    m_params.SetParam(AMF_PA_CAQ_STRENGTH, (amf_int64)prm->pa.CAQStrength);

    //m_params.SetParam(AMF_PARAM_END_OF_SEQUENCE(prm->codec),                false);
    m_params.SetParam(AMF_PARAM_INSERT_AUD(prm->codec),                     false);
    if (prm->codec == RGY_CODEC_H264) {
        //m_params.SetParam(AMF_PARAM_RATE_CONTROL_PREANALYSIS_ENABLE(prm->codec), (prm->preAnalysis) ? AMF_VIDEO_ENCODER_PREENCODE_ENABLED : AMF_VIDEO_ENCODER_PREENCODE_DISABLED);

        m_params.SetParam(AMF_VIDEO_ENCODER_SCANTYPE,           (amf_int64)((prm->input.picstruct & RGY_PICSTRUCT_INTERLACED) ? AMF_VIDEO_ENCODER_SCANTYPE_INTERLACED : AMF_VIDEO_ENCODER_SCANTYPE_PROGRESSIVE));

        m_params.SetParam(AMF_VIDEO_ENCODER_B_PIC_PATTERN, (amf_int64)prm->nBframes);
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
        //m_params.SetParam(AMF_VIDEO_ENCODER_PICTURE_STRUCTURE,                (amf_int64)prm->nPicStruct);
        //m_params.SetParam(AMF_VIDEO_ENCODER_MARK_CURRENT_WITH_LTR_INDEX,    false);
        //m_params.SetParam(AMF_VIDEO_ENCODER_FORCE_LTR_REFERENCE_BITFIELD,   (amf_int64)0);

        //m_params.SetParam(AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE, AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM);
        //m_params.SetParam(AMF_VIDEO_ENCODER_OUTPUT_MARKED_LTR_INDEX, (amf_int64)-1);
        //m_params.SetParam(AMF_VIDEO_ENCODER_OUTPUT_REFERENCED_LTR_INDEX_BITFIELD, (amf_int64)0);
        if (prm->vui.fullrange) {
            m_params.SetParam(AMF_VIDEO_ENCODER_FULL_RANGE_COLOR, true);
        }
    } else if (prm->codec == RGY_CODEC_HEVC) {
        //m_params.SetParam(AMF_PARAM_RATE_CONTROL_PREANALYSIS_ENABLE(prm->codec), prm->preAnalysis);

        m_params.SetParam(AMF_VIDEO_ENCODER_HEVC_TIER,                            (amf_int64)prm->codecParam[prm->codec].nTier);

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

    if (AMF_OK != (res = m_pEncoder->Init(formatIn, m_encWidth, m_encHeight))) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to initalize encoder: %s.\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("initalized encoder.\n"));

    m_params.Apply(m_pEncoder, AMF_PARAM_DYNAMIC, m_pLog.get());
    PrintMes(RGY_LOG_DEBUG, _T("pushed dynamic params.\n"));
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::initAMFFactory() {
    m_dll = std::unique_ptr<std::remove_pointer_t<HMODULE>, module_deleter>(LoadLibrary(AMF_DLL_NAME));
    if (!m_dll) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to load %s.\n"), AMF_DLL_NAME);
        return RGY_ERR_NOT_FOUND;
    }
    AMFInit_Fn initFun = (AMFInit_Fn)GetProcAddress(m_dll.get(), AMF_INIT_FUNCTION_NAME);
    if (initFun == NULL) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to load %s.\n"), AMF_INIT_FUNCTION_NAME);
        return RGY_ERR_NOT_FOUND;
    }
    AMF_RESULT res = initFun(AMF_FULL_VERSION, &m_pFactory);
    if (res != AMF_OK) {
        return err_to_rgy(res);
    }
    AMFQueryVersion_Fn versionFun = (AMFQueryVersion_Fn)GetProcAddress(m_dll.get(), AMF_QUERY_VERSION_FUNCTION_NAME);
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
    PrintMes(RGY_LOG_DEBUG, _T("Loaded %s.\n"), AMF_DLL_NAME);
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::initContext(int log_level) {
    m_pTrace->EnableWriter(AMF_TRACE_WRITER_DEBUG_OUTPUT, log_level < RGY_LOG_INFO);
    if (log_level < RGY_LOG_INFO)
        m_pTrace->SetWriterLevel(AMF_TRACE_WRITER_DEBUG_OUTPUT, loglevel_rgy_to_enc(log_level));
    m_pTrace->EnableWriter(AMF_TRACE_WRITER_CONSOLE, false);
    m_pTrace->SetGlobalLevel(loglevel_rgy_to_enc(log_level));

    m_tracer.init(m_pLog);
    m_pTrace->RegisterWriter(L"RGYLOGWriter", &m_tracer, log_level < RGY_LOG_INFO);
    m_pTrace->SetWriterLevel(L"RGYLOGWriter", loglevel_rgy_to_enc(log_level));

    auto res = m_pFactory->CreateContext(&m_pContext);
    if (res != AMF_OK) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to CreateContext(): %s.\n"), get_err_mes(err_to_rgy(res)));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("CreateContext() Success.\n"));
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::init(VCEParam *prm) {
    RGY_ERR ret = initLog(prm);
    if (ret != RGY_ERR_NONE) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to initalize logger: %s"), get_err_mes(ret));
        return ret;
    }

    ret = initAMFFactory();
    if (ret != RGY_ERR_NONE) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to initalize VCE factory: %s"), get_err_mes(ret));
        return ret;
    }

    ret = initContext(prm->ctrl.loglevel);
    if (ret != RGY_ERR_NONE) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to initalize VCE context: %s"), get_err_mes(ret));
        return ret;
    }
    PrintMes(RGY_LOG_DEBUG, _T("Created AMF Context.\n"));

    if (prm->bTimerPeriodTuning) {
        m_bTimerPeriodTuning = true;
        timeBeginPeriod(1);
        PrintMes(RGY_LOG_DEBUG, _T("timeBeginPeriod(1)\n"));
    }

    if (!m_pStatus) {
        m_pStatus = std::make_shared<EncodeStatus>();
    }

    m_pPerfMonitor = std::make_unique<CPerfMonitor>();

    prm->input.csp = RGY_CSP_NV12;
    m_nAVSyncMode = prm->common.AVSyncMode;

    if (RGY_ERR_NONE != (ret = initInput(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = checkParam(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initDevice(0, prm->interopD3d9, prm->interopD3d11))) {
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

    if (RGY_ERR_NONE != (ret = initPerfMonitor(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initOutput(prm))) {
        return ret;
    }

    return ret;
}

RGY_ERR VCECore::run_decode() {
    m_thDecoder = std::thread([this]() {
        auto pAVCodecReader = std::dynamic_pointer_cast<RGYInputAvcodec>(m_pFileReader);
        if (pAVCodecReader == nullptr) {
            return RGY_ERR_UNKNOWN;
        }
        const auto VCE_TIMEBASE = rgy_rational<int>(1, AMF_SECOND); // In 100 NanoSeconds
        const auto inTimebase = pAVCodecReader->GetInputVideoStream()->time_base;
        RGYBitstream bitstream = RGYBitstreamInit();
        RGY_ERR sts = RGY_ERR_NONE;
        for (int i = 0; sts == RGY_ERR_NONE && m_state == RGY_STATE_RUNNING; i++) {
            m_pFileReader->LoadNextFrame(nullptr); //進捗表示のため
            sts = m_pFileReader->GetNextBitstream(&bitstream);

            amf::AMFBufferPtr pictureBuffer;
            if (sts == RGY_ERR_NONE) {
                auto ar = m_pContext->AllocBuffer(amf::AMF_MEMORY_HOST, bitstream.size(), &pictureBuffer);
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
    PrintMes(RGY_LOG_DEBUG, _T("Started Encode thread.\n"));
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::run_output() {
    m_thOutput = std::thread([this]() {
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
            if (buffer->GetProperty(AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE, &value) == AMF_OK) {
                switch ((AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM)value) {
                case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_P: output.setFrametype(RGY_FRAMETYPE_P); break;
                case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_B: output.setFrametype(RGY_FRAMETYPE_B); break;
                case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_I: output.setFrametype(RGY_FRAMETYPE_I); break;
                case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_IDR:
                default:
                    output.setFrametype(RGY_FRAMETYPE_IDR); break;
                }

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
    const int pipelineDepth = 1;
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
        HANDLE thOutput = NULL;
        HANDLE thInput = NULL;
        HANDLE thAudProc = NULL;
        HANDLE thAudEnc = NULL;
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
        m_pPerfMonitor->SetThreadHandles((HANDLE)(m_thDecoder.native_handle()), thInput, thOutput, thAudProc, thAudEnc);
    }

    auto run_send_streams = [this, &pWriterForAudioStreams](int inputFrames) {
        vector<AVPacket> packetList = m_pFileReader->GetStreamDataPackets(inputFrames);

        //音声ファイルリーダーからのトラックを結合する
        for (const auto &reader : m_AudioReaders) {
            vector_cat(packetList, reader->GetStreamDataPackets(inputFrames));
        }
        //パケットを各Writerに分配する
        for (uint32_t i = 0; i < packetList.size(); i++) {
            const int nTrackId = packetList[i].flags >> 16;
            if (pWriterForAudioStreams.count(nTrackId)) {
                auto pWriter = pWriterForAudioStreams[nTrackId];
                if (pWriter == nullptr) {
                    PrintMes(RGY_LOG_ERROR, _T("Invalid writer found for track %d\n"), nTrackId);
                    return RGY_ERR_NULL_PTR;
                }
                auto ret = pWriter->WriteNextPacket(&packetList[i]);
                if (ret != RGY_ERR_NONE) {
                    return ret;
                }
            } else {
                PrintMes(RGY_LOG_ERROR, _T("Failed to find writer for track %d\n"), nTrackId);
                return RGY_ERR_NOT_FOUND;
            }
        }
        return RGY_ERR_NONE;
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
        return std::move(outFrames);
    };

    auto filter_frame = [&](int &nFilterFrame, unique_ptr<RGYFrame> &inframe, deque<unique_ptr<RGYFrame>> &dqEncFrames, bool &bDrain) {

        deque<std::pair<FrameInfo, uint32_t>> filterframes;

        bool skipFilters = false;
        if (bDrain) {
            filterframes.push_back(std::make_pair(initFrameInfo(), 0u));
        } else {
            auto &lastFilter = m_vpFilters[m_vpFilters.size()-1];
            const auto inframeInfo = inframe->info();
            if (typeid(*lastFilter.get()) == typeid(RGYFilterCspCrop)
                && m_vpFilters.size() == 1
                && lastFilter->GetFilterParam()->frameOut.csp == inframeInfo.csp
                && m_encWidth == inframeInfo.width
                && m_encHeight == inframeInfo.height) {
                skipFilters = true;
            }
            const auto& inAmf = inframe->amf();
            if (!skipFilters
                && inframeInfo.mem_type != RGY_MEM_TYPE_CPU
                && inAmf
                && inAmf->GetMemoryType() != amf::AMF_MEMORY_OPENCL) {
                amf::AMFContext::AMFOpenCLLocker locker(m_pContext);
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
                auto ar = m_pContext->AllocSurface(amf::AMF_MEMORY_HOST, csp_rgy_to_enc(frameinfo.csp),
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
            filterframes.push_back(std::make_pair(inframe->info(), 0u));
        }

        while (filterframes.size() > 0 || bDrain) {
            //フィルタリングするならここ
            for (uint32_t ifilter = filterframes.front().second; ifilter < m_vpFilters.size() - 1; ifilter++) {
                amf::AMFContext::AMFOpenCLLocker locker(m_pContext);
                int nOutFrames = 0;
                FrameInfo *outInfo[16] = { 0 };
                auto sts_filter = m_vpFilters[ifilter]->filter(&filterframes.front().first, (FrameInfo **)&outInfo, &nOutFrames);
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
                amf::AMFContext::AMFOpenCLLocker locker(m_pContext);
                auto &lastFilter = m_vpFilters[m_vpFilters.size()-1];
                amf::AMFSurfacePtr pSurface;
                if (m_dx11.GetDevice() != nullptr) {
                    auto ar = m_pContext->AllocSurface(amf::AMF_MEMORY_DX11, csp_rgy_to_enc(lastFilter->GetFilterParam()->frameOut.csp),
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
                    auto ar = m_pContext->AllocSurface(amf::AMF_MEMORY_OPENCL, csp_rgy_to_enc(lastFilter->GetFilterParam()->frameOut.csp),
                        m_encWidth, m_encHeight, &pSurface);
                    if (ar != AMF_OK) {
                        PrintMes(RGY_LOG_ERROR, _T("Failed to allocate surface: %s.\n"), get_err_mes(err_to_rgy(ar)));
                        return err_to_rgy(ar);
                    }
                }
                auto encSurface = std::make_unique<RGYFrame>(pSurface);
                //最後のフィルタはNVEncFilterCspCropでなければならない
                if (typeid(*lastFilter.get()) != typeid(RGYFilterCspCrop)) {
                    PrintMes(RGY_LOG_ERROR, _T("Last filter setting invalid.\n"));
                    return RGY_ERR_INVALID_PARAM;
                }
                //エンコードバッファのポインタを渡す
                int nOutFrames = 0;
                auto encSurfaceInfo = encSurface->info();
                FrameInfo *outInfo[1];
                outInfo[0] = &encSurfaceInfo;
                auto sts_filter = lastFilter->filter(&filterframes.front().first, (FrameInfo **)&outInfo, &nOutFrames);
                filterframes.pop_front();
                if (sts_filter != RGY_ERR_NONE) {
                    PrintMes(RGY_LOG_ERROR, _T("Error while running filter \"%s\".\n"), lastFilter->name().c_str());
                    return sts_filter;
                }
                auto err = m_cl->queue().finish();
                if (err != RGY_ERR_NONE) {
                    PrintMes(RGY_LOG_ERROR, _T("Failed to finish queue after \"%s\".\n"), lastFilter->name().c_str());
                    return sts_filter;
                }
                encSurface->setDuration(encSurfaceInfo.duration);
                encSurface->setTimestamp(encSurfaceInfo.timestamp);
                encSurface->setPicstruct(encSurfaceInfo.picstruct);
                encSurface->setInputFrameId(encSurfaceInfo.inputFrameId);
                encSurface->setFlags(encSurfaceInfo.flags);
                dqEncFrames.push_back(std::move(encSurface));
            }
        }
        return RGY_ERR_NONE;
    };

    auto send_encoder = [this](unique_ptr<RGYFrame>& encFrame) {
        int64_t pts = encFrame->timestamp();
        int64_t duration = encFrame->duration();
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
    for (int nFilterFrame = 0; m_state == RGY_STATE_RUNNING && !bInputEmpty && !bFilterEmpty; ) {
        if (m_pAbortByUser && *m_pAbortByUser) {
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
            auto ar = m_pContext->AllocSurface(amf::AMF_MEMORY_HOST, csp_rgy_to_enc(inputFrameInfo.csp),
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
            RGY_ERR err = filter_frame(nFilterFrame, inframe, dqEncFrames, bDrainFin);
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
            while (dqEncFrames.size() >= pipelineDepth) {
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
    if (m_thDecoder.joinable()) {
        DWORD exitCode = 0;
        while (GetExitCodeThread(m_thDecoder.native_handle(), &exitCode) == STILL_ACTIVE) {
            if ((res = run_send_streams(nInputFrame)) != RGY_ERR_NONE) {
                m_state = RGY_STATE_ERROR;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        m_thDecoder.join();
    }
    auto ar = AMF_INPUT_FULL;
    while (ar == AMF_INPUT_FULL) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        ar = m_pEncoder->Drain();
    }
    if (ar != AMF_OK) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to drain encoder: %s\n"), get_err_mes(err_to_rgy(ar)));
        return err_to_rgy(ar);
    }
    PrintMes(RGY_LOG_DEBUG, _T("Flushed Encoder\n"));
    if (m_thOutput.joinable()) {
        m_thOutput.join();
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
    return RGY_ERR_NONE;
}

tstring VCECore::getGPUInfo() {
    if (m_dx11.GetDevice()) {
        auto str = m_dx11.GetDisplayDeviceName();
        str = str_replace(str, L"(TM)", L"");
        str = str_replace(str, L"(R)", L"");
        str = str_replace(str, L" Series", L"");
        str = str_replace(str, L" Graphics", L"");
        return wstring_to_tstring(str);
    }
    return RGYOpenCLDevice(m_cl->platform()->dev(0)).infostr();
}

void VCECore::PrintEncoderParam() {
    PrintMes(RGY_LOG_INFO, GetEncoderParam().c_str());
}

tstring VCECore::GetEncoderParam() {
    const amf::AMFPropertyStorage *pProperty = m_pEncoder;

    auto GetPropertyStr = [pProperty](const wchar_t *pName) {
        const wchar_t *pProp;
        pProperty->GetPropertyWString(pName, &pProp);
        return wstring_to_string(pProp);
    };

    auto GetPropertyInt = [pProperty](const wchar_t *pName) {
        int64_t value;
        pProperty->GetProperty(pName, &value);
        return (int)value;
    };

    auto GetPropertyRatio = [pProperty](const wchar_t *pName) {
        AMFRatio value;
        pProperty->GetProperty(pName, &value);
        return value;
    };

    auto GetPropertyRate = [pProperty](const wchar_t *pName) {
        AMFRate value;
        pProperty->GetProperty(pName, &value);
        return value;
    };

    auto GetPropertyBool = [pProperty](const wchar_t *pName) {
        bool value;
        pProperty->GetProperty(pName, &value);
        return value;
    };

    auto getPropertyDesc = [pProperty, GetPropertyInt](const wchar_t *pName, const CX_DESC *list) {
        return tstring(get_cx_desc(list, GetPropertyInt(pName)));
    };

    tstring mes;

    TCHAR cpu_info[256];
    getCPUInfo(cpu_info);
    tstring gpu_info = getGPUInfo();

    OSVERSIONINFOEXW osversioninfo = { 0 };
    tstring osversionstr = getOSVersion(&osversioninfo);

    uint32_t nMotionEst = 0x0;
    nMotionEst |= GetPropertyInt(AMF_PARAM_MOTION_HALF_PIXEL(m_encCodec)) ? VCE_MOTION_EST_HALF : 0;
    nMotionEst |= GetPropertyInt(AMF_PARAM_MOTION_QUARTERPIXEL(m_encCodec)) ? VCE_MOTION_EST_QUATER | VCE_MOTION_EST_HALF : 0;

    mes += strsprintf(_T("%s\n"), get_encoder_version());
    mes += strsprintf(_T("OS Version     %s %s (%d)\n"), osversionstr.c_str(), rgy_is_64bit_os() ? _T("x64") : _T("x86"), osversioninfo.dwBuildNumber);
    mes += strsprintf(_T("CPU:           %s\n"), cpu_info);
    mes += strsprintf(_T("GPU:           %s, AMF %d.%d.%d\n"), gpu_info.c_str(),
        (int)AMF_GET_MAJOR_VERSION(m_AMFRuntimeVersion), (int)AMF_GET_MINOR_VERSION(m_AMFRuntimeVersion), (int)AMF_GET_SUBMINOR_VERSION(m_AMFRuntimeVersion));

    auto inputInfo = m_pFileReader->GetInputFrameInfo();
    mes += strsprintf(_T("Input Info:    %s\n"), m_pFileReader->GetInputMessage());
    if (cropEnabled(inputInfo.crop)) {
        mes += strsprintf(_T("Crop:          %d,%d,%d,%d\n"), inputInfo.crop.e.left, inputInfo.crop.e.up, inputInfo.crop.e.right, inputInfo.crop.e.bottom);
    }
    tstring vppFilterMes;
    for (const auto &filter : m_vpFilters) {
        vppFilterMes += strsprintf(_T("%s%s\n"), (vppFilterMes.length()) ? _T("               ") : _T("Vpp Filters    "), filter->GetInputMessage().c_str());
    }
    mes += vppFilterMes;
    mes += strsprintf(_T("Output:        %s  %s @ Level %s%s\n"),
        CodecToStr(m_encCodec).c_str(),
        getPropertyDesc(AMF_PARAM_PROFILE(m_encCodec), get_profile_list(m_encCodec)).c_str(),
        getPropertyDesc(AMF_PARAM_PROFILE_LEVEL(m_encCodec), get_level_list(m_encCodec)).c_str(),
        (m_encCodec == RGY_CODEC_HEVC) ? (tstring(_T(" (")) + getPropertyDesc(AMF_VIDEO_ENCODER_HEVC_TIER, get_tier_list(m_encCodec)) + _T(" tier)")).c_str() : _T(""));
    const AMF_VIDEO_ENCODER_SCANTYPE_ENUM scan_type = (m_encCodec == RGY_CODEC_H264) ? (AMF_VIDEO_ENCODER_SCANTYPE_ENUM)GetPropertyInt(AMF_VIDEO_ENCODER_SCANTYPE) : AMF_VIDEO_ENCODER_SCANTYPE_PROGRESSIVE;
    AMFRatio aspectRatio;
    m_params.GetParam(AMF_PARAM_ASPECT_RATIO(m_encCodec), aspectRatio);
    auto frameRate = GetPropertyRate(AMF_PARAM_FRAMERATE(m_encCodec));
    int64_t outWidth, outHeight;
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
    const int quality_preset = GetPropertyInt(AMF_PARAM_QUALITY_PRESET(m_encCodec));
    mes += strsprintf(_T("Quality:       %s\n"), list_vce_quality_preset[get_quality_index(m_encCodec, quality_preset)].desc);
    if (GetPropertyInt(AMF_PARAM_RATE_CONTROL_METHOD(m_encCodec)) == get_rc_method(m_encCodec)[0].value) {
        mes += strsprintf(_T("CQP:           I:%d, P:%d"),
            GetPropertyInt(AMF_PARAM_QP_I(m_encCodec)),
            GetPropertyInt(AMF_PARAM_QP_P(m_encCodec)));
        if (m_encCodec == RGY_CODEC_H264 && GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN)) {
            mes += strsprintf(_T(", B:%d"), GetPropertyInt(AMF_VIDEO_ENCODER_QP_B));
        }
        mes += _T("\n");
    } else {
        mes += strsprintf(_T("%s:           %d kbps, Max %d kbps\n"),
            getPropertyDesc(AMF_PARAM_RATE_CONTROL_METHOD(m_encCodec), get_rc_method(m_encCodec)).c_str(),
            GetPropertyInt(AMF_PARAM_TARGET_BITRATE(m_encCodec)) / 1000,
            GetPropertyInt(AMF_PARAM_PEAK_BITRATE(m_encCodec)) / 1000);
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
    mes += strsprintf(_T("PA:            "));
    if (GetPropertyBool(AMF_PA_SCENE_CHANGE_DETECTION_ENABLE)) {
        mes += _T("sc(") + getPropertyDesc(AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY, list_pa_sc_sensitivity) + _T(") ");
    }
    if (GetPropertyBool(AMF_PA_STATIC_SCENE_DETECTION_ENABLE)) {
        mes += _T("ss(") + getPropertyDesc(AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY, list_pa_ss_sensitivity) + _T(") ");
    }
    mes += _T("activity(") + getPropertyDesc(AMF_PA_ACTIVITY_TYPE, list_pa_activity) + _T(") ");
    mes += _T("caq(") + getPropertyDesc(AMF_PA_CAQ_STRENGTH, list_pa_caq_strength) + _T(") ");
    mes += _T("\n");
    mes += strsprintf(_T("Ref frames:    %d frames\n"), GetPropertyInt(AMF_PARAM_MAX_NUM_REFRAMES(m_encCodec)));
    mes += strsprintf(_T("LTR frames:    %d frames\n"), GetPropertyInt(AMF_PARAM_MAX_LTR_FRAMES(m_encCodec)));
    mes += strsprintf(_T("Motion Est:    %s\n"), get_cx_desc(list_mv_presicion, nMotionEst));
    mes += strsprintf(_T("Slices:        %d\n"), GetPropertyInt(AMF_PARAM_SLICES_PER_FRAME(m_encCodec)));
    mes += strsprintf(_T("GOP Len:       %d frames\n"), GetPropertyInt(AMF_PARAM_GOP_SIZE(m_encCodec)));
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
    if (m_pLog->getLogLevel() <= RGY_LOG_DEBUG) {
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

RGY_ERR VCEFeatures::init(int deviceId, int logLevel) {
    m_core = std::make_unique<VCECore>();
    auto err = RGY_ERR_NONE;
    if (   (err = m_core->initLog(logLevel)) != RGY_ERR_NONE
        || (err = m_core->initAMFFactory()) != RGY_ERR_NONE
        || (err = m_core->initContext(logLevel)) != RGY_ERR_NONE
        || (err = m_core->initDevice(deviceId, false, false)) != RGY_ERR_NONE) {
        return err;
    }
    return RGY_ERR_NONE;
}

tstring VCEFeatures::checkFeatures(RGY_CODEC codec) {
    tstring str;
    amf::AMFCapsPtr encCaps;
    if (m_core->getEncCaps(codec, encCaps) == RGY_ERR_NONE) {
        str = m_core->QueryIOCaps(codec, encCaps);
    }
    return str;
}

bool check_if_vce_available(int deviceId, int logLevel) {
    VCEFeatures vce;
    return vce.init(deviceId, logLevel) == RGY_ERR_NONE;
}

tstring check_vce_features(const std::vector<RGY_CODEC> &codecs, int deviceId, int logLevel) {
    VCEFeatures vce;
    if (vce.init(deviceId, logLevel) != RGY_ERR_NONE) {
        return _T("VCE not available.\n");
    }
    tstring str;
    for (const auto codec : codecs) {
        auto ret = vce.checkFeatures(codec);
        if (ret.length() > 0) {
            str += CodecToStr(codec) + _T(" encode features\n");
            str += vce.checkFeatures(codec) + _T("\n");
        }
    }
    return str;
}
