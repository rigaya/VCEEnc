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

#include <unordered_map>
#pragma warning(push)
#pragma warning(disable:4100)
#include "Factory.h"
#include "Context.h"
#include "Trace.h"
#pragma warning(pop)

#include "rgy_version.h"
#include "rgy_err.h"
#include "rgy_util.h"
#include "rgy_log.h"
#include "rgy_opencl.h"
#include "rgy_device.h"
#include "vce_param.h"

class VCEDevice {
public:
    VCEDevice(shared_ptr<RGYLog>& log, amf::AMFFactory *factory, amf::AMFTrace *trace);
    virtual ~VCEDevice();

    virtual RGY_ERR init(const int deviceId, const bool interopD3d9, const bool interopD3d11);

    amf::AMFCapsPtr getEncCaps(RGY_CODEC codec);
    amf::AMFCapsPtr getDecCaps(RGY_CODEC codec);
    tstring QueryIOCaps(amf::AMFIOCapsPtr& ioCaps);
    tstring QueryInputCaps(RGY_CODEC codec, amf::AMFCapsPtr& caps);
    tstring QueryOutputCaps(RGY_CODEC codec, amf::AMFCapsPtr& caps);
    tstring QueryIOCaps(RGY_CODEC codec, amf::AMFCapsPtr& caps);
    tstring QueryEncCaps(RGY_CODEC codec, amf::AMFCapsPtr& encoderCaps);
    tstring getGPUInfo() const;
    CodecCsp getHWDecCodecCsp();

    int id() const { return m_id; }
    const tstring &name() const { return m_devName; }
    LUID luid() const { return (m_dx11.isValid()) ? m_dx11.getLUID() : m_dx9.getLUID(); }
    amf::AMFContextPtr context() { return m_context; }
    shared_ptr<RGYOpenCLContext> cl() { return m_cl; }
    bool dx11interlop() const { return m_d3d11interlop; }
    bool dx9interlop() const { return m_d3d9interlop; }
protected:
    virtual RGY_ERR CreateContext();
    void getAllCaps();

    void PrintMes(int log_level, const tstring& str) {
        if (m_log == nullptr || log_level < m_log->getLogLevel()) {
            return;
        }
        auto lines = split(str, _T("\n"));
        for (const auto& line : lines) {
            if (line[0] != _T('\0')) {
                m_log->write(log_level, (m_devName + _T(": ") + line + _T("\n")).c_str());
            }
        }
    }
    void PrintMes(int log_level, const TCHAR *format, ... ) {
        if (m_log == nullptr || log_level < m_log->getLogLevel()) {
            return;
        }

        va_list args;
        va_start(args, format);
        int len = _vsctprintf(format, args) + 1; // _vscprintf doesn't count terminating '\0'
        tstring buffer;
        buffer.resize(len, _T('\0'));
        _vstprintf_s(&buffer[0], len, format, args);
        va_end(args);
        PrintMes(log_level, buffer);
    }

    std::shared_ptr<RGYLog> m_log;

    int m_id;
    tstring m_devName;
    bool m_d3d9interlop;
    DeviceDX9 m_dx9;
    bool m_d3d11interlop;
    DeviceDX11 m_dx11;
    std::shared_ptr<RGYOpenCLContext> m_cl;
    amf::AMFContextPtr m_context;
    amf::AMFFactory *m_factory;
    amf::AMFTrace *m_trace;
    bool m_gotCaps;
    std::unordered_map<RGY_CODEC, amf::AMFCapsPtr> m_encCaps;
    std::unordered_map<RGY_CODEC, amf::AMFCapsPtr> m_decCaps;
};
