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
