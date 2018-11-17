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
#include "rgy_version.h"
#include "rgy_bitstream.h"
#include "vce_input.h"
#include "vce_output.h"
#include "chapter_rw.h"

#include "VideoEncoderVCE.h"
#include "VideoEncoderHEVC.h"
#include "VideoDecoderUVD.h"
#include "VideoConverter.h"
#include "EncoderParamsAVC.h"
#include "EncoderParamsHEVC.h"

#include "h264_level.h"
#include "hevc_level.h"

#define ENCODER_SUBMIT_TIME     L"EncoderSubmitTime"  // private property to track submit tyme

static const amf::AMF_SURFACE_FORMAT formatOut = amf::AMF_SURFACE_NV12;

typedef decltype(WriteFile)* funcWriteFile;
static funcWriteFile origWriteFileFunc = nullptr;
static HANDLE hStdOut = NULL;
static HANDLE hStdErr = NULL;
BOOL __stdcall WriteFileHook(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {
    if (hFile == hStdOut || hFile == hStdErr) {
        //フィルタリングするメッセージ
        const char *mes_filter[] = {
            "Found NALU with forbidden_bit set, bit error",
            "read_new_slice: Found NALU_TYPE_FILL,",
            "Skipping these filling bits, proceeding w/ next NALU"
        };
        for (int i = 0; i < _countof(mes_filter); i++) {
            if (0 == strncmp((const char *)lpBuffer, mes_filter[i], strlen(mes_filter[i]))) {
                *lpNumberOfBytesWritten = 0;
                return FALSE;
            }
        }
    }
    return origWriteFileFunc(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

class VCECore::PipelineElementAMFComponent : public PipelineElement {
public:
    PipelineElementAMFComponent(amf::AMFComponentPtr pComponent) :
        m_pComponent(pComponent) {

    }

    virtual ~PipelineElementAMFComponent() {
    }

    virtual AMF_RESULT SubmitInput(amf::AMFData* pData) {
        __try {
            AMF_RESULT res = AMF_OK;
            if (pData == NULL) // EOF
            {
                res = m_pComponent->Drain();
            } else {
                res = m_pComponent->SubmitInput(pData);
                if (res == AMF_DECODER_NO_FREE_SURFACES || res == AMF_INPUT_FULL) {
                    return AMF_INPUT_FULL;
                }
            }
            return res;
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return AMF_FAIL;
        }
    }

    virtual AMF_RESULT QueryOutput(amf::AMFData** ppData) {
        AMF_RESULT res = AMF_OK;
        amf::AMFDataPtr data;
        res = m_pComponent->QueryOutput(&data);
        if (res == AMF_REPEAT) {
            res = AMF_OK;
        }
        if (res == AMF_EOF && data == NULL) {
        }
        if (data != NULL) {
            *ppData = data.Detach();
        }
        return res;
    }
    virtual AMF_RESULT Drain(amf_int32 inputSlot) {
        inputSlot;
        return m_pComponent->Drain();
    }
    virtual amf_int32 GetInputSlotCount() const override {
        return 1;
    }
    virtual amf_int32 GetOutputSlotCount() const override {
        return 1;
    }
protected:
    amf::AMFComponentPtr m_pComponent;
};

class VCECore::PipelineElementEncoder : public PipelineElementAMFComponent {
public:
    PipelineElementEncoder(amf::AMFComponentPtr pComponent,
        ParametersStorage* pParams, amf_int64 frameParameterFreq,
        amf_int64 dynamicParameterFreq, bool bCFR) :
        PipelineElementAMFComponent(pComponent), m_pParams(pParams),
        m_framesSubmitted(0), m_framesQueried(0),
        m_frameParameterFreq(frameParameterFreq),
        m_dynamicParameterFreq(dynamicParameterFreq),
        m_maxLatencyTime(0), m_TotalLatencyTime(0),
        m_maxLatencyFrame(0), m_LastReadyFrameTime(0),
        m_bCFR(bCFR) {

    }

    virtual ~PipelineElementEncoder() {
    }

    virtual AMF_RESULT SubmitInput(amf::AMFData* pData) {
        AMF_RESULT res = AMF_OK;
        if (pData == NULL) // EOF
        {
            res = m_pComponent->Drain();
        } else {
            amf_int64 submitTime = 0;
            amf_int64 currentTime = amf_high_precision_clock();
            if (pData->GetProperty(ENCODER_SUBMIT_TIME, &submitTime) != AMF_OK) {
                pData->SetProperty(ENCODER_SUBMIT_TIME, currentTime);
            }
            amf::AMFSurfacePtr surface(pData);
            //fprintf(stderr, "%dx%d, format: %d, planes: %d, frame type: %d\n",
            //    surface->GetPlaneAt(0)->GetWidth(),
            //    surface->GetPlaneAt(0)->GetHeight(),
            //    surface->GetFormat(),
            //    surface->GetPlanesCount(),
            //    surface->GetFrameType());
            //現状VCEはインタレをサポートしないので、強制的にプログレとして処理する
            surface->SetFrameType(amf::AMF_FRAME_PROGRESSIVE);
            if (m_bCFR) {
                surface->SetPts(m_framesSubmitted);
            }
            if (m_frameParameterFreq != 0 && m_framesSubmitted != 0
                && (m_framesSubmitted % m_frameParameterFreq) == 0) { // apply frame-specific properties to the current frame
                PushParamsToPropertyStorage(m_pParams, ParamEncoderFrame, pData);
            }
            if (m_dynamicParameterFreq != 0 && m_framesSubmitted != 0
                && (m_framesSubmitted % m_dynamicParameterFreq)
                == 0) { // apply dynamic properties to the encoder
                PushParamsToPropertyStorage(m_pParams, ParamEncoderDynamic,
                    m_pComponent);
            }
            //現状VCEはインタレをサポートしないので、強制的にプログレとして処理する
            //フレーム情報のほうもプログレに書き換えなければ、SubmitInputでエラーが返る
            m_pParams->SetParam(AMF_VIDEO_ENCODER_PICTURE_STRUCTURE, AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_TOP_FIELD);
            res = m_pComponent->SubmitInput(pData);
            if (res == AMF_DECODER_NO_FREE_SURFACES || res == AMF_INPUT_FULL) {
                return AMF_INPUT_FULL;
            }
            m_framesSubmitted++;
        }
        return res;
    }

    virtual AMF_RESULT QueryOutput(amf::AMFData** ppData) {
        AMF_RESULT ret = PipelineElementAMFComponent::QueryOutput(ppData);
        if (ret == AMF_OK && *ppData != NULL) {
            amf_int64 currentTime = amf_high_precision_clock();
            amf_int64 submitTime = 0;
            if ((*ppData)->GetProperty(ENCODER_SUBMIT_TIME, &submitTime)
                == AMF_OK) {
                amf_int64 latencyTime = currentTime - AMF_MAX(submitTime,
                    m_LastReadyFrameTime);
                if (m_maxLatencyTime < latencyTime) {
                    m_maxLatencyTime = latencyTime;
                    m_maxLatencyFrame = m_framesQueried;
                }
                m_TotalLatencyTime += latencyTime;
            }
            m_framesQueried++;
            m_LastReadyFrameTime = currentTime;
        }
        return ret;
    }
    virtual AMF_RESULT Drain(amf_int32 inputSlot) {
        inputSlot;
        return m_pComponent->Drain();
    }
    virtual std::wstring GetDisplayResult() {
        std::wstring ret;
        if (m_framesSubmitted > 0) {
            std::wstringstream messageStream;
            messageStream.precision(1);
            messageStream.setf(std::ios::fixed, std::ios::floatfield);
            double averageLatency = double(m_TotalLatencyTime) / 10000.
                / m_framesQueried;
            double maxLatency = double(m_maxLatencyTime) / 10000.;
            messageStream << L" Average (Max, fr#) Encode Latency: "
                << averageLatency << L" ms (" << maxLatency
                << " ms frame# " << m_maxLatencyFrame << L")";
            ret = messageStream.str();
        }
        return ret;
    }
protected:
    ParametersStorage* m_pParams;
    amf_int m_framesSubmitted;
    amf_int m_framesQueried;
    amf_int64 m_frameParameterFreq;
    amf_int64 m_dynamicParameterFreq;
    amf_int64 m_maxLatencyTime;
    amf_int64 m_TotalLatencyTime;
    amf_int64 m_LastReadyFrameTime;
    amf_int m_maxLatencyFrame;
    bool m_bCFR;
};

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
    m_AVChapterFromFile(),
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
    m_outputFps(),
    m_outputTimebase(),
    m_pContext(),
    m_pStreamOut(),
    m_pTrimParam(nullptr),
    m_pDecoder(),
    m_pEncoder(),
    m_pConverter(),
    m_thStreamSender(),
    m_deviceDX9(),
    m_deviceDX11(),
    m_Params(),
    m_apihook() {
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
    PrintMes(RGY_LOG_DEBUG, _T("Stopping pipeline...\n"));
    if (m_thStreamSender.joinable()) {
        m_thStreamSender.join();
    }
    Pipeline::Stop();
    PrintMes(RGY_LOG_DEBUG, _T("Pipeline Stopped.\n"));

    m_pStreamOut = NULL;

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

    m_deviceDX9.Terminate();
    m_deviceDX11.Terminate();

    m_pFileWriterListAudio.clear();
    m_pFileWriter.reset();
    m_AudioReaders.clear();
    m_pFileReader.reset();
    m_pStatus.reset();
    m_pLog.reset();
    m_encCodec = RGY_CODEC_UNKNOWN;
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
    m_AVChapterFromFile.clear();
    const auto& chapter_list = chapter.chapterlist();
    tstring chap_log;
    for (size_t i = 0; i < chapter_list.size(); i++) {
        unique_ptr<AVChapter> avchap(new AVChapter);
        avchap->time_base = av_make_q(1, 1000);
        avchap->start = chapter_list[i]->get_ms();
        avchap->end = (i < chapter_list.size()-1) ? chapter_list[i+1]->get_ms() : avchap->start + 1;
        avchap->id = (int)m_AVChapterFromFile.size();
        avchap->metadata = nullptr;
        av_dict_set(&avchap->metadata, "title", wstring_to_string(chapter_list[i]->name, CP_UTF8).c_str(), 0);
        chap_log += strsprintf(_T("chapter #%02d [%d.%02d.%02d.%03d]: %s.\n"),
            avchap->id, chapter_list[i]->h, chapter_list[i]->m, chapter_list[i]->s, chapter_list[i]->ms,
            wstring_to_tstring(chapter_list[i]->name).c_str());
        m_AVChapterFromFile.push_back(std::move(avchap));
    }
    PrintMes(RGY_LOG_DEBUG, _T("%s"), chap_log.c_str());
    return RGY_ERR_NONE;
#else
    PrintMes(RGY_LOG_ERROR, _T("chater reading unsupportted in this build"));
    return RGY_ERR_UNSUPPORTED;
#endif //#if ENABLE_AVSW_READER
}

RGY_ERR VCECore::initInput(VCEParam *inputParam) {

    int sourceAudioTrackIdStart = 1;    //トラック番号は1スタート
    int sourceSubtitleTrackIdStart = 1; //トラック番号は1スタート
#if ENABLE_RAW_READER
    if (inputParam->input.type == RGY_INPUT_FMT_AUTO) {
        if (check_ext(inputParam->inputFilename, { ".y4m" })) {
            inputParam->input.type = RGY_INPUT_FMT_Y4M;
        } else if (check_ext(inputParam->inputFilename, { ".yuv" })) {
            inputParam->input.type = RGY_INPUT_FMT_RAW;
#if ENABLE_AVI_READER
        } else if (check_ext(inputParam->inputFilename, { ".avi" })) {
            inputParam->input.type = RGY_INPUT_FMT_AVI;
#endif
#if ENABLE_AVISYNTH_READER
        } else if (check_ext(inputParam->inputFilename, { ".avs" })) {
            inputParam->input.type = RGY_INPUT_FMT_AVS;
#endif
#if ENABLE_VAPOURSYNTH_READER
        } else if (check_ext(inputParam->inputFilename, { ".vpy" })) {
            inputParam->input.type = RGY_INPUT_FMT_VPY_MT;
#endif
        } else {
#if ENABLE_AVSW_READER
            inputParam->input.type = RGY_INPUT_FMT_AVANY;
#else
            inputParam->input.type = RGY_INPUT_FMT_RAW;
#endif
        }
    }

    //Check if selected format is enabled
    if (inputParam->input.type == RGY_INPUT_FMT_AVS && !ENABLE_AVISYNTH_READER) {
        PrintMes(RGY_LOG_ERROR, _T("avs reader not compiled in this binary.\n"));
        return RGY_ERR_UNSUPPORTED;
    }
    if (inputParam->input.type == RGY_INPUT_FMT_VPY_MT && !ENABLE_VAPOURSYNTH_READER) {
        PrintMes(RGY_LOG_ERROR, _T("vpy reader not compiled in this binary.\n"));
        return RGY_ERR_UNSUPPORTED;
    }
    if (inputParam->input.type == RGY_INPUT_FMT_AVI && !ENABLE_AVI_READER) {
        PrintMes(RGY_LOG_ERROR, _T("avi reader not compiled in this binary.\n"));
        return RGY_ERR_UNSUPPORTED;
    }
    if (inputParam->input.type == RGY_INPUT_FMT_AVHW && !ENABLE_AVSW_READER) {
        PrintMes(RGY_LOG_ERROR, _T("avcodec + cuvid reader not compiled in this binary.\n"));
        return RGY_ERR_UNSUPPORTED;
    }
    if (inputParam->input.type == RGY_INPUT_FMT_AVSW && !ENABLE_AVSW_READER) {
        PrintMes(RGY_LOG_ERROR, _T("avsw reader not compiled in this binary.\n"));
        return RGY_ERR_UNSUPPORTED;
    }

#if ENABLE_AVSW_READER
    AvcodecReaderPrm inputInfoAVVCE = { 0 };
    DeviceCodecCsp HWDecCodecCsp;
    HWDecCodecCsp.push_back(std::make_pair(0, getHWDecCodecCsp()));
#endif
    void *pInputPrm = nullptr;
    switch (inputParam->input.type) {
#if ENABLE_AVI_READER
    case RGY_INPUT_FMT_AVI:
        PrintMes(RGY_LOG_DEBUG, _T("avi reader selected.\n"));
        m_pFileReader.reset(new RGYInputAvi());
        break;
#endif //ENABLE_AVI_READER
#if ENABLE_AVISYNTH_READER
    case RGY_INPUT_FMT_AVS:
        PrintMes(RGY_LOG_DEBUG, _T("avs reader selected.\n"));
        m_pFileReader.reset(new RGYInputAvs());
        break;
#endif //ENABLE_AVISYNTH_READER
#if ENABLE_VAPOURSYNTH_READER
    case RGY_INPUT_FMT_VPY:
    case RGY_INPUT_FMT_VPY_MT:
        PrintMes(RGY_LOG_DEBUG, _T("vpy reader selected.\n"));
        m_pFileReader.reset(new RGYInputVpy());
        break;
#endif //ENABLE_VAPOURSYNTH_READER
#if ENABLE_AVSW_READER
    case RGY_INPUT_FMT_AVHW:
    case RGY_INPUT_FMT_AVSW:
    case RGY_INPUT_FMT_AVANY:
        inputInfoAVVCE.pInputFormat = inputParam->pAVInputFormat;
        inputInfoAVVCE.bReadVideo = true;
        inputInfoAVVCE.nVideoTrack = inputParam->nVideoTrack;
        inputInfoAVVCE.nVideoStreamId = inputParam->nVideoStreamId;
        inputInfoAVVCE.nReadAudio = inputParam->nAudioSelectCount > 0;
        inputInfoAVVCE.bReadSubtitle = inputParam->nSubtitleSelectCount > 0;
        inputInfoAVVCE.bReadChapter = !!inputParam->bCopyChapter;
        inputInfoAVVCE.nVideoAvgFramerate = std::make_pair(inputParam->input.fpsN, inputParam->input.fpsD);
        inputInfoAVVCE.nAnalyzeSec = inputParam->nAVDemuxAnalyzeSec;
        inputInfoAVVCE.nTrimCount = inputParam->nTrimCount;
        inputInfoAVVCE.pTrimList = inputParam->pTrimList;
        inputInfoAVVCE.nAudioTrackStart = sourceAudioTrackIdStart;
        inputInfoAVVCE.nSubtitleTrackStart = sourceSubtitleTrackIdStart;
        inputInfoAVVCE.nAudioSelectCount = inputParam->nAudioSelectCount;
        inputInfoAVVCE.ppAudioSelect = inputParam->ppAudioSelectList;
        inputInfoAVVCE.nSubtitleSelectCount = inputParam->nSubtitleSelectCount;
        inputInfoAVVCE.pSubtitleSelect = inputParam->pSubtitleSelect;
        inputInfoAVVCE.nProcSpeedLimit = inputParam->nProcSpeedLimit;
        inputInfoAVVCE.nAVSyncMode = RGY_AVSYNC_ASSUME_CFR;
        inputInfoAVVCE.fSeekSec = inputParam->fSeekSec;
        inputInfoAVVCE.pFramePosListLog = inputParam->sFramePosListLog.c_str();
        inputInfoAVVCE.nInputThread = inputParam->nInputThread;
        inputInfoAVVCE.pQueueInfo = (m_pPerfMonitor) ? m_pPerfMonitor->GetQueueInfoPtr() : nullptr;
        inputInfoAVVCE.pHWDecCodecCsp = &HWDecCodecCsp;
        inputInfoAVVCE.bVideoDetectPulldown = /*!inputParam->vpp.rff && !inputParam->vpp.afs.enable && */inputParam->nAVSyncMode == RGY_AVSYNC_ASSUME_CFR;
        inputInfoAVVCE.caption2ass = inputParam->caption2ass;
        pInputPrm = &inputInfoAVVCE;
        PrintMes(RGY_LOG_DEBUG, _T("avhw reader selected.\n"));
        m_pFileReader.reset(new RGYInputAvcodec());
        break;
#endif //#if ENABLE_AVSW_READER
    case RGY_INPUT_FMT_RAW:
    case RGY_INPUT_FMT_Y4M:
    default:
        PrintMes(RGY_LOG_DEBUG, _T("raw/y4m reader selected.\n"));
        m_pFileReader.reset(new RGYInputRaw());
        break;
    }
    PrintMes(RGY_LOG_DEBUG, _T("InitInput: input selected : %d.\n"), inputParam->input.type);

    VideoInfo inputParamCopy = inputParam->input;
    m_pStatus.reset(new EncodeStatus());
    auto ret = m_pFileReader->Init(inputParam->inputFilename.c_str(), &inputParam->input, pInputPrm, m_pLog, m_pStatus);
    if (ret != RGY_ERR_NONE) {
        PrintMes(RGY_LOG_ERROR, m_pFileReader->GetInputMessage());
        return ret;
    }
    sourceAudioTrackIdStart    += m_pFileReader->GetAudioTrackCount();
    sourceSubtitleTrackIdStart += m_pFileReader->GetSubtitleTrackCount();

    //ユーザー指定のオプションを必要に応じて復元する
    inputParam->input.picstruct = inputParamCopy.picstruct;
    if (inputParamCopy.fpsN * inputParamCopy.fpsD > 0) {
        inputParam->input.fpsN = inputParamCopy.fpsN;
        inputParam->input.fpsD = inputParamCopy.fpsD;
    }
    if (inputParamCopy.sar[0] * inputParamCopy.sar[1] > 0) {
        inputParam->input.sar[0] = inputParamCopy.sar[0];
        inputParam->input.sar[1] = inputParamCopy.sar[1];
    }

    double inputFileDuration = 0.0;
    m_inputFps = rgy_rational<int>(inputParam->input.fpsN, inputParam->input.fpsD);
    m_outputTimebase = m_inputFps.inv() * rgy_rational<int>(1, 4);
    auto pAVCodecReader = std::dynamic_pointer_cast<RGYInputAvcodec>(m_pFileReader);
    if (pAVCodecReader) {
        inputFileDuration = pAVCodecReader->GetInputVideoDuration();
        if (m_nAVSyncMode & RGY_AVSYNC_VFR) {
            //avsync vfr時は、入力streamのtimebaseをそのまま使用する
            m_outputTimebase = to_rgy(pAVCodecReader->GetInputVideoStream()->time_base);
        }
    }
    m_outputFps = m_inputFps;
    //if (inputParam->vpp.deinterlace == cudaVideoDeinterlaceMode_Bob) {
    //    outputFps *= 2;
    //}

    //trim情報の作成
    if (m_pFileReader->getInputCodec() == RGY_CODEC_UNKNOWN
        && inputParam->nTrimCount > 0) {
        //avqsvリーダー以外は、trimは自分ではセットされないので、ここでセットする
        sTrimParam trimParam;
        trimParam.list = make_vector(inputParam->pTrimList, inputParam->nTrimCount);
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

    m_pStatus->Init(m_outputFps.n(), m_outputFps.d(), inputParam->input.frames, inputFileDuration, m_trimParam, m_pLog, m_pPerfMonitor);

    if (inputParam->nPerfMonitorSelect || inputParam->nPerfMonitorSelectMatplot) {
        m_pPerfMonitor->SetEncStatus(m_pStatus);
    }
#if ENABLE_AVSW_READER
    if ((m_nAVSyncMode & (RGY_AVSYNC_VFR | RGY_AVSYNC_FORCE_CFR))/* || inputParam->vpp.rff*/) {
        tstring err_target;
        if (m_nAVSyncMode & RGY_AVSYNC_VFR)       err_target += _T("avsync vfr, ");
        if (m_nAVSyncMode & RGY_AVSYNC_FORCE_CFR) err_target += _T("avsync forcecfr, ");
        //if (inputParam->vpp.rff)                  err_target += _T("vpp-rff, ");
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
        } else {
            PrintMes(RGY_LOG_ERROR, _T("%s can only be used with avhw /avsw reader.\n"), err_target.c_str());
            return RGY_ERR_INVALID_VIDEO_PARAM;
        }
    }

    if (inputParam->nAudioSourceCount > 0) {

        for (int i = 0; i < (int)inputParam->nAudioSourceCount; i++) {
            VideoInfo inputInfo = inputParam->input;

            AvcodecReaderPrm inputInfoAVAudioReader = { 0 };
            inputInfoAVAudioReader.bReadVideo = false;
            inputInfoAVAudioReader.nReadAudio = inputParam->nAudioSourceCount > 0;
            inputInfoAVAudioReader.bReadSubtitle = false;
            inputInfoAVAudioReader.bReadChapter = false;
            inputInfoAVAudioReader.nVideoAvgFramerate = std::make_pair(m_pStatus->m_sData.outputFPSRate, m_pStatus->m_sData.outputFPSScale);
            inputInfoAVAudioReader.nAnalyzeSec = inputParam->nAVDemuxAnalyzeSec;
            inputInfoAVAudioReader.nTrimCount = inputParam->nTrimCount;
            inputInfoAVAudioReader.pTrimList = inputParam->pTrimList;
            inputInfoAVAudioReader.nAudioTrackStart = sourceAudioTrackIdStart;
            inputInfoAVAudioReader.nSubtitleTrackStart = sourceSubtitleTrackIdStart;
            inputInfoAVAudioReader.nAudioSelectCount = inputParam->nAudioSelectCount;
            inputInfoAVAudioReader.ppAudioSelect = inputParam->ppAudioSelectList;
            inputInfoAVAudioReader.nProcSpeedLimit = inputParam->nProcSpeedLimit;
            inputInfoAVAudioReader.nAVSyncMode = RGY_AVSYNC_ASSUME_CFR;
            inputInfoAVAudioReader.fSeekSec = inputParam->fSeekSec;
            inputInfoAVAudioReader.pFramePosListLog = inputParam->sFramePosListLog.c_str();
            inputInfoAVAudioReader.nInputThread = 0;

            shared_ptr<RGYInput> audioReader(new RGYInputAvcodec());
            ret = audioReader->Init(inputParam->ppAudioSourceList[i], &inputInfo, &inputInfoAVAudioReader, m_pLog, nullptr);
            if (ret != 0) {
                PrintMes(RGY_LOG_ERROR, audioReader->GetInputMessage());
                return ret;
            }
            sourceAudioTrackIdStart += audioReader->GetAudioTrackCount();
            sourceSubtitleTrackIdStart += audioReader->GetSubtitleTrackCount();
            m_AudioReaders.push_back(std::move(audioReader));
        }
    }
#endif //ENABLE_RAW_READER
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
    if (prm->input.type == RGY_INPUT_FMT_AVHW && cropEnabled(prm->input.crop)) {
        PrintMes(RGY_LOG_ERROR, _T("crop not available with av") DECODER_NAME _T(" reader.\n"));
        return RGY_ERR_UNSUPPORTED;
    }
    prm->input.srcWidth -= (prm->input.crop.e.left + prm->input.crop.e.right);
    prm->input.srcHeight -= (prm->input.crop.e.bottom + prm->input.crop.e.up);
    if (prm->input.srcWidth % 2 != 0) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid input frame size (after crop) - non mod2 (width: %d).\n"), prm->input.srcWidth);
        return RGY_ERR_INVALID_PARAM;
    }
    if (prm->input.srcHeight % h_mul != 0) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid input frame size (after crop) - non mod%d (height: %d).\n"), h_mul, prm->input.srcHeight);
        return RGY_ERR_INVALID_PARAM;
    }
    if (prm->input.dstWidth <= 0) {
        prm->input.dstWidth = prm->input.srcWidth;
    }
    if (prm->input.dstHeight <= 0) {
        prm->input.dstHeight = prm->input.srcHeight;
    }
    if (prm->input.dstWidth % 2 != 0) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid output frame size - non mod2 (width: %d).\n"), prm->input.dstWidth);
        return RGY_ERR_INVALID_PARAM;
    }
    if (prm->input.dstHeight % h_mul != 0) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid output frame size - non mod%d (height: %d).\n"), h_mul, prm->input.dstHeight);
        return RGY_ERR_INVALID_PARAM;
    }
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
    RGY_ERR sts = RGY_ERR_NONE;
    bool stdoutUsed = false;
    const auto outputVideoInfo = videooutputinfo(
        inputParams->codec,
        formatOut,
        m_Params,
        inputParams->input.picstruct,
        inputParams->vui
    );
    HEVCHDRSei hedrsei;
    if (hedrsei.parse(inputParams->sMaxCll, inputParams->sMasterDisplay)) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to parse HEVC HDR10 metadata.\n"));
        return RGY_ERR_UNSUPPORTED;
    }
#if ENABLE_AVSW_READER
    vector<int> streamTrackUsed; //使用した音声/字幕のトラックIDを保存する
    bool useH264ESOutput =
        ((inputParams->AVMuxOutputFormat.length() > 0 && 0 == _tcscmp(inputParams->AVMuxOutputFormat.c_str(), _T("raw")))) //--formatにrawが指定されている
        || (PathFindExtension(inputParams->outputFilename.c_str()) == nullptr || PathFindExtension(inputParams->outputFilename.c_str())[0] != '.') //拡張子がしない
        || check_ext(inputParams->outputFilename.c_str(), { ".m2v", ".264", ".h264", ".avc", ".avc1", ".x264", ".265", ".h265", ".hevc" }); //特定の拡張子
    if (!useH264ESOutput) {
        inputParams->nAVMux |= RGY_MUX_VIDEO;
    }
    //if (inputParams->CodecId == MFX_CODEC_RAW) {
    //    inputParams->nAVMux &= ~RGY_MUX_VIDEO;
    //}


    { auto pAVCodecReader = std::dynamic_pointer_cast<RGYInputAvcodec>(m_pFileReader);
    if (pAVCodecReader != nullptr) {
        //caption2ass用の解像度情報の提供
        //これをしないと入力ファイルのデータをずっとバッファし続けるので注意
        pAVCodecReader->setOutputVideoInfo(outputVideoInfo.dstWidth, outputVideoInfo.dstHeight,
            outputVideoInfo.sar[0], outputVideoInfo.sar[1],
            (inputParams->nAVMux & RGY_MUX_VIDEO) != 0);
    }
    }

    bool audioCopyAll = false;
    if (inputParams->nAVMux & RGY_MUX_VIDEO) {
        PrintMes(RGY_LOG_DEBUG, _T("Output: Using avformat writer.\n"));
        m_pFileWriter = std::make_shared<RGYOutputAvcodec>();
        AvcodecWriterPrm writerPrm;
        writerPrm.pOutputFormat = inputParams->AVMuxOutputFormat.c_str();
        writerPrm.trimList                = m_trimParam.list;
        writerPrm.bVideoDtsUnavailable    = false;
        writerPrm.nOutputThread           = inputParams->nOutputThread;
        writerPrm.nAudioThread            = inputParams->nAudioThread;
        writerPrm.nBufSizeMB              = inputParams->nOutputBufSizeMB;
        writerPrm.nAudioResampler         = inputParams->nAudioResampler;
        writerPrm.nAudioIgnoreDecodeError = inputParams->nAudioIgnoreDecodeError;
        writerPrm.pQueueInfo = (m_pPerfMonitor) ? m_pPerfMonitor->GetQueueInfoPtr() : nullptr;
        writerPrm.pMuxVidTsLogFile        = inputParams->pMuxVidTsLogFile;
        writerPrm.rBitstreamTimebase      = av_make_q(m_outputTimebase);
        writerPrm.pHEVCHdrSei             = &hedrsei;
        if (inputParams->pMuxOpt > 0) {
            writerPrm.vMuxOpt = *inputParams->pMuxOpt;
        }
        auto pAVCodecReader = std::dynamic_pointer_cast<RGYInputAvcodec>(m_pFileReader);
        if (pAVCodecReader != nullptr) {
            writerPrm.pInputFormatMetadata = pAVCodecReader->GetInputFormatMetadata();
            if (inputParams->sChapterFile.length() > 0) {
                //チャプターファイルを読み込む
                auto chap_sts = readChapterFile(inputParams->sChapterFile);
                if (chap_sts != RGY_ERR_NONE) {
                    return chap_sts;
                }
                writerPrm.chapterList.clear();
                for (uint32_t i = 0; i < m_AVChapterFromFile.size(); i++) {
                    writerPrm.chapterList.push_back(m_AVChapterFromFile[i].get());
                }
            } else {
                //入力ファイルのチャプターをコピーする
                writerPrm.chapterList = pAVCodecReader->GetChapterList();
            }
            writerPrm.nVideoInputFirstKeyPts = pAVCodecReader->GetVideoFirstKeyPts();
            writerPrm.pVideoInputStream = pAVCodecReader->GetInputVideoStream();
        }
        if (inputParams->nAVMux & (RGY_MUX_AUDIO | RGY_MUX_SUBTITLE)) {
            PrintMes(RGY_LOG_DEBUG, _T("Output: Audio/Subtitle muxing enabled.\n"));
            for (int i = 0; !audioCopyAll && i < inputParams->nAudioSelectCount; i++) {
                //トラック"0"が指定されていれば、すべてのトラックをコピーするということ
                audioCopyAll = (inputParams->ppAudioSelectList[i]->nAudioSelect == 0);
            }
            PrintMes(RGY_LOG_DEBUG, _T("Output: CopyAll=%s\n"), (audioCopyAll) ? _T("true") : _T("false"));
            pAVCodecReader = std::dynamic_pointer_cast<RGYInputAvcodec>(m_pFileReader);
            vector<AVDemuxStream> streamList;
            if (pAVCodecReader) {
                streamList = pAVCodecReader->GetInputStreamInfo();
            }
            for (const auto& audioReader : m_AudioReaders) {
                if (audioReader->GetAudioTrackCount()) {
                    auto pAVCodecAudioReader = std::dynamic_pointer_cast<RGYInputAvcodec>(audioReader);
                    if (pAVCodecAudioReader) {
                        vector_cat(streamList, pAVCodecAudioReader->GetInputStreamInfo());
                    }
                    //もしavqsvリーダーでないなら、音声リーダーから情報を取得する必要がある
                    if (pAVCodecReader == nullptr) {
                        writerPrm.nVideoInputFirstKeyPts = pAVCodecAudioReader->GetVideoFirstKeyPts();
                        writerPrm.pVideoInputStream = pAVCodecAudioReader->GetInputVideoStream();
                    }
                }
            }

            for (auto& stream : streamList) {
                bool bStreamIsSubtitle = stream.nTrackId < 0;
                const sAudioSelect *pAudioSelect = nullptr;
                for (int i = 0; i < (int)inputParams->nAudioSelectCount; i++) {
                    if (stream.nTrackId == inputParams->ppAudioSelectList[i]->nAudioSelect
                        && inputParams->ppAudioSelectList[i]->pAudioExtractFilename == nullptr) {
                        pAudioSelect = inputParams->ppAudioSelectList[i];
                    }
                }
                if (pAudioSelect == nullptr) {
                    //一致するTrackIDがなければ、nAudioSelect = 0 (全指定)を探す
                    for (int i = 0; i < inputParams->nAudioSelectCount; i++) {
                        if (inputParams->ppAudioSelectList[i]->nAudioSelect == 0
                            && inputParams->ppAudioSelectList[i]->pAudioExtractFilename == nullptr) {
                            pAudioSelect = inputParams->ppAudioSelectList[i];
                        }
                    }
                }
                if (pAudioSelect != nullptr || bStreamIsSubtitle) {
                    streamTrackUsed.push_back(stream.nTrackId);
                    AVOutputStreamPrm prm;
                    prm.src = stream;
                    prm.nBitrate = pAudioSelect->nAVAudioEncodeBitrate;
                    prm.nSamplingRate = pAudioSelect->nAudioSamplingRate;
                    prm.pEncodeCodec = pAudioSelect->pAVAudioEncodeCodec;
                    prm.pEncodeCodecPrm = pAudioSelect->pAVAudioEncodeCodecPrm;
                    prm.pEncodeCodecProfile = pAudioSelect->pAVAudioEncodeCodecProfile;
                    prm.pFilter = pAudioSelect->pAudioFilter;
                    PrintMes(RGY_LOG_DEBUG, _T("Output: Added %s track#%d (stream idx %d) for mux, bitrate %d, codec: %s %s %s\n"),
                        (bStreamIsSubtitle) ? _T("sub") : _T("audio"),
                        stream.nTrackId, stream.nIndex, prm.nBitrate, prm.pEncodeCodec,
                        prm.pEncodeCodecProfile ? prm.pEncodeCodecProfile : _T(""),
                        prm.pEncodeCodecPrm ? prm.pEncodeCodecPrm : _T(""));
                    writerPrm.inputStreamList.push_back(std::move(prm));
                }
            }
        }
        sts = m_pFileWriter->Init(inputParams->outputFilename.c_str(), &outputVideoInfo, &writerPrm, m_pLog, m_pStatus);
        if (sts != 0) {
            PrintMes(RGY_LOG_ERROR, m_pFileWriter->GetOutputMessage());
            return sts;
        } else if (inputParams->nAVMux & (RGY_MUX_AUDIO | RGY_MUX_SUBTITLE)) {
            m_pFileWriterListAudio.push_back(m_pFileWriter);
        }
        stdoutUsed = m_pFileWriter->outputStdout();
        PrintMes(RGY_LOG_DEBUG, _T("Output: Initialized avformat writer%s.\n"), (stdoutUsed) ? _T("using stdout") : _T(""));
    } else if (inputParams->nAVMux & (RGY_MUX_AUDIO | RGY_MUX_SUBTITLE)) {
        PrintMes(RGY_LOG_ERROR, _T("Audio mux cannot be used alone, should be use with video mux.\n"));
        return RGY_ERR_INVALID_PARAM;
    } else {
#endif //ENABLE_AVSW_READER
        m_pFileWriter = std::make_shared<RGYOutputRaw>();
        RGYOutputRawPrm rawPrm;
        rawPrm.nBufSizeMB = inputParams->nOutputBufSizeMB;
        rawPrm.bBenchmark = false;
        rawPrm.codecId = inputParams->codec;
        rawPrm.seiNal = hedrsei.gen_nal();
        sts = m_pFileWriter->Init(inputParams->outputFilename.c_str(), &outputVideoInfo, &rawPrm, m_pLog, m_pStatus);
        if (sts != 0) {
            PrintMes(RGY_LOG_ERROR, m_pFileWriter->GetOutputMessage());
            return sts;
        }
        stdoutUsed = m_pFileWriter->outputStdout();
        PrintMes(RGY_LOG_DEBUG, _T("Output: Initialized bitstream writer%s.\n"), (stdoutUsed) ? _T("using stdout") : _T(""));
#if ENABLE_AVSW_READER
    }

    //音声の抽出
    if (inputParams->nAudioSelectCount + inputParams->nSubtitleSelectCount - (audioCopyAll ? 1 : 0) > (int)streamTrackUsed.size()) {
        PrintMes(RGY_LOG_DEBUG, _T("Output: Audio file output enabled.\n"));
        auto pAVCodecReader = std::dynamic_pointer_cast<RGYInputAvcodec>(m_pFileReader);
        if (pAVCodecReader == nullptr) {
            PrintMes(RGY_LOG_ERROR, _T("Audio output is only supported with transcoding (avhw/avsw reader).\n"));
            return RGY_ERR_INVALID_CALL;
        } else {
            auto inutAudioInfoList = pAVCodecReader->GetInputStreamInfo();
            for (auto& audioTrack : inutAudioInfoList) {
                bool bTrackAlreadyUsed = false;
                for (auto usedTrack : streamTrackUsed) {
                    if (usedTrack == audioTrack.nTrackId) {
                        bTrackAlreadyUsed = true;
                        PrintMes(RGY_LOG_DEBUG, _T("Audio track #%d is already set to be muxed, so cannot be extracted to file.\n"), audioTrack.nTrackId);
                        break;
                    }
                }
                if (bTrackAlreadyUsed) {
                    continue;
                }
                const sAudioSelect *pAudioSelect = nullptr;
                for (int i = 0; i < (int)inputParams->nAudioSelectCount; i++) {
                    if (audioTrack.nTrackId == inputParams->ppAudioSelectList[i]->nAudioSelect
                        && inputParams->ppAudioSelectList[i]->pAudioExtractFilename != nullptr) {
                        pAudioSelect = inputParams->ppAudioSelectList[i];
                    }
                }
                if (pAudioSelect == nullptr) {
                    PrintMes(RGY_LOG_ERROR, _T("Audio track #%d is not used anyware, this should not happen.\n"), audioTrack.nTrackId);
                    return RGY_ERR_UNKNOWN;
                }
                PrintMes(RGY_LOG_DEBUG, _T("Output: Output audio track #%d (stream index %d) to \"%s\", format: %s, codec %s, bitrate %d\n"),
                    audioTrack.nTrackId, audioTrack.nIndex, pAudioSelect->pAudioExtractFilename, pAudioSelect->pAudioExtractFormat, pAudioSelect->pAVAudioEncodeCodec, pAudioSelect->nAVAudioEncodeBitrate);

                AVOutputStreamPrm prm;
                prm.src = audioTrack;
                //pAudioSelect == nullptrは "copyAll" によるもの
                prm.nBitrate = pAudioSelect->nAVAudioEncodeBitrate;
                prm.pFilter = pAudioSelect->pAudioFilter;
                prm.pEncodeCodec = pAudioSelect->pAVAudioEncodeCodec;
                prm.nSamplingRate = pAudioSelect->nAudioSamplingRate;

                AvcodecWriterPrm writerAudioPrm;
                writerAudioPrm.nOutputThread   = inputParams->nOutputThread;
                writerAudioPrm.nAudioThread    = inputParams->nAudioThread;
                writerAudioPrm.nBufSizeMB      = inputParams->nOutputBufSizeMB;
                writerAudioPrm.pOutputFormat   = pAudioSelect->pAudioExtractFormat;
                writerAudioPrm.nAudioIgnoreDecodeError = inputParams->nAudioIgnoreDecodeError;
                writerAudioPrm.nAudioResampler = inputParams->nAudioResampler;
                writerAudioPrm.inputStreamList.push_back(prm);
                writerAudioPrm.trimList = m_trimParam.list;
                writerAudioPrm.nVideoInputFirstKeyPts = pAVCodecReader->GetVideoFirstKeyPts();
                writerAudioPrm.pVideoInputStream = pAVCodecReader->GetInputVideoStream();
                writerAudioPrm.rBitstreamTimebase = av_make_q(m_outputTimebase);

                shared_ptr<RGYOutput> pWriter = std::make_shared<RGYOutputAvcodec>();
                sts = pWriter->Init(pAudioSelect->pAudioExtractFilename, &outputVideoInfo, &writerAudioPrm, m_pLog, m_pStatus);
                if (sts != 0) {
                    PrintMes(RGY_LOG_ERROR, pWriter->GetOutputMessage());
                    return sts;
                }
                PrintMes(RGY_LOG_DEBUG, _T("Output: Intialized audio output for track #%d.\n"), audioTrack.nTrackId);
                bool audioStdout = pWriter->outputStdout();
                if (stdoutUsed && audioStdout) {
                    PrintMes(RGY_LOG_ERROR, _T("Multiple stream outputs are set to stdout, please remove conflict.\n"));
                    return RGY_ERR_INVALID_CALL;
                }
                stdoutUsed |= audioStdout;
                m_pFileWriterListAudio.push_back(std::move(pWriter));
            }
        }
    }
#endif //ENABLE_AVSW_READER
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::initDeviceDX9(VCEParam *prm) {
    AMF_RESULT res = AMF_OK;
    const auto inInfo = m_pFileReader->GetInputFrameInfo();
    if (AMF_OK != (res = m_deviceDX9.Init(true, prm->deviceID, false, inInfo.srcWidth, inInfo.srcHeight))) {
        PrintMes(RGY_LOG_DEBUG, _T("m_deviceDX9.Init() failed: %s.\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("initialized DX9 device.\n"));
    if (AMF_OK != (res = m_pContext->InitDX9(m_deviceDX9.GetDevice()))) {
        PrintMes(RGY_LOG_DEBUG, _T("InitDX9: fail: %s.\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("InitDX9: success.\n"));
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::initDeviceDX11(VCEParam *prm) {
    AMF_RESULT res = AMF_OK;
    if (AMF_OK != (res = m_deviceDX11.Init(prm->deviceID, false))) {
        PrintMes(RGY_LOG_DEBUG, _T("m_deviceDX11.Init() failed: %s.\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("initialized DX11 device.\n"));
    if (AMF_OK != (res = m_pContext->InitDX11(m_deviceDX11.GetDevice()))) {
        PrintMes(RGY_LOG_DEBUG, _T("InitDX11: fail: %s.\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("InitDX11: success.\n"));
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::initDevice(VCEParam *prm) {
    RGY_ERR err = RGY_ERR_NONE;
    if (prm->memoryTypeIn == amf::AMF_MEMORY_UNKNOWN
        || prm->memoryTypeIn == amf::AMF_MEMORY_DX9) {
        if (RGY_ERR_NONE == (err = initDeviceDX9(prm))) {
            PrintMes(RGY_LOG_DEBUG, _T("initialized context for DX9.\n"));
            prm->memoryTypeIn = amf::AMF_MEMORY_DX9;
            return RGY_ERR_NONE;
        }
        if (prm->memoryTypeIn == amf::AMF_MEMORY_DX9) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to initialize DX9 device: %s.\n"), get_err_mes(err));
            return err;
        }
        PrintMes(RGY_LOG_DEBUG, _T("Failed to initialize DX9 device (%s), try DX11.\n"), get_err_mes(err));
    }
    if (prm->memoryTypeIn == amf::AMF_MEMORY_UNKNOWN
        || prm->memoryTypeIn == amf::AMF_MEMORY_DX11) {
        if (RGY_ERR_NONE == (err = initDeviceDX11(prm))) {
            PrintMes(RGY_LOG_DEBUG, _T("initialized context for DX11.\n"));
            prm->memoryTypeIn = amf::AMF_MEMORY_DX11;
            return RGY_ERR_NONE;
        }
        PrintMes(RGY_LOG_ERROR, _T("Failed to initialize DX11 device: %s.\n"), get_err_mes(err));
        return err;
    }
    PrintMes(RGY_LOG_ERROR, _T("Unsupported memory type.\n"));
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
        PrintMes(RGY_LOG_ERROR, _T("Input codec \"%s\" not supported.\n"), CodecToStr(inputCodec));
        return RGY_ERR_UNSUPPORTED;
    }
    if (inputCodec == RGY_CODEC_HEVC && prm->input.csp == RGY_CSP_P010) {
        codec_uvd_name = AMFVideoDecoderHW_H265_MAIN10;
    }
    PrintMes(RGY_LOG_DEBUG, _T("decoder: use codec \"%s\".\n"), wstring_to_tstring(codec_uvd_name).c_str());
    auto res = g_AMFFactory.GetFactory()->CreateComponent(m_pContext, codec_uvd_name, &m_pDecoder);
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
        wstring_to_tstring(g_AMFFactory.GetTrace()->SurfaceGetFormatName(csp_rgy_to_enc(prm->input.csp))).c_str());
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
    auto res = g_AMFFactory.GetFactory()->CreateComponent(m_pContext, AMFVideoConverter, &m_pConverter);
    if (res != AMF_OK) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to create converter context: %s\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("created converter context.\n"));

    res = m_pConverter->SetProperty(AMF_VIDEO_CONVERTER_MEMORY_TYPE, prm->memoryTypeIn);
    res = m_pConverter->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_FORMAT, formatOut);
    res = m_pConverter->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_SIZE, AMFConstructSize(prm->input.dstWidth, prm->input.dstHeight));
    res = m_pConverter->SetProperty(AMF_VIDEO_CONVERTER_SCALE, AMF_VIDEO_CONVERTER_SCALE_BICUBIC);
    PrintMes(RGY_LOG_DEBUG, _T("initialize converter by mem type %s, format out %s, output size %dx%x.\n"),
        wstring_to_tstring(g_AMFFactory.GetTrace()->GetMemoryTypeName(prm->memoryTypeIn)).c_str(),
        wstring_to_tstring(g_AMFFactory.GetTrace()->SurfaceGetFormatName(formatOut)).c_str(),
        prm->input.dstWidth, prm->input.dstHeight);
    if (AMF_OK != (res = m_pConverter->Init(formatOut, prm->input.srcWidth, prm->input.srcHeight))) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to init converter: %s\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("initialized converter.\n"));
    return RGY_ERR_NONE;
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
                str += wstring_to_tstring(g_AMFFactory.GetTrace()->SurfaceGetFormatName(format)) + ((native) ? _T("(native)") : _T(""));
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
                    str += wstring_to_tstring(g_AMFFactory.GetTrace()->GetMemoryTypeName(memType)) + ((native) ? _T("(native)") : _T(""));
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

RGY_ERR VCECore::initEncoder(VCEParam *prm) {
    AMF_RESULT res = AMF_OK;

    if (m_pLog->getLogLevel() <= RGY_LOG_DEBUG) {
        TCHAR cpuInfo[256] = { 0 };
        TCHAR gpu_info[1024] = { 0 };
        std::wstring deviceName = (m_deviceDX9.GetDevice() == nullptr) ? m_deviceDX11.GetDisplayDeviceName() : m_deviceDX9.GetDisplayDeviceName();
        deviceName = str_replace(deviceName, L" (TM)", L"");
        deviceName = str_replace(deviceName, L" (R)", L"");
        deviceName = str_replace(deviceName, L" Series", L"");
        deviceName = str_replace(deviceName, L" Graphics", L"");
        getCPUInfo(cpuInfo, _countof(cpuInfo));
        getGPUInfo("Advanced Micro Devices", gpu_info, _countof(gpu_info));
        PrintMes(RGY_LOG_DEBUG, _T("VCEEnc    %s (%s)\n"), VER_STR_FILEVERSION_TCHAR, BUILD_ARCH_STR);
        PrintMes(RGY_LOG_DEBUG, _T("OS        %s (%s)\n"), getOSVersion().c_str(), rgy_is_64bit_os() ? _T("x64") : _T("x86"));
        PrintMes(RGY_LOG_DEBUG, _T("CPU Info  %s\n"), cpuInfo);
        PrintMes(RGY_LOG_DEBUG, _T("GPU Info  %s [%s]\n"), wstring_to_string(deviceName).c_str(), gpu_info);
    }

    m_encCodec = prm->codec;
    const amf::AMF_SURFACE_FORMAT formatIn = amf::AMF_SURFACE_NV12;
    if (AMF_OK != (res = g_AMFFactory.GetFactory()->CreateComponent(m_pContext, codec_rgy_to_enc(prm->codec), &m_pEncoder))) {
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
            if (prm->input.srcWidth < (uint32_t)minWidth || (uint32_t)maxWidth < prm->input.srcWidth) {
                PrintMes(RGY_LOG_ERROR, _T("Input width should be in range of %d - %d (%d specified).\n"),
                    minWidth, maxWidth, prm->input.srcWidth);
                return RGY_ERR_UNSUPPORTED;
            }

            int minHeight, maxHeight;
            inputCaps->GetHeightRange(&minHeight, &maxHeight);
            if (prm->input.srcHeight < (uint32_t)minHeight || (uint32_t)maxHeight < prm->input.srcHeight) {
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
                PrintMes(RGY_LOG_ERROR, _T("Input format %s not supported on this platform.\n"), g_AMFFactory.GetTrace()->SurfaceGetFormatName(formatIn));
                return RGY_ERR_UNSUPPORTED;
            }
        }

        amf::AMFIOCapsPtr outputCaps;
        if (encoderCaps->GetOutputCaps(&outputCaps) == AMF_OK) {
            int minWidth, maxWidth;
            outputCaps->GetWidthRange(&minWidth, &maxWidth);
            if (prm->input.dstWidth < (uint32_t)minWidth || (uint32_t)maxWidth < prm->input.dstWidth) {
                PrintMes(RGY_LOG_ERROR, _T("Output width should be in range of %d - %d (%d specified).\n"),
                    minWidth, maxWidth, prm->input.dstWidth);
                return RGY_ERR_UNSUPPORTED;
            }

            int minHeight, maxHeight;
            outputCaps->GetHeightRange(&minHeight, &maxHeight);
            if (prm->input.dstHeight < (uint32_t)minHeight || (uint32_t)maxHeight < prm->input.dstHeight) {
                PrintMes(RGY_LOG_ERROR, _T("Output height should be in range of %d - %d (%d specified).\n"),
                    minHeight, maxHeight, prm->input.dstHeight);
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
                PrintMes(RGY_LOG_ERROR, _T("Output format %s not supported on this platform.\n"), g_AMFFactory.GetTrace()->SurfaceGetFormatName(formatIn));
                return RGY_ERR_UNSUPPORTED;
            }
        }
    }

    m_Params.SetParamDescription(VCE_PARAM_KEY_INPUT,         ParamCommon, L"Input file name", NULL);
    m_Params.SetParamDescription(VCE_PARAM_KEY_INPUT_WIDTH,   ParamCommon, L"Input Frame width (integer, default = 0)", ParamConverterInt64);
    m_Params.SetParamDescription(VCE_PARAM_KEY_INPUT_HEIGHT,  ParamCommon, L"Input Frame height (integer, default = 0)", ParamConverterInt64);
    m_Params.SetParamDescription(VCE_PARAM_KEY_OUTPUT,        ParamCommon, L"Output file name", NULL);
    m_Params.SetParamDescription(VCE_PARAM_KEY_OUTPUT_WIDTH,  ParamCommon, L"Output Frame width (integer, default = 0)", ParamConverterInt64);
    m_Params.SetParamDescription(VCE_PARAM_KEY_OUTPUT_HEIGHT, ParamCommon, L"Output Frame height (integer, default = 0)", ParamConverterInt64);
    m_Params.SetParamDescription(VCE_PARAM_KEY_ENGINE,        ParamCommon, L"Specifies decoder/encoder engine type (DX9, DX11)", NULL);
    m_Params.SetParamDescription(VCE_PARAM_KEY_ADAPTERID,     ParamCommon, L"Specifies adapter ID (integer, default = 0)", ParamConverterInt64);
    m_Params.SetParamDescription(VCE_PARAM_KEY_CAPABILITY,    ParamCommon, L"Enable/Disable to display the device capabilities (true, false default =  false)", ParamConverterBoolean);

    switch (prm->codec) {
    case RGY_CODEC_H264:
        RegisterEncoderParamsAVC(&m_Params);
        break;
    case RGY_CODEC_HEVC:
        RegisterEncoderParamsHEVC(&m_Params);
        break;
    default:
        PrintMes(RGY_LOG_ERROR, _T("Unknown Codec.\n"));
        return RGY_ERR_UNSUPPORTED;
    }

    m_Params.SetParamAsString(VCE_PARAM_KEY_INPUT,     tchar_to_wstring(prm->inputFilename));
    m_Params.SetParamAsString(VCE_PARAM_KEY_OUTPUT,    tchar_to_wstring(prm->outputFilename));
    m_Params.SetParam(VCE_PARAM_KEY_ADAPTERID, (amf_int64)0);

    int nGOPLen = prm->nGOPLen;
    if (nGOPLen == 0) {
        nGOPLen = (int)(m_outputFps.n() / (double)m_outputFps.d() + 0.5) * 10;
    }
    //VCEにはlevelを自動で設定してくれる機能はないようで、"0"などとするとエラー終了してしまう。
    if (prm->codecParam[prm->codec].nLevel == 0 || prm->nMaxBitrate == 0) {
        int level = prm->codecParam[prm->codec].nLevel;
        int max_bitrate_kbps = prm->nMaxBitrate;
        int vbv_bufsize_kbps = prm->nVBVBufferSize;
        if (prm->codec == RGY_CODEC_H264) {
            const int profile = prm->codecParam[prm->codec].nProfile;
            if (level == 0) {
                level = calc_h264_auto_level(prm->input.dstWidth, prm->input.dstHeight, prm->nRefFrames, false,
                    m_outputFps.n(), m_outputFps.d(), profile, max_bitrate_kbps, vbv_bufsize_kbps);
            }
            get_h264_vbv_value(&max_bitrate_kbps, &vbv_bufsize_kbps, level, profile);
        } else if (prm->codec == RGY_CODEC_HEVC) {
            const bool high_tier = prm->codecParam[prm->codec].nTier == AMF_VIDEO_ENCODER_HEVC_TIER_HIGH;
            if (level == 0) {
                level = calc_hevc_auto_level(prm->input.dstWidth, prm->input.dstHeight, //m_stEncConfig.encodeCodecConfig.hevcConfig.maxNumRefFramesInDPB,
                    m_outputFps.n(), m_outputFps.d(), high_tier, max_bitrate_kbps);
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

    m_Params.SetParam(VCE_PARAM_KEY_INPUT_WIDTH,   (amf_int64)prm->input.srcWidth);
    m_Params.SetParam(VCE_PARAM_KEY_INPUT_HEIGHT,  (amf_int64)prm->input.srcHeight);
    m_Params.SetParam(VCE_PARAM_KEY_OUTPUT_WIDTH,  (amf_int64)prm->input.dstWidth);
    m_Params.SetParam(VCE_PARAM_KEY_OUTPUT_HEIGHT, (amf_int64)prm->input.dstHeight);
    m_Params.SetParam(VCE_PARAM_KEY_CAPABILITY,    false);
    m_Params.SetParam(SETFRAMEPARAMFREQ_PARAM_NAME,   0);
    m_Params.SetParam(SETDYNAMICPARAMFREQ_PARAM_NAME, 0);

    m_Params.SetParam(AMF_PARAM_FRAMESIZE(prm->codec),      AMFConstructSize(prm->input.dstWidth, prm->input.dstHeight));
    m_Params.SetParam(AMF_PARAM_FRAMERATE(prm->codec),      AMFConstructRate(m_outputFps.n(), m_outputFps.d()));
    m_Params.SetParam(AMF_PARAM_ASPECT_RATIO(prm->codec),   AMFConstructRatio(prm->input.sar[0], prm->input.sar[1]));
    m_Params.SetParam(AMF_PARAM_USAGE(prm->codec),          (amf_int64)((prm->codec == RGY_CODEC_HEVC) ? AMF_VIDEO_ENCODER_HEVC_USAGE_TRANSCONDING : AMF_VIDEO_ENCODER_USAGE_TRANSCONDING));
    m_Params.SetParam(AMF_PARAM_PROFILE(prm->codec),        (amf_int64)prm->codecParam[prm->codec].nProfile);
    m_Params.SetParam(AMF_PARAM_PROFILE_LEVEL(prm->codec),  (amf_int64)prm->codecParam[prm->codec].nLevel);
    m_Params.SetParam(AMF_PARAM_QUALITY_PRESET(prm->codec), (amf_int64)get_quality_preset(prm->codec)[prm->qualityPreset]);
    m_Params.SetParam(AMF_PARAM_QP_I(prm->codec),           (amf_int64)prm->nQPI);
    m_Params.SetParam(AMF_PARAM_QP_P(prm->codec),           (amf_int64)prm->nQPP);
    m_Params.SetParam(AMF_PARAM_TARGET_BITRATE(prm->codec), (amf_int64)prm->nBitrate * 1000);
    m_Params.SetParam(AMF_PARAM_PEAK_BITRATE(prm->codec),   (amf_int64)prm->nMaxBitrate * 1000);
    m_Params.SetParam(AMF_PARAM_MAX_NUM_REFRAMES(prm->codec), (amf_int64)prm->nRefFrames);
    m_Params.SetParam(AMF_PARAM_MAX_LTR_FRAMES(prm->codec), (amf_int64)prm->nLTRFrames);
    m_Params.SetParam(AMF_PARAM_RATE_CONTROL_SKIP_FRAME_ENABLE(prm->codec),  prm->bEnableSkipFrame);
    m_Params.SetParam(AMF_PARAM_RATE_CONTROL_METHOD(prm->codec),             (amf_int64)prm->rateControl);
    m_Params.SetParam(AMF_PARAM_RATE_CONTROL_PREANALYSIS_ENABLE(prm->codec), prm->preAnalysis);
    m_Params.SetParam(AMF_PARAM_VBV_BUFFER_SIZE(prm->codec),                 (amf_int64)prm->nVBVBufferSize * 1000);
    m_Params.SetParam(AMF_PARAM_INITIAL_VBV_BUFFER_FULLNESS(prm->codec),     (amf_int64)prm->nInitialVBVPercent);

    m_Params.SetParam(AMF_PARAM_ENFORCE_HRD(prm->codec),        prm->bEnforceHRD != 0);
    m_Params.SetParam(AMF_PARAM_FILLER_DATA_ENABLE(prm->codec), prm->bFiller != 0);
    if (prm->bVBAQ) m_Params.SetParam(AMF_PARAM_ENABLE_VBAQ(prm->codec), true);
    m_Params.SetParam(AMF_PARAM_SLICES_PER_FRAME(prm->codec),               (amf_int64)prm->nSlices);
    m_Params.SetParam(AMF_PARAM_GOP_SIZE(prm->codec),                       (amf_int64)nGOPLen);

    //m_Params.SetParam(AMF_PARAM_END_OF_SEQUENCE(prm->codec),                false);
    m_Params.SetParam(AMF_PARAM_INSERT_AUD(prm->codec),                     false);
    if (prm->codec == RGY_CODEC_H264) {
        m_Params.SetParam(AMF_VIDEO_ENCODER_SCANTYPE,           (amf_int64)((prm->input.picstruct & RGY_PICSTRUCT_INTERLACED) ? AMF_VIDEO_ENCODER_SCANTYPE_INTERLACED : AMF_VIDEO_ENCODER_SCANTYPE_PROGRESSIVE));

        m_Params.SetParam(AMF_VIDEO_ENCODER_B_PIC_PATTERN, (amf_int64)prm->nBframes);
        if (prm->nBframes > 0) {
            m_Params.SetParam(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP, (amf_int64)prm->nDeltaQPBFrame);
            m_Params.SetParam(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, (amf_int64)prm->nDeltaQPBFrameRef);
            m_Params.SetParam(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, prm->nBframes > 0 && !!prm->bBPyramid);
            m_Params.SetParam(AMF_VIDEO_ENCODER_QP_B, (amf_int64)prm->nQPB);
        }

        m_Params.SetParam(AMF_VIDEO_ENCODER_MIN_QP,                                (amf_int64)prm->nQPMin);
        m_Params.SetParam(AMF_VIDEO_ENCODER_MAX_QP,                                (amf_int64)prm->nQPMax);

        //m_Params.SetParam(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING,       (amf_int64)0);
        ////m_Params.SetParam(AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT, false);

        m_Params.SetParam(AMF_PARAM_MOTION_HALF_PIXEL(prm->codec),              !!(prm->nMotionEst & VCE_MOTION_EST_HALF));
        m_Params.SetParam(AMF_PARAM_MOTION_QUARTERPIXEL(prm->codec),            !!(prm->nMotionEst & VCE_MOTION_EST_QUATER));

        //m_Params.SetParam(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE,             (amf_int64)AMF_VIDEO_ENCODER_PICTURE_TYPE_NONE);
        m_Params.SetParam(AMF_VIDEO_ENCODER_INSERT_SPS, false);
        m_Params.SetParam(AMF_VIDEO_ENCODER_INSERT_PPS, false);
        //m_Params.SetParam(AMF_VIDEO_ENCODER_PICTURE_STRUCTURE,                (amf_int64)prm->nPicStruct);
        //m_Params.SetParam(AMF_VIDEO_ENCODER_MARK_CURRENT_WITH_LTR_INDEX,    false);
        //m_Params.SetParam(AMF_VIDEO_ENCODER_FORCE_LTR_REFERENCE_BITFIELD,   (amf_int64)0);

        //m_Params.SetParam(AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE, AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM);
        //m_Params.SetParam(AMF_VIDEO_ENCODER_OUTPUT_MARKED_LTR_INDEX, (amf_int64)-1);
        //m_Params.SetParam(AMF_VIDEO_ENCODER_OUTPUT_REFERENCED_LTR_INDEX_BITFIELD, (amf_int64)0);
        if (prm->vui.fullrange) {
            m_Params.SetParam(AMF_VIDEO_ENCODER_FULL_RANGE_COLOR, true);
        }
    } else if (prm->codec == RGY_CODEC_HEVC) {
        m_Params.SetParam(AMF_VIDEO_ENCODER_HEVC_TIER,                            (amf_int64)prm->codecParam[prm->codec].nTier);

        m_Params.SetParam(AMF_VIDEO_ENCODER_HEVC_MIN_QP_I,                        (amf_int64)prm->nQPMin);
        m_Params.SetParam(AMF_VIDEO_ENCODER_HEVC_MAX_QP_I,                        (amf_int64)prm->nQPMax);
        m_Params.SetParam(AMF_VIDEO_ENCODER_HEVC_MIN_QP_P,                        (amf_int64)prm->nQPMin);
        m_Params.SetParam(AMF_VIDEO_ENCODER_HEVC_MAX_QP_P,                        (amf_int64)prm->nQPMax);

        m_Params.SetParam(AMF_VIDEO_ENCODER_HEVC_DE_BLOCKING_FILTER_DISABLE,      !prm->bDeblockFilter);

        m_Params.SetParam(AMF_VIDEO_ENCODER_HEVC_INSERT_HEADER,                   true);
    } else {
        PrintMes(RGY_LOG_ERROR, _T("Unsupported codec.\n"));
        return RGY_ERR_UNSUPPORTED;
    }

    // Usage is preset that will set many parameters
    PushParamsToPropertyStorage(&m_Params, ParamEncoderUsage, m_pEncoder);
    PrintMes(RGY_LOG_DEBUG, _T("pushed usage params.\n"));
    // override some usage parameters
    PushParamsToPropertyStorage(&m_Params, ParamEncoderStatic, m_pEncoder);
    PrintMes(RGY_LOG_DEBUG, _T("pushed static params.\n"));

    if (AMF_OK != (res = m_pEncoder->Init(formatIn, prm->input.dstWidth, prm->input.dstHeight))) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to initalize encoder: %s.\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("initalized encoder.\n"));

    PushParamsToPropertyStorage(&m_Params, ParamEncoderDynamic, m_pEncoder);
    PrintMes(RGY_LOG_DEBUG, _T("pushed dynamic params.\n"));
    return RGY_ERR_NONE;
}

RGY_ERR VCECore::init(VCEParam *prm) {
    Terminate();
#if !VCE_AUO
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    hStdErr = GetStdHandle(STD_ERROR_HANDLE);
    m_apihook.hook(_T("kernel32.dll"), "WriteFile", WriteFileHook, (void **)&origWriteFileFunc);
#endif

    AMF_RESULT res = g_AMFFactory.Init();
    if (res != AMF_OK) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to initalize VCE: %s"), AMFRetString(res));
        return err_to_rgy(res);
    }

    res = g_AMFFactory.GetFactory()->CreateContext(&m_pContext);
    if (res != AMF_OK) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to create AMF Context: %s.\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("Created AMF Context.\n"));

    if (prm->bTimerPeriodTuning) {
        m_bTimerPeriodTuning = true;
        timeBeginPeriod(1);
        PrintMes(RGY_LOG_DEBUG, _T("timeBeginPeriod(1)\n"));
    }

    m_pLog.reset(new RGYLog(prm->logfile.c_str(), prm->loglevel));
    if (prm->logfile.length() > 0) {
        m_pLog->writeFileHeader(prm->outputFilename.c_str());
    }
    if (!m_pStatus) {
        m_pStatus = std::make_shared<EncodeStatus>();
    }

    prm->input.csp = RGY_CSP_NV12;

    RGY_ERR ret = RGY_ERR_NONE;
    if (RGY_ERR_NONE != (ret = initInput(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = checkParam(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initDevice(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initDecoder(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initConverter(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initEncoder(prm))) {
        return ret;
    }

    if (RGY_ERR_NONE != (ret = initOutput(prm))) {
        return ret;
    }

    // Connect pipeline
    if (AMF_OK != (res = Connect(PipelineElementPtr(new VCEInput(m_pFileReader, m_pContext)), 4, CT_Direct))) {
        PrintMes(RGY_LOG_ERROR, _T("failed to connect input to pipeline: %s\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    if (m_pDecoder) {
        if (AMF_OK != (res = Connect(PipelineElementPtr(new PipelineElementAMFComponent(m_pDecoder)), 4, CT_Direct))) {
            PrintMes(RGY_LOG_ERROR, _T("failed to connect deocder to pipeline: %s\n"), AMFRetString(res));
            return err_to_rgy(res);
        }
    }
    if (m_pConverter) {
        if (AMF_OK != (res = Connect(PipelineElementPtr(new PipelineElementAMFComponent(m_pConverter)), 4, CT_Direct))) {
            PrintMes(RGY_LOG_ERROR, _T("failed to connect converter to pipeline: %s\n"), AMFRetString(res));
            return err_to_rgy(res);
        }
    }
    if (AMF_OK != (res = Connect(PipelineElementPtr(new PipelineElementEncoder(m_pEncoder, &m_Params, 0, 0, true)), 30, CT_Direct))) {
        PrintMes(RGY_LOG_ERROR, _T("failed to connect encoder to pipeline: %s\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    if (AMF_OK != (res = Connect(PipelineElementPtr(new VCEOutput(m_pFileWriter, m_outputFps, m_outputTimebase)), 10, CT_ThreadPoll))) {
        PrintMes(RGY_LOG_ERROR, _T("failed to connect output to pipeline: %s\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("connected elements to pipeline.\n"));

    return ret;
}

RGY_ERR VCECore::run() {
    m_pStatus->SetStart();
    if (m_pFileWriterListAudio.size() > 0) {
#if ENABLE_AVSW_READER
        m_thStreamSender = std::thread([this](){
            //streamのindexから必要なwriteへのポインタを返すテーブルを作成
            std::map<int, shared_ptr<RGYOutputAvcodec>> pWriterForAudioStreams;
            for (auto pWriter : m_pFileWriterListAudio) {
                auto pAVCodecWriter = std::dynamic_pointer_cast<RGYOutputAvcodec>(pWriter);
                if (pAVCodecWriter) {
                    auto trackIdList = pAVCodecWriter->GetStreamTrackIdList();
                    for (auto trackID : trackIdList) {
                        pWriterForAudioStreams[trackID] = pAVCodecWriter;
                    }
                }
            }
            AMF_RESULT sts = AMF_OK;
            PipelineState state = PipelineStateRunning;
            while ((state = GetState()) == PipelineStateRunning) {
                auto pAVCodecReader = std::dynamic_pointer_cast<RGYInputAvcodec>(m_pFileReader);
                vector<AVPacket> packetList;
                if (pAVCodecReader != nullptr) {
                    packetList = pAVCodecReader->GetStreamDataPackets();
                }
                //音声ファイルリーダーからのトラックを結合する
                for (const auto& reader : m_AudioReaders) {
                    auto pReader = std::dynamic_pointer_cast<RGYInputAvcodec>(reader);
                    if (pReader != nullptr) {
                        vector_cat(packetList, pReader->GetStreamDataPackets());
                    }
                }
                //パケットを各Writerに分配する
                for (uint32_t i = 0; i < packetList.size(); i++) {
                    const int nTrackId = packetList[i].flags >> 16;
                    if (pWriterForAudioStreams.count(nTrackId)) {
                        auto pWriter = pWriterForAudioStreams[nTrackId];
                        if (pWriter == nullptr) {
                            PrintMes(RGY_LOG_ERROR, _T("Invalid writer found for track %d\n"), nTrackId);
                            return AMF_INVALID_POINTER;
                        }
                        auto ret = pWriter->WriteNextPacket(&packetList[i]);
                        if (ret != RGY_ERR_NONE) {
                            return AMF_FAIL;
                        }
                    } else {
                        PrintMes(RGY_LOG_ERROR, _T("Failed to find writer for track %d\n"), nTrackId);
                        return AMF_INVALID_POINTER;
                    }
                }
                amf_sleep(100);
            }
            if (sts != AMF_OK && sts != AMF_EOF) {

            }
            return GetState() == PipelineStateEof ? AMF_OK : AMF_FAIL;
        });
#endif //ENABLE_AVSW_READER
    }
    auto res = Pipeline::Start();
    if (res != AMF_OK) {
        PrintMes(RGY_LOG_ERROR, _T("failed to start pipeline: %s\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("started pipeline.\n"));

    return RGY_ERR_NONE;
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

    TCHAR cpu_info[256], gpu_info[256];
    getCPUInfo(cpu_info);
    getGPUInfo("Advanced Micro Device", gpu_info, _countof(gpu_info));

    uint32_t nMotionEst = 0x0;
    nMotionEst |= GetPropertyInt(AMF_PARAM_MOTION_HALF_PIXEL(m_encCodec)) ? VCE_MOTION_EST_HALF : 0;
    nMotionEst |= GetPropertyInt(AMF_PARAM_MOTION_QUARTERPIXEL(m_encCodec)) ? VCE_MOTION_EST_QUATER | VCE_MOTION_EST_HALF : 0;

    std::wstring deviceName = (m_deviceDX9.GetDevice() == nullptr) ? m_deviceDX11.GetDisplayDeviceName() : m_deviceDX9.GetDisplayDeviceName();
    deviceName = str_replace(deviceName, L"(TM)", L"");
    deviceName = str_replace(deviceName, L"(R)", L"");
    deviceName = str_replace(deviceName, L" Series", L"");
    deviceName = str_replace(deviceName, L" Graphics", L"");

    mes += strsprintf(_T("VCEEnc %s (%s) / %s (%s)\n"), VER_STR_FILEVERSION_TCHAR, BUILD_ARCH_STR, getOSVersion().c_str(), rgy_is_64bit_os() ? _T("x64") : _T("x86"));
    mes += strsprintf(_T("CPU:           %s\n"), cpu_info);
    mes += strsprintf(_T("GPU:           %s [%s, AMF %d.%d.%d]\n"), wstring_to_tstring(deviceName).c_str(), gpu_info,
        (int)AMF_GET_MAJOR_VERSION(g_AMFFactory.AMFQueryVersion()), (int)AMF_GET_MINOR_VERSION(g_AMFFactory.AMFQueryVersion()), (int)AMF_GET_SUBMINOR_VERSION(g_AMFFactory.AMFQueryVersion()));

    auto inputInfo = m_pFileReader->GetInputFrameInfo();
    mes += strsprintf(_T("Input Info:    %s\n"), m_pFileReader->GetInputMessage());
    if (cropEnabled(inputInfo.crop)) {
        mes += strsprintf(_T("Crop:          %d,%d,%d,%d\n"), inputInfo.crop.e.left, inputInfo.crop.e.up, inputInfo.crop.e.right, inputInfo.crop.e.bottom);
    }
    mes += strsprintf(_T("Output:        %s  %s @ Level %s%s\n"),
        CodecToStr(m_encCodec).c_str(),
        getPropertyDesc(AMF_PARAM_PROFILE(m_encCodec), get_profile_list(m_encCodec)).c_str(),
        getPropertyDesc(AMF_PARAM_PROFILE_LEVEL(m_encCodec), get_level_list(m_encCodec)).c_str(),
        (m_encCodec == RGY_CODEC_HEVC) ? (tstring(_T(" (")) + getPropertyDesc(AMF_VIDEO_ENCODER_HEVC_TIER, get_tier_list(m_encCodec)) + _T(" tier)")).c_str() : _T(""));
    const AMF_VIDEO_ENCODER_SCANTYPE_ENUM scan_type = (m_encCodec == RGY_CODEC_H264) ? (AMF_VIDEO_ENCODER_SCANTYPE_ENUM)GetPropertyInt(AMF_VIDEO_ENCODER_SCANTYPE) : AMF_VIDEO_ENCODER_SCANTYPE_PROGRESSIVE;
    AMFRatio aspectRatio;
    m_Params.GetParam(AMF_PARAM_ASPECT_RATIO(m_encCodec), aspectRatio);
    auto frameRate = GetPropertyRate(AMF_PARAM_FRAMERATE(m_encCodec));
    int64_t outWidth, outHeight;
    m_Params.GetParam(VCE_PARAM_KEY_OUTPUT_WIDTH, outWidth);
    m_Params.GetParam(VCE_PARAM_KEY_OUTPUT_HEIGHT, outHeight);
    mes += strsprintf(_T("               %dx%d%s %d:%d %0.3ffps (%d/%dfps)\n"),
        outWidth, outHeight,
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
    if (GetPropertyBool(AMF_PARAM_RATE_CONTROL_PREANALYSIS_ENABLE(m_encCodec))) {
        others += _T("pre-analysis ");
    }
    if (others.length() > 0) {
        mes += strsprintf(_T("Others:        %s\n"), others.c_str());
    }
    return mes;
}

void VCECore::PrintResult() {
    m_pStatus->WriteResults();
}

bool check_if_vce_available(RGY_CODEC codec, int nDeviceId) {
    bool ret = g_AMFFactory.Init() == AMF_OK;
    if (ret) {
        amf::AMFContextPtr p_context;
        ret = g_AMFFactory.GetFactory()->CreateContext(&p_context) == AMF_OK;
        if (ret) {
            DeviceDX9 device9;
            DeviceDX11 device11;
            ret = device9.Init(true, nDeviceId, false, 1280, 720) == AMF_OK;
            if (ret) {
                ret = p_context->InitDX9(device9.GetDevice()) == AMF_OK;
            }
            if (!ret) {
                ret = device11.Init(nDeviceId, false) == AMF_OK;
                if (ret) {
                    ret = p_context->InitDX11(device11.GetDevice()) == AMF_OK;
                }
            }
            amf::AMFComponentPtr p_encoder;
            ret = g_AMFFactory.GetFactory()->CreateComponent(p_context, codec_rgy_to_enc(codec), &p_encoder) == AMF_OK;
            if (p_encoder) {
                p_encoder->Terminate();
            }
            device9.Terminate();
            device11.Terminate();
            if (p_context) {
                p_context->Terminate();
            }
        }
    }
    g_AMFFactory.Terminate();
    return ret;
}

tstring check_vce_features(RGY_CODEC codec, int nDeviceId) {
    tstring str;
    bool ret = g_AMFFactory.Init() == AMF_OK;
    if (ret) {
        amf::AMFContextPtr p_context;
        ret = g_AMFFactory.GetFactory()->CreateContext(&p_context) == AMF_OK;
        if (ret) {
            DeviceDX9 device9;
            DeviceDX11 device11;
            ret = device9.Init(true, nDeviceId, false, 1280, 720) == AMF_OK;
            if (ret) {
                ret = p_context->InitDX9(device9.GetDevice()) == AMF_OK;
            }
            if (!ret) {
                ret = device11.Init(nDeviceId, false) == AMF_OK;
                if (ret) {
                    ret = p_context->InitDX11(device11.GetDevice()) == AMF_OK;
                }
            }
            amf::AMFComponentPtr p_encoder;
            ret = g_AMFFactory.GetFactory()->CreateComponent(p_context, codec_rgy_to_enc(codec), &p_encoder) == AMF_OK;
            if (ret) {
                amf::AMFCapsPtr encoderCaps;
                //HEVCでのAMFComponent::GetCaps()は、AMFComponent::Init()を呼んでおかないと成功しない
                p_encoder->Init(amf::AMF_SURFACE_NV12, 1280, 720);
                auto sts = p_encoder->GetCaps(&encoderCaps);
                if (sts == AMF_OK) {
                    str = VCECore::QueryIOCaps(codec, encoderCaps);
                } else {
                    str += _T("failed to get capability of ") + tstring(CodecToStr(codec)) + _T(" encoder");
                }
            } else {
                str += _T("failed to create ") + tstring(CodecToStr(codec)) + _T(" encoder");
            }
            if (p_encoder) {
                p_encoder->Terminate();
            }
            device9.Terminate();
            device11.Terminate();
            if (p_context) {
                p_context->Terminate();
            }
        } else {
            str += _T("failed to create context.\n");
        }
    } else {
        str += _T("failed to initialize AMF.\n");
    }
    g_AMFFactory.Terminate();
    return str;
}
