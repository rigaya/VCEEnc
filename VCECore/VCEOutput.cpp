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

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <tchar.h>

#include "VCEParam.h"
#include "VCEVersion.h"
#include "VCEOutput.h"
#include "VCELog.h"

VCEOutput::VCEOutput() :
    m_pPrintMes(), m_pEncSatusInfo(), m_strOutputInfo(), m_strWriterName(_T("raw writer")), m_bOutputIsStdout(false) {

}

VCEOutput::~VCEOutput() {
    close();
}

AMF_RESULT VCEOutput::Init(const tstring& dstFile, const void *pOption, shared_ptr<VCELog> pLog, shared_ptr<VCEStatus> pEncSatusInfo) {
    pOption = nullptr; //ここでは使用しない
    m_pPrintMes = pLog;
    m_pEncSatusInfo = pEncSatusInfo;
    if (dstFile == _T("-")) {
        AddMessage(VCE_LOG_ERROR, _T("pipe output not supported.\n"));
        return AMF_FILE_NOT_OPEN;
    }
    if (amf::AMFDataStream::OpenDataStream(tchar_to_wstring(dstFile).c_str(), amf::AMFSO_WRITE, amf::AMFFS_EXCLUSIVE, &m_pDataStream) != AMF_OK) {
        AddMessage(VCE_LOG_ERROR, _T("failed to open output file \"%s\".\n"), dstFile.c_str());
        return AMF_FILE_NOT_OPEN;
    }
    AddMessage(VCE_LOG_DEBUG, _T("Opend file \"%s\" as output.\n"), dstFile.c_str());
    return AMF_OK;
}

AMF_RESULT VCEOutput::close() {
    AddMessage(VCE_LOG_DEBUG, _T("Closing VCEOutput.\n"));
    m_pPrintMes.reset();
    m_pEncSatusInfo.reset();
    m_strOutputInfo.clear();
    return AMF_OK;
}

AMF_RESULT VCEOutput::SubmitInput(amf::AMFData* pData) {
    AMF_RESULT res = AMF_OK;
    if (pData) {
        amf::AMFBufferPtr pBuffer(pData);

        amf_size towrite = pBuffer->GetSize();
        amf_size written = 0;
        m_pDataStream->Write(pBuffer->GetNative(), towrite, &written);

        m_pEncSatusInfo->SetOutputData(written, 0x00);
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
