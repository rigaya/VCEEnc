//  -----------------------------------------------------------------------------------------
//    VCEEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

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
    virtual void operator()(int log_level, const TCHAR *format, ...) override;
private:
    vector<TCHAR> m_printBuf;
};

class AuoStatus : public VCEStatus {
public:
    AuoStatus();
    virtual ~AuoStatus();
protected:
    virtual void UpdateDisplay(const char *mes, int drop_frames) override;
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
    virtual AMF_RESULT initInput(VCEParam *prm) override;
};

#endif //_AUO_VCE_H_