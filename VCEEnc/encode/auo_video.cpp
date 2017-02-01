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

#include <Windows.h>
#pragma comment(lib, "user32.lib") //WaitforInputIdle
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <vector>

#include "output.h"
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

#include "VCECore.h"
#include "auo_vce.h"

DWORD tcfile_out(int *jitter, int frame_n, double fps, BOOL afs, const PRM_ENC *pe) {
    DWORD ret = AUO_RESULT_SUCCESS;
    char auotcfile[MAX_PATH_LEN];
    FILE *tcfile = NULL;

    if (afs)
        fps *= 4; //afsなら4倍精度
    double tm_multi = 1000.0 / fps;

    //ファイル名作成
    apply_appendix(auotcfile, sizeof(auotcfile), pe->temp_filename, pe->append.tc);

    if (fopen_s(&tcfile, auotcfile, "wb") == NULL) {
        fprintf(tcfile, "# timecode format v2\r\n");
        if (afs) {
            for (int i = 0; i < frame_n; i++)
                if (jitter[i] != DROP_FRAME_FLAG)
                    fprintf(tcfile, "%.6lf\r\n", (i * 4 + jitter[i]) * tm_multi);
        } else {
            for (int i = 0; i < frame_n; i++)
                fprintf(tcfile, "%.6lf\r\n", i * tm_multi);
        }
        fclose(tcfile);
    } else {
        ret |= AUO_RESULT_ERROR; warning_auo_tcfile_failed();
    }
    return ret;
}
//並列処理時に音声データを取得する
AUO_RESULT aud_parallel_task(const OUTPUT_INFO *oip, PRM_ENC *pe) {
    AUO_RESULT ret = AUO_RESULT_SUCCESS;
    AUD_PARALLEL_ENC *aud_p = &pe->aud_parallel; //長いんで省略したいだけ
    if (aud_p->th_aud) {
        //---   排他ブロック 開始  ---> 音声スレッドが止まっていなければならない
        if_valid_wait_for_single_object(aud_p->he_vid_start, INFINITE);
        if (aud_p->he_vid_start && aud_p->get_length) {
            DWORD required_buf_size = aud_p->get_length * (DWORD)oip->audio_size;
            if (aud_p->buf_max_size < required_buf_size) {
                //メモリ不足なら再確保
                if (aud_p->buffer) free(aud_p->buffer);
                aud_p->buf_max_size = required_buf_size;
                if (NULL == (aud_p->buffer = malloc(aud_p->buf_max_size)))
                    aud_p->buf_max_size = 0; //ここのmallocエラーは次の分岐でAUO_RESULT_ERRORに設定
            }
            void *data_ptr = NULL;
            if (NULL == aud_p->buffer || 
                NULL == (data_ptr = oip->func_get_audio(aud_p->start, aud_p->get_length, &aud_p->get_length))) {
                ret = AUO_RESULT_ERROR; //mallocエラーかget_audioのエラー
            } else {
                //自前のバッファにコピーしてdata_ptrが破棄されても良いようにする
                memcpy(aud_p->buffer, data_ptr, aud_p->get_length * oip->audio_size);
            }
            //すでにTRUEなら変更しないようにする
            aud_p->abort |= oip->func_is_abort();
        }
        flush_audio_log();
        if_valid_set_event(aud_p->he_aud_start);
        //---   排他ブロック 終了  ---> 音声スレッドを開始
    }
    return ret;
}

//音声処理をどんどん回して終了させる
static AUO_RESULT finish_aud_parallel_task(const OUTPUT_INFO *oip, PRM_ENC *pe, AUO_RESULT vid_ret) {
    //エラーが発生していたら音声出力ループをとめる
    pe->aud_parallel.abort |= (vid_ret != AUO_RESULT_SUCCESS);
    if (pe->aud_parallel.th_aud) {
        write_log_auo_line(LOG_INFO, "音声処理の終了を待機しています...");
        set_window_title("音声処理の終了を待機しています...", PROGRESSBAR_MARQUEE);
        while (pe->aud_parallel.he_vid_start)
            vid_ret |= aud_parallel_task(oip, pe);
        set_window_title(AUO_FULL_NAME, PROGRESSBAR_DISABLED);
    }
    return vid_ret;
}

//並列処理スレッドの終了を待ち、終了コードを回収する
static AUO_RESULT exit_audio_parallel_control(const OUTPUT_INFO *oip, PRM_ENC *pe, AUO_RESULT vid_ret) {
    vid_ret |= finish_aud_parallel_task(oip, pe, vid_ret); //wav出力を完了させる
    release_audio_parallel_events(pe);
    if (pe->aud_parallel.buffer) free(pe->aud_parallel.buffer);
    if (pe->aud_parallel.th_aud) {
        //音声エンコードを完了させる
        //2passエンコードとかだと音声エンコーダの終了を待機する必要あり
        BOOL wait_for_audio = FALSE;
        while (WaitForSingleObject(pe->aud_parallel.th_aud, LOG_UPDATE_INTERVAL) == WAIT_TIMEOUT) {
            if (!wait_for_audio) {
                set_window_title("音声処理の終了を待機しています...", PROGRESSBAR_MARQUEE);
                wait_for_audio = !wait_for_audio;
            }
            pe->aud_parallel.abort |= oip->func_is_abort();
            log_process_events();
        }
        flush_audio_log();
        if (wait_for_audio)
            set_window_title(AUO_FULL_NAME, PROGRESSBAR_DISABLED);

        DWORD exit_code = 0;
        //GetExitCodeThreadの返り値がNULLならエラー
        vid_ret |= (NULL == GetExitCodeThread(pe->aud_parallel.th_aud, &exit_code)) ? AUO_RESULT_ERROR : exit_code;
        CloseHandle(pe->aud_parallel.th_aud);
    }
    //初期化 (重要!!!)
    ZeroMemory(&pe->aud_parallel, sizeof(pe->aud_parallel));
    return vid_ret;
}

void set_conf_vce_prm(VCEInputInfo *pInfo, const OUTPUT_INFO *oip) {
    pInfo->srcWidth = oip->w;
    pInfo->srcHeight = oip->h;

    pInfo->fps.den = oip->scale;
    pInfo->fps.num = oip->rate;

    pInfo->frames = oip->n;
    pInfo->format = amf::AMF_SURFACE_NV12;
}

#pragma warning( push )
#pragma warning( disable: 4100 )
static DWORD video_output_inside(CONF_GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
    //動画エンコードの必要がなければ終了
    if (pe->video_out_type == VIDEO_OUTPUT_DISABLED)
        return AUO_RESULT_SUCCESS;

    VCEInputAuoParam auoParam = { 0 };
    auoParam.conf = conf;
    auoParam.oip = oip;
    auoParam.sys_dat = sys_dat;
    auoParam.pe = pe;
    auoParam.jitter = nullptr;

    VCEInputInfo info = { 0 };
    info.pPrivateParam = &auoParam;
    set_conf_vce_prm(&info, oip);

    tstring pe_tempfile = char_to_tstring(pe->temp_filename);
    conf->vce.pOutputFile = pe_tempfile.data();
    conf->vce.nLogLevel = sys_dat->exstg->s_log.log_level;

    DWORD tm_vce = timeGetTime();
    set_window_title("VCE エンコード", PROGRESSBAR_CONTINUOUS);

    AUO_RESULT ret = AUO_RESULT_SUCCESS;
    unique_ptr<VCECoreAuo> vce(new VCECoreAuo());
    if (conf->vid.afs && is_interlaced(&conf->vce)) {
        ret |= AUO_RESULT_ERROR; error_afs_interlace_stg();
    } else if (NULL == (auoParam.jitter = (int *)calloc(oip->n + 1, sizeof(int)))) {
        ret |= AUO_RESULT_ERROR; error_malloc_tc();
    } else if (AMF_OK != vce->init(&conf->vce, &info)) {
        ret |= AUO_RESULT_ERROR;
    } else {
        vce->PrintEncoderParam();
        if (conf->vid.afs) write_log_auo_line(LOG_INFO, "自動フィールドシフト  on");

        try {
            ret |= vce->run() ? AUO_RESULT_ERROR : AUO_RESULT_SUCCESS;

            PipelineState state = PipelineStateRunning;
            while (ret == AUO_RESULT_SUCCESS && (state = vce->GetState()) != PipelineStateEof && state != PipelineStateError) {
                if (pe->aud_parallel.abort || oip->func_is_abort()) {
                    pe->aud_parallel.abort = TRUE;
                    ret |= AUO_RESULT_ABORT;
                }
                log_process_events();
                amf_sleep(33);
            }
        } catch (...) {
            write_log_auo_line(LOG_INFO, "自動フィールドシフト  on");
            ret |= AUO_RESULT_ERROR;
        }
        flush_audio_log();
        write_log_auo_enc_time("VCE エンコード", timeGetTime() - tm_vce);
        log_process_events();
        //タイムコード出力
        if (!ret && (conf->vid.afs || conf->vid.auo_tcfile_out))
            tcfile_out(auoParam.jitter, oip->n, (double)oip->rate / (double)oip->scale, conf->vid.afs, pe);
        if (!ret && conf->vid.afs)
            write_log_auo_line_fmt(LOG_INFO, "drop %d / %d frames", pe->drop_count, oip->n);
        ret |= (pe->aud_parallel.abort) ? AUO_RESULT_ABORT : AUO_RESULT_SUCCESS;
        log_process_events();
        if (!(ret & AUO_RESULT_ABORT)) {
            vce->PrintResult();
        }
        vce->Terminate();
    }
    set_window_title(AUO_FULL_NAME, PROGRESSBAR_DISABLED);

    if (auoParam.jitter) free(auoParam.jitter);

    return ret;
}
#pragma warning( pop )

AUO_RESULT video_output(CONF_GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
    return exit_audio_parallel_control(oip, pe, video_output_inside(conf, oip, pe, sys_dat));
}
