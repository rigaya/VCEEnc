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
#include <optional>
#include "rgy_util.h"
#pragma warning(push)
#pragma warning(disable:4201)
#pragma warning(disable:4100)
RGY_DISABLE_WARNING_PUSH
RGY_DISABLE_WARNING_STR("-Wclass-memaccess")
#include "VideoEncoderVCE.h"
#include "VideoEncoderHEVC.h"
#include "VideoEncoderAV1.h"
RGY_DISABLE_WARNING_POP
#include "HQScaler.h"
#include "PreProcessing.h"
#include "VQEnhancer.h"
#include "rgy_caption.h"
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

static const wchar_t* RGY_PROP_TIMESTAMP = L"RGYPropTimestamp";
static const wchar_t* RGY_PROP_DURATION = L"RGYPropDuration";
static const wchar_t* RGY_PROP_INPUT_FRAMEID = L"RGYPropInputFrameID";

static const TCHAR *VCEENCC_ABORT_EVENT = _T("VCEEncC_abort_%u");

static const int VCE_FILTER_PP_STRENGTH_DEFAULT = 4;
static const int VCE_FILTER_PP_SENSITIVITY_DEFAULT = 4;
static const bool VCE_FILTER_PP_ADAPT_FILTER_DEFAULT = false;
static const int VCE_FILTER_VQENHANCER_RADIUS_DEFAULT = 2;

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
    { _T("av1"),  RGY_CODEC_AV1  },
    { NULL, 0 }
};
const CX_DESC list_codec_all[] = {
    { _T("h264"), RGY_CODEC_H264 },
    { _T("avc"),  RGY_CODEC_H264 },
    { _T("h265"), RGY_CODEC_HEVC },
    { _T("hevc"), RGY_CODEC_HEVC },
    { _T("av1"),  RGY_CODEC_AV1  },
    { NULL, 0 }
};

const CX_DESC list_avc_profile[] = {
    { _T("Baseline"), AMF_VIDEO_ENCODER_PROFILE_BASELINE },
    { _T("Main"),     AMF_VIDEO_ENCODER_PROFILE_MAIN     },
    { _T("High"),     AMF_VIDEO_ENCODER_PROFILE_HIGH     },
    { NULL, 0 }
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
    { NULL, 0 }
};

const CX_DESC list_hevc_profile[] = {
    { _T("main"),     AMF_VIDEO_ENCODER_HEVC_PROFILE_MAIN },
    { _T("main10"),   AMF_VIDEO_ENCODER_HEVC_PROFILE_MAIN_10 },
    { NULL, 0 }
};

const CX_DESC list_hevc_tier[] = {
    { _T("main"),     AMF_VIDEO_ENCODER_HEVC_TIER_MAIN },
    { _T("high"),     AMF_VIDEO_ENCODER_HEVC_TIER_HIGH },
    { NULL, 0 }
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
    { NULL, 0 }
};
const CX_DESC list_hevc_bitdepth[] = {
    { _T("8bit"),   8   },
    { _T("10bit"), 10  },
    { NULL, 0 }
};
const CX_DESC list_av1_bitdepth[] = {
    { _T("8bit"),   8   },
    { _T("10bit"), 10  },
    { NULL, 0 }
};

const CX_DESC list_av1_profile[] = {
    { _T("main"),     AMF_VIDEO_ENCODER_AV1_PROFILE_MAIN },
    { NULL, 0 }
};

const CX_DESC list_av1_tier[] = {
    { _T("main"),     AMF_VIDEO_ENCODER_HEVC_TIER_MAIN },
    { _T("high"),     AMF_VIDEO_ENCODER_HEVC_TIER_HIGH },
    { NULL, 0 }
};

const CX_DESC list_av1_level[] = {
    { _T("auto"), -1 },
    { _T("2"),   AMF_VIDEO_ENCODER_AV1_LEVEL_2_0 },
    { _T("2.1"), AMF_VIDEO_ENCODER_AV1_LEVEL_2_1 },
    { _T("2.2"), AMF_VIDEO_ENCODER_AV1_LEVEL_2_2 },
    { _T("2.3"), AMF_VIDEO_ENCODER_AV1_LEVEL_2_3 },
    { _T("3"),   AMF_VIDEO_ENCODER_AV1_LEVEL_3_0 },
    { _T("3.1"), AMF_VIDEO_ENCODER_AV1_LEVEL_3_1 },
    { _T("3.2"), AMF_VIDEO_ENCODER_AV1_LEVEL_3_2 },
    { _T("3.3"), AMF_VIDEO_ENCODER_AV1_LEVEL_3_3 },
    { _T("4"),   AMF_VIDEO_ENCODER_AV1_LEVEL_4_0 },
    { _T("4.1"), AMF_VIDEO_ENCODER_AV1_LEVEL_4_1 },
    { _T("4.2"), AMF_VIDEO_ENCODER_AV1_LEVEL_4_2 },
    { _T("4.3"), AMF_VIDEO_ENCODER_AV1_LEVEL_4_3 },
    { _T("5"),   AMF_VIDEO_ENCODER_AV1_LEVEL_5_0 },
    { _T("5.1"), AMF_VIDEO_ENCODER_AV1_LEVEL_5_1 },
    { _T("5.2"), AMF_VIDEO_ENCODER_AV1_LEVEL_5_2 },
    { _T("5.3"), AMF_VIDEO_ENCODER_AV1_LEVEL_5_3 },
    { _T("6"),   AMF_VIDEO_ENCODER_AV1_LEVEL_6_0 },
    { _T("6.1"), AMF_VIDEO_ENCODER_AV1_LEVEL_6_1 },
    { _T("6.2"), AMF_VIDEO_ENCODER_AV1_LEVEL_6_2 },
    { _T("6.3"), AMF_VIDEO_ENCODER_AV1_LEVEL_6_3 },
    { _T("7"),   AMF_VIDEO_ENCODER_AV1_LEVEL_7_0 },
    { _T("7.1"), AMF_VIDEO_ENCODER_AV1_LEVEL_7_1 },
    { _T("7.2"), AMF_VIDEO_ENCODER_AV1_LEVEL_7_2 },
    { _T("7.3"), AMF_VIDEO_ENCODER_AV1_LEVEL_7_3 },
    { NULL, 0 }
};

const CX_DESC list_mv_presicion[] = {
    { _T("full-pel"), VCE_MOTION_EST_FULL   },
    { _T("half-pel"), VCE_MOTION_EST_HALF   },
    { _T("Q-pel"),    VCE_MOTION_EST_QUATER | VCE_MOTION_EST_HALF },
    { NULL, 0 }
};

const CX_DESC list_vce_h264_rc_method[] = {
    { _T("CQP"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP          },
    { _T("CBR"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR                  },
    { _T("VBR"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR },
    { _T("VBR_LAT"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR },
    { _T("QVBR"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_QUALITY_VBR },
    { _T("VBR-HQ"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_HIGH_QUALITY_VBR },
    { _T("CBR-HQ"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_HIGH_QUALITY_CBR },
    { NULL, 0 }
};

const CX_DESC list_vce_hevc_rc_method[] = {
    { _T("CQP"), AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CONSTANT_QP },
    { _T("CBR"), AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CBR },
    { _T("VBR"), AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR },
    { _T("VBR_LAT"), AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR },
    { _T("QVBR"), AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_QUALITY_VBR },
    { _T("VBR-HQ"), AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_HIGH_QUALITY_VBR },
    { _T("CBR-HQ"), AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_HIGH_QUALITY_CBR },
    { NULL, 0 }
};

const CX_DESC list_vce_av1_rc_method[] = {
    { _T("CQP"), AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD_CONSTANT_QP },
    { _T("CBR"), AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD_CBR },
    { _T("VBR"), AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR },
    { _T("VBR_LAT"), AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR },
    { _T("QVBR"), AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD_QUALITY_VBR },
    { _T("VBR-HQ"), AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD_HIGH_QUALITY_VBR },
    { _T("CBR-HQ"), AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD_HIGH_QUALITY_CBR },
    { NULL, 0 }
};

const CX_DESC list_vce_preanalysis_h264[] = {
    { _T("off"), AMF_VIDEO_ENCODER_PREENCODE_DISABLED },
    { _T("on"),  AMF_VIDEO_ENCODER_PREENCODE_ENABLED },
    { NULL, 0 }
};

const CX_DESC list_av1_cdef_mode[] = {
    { _T("off"), AMF_VIDEO_ENCODER_AV1_CDEF_DISABLE },
    { _T("on"),  AMF_VIDEO_ENCODER_AV1_CDEF_ENABLE_DEFAULT  },
    { NULL, 0 }
};

const CX_DESC list_av1_aq_mode[] = {
    { _T("none"), AMF_VIDEO_ENCODER_AV1_AQ_MODE_NONE },
    { _T("caq"),  AMF_VIDEO_ENCODER_AV1_AQ_MODE_CAQ  },
    { NULL, 0 }
};

#define AMF_PA_SCENE_CHANGE_DETECTION_NONE (AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY_LOW-1)
#define AMF_PA_STATIC_SCENE_DETECTION_NONE (AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY_LOW-1)

const CX_DESC list_pa_sc_sensitivity[] = {
    { _T("none"),   AMF_PA_SCENE_CHANGE_DETECTION_NONE },
    { _T("low"),    AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY_LOW },
    { _T("medium"), AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY_MEDIUM },
    { _T("high"),   AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY_HIGH },
    { NULL, 0 }
};

const CX_DESC list_pa_ss_sensitivity[] = {
    { _T("none"),   AMF_PA_STATIC_SCENE_DETECTION_NONE },
    { _T("low"),    AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY_LOW },
    { _T("medium"), AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY_MEDIUM },
    { _T("high"),   AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY_HIGH },
    { NULL, 0 }
};

const CX_DESC list_pa_activity[] = {
    { _T("y"),   AMF_PA_ACTIVITY_Y },
    { _T("yuv"), AMF_PA_ACTIVITY_YUV },
    { NULL, 0 }
};

const CX_DESC list_pa_caq_strength[] = {
    { _T("low"),    AMF_PA_CAQ_STRENGTH_LOW },
    { _T("medium"), AMF_PA_CAQ_STRENGTH_MEDIUM },
    { _T("high"),   AMF_PA_CAQ_STRENGTH_HIGH },
    { NULL, 0 }
};

const CX_DESC list_pa_paq_mode[] = {
    { _T("none"), AMF_PA_PAQ_MODE_NONE },
    { _T("caq"),  AMF_PA_PAQ_MODE_CAQ  },
    { NULL, 0 }
};

const CX_DESC list_pa_taq_mode[] = {
    { _T("0"),  AMF_PA_TAQ_MODE_NONE  },
    { _T("1"),  AMF_PA_TAQ_MODE_1  },
    { _T("2"),  AMF_PA_TAQ_MODE_2  },
#if !FOR_AUO
    { _T("off"), AMF_PA_TAQ_MODE_NONE },
    { _T("on"),  AMF_PA_TAQ_MODE_1  },
#endif
    { NULL, 0 }
};

const CX_DESC list_pa_motion_quality_mode[] = {
    { _T("none"), AMF_PA_HIGH_MOTION_QUALITY_BOOST_MODE_NONE  },
    { _T("auto"), AMF_PA_HIGH_MOTION_QUALITY_BOOST_MODE_AUTO  },
    { NULL, 0 }
};

static const int AMF_VIDEO_ENCODER_QUALITY_PRESET_HIGH_QUALITY = AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY+1;

const CX_DESC list_vce_quality_preset_h264[] = {
    { _T("balanced"), AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED },
    { _T("fast"),     AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED },
    { _T("slow"),     AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY },
    { _T("slower"),   AMF_VIDEO_ENCODER_QUALITY_PRESET_HIGH_QUALITY },
    { NULL, 0 }
};

const CX_DESC list_vce_quality_preset_hevc[] = {
    { _T("balanced"), AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_BALANCED },
    { _T("fast"),     AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_SPEED },
    { _T("slow"),     AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_QUALITY },
    { NULL, 0 }
};

const CX_DESC list_vce_quality_preset_av1[] = {
    { _T("balanced"), AMF_VIDEO_ENCODER_AV1_QUALITY_PRESET_BALANCED },
    { _T("fast"),     AMF_VIDEO_ENCODER_AV1_QUALITY_PRESET_SPEED },
    { _T("slow"),     AMF_VIDEO_ENCODER_AV1_QUALITY_PRESET_QUALITY },
    { _T("slower"),   AMF_VIDEO_ENCODER_AV1_QUALITY_PRESET_HIGH_QUALITY },
    { NULL, 0 }
};

static const CX_DESC *get_quality_preset(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_H264: return list_vce_quality_preset_h264;
    case RGY_CODEC_HEVC: return list_vce_quality_preset_hevc;
    case RGY_CODEC_AV1:  return list_vce_quality_preset_av1;
    default:             return nullptr;
    }
}
static const CX_DESC *get_rc_method(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_H264: return list_vce_h264_rc_method;
    case RGY_CODEC_HEVC: return list_vce_hevc_rc_method;
    case RGY_CODEC_AV1:  return list_vce_av1_rc_method;
    default:             return list_empty;
    }
}

static inline const CX_DESC *get_level_list(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_H264:    return list_avc_level;
    case RGY_CODEC_HEVC:    return list_hevc_level;
    case RGY_CODEC_AV1:     return list_av1_level;
    default:                return list_empty;
    }
}
static int get_level_auto(RGY_CODEC codec) {
    return get_cx_value(get_level_list(codec), _T("auto"));
}

static inline const CX_DESC *get_profile_list(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_H264:    return list_avc_profile;
    case RGY_CODEC_HEVC:    return list_hevc_profile;
    case RGY_CODEC_AV1:     return list_av1_profile;
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
    case RGY_CODEC_H264: codec_cqp = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP; break;
    case RGY_CODEC_HEVC: codec_cqp = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CONSTANT_QP; break;
    case RGY_CODEC_AV1:  codec_cqp = AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD_CONSTANT_QP; break;
    default:
        fprintf(stderr, "Unsupported codec!\n");
        abort();
    }
    return codec_cqp;
}

static inline int get_codec_vbr(RGY_CODEC codec) {
    int codec_vbr = 0;
    switch (codec) {
    case RGY_CODEC_H264: codec_vbr = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR; break;
    case RGY_CODEC_HEVC: codec_vbr = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR; break;
    case RGY_CODEC_AV1:  codec_vbr = AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR; break;
    default:
        fprintf(stderr, "Unsupported codec!\n");
        abort();
    }
    return codec_vbr;
}

static inline int get_codec_hqvbr(RGY_CODEC codec) {
    int codec_vbr = 0;
    switch (codec) {
    case RGY_CODEC_H264: codec_vbr = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_HIGH_QUALITY_VBR; break;
    case RGY_CODEC_HEVC: codec_vbr = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_HIGH_QUALITY_VBR; break;
    case RGY_CODEC_AV1:  codec_vbr = AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD_HIGH_QUALITY_VBR; break;
    default:
        fprintf(stderr, "Unsupported codec!\n");
        abort();
    }
    return codec_vbr;
}

static inline int get_codec_vbr_lat(RGY_CODEC codec) {
    int codec_vbr = 0;
    switch (codec) {
    case RGY_CODEC_H264: codec_vbr = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR; break;
    case RGY_CODEC_HEVC: codec_vbr = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR; break;
    case RGY_CODEC_AV1:  codec_vbr = AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR; break;
    default:
        fprintf(stderr, "Unsupported codec!\n");
        abort();
    }
    return codec_vbr;
}

static inline int get_codec_cbr(RGY_CODEC codec) {
    int codec_cbr = 0;
    switch (codec) {
    case RGY_CODEC_H264: codec_cbr = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR; break;
    case RGY_CODEC_HEVC: codec_cbr = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CBR; break;
    case RGY_CODEC_AV1:  codec_cbr = AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD_CBR; break;
    default:
        fprintf(stderr, "Unsupported codec!\n");
        abort();
    }
    return codec_cbr;
}

static inline int get_codec_hqcbr(RGY_CODEC codec) {
    int codec_cbr = 0;
    switch (codec) {
    case RGY_CODEC_H264: codec_cbr = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_HIGH_QUALITY_CBR; break;
    case RGY_CODEC_HEVC: codec_cbr = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_HIGH_QUALITY_CBR; break;
    case RGY_CODEC_AV1:  codec_cbr = AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD_HIGH_QUALITY_CBR; break;
    default:
        fprintf(stderr, "Unsupported codec!\n");
        abort();
    }
    return codec_cbr;
}

static inline int get_codec_qvbr(RGY_CODEC codec) {
    int codec_vbr = 0;
    switch (codec) {
    case RGY_CODEC_H264: codec_vbr = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_QUALITY_VBR; break;
    case RGY_CODEC_HEVC: codec_vbr = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_QUALITY_VBR; break;
    case RGY_CODEC_AV1:  codec_vbr = AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD_QUALITY_VBR; break;
    default:
        fprintf(stderr, "Unsupported codec!\n");
        abort();
    }
    return codec_vbr;
}

struct VCECodecParam {
    int nProfile;
    int nTier;
    int nLevel;
    int nReserved;
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
    bool ltrEnable;
    AMF_PA_CAQ_STRENGTH_ENUM CAQStrength;
    AMF_PA_PAQ_MODE_ENUM PAQMode;
    AMF_PA_TAQ_MODE_ENUM TAQMode;
    AMF_PA_HIGH_MOTION_QUALITY_BOOST_MODE_ENUM motionQualityBoost;
    int lookaheadDepth;

    bool operator==(const VCEParamPA &x) const;
    bool operator!=(const VCEParamPA &x) const;

    VCEParamPA();
};

const CX_DESC list_vce_hq_scaler[] = {
    { _T("bilinear"), AMF_HQ_SCALER_ALGORITHM_BILINEAR },
    { _T("bicubic"),  AMF_HQ_SCALER_ALGORITHM_BICUBIC  },
    { _T("fsr"),      AMF_HQ_SCALER_ALGORITHM_FSR      },
    { _T("point"),    AMF_HQ_SCALER_ALGORITHM_POINT    },
    { NULL, 0 }
};

struct VppAMFHQScaler {
    bool enable;
    AMF_HQ_SCALER_ALGORITHM_ENUM algorithm;
    float sharpness;

    VppAMFHQScaler();
    bool operator==(const VppAMFHQScaler& x) const;
    bool operator!=(const VppAMFHQScaler& x) const;
    tstring print() const;
};

struct VppAMFPreProcessing {
    bool enable;
    int strength;
    int sensitivity;
    bool adaptiveFilter;

    VppAMFPreProcessing();
    bool operator==(const VppAMFPreProcessing& x) const;
    bool operator!=(const VppAMFPreProcessing& x) const;
    tstring print() const;
};

struct VppAMFVQEnhancer {
    bool enable;
    float attenuation;
    int fcrRadius;

    VppAMFVQEnhancer();
    bool operator==(const VppAMFVQEnhancer& x) const;
    bool operator!=(const VppAMFVQEnhancer& x) const;
    tstring print() const;
};

struct VCEFilterParam {
    VppAMFHQScaler scaler;
    VppAMFPreProcessing pp;
    VppAMFVQEnhancer enhancer;

    VCEFilterParam();
    bool operator==(const VCEFilterParam& x) const;
    bool operator!=(const VCEFilterParam& x) const;
};

struct VCEParam {
    VideoInfo input;              //入力する動画の情報
    RGYParamInput inprm;
    RGYParamCommon common;
    RGYParamControl ctrl;
    RGYParamVpp vpp;
    VCEFilterParam vppamf;

    RGY_CODEC codec;
    VCECodecParam codecParam[RGY_CODEC_NUM];

    int     deviceID;

    bool    interopD3d9;
    bool    interopD3d11;
    bool    interopVulkan;

    int     outputDepth;

    int     par[2];

    int     usage;
    int     rateControl;
    int     qualityPreset;
    int     qvbrLevel;
    int     nBitrate;
    int     nMaxBitrate;
    int     nVBVBufferSize;
    int     nInitialVBVPercent;
    int     nSlices;
    int     nMaxLTR;
    bool    bTimerPeriodTuning;

    bool    bDeblockFilter;
    std::optional<bool> enableSkipFrame;
    int     nQPI;
    int     nQPP;
    int     nQPB;
    std::optional<int> nQPMax;
    std::optional<int> nQPMin;
    std::optional<int> nQPMaxInter;
    std::optional<int> nQPMinInter;
    int     nDeltaQPBFrame;
    int     nDeltaQPBFrameRef;
    int     nBframes;
    bool    bBPyramid;
    bool    adaptMiniGOP;
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

    //AV1
    int     tiles;
    std::optional<int> cdefMode;
    std::optional<bool> screenContentTools;
    std::optional<bool> paletteMode;
    std::optional<bool> forceIntegerMV;
    std::optional<bool> cdfUpdate;
    std::optional<bool> cdfFrameEndUpdate;
    std::optional<int>  temporalLayers;
    std::optional<int>  aqMode;

    bool        ssim;
    bool        psnr;

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
    case RGY_CODEC_AV1:  return AMF_VIDEO_ENCODER_AV1_##x; \
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_##x; \
    case RGY_CODEC_H264: \
    default:             return AMF_VIDEO_ENCODER_##x; \
    } \
};

#define AMF_PARAM_H264_AV1(x) \
static const wchar_t *AMF_PARAM_##x(RGY_CODEC codec) { \
    switch (codec) { \
    case RGY_CODEC_AV1:  return AMF_VIDEO_ENCODER_AV1_##x; \
    case RGY_CODEC_H264: \
    default:             return AMF_VIDEO_ENCODER_##x; \
    } \
};

#define AMF_PARAM_H264_HEVC(x) \
static const wchar_t *AMF_PARAM_##x(RGY_CODEC codec) { \
    switch (codec) { \
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_##x; \
    case RGY_CODEC_H264: \
    default:             return AMF_VIDEO_ENCODER_##x; \
    } \
};

#define AMF_PARAM_HEVC_AV1(x) \
static const wchar_t *AMF_PARAM_##x(RGY_CODEC codec) { \
    switch (codec) { \
    case RGY_CODEC_AV1:  return AMF_VIDEO_ENCODER_AV1_##x; \
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_##x; \
    case RGY_CODEC_H264: \
    default: return nullptr; \
    } \
};

AMF_PARAM(FRAMESIZE);
AMF_PARAM(FRAMERATE);
AMF_PARAM(USAGE);
AMF_PARAM(PROFILE);
AMF_PARAM(MAX_LTR_FRAMES);
AMF_PARAM(MAX_NUM_REFRAMES);
AMF_PARAM(QUALITY_PRESET);
AMF_PARAM(RATE_CONTROL_METHOD);
AMF_PARAM(VBV_BUFFER_SIZE);
AMF_PARAM(INITIAL_VBV_BUFFER_FULLNESS);
AMF_PARAM(PRE_ANALYSIS_ENABLE);
AMF_PARAM(ENFORCE_HRD);
AMF_PARAM(TARGET_BITRATE);
AMF_PARAM(PEAK_BITRATE);
AMF_PARAM_H264_HEVC(ASPECT_RATIO);
AMF_PARAM_H264_HEVC(SLICES_PER_FRAME);
AMF_PARAM_H264_HEVC(ENABLE_VBAQ);
AMF_PARAM_H264_HEVC(LOWLATENCY_MODE);
AMF_PARAM_H264_HEVC(MAX_AU_SIZE);
AMF_PARAM_H264_HEVC(MOTION_HALF_PIXEL);
AMF_PARAM_H264_HEVC(MOTION_QUARTERPIXEL);
AMF_PARAM_H264_HEVC(END_OF_SEQUENCE);
AMF_PARAM_H264_HEVC(FORCE_PICTURE_TYPE);
AMF_PARAM_H264_HEVC(INSERT_AUD);
AMF_PARAM_HEVC_AV1(COLOR_BIT_DEPTH);
AMF_PARAM(QVBR_QUALITY_LEVEL);
AMF_PARAM(INPUT_COLOR_PROFILE);
AMF_PARAM(INPUT_TRANSFER_CHARACTERISTIC);
AMF_PARAM(INPUT_COLOR_PRIMARIES);
AMF_PARAM(OUTPUT_COLOR_PROFILE);
AMF_PARAM(OUTPUT_TRANSFER_CHARACTERISTIC);
AMF_PARAM(OUTPUT_COLOR_PRIMARIES);
AMF_PARAM(CAP_MAX_PROFILE);
AMF_PARAM(CAP_MAX_LEVEL);
AMF_PARAM(CAP_MAX_BITRATE);
AMF_PARAM(CAP_PRE_ANALYSIS);
AMF_PARAM(CAP_MAX_THROUGHPUT);
AMF_PARAM_H264_HEVC(CAP_MIN_REFERENCE_FRAMES);
AMF_PARAM_H264_HEVC(CAP_MAX_REFERENCE_FRAMES);
AMF_PARAM_H264_HEVC(CAP_ROI);
AMF_PARAM_H264_HEVC(STATISTICS_FEEDBACK);
AMF_PARAM_H264_HEVC(STATISTIC_FRAME_QP);
AMF_PARAM_H264_HEVC(STATISTIC_AVERAGE_QP);
AMF_PARAM_H264_HEVC(STATISTIC_MAX_QP);
AMF_PARAM_H264_HEVC(STATISTIC_MIN_QP);
AMF_PARAM_H264_HEVC(STATISTIC_PIX_NUM_INTRA);
AMF_PARAM_H264_HEVC(STATISTIC_PIX_NUM_INTER);
AMF_PARAM_H264_HEVC(STATISTIC_PIX_NUM_SKIP);
AMF_PARAM_H264_HEVC(STATISTIC_BITCOUNT_RESIDUAL);
AMF_PARAM_H264_HEVC(STATISTIC_BITCOUNT_MOTION);
AMF_PARAM_H264_HEVC(STATISTIC_BITCOUNT_INTER);
AMF_PARAM_H264_HEVC(STATISTIC_BITCOUNT_INTRA);
AMF_PARAM_H264_HEVC(STATISTIC_BITCOUNT_ALL_MINUS_HEADER);
AMF_PARAM_H264_HEVC(STATISTIC_MV_X);
AMF_PARAM_H264_HEVC(STATISTIC_MV_Y);
static const wchar_t *AMF_PARAM_RATE_CONTROL_SKIP_FRAME_ENABLE(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_AV1:  return AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_SKIP_FRAME;
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_SKIP_FRAME_ENABLE;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE;
    }
}
static const wchar_t *AMF_PARAM_FILLER_DATA_ENABLE(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_AV1:  return AMF_VIDEO_ENCODER_AV1_FILLER_DATA;
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_FILLER_DATA_ENABLE;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE;
    }
}
static const wchar_t *AMF_PARAM_PREENCODE_ENABLE(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_AV1:  return AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_PREENCODE;
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_PREENCODE_ENABLE;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_PREENCODE_ENABLE;
    }
}
static const wchar_t *AMF_PARAM_PROFILE_LEVEL(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_AV1:  return AMF_VIDEO_ENCODER_AV1_LEVEL;
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_PROFILE_LEVEL;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_PROFILE_LEVEL;
    }
}
static const wchar_t *AMF_PARAM_GOP_SIZE(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_AV1:  return AMF_VIDEO_ENCODER_AV1_GOP_SIZE;
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_GOP_SIZE;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_IDR_PERIOD;
    }
}
static const wchar_t *AMF_PARAM_QP_I(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_AV1:  return AMF_VIDEO_ENCODER_AV1_Q_INDEX_INTRA;
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_QP_I;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_QP_I;
    }
}
static const wchar_t *AMF_PARAM_QP_P(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_AV1:  return AMF_VIDEO_ENCODER_AV1_Q_INDEX_INTER;
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_QP_P;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_QP_P;
    }
}
static const wchar_t *AMF_PARAM_MIN_QP(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_AV1:  return AMF_VIDEO_ENCODER_AV1_MIN_Q_INDEX_INTRA;
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_MIN_QP_I;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_MIN_QP;
    }
}
static const wchar_t *AMF_PARAM_MAX_QP(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_AV1:  return AMF_VIDEO_ENCODER_AV1_MAX_Q_INDEX_INTRA;
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_MAX_QP_I;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_MAX_QP;
    }
}
static const wchar_t *AMF_PARAM_MIN_QP_INTER(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_AV1:  return AMF_VIDEO_ENCODER_AV1_MIN_Q_INDEX_INTER;
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_MIN_QP_P;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_MIN_QP;
    }
}
static const wchar_t *AMF_PARAM_MAX_QP_INTER(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_AV1:  return AMF_VIDEO_ENCODER_AV1_MAX_Q_INDEX_INTER;
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_MAX_QP_P;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_MAX_QP;
    }
}
static const wchar_t *AMF_PARAM_OUTPUT_DATA_TYPE(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_AV1:  return AMF_VIDEO_ENCODER_AV1_OUTPUT_FRAME_TYPE;
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_OUTPUT_DATA_TYPE;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE;
    }
}
static const wchar_t *AMF_PARAM_CAPS_QUERY_TIMEOUT_SUPPORT(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_CAPS_HEVC_QUERY_TIMEOUT_SUPPORT;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_CAPS_QUERY_TIMEOUT_SUPPORT;
    }
}
static const wchar_t *AMF_PARAM_CAP_NUM_OF_STREAMS(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_AV1:  return AMF_VIDEO_ENCODER_AV1_CAP_NUM_OF_HW_INSTANCES;
    case RGY_CODEC_HEVC: return AMF_VIDEO_ENCODER_HEVC_CAP_NUM_OF_STREAMS;
    case RGY_CODEC_H264:
    default:             return AMF_VIDEO_ENCODER_CAP_NUM_OF_STREAMS;
    }
}

static int64_t get_encoder_usage(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_AV1:  return (int64_t)AMF_VIDEO_ENCODER_AV1_USAGE_TRANSCODING;
    case RGY_CODEC_HEVC: return (int64_t)AMF_VIDEO_ENCODER_HEVC_USAGE_TRANSCONDING;
    case RGY_CODEC_H264:
    default:             return (int64_t)AMF_VIDEO_ENCODER_USAGE_TRANSCONDING;
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
    RGY_ERR SetParamTypeCodec(const RGY_CODEC codec);
    RGY_ERR SetParamTypeAVC();
    RGY_ERR SetParamTypeHEVC();
    RGY_ERR SetParamTypeAV1();
    RGY_ERR SetParamTypePA();

    RGY_ERR SetParam(const std::wstring &name, amf::AMFVariantStruct value);

    RGY_ERR Apply(amf::AMFPropertyStorage *storage, AMFParamType type, RGYLog *pLog = nullptr);

    template<typename _T> inline
    RGY_ERR SetParam(const std::wstring &name, const _T &value) {
        return SetParam(name, static_cast<const amf::AMFVariantStruct &>(amf::AMFVariant(value)));
    }

    int64_t CountParam(const AMFParamType type) const;
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
