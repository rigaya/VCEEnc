//  -----------------------------------------------------------------------------------------
//    VCEEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#pragma once

#include "VCEInput.h"

struct VCEInputRawParam {
    const TCHAR *srcFile;
    bool y4m;
};

class VCEInputRaw : public VCEInput {
public:
    VCEInputRaw();
    virtual ~VCEInputRaw();

    virtual AMF_RESULT init(shared_ptr<VCELog> pLog, shared_ptr<VCEStatus> pStatus, VCEInputInfo *pInfo, amf::AMFContextPtr pContext);
    virtual AMF_RESULT QueryOutput(amf::AMFData ** ppData) override;
    virtual AMF_RESULT Terminate() override;
private:
    int VCEInputRaw::ParseY4MHeader(char *buf, VCEInputInfo *inputInfo);
    unique_ptr<uint8_t, aligned_malloc_deleter> m_pBuffer;
    FILE *m_fp;
    bool m_bIsY4m;
};
