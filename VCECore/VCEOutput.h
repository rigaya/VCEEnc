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
#pragma warning(push)
#pragma warning(disable:4100)
#include "VideoEncoderVCE.h"

#include "DataStream.h"
#include "PipelineElement.h"

#include "VCEUtil.h"
#include "VCEParam.h"
#include "VCEStatus.h"
#pragma warning(pop)

struct AVOutputVideoPrm;

struct VCEOutRawParam {
    int  nBufSizeMB;
};

class VCEOutput : public PipelineElement {
public:
    VCEOutput();
    virtual ~VCEOutput();

    virtual AMF_RESULT Init(const tstring& dstFile, const void *pOption, shared_ptr<VCELog> pLog, shared_ptr<VCEStatus> pEncSatusInfo);
#pragma warning(push)
#pragma warning(disable: 4100)
    virtual AMF_RESULT SetVideoParam(AVOutputVideoPrm *pVidPrm, amf::AMFBufferPtr pExtradata) {
        return AMF_NOT_SUPPORTED;
    }
#pragma warning(pop)
    virtual AMF_RESULT close();
    virtual tstring GetOutputMessage() {
        return m_strOutputInfo;
    }

    virtual amf_int32 GetInputSlotCount() override {
        return 1;
    }
    virtual amf_int32 GetOutputSlotCount() override {
        return 0;
    }
    virtual AMF_RESULT SubmitInput(amf::AMFData* pData) override;
    virtual AMF_RESULT QueryOutput(amf::AMFData** ppData) override;
    virtual std::wstring GetDisplayResult() override;
    virtual void WaitFin() { }
    virtual bool outputStdout() {
        return m_bOutputIsStdout;
    }

protected:
    void AddMessage(int log_level, const tstring& str) {
        if (m_pPrintMes == nullptr || log_level < m_pPrintMes->getLogLevel()) {
            return;
        }
        auto lines = split(str, _T("\n"));
        for (const auto& line : lines) {
            if (line[0] != _T('\0')) {
                m_pPrintMes->write(log_level, (m_strWriterName + _T(": ") + line + _T("\n")).c_str());
            }
        }
    }
    void AddMessage(int log_level, const TCHAR *format, ...) {
        if (m_pPrintMes == nullptr || log_level < m_pPrintMes->getLogLevel()) {
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
    tstring m_strWriterName;
    amf::AMFDataStreamPtr m_pDataStream;
    shared_ptr<VCELog> m_pPrintMes;
    shared_ptr<VCEStatus> m_pEncSatusInfo;
    tstring m_strOutputInfo;
    bool m_bOutputIsStdout;
};
