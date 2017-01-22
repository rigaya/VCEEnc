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

#include "VCECore.h"
#include "VCEParam.h"
#include "VCEVersion.h"
#include "VCEInput.h"
#include "VCEInputRaw.h"
#include "VCEInputAvs.h"
#include "VCEInputVpy.h"
#include "avcodec_reader.h"
#include "avcodec_writer.h"
#include "chapter_rw.h"

#include "VideoEncoderVCE.h"
#include "VideoEncoderHEVC.h"
#include "VideoConverter.h"
#include "EncoderParamsAVC.h"
#include "EncoderParamsHEVC.h"

const wchar_t* VCECore::PARAM_NAME_INPUT = L"INPUT";
const wchar_t* VCECore::PARAM_NAME_INPUT_WIDTH = L"WIDTH";
const wchar_t* VCECore::PARAM_NAME_INPUT_HEIGHT = L"HEIGHT";

const wchar_t* VCECore::PARAM_NAME_OUTPUT = L"OUTPUT";
const wchar_t* VCECore::PARAM_NAME_OUTPUT_WIDTH = L"OUTPUT_WIDTH";
const wchar_t* VCECore::PARAM_NAME_OUTPUT_HEIGHT = L"OUTPUT_HEIGHT";

const wchar_t* VCECore::PARAM_NAME_ENGINE = L"ENGINE";

const wchar_t* VCECore::PARAM_NAME_ADAPTERID = L"ADAPTERID";
const wchar_t* VCECore::PARAM_NAME_CAPABILITY = L"DISPLAYCAPABILITY";

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
    virtual amf_int32 GetInputSlotCount() override {
        return 1;
    }
    virtual amf_int32 GetOutputSlotCount() override {
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

std::wstring VCECore::AccelTypeToString(amf::AMF_ACCELERATION_TYPE accelType) {
    std::wstring strValue;
    switch (accelType) {
    case amf::AMF_ACCEL_NOT_SUPPORTED:
        strValue = L"Not supported";
        break;
    case amf::AMF_ACCEL_HARDWARE:
        strValue = L"Hardware-accelerated";
        break;
    case amf::AMF_ACCEL_GPU:
        strValue = L"GPU-accelerated";
        break;
    case amf::AMF_ACCEL_SOFTWARE:
        strValue = L"Not accelerated (software)";
        break;
    }
    return strValue;
}

void VCECore::PrintMes(int log_level, const TCHAR *format, ...) {
    if (m_pVCELog.get() == nullptr || log_level < m_pVCELog->getLogLevel()) {
        return;
    }

    va_list args;
    va_start(args, format);

    int len = _vsctprintf(format, args) + 1; // _vscprintf doesn't count terminating '\0'
    vector<TCHAR> buffer(len, 0);
    _vstprintf_s(buffer.data(), len, format, args);
    va_end(args);

    m_pVCELog->write(log_level, buffer.data());
}





VCECore::VCECore() :
    m_pVCELog(),
    m_bTimerPeriodTuning(true),
#if ENABLE_AVCODEC_VCE_READER
    m_AVChapterFromFile(),
#endif
    m_pFileReader(),
    m_AudioReaders(),
    m_pFileWriter(),
    m_pFileWriterListAudio(),
    m_pEncSatusInfo(),
    m_inputInfo(),
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
    m_VCECodecId(VCE_CODEC_NONE) {
}

VCECore::~VCECore() {
    Terminate();
}

void VCECore::Terminate() {
    if (m_bTimerPeriodTuning) {
        timeEndPeriod(1);
        PrintMes(VCE_LOG_DEBUG, _T("timeEndPeriod(1)\n"));
        m_bTimerPeriodTuning = false;
    }
    PrintMes(VCE_LOG_DEBUG, _T("Stopping pipeline...\n"));
    if (m_thStreamSender.joinable()) {
        m_thStreamSender.join();
    }
    Pipeline::Stop();
    PrintMes(VCE_LOG_DEBUG, _T("Pipeline Stopped.\n"));

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
    m_pFileReader.reset();
    m_AudioReaders.clear();
    m_pFileWriter.reset();
    m_pEncSatusInfo.reset();
    m_pVCELog.reset();
    m_VCECodecId = VCE_CODEC_NONE;
}

AMF_RESULT VCECore::readChapterFile(tstring chapfile) {
#if ENABLE_AVCODEC_VCE_READER
    ChapterRW chapter;
    auto err = chapter.read_file(chapfile.c_str(), CODE_PAGE_UNSET, 0.0);
    if (err != AUO_CHAP_ERR_NONE) {
        PrintMes(VCE_LOG_ERROR, _T("failed to %s chapter file: \"%s\".\n"), (err == AUO_CHAP_ERR_FILE_OPEN) ? _T("open") : _T("read"), chapfile.c_str());
        return AMF_FAIL;
    }
    if (chapter.chapterlist().size() == 0) {
        PrintMes(VCE_LOG_ERROR, _T("no chapter found from chapter file: \"%s\".\n"), chapfile.c_str());
        return AMF_FAIL;
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
    PrintMes(VCE_LOG_DEBUG, _T("%s"), chap_log.c_str());
    return AMF_OK;
#else
    PrintMes(VCE_LOG_ERROR, _T("chater reading unsupportted in this build"));
    return AMF_NOT_SUPPORTED;
#endif //#if ENABLE_AVCODEC_VCE_READER
}

AMF_RESULT VCECore::initInput(VCEParam *pParams, const VCEInputInfo *pInputInfo) {
#if !VCE_AUO
    m_pVCELog.reset(new VCELog(pParams->pStrLogFile, pParams->nLogLevel));
    if (pParams->pStrLogFile) {
        m_pVCELog->writeFileHeader(pParams->pOutputFile);
    }
    if (!m_pEncSatusInfo) {
        m_pEncSatusInfo = std::make_shared<VCEStatus>();
    }

    int sourceAudioTrackIdStart = 1;    //トラック番号は1スタート
    int sourceSubtitleTrackIdStart = 1; //トラック番号は1スタート
    if (pParams->nInputType == VCE_INPUT_NONE) {
        if (check_ext(pParams->pInputFile, { ".y4m" })) {
            pParams->nInputType = VCE_INPUT_Y4M;
#if ENABLE_AVISYNTH_READER
        } else if (check_ext(pParams->pInputFile, { ".avs" })) {
            pParams->nInputType = VCE_INPUT_AVS;
#endif
#if ENABLE_VAPOURSYNTH_READER
        } else if (check_ext(pParams->pInputFile, { ".vpy" })) {
            pParams->nInputType = VCE_INPUT_VPY;
#endif
#if ENABLE_AVCODEC_VCE_READER
        } else if (usingAVProtocols(tchar_to_string(pParams->pInputFile, CP_UTF8), 0)
            || check_ext(pParams->pInputFile, { ".mp4", ".m4v", ".mkv", ".mov",
                    ".mts", ".m2ts", ".ts", ".264", ".h264", ".x264", ".avc", ".avc1",
                    ".265", ".h265", ".hevc",
                    ".mpg", ".mpeg", "m2v", ".vob", ".vro", ".flv", ".ogm",
                    ".webm", ".vp8", ".vp9",
                    ".wmv" })) {
            pParams->nInputType = VCE_INPUT_AVCODEC_VCE;
#endif //ENABLE_AVCODEC_VCE_READER
        } else {
            pParams->nInputType = VCE_INPUT_RAW;
        }
    }

    VCEInputRawParam rawParam = { 0 };
#if ENABLE_AVISYNTH_READER
    VCEInputAvsParam avsParam = { 0 };
#endif
#if ENABLE_VAPOURSYNTH_READER
    VCEInputVpyParam vpyParam = { 0 };
#endif
#if ENABLE_AVCODEC_VCE_READER
    AvcodecReaderPrm avcodecReaderPrm = { 0 };
#endif
    if (pParams->nInputType == VCE_INPUT_Y4M || pParams->nInputType == VCE_INPUT_RAW) {
        rawParam.y4m = pParams->nInputType == VCE_INPUT_Y4M;
        rawParam.srcFile = pParams->pInputFile;
        m_inputInfo.pPrivateParam = &rawParam;
        m_pFileReader.reset(new VCEInputRaw());
#if ENABLE_AVISYNTH_READER
    } else if (pParams->nInputType == VCE_INPUT_AVS) {
        avsParam.srcFile = pParams->pInputFile;
        m_inputInfo.pPrivateParam = &avsParam;
        m_pFileReader.reset(new VCEInputAvs());
#endif
#if ENABLE_VAPOURSYNTH_READER
    } else if (pParams->nInputType == VCE_INPUT_VPY || pParams->nInputType == VCE_INPUT_VPY_MT) {
        vpyParam.srcFile = pParams->pInputFile;
        vpyParam.bVpyMt = pParams->nInputType == VCE_INPUT_VPY_MT;
        m_inputInfo.pPrivateParam = &vpyParam;
        m_pFileReader.reset(new VCEInputVpy());
#endif
#if ENABLE_AVCODEC_VCE_READER
    } else if (pParams->nInputType == VCE_INPUT_AVCODEC_VCE) {
        avcodecReaderPrm.srcFile = pParams->pInputFile;
        avcodecReaderPrm.bReadVideo = true;
        avcodecReaderPrm.nVideoTrack = (int8_t)pParams->nVideoTrack;
        avcodecReaderPrm.nVideoStreamId = pParams->nVideoStreamId;
        avcodecReaderPrm.bReadChapter = !!pParams->bCopyChapter;
        avcodecReaderPrm.bReadSubtitle = !!pParams->nSubtitleSelectCount;
        avcodecReaderPrm.pTrimList = pParams->pTrimList;
        avcodecReaderPrm.nTrimCount = (uint16_t)pParams->nTrimCount;
        avcodecReaderPrm.nReadAudio |= pParams->nAudioSelectCount > 0; 
        avcodecReaderPrm.nAnalyzeSec = (uint16_t)pParams->nAVDemuxAnalyzeSec;
        avcodecReaderPrm.nVideoAvgFramerate = std::make_pair(pInputInfo->fps.num, pInputInfo->fps.den);
        avcodecReaderPrm.nAudioTrackStart = (int)sourceAudioTrackIdStart;
        avcodecReaderPrm.ppAudioSelect = pParams->ppAudioSelectList;
        avcodecReaderPrm.nAudioSelectCount = pParams->nAudioSelectCount;
        //avcodecReaderPrm.bReadSubtitle = prm->nSubtitleSelectCount + prm->vpp.subburn.nTrack > 0;
        //avcodecReaderPrm.pSubtitleSelect = (prm->vpp.subburn.nTrack) ? &prm->vpp.subburn.nTrack : prm->pSubtitleSelect;
        //avcodecReaderPrm.nSubtitleSelectCount = (prm->vpp.subburn.nTrack) ? 1 : prm->nSubtitleSelectCount;
        avcodecReaderPrm.pSubtitleSelect = pParams->pSubtitleSelect;
        avcodecReaderPrm.nSubtitleSelectCount = pParams->nSubtitleSelectCount;
        avcodecReaderPrm.nProcSpeedLimit = pParams->nProcSpeedLimit;
        avcodecReaderPrm.fSeekSec = pParams->fSeekSec;
        avcodecReaderPrm.pFramePosListLog = pParams->pFramePosListLog;
        avcodecReaderPrm.nInputThread = (int8_t)pParams->nInputThread;
        avcodecReaderPrm.bAudioIgnoreNoTrackError = (int8_t)pParams->bAudioIgnoreNoTrackError;
        avcodecReaderPrm.pQueueInfo = nullptr;
        m_inputInfo.pPrivateParam = &avcodecReaderPrm;
        m_pFileReader.reset(new CAvcodecReader());
        PrintMes(VCE_LOG_DEBUG, _T("Input: avqsv reader selected.\n"));
#endif
    } else {
        PrintMes(VCE_LOG_ERROR, _T("Unknown reader selected\n"));
        return AMF_NOT_SUPPORTED;
    }
    auto ret = m_pFileReader->init(m_pVCELog, m_pEncSatusInfo, &m_inputInfo, m_pContext);
    if (ret != AMF_OK) {
        PrintMes(VCE_LOG_ERROR, _T("Error: %s\n"), m_pFileReader->getMessage().c_str());
        return ret;
    }
    PrintMes(VCE_LOG_DEBUG, _T("Input: reader initialization successful.\n"));
    sourceAudioTrackIdStart    += m_pFileReader->GetAudioTrackCount();
    sourceSubtitleTrackIdStart += m_pFileReader->GetSubtitleTrackCount();

    if (pParams->nAudioSourceCount && pParams->ppAudioSourceList) {
        auto videoInfo = m_pFileReader->GetInputFrameInfo();

        for (int i = 0; i < pParams->nAudioSourceCount; i++) {
            AvcodecReaderPrm audioReaderPrm = { 0 };
            audioReaderPrm.srcFile = pParams->ppAudioSourceList[i];
            audioReaderPrm.bReadVideo = false;
            audioReaderPrm.nReadAudio |= pParams->nAudioSelectCount > 0;
            audioReaderPrm.nAnalyzeSec = (uint16_t)pParams->nAVDemuxAnalyzeSec;
            audioReaderPrm.pTrimList = pParams->pTrimList;
            audioReaderPrm.nTrimCount = (uint16_t)pParams->nTrimCount;
            audioReaderPrm.nVideoAvgFramerate = std::make_pair(videoInfo.fps.num, videoInfo.fps.den);
            audioReaderPrm.nAudioTrackStart = sourceAudioTrackIdStart;
            audioReaderPrm.nSubtitleTrackStart = sourceSubtitleTrackIdStart;
            audioReaderPrm.ppAudioSelect = pParams->ppAudioSelectList;
            audioReaderPrm.nAudioSelectCount = pParams->nAudioSelectCount;
            audioReaderPrm.nProcSpeedLimit = pParams->nProcSpeedLimit;
            audioReaderPrm.fSeekSec = pParams->fSeekSec;
            audioReaderPrm.bAudioIgnoreNoTrackError = (int8_t)pParams->bAudioIgnoreNoTrackError;
            audioReaderPrm.nInputThread = 0;
            audioReaderPrm.pQueueInfo = nullptr;
            auto inputInfoCopy = m_inputInfo;
            inputInfoCopy.pPrivateParam = &audioReaderPrm;

            unique_ptr<VCEInput> audioReader(new CAvcodecReader());
            ret = audioReader->init(m_pVCELog, nullptr, &inputInfoCopy, m_pContext);
            if (ret != AMF_OK) {
                PrintMes(VCE_LOG_ERROR, audioReader->getMessage().c_str());
                return ret;
            }
            sourceAudioTrackIdStart += audioReader->GetAudioTrackCount();
            sourceSubtitleTrackIdStart += audioReader->GetSubtitleTrackCount();
            m_AudioReaders.push_back(std::move(audioReader));
        }
    }

    if (!m_pFileReader->getInputCodec()
        && pParams->pTrimList && pParams->nTrimCount > 0) {
        //avqsvリーダー以外は、trimは自分ではセットされないので、ここでセットする
        sTrimParam trimParam;
        trimParam.list = make_vector(pParams->pTrimList, pParams->nTrimCount);
        trimParam.offset = 0;
        m_pFileReader->SetTrimParam(trimParam);
    }
    //trim情報をリーダーから取得する
    auto trimParam = m_pFileReader->GetTrimParam();
    m_pTrimParam = (trimParam->list.size()) ? trimParam : nullptr;
    if (m_pTrimParam) {
        PrintMes(VCE_LOG_DEBUG, _T("Input: trim options\n"));
        for (int i = 0; i < (int)m_pTrimParam->list.size(); i++) {
            PrintMes(VCE_LOG_DEBUG, _T("%d-%d "), m_pTrimParam->list[i].start, m_pTrimParam->list[i].fin);
        }
        PrintMes(VCE_LOG_DEBUG, _T(" (offset: %d)\n"), m_pTrimParam->offset);
    }
#endif //#if !VCE_AUO
    return AMF_OK;
}

AMF_RESULT VCECore::checkParam(VCEParam *prm) {
    auto srcInfo = m_pFileReader->GetInputFrameInfo();
    if (m_inputInfo.fps.num <= 0 || m_inputInfo.fps.den <= 0) {
        m_inputInfo.fps = srcInfo.fps;
    }
    if (srcInfo.srcWidth) {
        m_inputInfo.srcWidth = srcInfo.srcWidth;
    }
    if (srcInfo.srcHeight) {
        m_inputInfo.srcHeight = srcInfo.srcHeight;
    }
    if (srcInfo.frames) {
        m_inputInfo.frames = srcInfo.frames;
    }
    if (srcInfo.format) {
        m_inputInfo.format = srcInfo.format;
    }

    if (m_inputInfo.fps.num <= 0 || m_inputInfo.fps.den <= 0) {
        PrintMes(VCE_LOG_ERROR, _T("Invalid fps - zero or negative (%d/%d).\n"), m_inputInfo.fps.num, m_inputInfo.fps.den);
        return AMF_FAIL;
    }
    {
        int fps_gcd = vce_gcd(m_inputInfo.fps.num, m_inputInfo.fps.den);
        m_inputInfo.fps.num /= fps_gcd;
        m_inputInfo.fps.den /= fps_gcd;
    }
    if (m_inputInfo.srcWidth <= 0 || m_inputInfo.srcHeight <= 0) {
        PrintMes(VCE_LOG_ERROR, _T("Invalid frame size - zero or negative (%dx%d).\n"), m_inputInfo.srcWidth, m_inputInfo.srcHeight);
        return AMF_FAIL;
    }
    int h_mul = is_interlaced(prm) ? 4 : 2;
    if (m_inputInfo.srcWidth % 2 != 0) {
        PrintMes(VCE_LOG_ERROR, _T("Invalid input frame size - non mod2 (width: %d).\n"), m_inputInfo.srcWidth);
        return AMF_FAIL;
    }
    if (m_inputInfo.srcHeight % h_mul != 0) {
        PrintMes(VCE_LOG_ERROR, _T("Invalid input frame size - non mod%d (height: %d).\n"), h_mul, m_inputInfo.srcHeight);
        return AMF_FAIL;
    }
    if (m_inputInfo.srcWidth < (m_inputInfo.crop.left + m_inputInfo.crop.right)
        || m_inputInfo.srcHeight < (m_inputInfo.crop.bottom + m_inputInfo.crop.up)) {
        PrintMes(VCE_LOG_ERROR, _T("crop size is too big.\n"));
        return AMF_FAIL;
    }
    if (prm->nInputType == VCE_INPUT_AVCODEC_VCE && (m_inputInfo.crop.left | m_inputInfo.crop.right | m_inputInfo.crop.bottom | m_inputInfo.crop.up)) {
        PrintMes(VCE_LOG_ERROR, _T("crop not available with avvce readder.\n"));
        return AMF_NOT_SUPPORTED;
    }
    m_inputInfo.srcWidth -= (m_inputInfo.crop.left + m_inputInfo.crop.right);
    m_inputInfo.srcHeight -= (m_inputInfo.crop.bottom + m_inputInfo.crop.up);
    if (m_inputInfo.srcWidth % 2 != 0) {
        PrintMes(VCE_LOG_ERROR, _T("Invalid input frame size (after crop) - non mod2 (width: %d).\n"), m_inputInfo.srcWidth);
        return AMF_FAIL;
    }
    if (m_inputInfo.srcHeight % h_mul != 0) {
        PrintMes(VCE_LOG_ERROR, _T("Invalid input frame size (after crop) - non mod%d (height: %d).\n"), h_mul, m_inputInfo.srcHeight);
        return AMF_FAIL;
    }
    if (m_inputInfo.dstWidth <= 0) {
        m_inputInfo.dstWidth = m_inputInfo.srcWidth;
    }
    if (m_inputInfo.dstHeight <= 0) {
        m_inputInfo.dstHeight = m_inputInfo.srcHeight;
    }
    if (m_inputInfo.dstWidth % 2 != 0) {
        PrintMes(VCE_LOG_ERROR, _T("Invalid output frame size - non mod2 (width: %d).\n"), m_inputInfo.dstWidth);
        return AMF_FAIL;
    }
    if (m_inputInfo.dstHeight % h_mul != 0) {
        PrintMes(VCE_LOG_ERROR, _T("Invalid output frame size - non mod%d (height: %d).\n"), h_mul, m_inputInfo.dstHeight);
        return AMF_FAIL;
    }
    if (prm->nCodecId == VCE_CODEC_NONE) {
        prm->nCodecId = VCE_CODEC_H264;
    }
    if (prm->nBframes > VCE_MAX_BFRAMES) {
        PrintMes(VCE_LOG_WARN, _T("Maximum consecutive B frames is %d.\n"), VCE_MAX_BFRAMES);
        prm->nBframes = VCE_MAX_BFRAMES;
    }
    if (prm->nBitrate > VCE_MAX_BITRATE) {
        PrintMes(VCE_LOG_WARN, _T("Maximum bitrate is %d.\n"), VCE_MAX_BITRATE);
        prm->nBitrate = VCE_MAX_BITRATE;
    }
    if (prm->nMaxBitrate > VCE_MAX_BITRATE) {
        PrintMes(VCE_LOG_WARN, _T("Maximum max bitrate is %d.\n"), VCE_MAX_BITRATE);
        prm->nMaxBitrate = VCE_MAX_BITRATE;
    }
    if (prm->nVBVBufferSize > VCE_MAX_BITRATE) {
        PrintMes(VCE_LOG_WARN, _T("Maximum vbv buffer size is %d.\n"), VCE_MAX_BITRATE);
        prm->nVBVBufferSize = VCE_MAX_BITRATE;
    }
    if (prm->nGOPLen > VCE_MAX_GOP_LEN) {
        PrintMes(VCE_LOG_WARN, _T("Maximum GOP len is %d.\n"), VCE_MAX_GOP_LEN);
        prm->nGOPLen = VCE_MAX_GOP_LEN;
    }
    if (abs(prm->nDeltaQPBFrame) > VCE_MAX_B_DELTA_QP) {
        PrintMes(VCE_LOG_WARN, _T("Maximum Delta QP for Bframes is %d.\n"), VCE_MAX_B_DELTA_QP);
        prm->nDeltaQPBFrame = clamp(prm->nDeltaQPBFrame, -1 * VCE_MAX_B_DELTA_QP, VCE_MAX_B_DELTA_QP);
    }
    if (abs(prm->nDeltaQPBFrameRef) > VCE_MAX_B_DELTA_QP) {
        PrintMes(VCE_LOG_WARN, _T("Maximum Delta QP for Bframes is %d.\n"), VCE_MAX_B_DELTA_QP);
        prm->nDeltaQPBFrameRef = clamp(prm->nDeltaQPBFrameRef, -1 * VCE_MAX_B_DELTA_QP, VCE_MAX_B_DELTA_QP);
    }
    if (prm->nBframes > 0 && prm->nCodecId == VCE_CODEC_HEVC) {
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

    return AMF_OK;
}

AMF_RESULT VCECore::initOutput(VCEParam *pParams) {
    m_pEncSatusInfo->init(m_pVCELog, m_inputInfo.fps, m_inputInfo.frames);
    AMF_RESULT sts = AMF_OK;
    bool stdoutUsed = false;
#if ENABLE_AVCODEC_VCE_READER
    vector<int> streamTrackUsed; //使用した音声/字幕のトラックIDを保存する
    bool useH264ESOutput =
        ((pParams->pAVMuxOutputFormat && 0 == _tcscmp(pParams->pAVMuxOutputFormat, _T("raw")))) //--formatにrawが指定されている
        || (PathFindExtension(pParams->pOutputFile) == nullptr || PathFindExtension(pParams->pOutputFile)[0] != '.') //拡張子がしない
        || check_ext(pParams->pOutputFile, { ".m2v", ".264", ".h264", ".avc", ".avc1", ".x264", ".265", ".h265", ".hevc" }); //特定の拡張子
    if (!useH264ESOutput) {
        pParams->nAVMux |= VCEENC_MUX_VIDEO;
    }
    //if (pParams->nCodecId == VCE_CODEC_RAW) {
    //    pParams->nAVMux &= ~VCEENC_MUX_VIDEO;
    //}
    if (pParams->nAVMux & VCEENC_MUX_VIDEO) {
        //if (pParams->nCodecId == VCE_CODEC_VP8 || pParams->nCodecId == VCE_CODEC_VP9) {
        //    PrintMes(VCE_LOG_ERROR, _T("Output: muxing not supported with %s.\n"), CodecIdToStr(pParams->nCodecId));
        //    return AMF_NOT_SUPPORTED;
        //}
        PrintMes(VCE_LOG_DEBUG, _T("Output: Using avformat writer.\n"));
        m_pFileWriter = std::make_shared<CAvcodecWriter>();
        AvcodecWriterPrm writerPrm;
        writerPrm.vidPrm.nCodecId = pParams->nCodecId;
        writerPrm.vidPrm.nCodecLevel = pParams->codecParam[pParams->nCodecId].nLevel;
        writerPrm.vidPrm.nCodecProfile = pParams->codecParam[pParams->nCodecId].nProfile;
        writerPrm.vidPrm.nEncWidth = m_inputInfo.dstWidth;
        writerPrm.vidPrm.nEncHeight = m_inputInfo.dstHeight;
        writerPrm.vidPrm.nPicStruct = m_inputInfo.nPicStruct;
        writerPrm.vidPrm.sar = std::make_pair(m_inputInfo.AspectRatioW, m_inputInfo.AspectRatioH);
        writerPrm.vidPrm.outFps = av_make_q(m_inputInfo.fps.num, m_inputInfo.fps.den);
        writerPrm.vidPrm.nBframes = pParams->nBframes;
        writerPrm.vidPrm.nBPyramid = pParams->bBPyramid;
        writerPrm.vidPrm.nGopLength = pParams->nGOPLen;
        writerPrm.vidPrm.nRef = 1;
        writerPrm.vidPrm.vui.colormatrix = pParams->ColorMatrix;
        writerPrm.vidPrm.vui.colorprim = pParams->ColorPrim;
        writerPrm.vidPrm.vui.transfer = pParams->Transfer;
        writerPrm.vidPrm.vui.fullrange = pParams->bFullrange;
        writerPrm.vidPrm.vui.infoPresent = pParams->VuiEnable;
        writerPrm.vidPrm.bDtsUnavailable = true;
        writerPrm.vidPrm.bCFR = true;
        writerPrm.pOutputFormat = pParams->pAVMuxOutputFormat;
        if (m_pTrimParam) {
            writerPrm.trimList = m_pTrimParam->list;
        }
        writerPrm.nOutputThread = pParams->nOutputThread;
        writerPrm.nAudioThread  = pParams->nAudioThread;
        writerPrm.nBufSizeMB = pParams->nOutputBufSizeMB;
        writerPrm.nAudioResampler = pParams->nAudioResampler;
        writerPrm.nAudioIgnoreDecodeError = pParams->nAudioIgnoreDecodeError;
        writerPrm.bVideoDtsUnavailable = false;
        writerPrm.pQueueInfo = nullptr;
        writerPrm.nVideoInputFirstKeyPts = 0;
        writerPrm.pVideoInputCodecCtx = nullptr;
        //writerPrm.pQueueInfo = (m_pPerfMonitor) ? m_pPerfMonitor->GetQueueInfoPtr() : nullptr;
        if (pParams->pMuxOpt) {
            writerPrm.vMuxOpt = *pParams->pMuxOpt;
        }
        auto pAVCodecReader = std::dynamic_pointer_cast<CAvcodecReader>(m_pFileReader);
        if (pAVCodecReader != nullptr) {
            writerPrm.pInputFormatMetadata = pAVCodecReader->GetInputFormatMetadata();
            if (pParams->pChapterFile) {
                //チャプターファイルを読み込む
                if (AMF_OK != readChapterFile(pParams->pChapterFile)) {
                    return AMF_INVALID_FORMAT;
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
            writerPrm.pVideoInputCodecCtx = pAVCodecReader->GetInputVideoCodecCtx();
        }
        if (pParams->nAVMux & (VCEENC_MUX_AUDIO | VCEENC_MUX_SUBTITLE)) {
            PrintMes(VCE_LOG_DEBUG, _T("Output: Audio/Subtitle muxing enabled.\n"));
            pAVCodecReader = std::dynamic_pointer_cast<CAvcodecReader>(m_pFileReader);
            bool copyAll = false;
            for (int i = 0; !copyAll && i < pParams->nAudioSelectCount; i++) {
                //トラック"0"が指定されていれば、すべてのトラックをコピーするということ
                copyAll = (pParams->ppAudioSelectList[i]->nAudioSelect == 0);
            }
            PrintMes(VCE_LOG_DEBUG, _T("Output: CopyAll=%s\n"), (copyAll) ? _T("true") : _T("false"));
            vector<AVDemuxStream> streamList;
            if (pAVCodecReader) {
                streamList = pAVCodecReader->GetInputStreamInfo();
            }
            for (const auto& audioReader : m_AudioReaders) {
                if (audioReader->GetAudioTrackCount()) {
                    auto pAVCodecAudioReader = std::dynamic_pointer_cast<CAvcodecReader>(audioReader);
                    if (pAVCodecAudioReader) {
                        vector_cat(streamList, pAVCodecAudioReader->GetInputStreamInfo());
                    }
                    //もしavqsvリーダーでないなら、音声リーダーから情報を取得する必要がある
                    if (pAVCodecReader == nullptr) {
                        writerPrm.nVideoInputFirstKeyPts = pAVCodecAudioReader->GetVideoFirstKeyPts();
                        writerPrm.pVideoInputCodecCtx = pAVCodecAudioReader->GetInputVideoCodecCtx();
                    }
                }
            }

            for (auto& stream : streamList) {
                bool bStreamIsSubtitle = stream.nTrackId < 0;
                const sAudioSelect *pAudioSelect = nullptr;
                for (int i = 0; i < pParams->nAudioSelectCount; i++) {
                    if (stream.nTrackId == pParams->ppAudioSelectList[i]->nAudioSelect
                        && pParams->ppAudioSelectList[i]->pAudioExtractFilename == nullptr) {
                        pAudioSelect = pParams->ppAudioSelectList[i];
                    }
                }
                if (pAudioSelect != nullptr || copyAll || bStreamIsSubtitle) {
                    streamTrackUsed.push_back(stream.nTrackId);
#if ENABLE_LIBASS_SUBBURN
                    if (bStreamIsSubtitle && pParams->vpp.subburn.nTrack != 0) {
                        continue;
                    }
#endif //#if ENABLE_LIBASS_SUBBURN
                    AVOutputStreamPrm prm;
                    prm.src = stream;
                    //pAudioSelect == nullptrは "copyAll" か 字幕ストリーム によるもの
                    prm.nBitrate = (pAudioSelect == nullptr) ? 0 : pAudioSelect->nAVAudioEncodeBitrate;
                    prm.nSamplingRate = (pAudioSelect == nullptr) ? 0 : pAudioSelect->nAudioSamplingRate;
                    prm.pEncodeCodec = (pAudioSelect == nullptr) ? AVVCE_CODEC_COPY : pAudioSelect->pAVAudioEncodeCodec;
                    prm.pFilter = (pAudioSelect == nullptr) ? nullptr : pAudioSelect->pAudioFilter;
                    PrintMes(VCE_LOG_DEBUG, _T("Output: Added %s track#%d (stream idx %d) for mux, bitrate %d, codec: %s\n"),
                        (bStreamIsSubtitle) ? _T("sub") : _T("audio"),
                        stream.nTrackId, stream.nIndex, prm.nBitrate, prm.pEncodeCodec);
                    writerPrm.inputStreamList.push_back(std::move(prm));
                }
            }
        }
        sts = m_pFileWriter->Init(pParams->pOutputFile, &writerPrm, m_pVCELog, m_pEncSatusInfo);
        if (sts < AMF_OK) {
            PrintMes(VCE_LOG_ERROR, m_pFileWriter->GetOutputMessage().c_str());
            return sts;
        } else if (pParams->nAVMux & (VCEENC_MUX_AUDIO | VCEENC_MUX_SUBTITLE)) {
            m_pFileWriterListAudio.push_back(m_pFileWriter);
        }
        stdoutUsed = m_pFileWriter->outputStdout();
        PrintMes(VCE_LOG_DEBUG, _T("Output: Initialized avformat writer%s.\n"), (stdoutUsed) ? _T("using stdout") : _T(""));
    } else if (pParams->nAVMux & (VCEENC_MUX_AUDIO | VCEENC_MUX_SUBTITLE)) {
        PrintMes(VCE_LOG_ERROR, _T("Audio mux cannot be used alone, should be use with video mux.\n"));
        return AMF_NOT_SUPPORTED;
    } else {
#endif
        //if (pParams->nCodecId == VCE_CODEC_RAW) {
        //    m_pFileWriter.reset(new CVCEOutFrame());
        //    m_pFileWriter->SetVCELogPtr(m_pVCELog);
        //    YUVWriterParam param;
        //    param.bY4m = true;
        //    param.memType = m_memType;
        //    sts = m_pFileWriter->Init(pParams->strDstFile, &param, m_pEncSatusInfo);
        //    if (sts < AMF_OK) {
        //        PrintMes(VCE_LOG_ERROR, m_pFileWriter->GetOutputMessage());
        //        return sts;
        //    }
        //    stdoutUsed = m_pFileWriter->outputStdout();
        //    PrintMes(VCE_LOG_DEBUG, _T("Output: Initialized yuv frame writer%s.\n"), (stdoutUsed) ? _T("using stdout") : _T(""));
        //} else {
        m_pFileWriter = std::make_shared<VCEOutput>();
        VCEOutRawParam rawPrm = { 0 };
        //rawPrm.bBenchmark = pParams->bBenchmark != 0;
        rawPrm.nBufSizeMB = pParams->nOutputBufSizeMB;
        sts = m_pFileWriter->Init(pParams->pOutputFile, &rawPrm, m_pVCELog, m_pEncSatusInfo);
        if (sts < AMF_OK) {
            PrintMes(VCE_LOG_ERROR, m_pFileWriter->GetOutputMessage().c_str());
            return sts;
        }
        stdoutUsed = m_pFileWriter->outputStdout();
        PrintMes(VCE_LOG_DEBUG, _T("Output: Initialized bitstream writer%s.\n"), (stdoutUsed) ? _T("using stdout") : _T(""));
        //}
#if ENABLE_AVCODEC_VCE_READER
    } //ENABLE_AVCODEC_VCE_READER

      //音声の抽出
    if (pParams->nAudioSelectCount + pParams->nSubtitleSelectCount > (int)streamTrackUsed.size()) {
        PrintMes(VCE_LOG_DEBUG, _T("Output: Audio file output enabled.\n"));
        auto pAVCodecReader = std::dynamic_pointer_cast<CAvcodecReader>(m_pFileReader);
        if (pParams->nInputType != VCE_INPUT_AVCODEC_VCE || pAVCodecReader == nullptr) {
            PrintMes(VCE_LOG_ERROR, _T("Audio output is only supported with transcoding (avqsv reader).\n"));
            return AMF_NOT_SUPPORTED;
        } else {
            auto inutAudioInfoList = pAVCodecReader->GetInputStreamInfo();
            for (auto& audioTrack : inutAudioInfoList) {
                bool bTrackAlreadyUsed = false;
                for (auto usedTrack : streamTrackUsed) {
                    if (usedTrack == audioTrack.nTrackId) {
                        bTrackAlreadyUsed = true;
                        PrintMes(VCE_LOG_DEBUG, _T("Audio track #%d is already set to be muxed, so cannot be extracted to file.\n"), audioTrack.nTrackId);
                        break;
                    }
                }
                if (bTrackAlreadyUsed) {
                    continue;
                }
                const sAudioSelect *pAudioSelect = nullptr;
                for (int i = 0; i < pParams->nAudioSelectCount; i++) {
                    if (audioTrack.nTrackId == pParams->ppAudioSelectList[i]->nAudioSelect
                        && pParams->ppAudioSelectList[i]->pAudioExtractFilename != nullptr) {
                        pAudioSelect = pParams->ppAudioSelectList[i];
                    }
                }
                if (pAudioSelect == nullptr) {
                    PrintMes(VCE_LOG_ERROR, _T("Audio track #%d is not used anyware, this should not happen.\n"), audioTrack.nTrackId);
                    return AMF_UNEXPECTED;
                }
                PrintMes(VCE_LOG_DEBUG, _T("Output: Output audio track #%d (stream index %d) to \"%s\", format: %s, codec %s, bitrate %d\n"),
                    audioTrack.nTrackId, audioTrack.nIndex, pAudioSelect->pAudioExtractFilename, pAudioSelect->pAudioExtractFormat, pAudioSelect->pAVAudioEncodeCodec, pAudioSelect->nAVAudioEncodeBitrate);

                AVOutputStreamPrm prm;
                prm.src = audioTrack;
                //pAudioSelect == nullptrは "copyAll" によるもの
                prm.nBitrate = pAudioSelect->nAVAudioEncodeBitrate;
                prm.pFilter = pAudioSelect->pAudioFilter;
                prm.pEncodeCodec = pAudioSelect->pAVAudioEncodeCodec;
                prm.nSamplingRate = pAudioSelect->nAudioSamplingRate;

                AvcodecWriterPrm writerAudioPrm;
                writerAudioPrm.nOutputThread   = pParams->nOutputThread;
                writerAudioPrm.nAudioThread    = pParams->nAudioThread;
                writerAudioPrm.nBufSizeMB      = pParams->nOutputBufSizeMB;
                writerAudioPrm.pOutputFormat   = pAudioSelect->pAudioExtractFormat;
                writerAudioPrm.nAudioIgnoreDecodeError = pParams->nAudioIgnoreDecodeError;
                writerAudioPrm.nAudioResampler = pParams->nAudioResampler;
                writerAudioPrm.inputStreamList.push_back(prm);
                writerAudioPrm.pQueueInfo = nullptr;
                if (m_pTrimParam) {
                    writerAudioPrm.trimList = m_pTrimParam->list;
                }
                writerAudioPrm.nVideoInputFirstKeyPts = pAVCodecReader->GetVideoFirstKeyPts();
                writerAudioPrm.pVideoInputCodecCtx = pAVCodecReader->GetInputVideoCodecCtx();

                auto pWriter = std::make_shared<CAvcodecWriter>();
                sts = pWriter->Init(pAudioSelect->pAudioExtractFilename, &writerAudioPrm, m_pVCELog, m_pEncSatusInfo);
                if (sts < AMF_OK) {
                    PrintMes(VCE_LOG_ERROR, pWriter->GetOutputMessage().c_str());
                    return sts;
                }
                PrintMes(VCE_LOG_DEBUG, _T("Output: Intialized audio output for track #%d.\n"), audioTrack.nTrackId);
                bool audioStdout = pWriter->outputStdout();
                if (stdoutUsed && audioStdout) {
                    PrintMes(VCE_LOG_ERROR, _T("Multiple stream outputs are set to stdout, please remove conflict.\n"));
                    return AMF_INVALID_ARG;
                }
                stdoutUsed |= audioStdout;
                m_pFileWriterListAudio.push_back(std::move(pWriter));
            }
        }
    }
#endif //ENABLE_AVCODEC_VCE_READER
    return sts;
}

AMF_RESULT VCECore::initDevice(VCEParam *prm) {
    AMF_RESULT res = AMF_OK;

    if (prm->memoryTypeIn == amf::AMF_MEMORY_DX9) {
        if (AMF_OK != (res = m_deviceDX9.Init(true, prm->nAdapterId, false, m_inputInfo.srcWidth, m_inputInfo.srcHeight))) {
            PrintMes(VCE_LOG_ERROR, _T("Failed to initialize DX9 device.\n"));
            return AMF_FAIL;
        }
        PrintMes(VCE_LOG_DEBUG, _T("initialized DX9 device.\n"));
        if (AMF_OK != (res = m_pContext->InitDX9(m_deviceDX9.GetDevice()))) {
            PrintMes(VCE_LOG_ERROR, _T("Failed to InitDX9.\n"));
            return AMF_FAIL;
        }
        PrintMes(VCE_LOG_DEBUG, _T("initialized context for DX9.\n"));
    } else if (prm->memoryTypeIn == amf::AMF_MEMORY_DX11) {
        if (AMF_OK != (res = m_deviceDX11.Init(prm->nAdapterId, false))) {
            PrintMes(VCE_LOG_ERROR, _T("Failed to initialize DX11 device.\n"));
            return AMF_FAIL;
        }
        PrintMes(VCE_LOG_DEBUG, _T("initialized DX11 device.\n"));
        if (AMF_OK != (res = m_pContext->InitDX11(m_deviceDX11.GetDevice()))) {
            PrintMes(VCE_LOG_ERROR, _T("Failed to InitDX11.\n"));
            return AMF_FAIL;
        }
        PrintMes(VCE_LOG_DEBUG, _T("initialized context for DX11.\n"));
    } else {
        PrintMes(VCE_LOG_ERROR, _T("Invalid memory type.\n"));
        return AMF_FAIL;
    }
    return res;
}

#pragma warning(push)
#pragma warning(disable: 4100)
AMF_RESULT VCECore::initDecoder(VCEParam *prm) {
#if ENABLE_AVCODEC_VCE_READER
    auto inputCodec = m_pFileReader->getInputCodec();
    if (inputCodec == VCE_CODEC_NONE) {
        return AMF_OK;
    }
    if (VCE_CODEC_UVD_NAME.find(inputCodec) == VCE_CODEC_UVD_NAME.end()) {
        PrintMes(VCE_LOG_ERROR, _T("Input codec \"%s\" not supported.\n"), CodecIdToStr(inputCodec));
        return AMF_NOT_SUPPORTED;
    }
    const auto codec_uvd_name = VCE_CODEC_UVD_NAME.at(inputCodec);
    auto res = g_AMFFactory.GetFactory()->CreateComponent(m_pContext, codec_uvd_name, &m_pDecoder);
    if (res != AMF_OK) {
        PrintMes(VCE_LOG_ERROR, _T("Failed to create decoder context: %d\n"), res);
        return AMF_FAIL;
    }

    if (AMF_OK != (res = m_pDecoder->SetProperty(AMF_TIMESTAMP_MODE, amf_int64(AMF_TS_PRESENTATION)))) {
        PrintMes(VCE_LOG_ERROR, _T("Failed to set deocder: %d\n"), res);
        return AMF_FAIL;
    }
    sBitstream header = { 0 };
    if (AMF_OK != (res = m_pFileReader->GetHeader(&header))) {
        PrintMes(VCE_LOG_ERROR, _T("Failed to get video header: %d\n"), res);
        return AMF_FAIL;
    }
    amf::AMFBufferPtr buffer;
    m_pContext->AllocBuffer(amf::AMF_MEMORY_HOST, header.DataLength, &buffer);

    memcpy(buffer->GetNative(), header.Data, header.DataLength);
    m_pDecoder->SetProperty(AMF_VIDEO_DECODER_EXTRADATA, amf::AMFVariant(buffer));

    if (AMF_OK != (res = m_pDecoder->Init(formatOut, m_inputInfo.srcWidth, m_inputInfo.srcHeight))) {
        PrintMes(VCE_LOG_ERROR, _T("Failed to init decoder: %d\n"), res);
        return res;
    }
    PrintMes(VCE_LOG_DEBUG, _T("Initialized decoder\n"), res);
    return res;
#else
    return AMF_OK;
#endif
}
#pragma warning(pop)

AMF_RESULT VCECore::initConverter(VCEParam *prm) {
#if ENABLE_AVCODEC_VCE_READER
    if (m_inputInfo.dstWidth == m_inputInfo.srcWidth && m_inputInfo.dstHeight == m_inputInfo.srcHeight) {
        return AMF_OK;
    }
    auto res = g_AMFFactory.GetFactory()->CreateComponent(m_pContext, AMFVideoConverter, &m_pConverter);
    if (res != AMF_OK) {
        PrintMes(VCE_LOG_ERROR, _T("Failed to create converter context: %d\n"), res);
        return AMF_FAIL;
    }

    res = m_pConverter->SetProperty(AMF_VIDEO_CONVERTER_MEMORY_TYPE, prm->memoryTypeIn);
    res = m_pConverter->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_FORMAT, formatOut);
    res = m_pConverter->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_SIZE,   AMFConstructSize(m_inputInfo.dstWidth, m_inputInfo.dstHeight));
    res = m_pConverter->SetProperty(AMF_VIDEO_CONVERTER_SCALE, AMF_VIDEO_CONVERTER_SCALE_BICUBIC);
    if (AMF_OK != (res = m_pConverter->Init(formatOut, m_inputInfo.srcWidth, m_inputInfo.srcHeight))) {
        PrintMes(VCE_LOG_ERROR, _T("Failed to init converter: %d\n"), res);
        return res;
    }
    return res;
#else
    return AMF_OK;
#endif
}

bool VCECore::QueryIOCaps(amf::AMFIOCapsPtr& ioCaps) {
    bool result = true;
    if (ioCaps != NULL) {
        amf_int32 minWidth, maxWidth;
        ioCaps->GetWidthRange(&minWidth, &maxWidth);
        std::wcout << L"\t\t\tWidth: [" << minWidth << L"-" << maxWidth << L"]\n";

        amf_int32 minHeight, maxHeight;
        ioCaps->GetHeightRange(&minHeight, &maxHeight);
        std::wcout << L"\t\t\tHeight: [" << minHeight << L"-" << maxHeight << L"]\n";

        amf_int32 vertAlign = ioCaps->GetVertAlign();
        std::wcout << L"\t\t\tVertical alignment: " << vertAlign << L" lines.\n";

        amf_bool interlacedSupport = ioCaps->IsInterlacedSupported();
        std::wcout << L"\t\t\tInterlaced support: " << (interlacedSupport ? L"YES" : L"NO") << L"\n";

        amf_int32 numOfFormats = ioCaps->GetNumOfFormats();
        std::wcout << L"\t\t\tTotal of " << numOfFormats << L" pixel format(s) supported:\n";
        for (amf_int32 i = 0; i < numOfFormats; i++) {
            amf::AMF_SURFACE_FORMAT format;
            amf_bool native = false;
            if (ioCaps->GetFormatAt(i, &format, &native) == AMF_OK) {
                std::wcout << L"\t\t\t\t" << i << L": " << g_AMFFactory.GetTrace()->SurfaceGetFormatName(format) << L" " << (native ? L"(native)" : L"") << L"\n";
            } else {
                result = false;
                break;
            }
        }

        if (result == true) {
            amf_int32 numOfMemTypes = ioCaps->GetNumOfMemoryTypes();
            std::wcout << L"\t\t\tTotal of " << numOfMemTypes << L" memory type(s) supported:\n";
            for (amf_int32 i = 0; i < numOfMemTypes; i++) {
                amf::AMF_MEMORY_TYPE memType;
                amf_bool native = false;
                if (ioCaps->GetMemoryTypeAt(i, &memType, &native) == AMF_OK) {
                    std::wcout << L"\t\t\t\t" << i << L": " << g_AMFFactory.GetTrace()->GetMemoryTypeName(memType) << L" " << (native ? L"(native)" : L"") << L"\n";
                }
            }
        }
    } else {
        std::wcerr << L"ERROR: ioCaps == NULL\n";
        result = false;
    }
    return result;
}

bool VCECore::QueryIOCaps(int encCodecId, amf::AMFCapsPtr& encoderCaps) {
    if (encoderCaps == NULL) {
        return false;
    }
    if (encCodecId == VCE_CODEC_H264) {
        amf::AMF_ACCELERATION_TYPE accelType = encoderCaps->GetAccelerationType();
        std::wcout << L"\t\tAcceleration Type:" << AccelTypeToString(accelType) << L"\n";

        amf_uint32 maxProfile = 0;
        encoderCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_PROFILE, &maxProfile);
        std::wcout << L"\t\tmaximum profile:" <<maxProfile << L"\n";

        amf_uint32 maxLevel = 0;
        encoderCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_LEVEL, &maxLevel);
        std::wcout << L"\t\tmaximum level:" <<maxLevel << L"\n";

        amf_uint32 maxTemporalLayers = 0;
        encoderCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_TEMPORAL_LAYERS, &maxTemporalLayers);
        std::wcout << L"\t\tNumber of temporal Layers:" << maxTemporalLayers << L"\n";

        bool bBPictureSupported = false;
        encoderCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_BFRAMES, &bBPictureSupported);
        std::wcout << L"\t\tIsBPictureSupported:" << bBPictureSupported << L"\n\n";

        amf_uint32 maxNumOfStreams = 0;
        encoderCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_NUM_OF_STREAMS, &maxNumOfStreams);
        std::wcout << L"\t\tMax Number of streams supported:" << maxNumOfStreams << L"\n";

        amf_uint32 NumOfHWInstances = 0;
        encoderCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_NUM_OF_HW_INSTANCES, &NumOfHWInstances);
        std::wcout << L"\t\tNumber HW instances:" << NumOfHWInstances << L"\n";
    } else if (encCodecId == VCE_CODEC_HEVC) {
        amf::AMF_ACCELERATION_TYPE accelType = encoderCaps->GetAccelerationType();
        std::wcout << L"\t\tAcceleration Type:" << AccelTypeToString(accelType) << L"\n";

        amf_uint32 maxProfile = 0;
        encoderCaps->GetProperty(AMF_VIDEO_ENCODER_HEVC_CAP_MAX_PROFILE, &maxProfile);
        std::wcout << L"\t\tmaximum profile:" <<maxProfile << L"\n";

        amf_uint32 maxTier = 0;
        encoderCaps->GetProperty(AMF_VIDEO_ENCODER_HEVC_CAP_MAX_TIER, &maxTier);
        std::wcout << L"\t\tmaximum tier:" <<maxTier << L"\n";

        amf_uint32 maxLevel = 0;
        encoderCaps->GetProperty(AMF_VIDEO_ENCODER_HEVC_CAP_MAX_LEVEL, &maxLevel);
        std::wcout << L"\t\tmaximum level:" <<maxLevel << L"\n";

        amf_uint32 maxNumOfStreams = 0;
        encoderCaps->GetProperty(AMF_VIDEO_ENCODER_HEVC_CAP_NUM_OF_STREAMS, &maxNumOfStreams);
        std::wcout << L"\t\tMax Number of streams supported:" << maxNumOfStreams << L"\n";
    }
    std::wcout << L"\t\tEncoder input:\n";
    amf::AMFIOCapsPtr inputCaps;
    if (encoderCaps->GetInputCaps(&inputCaps) == AMF_OK) {
        QueryIOCaps(inputCaps);
    }

    std::wcout << L"\t\tEncoder output:\n";
    amf::AMFIOCapsPtr outputCaps;
    if (encoderCaps->GetOutputCaps(&outputCaps) == AMF_OK) {
        QueryIOCaps(outputCaps);
    }
    return true;
}


AMF_RESULT VCECore::initEncoder(VCEParam *prm) {
    AMF_RESULT res = AMF_OK;

    if (m_pVCELog->getLogLevel() <= VCE_LOG_DEBUG) {
        TCHAR cpuInfo[256] = { 0 };
        TCHAR gpu_info[1024] = { 0 };
        std::wstring deviceName = (m_deviceDX9.GetDevice() == nullptr) ? m_deviceDX11.GetDisplayDeviceName() : m_deviceDX9.GetDisplayDeviceName();
        deviceName = str_replace(deviceName, L" (TM)", L"");
        deviceName = str_replace(deviceName, L" (R)", L"");
        deviceName = str_replace(deviceName, L" Series", L"");
        getCPUInfo(cpuInfo, _countof(cpuInfo));
        getGPUInfo("Advanced Micro Devices", gpu_info, _countof(gpu_info));
        PrintMes(VCE_LOG_DEBUG, _T("VCEEnc    %s (%s)\n"), VER_STR_FILEVERSION_TCHAR, BUILD_ARCH_STR);
        PrintMes(VCE_LOG_DEBUG, _T("OS        %s (%s)\n"), getOSVersion().c_str(), is_64bit_os() ? _T("x64") : _T("x86"));
        PrintMes(VCE_LOG_DEBUG, _T("CPU Info  %s\n"), cpuInfo);
        PrintMes(VCE_LOG_DEBUG, _T("GPU Info  %s [%s]\n"), wstring_to_string(deviceName).c_str(), gpu_info);
    }

    m_VCECodecId = prm->nCodecId;
    if (AMF_OK != (res = g_AMFFactory.GetFactory()->CreateComponent(m_pContext, list_codec_key[prm->nCodecId], &m_pEncoder))) {
        PrintMes(VCE_LOG_ERROR, _T("Failed to AMFCreateComponent.\n"));
        return AMF_FAIL;
    }
    PrintMes(VCE_LOG_DEBUG, _T("initialized Encoder component.\n"));

    m_Params.SetParamDescription(PARAM_NAME_INPUT,         ParamCommon, L"Input file name", NULL);
    m_Params.SetParamDescription(PARAM_NAME_INPUT_WIDTH,   ParamCommon, L"Input Frame width (integer, default = 0)", ParamConverterInt64);
    m_Params.SetParamDescription(PARAM_NAME_INPUT_HEIGHT,  ParamCommon, L"Input Frame height (integer, default = 0)", ParamConverterInt64);
    m_Params.SetParamDescription(PARAM_NAME_OUTPUT,        ParamCommon, L"Output file name", NULL);
    m_Params.SetParamDescription(PARAM_NAME_OUTPUT_WIDTH,  ParamCommon, L"Output Frame width (integer, default = 0)", ParamConverterInt64);
    m_Params.SetParamDescription(PARAM_NAME_OUTPUT_HEIGHT, ParamCommon, L"Output Frame height (integer, default = 0)", ParamConverterInt64);
    m_Params.SetParamDescription(PARAM_NAME_ENGINE,        ParamCommon, L"Specifies decoder/encoder engine type (DX9, DX11)", NULL);
    m_Params.SetParamDescription(PARAM_NAME_ADAPTERID,     ParamCommon, L"Specifies adapter ID (integer, default = 0)", ParamConverterInt64);
    m_Params.SetParamDescription(PARAM_NAME_CAPABILITY,    ParamCommon, L"Enable/Disable to display the device capabilities (true, false default =  false)", ParamConverterBoolean);

    switch (prm->nCodecId) {
    case VCE_CODEC_H264:
        RegisterEncoderParamsAVC(&m_Params);
        break;
    case VCE_CODEC_HEVC:
        RegisterEncoderParamsHEVC(&m_Params);
        break;
    default:
        PrintMes(VCE_LOG_ERROR, _T("Unknown Codec.\n"));
        return AMF_FAIL;
    }

    m_Params.SetParamAsString(PARAM_NAME_INPUT,     tchar_to_wstring(prm->pInputFile));
    m_Params.SetParamAsString(PARAM_NAME_OUTPUT,    tchar_to_wstring(prm->pOutputFile));
    m_Params.SetParam(PARAM_NAME_ADAPTERID, (amf_int64)0);

    int nGOPLen = prm->nGOPLen;
    if (nGOPLen == 0) {
        nGOPLen = (int)(m_inputInfo.fps.num / (double)m_inputInfo.fps.den + 0.5) * 10;
    }
    //VCEにはlevelを自動で設定してくれる機能はないようで、"0"などとするとエラー終了してしまう。
    if (prm->codecParam[prm->nCodecId].nLevel == 0) {
        prm->codecParam[prm->nCodecId].nLevel = (prm->nCodecId == VCE_CODEC_HEVC) ? AMF_LEVEL_4_1 : 41;
    }

    m_Params.SetParam(PARAM_NAME_INPUT_WIDTH,   m_inputInfo.srcWidth);
    m_Params.SetParam(PARAM_NAME_INPUT_HEIGHT,  m_inputInfo.srcHeight);
    m_Params.SetParam(PARAM_NAME_OUTPUT_WIDTH,  m_inputInfo.dstWidth);
    m_Params.SetParam(PARAM_NAME_OUTPUT_HEIGHT, m_inputInfo.dstHeight);
    m_Params.SetParam(PARAM_NAME_CAPABILITY,    false);
    m_Params.SetParam(SETFRAMEPARAMFREQ_PARAM_NAME,   0);
    m_Params.SetParam(SETDYNAMICPARAMFREQ_PARAM_NAME, 0);

    m_Params.SetParam(AMF_PARAM_FRAMESIZE(prm->nCodecId),      AMFConstructSize(m_inputInfo.dstWidth, m_inputInfo.dstHeight));
    m_Params.SetParam(AMF_PARAM_FRAMERATE(prm->nCodecId),      AMFConstructRate(m_inputInfo.fps.num, m_inputInfo.fps.den));
    m_Params.SetParam(AMF_PARAM_ASPECT_RATIO(prm->nCodecId),   AMFConstructRatio(m_inputInfo.AspectRatioW, m_inputInfo.AspectRatioH));
    m_Params.SetParam(AMF_PARAM_USAGE(prm->nCodecId),          (amf_int64)((prm->nCodecId == VCE_CODEC_HEVC) ? AMF_VIDEO_ENCODER_HEVC_USAGE_TRANSCONDING : AMF_VIDEO_ENCODER_USAGE_TRANSCONDING));
    m_Params.SetParam(AMF_PARAM_PROFILE(prm->nCodecId),        (amf_int64)prm->codecParam[prm->nCodecId].nProfile);
    m_Params.SetParam(AMF_PARAM_PROFILE_LEVEL(prm->nCodecId),  (amf_int64)prm->codecParam[prm->nCodecId].nLevel);
    m_Params.SetParam(AMF_PARAM_QUALITY_PRESET(prm->nCodecId), (amf_int64)get_quality_preset(prm->nCodecId)[prm->nQualityPreset]);
    m_Params.SetParam(AMF_PARAM_QP_I(prm->nCodecId),           (amf_int64)prm->nQPI);
    m_Params.SetParam(AMF_PARAM_QP_P(prm->nCodecId),           (amf_int64)prm->nQPP);
    m_Params.SetParam(AMF_PARAM_TARGET_BITRATE(prm->nCodecId), (amf_int64)prm->nBitrate * 1000);
    m_Params.SetParam(AMF_PARAM_PEAK_BITRATE(prm->nCodecId),   (amf_int64)prm->nMaxBitrate * 1000);
    m_Params.SetParam(AMF_PARAM_RATE_CONTROL_SKIP_FRAME_ENABLE(prm->nCodecId),  !!prm->bEnableSkipFrame);
    m_Params.SetParam(AMF_PARAM_RATE_CONTROL_METHOD(prm->nCodecId),             (amf_int64)prm->nRateControl);
    m_Params.SetParam(AMF_PARAM_RATE_CONTROL_PREANALYSIS_ENABLE(prm->nCodecId), (amf_int64)prm->nPreAnalysis);
    m_Params.SetParam(AMF_PARAM_VBV_BUFFER_SIZE(prm->nCodecId),                 (amf_int64)prm->nVBVBufferSize * 1000);
    m_Params.SetParam(AMF_PARAM_INITIAL_VBV_BUFFER_FULLNESS(prm->nCodecId),     (amf_int64)prm->nInitialVBVPercent);

    m_Params.SetParam(AMF_PARAM_ENFORCE_HRD(prm->nCodecId),        true);
    //m_Params.SetParam(AMF_PARAM_FILLER_DATA_ENABLE(prm->nCodecId), false);
    if (prm->bVBAQ) m_Params.SetParam(AMF_PARAM_ENABLE_VBAQ(prm->nCodecId), true);
    m_Params.SetParam(AMF_PARAM_SLICES_PER_FRAME(prm->nCodecId),               (amf_int64)prm->nSlices);
    m_Params.SetParam(AMF_PARAM_GOP_SIZE(prm->nCodecId),                       (amf_int64)nGOPLen);

    //m_Params.SetParam(AMF_PARAM_END_OF_SEQUENCE(prm->nCodecId),                false);
    m_Params.SetParam(AMF_PARAM_INSERT_AUD(prm->nCodecId),                     false);
    if (prm->nCodecId == VCE_CODEC_H264) {
        m_Params.SetParam(AMF_VIDEO_ENCODER_SCANTYPE,           (amf_int64)(is_interlaced(prm) ? AMF_VIDEO_ENCODER_SCANTYPE_INTERLACED : AMF_VIDEO_ENCODER_SCANTYPE_PROGRESSIVE));
        
        m_Params.SetParam(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP,     (amf_int64)prm->nDeltaQPBFrame);
        m_Params.SetParam(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, (amf_int64)prm->nDeltaQPBFrameRef);

        m_Params.SetParam(AMF_VIDEO_ENCODER_MIN_QP,                                (amf_int64)prm->nQPMin);
        m_Params.SetParam(AMF_VIDEO_ENCODER_MAX_QP,                                (amf_int64)prm->nQPMax);
        m_Params.SetParam(AMF_VIDEO_ENCODER_QP_B,                                  (amf_int64)prm->nQPB);

        //m_Params.SetParam(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING,       (amf_int64)0);
        m_Params.SetParam(AMF_VIDEO_ENCODER_B_PIC_PATTERN,                  (amf_int64)prm->nBframes);
        m_Params.SetParam(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE,             prm->nBframes > 0 && !!prm->bBPyramid);
        ////m_Params.SetParam(AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT, false);

        m_Params.SetParam(AMF_PARAM_MOTION_HALF_PIXEL(prm->nCodecId),              !!(prm->nMotionEst & VCE_MOTION_EST_HALF));
        m_Params.SetParam(AMF_PARAM_MOTION_QUARTERPIXEL(prm->nCodecId),            !!(prm->nMotionEst & VCE_MOTION_EST_QUATER));

        //m_Params.SetParam(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE,             (amf_int64)AMF_VIDEO_ENCODER_PICTURE_TYPE_NONE);
        m_Params.SetParam(AMF_VIDEO_ENCODER_INSERT_SPS, false);
        m_Params.SetParam(AMF_VIDEO_ENCODER_INSERT_PPS, false);
        //m_Params.SetParam(AMF_VIDEO_ENCODER_PICTURE_STRUCTURE,                (amf_int64)prm->nPicStruct);
        //m_Params.SetParam(AMF_VIDEO_ENCODER_MARK_CURRENT_WITH_LTR_INDEX,    false);
        //m_Params.SetParam(AMF_VIDEO_ENCODER_FORCE_LTR_REFERENCE_BITFIELD,   (amf_int64)0);

        //m_Params.SetParam(AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE, AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM);
        //m_Params.SetParam(AMF_VIDEO_ENCODER_OUTPUT_MARKED_LTR_INDEX, (amf_int64)-1);
        //m_Params.SetParam(AMF_VIDEO_ENCODER_OUTPUT_REFERENCED_LTR_INDEX_BITFIELD, (amf_int64)0);
    } else if (prm->nCodecId == VCE_CODEC_HEVC) {
        m_Params.SetParam(AMF_VIDEO_ENCODER_HEVC_MIN_QP_I,                        (amf_int64)prm->nQPMin);
        m_Params.SetParam(AMF_VIDEO_ENCODER_HEVC_MAX_QP_I,                        (amf_int64)prm->nQPMax);
        m_Params.SetParam(AMF_VIDEO_ENCODER_HEVC_MIN_QP_P,                        (amf_int64)prm->nQPMin);
        m_Params.SetParam(AMF_VIDEO_ENCODER_HEVC_MAX_QP_P,                        (amf_int64)prm->nQPMax);

        m_Params.SetParam(AMF_VIDEO_ENCODER_HEVC_DE_BLOCKING_FILTER_DISABLE,      !prm->bDeblockFilter);

        m_Params.SetParam(AMF_VIDEO_ENCODER_HEVC_INSERT_HEADER,                   true);
    } else {
        PrintMes(VCE_LOG_ERROR, _T("Unsupported codec.\n"));
        return res;
    }

    // Usage is preset that will set many parameters
    PushParamsToPropertyStorage(&m_Params, ParamEncoderUsage, m_pEncoder);
    // override some usage parameters
    PushParamsToPropertyStorage(&m_Params, ParamEncoderStatic, m_pEncoder);

    const amf::AMF_SURFACE_FORMAT formatIn = amf::AMF_SURFACE_NV12;
    if (AMF_OK != (res = m_pEncoder->Init(formatIn, m_inputInfo.dstWidth, m_inputInfo.dstHeight))) {
        PrintMes(VCE_LOG_ERROR, _T("Failed to initalize encoder.\n"));
        return res;
    }
    PrintMes(VCE_LOG_DEBUG, _T("initalized encoder.\n"));

    PushParamsToPropertyStorage(&m_Params, ParamEncoderDynamic, m_pEncoder);

    // Connect pipeline
    if (AMF_OK != (res = Connect(m_pFileReader, 4, CT_Direct))) {
        PrintMes(VCE_LOG_ERROR, _T("failed to connect input to pipeline.\n"));
        return res;
    }
    if (m_pDecoder) {
        if (AMF_OK != (res = Connect(PipelineElementPtr(new PipelineElementAMFComponent(m_pDecoder)), 4, CT_Direct))) {
            PrintMes(VCE_LOG_ERROR, _T("failed to connect deocder to pipeline.\n"));
            return res;
        }
    }
    if (m_pConverter) {
        if (AMF_OK != (res = Connect(PipelineElementPtr(new PipelineElementAMFComponent(m_pConverter)), 4, CT_Direct))) {
            PrintMes(VCE_LOG_ERROR, _T("failed to connect converter to pipeline.\n"));
            return res;
        }
    }
    if (AMF_OK != (res = Connect(PipelineElementPtr(new PipelineElementEncoder(m_pEncoder, &m_Params, 0, 0, true)), 10, CT_Direct))) {
        PrintMes(VCE_LOG_ERROR, _T("failed to connect encoder to pipeline.\n"));
        return res;
    }
    if (AMF_OK != (res = Connect(m_pFileWriter, 5, CT_ThreadPoll))) {
        PrintMes(VCE_LOG_ERROR, _T("failed to connect output to pipeline.\n"));
        return res;
    }
    PrintMes(VCE_LOG_DEBUG, _T("connected elements to pipeline.\n"));
    return res;
}

AMF_RESULT VCECore::init(VCEParam *prm, VCEInputInfo *inputInfo) {
    Terminate();
#if !VCE_AUO
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    hStdErr = GetStdHandle(STD_ERROR_HANDLE);
    m_apihook.hook(_T("kernel32.dll"), "WriteFile", WriteFileHook, (void **)&origWriteFileFunc);
#endif

    AMF_RESULT ret = g_AMFFactory.Init();

    if (ret != AMF_OK) {
        PrintMes(VCE_LOG_ERROR, _T("Failed to initalize VCE"));
        return AMF_NO_DEVICE;
    }

    AMF_RESULT res = g_AMFFactory.GetFactory()->CreateContext(&m_pContext);
    if (res != AMF_OK) {
        PrintMes(VCE_LOG_ERROR, _T("Failed to create AMF Context.\n"));
        return res;
    }
    PrintMes(VCE_LOG_DEBUG, _T("Created AMF Context.\n"));

    m_inputInfo = *inputInfo;

    if (prm->bTimerPeriodTuning) {
        m_bTimerPeriodTuning = true;
        timeBeginPeriod(1);
        PrintMes(VCE_LOG_DEBUG, _T("timeBeginPeriod(1)\n"));
    }

    if (AMF_OK != (res = initInput(prm, &m_inputInfo))) {
        return res;
    }

    if (AMF_OK != (res = checkParam(prm))) {
        return res;
    }

    if (AMF_OK != (res = initOutput(prm))) {
        return res;
    }

    if (AMF_OK != (res = initDevice(prm))) {
        return res;
    }

    if (AMF_OK != (res = initDecoder(prm))) {
        return res;
    }

    if (AMF_OK != (res = initConverter(prm))) {
        return res;
    }

    return initEncoder(prm);
}

AMF_RESULT VCECore::run() {
    AMF_RESULT res = AMF_OK;
    m_pEncSatusInfo->SetStart();
    if (m_pFileWriterListAudio.size() > 0) {
#if ENABLE_AVCODEC_VCE_READER
        m_thStreamSender = std::thread([this](){
            //streamのindexから必要なwriteへのポインタを返すテーブルを作成
            std::map<int, shared_ptr<CAvcodecWriter>> pWriterForAudioStreams;
            for (auto pWriter : m_pFileWriterListAudio) {
                auto pAVCodecWriter = std::dynamic_pointer_cast<CAvcodecWriter>(pWriter);
                if (pAVCodecWriter) {
                    auto trackIdList = pAVCodecWriter->GetStreamTrackIdList();
                    for (auto trackID : trackIdList) {
                        pWriterForAudioStreams[trackID] = pAVCodecWriter;
                    }
                }
            }
            AMF_RESULT sts = AMF_OK;
            PipelineState state = PipelineStateRunning;
            while ((state = GetState()) != PipelineStateRunning) {
                auto pAVCodecReader = std::dynamic_pointer_cast<CAvcodecReader>(m_pFileReader);
                vector<AVPacket> packetList;
                if (pAVCodecReader != nullptr) {
                    packetList = pAVCodecReader->GetStreamDataPackets();
                }
                //音声ファイルリーダーからのトラックを結合する
                for (const auto& reader : m_AudioReaders) {
                    auto pReader = std::dynamic_pointer_cast<CAvcodecReader>(reader);
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
                            PrintMes(VCE_LOG_ERROR, _T("Invalid writer found for track %d\n"), nTrackId);
                            return AMF_INVALID_POINTER;
                        }
                        if (AMF_OK != (sts = pWriter->WriteNextPacket(&packetList[i]))) {
                            return sts;
                        }
                    } else {
                        PrintMes(VCE_LOG_ERROR, _T("Failed to find writer for track %d\n"), nTrackId);
                        return AMF_INVALID_POINTER;
                    }
                }
                amf_sleep(100);
            }
            if (sts != AMF_OK && sts != AMF_EOF) {

            }
            return GetState() == PipelineStateEof ? AMF_OK : AMF_FAIL;
        });
#endif //ENABLE_AVCODEC_VCE_READER
    }
    res = Pipeline::Start();
    if (res != AMF_OK) {
        PrintMes(VCE_LOG_ERROR, _T("failed to start pipeline\n"));
        return res;
    }
    PrintMes(VCE_LOG_DEBUG, _T("started pipeline.\n"));

    return AMF_OK;
}

void VCECore::PrintEncoderParam() {
    PrintMes(VCE_LOG_INFO, GetEncoderParam().c_str());
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

    AMFSize frameSize;
    pProperty->GetProperty(AMF_PARAM_FRAMESIZE(m_VCECodecId), &frameSize);

    AMFRate frameRate;
    pProperty->GetProperty(AMF_PARAM_FRAMERATE(m_VCECodecId), &frameRate);

    uint32_t nMotionEst = 0x0;
    nMotionEst |= GetPropertyInt(AMF_PARAM_MOTION_HALF_PIXEL(m_VCECodecId)) ? VCE_MOTION_EST_HALF : 0;
    nMotionEst |= GetPropertyInt(AMF_PARAM_MOTION_QUARTERPIXEL(m_VCECodecId)) ? VCE_MOTION_EST_QUATER | VCE_MOTION_EST_HALF : 0;

    std::wstring deviceName = (m_deviceDX9.GetDevice() == nullptr) ? m_deviceDX11.GetDisplayDeviceName() : m_deviceDX9.GetDisplayDeviceName();
    deviceName = str_replace(deviceName, L" (TM)", L"");
    deviceName = str_replace(deviceName, L" (R)", L"");
    deviceName = str_replace(deviceName, L" Series", L"");

    mes += strsprintf(_T("VCEEnc %s (%s) / %s (%s)\n"), VER_STR_FILEVERSION_TCHAR, BUILD_ARCH_STR, getOSVersion().c_str(), is_64bit_os() ? _T("x64") : _T("x86"));
    mes += strsprintf(_T("CPU:           %s\n"), cpu_info);
    mes += strsprintf(_T("GPU:           %s [%s]\n"), wstring_to_tstring(deviceName).c_str(), gpu_info);
    mes += strsprintf(_T("Input Info:    %s\n"), m_pFileReader->GetInputInfoStr().c_str());
    if (m_inputInfo.crop.left || m_inputInfo.crop.up || m_inputInfo.crop.right || m_inputInfo.crop.bottom) {
        mes += strsprintf(_T("Crop:          %d,%d,%d,%d\n"), m_inputInfo.crop.left, m_inputInfo.crop.up, m_inputInfo.crop.right, m_inputInfo.crop.bottom);
    }
    mes += strsprintf(_T("Output:        %s  %s @ Level %s\n"),
        CodecIdToStr(m_VCECodecId),
        getPropertyDesc(AMF_PARAM_PROFILE(m_VCECodecId), get_profile_list(m_VCECodecId)).c_str(),
        getPropertyDesc(AMF_PARAM_PROFILE_LEVEL(m_VCECodecId), get_level_list(m_VCECodecId)).c_str());
    const AMF_VIDEO_ENCODER_SCANTYPE_ENUM scan_type = (m_VCECodecId == VCE_CODEC_H264) ? AMF_VIDEO_ENCODER_SCANTYPE_PROGRESSIVE : (AMF_VIDEO_ENCODER_SCANTYPE_ENUM)GetPropertyInt(AMF_VIDEO_ENCODER_SCANTYPE);
    AMFRatio aspectRatio;
    pProperty->GetProperty(AMF_PARAM_ASPECT_RATIO(m_VCECodecId), &aspectRatio);
    mes += strsprintf(_T("               %dx%d%s %d:%d %0.3ffps (%d/%dfps)\n"),
        frameSize.width, frameSize.height,
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
    const int quality_preset = GetPropertyInt(AMF_PARAM_QUALITY_PRESET(m_VCECodecId));
    mes += strsprintf(_T("Quality:       %s\n"), list_vce_quality_preset[get_quality_index(m_VCECodecId, quality_preset)].desc);
    if (GetPropertyInt(AMF_PARAM_RATE_CONTROL_METHOD(m_VCECodecId)) == get_rc_method(m_VCECodecId)[0].value) {
        mes += strsprintf(_T("CQP:           I:%d, P:%d"),
            GetPropertyInt(AMF_PARAM_QP_I(m_VCECodecId)),
            GetPropertyInt(AMF_PARAM_QP_P(m_VCECodecId)));
        if (m_VCECodecId == VCE_CODEC_H264 && GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN)) {
            mes += strsprintf(_T(", B:%d"), GetPropertyInt(AMF_VIDEO_ENCODER_QP_B));
        }
        mes += _T("\n");
    } else {
        mes += strsprintf(_T("%s:           %d kbps, Max %d kbps\n"),
            getPropertyDesc(AMF_PARAM_RATE_CONTROL_METHOD(m_VCECodecId), get_rc_method(m_VCECodecId)).c_str(),
            GetPropertyInt(AMF_PARAM_TARGET_BITRATE(m_VCECodecId)) / 1000,
            GetPropertyInt(AMF_PARAM_PEAK_BITRATE(m_VCECodecId)) / 1000);
        mes += strsprintf(_T("QP:            Min: %d, Max: %d\n"),
            GetPropertyInt(AMF_PARAM_MIN_QP(m_VCECodecId)),
            GetPropertyInt(AMF_PARAM_MAX_QP(m_VCECodecId)));
    }
    mes += strsprintf(_T("VBV Bufsize:   %d kbps\n"), GetPropertyInt(AMF_PARAM_VBV_BUFFER_SIZE(m_VCECodecId)) / 1000);
    if (m_VCECodecId == VCE_CODEC_H264 && GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN)) {
        mes += strsprintf(_T("Bframes:       %d frames, b-pyramid: %s\n"),
            GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN),
            (GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN) && GetPropertyInt(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE) ? _T("on") : _T("off")));
        if (GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN)) {
            mes += strsprintf(_T("Delta QP:      Bframe: %d, RefBframe: %d\n"), GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP), GetPropertyInt(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP));
        }
    } else {
        mes += strsprintf(_T("Bframes:       0 frames\n"));
    }
    mes += strsprintf(_T("Motion Est:    %s\n"), get_cx_desc(list_mv_presicion, nMotionEst));
    mes += strsprintf(_T("Slices:        %d\n"), GetPropertyInt(AMF_PARAM_SLICES_PER_FRAME(m_VCECodecId)));
    mes += strsprintf(_T("GOP Len:       %d frames\n"), GetPropertyInt(AMF_PARAM_GOP_SIZE(m_VCECodecId)));
    tstring others;
    if (GetPropertyBool(AMF_PARAM_RATE_CONTROL_SKIP_FRAME_ENABLE(m_VCECodecId))) {
        others += _T("skip_frame ");
    }
    bool bDeblock = true;
    switch (m_VCECodecId) {
    case VCE_CODEC_HEVC:
        bDeblock = GetPropertyBool(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER);
        break;
    case VCE_CODEC_H264:
        bDeblock = !GetPropertyBool(AMF_VIDEO_ENCODER_HEVC_DE_BLOCKING_FILTER_DISABLE);
        break;
    default:
        break;
    }
    others += (bDeblock) ? _T("deblock ") : _T("no_deblock ");
    if (m_pVCELog->getLogLevel() <= VCE_LOG_DEBUG) {
        if (GetPropertyBool(AMF_PARAM_INSERT_AUD(m_VCECodecId))) {
            others += _T("aud ");
        }
        if (m_VCECodecId == VCE_CODEC_H264) {
            if (GetPropertyBool(AMF_VIDEO_ENCODER_INSERT_SPS)) {
                others += _T("sps ");
            }
            if (GetPropertyBool(AMF_VIDEO_ENCODER_INSERT_PPS)) {
                others += _T("pps ");
            }
        } else if (m_VCECodecId == VCE_CODEC_HEVC) {
            if (GetPropertyBool(AMF_VIDEO_ENCODER_HEVC_INSERT_HEADER)) {
                others += _T("sps pps vps ");
            }
        }
    }
    if (GetPropertyBool(AMF_PARAM_ENFORCE_HRD(m_VCECodecId))) {
        others += _T("hrd ");
    }
    if (GetPropertyBool(AMF_PARAM_FILLER_DATA_ENABLE(m_VCECodecId))) {
        others += _T("filler ");
    }
    if (GetPropertyBool(AMF_PARAM_ENABLE_VBAQ(m_VCECodecId))) {
        others += _T("vbaq ");
    }
    int nPreAnalysis = GetPropertyInt(AMF_PARAM_RATE_CONTROL_PREANALYSIS_ENABLE(m_VCECodecId));
    if (nPreAnalysis != 0) {
        others += tstring(_T("pre-analysis:")) + get_cx_desc(get_pre_analysis_list(m_VCECodecId), nPreAnalysis) + _T(" ");
    }
    if (others.length() > 0) {
        mes += strsprintf(_T("Others:        %s\n"), others.c_str());
    }
    return mes;
}

AMF_RESULT VCECore::PrintResult() {
    m_pEncSatusInfo->WriteResults();
    return AMF_OK;
}

bool check_if_vce_available() {
    bool ret = g_AMFFactory.Init() == AMF_OK;
    g_AMFFactory.Terminate();
    return ret;
}

bool check_if_vce_hevc_available() {
    bool ret = g_AMFFactory.Init() == AMF_OK;
    if (ret) {
        amf::AMFContextPtr p_context;
        ret = g_AMFFactory.GetFactory()->CreateContext(&p_context) == AMF_OK;
        if (ret) {
            DeviceDX9 device;
            ret = device.Init(true, 0, false, 1280, 720) == AMF_OK;
            if (ret) {
                ret = p_context->InitDX9(device.GetDevice()) == AMF_OK;
            }
            amf::AMFComponentPtr p_encoder;
            ret = g_AMFFactory.GetFactory()->CreateComponent(p_context, list_codec_key[VCE_CODEC_HEVC], &p_encoder) == AMF_OK;
            if (p_encoder) {
                p_encoder->Terminate();
            }
            device.Terminate();
            if (p_context) {
                p_context->Terminate();
            }
        }
    }
    g_AMFFactory.Terminate();
    return ret;
}
