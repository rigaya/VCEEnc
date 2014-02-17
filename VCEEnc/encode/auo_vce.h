//  -----------------------------------------------------------------------------------------
//    VCEEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _AUO_VCE_H_
#define _AUO_VCE_H_

#include <Windows.h>
#include <stdio.h>
#include "output.h"
#include "auo.h"
#include "auo_conf.h"
#include "auo_system.h"

#include "VCECore.h"

DWORD set_auo_vce_g_data(const OUTPUT_INFO *_oip, CONF_GUIEX *conf, PRM_ENC *_pe, int *jitter, BOOL *pause);
void clear_auo_vce_g_data();

bool auo_vce_read(FILE *fr, uint32 uiHeight, uint32 uiWidth, uint32 alignedSurfaceWidth, int8 *pBitstreamData);
int auo_vce_print_mes(FILE *fp, int log_level, double progress, const char *format, ... );

#endif //_AUO_VCE_H_