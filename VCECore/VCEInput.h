// -----------------------------------------------------------------------------------------
//     VCEEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2014-2016 rigaya
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
#include "VideoEncoderVCE.h"
#include "AMFPlatform.h"
#include "PlatformWindows.h"
#include "Thread.h"

#include "PipelineElement.h"

#include "VCEUtil.h"
#include "VCEParam.h"
#include "VCELog.h"
#include "VCEStatus.h"
#include "ConvertCsp.h"

class VCEInput : public PipelineElement {
public:
    VCEInput();
    virtual ~VCEInput();

    virtual AMF_RESULT init(shared_ptr<VCELog> pLog, shared_ptr<VCEStatus> pStatus, VCEInputInfo *pInfo, amf::AMFContextPtr m_pContext) = 0;

    virtual tstring getMessage() {
        return m_message;
    }

    virtual amf::AMF_SURFACE_FORMAT GetFormat() {
        return m_inputInfo.format;
    }
    virtual amf_int32 GetWidth() {
        return m_inputInfo.srcWidth;
    }
    virtual amf_int32 GetHeight() {
        return m_inputInfo.srcHeight;
    }
    virtual VCEInputInfo GetInputInfo() {
        return m_inputInfo;
    }

    virtual amf_int32 GetInputSlotCount()  override {
        return 0;
    }
    virtual tstring GetInputInfoStr() {
        return m_strInputInfo;
    }
    //virtual amf_double GetPosition() {
    //    return static_cast<amf_double> (m_framesCountRead) / m_framesCount;
    //}
    virtual AMF_RESULT SubmitInput(amf::AMFData* pData) override;
    virtual AMF_RESULT Terminate();
protected:
    void AddMessage(int log_level, const tstring& str) {
        if (m_pVCELog == nullptr || log_level < m_pVCELog->getLogLevel()) {
            return;
        }
        auto lines = split(str, _T("\n"));
        for (const auto& line : lines) {
            if (line[0] != _T('\0')) {
                (*m_pVCELog)(log_level, (m_strReaderName + _T(": ") + line + _T("\n")).c_str());
            }
        }
    }
    void AddMessage(int log_level, const TCHAR *format, ...) {
        if (m_pVCELog == nullptr || log_level < m_pVCELog->getLogLevel()) {
            return;
        }

        va_list args;
        va_start(args, format);
        int len = _vsctprintf(format, args) + 1; // _vscprintf doesn't count terminating '\0'
        tstring buffer;
        buffer.resize(len, _T('\0'));
        _vstprintf_s(&buffer[0], len, format, args);
        va_end(args);
        AddMessage(log_level, buffer);
    }
    uint32_t m_tmLastUpdate;
    VCEInputInfo m_inputInfo;
    shared_ptr<VCELog> m_pVCELog;
    shared_ptr<VCEStatus> m_pStatus;
    tstring m_message;
    tstring m_strReaderName;
    tstring m_strInputInfo;
    amf::AMFContextPtr m_pContext;
    const ConvertCSP *m_pConvertCsp;
};
