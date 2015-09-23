//  -----------------------------------------------------------------------------------------
//    VCEEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#pragma once
#pragma warning(disable:4201)
#include "VideoEncoderVCE.h"
#include "VCEUtil.h"

enum {
    VCE_RC_CQP = 0,
    VCE_RC_CBR = 3,
    VCE_RC_VBR = 4,
};

enum {
    VCE_INPUT_NONE = 0,
    VCE_INPUT_AUO = 0,
    VCE_INPUT_RAW,
    VCE_INPUT_Y4M,
    VCE_INPUT_AVS,
    VCE_INPUT_VPY,
    VCE_INPUT_VPY_MT
};

enum : uint8_t {
    VCE_MOTION_EST_FULL   = 0x00,
    VCE_MOTION_EST_HALF   = 0x01,
    VCE_MOTION_EST_QUATER = 0x02,
};

typedef struct {
    TCHAR *desc;
    int value;
} CX_DESC;

static const wchar_t *list_codecs[] = {
    AMFVideoEncoderVCE_AVC,
    AMFVideoEncoderVCE_SVC
};

const CX_DESC list_log_level[] = {
    { _T("trace"), VCE_LOG_TRACE },
    { _T("debug"), VCE_LOG_DEBUG },
    { _T("more"),  VCE_LOG_MORE  },
    { _T("info"),  VCE_LOG_INFO  },
    { _T("warn"),  VCE_LOG_WARN  },
    { _T("error"), VCE_LOG_ERROR },
    { NULL, NULL }
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
    { NULL, NULL }
};

const int COLOR_VALUE_AUTO = USHRT_MAX;
const int HD_HEIGHT_THRESHOLD = 720;
const int HD_INDEX = 2;
const int SD_INDEX = 3;
const CX_DESC list_colorprim[] = {
    { _T("undef"),     2  },
    { _T("auto"),      COLOR_VALUE_AUTO },
    { _T("bt709"),     1  },
    { _T("smpte170m"), 6  },
    { _T("bt470m"),    4  },
    { _T("bt470bg"),   5  },
    { _T("smpte240m"), 7  },
    { _T("film"),      8  },
    { NULL, NULL }
};
const CX_DESC list_transfer[] = {
    { _T("undef"),     2  },
    { _T("auto"),      COLOR_VALUE_AUTO },
    { _T("bt709"),     1  },
    { _T("smpte170m"), 6  },
    { _T("bt470m"),    4  },
    { _T("bt470bg"),   5  },
    { _T("smpte240m"), 7  },
    { _T("linear"),    8  },
    { _T("log100"),    9  },
    { _T("log316"),    10 },
    { NULL, NULL }
};
const CX_DESC list_colormatrix[] = {
    { _T("undef"),     2  },
    { _T("auto"),      COLOR_VALUE_AUTO },
    { _T("bt709"),     1  },
    { _T("smpte170m"), 6  },
    { _T("bt470bg"),   5  },
    { _T("smpte240m"), 7  },
    { _T("YCgCo"),     8  },
    { _T("fcc"),       4  },
    { _T("GBR"),       0  },
    { NULL, NULL }
};
const CX_DESC list_videoformat[] = {
    { _T("undef"),     5  },
    { _T("ntsc"),      2  },
    { _T("component"), 0  },
    { _T("pal"),       1  },
    { _T("secam"),     3  },
    { _T("mac"),       4  },
    { NULL, NULL } 
};

const CX_DESC list_mv_presicion[] = {
    { _T("full-pel"), VCE_MOTION_EST_FULL   },
    { _T("half-pel"), VCE_MOTION_EST_HALF   },
    { _T("Q-pel"),    VCE_MOTION_EST_QUATER | VCE_MOTION_EST_HALF },
    { NULL, NULL }
};

const CX_DESC list_vce_rc_method[] = {
    { _T("CQP"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTRAINED_QP       },
    { _T("CBR"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR                  },
    { _T("VBR"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR },
    { NULL, NULL }
};

const CX_DESC list_vce_quality_preset[] = {
    { _T("balanced"), AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED },
    { _T("fast"),     AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED    },
    { _T("slow"),     AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY  },
    { NULL, NULL }
};

const CX_DESC list_interlaced[] = {
    { _T("Progressive"), AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_FRAME        },
    //{ _T("tff"),         AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_TOP_FIELD    },
    //{ _T("bff"),         AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_BOTTOM_FIELD },
    { NULL, NULL }
};

static int get_cx_index(const CX_DESC * list, int v) {
    for (int i = 0; list[i].desc; i++)
        if (list[i].value == v)
            return i;
    return 0;
}

static const TCHAR *get_cx_desc(const CX_DESC * list, int v) {
    for (int i = 0; list[i].desc; i++)
        if (list[i].value == v)
            return list[i].desc;
    return _T("");
}

static int PARSE_ERROR_FLAG = INT_MIN;
static int get_value_from_chr(const CX_DESC *list, const TCHAR *chr) {
    for (int i = 0; list[i].desc; i++)
        if (_tcsicmp(list[i].desc, chr) == 0)
            return list[i].value;
    return PARSE_ERROR_FLAG;
}

typedef struct {
    int num, den;
} VCERational;

typedef union {
    struct {
        int left, up, right, bottom;
    };
    int c[4];
} InputCrop;

typedef struct {
    int srcWidth, srcHeight;
    int dstWidth, dstHeight;
    int frames;
    VCERational fps;
    InputCrop crop;
    int interlaced;
    amf::AMF_SURFACE_FORMAT format;
    void *pPrivateParam;
} VCEInputInfo;

typedef struct {
    int     nProfile;
    int     nLevel;
} VCECodecParam;

typedef struct {
    amf::AMF_MEMORY_TYPE memoryTypeIn;
    int     nCodecId;
    VCECodecParam codecParam[8];

    int     nAdapterId;
    int     nLogLevel;
    int     nInputType;

    int     nUsage;
    int     nRateControl;
    int     nQualityPreset;
    int     nBitrate;
    int     nMaxBitrate;
    int     nVBVBufferSize;
    int     nInitialVBVPercent;
    int     nSlices;
    int     nMaxLTR;
    int     bTimerPeriodTuning;
    int     nInterlaced;

    int     bDeblockFilter;
    int     bEnableSkipFrame;
    int     nQPI;
    int     nQPP;
    int     nQPB;
    int     nQPMax;
    int     nQPMin;
    int     nDeltaQPBFrame;
    int     nDeltaQPBFrameRef;
    int     nBframes;
    int     bBPyramid;
    int     nMotionEst;
    int     bEnforceHDR;
    int     nIDRPeriod;
    int     nGOPLen;
    const TCHAR *pInputFile;
    const TCHAR *pOutputFile;
    const TCHAR *pStrLog;
} VCEParam;

bool is_interlaced(VCEParam *prm);

void init_vce_param(VCEParam *prm);

static const int VCE_DEFAULT_QPI = 22;
static const int VCE_DEFAULT_QPP = 24;
static const int VCE_DEFAULT_QPB = 27;
static const int VCE_DEFAULT_BFRAMES = 0;
static const int VCE_DEFAULT_MAX_BITRATE = 20000;
static const int VCE_DEFAULT_VBV_BUFSIZE = 20000;
static const int VCE_DEFAULT_SLICES = 1;

static const int VCE_MAX_BFRAMES = 3;
static const int VCE_MAX_BITRATE = 100000;
static const int VCE_MAX_GOP_LEN = 1000;
static const int VCE_MAX_B_DELTA_QP = 10;
