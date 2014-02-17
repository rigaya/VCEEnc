//  -----------------------------------------------------------------------------------------
//    VCEEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

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

void set_conf_vce_prm(OvConfigCtrl *vce_prm, const OUTPUT_INFO *oip) {
	vce_prm->width = oip->w;
	vce_prm->height = oip->h;

	vce_prm->rateControl.encRateControlFrameRateDenominator = oip->scale;
	vce_prm->rateControl.encRateControlFrameRateNumerator = oip->rate;
}

#pragma warning( push )
#pragma warning( disable: 4100 )
static DWORD video_output_inside(CONF_GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
	//動画エンコードの必要がなければ終了
	if (pe->video_out_type == VIDEO_OUTPUT_DISABLED)
		return AUO_RESULT_SUCCESS;

	//H.264/ESしか出せないので拡張子を変更
	change_ext(pe->temp_filename, _countof(pe->temp_filename), ".264");

	set_conf_vce_prm(&conf->vce, oip);

	DWORD tm_vce = timeGetTime();
	set_window_title("VCE エンコード", PROGRESSBAR_CONTINUOUS);

	AUO_RESULT ret = AUO_RESULT_SUCCESS;
	int *jitter = NULL;
	BOOL pause = FALSE;
	vce_handle_t *vce = NULL;
	if (conf->vid.afs && is_interlaced(conf->vid.vce_ext_prm.pic_struct)) {
		ret |= AUO_RESULT_ERROR; error_afs_interlace_stg();
	} else if (NULL == (jitter = (int *)calloc(oip->n + 1, sizeof(int)))) {
		ret |= AUO_RESULT_ERROR; error_malloc_tc();
	} else if (set_auo_vce_g_data(oip, conf, pe, jitter, &pause)
		|| NULL == (vce = vce_init(&conf->vce, (bool*)&pe->aud_parallel.abort, auo_vce_print_mes, auo_vce_read, NULL, NULL, pe->temp_filename, &conf->vid.vce_ext_prm, oip->n))) {
			ret |= AUO_RESULT_ERROR;
	} else {
		if (conf->vid.afs) write_log_auo_line(LOG_INFO, "自動フィールドシフト  on");
		enable_enc_control(&pause, pe->afs_init, FALSE, timeGetTime(), oip->n);
		ret |= vce_run(vce) ? AUO_RESULT_SUCCESS : AUO_RESULT_ERROR;
		flush_audio_log();
		write_log_auo_enc_time("VCE エンコード", timeGetTime() - tm_vce);
		//タイムコード出力
		if (!ret && (conf->vid.afs || conf->vid.auo_tcfile_out))
			tcfile_out(jitter, oip->n, (double)oip->rate / (double)oip->scale, conf->vid.afs, pe);
		if (!ret && conf->vid.afs)
			write_log_auo_line_fmt(LOG_INFO, "drop %d / %d frames", pe->drop_count, oip->n);
		ret |= (pe->aud_parallel.abort) ? AUO_RESULT_ABORT : AUO_RESULT_SUCCESS;
		disable_enc_control();
		vce_close(vce);
	}
	set_window_title(AUO_FULL_NAME, PROGRESSBAR_DISABLED);
	
	clear_auo_vce_g_data();
	if (jitter) free(jitter);

	return ret;
}
#pragma warning( pop )

AUO_RESULT video_output(CONF_GUIEX *conf, const OUTPUT_INFO *oip, PRM_ENC *pe, const SYSTEM_DATA *sys_dat) {
	return exit_audio_parallel_control(oip, pe, video_output_inside(conf, oip, pe, sys_dat));
}
