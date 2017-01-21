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

#pragma once

#include <d3d9.h>
#include <d3d11.h>
#include <thread>
#pragma warning(push)
#pragma warning(disable:4100)
#include "VideoEncoderVCE.h"
#include "DeviceDX9.h"
#include "DeviceDX11.h"

#include "PipelineElement.h"
#include "Pipeline.h"
#include "PipelineMod.h"
#include "ParametersStorage.h"

#include "AMFFactory.h"

#include "VCEVersion.h"
#include "VCEUtil.h"
#include "VCEParam.h"
#include "VCELog.h"
#include "VCEInput.h"
#include "VCEOutput.h"

#pragma warning(pop)

#include "api_hook.h"

#if ENABLE_AVCODEC_VCE_READER
struct AVChapter;
#endif //#if ENABLE_AVCODEC_VCE_READER

class VCECore : public Pipeline {
    class PipelineElementAMFComponent;
    class PipelineElementEncoder;
public:
    VCECore();
    virtual ~VCECore();

    AMF_RESULT init(VCEParam *prm, VCEInputInfo *inputInfo);
    virtual AMF_RESULT initInput(VCEParam *pParams, const VCEInputInfo *pInputInfo);
    AMF_RESULT initOutput(VCEParam *prm);
    AMF_RESULT run();
    void Terminate();

    void PrintMes(int log_level, const TCHAR *format, ...);

    tstring GetEncoderParam();
    void PrintEncoderParam();
    AMF_RESULT PrintResult();

    static const wchar_t* PARAM_NAME_INPUT;
    static const wchar_t* PARAM_NAME_INPUT_WIDTH;
    static const wchar_t* PARAM_NAME_INPUT_HEIGHT;

    static const wchar_t* PARAM_NAME_OUTPUT;
    static const wchar_t* PARAM_NAME_OUTPUT_WIDTH;
    static const wchar_t* PARAM_NAME_OUTPUT_HEIGHT;

    static const wchar_t* PARAM_NAME_ENGINE;

    static const wchar_t* PARAM_NAME_ADAPTERID;
    static const wchar_t* PARAM_NAME_CAPABILITY;
protected:
    std::wstring AccelTypeToString(amf::AMF_ACCELERATION_TYPE accelType);
    bool QueryIOCaps(amf::AMFIOCapsPtr& ioCaps);
    bool QueryIOCaps(int encCodecId, amf::AMFCapsPtr& encoderCaps);
    AMF_RESULT readChapterFile(tstring chapfile);

    virtual AMF_RESULT checkParam(VCEParam *prm);
    virtual AMF_RESULT initDevice(VCEParam *prm);
    virtual AMF_RESULT initDecoder(VCEParam *prm);
    virtual AMF_RESULT initConverter(VCEParam *prm);
    virtual AMF_RESULT initEncoder(VCEParam *prm);

    shared_ptr<VCELog> m_pVCELog;
    bool m_bTimerPeriodTuning;
#if ENABLE_AVCODEC_VCE_READER
    vector<unique_ptr<AVChapter>> m_AVChapterFromFile;
#endif //#if ENABLE_AVCODEC_VCE_READER
    shared_ptr<VCEInput> m_pFileReader;
    vector<shared_ptr<VCEInput>> m_AudioReaders;
    shared_ptr<VCEOutput> m_pFileWriter;
    vector<shared_ptr<VCEOutput>> m_pFileWriterListAudio;
    shared_ptr<VCEStatus> m_pEncSatusInfo;

    VCEInputInfo m_inputInfo;

    amf::AMFContextPtr m_pContext;

    amf::AMFDataStreamPtr m_pStreamOut;

    sTrimParam *m_pTrimParam;

    amf::AMFComponentPtr m_pDecoder;
    amf::AMFComponentPtr m_pEncoder;
    amf::AMFComponentPtr m_pConverter;
    std::thread m_thStreamSender;

    DeviceDX9 m_deviceDX9;
    DeviceDX11 m_deviceDX11;

    ParametersStorage m_Params;
    int m_VCECodecId;
    apihook m_apihook;
};
