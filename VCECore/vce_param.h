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
#pragma warning(push)
#pragma warning(disable:4201)
#pragma warning(disable:4100)
#include "VideoEncoderVCE.h"
#include "VideoEncoderHEVC.h"
#include "rgy_caption.h"
#include "rgy_util.h"
#include "rgy_prm.h"

static const wchar_t* VCE_PARAM_KEY_INPUT = L"INPUT";
static const wchar_t* VCE_PARAM_KEY_INPUT_WIDTH = L"WIDTH";
static const wchar_t* VCE_PARAM_KEY_INPUT_HEIGHT = L"HEIGHT";

static const wchar_t* VCE_PARAM_KEY_OUTPUT = L"OUTPUT";
static const wchar_t* VCE_PARAM_KEY_OUTPUT_WIDTH = L"OUTPUT_WIDTH";
static const wchar_t* VCE_PARAM_KEY_OUTPUT_HEIGHT = L"OUTPUT_HEIGHT";

static const wchar_t* VCE_PARAM_KEY_ENGINE = L"ENGINE";

static const wchar_t* VCE_PARAM_KEY_ADAPTERID = L"ADAPTERID";
static const wchar_t* VCE_PARAM_KEY_CAPABILITY = L"DISPLAYCAPABILITY";

static const int   FILTER_DEFAULT_AFS_CLIP_TB = 16;
static const int   FILTER_DEFAULT_AFS_CLIP_LR = 32;
static const int   FILTER_DEFAULT_AFS_TB_ORDER = 0;
static const int   FILTER_DEFAULT_AFS_METHOD_SWITCH = 0;
static const int   FILTER_DEFAULT_AFS_COEFF_SHIFT = 192;
static const int   FILTER_DEFAULT_AFS_THRE_SHIFT = 128;
static const int   FILTER_DEFAULT_AFS_THRE_DEINT = 48;
static const int   FILTER_DEFAULT_AFS_THRE_YMOTION = 112;
static const int   FILTER_DEFAULT_AFS_THRE_CMOTION = 224;
static const int   FILTER_DEFAULT_AFS_ANALYZE = 3;
static const bool  FILTER_DEFAULT_AFS_SHIFT = true;
static const bool  FILTER_DEFAULT_AFS_DROP = false;
static const bool  FILTER_DEFAULT_AFS_SMOOTH = false;
static const bool  FILTER_DEFAULT_AFS_FORCE24 = false;
static const bool  FILTER_DEFAULT_AFS_TUNE = false;
static const bool  FILTER_DEFAULT_AFS_RFF = false;
static const bool  FILTER_DEFAULT_AFS_TIMECODE = false;
static const bool  FILTER_DEFAULT_AFS_LOG = false;

static const int   FILTER_DEFAULT_DECIMATE_CYCLE = 5;
static const float FILTER_DEFAULT_DECIMATE_THRE_DUP = 1.1f;
static const float FILTER_DEFAULT_DECIMATE_THRE_SC = 15.0f;
static const int   FILTER_DEFAULT_DECIMATE_BLOCK_X = 32;
static const int   FILTER_DEFAULT_DECIMATE_BLOCK_Y = 32;
static const bool  FILTER_DEFAULT_DECIMATE_PREPROCESSED = false;
static const bool  FILTER_DEFAULT_DECIMATE_CHROMA = true;
static const bool  FILTER_DEFAULT_DECIMATE_LOG = false;

static const int   FILTER_DEFAULT_KNN_RADIUS = 3;
static const float FILTER_DEFAULT_KNN_STRENGTH = 0.08f;
static const float FILTER_DEFAULT_KNN_LERPC = 0.20f;
static const float FILTER_DEFAULT_KNN_WEIGHT_THRESHOLD = 0.01f;
static const float FILTER_DEFAULT_KNN_LERPC_THRESHOLD = 0.80f;

static const float FILTER_DEFAULT_PMD_STRENGTH = 100.0f;
static const float FILTER_DEFAULT_PMD_THRESHOLD = 100.0f;
static const int   FILTER_DEFAULT_PMD_APPLY_COUNT = 2;
static const bool  FILTER_DEFAULT_PMD_USE_EXP = true;

static const int   FILTER_DEFAULT_SMOOTH_QUALITY = 3;
static const int   FILTER_DEFAULT_SMOOTH_QP = 12;
static const float FILTER_DEFAULT_SMOOTH_STRENGTH = 0.0f;
static const float FILTER_DEFAULT_SMOOTH_THRESHOLD = 0.0f;
static const int   FILTER_DEFAULT_SMOOTH_MODE = 0;
static const float FILTER_DEFAULT_SMOOTH_B_RATIO = 0.5f;
static const int   FILTER_DEFAULT_SMOOTH_MAX_QPTABLE_ERR = 10;

static const float FILTER_DEFAULT_TWEAK_BRIGHTNESS = 0.0f;
static const float FILTER_DEFAULT_TWEAK_CONTRAST = 1.0f;
static const float FILTER_DEFAULT_TWEAK_GAMMA = 1.0f;
static const float FILTER_DEFAULT_TWEAK_SATURATION = 1.0f;
static const float FILTER_DEFAULT_TWEAK_HUE = 0.0f;

static const float FILTER_DEFAULT_EDGELEVEL_STRENGTH = 5.0f;
static const float FILTER_DEFAULT_EDGELEVEL_THRESHOLD = 20.0f;
static const float FILTER_DEFAULT_EDGELEVEL_BLACK = 0.0f;
static const float FILTER_DEFAULT_EDGELEVEL_WHITE = 0.0f;

static const int   FILTER_DEFAULT_UNSHARP_RADIUS = 3;
static const float FILTER_DEFAULT_UNSHARP_WEIGHT = 0.5f;
static const float FILTER_DEFAULT_UNSHARP_THRESHOLD = 10.0f;

static const int   FILTER_DEFAULT_DEBAND_RANGE = 15;
static const int   FILTER_DEFAULT_DEBAND_THRE_Y = 15;
static const int   FILTER_DEFAULT_DEBAND_THRE_CB = 15;
static const int   FILTER_DEFAULT_DEBAND_THRE_CR = 15;
static const int   FILTER_DEFAULT_DEBAND_DITHER_Y = 15;
static const int   FILTER_DEFAULT_DEBAND_DITHER_C = 15;
static const int   FILTER_DEFAULT_DEBAND_MODE = 1;
static const int   FILTER_DEFAULT_DEBAND_SEED = 1234;
static const bool  FILTER_DEFAULT_DEBAND_BLUR_FIRST = false;
static const bool  FILTER_DEFAULT_DEBAND_RAND_EACH_FRAME = false;

enum {
    VCE_RC_CQP = 0,
    VCE_RC_CBR = 3,
    VCE_RC_VBR = 4,
};

enum : uint8_t {
    VCE_MOTION_EST_FULL   = 0x00,
    VCE_MOTION_EST_HALF   = 0x01,
    VCE_MOTION_EST_QUATER = 0x02,
};

const CX_DESC list_codec[] = {
    { _T("h264"), RGY_CODEC_H264 },
    { _T("hevc"), RGY_CODEC_HEVC },
    { NULL, 0 }
};
const CX_DESC list_codec_all[] = {
    { _T("h264"), RGY_CODEC_H264 },
    { _T("avc"),  RGY_CODEC_H264 },
    { _T("h265"), RGY_CODEC_HEVC },
    { _T("hevc"), RGY_CODEC_HEVC },
    { NULL, 0 }
};

const CX_DESC list_avc_profile[] = {
    { _T("Baseline"), AMF_VIDEO_ENCODER_PROFILE_BASELINE },
    { _T("Main"),     AMF_VIDEO_ENCODER_PROFILE_MAIN     },
    { _T("High"),     AMF_VIDEO_ENCODER_PROFILE_HIGH     },
    { NULL, NULL }
};

const CX_DESC list_avc_level[] = {
    { _T("auto"), 0   },
    { _T("1"),    10  },
    { _T("1b"),   9   },
    { _T("1.1"),  11  },
    { _T("1.2"),  12  },
    { _T("1.3"),  13  },
    { _T("2"),    20  },
    { _T("2.1"),  21  },
    { _T("2.2"),  22  },
    { _T("3"),    30  },
    { _T("3.1"),  31  },
    { _T("3.2"),  32  },
    { _T("4"),    40  },
    { _T("4.1"),  41  },
    { _T("4.2"),  42  },
    { _T("5"),    50  },
    { _T("5.1"),  51  },
    { _T("5.2"),  52  },
    { NULL, NULL }
};

const CX_DESC list_hevc_profile[] = {
    { _T("main"),     AMF_VIDEO_ENCODER_HEVC_PROFILE_MAIN },
    { NULL, NULL }
};

const CX_DESC list_hevc_tier[] = {
    { _T("main"),     AMF_VIDEO_ENCODER_HEVC_TIER_MAIN },
    { _T("high"),     AMF_VIDEO_ENCODER_HEVC_TIER_HIGH },
    { NULL, NULL }
};

const CX_DESC list_hevc_level[] = {
    { _T("auto"), 0 },
    { _T("1"),    AMF_LEVEL_1 },
    { _T("2"),    AMF_LEVEL_2 },
    { _T("2.1"),  AMF_LEVEL_2_1 },
    { _T("3"),    AMF_LEVEL_3 },
    { _T("3.1"),  AMF_LEVEL_3_1 },
    { _T("4"),    AMF_LEVEL_4 },
    { _T("4.1"),  AMF_LEVEL_4_1 },
    { _T("5"),    AMF_LEVEL_5 },
    { _T("5.1"),  AMF_LEVEL_5_1 },
    { _T("5.2"),  AMF_LEVEL_5_2 },
    { _T("6"),    AMF_LEVEL_6 },
    { _T("6.1"),  AMF_LEVEL_6_1 },
    { _T("6.2"),  AMF_LEVEL_6_2 },
    { NULL, NULL }
};

const CX_DESC list_mv_presicion[] = {
    { _T("full-pel"), VCE_MOTION_EST_FULL   },
    { _T("half-pel"), VCE_MOTION_EST_HALF   },
    { _T("Q-pel"),    VCE_MOTION_EST_QUATER | VCE_MOTION_EST_HALF },
    { NULL, NULL }
};

const CX_DESC list_vce_h264_rc_method[] = {
    { _T("CQP"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP          },
    { _T("CBR"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR                  },
    { _T("VBR"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR },
    { _T("VBR_LAT"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR },
    { NULL, NULL }
};

const CX_DESC list_vce_hevc_rc_method[] = {
    { _T("CQP"), AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CONSTANT_QP },
    { _T("CBR"), AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CBR },
    { _T("VBR"), AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR },
    { _T("VBR_LAT"), AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR },
    { NULL, NULL }
};

const CX_DESC list_vce_preanalysis_h264[] = {
    { _T("off"), AMF_VIDEO_ENCODER_PREENCODE_DISABLED },
    { _T("on"),  AMF_VIDEO_ENCODER_PREENCODE_ENABLED },
};

#define AMF_PA_SCENE_CHANGE_DETECTION_NONE (AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY_LOW-1)
#define AMF_PA_STATIC_SCENE_DETECTION_NONE (AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY_LOW-1)

const CX_DESC list_pa_sc_sensitivity[] = {
    { _T("none"),   AMF_PA_SCENE_CHANGE_DETECTION_NONE },
    { _T("low"),    AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY_LOW },
    { _T("medium"), AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY_MEDIUM },
    { _T("high"),   AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY_HIGH },
    { NULL, NULL }
};

const CX_DESC list_pa_ss_sensitivity[] = {
    { _T("none"),   AMF_PA_STATIC_SCENE_DETECTION_NONE },
    { _T("low"),    AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY_LOW },
    { _T("medium"), AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY_MEDIUM },
    { _T("high"),   AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY_HIGH },
    { NULL, NULL }
};

const CX_DESC list_pa_activity[] = {
    { _T("y"),   AMF_PA_ACTIVITY_Y },
    { _T("yuv"), AMF_PA_ACTIVITY_YUV },
    { NULL, NULL }
};

const CX_DESC list_pa_caq_strength[] = {
    { _T("low"),    AMF_PA_CAQ_STRENGTH_LOW },
    { _T("medium"), AMF_PA_CAQ_STRENGTH_MEDIUM },
    { _T("high"),   AMF_PA_CAQ_STRENGTH_HIGH },
    { NULL, NULL }
};



const CX_DESC list_vpp_denoise[] = {
    { _T("none"),   0 },
    { _T("knn"),    1 },
    { _T("pmd"),    2 },
    //{ _T("smooth"), 3 },
    { NULL, 0 }
};

const CX_DESC list_vpp_detail_enahance[] = {
    { _T("none"),       0 },
    { _T("unsharp"),    1 },
    { _T("edgelevel"),  2 },
    { NULL, 0 }
};

enum RGY_VPP_RESIZE_ALGO {
    RGY_VPP_RESIZE_AUTO,
    RGY_VPP_RESIZE_BILINEAR,
    RGY_VPP_RESIZE_SPLINE16,
    RGY_VPP_RESIZE_SPLINE36,
    RGY_VPP_RESIZE_SPLINE64,
    RGY_VPP_RESIZE_LANCZOS2,
    RGY_VPP_RESIZE_LANCZOS3,
    RGY_VPP_RESIZE_LANCZOS4,
};

const CX_DESC list_vpp_resize[] = {
    { _T("auto"),     RGY_VPP_RESIZE_AUTO },
    { _T("spline16"), RGY_VPP_RESIZE_SPLINE16 },
    { _T("spline36"), RGY_VPP_RESIZE_SPLINE36 },
    { _T("spline64"), RGY_VPP_RESIZE_SPLINE64 },
    { _T("lanczos2"), RGY_VPP_RESIZE_LANCZOS2 },
    { _T("lanczos3"), RGY_VPP_RESIZE_LANCZOS3 },
    { _T("lanczos4"), RGY_VPP_RESIZE_LANCZOS4 },
    { NULL, NULL }
};

enum VppFpPrecision {
    VPP_FP_PRECISION_UNKNOWN = -1,

    VPP_FP_PRECISION_AUTO = 0,
    VPP_FP_PRECISION_FP32,
    VPP_FP_PRECISION_FP16,

    VPP_FP_PRECISION_MAX,
};

const CX_DESC list_vpp_fp_prec[] = {
    { _T("auto"), VPP_FP_PRECISION_AUTO },
    { _T("fp32"), VPP_FP_PRECISION_FP32 },
    { _T("fp16"), VPP_FP_PRECISION_FP16 },
    { NULL, 0 }
};

enum VppNnediField {
    VPP_NNEDI_FIELD_UNKNOWN = 0,
    VPP_NNEDI_FIELD_BOB_AUTO,
    VPP_NNEDI_FIELD_USE_AUTO,
    VPP_NNEDI_FIELD_USE_TOP,
    VPP_NNEDI_FIELD_USE_BOTTOM,
    VPP_NNEDI_FIELD_BOB_TOP_BOTTOM,
    VPP_NNEDI_FIELD_BOB_BOTTOM_TOP,

    VPP_NNEDI_FIELD_MAX,
};

const CX_DESC list_vpp_nnedi_field[] = {
    { _T("bob"),     VPP_NNEDI_FIELD_BOB_AUTO },
    { _T("auto"),    VPP_NNEDI_FIELD_USE_AUTO },
    { _T("top"),     VPP_NNEDI_FIELD_USE_TOP },
    { _T("bottom"),  VPP_NNEDI_FIELD_USE_BOTTOM },
    { _T("bob_tff"), VPP_NNEDI_FIELD_BOB_TOP_BOTTOM },
    { _T("bob_bff"), VPP_NNEDI_FIELD_BOB_BOTTOM_TOP },
    { NULL, 0 }
};

const CX_DESC list_vpp_nnedi_nns[] = {
    { _T("16"),   16 },
    { _T("32"),   32 },
    { _T("64"),   64 },
    { _T("128"), 128 },
    { _T("256"), 256 },
    { NULL, 0 }
};

enum VppNnediNSize {
    VPP_NNEDI_NSIZE_UNKNOWN = -1,

    VPP_NNEDI_NSIZE_8x6 = 0,
    VPP_NNEDI_NSIZE_16x6,
    VPP_NNEDI_NSIZE_32x6,
    VPP_NNEDI_NSIZE_48x6,
    VPP_NNEDI_NSIZE_8x4,
    VPP_NNEDI_NSIZE_16x4,
    VPP_NNEDI_NSIZE_32x4,

    VPP_NNEDI_NSIZE_MAX,
};

const CX_DESC list_vpp_nnedi_nsize[] = {
    { _T("8x6"),  VPP_NNEDI_NSIZE_8x6  },
    { _T("16x6"), VPP_NNEDI_NSIZE_16x6 },
    { _T("32x6"), VPP_NNEDI_NSIZE_32x6 },
    { _T("48x6"), VPP_NNEDI_NSIZE_48x6 },
    { _T("8x4"),  VPP_NNEDI_NSIZE_8x4  },
    { _T("16x4"), VPP_NNEDI_NSIZE_16x4 },
    { _T("32x4"), VPP_NNEDI_NSIZE_32x4 },
    { NULL, 0 }
};

enum VppNnediQuality {
    VPP_NNEDI_QUALITY_UNKNOWN = 0,
    VPP_NNEDI_QUALITY_FAST,
    VPP_NNEDI_QUALITY_SLOW,

    VPP_NNEDI_QUALITY_MAX,
};

const CX_DESC list_vpp_nnedi_quality[] = {
    { _T("fast"), VPP_NNEDI_QUALITY_FAST },
    { _T("slow"), VPP_NNEDI_QUALITY_SLOW },
    { NULL, 0 }
};

enum VppNnediPreScreen : uint32_t {
    VPP_NNEDI_PRE_SCREEN_NONE            = 0x00,
    VPP_NNEDI_PRE_SCREEN_ORIGINAL        = 0x01,
    VPP_NNEDI_PRE_SCREEN_NEW             = 0x02,
    VPP_NNEDI_PRE_SCREEN_MODE            = 0x07,
    VPP_NNEDI_PRE_SCREEN_BLOCK           = 0x10,
    VPP_NNEDI_PRE_SCREEN_ONLY            = 0x20,
    VPP_NNEDI_PRE_SCREEN_ORIGINAL_BLOCK  = VPP_NNEDI_PRE_SCREEN_ORIGINAL | VPP_NNEDI_PRE_SCREEN_BLOCK,
    VPP_NNEDI_PRE_SCREEN_NEW_BLOCK       = VPP_NNEDI_PRE_SCREEN_NEW      | VPP_NNEDI_PRE_SCREEN_BLOCK,
    VPP_NNEDI_PRE_SCREEN_ORIGINAL_ONLY   = VPP_NNEDI_PRE_SCREEN_ORIGINAL | VPP_NNEDI_PRE_SCREEN_ONLY,
    VPP_NNEDI_PRE_SCREEN_NEW_ONLY        = VPP_NNEDI_PRE_SCREEN_NEW      | VPP_NNEDI_PRE_SCREEN_ONLY,

    VPP_NNEDI_PRE_SCREEN_MAX,
};

static VppNnediPreScreen operator|(VppNnediPreScreen a, VppNnediPreScreen b) {
    return (VppNnediPreScreen)((uint32_t)a | (uint32_t)b);
}

static VppNnediPreScreen operator|=(VppNnediPreScreen& a, VppNnediPreScreen b) {
    a = a | b;
    return a;
}

static VppNnediPreScreen operator&(VppNnediPreScreen a, VppNnediPreScreen b) {
    return (VppNnediPreScreen)((uint32_t)a & (uint32_t)b);
}

static VppNnediPreScreen operator&=(VppNnediPreScreen& a, VppNnediPreScreen b) {
    a = (VppNnediPreScreen)((uint32_t)a & (uint32_t)b);
    return a;
}

const CX_DESC list_vpp_nnedi_pre_screen[] = {
    { _T("none"),           VPP_NNEDI_PRE_SCREEN_NONE },
    { _T("original"),       VPP_NNEDI_PRE_SCREEN_ORIGINAL },
    { _T("new"),            VPP_NNEDI_PRE_SCREEN_NEW },
    { _T("original_block"), VPP_NNEDI_PRE_SCREEN_ORIGINAL_BLOCK },
    { _T("new_block"),      VPP_NNEDI_PRE_SCREEN_NEW_BLOCK },
    { _T("original_only"),  VPP_NNEDI_PRE_SCREEN_ORIGINAL_ONLY },
    { _T("new_only"),       VPP_NNEDI_PRE_SCREEN_NEW_ONLY },
    { NULL, 0 }
};

enum VppNnediErrorType {
    VPP_NNEDI_ETYPE_ABS = 0,
    VPP_NNEDI_ETYPE_SQUARE,

    VPP_NNEDI_ETYPE_MAX,
};

const CX_DESC list_vpp_nnedi_error_type[] = {
    { _T("abs"),    VPP_NNEDI_ETYPE_ABS },
    { _T("square"), VPP_NNEDI_ETYPE_SQUARE },
    { NULL, 0 }
};

const CX_DESC list_vpp_deband[] = {
    { _T("0 - 1点参照"),  0 },
    { _T("1 - 2点参照"),  1 },
    { _T("2 - 4点参照"),  2 },
    { NULL, 0 }
};

const CX_DESC list_vpp_rotate[] = {
    { _T("90"),   90 },
    { _T("180"), 180 },
    { _T("270"), 270 },
    { NULL, 0 }
};

const CX_DESC list_vce_quality_preset_h264[] = {
    { _T("balanced"), AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED },
    { _T("fast"),     AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED },
    { _T("slow"),     AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY },
    { NULL, NULL }
};

const CX_DESC list_vce_quality_preset_hevc[] = {
    { _T("balanced"), AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_BALANCED },
    { _T("fast"),     AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_SPEED },
    { _T("slow"),     AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_QUALITY },
    { NULL, NULL }
};

static const CX_DESC *get_quality_preset(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_HEVC: return list_vce_quality_preset_hevc;
    case RGY_CODEC_H264: return list_vce_quality_preset_h264;
    default:             return nullptr;
    }
}
static const CX_DESC *get_rc_method(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_HEVC: return list_vce_hevc_rc_method;
    case RGY_CODEC_H264: return list_vce_h264_rc_method;
    default:             return list_empty;
    }
}

static inline const CX_DESC *get_level_list(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_H264:    return list_avc_level;
    case RGY_CODEC_HEVC:    return list_hevc_level;
    default:                return list_empty;
    }
}

static inline const CX_DESC *get_profile_list(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_H264:    return list_avc_profile;
    case RGY_CODEC_HEVC:    return list_hevc_profile;
    default:                return list_empty;
    }
}

static inline const CX_DESC *get_tier_list(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_HEVC:    return list_hevc_tier;
    default:                return list_empty;
    }
}

static inline int get_codec_cqp(RGY_CODEC codec) {
    int codec_cqp = 0;
    switch (codec) {
    case RGY_CODEC_HEVC: codec_cqp = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CONSTANT_QP; break;
    case RGY_CODEC_H264: codec_cqp = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP; break;
    default:
        fprintf(stderr, "Unsupported codec!\n");
        abort();
    }
    return codec_cqp;
}

static inline int get_codec_vbr(RGY_CODEC codec) {
    int codec_vbr = 0;
    switch (codec) {
    case RGY_CODEC_HEVC: codec_vbr = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR; break;
    case RGY_CODEC_H264: codec_vbr = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR; break;
    default:
        fprintf(stderr, "Unsupported codec!\n");
        abort();
    }
    return codec_vbr;
}

static inline int get_codec_vbr_lat(RGY_CODEC codec) {
    int codec_vbr = 0;
    switch (codec) {
    case RGY_CODEC_HEVC: codec_vbr = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR; break;
    case RGY_CODEC_H264: codec_vbr = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR; break;
    default:
        fprintf(stderr, "Unsupported codec!\n");
        abort();
    }
    return codec_vbr;
}

static inline int get_codec_cbr(RGY_CODEC codec) {
    int codec_cbr = 0;
    switch (codec) {
    case RGY_CODEC_HEVC: codec_cbr = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CBR; break;
    case RGY_CODEC_H264: codec_cbr = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR; break;
    default:
        fprintf(stderr, "Unsupported codec!\n");
        abort();
    }
    return codec_cbr;
}

struct VCECodecParam {
    int nProfile;
    int nTier;
    int nLevel;
    int nReserved;
};

enum {
    AFS_PRESET_DEFAULT = 0,
    AFS_PRESET_TRIPLE,        //動き重視
    AFS_PRESET_DOUBLE,        //二重化
    AFS_PRESET_ANIME,                     //映画/アニメ
    AFS_PRESET_CINEMA = AFS_PRESET_ANIME, //映画/アニメ
    AFS_PRESET_MIN_AFTERIMG,              //残像最小化
    AFS_PRESET_FORCE24_SD,                //24fps固定
    AFS_PRESET_FORCE24_HD,                //24fps固定 (HD)
    AFS_PRESET_FORCE30,                   //30fps固定
};

const CX_DESC list_afs_preset[] = {
    { _T("default"),      AFS_PRESET_DEFAULT },
    { _T("triple"),       AFS_PRESET_TRIPLE },
    { _T("double"),       AFS_PRESET_DOUBLE },
    { _T("anime/cinema"), AFS_PRESET_ANIME },
    { _T("anime"),        AFS_PRESET_ANIME },
    { _T("cinema"),       AFS_PRESET_CINEMA },
    { _T("min_afterimg"), AFS_PRESET_MIN_AFTERIMG },
    { _T("24fps"),        AFS_PRESET_FORCE24_HD },
    { _T("24fps_sd"),     AFS_PRESET_FORCE24_SD },
    { _T("30fps"),        AFS_PRESET_FORCE30 },
    { NULL, NULL }
};

typedef struct {
    int top, bottom, left, right;
} AFS_SCAN_CLIP;

static inline AFS_SCAN_CLIP scan_clip(int top, int bottom, int left, int right) {
    AFS_SCAN_CLIP clip;
    clip.top = top;
    clip.bottom = bottom;
    clip.left = left;
    clip.right = right;
    return clip;
}

struct VppAfs {
    bool enable;
    int tb_order;
    AFS_SCAN_CLIP clip;    //上下左右
    int method_switch;     //切替点
    int coeff_shift;       //判定比
    int thre_shift;        //縞(ｼﾌﾄ)
    int thre_deint;        //縞(解除)
    int thre_Ymotion;      //Y動き
    int thre_Cmotion;      //C動き
    int analyze;           //解除Lv
    bool shift;            //フィールドシフト
    bool drop;             //間引き
    bool smooth;           //スムージング
    bool force24;          //24fps化
    bool tune;             //調整モード
    bool rff;              //rffフラグを認識して調整
    bool timecode;         //timecode出力
    bool log;              //log出力

    VppAfs();
    void set_preset(int preset);
    int read_afs_inifile(const TCHAR *inifile);
    bool operator==(const VppAfs &x) const;
    bool operator!=(const VppAfs &x) const;
    tstring print() const;

    void check();
};

struct VppNnedi {
    bool              enable;
    VppNnediField     field;
    int               nns;
    VppNnediNSize     nsize;
    VppNnediQuality   quality;
    VppFpPrecision precision;
    VppNnediPreScreen pre_screen;
    VppNnediErrorType errortype;
    tstring           weightfile;

    bool isbob();
    VppNnedi();
    bool operator==(const VppNnedi &x) const;
    bool operator!=(const VppNnedi &x) const;
    tstring print() const;
};

const CX_DESC list_vpp_decimate_block[] = {
    { _T("4"),    4 },
    { _T("8"),    8 },
    { _T("16"),  16 },
    { _T("32"),  32 },
    { _T("64"),  64 },
    { NULL, 0 }
};

struct VppDecimate {
    bool enable;
    int cycle;
    float threDuplicate;
    float threSceneChange;
    int blockX;
    int blockY;
    bool preProcessed;
    bool chroma;
    bool log;

    VppDecimate();
    bool operator==(const VppDecimate &x) const;
    bool operator!=(const VppDecimate &x) const;
    tstring print() const;
};

struct VppPad {
    bool enable;
    int left, top, right, bottom;

    VppPad();
    bool operator==(const VppPad &x) const;
    bool operator!=(const VppPad &x) const;
    tstring print() const;
};

struct VppKnn {
    bool  enable;
    int   radius;
    float strength;
    float lerpC;
    float weight_threshold;
    float lerp_threshold;

    VppKnn();
    bool operator==(const VppKnn &x) const;
    bool operator!=(const VppKnn &x) const;
    tstring print() const;
};

struct VppPmd {
    bool  enable;
    float strength;
    float threshold;
    int   applyCount;
    bool  useExp;

    VppPmd();
    bool operator==(const VppPmd &x) const;
    bool operator!=(const VppPmd &x) const;
    tstring print() const;
};

struct VppSmooth {
    bool enable;
    int quality;
    int qp;
    VppFpPrecision prec;
    bool useQPTable;
    float strength;
    float threshold;
    float bratio;
    int maxQPTableErrCount;
    VppSmooth();
    bool operator==(const VppSmooth &x) const;
    bool operator!=(const VppSmooth &x) const;
    tstring print() const;
};

struct VppUnsharp {
    bool  enable;
    int   radius;
    float weight;
    float threshold;

    VppUnsharp();
    bool operator==(const VppUnsharp &x) const;
    bool operator!=(const VppUnsharp &x) const;
    tstring print() const;
};

struct VppEdgelevel {
    bool  enable;
    float strength;
    float threshold;
    float black;
    float white;

    VppEdgelevel();
    bool operator==(const VppEdgelevel &x) const;
    bool operator!=(const VppEdgelevel &x) const;
    tstring print() const;
};

struct VppTweak {
    bool  enable;
    float brightness; // -1.0 - 1.0 (0.0)
    float contrast;   // -2.0 - 2.0 (1.0)
    float gamma;      //  0.1 - 10.0 (1.0)
    float saturation; //  0.0 - 3.0 (1.0)
    float hue;        // -180 - 180 (0.0)

    VppTweak();
    bool operator==(const VppTweak &x) const;
    bool operator!=(const VppTweak &x) const;
    tstring print() const;
};

struct VppTransform {
    bool enable;
    bool transpose;
    bool flipX;
    bool flipY;

    VppTransform();
    int rotate() const;
    bool setRotate(int rotate);
    bool operator==(const VppTransform &x) const;
    bool operator!=(const VppTransform &x) const;
    tstring print() const;
};

struct VppDeband {
    bool enable;
    int range;
    int threY;
    int threCb;
    int threCr;
    int ditherY;
    int ditherC;
    int sample;
    int seed;
    bool blurFirst;
    bool randEachFrame;

    VppDeband();
    bool operator==(const VppDeband &x) const;
    bool operator!=(const VppDeband &x) const;
    tstring print() const;
};

struct VCEVppParam {
    RGY_VPP_RESIZE_ALGO resize;
    VppAfs afs;
    VppNnedi nnedi;
    VppDecimate decimate;
    VppPad pad;
    VppKnn knn;
    VppPmd pmd;
    VppSmooth smooth;
    VppUnsharp unsharp;
    VppEdgelevel edgelevel;
    VppTweak tweak;
    VppTransform transform;
    VppDeband deband;

    VCEVppParam();
};

static const int AMF_PA_INITQPSC_AUTO = -1;

struct VCEParamPA {
    bool enable;
    bool sc;
    AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY_ENUM scSensitivity;
    bool ss;
    AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY_ENUM ssSensitivity;
    AMF_PA_ACTIVITY_TYPE_ENUM activityType;
    int initQPSC;
    int maxQPBeforeForceSkip;
    AMF_PA_CAQ_STRENGTH_ENUM CAQStrength;
    VCEParamPA();
};

struct VCEParam {
    VideoInfo input;              //入力する動画の情報
    RGYParamCommon common;
    RGYParamControl ctrl;

    RGY_CODEC codec;
    VCECodecParam codecParam[RGY_CODEC_NUM];

    int     deviceID;

    bool    interopD3d9;
    bool    interopD3d11;

    int     par[2];

    int     usage;
    int     rateControl;
    int     qualityPreset;
    int     nBitrate;
    int     nMaxBitrate;
    int     nVBVBufferSize;
    int     nInitialVBVPercent;
    int     nSlices;
    int     nMaxLTR;
    bool    bTimerPeriodTuning;

    bool    bDeblockFilter;
    bool    bEnableSkipFrame;
    int     nQPI;
    int     nQPP;
    int     nQPB;
    int     nQPMax;
    int     nQPMin;
    int     nDeltaQPBFrame;
    int     nDeltaQPBFrameRef;
    int     nBframes;
    bool    bBPyramid;
    int     nMotionEst;
    bool    bEnforceHRD;
    int     nIDRPeriod;
    int     nGOPLen;

    int nRefFrames;
    int nLTRFrames;
    bool bFiller;

    VCEParamPA pa;
    bool       pe;

    bool        bVBAQ;

    bool        ssim;
    bool        psnr;

    VCEVppParam vpp;

    VCEParam();
    ~VCEParam();
};

static const int VCE_DEFAULT_QPI = 22;
static const int VCE_DEFAULT_QPP = 24;
static const int VCE_DEFAULT_QPB = 27;
static const int VCE_DEFAULT_BFRAMES = 0;
static const int VCE_DEFAULT_REF_FRAMES = 2;
static const int VCE_DEFAULT_LTR_FRAMES = 0;
static const int VCE_DEFAULT_MAX_BITRATE = 20000;
static const int VCE_DEFAULT_VBV_BUFSIZE = 20000;
static const int VCE_DEFAULT_SLICES = 1;

static const int VCE_MAX_BFRAMES = 3;
static const int VCE_MAX_GOP_LEN = 1000;
static const int VCE_MAX_B_DELTA_QP = 10;

static const int VCE_OUTPUT_BUF_MB_MAX = 128;

static const int VCE_DEFAULT_AUDIO_IGNORE_DECODE_ERROR = 10;

#define AMF_PARAM(x) \
static const wchar_t *AMF_PARAM_##x(RGY_CODEC codec) { \
    switch (codec) { \
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_##x; \
    case RGY_CODEC_H264: \
    default:             return AMF_VIDEO_ENCODER_##x; \
    } \
};

AMF_PARAM(FRAMESIZE);
AMF_PARAM(FRAMERATE);
AMF_PARAM(USAGE);
AMF_PARAM(PROFILE);
AMF_PARAM(PROFILE_LEVEL);
AMF_PARAM(MAX_LTR_FRAMES);
AMF_PARAM(MAX_NUM_REFRAMES);
AMF_PARAM(QUALITY_PRESET);
AMF_PARAM(ASPECT_RATIO);
AMF_PARAM(SLICES_PER_FRAME);
AMF_PARAM(RATE_CONTROL_METHOD);
AMF_PARAM(VBV_BUFFER_SIZE);
AMF_PARAM(INITIAL_VBV_BUFFER_FULLNESS);
AMF_PARAM(PREENCODE_ENABLE);
AMF_PARAM(PRE_ANALYSIS_ENABLE);
AMF_PARAM(ENABLE_VBAQ);
AMF_PARAM(ENFORCE_HRD);
AMF_PARAM(TARGET_BITRATE);
AMF_PARAM(FILLER_DATA_ENABLE);
AMF_PARAM(LOWLATENCY_MODE);
AMF_PARAM(PEAK_BITRATE);
AMF_PARAM(MAX_AU_SIZE);
AMF_PARAM(QP_I);
AMF_PARAM(QP_P);
AMF_PARAM(RATE_CONTROL_SKIP_FRAME_ENABLE);
AMF_PARAM(MOTION_HALF_PIXEL);
AMF_PARAM(MOTION_QUARTERPIXEL);
AMF_PARAM(END_OF_SEQUENCE);
AMF_PARAM(FORCE_PICTURE_TYPE);
AMF_PARAM(INSERT_AUD);
AMF_PARAM(CAP_MAX_PROFILE);
AMF_PARAM(CAP_MAX_LEVEL);
AMF_PARAM(CAP_NUM_OF_STREAMS);
AMF_PARAM(CAP_MAX_BITRATE);
AMF_PARAM(CAP_MIN_REFERENCE_FRAMES);
AMF_PARAM(CAP_MAX_REFERENCE_FRAMES);
AMF_PARAM(CAP_PRE_ANALYSIS);
static const wchar_t *AMF_PARAM_GOP_SIZE(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_GOP_SIZE;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_IDR_PERIOD;
    }
}
static const wchar_t *AMF_PARAM_MIN_QP(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_MIN_QP_I;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_MIN_QP;
    }
}
static const wchar_t *AMF_PARAM_MAX_QP(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_MAX_QP_I;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_MAX_QP;
    }
}


enum AMFParamType {
    AMF_PARAM_UNKNOWN = -1,
    AMF_PARAM_COMMON = 0,
    AMF_PARAM_ENCODER_USAGE,   // sets to encoder first
    AMF_PARAM_STATIC,          // sets to encoder before initialization
    AMF_PARAM_DYNAMIC,         // sets to encoder at any time
    AMF_PARAM_FRAME,           // sets to frame before frame submission
};

#define SETFRAMEPARAMFREQ_PARAM_NAME L"SETFRAMEPARAMFREQ"
#define SETDYNAMICPARAMFREQ_PARAM_NAME L"SETDYNAMICPARAMFREQ"

struct AMFParam {
    AMFParamType type;
    std::wstring desc;
    amf::AMFVariant value;

    AMFParam() : type(AMF_PARAM_UNKNOWN), desc(), value() {};
    AMFParam(AMFParamType type_, const std::wstring &desc_, amf::AMFVariant value_)
        : type(type_), desc(desc_), value(value_) { }
};

class AMFParams {
public:
    AMFParams() {};
    virtual ~AMFParams() {};

    RGY_ERR SetParamType(const std::wstring &name, AMFParamType type, const std::wstring &desc);
    RGY_ERR SetParamTypeAVC();
    RGY_ERR SetParamTypeHEVC();

    RGY_ERR SetParam(const std::wstring &name, amf::AMFVariantStruct value);

    RGY_ERR Apply(amf::AMFPropertyStorage *storage, AMFParamType type, RGYLog *pLog = nullptr);

    template<typename _T> inline
    RGY_ERR SetParam(const std::wstring &name, const _T &value) {
        return SetParam(name, static_cast<const amf::AMFVariantStruct &>(amf::AMFVariant(value)));
    }

    RGY_ERR GetParam(const std::wstring &name, amf::AMFVariantStruct *value) const;

    template<typename T> inline
    T get(const std::wstring &name) const {
        T t = T();
        GetParam(name, t);
        return t;
    }

    template<typename _T> inline
    RGY_ERR GetParam(const std::wstring &name, _T &value) const {
        amf::AMFVariant var;
        auto err = GetParam(name, static_cast<amf::AMFVariantStruct *>(&var));
        if (err == RGY_ERR_NONE) {
            value = static_cast<_T>(var);
        }
        return err;
    }
    template<typename _T> inline
    RGY_ERR GetParamWString(const std::wstring &name, _T &value) const {
        amf::AMFVariant var;
        auto err = GetParam(name, static_cast<amf::AMFVariantStruct *>(&var));
        if (err == RGY_ERR_NONE) {
            value = var.ToWString().c_str();
        }
        return err;
    }
protected:
    std::map<std::wstring, AMFParam> m_params;
};

#pragma warning(pop)
