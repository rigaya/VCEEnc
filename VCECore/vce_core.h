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

#include "rgy_version.h"
#include "rgy_err.h"
#include "rgy_util.h"
#include "rgy_log.h"
#include "vce_param.h"
#include "vce_input.h"
#include "vce_output.h"

#pragma warning(pop)

#include "api_hook.h"

const TCHAR *AMFRetString(AMF_RESULT ret);

#if ENABLE_AVSW_READER
struct AVChapter;
#endif //#if ENABLE_AVSW_READER

class VCECore : public Pipeline {
    class PipelineElementAMFComponent;
    class PipelineElementEncoder;
public:
    VCECore();
    virtual ~VCECore();

    RGY_ERR init(VCEParam *prm);
    virtual RGY_ERR initInput(VCEParam *pParams);
    RGY_ERR initOutput(VCEParam *prm);
    RGY_ERR run();
    void Terminate();

    void PrintMes(int log_level, const TCHAR *format, ...);

    tstring GetEncoderParam();
    void PrintEncoderParam();
    void PrintResult();

    static tstring QueryIOCaps(amf::AMFIOCapsPtr& ioCaps);
    static tstring QueryIOCaps(RGY_CODEC codec, amf::AMFCapsPtr& encoderCaps);
protected:
    static tstring AccelTypeToString(amf::AMF_ACCELERATION_TYPE accelType);
    RGY_ERR readChapterFile(tstring chapfile);

    virtual RGY_ERR checkParam(VCEParam *prm);
    virtual RGY_ERR initDeviceDX9(VCEParam *prm);
    virtual RGY_ERR initDeviceDX11(VCEParam *prm);
    virtual RGY_ERR initDevice(VCEParam *prm);
    virtual RGY_ERR initDecoder(VCEParam *prm);
    virtual RGY_ERR initConverter(VCEParam *prm);
    virtual RGY_ERR initEncoder(VCEParam *prm);

    RGY_CODEC          m_encCodec;
    shared_ptr<RGYLog> m_pLog;
    bool m_bTimerPeriodTuning;
#if ENABLE_AVSW_READER
    vector<unique_ptr<AVChapter>> m_AVChapterFromFile;
#endif //#if ENABLE_AVSW_READER
    sTrimParam m_trimParam;
    shared_ptr<RGYInput> m_pFileReader;
    vector<shared_ptr<RGYInput>> m_AudioReaders;
    shared_ptr<RGYOutput> m_pFileWriter;
    vector<shared_ptr<RGYOutput>> m_pFileWriterListAudio;
    shared_ptr<EncodeStatus> m_pStatus;
    shared_ptr<CPerfMonitor> m_pPerfMonitor;

    int                m_nProcSpeedLimit;       //処理速度制限 (0で制限なし)
    RGYAVSync          m_nAVSyncMode;           //映像音声同期設定
    rgy_rational<int>  m_inputFps;              //入力フレームレート
    rgy_rational<int>  m_outputFps;             //出力フレームレート
    rgy_rational<int>  m_outputTimebase;        //出力のtimebase

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
    apihook m_apihook;
};
