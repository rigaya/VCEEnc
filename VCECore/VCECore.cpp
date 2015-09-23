//  -----------------------------------------------------------------------------------------
//    VCEEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <cmath>
#include <tchar.h>
#include <process.h>
#pragma comment(lib, "winmm.lib") 

#include <iostream>

#include "VCECore.h"
#include "VCEParam.h"
#include "VCEVersion.h"
#include "VCEInput.h"
#include "VCEInputRaw.h"
#include "VCEInputAvs.h"
#include "VCEInputVpy.h"
#include "EncoderParams.h"

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

class VCECore::PipelineElementAMFComponent : public PipelineElement {
public:
    PipelineElementAMFComponent(amf::AMFComponentPtr pComponent) :
        m_pComponent(pComponent) {

    }

    virtual ~PipelineElementAMFComponent() {
    }

    virtual AMF_RESULT SubmitInput(amf::AMFData* pData) {
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
    virtual AMF_RESULT Drain() {
        return m_pComponent->Drain();
    }
protected:
    amf::AMFComponentPtr m_pComponent;
};

class VCECore::PipelineElementEncoder : public PipelineElementAMFComponent {
public:
    PipelineElementEncoder(amf::AMFComponentPtr pComponent,
        ParametersStorage* pParams, amf_int64 frameParameterFreq,
        amf_int64 dynamicParameterFreq) :
        PipelineElementAMFComponent(pComponent), m_pParams(pParams),
        m_framesSubmitted(0), m_framesQueried(0),
        m_frameParameterFreq(frameParameterFreq),
        m_dynamicParameterFreq(dynamicParameterFreq),
        m_maxLatencyTime(0), m_TotalLatencyTime(0),
        m_maxLatencyFrame(0), m_LastReadyFrameTime(0) {

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
                std::wcout << L"\t\t\t\t" << i << L": " << amf::AMFSurfaceGetFormatName(format) << L" " << (native ? L"(native)" : L"") << L"\n";
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
                    std::wcout << L"\t\t\t\t" << i << L": " << amf::AMFGetMemoryTypeName(memType) << L" " << (native ? L"(native)" : L"") << L"\n";
                }
            }
        }
    } else {
        std::wcerr << L"ERROR: ioCaps == NULL\n";
        result = false;
    }
    return result;
}

bool VCECore::QueryEncoderForCodec(const wchar_t *componentID, amf::AMFCapabilityManagerPtr& capsManager) {
    std::wcout << L"\tCodec " << componentID << L"\n";
    amf::AMFEncoderCapsPtr encoderCaps;
    bool result = false;
    if (capsManager->GetEncoderCaps(componentID, &encoderCaps) == AMF_OK) {
        amf::AMF_ACCELERATION_TYPE accelType = encoderCaps->GetAccelerationType();
        std::wcout << L"\t\tAcceleration Type:" << AccelTypeToString(accelType) << L"\n";

        amf::H264EncoderCapsPtr encoderH264Caps = (amf::H264EncoderCapsPtr)encoderCaps;

        amf_uint32 numProfiles = encoderH264Caps->GetNumOfSupportedProfiles();
        amf_uint32 numLevels = encoderH264Caps->GetNumOfSupportedLevels();
        std::wcout << L"\t\tnumber of supported profiles:" <<numProfiles << L"\n";

        for (amf_uint32 i = 0; i < numProfiles; i++) {
            std::wcout << L"\t\t\t" << encoderH264Caps->GetProfile(i) << L"\n";

        }
        std::wcout << L"\t\tnumber of supported levels:" << numLevels << L"\n";

        for (amf_uint32 i = 0; i < numLevels; i++) {
            std::wcout << L"\t\t\t" << encoderH264Caps->GetLevel(i) << L"\n";

        }

        std::wcout << L"\t\tnumber of supported Rate Control Metheds:" << encoderH264Caps->GetNumOfRateControlMethods() << L"\n";

        for (amf_int32 i = 0; i < encoderH264Caps->GetNumOfRateControlMethods(); i++) {
            std::wcout << L"\t\t\t" << encoderH264Caps->GetRateControlMethod(i) << L"\n";

        }

        std::wcout << L"\t\tNumber of temporal Layers:" << encoderH264Caps->GetMaxNumOfTemporalLayers() << L"\n";
        std::wcout << L"\t\tMax Supported Job Priority:" << encoderH264Caps->GetMaxSupportedJobPriority() << L"\n";
        std::wcout << L"\t\tIsBPictureSupported:" << encoderH264Caps->IsBPictureSupported() << L"\n\n";
        std::wcout << L"\t\tMax Number of streams supported:" << encoderH264Caps->GetMaxNumOfStreams() << L"\n";
        std::wcout << L"\t\tEncoder input:\n";
        amf::AMFIOCapsPtr inputCaps;
        if (encoderCaps->GetInputCaps(&inputCaps) == AMF_OK) {
            result = QueryIOCaps(inputCaps);
        }

        std::wcout << L"\t\tEncoder output:\n";
        amf::AMFIOCapsPtr outputCaps;
        if (encoderCaps->GetOutputCaps(&outputCaps) == AMF_OK) {
            result = QueryIOCaps(outputCaps);
        }
        return true;
    } else {
        std::wcout << AccelTypeToString(amf::AMF_ACCEL_NOT_SUPPORTED) << L"\n";
        return false;
    }
}

bool VCECore::QueryEncoderCaps(amf::AMFCapabilityManagerPtr& capsManager) {
    std::wcout << L"Querying video encoder capabilities...\n";

    return  QueryEncoderForCodec(AMFVideoEncoderVCE_AVC, capsManager);
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

    (*m_pVCELog)(log_level, buffer.data());
}





VCECore::VCECore() :
    m_pVCELog(),
    m_bTimerPeriodTuning(true),
    m_pInput(),
    m_pOutput(),
    m_pStatus(),
    m_inputInfo(),
    m_pContext(),
    m_pStreamOut(),
    m_pEncoder(),
    m_deviceDX9(),
    m_deviceDX11(),
    m_Params() {
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
    Pipeline::Stop();
    PrintMes(VCE_LOG_DEBUG, _T("Pipeline Stopped.\n"));

    m_pStreamOut = NULL;

    if (m_pEncoder != nullptr) {
        m_pEncoder->Terminate();
        m_pEncoder = nullptr;
    }

    if (m_pContext != nullptr) {
        m_pContext->Terminate();
        m_pContext = nullptr;
    }

    m_deviceDX9.Terminate();
    m_deviceDX11.Terminate();

    m_pInput.reset();
    m_pOutput.reset();
    m_pStatus.reset();
    m_pVCELog.reset();
}

AMF_RESULT VCECore::initInput(VCEParam *prm) {
#if !VCE_AUO
    m_pVCELog.reset(new VCELog(prm->pStrLog, prm->nLogLevel));
    m_pStatus.reset(new VCEStatus());

    if (prm->nInputType == VCE_INPUT_NONE) {
        if (check_ext(prm->pInputFile, { ".y4m" })) {
            prm->nInputType = VCE_INPUT_Y4M;
#if ENABLE_AVISYNTH_READER
        } else if (check_ext(prm->pInputFile, { ".avs" })) {
            prm->nInputType = VCE_INPUT_AVS;
#endif
#if ENABLE_VAPOURSYNTH_READER
        } else if (check_ext(prm->pInputFile, { ".vpy" })) {
            prm->nInputType = VCE_INPUT_VPY;
#endif
        } else {
            prm->nInputType = VCE_INPUT_RAW;
        }
    }

    VCEInputRawParam rawParam = { 0 };
#if ENABLE_AVISYNTH_READER
    VCEInputAvsParam avsParam = { 0 };
#endif
#if ENABLE_VAPOURSYNTH_READER
    VCEInputVpyParam vpyParam = { 0 };
#endif
    if (prm->nInputType == VCE_INPUT_Y4M || prm->nInputType == VCE_INPUT_RAW) {
        rawParam.y4m = prm->nInputType == VCE_INPUT_Y4M;
        rawParam.srcFile = prm->pInputFile;
        m_inputInfo.pPrivateParam = &rawParam;
        m_pInput.reset(new VCEInputRaw());
#if ENABLE_AVISYNTH_READER
    } else if (prm->nInputType == VCE_INPUT_AVS) {
        avsParam.srcFile = prm->pInputFile;
        m_inputInfo.pPrivateParam = &avsParam;
        m_pInput.reset(new VCEInputAvs());
#endif
#if ENABLE_VAPOURSYNTH_READER
    } else if (prm->nInputType == VCE_INPUT_VPY || prm->nInputType == VCE_INPUT_VPY_MT) {
        vpyParam.srcFile = prm->pInputFile;
        vpyParam.bVpyMt = prm->nInputType == VCE_INPUT_VPY_MT;
        m_inputInfo.pPrivateParam = &vpyParam;
        m_pInput.reset(new VCEInputVpy());
#endif
    }
    auto ret = m_pInput->init(m_pVCELog, m_pStatus, &m_inputInfo, m_pContext);
    if (ret != AMF_OK) {
        PrintMes(VCE_LOG_ERROR, _T("Error: %s\n"), m_pInput->getMessage().c_str());
        return ret;
    }
#endif
    return AMF_OK;
}

AMF_RESULT VCECore::checkParam(VCEParam *prm) {
    auto srcInfo = m_pInput->GetInputInfo();
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
    prm->nQPMax = clamp(prm->nQPMax, 0, 51);
    prm->nQPMin = clamp(prm->nQPMin, 0, 51);
    prm->nQPI   = clamp(prm->nQPI,   0, 51);
    prm->nQPP   = clamp(prm->nQPP,   0, 51);
    prm->nQPB   = clamp(prm->nQPB,   0, 51);

    return AMF_OK;
}

AMF_RESULT VCECore::initOutput(VCEParam *prm) {
    m_pStatus->init(m_pVCELog, m_inputInfo.fps, m_inputInfo.frames);

    m_pOutput.reset(new VCEOutput());

    auto ret = m_pOutput->init(prm->pOutputFile, m_pVCELog, m_pStatus);
    if (ret != AMF_OK) {
        PrintMes(VCE_LOG_ERROR, _T("Error: %s\n"), m_pOutput->getMessage().c_str());
        return ret;
    }
    return ret;
}

AMF_RESULT VCECore::initEncoder(VCEParam *prm) {
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
        PrintMes(VCE_LOG_DEBUG, _T("OS        %s (%s)\n"), getOSVersion(), is_64bit_os() ? _T("x64") : _T("x86"));
        PrintMes(VCE_LOG_DEBUG, _T("CPU Info  %s\n"), cpuInfo);
        PrintMes(VCE_LOG_DEBUG, _T("GPU Info  %s [%s]\n"), wchar_to_tstring(deviceName).c_str(), gpu_info);
    }

    if (AMF_OK != (res = AMFCreateComponent(m_pContext, list_codecs[prm->nCodecId], &m_pEncoder))) {
        PrintMes(VCE_LOG_ERROR, _T("Failed to AMFCreateComponent.\n"));
        return AMF_FAIL;
    }
    PrintMes(VCE_LOG_DEBUG, _T("initialized Encoder component.\n"));

    m_Params.SetParamDescription(PARAM_NAME_INPUT,         ParamCommon, L"Input file name");
    m_Params.SetParamDescription(PARAM_NAME_INPUT_WIDTH,   ParamCommon, L"Input Frame width (integer, default = 0)");
    m_Params.SetParamDescription(PARAM_NAME_INPUT_HEIGHT,  ParamCommon, L"Input Frame height (integer, default = 0)");
    m_Params.SetParamDescription(PARAM_NAME_OUTPUT,        ParamCommon, L"Output file name");
    m_Params.SetParamDescription(PARAM_NAME_OUTPUT_WIDTH,  ParamCommon, L"Output Frame width (integer, default = 0)");
    m_Params.SetParamDescription(PARAM_NAME_OUTPUT_HEIGHT, ParamCommon, L"Output Frame height (integer, default = 0)");
    m_Params.SetParamDescription(PARAM_NAME_ENGINE,        ParamCommon, L"Specifies decoder/encoder engine type (DX9, DX11)");
    m_Params.SetParamDescription(PARAM_NAME_ADAPTERID,     ParamCommon, L"Specifies adapter ID (integer, default = 0)");
    m_Params.SetParamDescription(PARAM_NAME_CAPABILITY,    ParamCommon, L"Enable/Disable to display the device capabilities (true, false default =  false)");

    RegisterEncoderParams(&m_Params);

    m_Params.SetParamAsString(PARAM_NAME_INPUT,     tchar_to_wstring(prm->pInputFile));
    m_Params.SetParamAsString(PARAM_NAME_OUTPUT,    tchar_to_wstring(prm->pOutputFile));
    m_Params.SetParam(PARAM_NAME_ADAPTERID, (amf_int64)0);

    int nGOPLen = prm->nGOPLen;
    if (nGOPLen == 0) {
        nGOPLen = (int)(m_inputInfo.fps.num / (double)m_inputInfo.fps.den + 0.5) * 10;
    }

    m_Params.SetParam(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, (amf_int64)AMF_VIDEO_ENCODER_PICTURE_TYPE_IDR);

    m_Params.SetParam(PARAM_NAME_INPUT_WIDTH,   m_inputInfo.srcWidth);
    m_Params.SetParam(PARAM_NAME_INPUT_HEIGHT,  m_inputInfo.srcHeight);
    m_Params.SetParam(PARAM_NAME_OUTPUT_WIDTH,  m_inputInfo.dstWidth);
    m_Params.SetParam(PARAM_NAME_OUTPUT_HEIGHT, m_inputInfo.dstHeight);
    m_Params.SetParam(PARAM_NAME_CAPABILITY,    false);
    m_Params.SetParam(SETFRAMEPARAMFREQ_PARAM_NAME,   0);
    m_Params.SetParam(SETDYNAMICPARAMFREQ_PARAM_NAME, 0);

    m_Params.SetParam(AMF_VIDEO_ENCODER_FRAMESIZE,          AMFConstructSize(m_inputInfo.dstWidth, m_inputInfo.dstHeight));
    m_Params.SetParam(AMF_VIDEO_ENCODER_FRAMERATE,          AMFConstructRate(m_inputInfo.fps.num, m_inputInfo.fps.den));
    m_Params.SetParam(AMF_VIDEO_ENCODER_USAGE,              (amf_int64)AMF_VIDEO_ENCODER_USAGE_TRANSCONDING);
    m_Params.SetParam(AMF_VIDEO_ENCODER_PROFILE,            (amf_int64)prm->codecParam[prm->nCodecId].nProfile);
    //m_Params.SetParam(AMF_VIDEO_ENCODER_PROFILE_LEVEL,      (amf_int64)prm->codecParam[prm->nCodecId].nLevel);
    m_Params.SetParam(AMF_VIDEO_ENCODER_SCANTYPE,           (amf_int64)is_interlaced(prm));
    m_Params.SetParam(AMF_VIDEO_ENCODER_QUALITY_PRESET,     (amf_int64)prm->nQualityPreset);

    m_Params.SetParam(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP,     (amf_int64)prm->nDeltaQPBFrame);
    m_Params.SetParam(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, (amf_int64)prm->nDeltaQPBFrameRef);


    m_Params.SetParam(AMF_VIDEO_ENCODER_ENFORCE_HRD,        true);
    //m_Params.SetParam(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE, false);

    m_Params.SetParam(AMF_VIDEO_ENCODER_GOP_SIZE,                       (amf_int64)nGOPLen);
    m_Params.SetParam(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE,                (amf_int64)prm->nVBVBufferSize * 1000);
    m_Params.SetParam(AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS,    (amf_int64)prm->nInitialVBVPercent);

    m_Params.SetParam(AMF_VIDEO_ENCODER_MIN_QP,                         (amf_int64)prm->nQPMin);
    m_Params.SetParam(AMF_VIDEO_ENCODER_MAX_QP,                         (amf_int64)prm->nQPMax);
    m_Params.SetParam(AMF_VIDEO_ENCODER_QP_I,                           (amf_int64)prm->nQPI);
    m_Params.SetParam(AMF_VIDEO_ENCODER_QP_P,                           (amf_int64)prm->nQPP);
    m_Params.SetParam(AMF_VIDEO_ENCODER_QP_B,                           (amf_int64)prm->nQPB);
    m_Params.SetParam(AMF_VIDEO_ENCODER_TARGET_BITRATE,                 (amf_int64)prm->nBitrate * 1000);
    m_Params.SetParam(AMF_VIDEO_ENCODER_PEAK_BITRATE,                   (amf_int64)prm->nMaxBitrate * 1000);
    m_Params.SetParam(AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE, !!prm->bEnableSkipFrame);
    m_Params.SetParam(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD,            (amf_int64)prm->nRateControl);

    //m_Params.SetParam(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING,       (amf_int64)0);
    m_Params.SetParam(AMF_VIDEO_ENCODER_B_PIC_PATTERN,                  (amf_int64)prm->nBframes);
    m_Params.SetParam(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER,             !!prm->bDeblockFilter);
    m_Params.SetParam(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE,             !!prm->bBPyramid);
    m_Params.SetParam(AMF_VIDEO_ENCODER_IDR_PERIOD,                     (amf_int64)nGOPLen);
    ////m_Params.SetParam(AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT, false);
    m_Params.SetParam(AMF_VIDEO_ENCODER_SLICES_PER_FRAME,               (amf_int64)prm->nSlices);

    m_Params.SetParam(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL,              !!(prm->nMotionEst & VCE_MOTION_EST_HALF));
    m_Params.SetParam(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL,            !!(prm->nMotionEst & VCE_MOTION_EST_QUATER));


    //m_Params.SetParam(AMF_VIDEO_ENCODER_END_OF_SEQUENCE,                false);
    //m_Params.SetParam(AMF_VIDEO_ENCODER_END_OF_STREAM,                  false);
    //m_Params.SetParam(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE,             (amf_int64)AMF_VIDEO_ENCODER_PICTURE_TYPE_NONE);
    //m_Params.SetParam(AMF_VIDEO_ENCODER_INSERT_AUD,                     false);
    //m_Params.SetParam(AMF_VIDEO_ENCODER_INSERT_SPS,                     false);
    //m_Params.SetParam(AMF_VIDEO_ENCODER_INSERT_PPS,                     false);
    m_Params.SetParam(AMF_VIDEO_ENCODER_PICTURE_STRUCTURE,              (amf_int64)prm->nInterlaced);
    //m_Params.SetParam(AMF_VIDEO_ENCODER_MARK_CURRENT_WITH_LTR_INDEX,    false);
    //m_Params.SetParam(AMF_VIDEO_ENCODER_FORCE_LTR_REFERENCE_BITFIELD,   (amf_int64)0);

    //m_Params.SetParam(AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE, AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM);
    //m_Params.SetParam(AMF_VIDEO_ENCODER_OUTPUT_MARKED_LTR_INDEX, (amf_int64)-1);
    //m_Params.SetParam(AMF_VIDEO_ENCODER_OUTPUT_REFERENCED_LTR_INDEX_BITFIELD, (amf_int64)0);

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
    if (AMF_OK != (res = Connect(m_pInput, 4))) {
        PrintMes(VCE_LOG_ERROR, _T("failed to connect input to pipeline.\n"));
        return res;
    }
    if (AMF_OK != (res = Connect(PipelineElementPtr(new PipelineElementEncoder(m_pEncoder, &m_Params, 0, 0)), 10))) {
        PrintMes(VCE_LOG_ERROR, _T("failed to connect encoder to pipeline.\n"));
        return res;
    }
    if (AMF_OK != (res = Connect(m_pOutput, 5))) {
        PrintMes(VCE_LOG_ERROR, _T("failed to connect output to pipeline.\n"));
        return res;
    }
    PrintMes(VCE_LOG_DEBUG, _T("connected elements to pipeline.\n"));
    return res;
}

AMF_RESULT VCECore::init(VCEParam *prm, VCEInputInfo *inputInfo) {
    Terminate();

    tstring vce_check;
    if (!check_if_vce_available(vce_check)) {
        PrintMes(VCE_LOG_ERROR, _T("%s\n"), vce_check);
        return AMF_NO_DEVICE;
    }

    AMF_RESULT res = AMFCreateContext(&m_pContext);
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

    if (AMF_OK != (res = initInput(prm))) {
        return res;
    }

    if (AMF_OK != (res = checkParam(prm))) {
        return res;
    }

    if (AMF_OK != (res = initOutput(prm))) {
        return res;
    }

    return initEncoder(prm);
}

AMF_RESULT VCECore::run() {
    AMF_RESULT res = AMF_OK;
    m_pStatus->SetStart();
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
        return wchar_to_tstring(pProp);
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
    pProperty->GetProperty(AMF_VIDEO_ENCODER_FRAMESIZE, &frameSize);

    AMFRate frameRate;
    pProperty->GetProperty(AMF_VIDEO_ENCODER_FRAMERATE, &frameRate);

    uint32_t nMotionEst = 0x0;
    nMotionEst |= GetPropertyInt(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL) ? VCE_MOTION_EST_HALF : 0;
    nMotionEst |= GetPropertyInt(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL) ? VCE_MOTION_EST_QUATER | VCE_MOTION_EST_HALF : 0;

    std::wstring deviceName = (m_deviceDX9.GetDevice() == nullptr) ? m_deviceDX11.GetDisplayDeviceName() : m_deviceDX9.GetDisplayDeviceName();
    deviceName = str_replace(deviceName, L" (TM)", L"");
    deviceName = str_replace(deviceName, L" (R)", L"");
    deviceName = str_replace(deviceName, L" Series", L"");

    mes += strsprintf(_T("VCEEnc %s (%s) / %s (%s)\n"), VER_STR_FILEVERSION_TCHAR, BUILD_ARCH_STR, getOSVersion(), is_64bit_os() ? _T("x64") : _T("x86"));
    mes += strsprintf(_T("CPU:           %s\n"), cpu_info);
    mes += strsprintf(_T("GPU:           %s [%s]\n"), wchar_to_tstring(deviceName).c_str(), gpu_info);
    mes += strsprintf(_T("Input:         %s\n"), m_pInput->GetInputInfoStr().c_str());
    if (m_inputInfo.crop.left || m_inputInfo.crop.up || m_inputInfo.crop.right || m_inputInfo.crop.bottom) {
        mes += strsprintf(_T("Crop:          %d,%d,%d,%d\n"), m_inputInfo.crop.left, m_inputInfo.crop.up, m_inputInfo.crop.right, m_inputInfo.crop.bottom);
    }
    mes += strsprintf(_T("Output:        H.264/AVC %s @ %s %dx%d%s %d/%d(%.3f) fps\n"),
        getPropertyDesc(AMF_VIDEO_ENCODER_PROFILE, list_avc_profile).c_str(),
        getPropertyDesc(AMF_VIDEO_ENCODER_PROFILE_LEVEL, list_avc_level).c_str(),
        frameSize.width, frameSize.height, GetPropertyInt(AMF_VIDEO_ENCODER_SCANTYPE) ? _T("i") : _T("p"), frameRate.num, frameRate.den, frameRate.num / (double)frameRate.den);
    mes += strsprintf(_T("Quality:       %s\n"), getPropertyDesc(AMF_VIDEO_ENCODER_QUALITY_PRESET, list_vce_quality_preset).c_str());
    if (GetPropertyInt(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD) == AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTRAINED_QP) {
        mes += strsprintf(_T("CQP:           I:%d, P:%d"),
            GetPropertyInt(AMF_VIDEO_ENCODER_QP_I),
            GetPropertyInt(AMF_VIDEO_ENCODER_QP_P));
        if (GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN)) {
            mes += strsprintf(_T(", B:%d"), GetPropertyInt(AMF_VIDEO_ENCODER_QP_B));
        }
        mes += _T("\n");
    } else {
        mes += strsprintf(_T("%s:           %d kbps, Max %d kbps\n"),
            getPropertyDesc(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, list_vce_rc_method).c_str(),
            GetPropertyInt(AMF_VIDEO_ENCODER_TARGET_BITRATE) / 1000,
            GetPropertyInt(AMF_VIDEO_ENCODER_PEAK_BITRATE) / 1000);
        mes += strsprintf(_T("QP:            Min: %d, Max: %d\n"),
            GetPropertyInt(AMF_VIDEO_ENCODER_MIN_QP),
            GetPropertyInt(AMF_VIDEO_ENCODER_MAX_QP));
    }
    mes += strsprintf(_T("VBV Bufsize:   %d kbps\n"), GetPropertyInt(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE) / 1000);
    mes += strsprintf(_T("Bframes:       %d frames, b-pyramid: %s\n"),
        GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN),
        (GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN) && GetPropertyInt(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE) ? _T("on") : _T("off")));
    if (GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_PATTERN)) {
        mes += strsprintf(_T("Delta QP:      Bframe: %d, RefBframe: %d\n"), GetPropertyInt(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP), GetPropertyInt(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP));
    }
    mes += strsprintf(_T("Motion Est:    %s\n"), get_cx_desc(list_mv_presicion, nMotionEst));
    mes += strsprintf(_T("Slices:        %d\n"), GetPropertyInt(AMF_VIDEO_ENCODER_SLICES_PER_FRAME));
    mes += strsprintf(_T("GOP Len:       %d frames\n"), GetPropertyInt(AMF_VIDEO_ENCODER_GOP_SIZE));
    tstring others;
    if (GetPropertyBool(AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE)) {
        others += _T("skip_frame ");
    }
    if (!GetPropertyBool(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER)) {
        others += _T("no_deblock ");
    } else {
        others += _T("deblock ");
    }
    if (m_pVCELog->getLogLevel() <= VCE_LOG_DEBUG) {
        if (GetPropertyBool(AMF_VIDEO_ENCODER_INSERT_AUD)) {
            others += _T("aud ");
        }
        if (GetPropertyBool(AMF_VIDEO_ENCODER_INSERT_SPS)) {
            others += _T("sps ");
        }
        if (GetPropertyBool(AMF_VIDEO_ENCODER_INSERT_PPS)) {
            others += _T("pps ");
        }
    }
    if (GetPropertyBool(AMF_VIDEO_ENCODER_ENFORCE_HRD)) {
        others += _T("hrd ");
    }
    if (GetPropertyBool(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE)) {
        others += _T("filler ");
    }
    if (others.length() > 0) {
        mes += strsprintf(_T("Others:        %s\n"), others.c_str());
    }
    return mes;
}

AMF_RESULT VCECore::PrintResult() {
    m_pStatus->WriteResults();
    return AMF_OK;
}

bool check_if_vce_available(tstring& mes) {
#if _M_IX86
    const TCHAR *dllNameCore      = _T("amf-core-windesktop32.dll");
    const TCHAR *dllNameComponent = _T("amf-component-vce-windesktop32.dll");
#else
    const TCHAR *dllNameCore      = _T("amf-core-windesktop64.dll");
    const TCHAR *dllNameComponent = _T("amf-component-vce-windesktop64.dll");
#endif
    HMODULE hModuleCore = LoadLibrary(dllNameCore);
    HMODULE hModuleComponent = LoadLibrary(dllNameComponent);

    bool ret = true;
    mes = _T("");
    if (hModuleComponent == NULL) {
        ret = false;
        mes += tstring(dllNameComponent) + _T(" not found on system");
    } else {
        FreeLibrary(hModuleComponent);
    }
    if (hModuleCore == NULL) {
        ret = false;
        mes += tstring(dllNameCore) + _T(" not found on system");
    } else {
        FreeLibrary(hModuleCore);
    }
    if (ret) {
        uint32_t count = 0;
        amf::AMFContextPtr pContext;
        amf::AMFComponentPtr pEncoder;
        DeviceDX9 deviceDX9;
        if (   AMF_OK != deviceDX9.GetAdapterCount(&count)
            || count == 0
            || AMF_OK != AMFCreateContext(&pContext)
            || AMF_OK != pContext->InitDX9(deviceDX9.GetDevice())
            || AMF_OK != AMFCreateComponent(pContext, list_codecs[0], &pEncoder)) {
            ret = false;
            mes = _T("System has no GPU supporting VCE.");
        }

        if (pEncoder != nullptr) {
            pEncoder->Terminate();
            pEncoder = nullptr;
        }

        if (pContext != nullptr) {
            pContext->Terminate();
            pContext = nullptr;
        }

        deviceDX9.Terminate();
    }
    return ret;
}

bool check_if_vce_available() {
    tstring dummy;
    return check_if_vce_available(dummy);
}
