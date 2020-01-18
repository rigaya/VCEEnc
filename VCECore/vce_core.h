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

#include <thread>
#include <future>
#pragma warning(push)
#pragma warning(disable:4100)
#include "VideoEncoderVCE.h"
#include "Factory.h"
#include "Trace.h"

#include "rgy_version.h"
#include "rgy_err.h"
#include "rgy_util.h"
#include "rgy_log.h"
#include "rgy_input.h"
#include "rgy_output.h"
#include "rgy_opencl.h"
#include "rgy_device.h"
#include "vce_param.h"
#include "vce_filter.h"
#include "vce_filter_ssim.h"

#pragma warning(pop)

#define RGY_PROP_TIMESTAMP L"RGYPropTimestamp"
#define RGY_PROP_DURATION  L"RGYPropDuration"

const TCHAR *AMFRetString(AMF_RESULT ret);

#if ENABLE_AVSW_READER
struct AVChapter;
#endif //#if ENABLE_AVSW_READER

class RGYPipelineFrame {
    RGYPipelineFrame(amf::AMFSurfacePtr surf) : surface(surf), frame() {};
    RGYPipelineFrame(shared_ptr<RGYCLFrame> clframe) : surface(), frame(clframe) {};
    ~RGYPipelineFrame() {
        surface->Release();
        frame.reset();
    };
protected:
    amf::AMFSurfacePtr surface;
    shared_ptr<RGYCLFrame> frame;
};

enum RGYRunState {
    RGY_STATE_STOPPED,
    RGY_STATE_RUNNING,
    RGY_STATE_ERROR,
    RGY_STATE_ABORT,
    RGY_STATE_EOF
};

class RGYLogTracer : public amf::AMFTraceWriter {
public:
    RGYLogTracer() : m_pLog() {};
    virtual ~RGYLogTracer() { m_pLog.reset(); }
    virtual void init(shared_ptr<RGYLog> pLog) { m_pLog = pLog; };
    virtual void AMF_CDECL_CALL Write(const wchar_t *scope, const wchar_t *message) override {
        m_pLog->write(RGY_LOG_INFO, _T("[%s] %s"), scope, message);
    };
    virtual void AMF_CDECL_CALL Flush() override {
    };
    virtual void reset() {
        m_pLog.reset();
    }
protected:
    shared_ptr<RGYLog> m_pLog;
};

class VCECore {
public:
    VCECore();
    virtual ~VCECore();

    virtual RGY_ERR init(VCEParam *prm);
    virtual RGY_ERR initLog(int loglevel);
    virtual RGY_ERR initLog(VCEParam *prm);
    virtual RGY_ERR initAMFFactory();
    virtual RGY_ERR initContext(int log_level);
    virtual RGY_ERR initDevice(const int deviceId, const bool interopD3d9, const bool interopD3d11);
    virtual RGY_ERR initInput(VCEParam *pParams);
    virtual RGY_ERR initOutput(VCEParam *prm);
    virtual RGY_ERR run();
    void Terminate();

    void PrintMes(int log_level, const TCHAR *format, ...);

    tstring GetEncoderParam();
    void PrintEncoderParam();
    void PrintResult();

    RGY_ERR getEncCaps(RGY_CODEC codec, amf::AMFCapsPtr &encoderCaps);
    tstring QueryIOCaps(amf::AMFIOCapsPtr& ioCaps);
    tstring QueryIOCaps(RGY_CODEC codec, amf::AMFCapsPtr& encoderCaps);

    void SetAbortFlagPointer(bool *abortFlag);
protected:
    static tstring AccelTypeToString(amf::AMF_ACCELERATION_TYPE accelType);
    virtual RGY_ERR readChapterFile(tstring chapfile);

    virtual RGY_CSP GetEncoderCSP(const VCEParam *inputParam);
    virtual RGY_ERR checkParam(VCEParam *prm);
    virtual RGY_ERR initPerfMonitor(VCEParam *prm);
    virtual RGY_ERR initDecoder(VCEParam *prm);
    virtual RGY_ERR initFilters(VCEParam *prm);
    virtual RGY_ERR initConverter(VCEParam *prm);
    virtual RGY_ERR InitChapters(VCEParam *prm);
    virtual RGY_ERR initEncoder(VCEParam *prm);
    virtual RGY_ERR initSSIMCalc(VCEParam *prm);
    virtual tstring getGPUInfo();

    virtual RGY_ERR run_decode();
    virtual RGY_ERR run_output();

    RGY_CODEC          m_encCodec;
    shared_ptr<RGYLog> m_pLog;
    bool m_bTimerPeriodTuning;
#if ENABLE_AVSW_READER
    bool                          m_keyOnChapter;        //チャプター上にキーフレームを配置する
    vector<int>                   m_keyFile;             //キーフレームの指定
    vector<unique_ptr<AVChapter>> m_Chapters;            //ファイルから読み込んだチャプター
#endif //#if ENABLE_AVSW_READER
    unique_ptr<RGYHDR10Plus>      m_hdr10plus;

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
    rgy_rational<int>  m_encFps;             //出力フレームレート
    rgy_rational<int>  m_outputTimebase;        //出力のtimebase
    int                m_encWidth;
    int                m_encHeight;
    rgy_rational<int>  m_sar;
    RGY_PICSTRUCT      m_picStruct;

    DeviceDX9 m_dx9;
    DeviceDX11 m_dx11;
    shared_ptr<RGYOpenCLContext> m_cl;
    unique_ptr<std::remove_pointer_t<HMODULE>, module_deleter> m_dll;
    amf::AMFFactory *m_pFactory;
    amf::AMFDebug *m_pDebug;
    amf::AMFTrace *m_pTrace;
    RGYLogTracer m_tracer;
    uint64_t m_AMFRuntimeVersion;
    amf::AMFContextPtr m_pContext;

    vector<unique_ptr<RGYFilter>> m_vpFilters;
    shared_ptr<RGYFilterParam>    m_pLastFilterParam;
    unique_ptr<RGYFilterSsim>     m_ssim;

    RGYRunState m_state;

    sTrimParam *m_pTrimParam;

    amf::AMFComponentPtr m_pDecoder;
    amf::AMFComponentPtr m_pEncoder;
    amf::AMFComponentPtr m_pConverter;
    std::thread m_thDecoder;
    std::thread m_thOutput;

    AMFParams m_params;

    bool *m_pAbortByUser;
};

class VCEFeatures {
public:
    VCEFeatures() : m_core() {};

    virtual ~VCEFeatures() {};

    RGY_ERR init(int deviceId, int logLevel);
    tstring checkFeatures(RGY_CODEC codec);
protected:
    std::unique_ptr<VCECore> m_core;
};