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

#include "VCEUtil.h"
#include "VCEParam.h"

class VCELog {
protected:
    int m_nLogLevel = VCE_LOG_INFO;
    const TCHAR *m_pStrLog = nullptr;
    bool m_bHtml = false;
    CRITICAL_SECTION cs;
    static const char *HTML_FOOTER;
public:
    VCELog(const TCHAR *pLogFile, int log_level = VCE_LOG_INFO) {
        InitializeCriticalSection(&cs);
        init(pLogFile, log_level);
    };
    virtual ~VCELog() {
        DeleteCriticalSection(&cs);
    };
    void init(const TCHAR *pLogFile, int log_level = VCE_LOG_INFO);
    void writeFileHeader(const TCHAR *pDstFilename);
    void writeHtmlHeader();
    void writeFileFooter();
    int getLogLevel() {
        return m_nLogLevel;
    }
    virtual void operator()(int log_level, const TCHAR *format, ...);
};
