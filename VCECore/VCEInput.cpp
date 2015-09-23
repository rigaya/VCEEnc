//  -----------------------------------------------------------------------------------------
//    VCEEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#include <tchar.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include "VCEParam.h"
#include "VCEVersion.h"
#include "VCEInput.h"
#include "VCELog.h"

VCEInput::VCEInput() :
    m_pVCELog(), m_pStatus(), m_message(), m_strReaderName(_T("general reader")),
    m_strInputInfo(), m_pContext(nullptr), m_pConvertCsp(nullptr), m_tmLastUpdate(timeGetTime()) {

}

VCEInput::~VCEInput() {
    Terminate();
}

AMF_RESULT VCEInput::Terminate() {
    AddMessage(VCE_LOG_DEBUG, _T("Closing VCEInput.\n"));
    m_pVCELog.reset();
    m_pStatus.reset();
    m_message.clear();
    m_strInputInfo.clear();
    m_pContext = nullptr;
    return AMF_OK;
}

#pragma warning(push)
#pragma warning(disable: 4100)
AMF_RESULT VCEInput::SubmitInput(amf::AMFData* pData) {
    return AMF_NOT_SUPPORTED;
}
#pragma warning(pop)
