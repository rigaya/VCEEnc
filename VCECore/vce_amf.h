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
#include "rgy_device.h"
#include "vce_device.h"
#include "vce_param.h"

#pragma warning(pop)
class RGYLogTracer : public amf::AMFTraceWriter {
public:
    RGYLogTracer() : m_pLog() {};
    virtual ~RGYLogTracer() { m_pLog.reset(); }
    virtual void init(shared_ptr<RGYLog> pLog) { m_pLog = pLog; };
    virtual void AMF_CDECL_CALL Write(const wchar_t *scope, const wchar_t *message) override {
        m_pLog->write(RGY_LOG_INFO, RGY_LOGT_AMF, _T("[%s] %s"), scope, str_replace(wstring_to_tstring(message), _T("\r\n"), _T("\n")).c_str());
    };
    virtual void AMF_CDECL_CALL Flush() override {
    };
    virtual void reset() {
        m_pLog.reset();
    }
protected:
    shared_ptr<RGYLog> m_pLog;
};

class VCEAMF {
public:
    VCEAMF();
    virtual ~VCEAMF();

    virtual RGY_ERR initLogLevel(RGYLogLevel loglevel);
    virtual RGY_ERR initLogLevel(const RGYParamLogLevel& loglevel);
    virtual RGY_ERR initAMFFactory();
    virtual RGY_ERR initTracer(int log_level);
    virtual void Terminate();

    void PrintMes(RGYLogLevel log_level, const TCHAR *format, ...);

    virtual std::vector<std::unique_ptr<VCEDevice>> createDeviceList(bool interopD3d9, bool interopD3d11, RGYParamInitVulkan interopVulkan, bool enableOpenCL, bool enableVppPerfMonitor, bool enableAV1HWDec, int openCLBuildThreads);

    amf::AMFFactory *factory() { return m_pFactory; }
    uint64_t runtimeVersion() const { return m_AMFRuntimeVersion;}
protected:
    shared_ptr<RGYLog> m_pLog;
    unique_ptr<std::remove_pointer_t<HMODULE>, module_deleter> m_dll;
    amf::AMFFactory *m_pFactory;
    amf::AMFDebug *m_pDebug;
    amf::AMFTrace *m_pTrace;
    RGYLogTracer m_tracer;
    uint64_t m_AMFRuntimeVersion;
};
