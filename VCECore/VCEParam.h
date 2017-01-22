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
#include "VCEUtil.h"

typedef AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_ENUM VCE_PICSTRUCT;

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
    VCE_INPUT_VPY_MT,
    VCE_INPUT_AVCODEC_VCE,
};

enum {
    VCE_CODEC_NONE = 0,
    VCE_CODEC_H264,
    VCE_CODEC_SVC,
    VCE_CODEC_HEVC,
    VCE_CODEC_MPEG2,
    VCE_CODEC_VC1,
    VCE_CODEC_WMV3
};

static const wchar_t *list_codec_key[] = {
    L"Unknown",
    AMFVideoEncoderVCE_AVC,
    AMFVideoEncoderVCE_SVC,
    AMFVideoEncoder_HEVC
};

enum {
    VCE_RESAMPLER_SWR,
    VCE_RESAMPLER_SOXR,
};

static const int VCE_OUTPUT_THREAD_AUTO = -1;
static const int VCE_AUDIO_THREAD_AUTO = -1;
static const int VCE_INPUT_THREAD_AUTO = -1;

typedef struct {
    int start, fin;
} sTrim;

typedef struct {
    std::vector<sTrim> list;
    int offset;
} sTrimParam;

typedef  std::vector<std::pair<tstring, tstring>> muxOptList;

static const int TRIM_MAX = INT_MAX;
static const int TRIM_OVERREAD_FRAMES = 128;

static bool inline frame_inside_range(int frame, const std::vector<sTrim>& trimList) {
    if (trimList.size() == 0)
        return true;
    if (frame < 0)
        return false;
    for (auto trim : trimList) {
        if (trim.start <= frame && frame <= trim.fin) {
            return true;
        }
    }
    return false;
}

static bool inline rearrange_trim_list(int frame, int offset, std::vector<sTrim>& trimList) {
    if (trimList.size() == 0)
        return true;
    if (frame < 0)
        return false;
    for (uint32_t i = 0; i < trimList.size(); i++) {
        if (trimList[i].start >= frame) {
            trimList[i].start = clamp(trimList[i].start + offset, 0, TRIM_MAX);
        }
        if (trimList[i].fin && trimList[i].fin >= frame) {
            trimList[i].fin = (int)clamp((int64_t)trimList[i].fin + offset, 0, (int64_t)TRIM_MAX);
        }
    }
    return false;
}

enum AVSync : uint32_t {
    VCE_AVSYNC_THROUGH   = 0x00,
    VCE_AVSYNC_INIT      = 0x01,
    VCE_AVSYNC_CHECK_PTS = 0x02,
    VCE_AVSYNC_VFR       = 0x02,
    VCE_AVSYNC_FORCE_CFR = 0x04 | VCE_AVSYNC_CHECK_PTS,
};

enum {
    VCEENC_MUX_NONE     = 0x00,
    VCEENC_MUX_VIDEO    = 0x01,
    VCEENC_MUX_AUDIO    = 0x02,
    VCEENC_MUX_SUBTITLE = 0x04,
};

enum {
    VCE_VPP_SUB_SIMPLE = 0,
    VCE_VPP_SUB_COMPLEX,
};

static const uint32_t MAX_SPLIT_CHANNELS = 32;
static const uint64_t VCE_CHANNEL_AUTO = UINT64_MAX;

template <uint32_t size>
static bool bSplitChannelsEnabled(uint64_t (&pnStreamChannels)[size]) {
    bool bEnabled = false;
    for (uint32_t i = 0; i < size; i++) {
        bEnabled |= pnStreamChannels[i] != 0;
    }
    return bEnabled;
}

template <uint32_t size>
static void setSplitChannelAuto(uint64_t (&pnStreamChannels)[size]) {
    for (uint32_t i = 0; i < size; i++) {
        pnStreamChannels[i] = ((uint64_t)1) << i;
    }
}

template <uint32_t size>
static bool isSplitChannelAuto(uint64_t (&pnStreamChannels)[size]) {
    bool isAuto = true;
    for (uint32_t i = 0; isAuto && i < size; i++) {
        isAuto &= (pnStreamChannels[i] == (((uint64_t)1) << i));
    }
    return isAuto;
}

typedef struct sAudioSelect {
    int    nAudioSelect;      //選択した音声トラックのリスト 1,2,...(1から連番で指定)
    TCHAR *pAVAudioEncodeCodec; //音声エンコードのコーデック
    int    nAVAudioEncodeBitrate; //音声エンコードに選択した音声トラックのビットレート
    int    nAudioSamplingRate;    //サンプリング周波数
    TCHAR *pAudioExtractFilename; //抽出する音声のファイル名のリスト
    TCHAR *pAudioExtractFormat; //抽出する音声ファイルのフォーマット
    TCHAR *pAudioFilter; //音声フィルタ
    uint64_t pnStreamChannelSelect[MAX_SPLIT_CHANNELS]; //入力音声の使用するチャンネル
    uint64_t pnStreamChannelOut[MAX_SPLIT_CHANNELS];    //出力音声のチャンネル
} sAudioSelect;

enum : uint8_t {
    VCE_MOTION_EST_FULL   = 0x00,
    VCE_MOTION_EST_HALF   = 0x01,
    VCE_MOTION_EST_QUATER = 0x02,
};

typedef struct {
    TCHAR *desc;
    int value;
} CX_DESC;

const CX_DESC list_empty[] = {
    { NULL, 0 }
};
const CX_DESC list_codec[] = {
    { _T("h264"), VCE_CODEC_H264 },
    { _T("hevc"), VCE_CODEC_HEVC },
    { NULL, 0 }
};
const CX_DESC list_codec_all[] = {
    { _T("h264"), VCE_CODEC_H264 },
    { _T("avc"),  VCE_CODEC_H264 },
    { _T("h265"), VCE_CODEC_HEVC },
    { _T("hevc"), VCE_CODEC_HEVC },
    { NULL, 0 }
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

const CX_DESC list_vce_h264_rc_method[] = {
    { _T("CQP"), AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP          },
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
    { _T("tff"),         AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_TOP_FIELD    },
    { _T("bff"),         AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_BOTTOM_FIELD },
    { NULL, NULL }
};

const CX_DESC list_pre_analysis[] = {
    { _T("none"),   AMF_VIDEO_ENCODER_PREENCODE_DISABLED },
    { _T("full"),   AMF_VIDEO_ENCODER_PREENCODE_ENABLED },
    { _T("half"),   AMF_VIDEO_ENCODER_PREENCODE_ENABLED_DOWNSCALEFACTOR_2 },
    { _T("quater"), AMF_VIDEO_ENCODER_PREENCODE_ENABLED_DOWNSCALEFACTOR_4 },
    { NULL, NULL }
};

const CX_DESC list_avsync[] = {
    { _T("through"),  VCE_AVSYNC_THROUGH   },
    { _T("forcecfr"), VCE_AVSYNC_FORCE_CFR },
    { NULL, 0 }
};

const CX_DESC list_resampler[] = {
    { _T("swr"),  VCE_RESAMPLER_SWR  },
    { _T("soxr"), VCE_RESAMPLER_SOXR },
    { NULL, 0 }
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
static const TCHAR *get_chr_from_value(const CX_DESC *list, int v) {
    for (int i = 0; list[i].desc; i++)
        if (list[i].value == v)
            return list[i].desc;
    return _T("unknown");
}

static inline const CX_DESC *get_level_list(int CodecID) {
    switch (CodecID) {
    case VCE_CODEC_H264:    return list_avc_level;
    case VCE_CODEC_HEVC:    return list_hevc_level;
    default:                return list_empty;
    }
}

static inline const CX_DESC *get_profile_list(int CodecID) {
    switch (CodecID) {
    case VCE_CODEC_H264:    return list_avc_profile;
    case VCE_CODEC_HEVC:    return list_hevc_profile;
    default:                return list_empty;
    }
}
    default:                return list_empty;
    }
}

typedef struct {
    int num, den;
} VCERational;

typedef union {
    struct {
        int left, up, right, bottom;
    };
    int c[4];
} sInputCrop;

typedef struct {
    int srcWidth, srcHeight;
    int dstWidth, dstHeight;
    int frames;
    VCERational fps;
    int AspectRatioW;
    int AspectRatioH;
    sInputCrop crop;
    VCE_PICSTRUCT nPicStruct;
    amf::AMF_SURFACE_FORMAT format;
    void *pPrivateParam;
} VCEInputInfo;

typedef struct {
    int     nProfile;
    int     nLevel;
} VCECodecParam;

typedef struct {
    int infoPresent;
    int colormatrix;
    int colorprim;
    int transfer;
    int videoformat;
    int fullrange;
} VCEVuiInfo;

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
    VCE_PICSTRUCT nPicStruct;

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
    const TCHAR *pStrLogFile;
    const TCHAR *pFramePosListLog;

    int VuiEnable;
    int VideoFormat;
    int ColorMatrix;
    int ColorPrim;
    int Transfer;
    bool bFullrange;

    int     nVideoTrack;
    int     nVideoStreamId;
    int     bCopyChapter;

    int     nSubtitleSelectCount;
    int    *pSubtitleSelect;
    int     nOutputThread;
    int     nAudioThread;
    int     nAudioResampler;

    int            nAudioSelectCount;
    sAudioSelect **ppAudioSelectList;

    int         nAudioSourceCount;
    TCHAR      **ppAudioSourceList;

    TCHAR     *pAVMuxOutputFormat;

    int        nTrimCount;
    sTrim     *pTrimList;
    int        nAVMux; //VCEENC_MUX_xxx
    int        nAVDemuxAnalyzeSec;

    muxOptList *pMuxOpt;
    TCHAR      *pChapterFile;
    uint32_t    nAudioIgnoreDecodeError;
    AVSync      nAVSyncMode;     //avsyncの方法 (VCE_AVSYNC_xxx)
    uint32_t    nProcSpeedLimit; //プリデコードする場合の処理速度制限 (0で制限なし)
    int         nInputThread;
    int         bAudioIgnoreNoTrackError;
    float       fSeekSec; //指定された秒数分先頭を飛ばす

    int         nOutputBufSizeMB;

    VCEVuiInfo  vui;

    int         bVBAQ;
    int         nPreAnalysis;
} VCEParam;

static bool is_interlaced(VCE_PICSTRUCT nInterlaced) {
    return nInterlaced == AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_TOP_FIELD
        || nInterlaced == AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_BOTTOM_FIELD;
}

static bool is_interlaced(VCEParam *prm) {
    return is_interlaced(prm->nPicStruct);
}

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

static const int VCE_OUTPUT_BUF_MB_MAX = 128;

static const int VCE_DEFAULT_AUDIO_IGNORE_DECODE_ERROR = 10;

#pragma warning(pop)
