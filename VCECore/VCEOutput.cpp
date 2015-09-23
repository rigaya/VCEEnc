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

#include "VCEParam.h"
#include "VCEVersion.h"
#include "VCEOutput.h"
#include "VCELog.h"

VCEOutput::VCEOutput() :
    m_pVCELog(), m_pStatus(), m_message(), m_strWriterName(_T("raw writer")) {

}

VCEOutput::~VCEOutput() {
    close();
}

AMF_RESULT VCEOutput::init(const tstring& dstFile, shared_ptr<VCELog> pLog, shared_ptr<VCEStatus> pStatus) {
    m_pVCELog = pLog;
    m_pStatus = pStatus;
    if (dstFile == _T("-")) {
        AddMessage(VCE_LOG_ERROR, _T("pipe output not supported.\n"));
        return AMF_FILE_NOT_OPEN;
    }
    m_pDataStream = AMFDataStream::Create(tchar_to_wstring(dstFile).c_str(), AMF_FileWrite);
    if (m_pDataStream.get() == nullptr) {
        AddMessage(VCE_LOG_ERROR, _T("failed to open output file \"%s\".\n"), dstFile.c_str());
        return AMF_FILE_NOT_OPEN;
    }
    AddMessage(VCE_LOG_DEBUG, _T("Opend file \"%s\" as output.\n"), dstFile.c_str());
    return AMF_OK;
}

AMF_RESULT VCEOutput::close() {
    AddMessage(VCE_LOG_DEBUG, _T("Closing VCEOutput.\n"));
    m_pVCELog.reset();
    m_pStatus.reset();
    m_message.clear();
    return AMF_OK;
}

AMF_RESULT VCEOutput::SubmitInput(amf::AMFData* pData) {
    AMF_RESULT res = AMF_OK;
    if (pData) {
        amf::AMFBufferPtr pBuffer(pData);

        amf_size towrite = pBuffer->GetSize();
        amf_size written = m_pDataStream->Write(pBuffer->GetNative(), towrite);

        m_pStatus->setOutputData(written, 0x00);
    } else {
        res = AMF_EOF;
    }
    return res;
}

#pragma warning(push)
#pragma warning(disable: 4100)
AMF_RESULT VCEOutput::QueryOutput(amf::AMFData** ppData) {
    return AMF_NOT_SUPPORTED;
}
#pragma warning(pop)

std::wstring VCEOutput::GetDisplayResult() {
    return std::wstring(L"");
}
