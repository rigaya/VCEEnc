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

#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include "VCEInputVpy.h"
#include "VCEUtil.h"

#if ENABLE_VAPOURSYNTH_READER

VCEInputVpy::VCEInputVpy() : m_bAbortAsync(false), m_nCopyOfInputFrames(0),
    m_sVSapi(nullptr), m_sVSscript(nullptr), m_sVSnode(nullptr), m_nAsyncFrames(0) {
    memset(m_pAsyncBuffer, 0, sizeof(m_pAsyncBuffer));
    memset(m_hAsyncEventFrameSetFin, 0, sizeof(m_hAsyncEventFrameSetFin));
    memset(m_hAsyncEventFrameSetStart, 0, sizeof(m_hAsyncEventFrameSetStart));
    memset(&m_sVS, 0, sizeof(m_sVS));
    m_strReaderName = _T("vpy");
}

VCEInputVpy::~VCEInputVpy() {
    Terminate();
}

void VCEInputVpy::release_vapoursynth() {
    if (m_sVS.hVSScriptDLL)
        FreeLibrary(m_sVS.hVSScriptDLL);

    memset(&m_sVS, 0, sizeof(m_sVS));
}

int VCEInputVpy::load_vapoursynth() {
    release_vapoursynth();

    if (NULL == (m_sVS.hVSScriptDLL = LoadLibrary(_T("vsscript.dll")))) {
        AddMessage(VCE_LOG_ERROR, _T("Failed to load vsscript.dll.\n"));
        return 1;
    }

    std::map<void **, const char*> vs_func_list ={
        { (void **)&m_sVS.init,           (VPY_X64) ? "vsscript_init"           : "_vsscript_init@0" },
        { (void **)&m_sVS.finalize,       (VPY_X64) ? "vsscript_finalize"       : "_vsscript_finalize@0", },
        { (void **)&m_sVS.evaluateScript, (VPY_X64) ? "vsscript_evaluateScript" : "_vsscript_evaluateScript@16" },
        { (void **)&m_sVS.evaluateFile,   (VPY_X64) ? "vsscript_evaluateFile"   : "_vsscript_evaluateFile@12" },
        { (void **)&m_sVS.freeScript,     (VPY_X64) ? "vsscript_freeScript"     : "_vsscript_freeScript@4" },
        { (void **)&m_sVS.getError,       (VPY_X64) ? "vsscript_getError"       : "_vsscript_getError@4" },
        { (void **)&m_sVS.getOutput,      (VPY_X64) ? "vsscript_getOutput"      : "_vsscript_getOutput@8" },
        { (void **)&m_sVS.clearOutput,    (VPY_X64) ? "vsscript_clearOutput"    : "_vsscript_clearOutput@8" },
        { (void **)&m_sVS.getCore,        (VPY_X64) ? "vsscript_getCore"        : "_vsscript_getCore@4" },
        { (void **)&m_sVS.getVSApi,       (VPY_X64) ? "vsscript_getVSApi"       : "_vsscript_getVSApi@0" },
    };

    for (auto vs_func : vs_func_list) {
        if (NULL == (*(vs_func.first) = GetProcAddress(m_sVS.hVSScriptDLL, vs_func.second))) {
            AddMessage(VCE_LOG_ERROR, _T("Failed to load vsscript functions.\n"));
            return 1;
        }
    }
    return 0;
}

int VCEInputVpy::initAsyncEvents() {
    for (int i = 0; i < _countof(m_hAsyncEventFrameSetFin); i++) {
        if (   NULL == (m_hAsyncEventFrameSetFin[i]   = CreateEvent(NULL, FALSE, FALSE, NULL))
            || NULL == (m_hAsyncEventFrameSetStart[i] = CreateEvent(NULL, FALSE, TRUE, NULL)))
            return 1;
    }
    return 0;
}
void VCEInputVpy::closeAsyncEvents() {
    m_bAbortAsync = true;
    int frame_fin = (std::min)(m_nAsyncFrames, (int)m_nCopyOfInputFrames + ASYNC_BUFFER_SIZE);
    for (int i_frame = m_nCopyOfInputFrames; i_frame < frame_fin; i_frame++) {
        if (m_hAsyncEventFrameSetFin[i_frame & (ASYNC_BUFFER_SIZE-1)])
            WaitForSingleObject(m_hAsyncEventFrameSetFin[i_frame & (ASYNC_BUFFER_SIZE-1)], INFINITE);
    }
    for (int i = 0; i < _countof(m_hAsyncEventFrameSetFin); i++) {
        if (m_hAsyncEventFrameSetFin[i])
            CloseHandle(m_hAsyncEventFrameSetFin[i]);
        if (m_hAsyncEventFrameSetStart[i])
            CloseHandle(m_hAsyncEventFrameSetStart[i]);
    }
    memset(m_hAsyncEventFrameSetFin, 0, sizeof(m_hAsyncEventFrameSetFin));
    memset(m_hAsyncEventFrameSetStart, 0, sizeof(m_hAsyncEventFrameSetStart));
    m_bAbortAsync = false;
}

#pragma warning(push)
#pragma warning(disable:4100)
void __stdcall frameDoneCallback(void *userData, const VSFrameRef *f, int n, VSNodeRef *, const char *errorMsg) {
    reinterpret_cast<VCEInputVpy*>(userData)->setFrameToAsyncBuffer(n, f);
}
#pragma warning(pop)

void VCEInputVpy::setFrameToAsyncBuffer(int n, const VSFrameRef* f) {
    WaitForSingleObject(m_hAsyncEventFrameSetStart[n & (ASYNC_BUFFER_SIZE-1)], INFINITE);
    m_pAsyncBuffer[n & (ASYNC_BUFFER_SIZE-1)] = f;
    SetEvent(m_hAsyncEventFrameSetFin[n & (ASYNC_BUFFER_SIZE-1)]);

    if (m_nAsyncFrames < m_inputFrameInfo.frames && !m_bAbortAsync) {
        m_sVSapi->getFrameAsync(m_nAsyncFrames, m_sVSnode, frameDoneCallback, this);
        m_nAsyncFrames++;
    }
}

int VCEInputVpy::getRevInfo(const char *vsVersionString) {
    char *api_info = NULL;
    char buf[1024];
    strcpy_s(buf, _countof(buf), vsVersionString);
    for (char *p = buf, *q = NULL, *r = NULL; NULL != (q = strtok_s(p, "\n", &r)); ) {
        if (NULL != (api_info = strstr(q, "Core"))) {
            strcpy_s(buf, _countof(buf), api_info);
            for (char *s = buf; *s; s++)
                *s = (char)tolower(*s);
            int rev = 0;
            return (1 == sscanf_s(buf, "core r%d", &rev)) ? rev : 0;
        }
        p = NULL;
    }
    return 0;
}

AMF_RESULT VCEInputVpy::init(shared_ptr<VCELog> pLog, shared_ptr<VCEStatus> pStatus, VCEInputInfo *pInfo, amf::AMFContextPtr pContext) {
    m_pPrintMes = pLog;
    m_pEncSatusInfo = pStatus;
    m_pContext = pContext;
    
    m_inputFrameInfo = *pInfo;
    VCEInputVpyParam *pVpyParam = (VCEInputVpyParam *)pInfo->pPrivateParam;

    m_bVpyMT = pVpyParam->bVpyMt;

    if (load_vapoursynth()) {
        return AMF_FAIL;
    }

    //ファイルデータ読み込み
    std::ifstream inputFile(pVpyParam->srcFile);
    if (inputFile.bad()) {
        AddMessage(VCE_LOG_ERROR, _T("Failed to open vpy file \"%s\".\n"), pVpyParam->srcFile);
        return AMF_FAIL;
    }
    AddMessage(VCE_LOG_DEBUG, _T("Opened file \"%s\""), pVpyParam->srcFile);
    std::istreambuf_iterator<char> data_begin(inputFile);
    std::istreambuf_iterator<char> data_end;
    std::string script_data = std::string(data_begin, data_end);
    inputFile.close();

    const VSVideoInfo *vsvideoinfo = NULL;
    const VSCoreInfo *vscoreinfo = NULL;
    if (!m_sVS.init()
        || initAsyncEvents()
        || NULL == (m_sVSapi = m_sVS.getVSApi())
        || m_sVS.evaluateScript(&m_sVSscript, script_data.c_str(), NULL, efSetWorkingDir)
        || NULL == (m_sVSnode = m_sVS.getOutput(m_sVSscript, 0))
        || NULL == (vsvideoinfo = m_sVSapi->getVideoInfo(m_sVSnode))
        || NULL == (vscoreinfo = m_sVSapi->getCoreInfo(m_sVS.getCore(m_sVSscript)))) {
        AddMessage(VCE_LOG_ERROR, _T("VapourSynth Initialize Error.\n"));
        if (m_sVSscript) {
            AddMessage(VCE_LOG_ERROR, char_to_tstring(m_sVS.getError(m_sVSscript)).c_str());
        }
        return AMF_FAIL;
    }
    if (vscoreinfo->api < 3) {
        AddMessage(VCE_LOG_ERROR, _T("VapourSynth API v3 or later is necessary.\n"));
        return AMF_FAIL;
    }

    if (vsvideoinfo->height <= 0 || vsvideoinfo->width <= 0) {
        AddMessage(VCE_LOG_ERROR, _T("Variable resolution is not supported.\n"));
        return AMF_FAIL;
    }

    if (vsvideoinfo->numFrames == 0) {
        AddMessage(VCE_LOG_ERROR, _T("Length of input video is unknown.\n"));
        return AMF_FAIL;
    }

    if (!vsvideoinfo->format) {
        AddMessage(VCE_LOG_ERROR, _T("Variable colorformat is not supported.\n"));
        return AMF_FAIL;
    }

    typedef struct CSPMap {
        int fmtID;
        VCE_CSP in, out;
    } CSPMap;

    static const std::vector<CSPMap> valid_csp_list = {
        { pfYUV420P8,  VCE_CSP_YV12, VCE_CSP_NV12 },
    };

    for (auto csp : valid_csp_list) {
        if (csp.fmtID == vsvideoinfo->format->id) {
            m_pConvertCsp = get_convert_csp_func(csp.in, csp.out, false);
            break;
        }
    }

    if (nullptr == m_pConvertCsp) {
        AddMessage(VCE_LOG_ERROR, _T("invalid colorformat.\n"));
        return AMF_FAIL;
    }

    if (vsvideoinfo->fpsNum <= 0 || vsvideoinfo->fpsDen <= 0) {
        AddMessage(VCE_LOG_ERROR, _T("Invalid framerate %d/%d.\n"), vsvideoinfo->fpsNum, vsvideoinfo->fpsDen);
        return AMF_FAIL;
    }
    const auto fps_gcd = vce_gcd(vsvideoinfo->fpsNum, vsvideoinfo->fpsDen);

    m_inputFrameInfo.srcWidth = vsvideoinfo->width;
    m_inputFrameInfo.srcHeight = vsvideoinfo->height;
    m_inputFrameInfo.fps.num = (int)(vsvideoinfo->fpsNum / fps_gcd);
    m_inputFrameInfo.fps.den = (int)(vsvideoinfo->fpsDen / fps_gcd);
    m_inputFrameInfo.frames = vsvideoinfo->numFrames;
    m_nAsyncFrames = vsvideoinfo->numFrames;
    m_nAsyncFrames = (std::min)(m_nAsyncFrames, vscoreinfo->numThreads);
    m_nAsyncFrames = (std::min)(m_nAsyncFrames, ASYNC_BUFFER_SIZE-1);
    if (!m_bVpyMT)
        m_nAsyncFrames = 1;

    AddMessage(VCE_LOG_DEBUG, _T("VpyReader using %d async frames.\n"), m_nAsyncFrames);
    for (int i = 0; i < m_nAsyncFrames; i++)
        m_sVSapi->getFrameAsync(i, m_sVSnode, frameDoneCallback, this);

    tstring rev_info = _T("");
    tstring intputBitdepthStr = _T("");
    int rev = getRevInfo(vscoreinfo->versionString);
    if (0 != rev) {
        rev_info = strsprintf(_T(" r%d"), rev);
    }

    tstring mes = strsprintf(_T("VapourSynth%s%s %s->%s[%s], %dx%d%s, %d/%d fps"),
        (m_bVpyMT) ? _T("MT") : _T(""), rev_info.c_str(),
        VCE_CSP_NAMES[m_pConvertCsp->csp_from], VCE_CSP_NAMES[m_pConvertCsp->csp_to], get_simd_str(m_pConvertCsp->simd),
        m_inputFrameInfo.srcWidth, m_inputFrameInfo.srcHeight, is_interlaced(m_inputFrameInfo.nPicStruct) ? _T("i") : _T("p"), m_inputFrameInfo.fps.num, m_inputFrameInfo.fps.den);

    AddMessage(VCE_LOG_DEBUG, _T("%s\n"), mes.c_str());
    m_strInputInfo += mes;
    return AMF_OK;
}

AMF_RESULT VCEInputVpy::Terminate() {
    AddMessage(VCE_LOG_DEBUG, _T("Closing...\n"));
    closeAsyncEvents();
    if (m_sVSapi && m_sVSnode)
        m_sVSapi->freeNode(m_sVSnode);
    if (m_sVSscript)
        m_sVS.freeScript(m_sVSscript);
    if (m_sVSapi)
        m_sVS.finalize();

    release_vapoursynth();

    m_bAbortAsync = false;
    m_nCopyOfInputFrames = 0;

    m_sVSapi = nullptr;
    m_sVSscript = nullptr;
    m_sVSnode = nullptr;
    m_nAsyncFrames = 0;

    m_pPrintMes.reset();
    m_pEncSatusInfo.reset();
    m_message.clear();
    m_pContext = nullptr;
    return AMF_OK;
}

AMF_RESULT VCEInputVpy::QueryOutput(amf::AMFData** ppData) {
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

    if (m_pEncSatusInfo->m_nInputFrames >= (uint32_t)m_inputFrameInfo.frames)
        return AMF_EOF;

    const VSFrameRef *src_frame = getFrameFromAsyncBuffer(m_pEncSatusInfo->m_nInputFrames);
    if (nullptr == src_frame) {
        return AMF_EOF;
    }
    const void *src_ptr[3] = { m_sVSapi->getReadPtr(src_frame, 0), m_sVSapi->getReadPtr(src_frame, 1), m_sVSapi->getReadPtr(src_frame, 2) };
    auto plane = pSurface->GetPlaneAt(0);
    int dst_stride = plane->GetHPitch();
    int dst_height = plane->GetVPitch();

    void *dst_ptr[2];
    dst_ptr[0] = (uint8_t *)plane->GetNative();
    dst_ptr[1] = (uint8_t *)dst_ptr[0] + dst_height * dst_stride;
    m_pConvertCsp->func[is_interlaced(m_inputFrameInfo.nPicStruct) ? 1 : 0](dst_ptr, src_ptr, m_inputFrameInfo.srcWidth, m_sVSapi->getStride(src_frame, 0), m_sVSapi->getStride(src_frame, 1), dst_stride, m_inputFrameInfo.srcHeight, dst_height, m_inputFrameInfo.crop.c);
    m_pEncSatusInfo->m_nInputFrames++;
    m_nCopyOfInputFrames = m_pEncSatusInfo->m_nInputFrames;

    m_sVSapi->freeFrame(src_frame);
    m_pEncSatusInfo->UpdateDisplay(0);

    *ppData = pSurface.Detach();
    return AMF_OK;
}

#endif //ENABLE_VAPOURSYNTH_READER
