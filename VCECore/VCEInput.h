//  -----------------------------------------------------------------------------------------
//    VCEEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

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
