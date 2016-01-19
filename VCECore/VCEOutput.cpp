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
