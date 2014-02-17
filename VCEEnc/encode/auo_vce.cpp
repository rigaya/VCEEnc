//  -----------------------------------------------------------------------------------------
//    VCEEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

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

BOOL setup_afsvideo(const OUTPUT_INFO *oip, CONF_GUIEX *conf, PRM_ENC *pe, BOOL auto_afs_disable) {
	//すでに初期化してある または 必要ない
	if (pe->afs_init || pe->video_out_type == VIDEO_OUTPUT_DISABLED || !conf->vid.afs)
		return TRUE;

	const int color_format = CF_YUY2;
	const int frame_size = calc_input_frame_size(oip->w, oip->h, color_format);
	//Aviutl(自動フィールドシフト)からの映像入力
	if (afs_vbuf_setup((OUTPUT_INFO *)oip, conf->vid.afs, frame_size, COLORFORMATS[color_format].FOURCC)) {
		pe->afs_init = TRUE;
		return TRUE;
	} else if (conf->vid.afs && auto_afs_disable) {
		afs_vbuf_release(); //一度解放
		warning_auto_afs_disable();
		conf->vid.afs = FALSE;
		//再度使用するmuxerをチェックする
		pe->muxer_to_be_used = check_muxer_to_be_used(conf, pe->video_out_type, (oip->flag & OUTPUT_INFO_FLAG_AUDIO) != 0);
		return TRUE;
	}
	//エラー
	error_afs_setup(conf->vid.afs, auto_afs_disable);
	return FALSE;
}

void close_afsvideo(PRM_ENC *pe) {
	if (!pe->afs_init || pe->video_out_type == VIDEO_OUTPUT_DISABLED)
		return;

	afs_vbuf_release();

	pe->afs_init = FALSE;
}

static const func_convert_frame CONVERT_FUNC[][2] = {
	{ convert_yuy2_to_nv12_sse2_aligned,    convert_yuy2_to_nv12_sse2   },
	{ convert_yuy2_to_nv12_i_sse2_aligned,  convert_yuy2_to_nv12_i_sse2 },
	{ convert_yuy2_to_nv12_sse2_aligned,    convert_yuy2_to_nv12_sse2   },
	{ convert_yuy2_to_nv12_i_ssse3_aligned, convert_yuy2_to_nv12_i_ssse3 },
#if (_MSC_VER >= 1600)
	{ convert_yuy2_to_nv12_avx_aligned,     convert_yuy2_to_nv12_avx   },
	{ convert_yuy2_to_nv12_i_avx_aligned,   convert_yuy2_to_nv12_i_avx }
#endif
};

static const func_convert_frame *convert_frame;
static const OUTPUT_INFO *g_oip = NULL;
static CONF_GUIEX *g_conf = NULL;
static PRM_ENC *g_pe = NULL;
static int g_frames = 0;
static int *g_jitter = NULL;
static BOOL *g_pause = FALSE;

DWORD set_auo_vce_g_data(const OUTPUT_INFO *_oip, CONF_GUIEX *conf, PRM_ENC *_pe, int *jitter, BOOL *_pause) {
	g_oip = _oip;
	g_conf = conf;
	g_pe =_pe;
	g_frames = 0;
	g_jitter = jitter;
	g_pause = _pause;

	BOOL interlaced = (is_interlaced(conf->vid.vce_ext_prm.pic_struct)) ? TRUE : FALSE;
	BOOL ssse3_available = check_ssse3() != FALSE;
	BOOL avx_available = FALSE;
#if (_MSC_VER >= 1600)
	avx_available = check_avx() != FALSE;
#endif
	convert_frame = CONVERT_FUNC[interlaced+(ssse3_available+avx_available)*2];
	write_log_auo_line_fmt(LOG_INFO, "converting YUY2 -> NV12%s, using SSE2%s%s",
		((interlaced) ? "i" : "p"),
		((interlaced && ssse3_available) ? " SSSE3" : ""),
		((avx_available) ? " AVX" : "")
		);
	return 0;
}

void clear_auo_vce_g_data() {
	g_oip = NULL;
	g_conf = NULL;
	g_pe = NULL;
	g_frames = 0;
	g_jitter = NULL;
	g_pause = NULL;
}

#pragma warning( push )
#pragma warning( disable: 4100 )
//true if successful; otherwise false.
bool auo_vce_read(FILE *fr, uint32 uiHeight, uint32 uiWidth, uint32 alignedSurfaceWidth, int8 *pBitstreamData) {
	if (FALSE != (g_pe->aud_parallel.abort = g_oip->func_is_abort()))
		return false;

	int i_frame = g_frames + g_pe->drop_count;

	if (i_frame >= g_oip->n) {
		g_oip->func_rest_time_disp(i_frame, g_oip->n);
		release_audio_parallel_events(g_pe);
		return false;
	}

	while (*g_pause) {
		Sleep(LOG_UPDATE_INTERVAL);
		if (FALSE != (g_pe->aud_parallel.abort = g_oip->func_is_abort()))
			return false;
		log_process_events();
	}

	void *frame;
	if (g_conf->vid.afs) {
		BOOL drop = FALSE;
		for ( ; ; ) {
			if ((frame = afs_get_video((OUTPUT_INFO *)g_oip, i_frame, &drop, &g_jitter[i_frame + 1])) == NULL) {
				error_afs_get_frame();
				return false;
			}
			if (!drop)
				break;
			g_jitter[i_frame] = DROP_FRAME_FLAG;
			g_pe->drop_count++;
			i_frame++;
			if (i_frame >= g_oip->n) {
				g_oip->func_rest_time_disp(i_frame, g_oip->n);
				release_audio_parallel_events(g_pe);
				return false;
			}
		}
	} else {
		if ((frame = g_oip->func_get_video_ex(i_frame, COLORFORMATS[CF_YUY2].FOURCC)) == NULL) {
			error_afs_get_frame();
			return false;
		}
	}

	int bAligned = (((size_t)pBitstreamData) & 0x0F) != 0x00;
	convert_frame[bAligned](frame, (BYTE *)pBitstreamData, (BYTE *)pBitstreamData + alignedSurfaceWidth * uiHeight, uiWidth, uiHeight, alignedSurfaceWidth);

	if (!(g_frames & 7)) {
		aud_parallel_task(g_oip, g_pe);
		g_oip->func_rest_time_disp(g_frames + g_pe->drop_count, g_oip->n);
		g_oip->func_update_preview();
	}

	g_frames++;
	return true;
}


int auo_vce_print_mes(FILE *fp, int log_level, double progress, const char *format, ... ) {
	log_level = clamp(log_level, LOG_INFO, LOG_ERROR);

    va_list args;
    va_start(args, format);

    int len = _vscprintf(format, args) + 1; // _vscprintf doesn't count terminating '\0'
    char *const buffer = (char*)malloc(len * sizeof(buffer[0]));

    vsprintf_s(buffer, len, format, args);

	static const char *const LOG_LEVEL_STR[] = { "info", "warning", "error" };
	const int mes_line_len = len + strlen("vce [warning]: ");
	char *const mes_line = (char *)malloc(mes_line_len * sizeof(mes_line[0]));

	char *a, *b, *mes = buffer;
	char *const fin = mes + len; //null文字の位置
	while ((a = strchr(mes, '\n')) != NULL) {
		if ((b = strrchr(mes, '\r', a - mes - 2)) != NULL)
			mes = b + 1;
		*a = '\0';
		sprintf_s(mes_line, mes_line_len, "vce [%s]: %s", LOG_LEVEL_STR[log_level], mes);
		write_log_line(log_level, mes_line);
		mes = a + 1;
	}
	if ((a = strrchr(mes, '\r', fin - mes - 1)) != NULL) {
		b = a - 1;
		while (*b == ' ' || *b == '\r')
			b--;
		*(b+1) = '\0';
		if ((b = strrchr(mes, '\r', b - mes - 2)) != NULL)
			mes = b + 1;
		set_log_title_and_progress(mes, progress);
		mes = a + 1;
	}

	free(buffer);
	free(mes_line);
	return len - 1;
}
