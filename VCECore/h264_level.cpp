//  -----------------------------------------------------------------------------------------
//    VCEEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#include "h264_level.h"

const int MAX_REF_FRAMES = 16;
const int PROGRESSIVE    = 1;
const int INTERLACED     = 2;
const int LEVEL_COLUMNS  = 7;
const int COLUMN_VBVMAX  = 4;
const int COLUMN_VBVBUF  = 5;

static inline int ceil_div_int(int i, int div) {
	return (i + (div-1)) / div;
}
static inline __int64 ceil_div_int64(__int64 i, int div) {
	return (i + (div-1)) / div;
}

const int H264_LEVELS[] = {
	0,
	10,9,11,12,13,
	20,21,22,
	30,31,32,
	40,41,42,
	50,51,52,
	0
};

static const int H264_LEVEL_LIMITS[][LEVEL_COLUMNS] =
{   //interlaced, MaxMBpsec, MaxMBpframe, MaxDpbMbs, MaxVBVMaxrate, MaxVBVBuf,    end,  level
	{ PROGRESSIVE,        -1,          -1,        -1,             0,         0,  NULL}, // auto
	{ PROGRESSIVE,      1485,          99,       396,            64,       175,  NULL}, // 1
	{ PROGRESSIVE,      1485,          99,       396,           128,       350,  NULL}, // 1b
	{ PROGRESSIVE,      3000,         396,       900,           192,       500,  NULL}, // 1.1
	{ PROGRESSIVE,      6000,         396,      2376,           384,      1000,  NULL}, // 1.2
	{ PROGRESSIVE,     11880,         396,      2376,           768,      2000,  NULL}, // 1.3
	{ PROGRESSIVE,     11880,         396,      2376,          2000,      2000,  NULL}, // 2
	{  INTERLACED,     19800,         792,      4752,          4000,      4000,  NULL}, // 2.1
	{  INTERLACED,     20250,        1620,      8100,          4000,      4000,  NULL}, // 2.2
	{  INTERLACED,     40500,        1620,      8100,         10000,     10000,  NULL}, // 3
	{  INTERLACED,    108000,        3600,     18000,         14000,     14000,  NULL}, // 3.1
	{  INTERLACED,    216000,        5120,     20480,         20000,     20000,  NULL}, // 3.2
	{  INTERLACED,    245760,        8192,     32768,         20000,     25000,  NULL}, // 4
	{  INTERLACED,    245760,        8192,     32768,         50000,     62500,  NULL}, // 4.1
	{  INTERLACED,    522240,        8704,     34816,         50000,     62500,  NULL}, // 4.2
	{ PROGRESSIVE,    589824,       22080,    110400,        135000,    135000,  NULL}, // 5
	{ PROGRESSIVE,    983040,       36864,    184320,        240000,    240000,  NULL}, // 5.1
	{ PROGRESSIVE,   2073600,       36864,    184320,        240000,    240000,  NULL}, // 5.2
	{        NULL,      NULL,        NULL,      NULL,          NULL,      NULL,  NULL}, // end
};
//必要なLevelを計算する, 適合するLevelがなければ 0 を返す
int calc_auto_level(int width, int height, int ref, BOOL interlaced, int fps_num, int fps_den, int vbv_max, int vbv_buf) {
	int i, j = (interlaced) ? INTERLACED : PROGRESSIVE;
	int MB_frame = ceil_div_int(width, 16) * (j * ceil_div_int(height, 16*j));
	int data[LEVEL_COLUMNS] = {
		j,
		(int)ceil_div_int64((UINT64)MB_frame * fps_num, fps_den),
		MB_frame,
		MB_frame * ref,
		vbv_max, 
		vbv_buf,
		NULL
	};

	//あとはひたすら比較
	i = 1, j = 0; // i -> 行(Level), j -> 列(項目)
	while (H264_LEVEL_LIMITS[i][j])
		(data[j] > H264_LEVEL_LIMITS[i][j]) ? i++ : j++;
	//一番右の列まで行き着いてればそれが求めるレベル 一応インターレースについても再チェック
	int level_idx = (j == (LEVEL_COLUMNS-1) && data[0] <= H264_LEVEL_LIMITS[i][0]) ? i : 0;
	return H264_LEVELS[level_idx];
}

const float VBV_MULTI[] = {
	1.00, 1.00, 1.25, 3.00, 0.00
};

//vbv値を求める *vbv_max と *vbv_buf はNULLでもOK
void get_vbv_value(int *vbv_max, int *vbv_buf, int level, int profile_index) {
	if (level > 0 && H264_LEVEL_LIMITS[level][1] > 0) {

		float profile_multi = VBV_MULTI[profile_index];

		if (vbv_max)
			*vbv_max = (int)(H264_LEVEL_LIMITS[level][COLUMN_VBVMAX] * profile_multi);
		if (vbv_buf)
			*vbv_buf = (int)(H264_LEVEL_LIMITS[level][COLUMN_VBVBUF] * profile_multi);
	} else {
		if (vbv_max)
			*vbv_max = 0;
		if (vbv_buf)
			*vbv_buf = 0;
	}
	return;
}
