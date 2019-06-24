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

#include "rgy_util.h"
#include "vce_param.h"

VCEParam::VCEParam() :
    input(),
    inputFilename(),
    outputFilename(),
    AVMuxOutputFormat(),
    logfile(),
    loglevel(RGY_LOG_INFO),
    codec(RGY_CODEC_H264),
    codecParam(),
    deviceID(0),
    usage(AMF_VIDEO_ENCODER_USAGE_TRANSCONDING),
    rateControl(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP),
    qualityPreset(0),
    nBitrate(5000),
    nMaxBitrate(0),
    nVBVBufferSize(0),
    nInitialVBVPercent(64),
    nSlices(1),
    nMaxLTR(0),
    bTimerPeriodTuning(true),
    sFramePosListLog(),
    bDeblockFilter(true),
    bEnableSkipFrame(false),
    nQPI(VCE_DEFAULT_QPI),
    nQPP(VCE_DEFAULT_QPP),
    nQPB(VCE_DEFAULT_QPB),
    nQPMax(51),
    nQPMin(0),
    nDeltaQPBFrame(6),
    nDeltaQPBFrameRef(4),
    nBframes(VCE_DEFAULT_BFRAMES),
    bBPyramid(false),
    nMotionEst(VCE_MOTION_EST_QUATER | VCE_MOTION_EST_HALF),
    bEnforceHRD(true),
    nIDRPeriod(1),
    nGOPLen(0),
    nRefFrames(VCE_DEFAULT_REF_FRAMES),
    nLTRFrames(0),
    bFiller(false),

    nVideoTrack(0),
    nVideoStreamId(0),
    bCopyChapter(false),
    caption2ass(FORMAT_INVALID),
    nSubtitleSelectCount(0),
    pSubtitleSelect(nullptr),
    nInputThread(RGY_INPUT_THREAD_AUTO),
    nOutputThread(RGY_OUTPUT_THREAD_AUTO),
    nAudioThread(RGY_AUDIO_THREAD_AUTO),
    nAudioResampler(0),

    nAudioSelectCount(0),
    ppAudioSelectList(nullptr),
    nAudioSourceCount(0),
    ppAudioSourceList(nullptr),
    pAVMuxOutputFormat(nullptr),

    nTrimCount(0),
    pTrimList(nullptr),

    nAVMux(RGY_MUX_NONE),
    nAVDemuxAnalyzeSec(0),

    pMuxOpt(nullptr),
    sChapterFile(),
    nAudioIgnoreDecodeError(VCE_DEFAULT_AUDIO_IGNORE_DECODE_ERROR),

    nAVSyncMode(RGY_AVSYNC_ASSUME_CFR),
    nProcSpeedLimit(0),
    bAudioIgnoreNoTrackError(false),
    fSeekSec(0.0),

    nOutputBufSizeMB(8),

    vui(),
    sMaxCll(),
    sMasterDisplay(),

    bVBAQ(false),
    preAnalysis(false),

    pMuxVidTsLogFile(nullptr),
    pAVInputFormat(nullptr),
    nPerfMonitorSelect(0),
    nPerfMonitorSelectMatplot(0),
    nPerfMonitorInterval(0) {
    codecParam[RGY_CODEC_H264].nLevel   = 0;
    codecParam[RGY_CODEC_H264].nProfile = list_avc_profile[2].value;
    codecParam[RGY_CODEC_HEVC].nLevel   = 0;
    codecParam[RGY_CODEC_HEVC].nProfile = AMF_VIDEO_ENCODER_HEVC_PROFILE_MAIN;
    codecParam[RGY_CODEC_HEVC].nTier    = AMF_VIDEO_ENCODER_HEVC_TIER_MAIN;

    vui.format    = get_value_from_chr(list_videoformat, _T("undef"));
    vui.matrix    = get_value_from_chr(list_colormatrix, _T("undef"));
    vui.colorprim = get_value_from_chr(list_colorprim, _T("undef"));
    vui.transfer  = get_value_from_chr(list_transfer, _T("undef"));
    vui.fullrange   = false;
    vui.chromaloc = 0;
}

VCEParam::~VCEParam() {

}

RGY_ERR AMFParams::SetParamType(const std::wstring &name, AMFParamType type, const std::wstring& desc) {
    auto found = m_params.find(name);
    if (found == m_params.end()) {
        m_params[name] = AMFParam(type, desc, amf::AMFVariant());
    } else {
        m_params[name].type = type;
    }
    return RGY_ERR_NONE;
}

RGY_ERR AMFParams::SetParam(const std::wstring &name, amf::AMFVariantStruct value) {
    auto found = m_params.find(name);
    if (found == m_params.end()) {
        return RGY_ERR_NOT_FOUND;
    }
    m_params[name].value = value;
    return RGY_ERR_NONE;
}

RGY_ERR AMFParams::Apply(amf::AMFPropertyStorage *storage, AMFParamType prmType, RGYLog *pLog) {
    for (const auto prm : m_params) {
        const auto &name = prm.first;
        const auto type = prm.second.type;
        const auto &value = prm.second.value;
        if (type == prmType && value.type != amf::AMF_VARIANT_EMPTY) {
            if (storage->SetProperty(name.c_str(), value)) {
                if (pLog) {
                    pLog->write(RGY_LOG_ERROR, _T("storage->SetProperty(%s)=%s failed."),
                        wstring_to_tstring(name).c_str(),
                        wstring_to_tstring(value.ToWString().c_str()).c_str());
                }
            }
        }
    }
    return RGY_ERR_NONE;
}

RGY_ERR AMFParams::GetParam(const std::wstring &name, amf::AMFVariantStruct *value) const {
    auto found = m_params.find(name);
    if (found == m_params.end()) {
        return RGY_ERR_NOT_FOUND;
    }
    amf::AMFVariantCopy(value, &found->second.value);
    return RGY_ERR_NONE;
}

RGY_ERR AMFParams::SetParamTypeAVC() {
    SetParamType(SETFRAMEPARAMFREQ_PARAM_NAME, AMF_PARAM_COMMON, L"Frequency of applying frame parameters (in frames, default = 0 )");
    SetParamType(SETDYNAMICPARAMFREQ_PARAM_NAME, AMF_PARAM_COMMON, L"Frequency of applying dynamic parameters. (in frames, default = 0 )");


    // ------------- Encoder params usage---------------
    SetParamType(AMF_VIDEO_ENCODER_USAGE, AMF_PARAM_ENCODER_USAGE, L"Encoder usage type. Set many default parameters. (TRANSCONDING, ULTRALOWLATENCY, LOWLATENCY, WEBCAM, default = N/A)");
    // ------------- Encoder params static---------------
    SetParamType(AMF_VIDEO_ENCODER_PROFILE, AMF_PARAM_STATIC, L"H264 profile (Main, Baseline,High, default = Main");
    SetParamType(AMF_VIDEO_ENCODER_PROFILE_LEVEL, AMF_PARAM_STATIC, L"H264 profile level (float or integer, default = 4.2 (or 42)");
    SetParamType(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_PARAM_STATIC, L"Quality Preset (BALANCED, SPEED, QUALITY default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_SCANTYPE, AMF_PARAM_STATIC, L"Scan Type (PROGRESSIVE, INTERLACED, default = PROGRESSIVE)");
    SetParamType(AMF_VIDEO_ENCODER_MAX_LTR_FRAMES, AMF_PARAM_STATIC, L"Max Of LTR frames (integer, default = 0)");
    SetParamType(AMF_VIDEO_ENCODER_MAX_NUM_REFRAMES, AMF_PARAM_STATIC, L"Max Of Reference frames (integer, default = 4)");
    SetParamType(AMF_VIDEO_ENCODER_ENABLE_VBAQ, AMF_PARAM_STATIC, L"Eanble VBAQ (integer, default = 0)");
    SetParamType(AMF_VIDEO_ENCODER_RATE_CONTROL_PREANALYSIS_ENABLE, AMF_PARAM_STATIC, L"Rate Control Preanalysis Enabled (EANBLED, DISABLED, default = DISABLED)");
    SetParamType(AMF_VIDEO_ENCODER_ASPECT_RATIO, AMF_PARAM_STATIC, L"Controls aspect ratio, defulat (1,1)");
    SetParamType(AMF_VIDEO_ENCODER_FULL_RANGE_COLOR, AMF_PARAM_STATIC, L"Inidicates that YUV input is (0,255) (bool, default = false)");

    // ------------- Encoder params dynamic ---------------
    //SetParamType(AMF_VIDEO_ENCODER_WIDTH, AMF_PARAM_DYNAMIC, L"Frame width (integer, default = 0)");
    //SetParamType(AMF_VIDEO_ENCODER_HEIGHT, AMF_PARAM_DYNAMIC, L"Frame height (integer, default = 0)");
    //SetParamType(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP, AMF_PARAM_DYNAMIC, L"B-picture Delta  (integer, default = depends on USAGE)");
    //SetParamType(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, AMF_PARAM_DYNAMIC, L"Reference B-picture Delta  (integer, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_FRAMERATE, AMF_PARAM_DYNAMIC, L"Frame Rate (num,den), default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_MAX_AU_SIZE, AMF_PARAM_DYNAMIC, L"Max AU Size (in bits, default = 0)");
    SetParamType(AMF_VIDEO_ENCODER_TARGET_BITRATE, AMF_PARAM_DYNAMIC, L"Target bit rate (in bits, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_PEAK_BITRATE, AMF_PARAM_DYNAMIC, L"Peak bit rate (in bits, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_B_PIC_PATTERN, AMF_PARAM_DYNAMIC, L"B-picture Pattern (number of B-Frames, default = 3)");
    SetParamType(AMF_VIDEO_ENCODER_IDR_PERIOD, AMF_PARAM_DYNAMIC, L"IDR Period, (in frames, default = depends on USAGE) ");
    SetParamType(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, AMF_PARAM_DYNAMIC, L"Slices Per Frame (integer, default = 1)");
    SetParamType(AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT, AMF_PARAM_DYNAMIC, L"Intra Refresh MBs Number Per Slot (in Macroblocks, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS, AMF_PARAM_DYNAMIC, L"Initial VBV Buffer Fullness (integer, 0=0% 64=100% , default = 64)");
    SetParamType(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, AMF_PARAM_DYNAMIC, L"VBV Buffer Size (in bits, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_MIN_QP, AMF_PARAM_DYNAMIC, L"Min QP (integer 0-51, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_MAX_QP, AMF_PARAM_DYNAMIC, L"Max QP (integer 0-51, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_QP_I, AMF_PARAM_DYNAMIC, L"QP I (integer 0-51, default = 22)");
    SetParamType(AMF_VIDEO_ENCODER_QP_P, AMF_PARAM_DYNAMIC, L"QP P (integer 0-51, default = 22)");
    SetParamType(AMF_VIDEO_ENCODER_QP_B, AMF_PARAM_DYNAMIC, L"QP B (integer 0-51, default = 22)");
    SetParamType(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING, AMF_PARAM_DYNAMIC, L"Insertion spacing");

    SetParamType(AMF_VIDEO_ENCODER_ENFORCE_HRD, AMF_PARAM_DYNAMIC, L"Enforce HRD (true, false default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE, AMF_PARAM_DYNAMIC, L"Filler Data Enable (true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE, AMF_PARAM_DYNAMIC, L"Rate Control Based Frame Skip (true, false default =  depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER, AMF_PARAM_DYNAMIC, L"De-blocking Filter (true, false default =  depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, AMF_PARAM_DYNAMIC, L"Enable B Refrence (true, false default =  true)");
    SetParamType(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL, AMF_PARAM_DYNAMIC, L"Half Pixel (true, false default =  true)");
    SetParamType(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL, AMF_PARAM_DYNAMIC, L"Quarter Pixel (true, false default =  true");
    //SetParamType(AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS, AMF_PARAM_DYNAMIC, L"Num Of Temporal Enhancment Layers (SVC) (integer, default = 0, range = 0, min(2, caps->GetMaxNumOfTemporalLayers())");
    SetParamType(AMF_VIDEO_ENCODER_CABAC_ENABLE, AMF_PARAM_DYNAMIC, L"Encoding method (UNDEFINED, CAABC, CALV) default =UNDEFINED");

    SetParamType(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, AMF_PARAM_DYNAMIC, L"Rate Control Method (CQP, CBR, VBR, VBR_LAT default = depends on USAGE)");

    // ------------- Encoder params per frame ---------------
    SetParamType(AMF_VIDEO_ENCODER_INSERT_SPS, AMF_PARAM_FRAME, L"Insert SPS (true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_INSERT_PPS, AMF_PARAM_FRAME, L"Insert PPS (true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_INSERT_AUD, AMF_PARAM_FRAME, L"Insert AUD (true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_MARK_CURRENT_WITH_LTR_INDEX, AMF_PARAM_FRAME, L"Mark With LTR Index (integer, default -1)");
    SetParamType(AMF_VIDEO_ENCODER_FORCE_LTR_REFERENCE_BITFIELD, AMF_PARAM_FRAME, L"Force LTR Reference Bitfield (bitfield default = 0)");
    SetParamType(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_PARAM_FRAME, L"Force Picture Type (NONE, SKIP, IDR, I, P, B, default = NONE)");
    return RGY_ERR_NONE;
}

RGY_ERR AMFParams::SetParamTypeHEVC() {
    SetParamType(SETFRAMEPARAMFREQ_PARAM_NAME, AMF_PARAM_COMMON, L"Frequency of applying frame parameters (in frames, default = 0 )");
    SetParamType(SETDYNAMICPARAMFREQ_PARAM_NAME, AMF_PARAM_COMMON, L"Frequency of applying dynamic parameters. (in frames, default = 0 )");


    // ------------- Encoder params usage---------------
    SetParamType(AMF_VIDEO_ENCODER_HEVC_USAGE, AMF_PARAM_ENCODER_USAGE, L"Encoder usage type. Set many default parameters. (TRANSCONDING, ULTRALOWLATENCY, LOWLATENCY, WEBCAM, default = N/A)");
    // ------------- Encoder params static---------------
    SetParamType(AMF_VIDEO_ENCODER_HEVC_PROFILE, AMF_PARAM_STATIC, L"HEVC profile (Main, default = Main");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_TIER, AMF_PARAM_STATIC, L"HEVC tier (Main, High, default = Main");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_PROFILE_LEVEL, AMF_PARAM_STATIC, L"HEVC profile level (float or integer, default = based on HW");

    SetParamType(AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET, AMF_PARAM_STATIC, L"Quality Preset (BALANCED, SPEED, QUALITY default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MAX_LTR_FRAMES, AMF_PARAM_STATIC, L"Max Of LTR frames (integer, default = 0)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MAX_NUM_REFRAMES, AMF_PARAM_STATIC, L" Maximum number of reference frames default = 1");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_ENABLE_VBAQ, AMF_PARAM_STATIC, L"Enable VBAQ(true, false default =  false)");

    // Picture control properties
    SetParamType(AMF_VIDEO_ENCODER_HEVC_DE_BLOCKING_FILTER_DISABLE, AMF_PARAM_STATIC, L"De-blocking Filter(true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_NUM_GOPS_PER_IDR, AMF_PARAM_STATIC, L"The frequency to insert IDR as start of a GOP. 0 means no IDR will be inserted (in frames, default= 60 )");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_GOP_SIZE, AMF_PARAM_STATIC, L"GOP Size (in frames, default= 60 )");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_HEADER_INSERTION_MODE, AMF_PARAM_STATIC, L"insertion mode (none, gop, idr default = none");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_SLICES_PER_FRAME, AMF_PARAM_STATIC, L"Slices Per Frame (integer, default = 1)");


    // Rate control properties
    SetParamType(AMF_VIDEO_ENCODER_HEVC_FRAMERATE, AMF_PARAM_STATIC, L"Frame Rate (num,den), default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_VBV_BUFFER_SIZE, AMF_PARAM_STATIC, L"VBV Buffer Size (in bits, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_INITIAL_VBV_BUFFER_FULLNESS, AMF_PARAM_STATIC, L"Initial VBV Buffer Fullness (integer, 0=0% 64=100% , default = 64)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_TARGET_BITRATE, AMF_PARAM_DYNAMIC, L"Target bit rate (in bits, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_PEAK_BITRATE, AMF_PARAM_DYNAMIC, L"Peak bit rate (in bits, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD, AMF_PARAM_STATIC, L"Rate Control Method (CQP, CBR, VBR, VBR_LAT default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_PREANALYSIS_ENABLE, AMF_PARAM_STATIC, L"Enable Preanalysis(true, false default = depends on USAGE)");

    // Motion estimation
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MOTION_HALF_PIXEL, AMF_PARAM_STATIC, L"Half Pixel (true, false default =  true)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MOTION_QUARTERPIXEL, AMF_PARAM_STATIC, L"Quarter Pixel (true, false default =  true");

    // ------------- Encoder params dynamic ---------------
//    SetParamType(AMF_VIDEO_ENCODER_HEVC_WIDTH, AMF_PARAM_DYNAMIC, L"Frame width (integer, default = 0)");
//    SetParamType(AMF_VIDEO_ENCODER_HEVC_HEIGHT, AMF_PARAM_DYNAMIC, L"Frame height (integer, default = 0)");


// Rate control properties
    SetParamType(AMF_VIDEO_ENCODER_HEVC_ENFORCE_HRD, AMF_PARAM_DYNAMIC, L"Enforce HRD (true, false default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_FILLER_DATA_ENABLE, AMF_PARAM_DYNAMIC, L"Filler Data Enable (true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MAX_AU_SIZE, AMF_PARAM_DYNAMIC, L"Max AU Size (in bits, default = 0)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MIN_QP_I, AMF_PARAM_DYNAMIC, L"Min QP I frame (integer 0-51, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MAX_QP_I, AMF_PARAM_DYNAMIC, L"Max QP I frame (integer 0-51, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MIN_QP_P, AMF_PARAM_DYNAMIC, L"Min QP I frame (integer 0-51, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MAX_QP_P, AMF_PARAM_DYNAMIC, L"Max QP I frame (integer 0-51, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_QP_I, AMF_PARAM_DYNAMIC, L"QP I (integer 0-51, default = 22)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_QP_P, AMF_PARAM_DYNAMIC, L"QP P (integer 0-51, default = 22)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_SKIP_FRAME_ENABLE, AMF_PARAM_DYNAMIC, L"Rate Control Based Frame Skip (true, false default =  depends on USAGE)");

    // ------------- Encoder params per frame ---------------
    SetParamType(AMF_VIDEO_ENCODER_HEVC_INSERT_HEADER, AMF_PARAM_FRAME, L"Insert Header (true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_INSERT_AUD, AMF_PARAM_FRAME, L"Insert AUD (true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MARK_CURRENT_WITH_LTR_INDEX, AMF_PARAM_FRAME, L"Mark With LTR Index (integer, default -1)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_FORCE_LTR_REFERENCE_BITFIELD, AMF_PARAM_FRAME, L"Force LTR Reference Bitfield (bitfield default = 0)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_FORCE_PICTURE_TYPE, AMF_PARAM_FRAME, L"Force Picture Type (NONE, SKIP, IDR, I, P, B, default = NONE)");
    return RGY_ERR_NONE;
}
