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

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <tchar.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include "VCEParam.h"
#include "VCEVersion.h"
#include "VCEInput.h"
#include "VCELog.h"

VCEInput::VCEInput() :
    m_nInputCodec(VCE_CODEC_NONE),
    m_sInputCrop({ 0 }),
    m_inputFrameInfo({ 0 }),
    m_pPrintMes(),
    m_pEncSatusInfo(),
    m_message(),
    m_strReaderName(_T("general reader")),
    m_strInputInfo(),
    m_pContext(nullptr),
    m_pConvertCsp(nullptr),
    m_sTrimParam() {

}

VCEInput::~VCEInput() {
    Terminate();
}

AMF_RESULT VCEInput::Terminate() {
    AddMessage(VCE_LOG_DEBUG, _T("Closing VCEInput.\n"));
    memset(&m_sInputCrop, 0, sizeof(m_sInputCrop));
    m_pPrintMes.reset();
    m_pEncSatusInfo.reset();
    m_message.clear();
    m_strInputInfo.clear();
    m_pContext = nullptr;
    m_pConvertCsp = nullptr;
    memset(&m_sTrimParam, 0, sizeof(m_sTrimParam));
    return AMF_OK;
}

#pragma warning(push)
#pragma warning(disable: 4100)
AMF_RESULT VCEInput::SubmitInput(amf::AMFData* pData) {
    return AMF_NOT_SUPPORTED;
}
#pragma warning(pop)
