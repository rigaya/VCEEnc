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

#include <io.h>
#include <fcntl.h>
#include "VCEInputRaw.h"
#include "VCEUtil.h"

VCEInputRaw::VCEInputRaw() : m_pBuffer(), m_bIsY4m(false), m_fp(NULL) {
    m_strReaderName = _T("raw");
}

VCEInputRaw::~VCEInputRaw() {
    Terminate();
}

AMF_RESULT VCEInputRaw::init(shared_ptr<VCELog> pLog, shared_ptr<VCEStatus> pStatus, VCEInputInfo *pInfo, amf::AMFContextPtr pContext) {
    m_pPrintMes = pLog;
    m_pEncSatusInfo = pStatus;
    m_pContext = pContext;
    
    m_inputFrameInfo = *pInfo;
    VCEInputRawParam *pRawParam = (VCEInputRawParam *)pInfo->pPrivateParam;
    m_bIsY4m = pRawParam->y4m;
    if (m_bIsY4m) {
        m_strReaderName = _T("y4m");
    }

    if (0 == _tcscmp(pRawParam->srcFile, _T("-"))) {
        if (_setmode(_fileno( stdin ), _O_BINARY) == 1) {
            AddMessage(VCE_LOG_ERROR, _T("failed to switch stdint to binary mode.\n"));
            return AMF_UNEXPECTED;
        }
        m_fp = stdin;
        AddMessage(VCE_LOG_DEBUG, _T("Opened stdin as input.\n"));
    } else {
        if (_tfopen_s(&m_fp, pRawParam->srcFile, _T("rb")) || NULL == m_fp) {
            AddMessage(VCE_LOG_ERROR, _T("Failed to open input file \"%s\".\n"), pRawParam->srcFile);
            return AMF_FILE_NOT_OPEN;
        }
        AddMessage(VCE_LOG_DEBUG, _T("Opened \"%s\" as input.\n"), pRawParam->srcFile);
    }

    if (m_bIsY4m) {
        char buf[128] = { 0 };
        if (fread(buf, 1, strlen("YUV4MPEG2"), m_fp) != strlen("YUV4MPEG2")
            || strcmp(buf, "YUV4MPEG2") != 0
            || !fgets(buf, sizeof(buf), m_fp)
            || ParseY4MHeader(buf, &m_inputFrameInfo)) {
            return AMF_UNEXPECTED;
        }
    }

    //読み込みバッファの確保
    m_pBuffer.reset((uint8_t *)_aligned_malloc(m_inputFrameInfo.srcWidth * m_inputFrameInfo.srcHeight * 3 / 2, 32));
    if (m_pBuffer.get() == nullptr) {
        AddMessage(VCE_LOG_ERROR, _T("Failed to allocate memory for input.\n"));
        return AMF_OUT_OF_MEMORY;
    }

    if (nullptr == (m_pConvertCsp = get_convert_csp_func(VCE_CSP_YV12, VCE_CSP_NV12, false))) {
        AddMessage(VCE_LOG_ERROR, _T("Failed to find converter for yv12 -> nv12.\n"));
        return AMF_NOT_SUPPORTED;
    }

    if (m_inputFrameInfo.fps.num > 0 && m_inputFrameInfo.fps.den > 0) {
        int fps_gcd = vce_gcd(m_inputFrameInfo.fps.num, m_inputFrameInfo.fps.den);
        m_inputFrameInfo.fps.num /= fps_gcd;
        m_inputFrameInfo.fps.den /= fps_gcd;
    }

    tstring mes;
    if (m_bIsY4m) {
        mes = strsprintf(_T("y4m: %s->%s[%s], %dx%d%s, %d/%d fps"),
            VCE_CSP_NAMES[m_pConvertCsp->csp_from], VCE_CSP_NAMES[m_pConvertCsp->csp_to], get_simd_str(m_pConvertCsp->simd),
            m_inputFrameInfo.srcWidth, m_inputFrameInfo.srcHeight, is_interlaced(m_inputFrameInfo.nPicStruct) ? _T("i") : _T("p"), m_inputFrameInfo.fps.num, m_inputFrameInfo.fps.den);
    } else {
        mes = _T("raw");
    }
    AddMessage(VCE_LOG_DEBUG, _T("%s\n"), mes.c_str());
    m_strInputInfo += mes;
    return AMF_OK;
}

AMF_RESULT VCEInputRaw::Terminate() {
    m_pPrintMes.reset();
    m_pEncSatusInfo.reset();
    m_message.clear();
    m_pContext = nullptr;
    m_pBuffer.reset();
    if (m_fp) {
        fclose(m_fp);
    }
    return AMF_OK;
}

AMF_RESULT VCEInputRaw::QueryOutput(amf::AMFData** ppData) {
    AMF_RESULT res = AMF_OK;
    amf::AMFSurfacePtr pSurface;
    res = m_pContext->AllocSurface(amf::AMF_MEMORY_HOST, m_inputFrameInfo.format,
        m_inputFrameInfo.srcWidth - m_inputFrameInfo.crop.left - m_inputFrameInfo.crop.right,
        m_inputFrameInfo.srcHeight - m_inputFrameInfo.crop.bottom - m_inputFrameInfo.crop.up,
        &pSurface);
    if (res != AMF_OK) {
        AddMessage(VCE_LOG_ERROR, _T("AMFContext::AllocSurface(amf::AMF_MEMORY_HOST) failed.\n"));
        return res;
    }

    if (m_bIsY4m) {
        BYTE y4m_buf[8] = { 0 };
        if (fread(y4m_buf, 1, strlen("FRAME"), m_fp) != strlen("FRAME"))
            return AMF_EOF;
        if (memcmp(y4m_buf, "FRAME", strlen("FRAME")) != NULL)
            return AMF_EOF;
        int i;
        for (i = 0; fgetc(m_fp) != '\n'; i++)
            if (i >= 64)
                return AMF_EOF;
    }
    
    size_t frameSize = m_inputFrameInfo.srcWidth * m_inputFrameInfo.srcHeight * 3 / 2;
    if (frameSize != fread(m_pBuffer.get(), 1, frameSize, m_fp)) {
        return AMF_EOF;
    }

    const void *src_ptr[3];
    src_ptr[0] = m_pBuffer.get();
    src_ptr[1] = m_pBuffer.get() + m_inputFrameInfo.srcWidth * m_inputFrameInfo.srcHeight;
    src_ptr[2] = m_pBuffer.get() + m_inputFrameInfo.srcWidth * m_inputFrameInfo.srcHeight * 5 / 4;

    auto plane = pSurface->GetPlaneAt(0);
    int dst_stride = plane->GetHPitch();
    int dst_height = plane->GetVPitch();

    void *dst_ptr[2];
    dst_ptr[0] = (uint8_t *)plane->GetNative();
    dst_ptr[1] = (uint8_t *)dst_ptr[0] + dst_height * dst_stride;
    m_pConvertCsp->func[is_interlaced(m_inputFrameInfo.nPicStruct) ? 1 : 0](dst_ptr, src_ptr, m_inputFrameInfo.srcWidth, m_inputFrameInfo.srcWidth, m_inputFrameInfo.srcWidth / 2, dst_stride, m_inputFrameInfo.srcHeight, dst_height, m_inputFrameInfo.crop.c);
    m_pEncSatusInfo->m_nInputFrames++;
    m_pEncSatusInfo->UpdateDisplay(0);

    *ppData = pSurface.Detach();
    return AMF_OK;
}

int VCEInputRaw::ParseY4MHeader(char *buf, VCEInputInfo *inputInfo) {
    char *p, *q = NULL;
    for (p = buf; (p = strtok_s(p, " ", &q)) != NULL; ) {
        switch (*p) {
        case 'W':
        {
            char *eptr = NULL;
            int w = strtol(p+1, &eptr, 10);
            if (*eptr == '\0' && w)
                inputInfo->srcWidth = w;
        }
        break;
        case 'H':
        {
            char *eptr = NULL;
            int h = strtol(p+1, &eptr, 10);
            if (*eptr == '\0' && h)
                inputInfo->srcHeight = h;
        }
        break;
        case 'F':
        {
            int rate = 0, scale = 0;
            if (   (inputInfo->fps.den == 0 || inputInfo->fps.num == 0)
                && sscanf_s(p+1, "%d:%d", &rate, &scale) == 2) {
                if (rate && scale) {
                    inputInfo->fps.num = rate;
                    inputInfo->fps.den = scale;
                }
            }
        }
        break;
        case 'A':
        {
            int sar_x = 0, sar_y = 0;
            if (   (inputInfo->AspectRatioW == 0 || inputInfo->AspectRatioH == 0)
                && sscanf_s(p+1, "%d:%d", &sar_x, &sar_y) == 2) {
                    if (sar_x && sar_y) {
                        inputInfo->AspectRatioW = sar_x;
                        inputInfo->AspectRatioH = sar_y;
                    }
            }
        }
        break;
        //case 'I':
        //    switch (*(p+1)) {
        //case 'b':
        //    info->PicStruct = MFX_PICSTRUCT_FIELD_BFF;
        //    break;
        //case 't':
        //case 'm':
        //    info->PicStruct = MFX_PICSTRUCT_FIELD_TFF;
        //    break;
        //default:
        //    break;
        //    }
        //    break;
        case 'C':
            if (   0 != _strnicmp(p+1, "420",      strlen("420"))
                && 0 != _strnicmp(p+1, "420mpeg2", strlen("420mpeg2"))
                && 0 != _strnicmp(p+1, "420jpeg",  strlen("420jpeg"))
                && 0 != _strnicmp(p+1, "420paldv", strlen("420paldv"))) {
                return 1;
            }
            break;
        default:
            break;
        }
        p = NULL;
    }
    if (inputInfo->fps.num > 0 && inputInfo->fps.den > 0) {
        int fps_gcd = vce_gcd(inputInfo->fps.num, inputInfo->fps.den);
        inputInfo->fps.num /= fps_gcd;
        inputInfo->fps.den /= fps_gcd;
    }

    return 0;
}
