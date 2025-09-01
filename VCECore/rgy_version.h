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

#pragma once
#ifndef __RGY_VERSION_H__
#define __RGY_VERSION_H__

#define VER_FILEVERSION             0,9,0,0
#define VER_STR_FILEVERSION          "9.00"
#define VER_STR_FILEVERSION_TCHAR _T("9.00")


#ifdef _M_IX86
#define BUILD_ARCH_STR _T("x86")
#else
#define BUILD_ARCH_STR _T("x64")
#endif

#if _UNICODE
const wchar_t *get_encoder_version();
#else
const char *get_encoder_version();
#endif

#define ENCODER_QSV    0
#define ENCODER_NVENC  0
#define ENCODER_VCEENC 1
#define ENCODER_MPP    0
#define CLFILTERS_AUF  0

#define AV1_TIMESTAMP_OVERRIDE 1

#define DONOTSHOW_AMF_POINT_FSR11 1
#define ENABLE_AVOID_IDLE_CLOCK 0
#define ENABLE_VPP_SMOOTH_QP_FRAME 0
#define ENABLE_DOVI_METADATA_OPTIONS 1

#if defined(_WIN32) || defined(_WIN64)

#define ENABLE_PERF_COUNTER 1
#define ENABLE_AVCODEC_OUT_THREAD 1
#define ENABLE_AVCODEC_AUDPROCESS_THREAD 1
#define ENABLE_D3D9 1
#define ENABLE_D3D11 1
#define ENABLE_D3D11_DEVINFO_WMI 1
#define ENABLE_VULKAN 0
#define VULKAN_DEFAULT_DEVICE_ONLY 0
#define ENABLE_CPP_REGEX 1
#define ENABLE_DTL 1
#define ENABLE_LIBASS_SUBBURN 1
#define ENABLE_LIBDOVI 1
#define ENABLE_LIBHDR10PLUS 1
#define ENABLE_LIBPLACEBO 1

#define AV_CHANNEL_LAYOUT_STRUCT_AVAIL 1
#define AV_FRAME_DURATION_AVAIL 1
#define AVCODEC_PAR_CODED_SIDE_DATA_AVAIL 1
#define AVS_INTERF_VER 11

#define ENABLE_KEYFRAME_INSERT 0

#define GPU_VENDOR "AMD"
#define DECODER_NAME  "vce"
#define HW_TIMEBASE 10000000L //AMF_SECOND

#if defined(VCE_AUO)
#define ENCODER_NAME  "VCEEnc"
#define FOR_AUO                   1
#define ENABLE_RAW_READER         0
#define ENABLE_AVI_READER         0
#define ENABLE_AVISYNTH_READER    0
#define ENABLE_VAPOURSYNTH_READER 0
#define ENABLE_AVSW_READER        0
#define ENABLE_LIBAVDEVICE        0
#define ENABLE_SM_READER          0
#define ENABLE_OPENCL             0
#define ENABLE_CAPTION2ASS        0
#define ENABLE_AUTO_PICSTRUCT     0
#else
#define ENCODER_NAME  "VCEEnc"
#define FOR_AUO                   0
#define ENABLE_RAW_READER         1
#define ENABLE_AVI_READER         1
#define ENABLE_AVISYNTH_READER    1
#define ENABLE_VAPOURSYNTH_READER 1
#define ENABLE_AVSW_READER        1
#define ENABLE_SM_READER          1
#define ENABLE_LIBAVDEVICE        1
#define ENABLE_OPENCL             1
#define ENABLE_CAPTION2ASS        1
#define ENABLE_AUTO_PICSTRUCT     1
#endif

#else //#if defined(WIN32) || defined(WIN64)
#define FOR_AUO 0
#define ENABLE_PERF_COUNTER 0
#define ENABLE_AVCODEC_OUT_THREAD 1
#define ENABLE_AVCODEC_AUDPROCESS_THREAD 1
#define ENABLE_D3D9 0
#define ENABLE_D3D11 0
#define ENABLE_D3D11_DEVINFO_WMI   0
#define VULKAN_DEFAULT_DEVICE_ONLY 1
#define ENABLE_CAPTION2ASS 0

#define ENABLE_KEYFRAME_INSERT 0
#define ENABLE_AUTO_PICSTRUCT 0
#define ENABLE_SM_READER          0

#include "rgy_config.h"
#define ENCODER_NAME              "VCEEncC"
#define DECODER_NAME              "vce"
#define HW_TIMEBASE 10000000L //AMF_SECOND
#endif // #if defined(WIN32) || defined(WIN64)

#endif //##ifndef __RGY_VERSION_H__
