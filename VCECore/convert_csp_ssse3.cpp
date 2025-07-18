﻿// -----------------------------------------------------------------------------------------
// QSVEnc/NVEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2011-2016 rigaya
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
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// ------------------------------------------------------------------------------------------
#if defined(_M_IX86) || defined(_M_X64) || defined(__x86_64)
#define USE_SSE2  1
#define USE_SSSE3 1
#define USE_SSE41 0
#define USE_AVX   0
#define USE_AVX2  0
#include "convert_csp_simd.h"

#pragma warning (push)
#pragma warning (disable: 4100)

void convert_yc48_to_p010_ssse3(void **dst, const void **src, int width, int src_y_pitch_byte, int src_uv_pitch_byte, int dst_y_pitch_byte, int dst_uv_pitch_byte, int height, int dst_height, int thread_id, int thread_n, int *crop) {
    convert_yc48_to_p010_simd<false>(dst, src, width, src_y_pitch_byte, src_uv_pitch_byte, dst_y_pitch_byte, dst_uv_pitch_byte, height, dst_height, thread_id, thread_n, crop);
}

void convert_yc48_to_p010_i_ssse3(void **dst, const void **src, int width, int src_y_pitch_byte, int src_uv_pitch_byte, int dst_y_pitch_byte, int dst_uv_pitch_byte, int height, int dst_height, int thread_id, int thread_n, int *crop) {
    convert_yc48_to_p010_i_simd<false>(dst, src, width, src_y_pitch_byte, src_uv_pitch_byte, dst_y_pitch_byte, dst_uv_pitch_byte, height, dst_height, thread_id, thread_n, crop);
}

void convert_yuy2_to_nv12_i_ssse3(void **dst, const void **src, int width, int src_y_pitch_byte, int src_uv_pitch_byte, int dst_y_pitch_byte, int dst_uv_pitch_byte, int height, int dst_height, int thread_id, int thread_n, int *crop) {
    return convert_yuy2_to_nv12_i_simd(dst, src, width, src_y_pitch_byte, src_uv_pitch_byte, dst_y_pitch_byte, dst_uv_pitch_byte, height, dst_height, thread_id, thread_n, crop);
}

void convert_bgr24_to_rgb24_ssse3(void **dst, const void **src, int width, int src_y_pitch_byte, int src_uv_pitch_byte, int dst_y_pitch_byte, int dst_uv_pitch_byte, int height, int dst_height, int thread_id, int thread_n, int *crop) {
    convert_rgb24_to_rgb24_simd<RGY_CSP_BGR24>(dst, src, width, src_y_pitch_byte, src_uv_pitch_byte, dst_y_pitch_byte, dst_uv_pitch_byte, height, dst_height, thread_id, thread_n, crop);
}

void convert_bgr32_to_rgb32_ssse3(void **dst, const void **src, int width, int src_y_pitch_byte, int src_uv_pitch_byte, int dst_y_pitch_byte, int dst_uv_pitch_byte, int height, int dst_height, int thread_id, int thread_n, int *crop) {
    convert_rgb32_to_rgb32_simd<RGY_CSP_BGR32>(dst, src, width, src_y_pitch_byte, src_uv_pitch_byte, dst_y_pitch_byte, dst_uv_pitch_byte, height, dst_height, thread_id, thread_n, crop);
}

void convert_rgb24_to_rgb32_ssse3(void **dst, const void **src, int width, int src_y_pitch_byte, int src_uv_pitch_byte, int dst_y_pitch_byte, int dst_uv_pitch_byte, int height, int dst_height, int thread_id, int thread_n, int *crop) {
    convert_rgb24_to_rgb32_simd(dst, src, width, src_y_pitch_byte, src_uv_pitch_byte, dst_y_pitch_byte, dst_uv_pitch_byte, height, dst_height, thread_id, thread_n, crop);
}

void convert_bgr24r_to_bgr32_ssse3(void **dst, const void **src, int width, int src_y_pitch_byte, int src_uv_pitch_byte, int dst_y_pitch_byte, int dst_uv_pitch_byte, int height, int dst_height, int thread_id, int thread_n, int *crop) {
    convert_rgb24r_to_rgb32_simd(dst, src, width, src_y_pitch_byte, src_uv_pitch_byte, dst_y_pitch_byte, dst_uv_pitch_byte, height, dst_height, thread_id, thread_n, crop);
}

void convert_gbr_to_rgb24_ssse3(void **dst, const void **src, int width, int src_y_pitch_byte, int src_uv_pitch_byte, int dst_y_pitch_byte, int dst_uv_pitch_byte, int height, int dst_height, int thread_id, int thread_n, int *crop) {
    convert_rgb_to_rgb24_simd<RGB_PLANE(1, 0, 2, -1)>(dst, src, width, src_y_pitch_byte, src_uv_pitch_byte, dst_y_pitch_byte, dst_uv_pitch_byte, height, dst_height, thread_id, thread_n, crop);
}

void convert_rgb24_to_rgb_ssse3(void **dst, const void **src, int width, int src_y_pitch_byte, int src_uv_pitch_byte, int dst_y_pitch_byte, int dst_uv_pitch_byte, int height, int dst_height, int thread_id, int thread_n, int *crop) {
    convert_rgb24_to_rgb_simd<RGB_PLANE(2, 1, 0, -1), false>(dst, src, width, src_y_pitch_byte, src_uv_pitch_byte, dst_y_pitch_byte, dst_uv_pitch_byte, height, dst_height, thread_id, thread_n, crop);
}

void convert_bgr24_to_rgb_ssse3(void **dst, const void **src, int width, int src_y_pitch_byte, int src_uv_pitch_byte, int dst_y_pitch_byte, int dst_uv_pitch_byte, int height, int dst_height, int thread_id, int thread_n, int *crop) {
    convert_rgb24_to_rgb_simd<RGB_PLANE(0, 1, 2, -1), false>(dst, src, width, src_y_pitch_byte, src_uv_pitch_byte, dst_y_pitch_byte, dst_uv_pitch_byte, height, dst_height, thread_id, thread_n, crop);
}

void convert_bgr24r_to_rgb_ssse3(void **dst, const void **src, int width, int src_y_pitch_byte, int src_uv_pitch_byte, int dst_y_pitch_byte, int dst_uv_pitch_byte, int height, int dst_height, int thread_id, int thread_n, int *crop) {
    convert_rgb24_to_rgb_simd<RGB_PLANE(2, 1, 0, -1), true>(dst, src, width, src_y_pitch_byte, src_uv_pitch_byte, dst_y_pitch_byte, dst_uv_pitch_byte, height, dst_height, thread_id, thread_n, crop);
}

void convert_yc48_to_yuv444_16bit_ssse3(void **dst, const void **src, int width, int src_y_pitch_byte, int src_uv_pitch_byte, int dst_y_pitch_byte, int dst_uv_pitch_byte, int height, int dst_height, int thread_id, int thread_n, int *crop) {
    convert_yc48_to_yuv444_16bit_simd<false>(dst, src, width, src_y_pitch_byte, src_uv_pitch_byte, dst_y_pitch_byte, dst_uv_pitch_byte, height, dst_height, thread_id, thread_n, crop);
}
#pragma warning (pop)
#endif //#if defined(_M_IX86) || defined(_M_X64) || defined(__x86_64)