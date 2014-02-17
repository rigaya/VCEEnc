//  -----------------------------------------------------------------------------------------
//    VCEEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _H264_LEVEL_H_
#define _H264_LEVEL_H_

int calc_auto_level(int width, int height, int ref, BOOL interlaced, int fps_num, int fps_den, int vbv_max, int vbv_buf);
void get_vbv_value(int *vbv_max, int *vbv_buf, int level, int profile_index);

#endif //_H264_LEVEL_H_
