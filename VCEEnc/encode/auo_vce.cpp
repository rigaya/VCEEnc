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
#include <Process.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib") 
#include <stdlib.h>
#include <stdio.h>

#include "output.h"
#include "vphelp_client.h"

#pragma warning( push )
#pragma warning( disable: 4127 )
#include "afs_client.h"
#pragma warning( pop )

#include "convert.h"

#include "auo.h"
#include "auo_frm.h"
#include "auo_pipe.h"
#include "auo_error.h"
#include "auo_conf.h"
#include "auo_util.h"
#include "auo_system.h"
#include "auo_version.h"

#include "auo_encode.h"
#include "auo_video.h"
#include "auo_audio_parallel.h"

#include "auo_vce.h"

AUO_RESULT aud_parallel_task(const OUTPUT_INFO *oip, PRM_ENC *pe);

static int calc_input_frame_size(int width, int height, int color_format) {
    width = (color_format == CF_RGB) ? (width+3) & ~3 : (width+1) & ~1;
    return width * height * COLORFORMATS[color_format].size;
}

BOOL setup_afsvideo(const OUTPUT_INFO *oip, const SYSTEM_DATA *sys_dat, CONF_GUIEX *conf, PRM_ENC *pe) {
    //すでに初期化してある または 必要ない
    if (pe->afs_init || pe->video_out_type == VIDEO_OUTPUT_DISABLED || !conf->vid.afs)
        return TRUE;

    const int color_format = CF_YUY2;
    const int frame_size = calc_input_frame_size(oip->w, oip->h, color_format);
    //Aviutl(自動フィールドシフト)からの映像入力
    if (afs_vbuf_setup((OUTPUT_INFO *)oip, conf->vid.afs, frame_size, COLORFORMATS[color_format].FOURCC)) {
        pe->afs_init = TRUE;
        return TRUE;
    } else if (conf->vid.afs && sys_dat->exstg->s_local.auto_afs_disable) {
        afs_vbuf_release(); //一度解放
        warning_auto_afs_disable();
        conf->vid.afs = FALSE;
        //再度使用するmuxerをチェックする
        pe->muxer_to_be_used = check_muxer_to_be_used(conf, sys_dat, pe->temp_filename, pe->video_out_type, (oip->flag & OUTPUT_INFO_FLAG_AUDIO) != 0);
        return TRUE;
    }
    //エラー
    error_afs_setup(conf->vid.afs, sys_dat->exstg->s_local.auto_afs_disable);
    return FALSE;
}

void close_afsvideo(PRM_ENC *pe) {
    if (!pe->afs_init || pe->video_out_type == VIDEO_OUTPUT_DISABLED)
        return;

    afs_vbuf_release();

    pe->afs_init = FALSE;
}

AuoLog::~AuoLog() {
    m_printBuf.clear();
}

#pragma warning(push)
#pragma warning(disable:4100)
void AuoLog::write_log(int log_level, const TCHAR *mes, bool file_only) {
    int len = _tcslen(mes) + 1;
    std::vector<TCHAR> buffer(len * 2 + 64, 0);
    memcpy(buffer.data(), mes, sizeof(buffer[0]) * len);
    TCHAR *buffer_line = buffer.data() + len;
    TCHAR *q = NULL;
    for (TCHAR *p = buffer.data(); (p = _tcstok_s(p, _T("\n"), &q)) != NULL; ) {
        static const TCHAR *const LOG_STRING[] ={ _T("trace"),  _T("debug"), _T("info"), _T("info"), _T("warn"), _T("error") };
        _stprintf_s(buffer_line, len + 64, "qsv [%s]: %s", LOG_STRING[clamp(log_level, VCE_LOG_TRACE, VCE_LOG_ERROR) - VCE_LOG_TRACE], p);
        write_log_line(log_level, buffer_line);
        p = NULL;
    }
}
#pragma warning(pop)

void AuoLog::write(int log_level, const TCHAR *format, ...) {
    if (log_level < m_nLogLevel) {
        return;
    }

    va_list args;
    va_start(args, format);

    size_t len = _vsctprintf(format, args) + 1; // _vscprintf doesn't count terminating '\0'
    if (m_printBuf.size() < len * 2 + 64) {
        m_printBuf.resize(len * 2 + 256, _T('\0'));
    }

    _vstprintf_s(m_printBuf.data(), len, format, args);
    write_log(log_level, m_printBuf.data());
}

AuoStatus::AuoStatus() : m_lineBuf() {

}

AuoStatus::~AuoStatus() {
    m_lineBuf.clear();
}

void AuoStatus::UpdateDisplay(const char *mes, int drop_frames, double progressPercent) {
    set_log_title_and_progress(mes, progressPercent * 0.01);
}

AMF_RESULT AuoStatus::UpdateDisplay(int drop_frames, double progressPercent) {
    return VCEStatus::UpdateDisplay(drop_frames, progressPercent);
}

void AuoStatus::WriteLine(const TCHAR *mes) {
    const char *HEADER = "vce [info]: ";
    size_t buf_len = strlen(mes) + 1 + strlen(HEADER);
    if (m_lineBuf.size() < buf_len) {
        m_lineBuf.resize(buf_len * 2, 0);
    }
    memcpy(m_lineBuf.data(), HEADER, strlen(HEADER));
    memcpy(m_lineBuf.data() + strlen(HEADER), mes, strlen(mes) + 1);
    write_log_line(LOG_INFO, m_lineBuf.data());
}

VCEInputAuo::VCEInputAuo() : m_param() {
    m_strReaderName = _T("auo");
    memset(&m_param, 0, sizeof(m_param));
}

VCEInputAuo::~VCEInputAuo() {

}


AMF_RESULT VCEInputAuo::init(shared_ptr<VCELog> pLog, shared_ptr<VCEStatus> pStatus, VCEInputInfo *pInfo, amf::AMFContextPtr pContext) {
    m_pPrintMes = pLog;
    m_pEncSatusInfo = pStatus;
    m_pContext = pContext;

    m_inputFrameInfo = *pInfo;
    m_param = *(VCEInputAuoParam *)m_inputFrameInfo.pPrivateParam;

    setup_afsvideo(m_param.oip, m_param.sys_dat, m_param.conf, m_param.pe);

    if (nullptr == (m_pConvertCsp = get_convert_csp_func(VCE_CSP_YUY2, VCE_CSP_NV12, false))) {
        AddMessage(VCE_LOG_ERROR, _T("Failed to find converter for yuy2 -> nv12.\n"));
        return AMF_FAIL;
    }
    tstring mes = strsprintf(_T("auo: %s->%s[%s], %dx%d%s, %d/%d fps"),
        VCE_CSP_NAMES[m_pConvertCsp->csp_from], VCE_CSP_NAMES[m_pConvertCsp->csp_to], get_simd_str(m_pConvertCsp->simd),
        m_inputFrameInfo.srcWidth, m_inputFrameInfo.srcHeight, is_interlaced(m_inputFrameInfo.nPicStruct) ? _T("i") : _T("p"), m_inputFrameInfo.fps.num, m_inputFrameInfo.fps.den);
    AddMessage(VCE_LOG_DEBUG, _T("%s\n"), mes.c_str());
    m_strInputInfo += mes;
    return AMF_OK;
}

#pragma warning(push)
#pragma warning(disable: 4100)
AMF_RESULT VCEInputAuo::SubmitInput(amf::AMFData* pData) {
    return AMF_NOT_SUPPORTED;
}
#pragma warning(pop)

AMF_RESULT VCEInputAuo::QueryOutput(amf::AMFData** ppData) {
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

    int i_frame = m_pEncSatusInfo->m_nInputFrames + m_param.pe->drop_count;

    if (i_frame >= m_param.oip->n || m_param.pe->aud_parallel.abort) {
        //m_param.oip->func_rest_time_disp(i_frame, m_param.oip->n);
        release_audio_parallel_events(m_param.pe);
        return AMF_EOF;
    }

    const void *frame = nullptr;
    if (m_param.conf->vid.afs) {
        BOOL drop = FALSE;
        for ( ; ; ) {
            if ((frame = afs_get_video((OUTPUT_INFO *)m_param.oip, i_frame, &drop, &m_param.jitter[i_frame + 1])) == nullptr) {
                error_afs_get_frame();
                return AMF_FAIL;
            }
            if (!drop)
                break;
            m_param.jitter[i_frame] = DROP_FRAME_FLAG;
            m_param.pe->drop_count++;
            i_frame++;
            if (i_frame >= m_param.oip->n || m_param.pe->aud_parallel.abort) {
                //m_param.oip->func_rest_time_disp(i_frame, m_param.oip->n);
                release_audio_parallel_events(m_param.pe);
                return AMF_EOF;
            }
        }
    } else {
        if ((frame = m_param.oip->func_get_video_ex(i_frame, COLORFORMATS[CF_YUY2].FOURCC)) == nullptr) {
            error_afs_get_frame();
            return AMF_FAIL;
        }
    }

    auto plane = pSurface->GetPlaneAt(0);
    int dst_stride = plane->GetHPitch();
    int dst_height = plane->GetVPitch();

    void *dst_ptr[2];
    dst_ptr[0] = (uint8_t *)plane->GetNative();
    dst_ptr[1] = (uint8_t *)dst_ptr[0] + dst_height * dst_stride;
    int crop[4] = { 0 };
    m_pConvertCsp->func[is_interlaced(m_inputFrameInfo.nPicStruct) ? 1 : 0](dst_ptr, &frame, m_inputFrameInfo.srcWidth, m_inputFrameInfo.srcWidth * 2, 0, dst_stride, m_inputFrameInfo.srcHeight, dst_height, crop);

    m_pEncSatusInfo->m_nInputFrames++;
    if (!(m_pEncSatusInfo->m_nInputFrames & 7))
        aud_parallel_task(m_param.oip, m_param.pe);

    m_pEncSatusInfo->UpdateDisplay(m_param.pe->drop_count);
    *ppData = pSurface.Detach();
    return AMF_OK;
}

AMF_RESULT VCEInputAuo::Terminate() {
    if (m_param.pe) {
        close_afsvideo(m_param.pe);
    }
    m_pPrintMes.reset();
    m_pEncSatusInfo.reset();
    m_message.clear();
    m_pContext = nullptr;
    return AMF_OK;
}




VCECoreAuo::VCECoreAuo() {

}

VCECoreAuo::~VCECoreAuo() {

}

AMF_RESULT VCECoreAuo::initInput(VCEParam *pParams, const VCEInputInfo *pInputInfo) {
    m_pVCELog.reset(new AuoLog(pParams->pStrLogFile, pParams->nLogLevel));
    m_pEncSatusInfo.reset(new AuoStatus());
    m_pFileReader.reset(new VCEInputAuo());

    auto ret = m_pFileReader->init(m_pVCELog, m_pEncSatusInfo, &m_inputInfo, m_pContext);
    if (ret != AMF_OK) {
        PrintMes(VCE_LOG_ERROR, _T("Error: %s\n"), m_pFileReader->getMessage().c_str());
        return ret;
    }
    m_inputInfo = m_pFileReader->GetInputFrameInfo();
    m_pEncSatusInfo->init(m_pVCELog, m_inputInfo.fps, m_inputInfo.frames);
    return ret;
}
