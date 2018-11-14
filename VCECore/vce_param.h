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

static const wchar_t* VCE_PARAM_KEY_INPUT = L"INPUT";
static const wchar_t* VCE_PARAM_KEY_INPUT_WIDTH = L"WIDTH";
static const wchar_t* VCE_PARAM_KEY_INPUT_HEIGHT = L"HEIGHT";

static const wchar_t* VCE_PARAM_KEY_OUTPUT = L"OUTPUT";
static const wchar_t* VCE_PARAM_KEY_OUTPUT_WIDTH = L"OUTPUT_WIDTH";
static const wchar_t* VCE_PARAM_KEY_OUTPUT_HEIGHT = L"OUTPUT_HEIGHT";

static const wchar_t* VCE_PARAM_KEY_ENGINE = L"ENGINE";

static const wchar_t* VCE_PARAM_KEY_ADAPTERID = L"ADAPTERID";
static const wchar_t* VCE_PARAM_KEY_CAPABILITY = L"DISPLAYCAPABILITY";

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

const CX_DESC list_vce_hevc_rc_method[] = {
    { _T("CQP"), AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CONSTANT_QP },
    { _T("CBR"), AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CBR },
    { _T("VBR"), AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR },
    { NULL, NULL }
};

const CX_DESC list_vce_quality_preset[] = {
    { _T("balanced"), 0 },
    { _T("fast"),     1 },
    { _T("slow"),     2 },
    { NULL, NULL }
};

static const int H264_QUALITY_PRESET[3] = {
    AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED,
    AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED,
    AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY
};

static const int HEVC_QUALITY_PRESET[3] = {
    AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_BALANCED,
    AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_SPEED,
    AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_QUALITY
};
static const int *get_quality_preset(RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_HEVC: return HEVC_QUALITY_PRESET;
    case RGY_CODEC_H264: return H264_QUALITY_PRESET;
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
static int get_quality_index(RGY_CODEC codec, int quality_preset) {
    const int *preset = nullptr;
    switch (codec) {
    case RGY_CODEC_HEVC:
        preset = HEVC_QUALITY_PRESET;
        break;
    case RGY_CODEC_H264:
    default:
        preset = H264_QUALITY_PRESET;
        break;
    }
    if (preset) {
        for (int i = 0; i < 3; i++) {
            if (preset[i] == quality_preset) {
                return i;
            }
        }
    }
    return 0;
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

struct VCECodecParam {
    int nProfile;
    int nTier;
    int nLevel;
    int nReserved;
};

struct VCEParam {
    VideoInfo input;              //入力する動画の情報
    tstring inputFilename;        //入力ファイル名
    tstring outputFilename;       //出力ファイル名
    tstring AVMuxOutputFormat;   //出力フォーマット

    tstring logfile;              //ログ出力先
    int loglevel;                 //ログ出力レベル

    amf::AMF_MEMORY_TYPE memoryTypeIn;
    RGY_CODEC codec;
    VCECodecParam codecParam[RGY_CODEC_NUM];

    int     deviceID;

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
    tstring sFramePosListLog;     //framePosList出力先

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

    int par[2];
    int nRefFrames;
    int nLTRFrames;
    bool bFiller;
    bool bReserved;

    int     nVideoTrack;
    int     nVideoStreamId;
    bool    bCopyChapter;

    C2AFormat caption2ass;
    int     nSubtitleSelectCount;
    int    *pSubtitleSelect;
    int     nInputThread;
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
    int        nAVMux; //RGY_MUX_xxx
    int        nAVDemuxAnalyzeSec;

    muxOptList *pMuxOpt;
    tstring     sChapterFile;
    uint32_t    nAudioIgnoreDecodeError;
    RGYAVSync   nAVSyncMode;     //avsyncの方法
    uint32_t    nProcSpeedLimit; //プリデコードする場合の処理速度制限 (0で制限なし)
    bool        bAudioIgnoreNoTrackError;
    float       fSeekSec; //指定された秒数分先頭を飛ばす

    int         nOutputBufSizeMB;

    VideoVUIInfo vui;

    std::string  sMaxCll;
    std::string  sMasterDisplay;

    bool        bVBAQ;
    bool        preAnalysis;

    TCHAR *pMuxVidTsLogFile;
    TCHAR *pAVInputFormat;
    int nPerfMonitorSelect;
    int nPerfMonitorSelectMatplot;
    int nPerfMonitorInterval;

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
AMF_PARAM(RATE_CONTROL_PREANALYSIS_ENABLE);
AMF_PARAM(ENABLE_VBAQ);
AMF_PARAM(ENFORCE_HRD);
AMF_PARAM(FILLER_DATA_ENABLE);
AMF_PARAM(TARGET_BITRATE);
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

#pragma warning(pop)
