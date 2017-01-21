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

#ifndef _AUO_VCE_H_
#define _AUO_VCE_H_

#include <Windows.h>
#include <stdio.h>
#include "output.h"
#include "auo.h"
#include "auo_conf.h"
#include "auo_system.h"

#include "VCEUtil.h"
#include "VCEInput.h"
#include "VCECore.h"

struct VCEInputAuoParam {
    const OUTPUT_INFO *oip;
    const SYSTEM_DATA *sys_dat;
    CONF_GUIEX *conf;
    PRM_ENC *pe;
    int *jitter;
};

class AuoLog : public VCELog {
public:
    AuoLog(const TCHAR *pLogFile, int log_level) : VCELog(pLogFile, log_level), m_printBuf() { };
    virtual ~AuoLog();

    virtual void write_log(int log_level, const TCHAR *buffer, bool file_only = false) override;
    virtual void write(int log_level, const TCHAR *format, ...) override;
private:
    vector<TCHAR> m_printBuf;
};

class AuoStatus : public VCEStatus {
public:
    AuoStatus();
    virtual ~AuoStatus();
protected:
    virtual void UpdateDisplay(const char *mes, int drop_frames, double progressPercent) override;
    virtual AMF_RESULT UpdateDisplay(int drop_frames, double progressPercent = 0.0) override;
    virtual void WriteLine(const TCHAR *mes) override;

    vector<char> m_lineBuf;
};

class VCEInputAuo : public VCEInput {
public:
    VCEInputAuo();
    virtual ~VCEInputAuo();

    virtual AMF_RESULT init(shared_ptr<VCELog> pLog, shared_ptr<VCEStatus> pStatus, VCEInputInfo *pInfo, amf::AMFContextPtr pContext) override;

    virtual AMF_RESULT SubmitInput(amf::AMFData* pData) override;
    virtual AMF_RESULT QueryOutput(amf::AMFData** ppData) override;
    virtual AMF_RESULT Terminate() override;
private:
    VCEInputAuoParam m_param;
};

class VCECoreAuo : public VCECore {
public:
    VCECoreAuo();
    virtual ~VCECoreAuo();
    virtual AMF_RESULT initInput(VCEParam *pParams, const VCEInputInfo *pInputInfo) override;
};

#endif //_AUO_VCE_H_