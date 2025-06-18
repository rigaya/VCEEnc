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

#include <algorithm>
#include "rgy_util.h"
#include "vce_param.h"
#include "rgy_bitstream.h"

VppAMFHQScaler::VppAMFHQScaler() :
    enable(false),
    algorithm(AMF_HQ_SCALER_ALGORITHM_BICUBIC),
    sharpness(VCE_FILTER_HQ_SCALER_SHARPNESS_DEFAULT) {
};
bool VppAMFHQScaler::operator==(const VppAMFHQScaler& x) const {
    return enable == x.enable
        && algorithm == x.algorithm
        && sharpness == x.sharpness;
}
bool VppAMFHQScaler::operator!=(const VppAMFHQScaler& x) const {
    return !(*this == x);
}
tstring VppAMFHQScaler::print() const {
    tstring str = strsprintf(
        _T("scaler: %s"),
        get_cx_desc(list_vce_hq_scaler, algorithm));
    if (sharpness != VppAMFHQScaler().sharpness) {
        str += strsprintf(_T(", sharpness % .1f"), sharpness);
    }
    str += _T("\n");
    return str;
}

VppAMFPreProcessing::VppAMFPreProcessing() :
    enable(false),
    strength(VCE_FILTER_PP_STRENGTH_DEFAULT),
    sensitivity(VCE_FILTER_PP_SENSITIVITY_DEFAULT),
    adaptiveFilter(VCE_FILTER_PP_ADAPT_FILTER_DEFAULT) {
};
bool VppAMFPreProcessing::operator==(const VppAMFPreProcessing& x) const {
    return enable == x.enable
        && strength == x.strength
        && sensitivity == x.sensitivity
        && adaptiveFilter == x.adaptiveFilter;
}
bool VppAMFPreProcessing::operator!=(const VppAMFPreProcessing& x) const {
    return !(*this == x);
}
tstring VppAMFPreProcessing::print() const {
    return strsprintf(
        _T("pp: strength %d, sensitivity %d, adaptivefilter %s\n"),
        strength, sensitivity,
        adaptiveFilter ? _T("on") : _T("off"));
}

VppAMFVQEnhancer::VppAMFVQEnhancer() :
    enable(false),
    attenuation((float)VE_FCR_DEFAULT_ATTENUATION),
    fcrRadius(3) {
};
bool VppAMFVQEnhancer::operator==(const VppAMFVQEnhancer& x) const {
    return enable == x.enable
        && attenuation == x.attenuation
        && fcrRadius == x.fcrRadius;
}
bool VppAMFVQEnhancer::operator!=(const VppAMFVQEnhancer& x) const {
    return !(*this == x);
}
tstring VppAMFVQEnhancer::print() const {
    return strsprintf(
        _T("vqenhancer: attenuation %.1f, radius %d\n"),
        attenuation, fcrRadius);
}

VppAMFFRC::VppAMFFRC() :
    enable(false),
    profile(VCE_FILTER_FRC_PROFILE_DEFAULT),
    mvSearchMode(VCE_FILTER_FRC_MV_SEARCH_DEFAULT),
    enableBlend(VCE_FILTER_FRC_BLEND_DEFAULT) {
};

bool VppAMFFRC::operator==(const VppAMFFRC& x) const {
    return enable == x.enable
        && profile == x.profile
        && mvSearchMode == x.mvSearchMode
        && enableBlend == x.enableBlend;
}

bool VppAMFFRC::operator!=(const VppAMFFRC& x) const {
    return !(*this == x);
}

tstring VppAMFFRC::print() const {
    return strsprintf(
        _T("frc: profile %s, mvSearchMode %s\n"),
        get_cx_desc(list_frc_profile, profile),
        get_cx_desc(list_frc_mv_search, mvSearchMode)/*,
        enableBlend ? _T("on") : _T("off")*/
    );
}

VCEFilterParam::VCEFilterParam() :
    scaler(),
    pp(),
    enhancer(),
    frc() {

}
bool VCEFilterParam::operator==(const VCEFilterParam& x) const {
    return scaler == x.scaler
        && pp == x.pp
        && enhancer == x.enhancer
        && frc == x.frc;
}
bool VCEFilterParam::operator!=(const VCEFilterParam& x) const {
    return !(*this == x);
}

VCEParamPA::VCEParamPA() :
    enable(false),
    sc(true),
    scSensitivity(AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY_MEDIUM),
    ss(true),
    ssSensitivity(AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY_HIGH),
    activityType(AMF_PA_ACTIVITY_Y),
    initQPSC(AMF_PA_INITQPSC_AUTO),
    maxQPBeforeForceSkip(35),
    ltrEnable(false),
    CAQStrength(AMF_PA_CAQ_STRENGTH_MEDIUM),
    PAQMode(AMF_PA_PAQ_MODE_NONE),
    TAQMode(AMF_PA_TAQ_MODE_NONE),
    motionQualityBoost(AMF_PA_HIGH_MOTION_QUALITY_BOOST_MODE_NONE),
    lookaheadDepth(0) {
};


bool VCEParamPA::operator==(const VCEParamPA& x) const {
    return enable == x.enable
        && sc == x.sc
        && scSensitivity == x.scSensitivity
        && ss == x.ss
        && ssSensitivity == x.ssSensitivity
        && activityType == x.activityType
        && initQPSC == x.initQPSC
        && maxQPBeforeForceSkip == x.maxQPBeforeForceSkip
        && ltrEnable == x.ltrEnable
        && CAQStrength == x.CAQStrength
        && PAQMode == x.PAQMode
        && TAQMode == x.TAQMode
        && motionQualityBoost == x.motionQualityBoost
        && lookaheadDepth == x.lookaheadDepth;
}
bool VCEParamPA::operator!=(const VCEParamPA& x) const {
    return !(*this == x);
}

VCEParam::VCEParam() :
    input(),
    inprm(),
    common(),
    ctrl(),
    vpp(),
    vppamf(),
    codec(RGY_CODEC_H264),
    codecParam(),
    deviceID(-1),
#if ENABLE_D3D11
    interopD3d9(false),
    interopD3d11(true),
#else
    interopD3d9(false),
    interopD3d11(false),
#endif
    outputDepth(8),
    par(),
    usage(AMF_VIDEO_ENCODER_USAGE_TRANSCONDING),
    rateControl(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP),
    qualityPreset(0),
    qvbrLevel(23),
    nBitrate(5000),
    nMaxBitrate(0),
    nVBVBufferSize(0),
    initialVBVPercent(),
    nSlices(1),
    maxLTR(),
    bTimerPeriodTuning(true),
    qp(VCE_DEFAULT_QPI, VCE_DEFAULT_QPP, VCE_DEFAULT_QPB),
    deblockFilter(),
    enableSkipFrame(),
    nQPMax(),
    nQPMin(),
    nQPMaxInter(),
    nQPMinInter(),
    deltaQPBFrame(),
    deltaQPBFrameRef(),
    bframes(),
    bPyramid(),
    adaptMiniGOP(false),
    nMotionEst(VCE_MOTION_EST_QUATER | VCE_MOTION_EST_HALF),
    bEnforceHRD(false),
    nIDRPeriod(1),
    nGOPLen(0),
    refFrames(),
    LTRFrames(),
    bFiller(false),
    pa(),
    pe(),
    bVBAQ(),
    tiles(0),
    cdefMode(),
    screenContentTools(true),
    paletteMode(true),
    forceIntegerMV(false),
    cdfUpdate(),
    cdfFrameEndUpdate(),
    temporalLayers(),
    aqMode(),
    ssim(false),
    psnr(false),
    smartAccessVideo(false),
    multiInstance(false),
    enableAV1HWDec(true) {
    codecParam[RGY_CODEC_H264].nLevel   = 0;
#if defined(_WIN32) || defined(_WIN64)
    codecParam[RGY_CODEC_H264].nProfile = list_avc_profile[2].value;
#else
    codecParam[RGY_CODEC_H264].nProfile = list_avc_profile[1].value;
#endif
    codecParam[RGY_CODEC_HEVC].nLevel   = 0;
    codecParam[RGY_CODEC_HEVC].nProfile = AMF_VIDEO_ENCODER_HEVC_PROFILE_MAIN;
    codecParam[RGY_CODEC_HEVC].nTier    = AMF_VIDEO_ENCODER_HEVC_TIER_MAIN;

    codecParam[RGY_CODEC_AV1].nLevel = -1; // as auto
    codecParam[RGY_CODEC_AV1].nProfile = AMF_VIDEO_ENCODER_AV1_PROFILE_MAIN;
    par[0] = par[1] = 0;
#if ENABLE_VULKAN
    ctrl.enableVulkan = RGYParamInitVulkan::TargetVendor;
#endif
    input.vui = VideoVUIInfo();
}

VCEParam::~VCEParam() {

}

void VCEParam::applyDOVIProfile(const RGYDOVIProfile inputProfile) {
#if !FOR_AUO
    if (codec != RGY_CODEC_HEVC) {
        return;
    }
    auto targetDoviProfile = (common.doviProfile == RGY_DOVI_PROFILE_COPY) ? inputProfile : common.doviProfile;
    if (targetDoviProfile == 0) {
        return;
    }
    auto profile = getDOVIProfile(targetDoviProfile);
    if (profile == nullptr) {
        return;
    }

    common.out_vui.setIfUnset(profile->vui);
    if (profile->aud) {
        //bOutputAud = true;
    }
    if (profile->HRDSEI) {
        //bufPeriodSEI = true;
        //bOutputPicStruct = true;
    }
    if (profile->profile == 50) {
        //crQPIndexOffset = 3;
    }
    if (profile->profile == 81) {
        //hdr10sei
        //maxcll
    }
#endif
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
    for (const auto& prm : m_params) {
        const auto &name = prm.first;
        const auto type = prm.second.type;
        const auto &value = prm.second.value;
        if (type == prmType && value.type != amf::AMF_VARIANT_EMPTY) {
            const auto ret = storage->SetProperty(name.c_str(), value);
            if (ret != AMF_OK) {
                if (pLog) {
                    pLog->write(RGY_LOG_ERROR, RGY_LOGT_CORE, _T("storage->SetProperty(%s)=%s failed: %s.\n"),
                        wstring_to_tstring(name).c_str(),
                        wstring_to_tstring(value.ToWString().c_str()).c_str(),
                        get_err_mes(err_to_rgy(ret)));
                }
            }
        }
    }
    return RGY_ERR_NONE;
}

int64_t AMFParams::CountParam(const AMFParamType prmType) const {
    return std::count_if(m_params.begin(), m_params.end(), [prmType](const auto& p) {
        return p.second.type == prmType;
    });
}

RGY_ERR AMFParams::GetParam(const std::wstring &name, amf::AMFVariantStruct *value) const {
    auto found = m_params.find(name);
    if (found == m_params.end()) {
        return RGY_ERR_NOT_FOUND;
    }
    amf::AMFVariantCopy(value, &found->second.value);
    return RGY_ERR_NONE;
}

RGY_ERR AMFParams::SetParamTypeCodec(const RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_H264: SetParamTypeAVC(); break;
    case RGY_CODEC_HEVC: SetParamTypeHEVC(); break;
    case RGY_CODEC_AV1: SetParamTypeAV1(); break;
    default: return RGY_ERR_UNSUPPORTED;
    }
    return RGY_ERR_NONE;
}

RGY_ERR AMFParams::SetParamTypePA() {
    // ------------- PA parameters ---------------
    SetParamType(AMF_PA_ENGINE_TYPE, AMF_PARAM_STATIC, L"Engine Type (DX11, OPENCL default = OPENCL)");

    SetParamType(AMF_PA_SCENE_CHANGE_DETECTION_ENABLE, AMF_PARAM_DYNAMIC, L"Scene Change Detection Enable (true, false default =  true)");
    SetParamType(AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY, AMF_PARAM_DYNAMIC, L"Scene Change Detection Sensitivity (LOW, MEDIUM, HIGH default = MEDIUM)");
    SetParamType(AMF_PA_STATIC_SCENE_DETECTION_ENABLE, AMF_PARAM_DYNAMIC, L"Static Scene Detection Enable (true, false default =  true)");
    SetParamType(AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY, AMF_PARAM_DYNAMIC, L"Scene Change Detection Sensitivity (LOW, MEDIUM, HIGH default = HIGH)");
    SetParamType(AMF_PA_ACTIVITY_TYPE, AMF_PARAM_DYNAMIC, L"Activity Type (Y, YUV default = Y)");

    SetParamType(AMF_PA_INITIAL_QP_AFTER_SCENE_CHANGE, AMF_PARAM_DYNAMIC, L"QP After Scene Change (integer 0-51, default = 0)");
    SetParamType(AMF_PA_MAX_QP_BEFORE_FORCE_SKIP, AMF_PARAM_DYNAMIC, L"Max QP Before Force Skip (integer 0-51, default = 35)");

    SetParamType(AMF_PA_LTR_ENABLE, AMF_PARAM_DYNAMIC, L"Enable Automatic Long Term Reference frame management");
    SetParamType(AMF_PA_LOOKAHEAD_BUFFER_DEPTH, AMF_PARAM_DYNAMIC, L"PA lookahead buffer size (default : 0)           Values: [0, MAX_LOOKAHEAD_DEPTH]");
    SetParamType(AMF_PA_PAQ_MODE, AMF_PARAM_DYNAMIC, L"Perceptual AQ mode (default : AMF_PA_PAQ_MODE_NONE)");
    SetParamType(AMF_PA_TAQ_MODE, AMF_PARAM_DYNAMIC, L"Temporal AQ mode (default: AMF_PA_TAQ_MODE_NONE)");
    SetParamType(AMF_PA_HIGH_MOTION_QUALITY_BOOST_MODE, AMF_PARAM_DYNAMIC, L"High motion quality boost mode (default: None)");
    //  SetParamType(AMF_IN_FRAME_QP_FILTERING_STRENGTH, AMF_PARAM_DYNAMIC, L"In Frame QP Filtering Strength (integer 0-255, default = 0)");
    //  SetParamType(AMF_BETWEEN_FRAME_QP_FILTERING_STRENGTH, AMF_PARAM_DYNAMIC, L"Between Frame QP Filtering Strength (integer 0-255, default = 0)");
    SetParamType(AMF_PA_CAQ_STRENGTH, AMF_PARAM_DYNAMIC, L"CAQ Strength (LOW, MEDIUM, HIGH default = MEDIUM)");
    return RGY_ERR_NONE;
}

RGY_ERR AMFParams::SetParamTypeAVC() {
    SetParamType(SETFRAMEPARAMFREQ_PARAM_NAME, AMF_PARAM_COMMON, L"Frequency of applying frame parameters (in frames, default = 0 )");
    SetParamType(SETDYNAMICPARAMFREQ_PARAM_NAME, AMF_PARAM_COMMON, L"Frequency of applying dynamic parameters. (in frames, default = 0 )");


    // ------------- Encoder params usage---------------
    SetParamType(AMF_VIDEO_ENCODER_USAGE, AMF_PARAM_ENCODER_USAGE, L"Encoder usage type. Set many default parameters. (TRANSCONDING, ULTRALOWLATENCY, LOWLATENCY, WEBCAM, default = N/A)");
    // ------------- Encoder params static---------------
    SetParamType(AMF_VIDEO_ENCODER_FRAMESIZE, AMF_PARAM_STATIC, L"AMFSize; default = 0,0; Frame size");
    SetParamType(AMF_VIDEO_ENCODER_FRAMERATE, AMF_PARAM_STATIC, L"Frame Rate (num,den), default = depends on USAGE)");

    SetParamType(AMF_VIDEO_ENCODER_PROFILE, AMF_PARAM_STATIC, L"H264 profile (Main, Baseline,High, default = Main");
    SetParamType(AMF_VIDEO_ENCODER_PROFILE_LEVEL, AMF_PARAM_STATIC, L"H264 profile level (float or integer, default = 4.2 (or 42)");
    SetParamType(AMF_VIDEO_ENCODER_MAX_LTR_FRAMES, AMF_PARAM_STATIC, L"Max Of LTR frames (integer, default = 0)");
    SetParamType(AMF_VIDEO_ENCODER_LTR_MODE, AMF_PARAM_STATIC, L"amf_int64(AMF_VIDEO_ENCODER_LTR_MODE_ENUM); default = AMF_VIDEO_ENCODER_LTR_MODE_RESET_UNUSED; remove/keep unused LTRs (not specified in property AMF_VIDEO_ENCODER_FORCE_LTR_REFERENCE_BITFIELD)");
    SetParamType(AMF_VIDEO_ENCODER_SCANTYPE, AMF_PARAM_STATIC, L"Scan Type (PROGRESSIVE, INTERLACED, default = PROGRESSIVE)");
    SetParamType(AMF_VIDEO_ENCODER_MAX_NUM_REFRAMES, AMF_PARAM_STATIC, L"Max Of Reference frames (integer, default = 4)");
    SetParamType(AMF_VIDEO_ENCODER_MAX_CONSECUTIVE_BPICTURES, AMF_PARAM_STATIC, L"amf_int64; Maximum number of consecutive B Pictures");
    SetParamType(AMF_VIDEO_ENCODER_ADAPTIVE_MINIGOP, AMF_PARAM_STATIC, L"bool; default = false; Disable/Enable Adaptive MiniGOP");
    SetParamType(AMF_VIDEO_ENCODER_ASPECT_RATIO, AMF_PARAM_STATIC, L"Controls aspect ratio, defulat (1,1)");
    SetParamType(AMF_VIDEO_ENCODER_FULL_RANGE_COLOR, AMF_PARAM_STATIC, L"Inidicates that YUV input is (0,255) (bool, default = false)");
    SetParamType(AMF_VIDEO_ENCODER_LOWLATENCY_MODE, AMF_PARAM_STATIC, L"bool; default = false, enables low latency mode");
#pragma warning(push)
#pragma warning(disable:4995) //warning C4995: 'AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_PREANALYSIS_ENABLE': 名前が避けられた #pragma として記述されています。
    SetParamType(AMF_VIDEO_ENCODER_RATE_CONTROL_PREANALYSIS_ENABLE, AMF_PARAM_STATIC, L"Rate Control Preanalysis Enabled (EANBLED, DISABLED, default = DISABLED)");
#pragma warning(pop)
    SetParamType(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, AMF_PARAM_DYNAMIC, L"Rate Control Method (CQP, CBR, VBR, VBR_LAT default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_QVBR_QUALITY_LEVEL, AMF_PARAM_STATIC, L"amf_int64; default = 23; QVBR quality level; range = 1-51");
    SetParamType(AMF_VIDEO_ENCODER_PRE_ANALYSIS_ENABLE, AMF_PARAM_STATIC, L"Enable PA (true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_PREENCODE_ENABLE, AMF_PARAM_STATIC, L"enables pre-encode assisted rate control");

    SetParamType(AMF_VIDEO_ENCODER_QUALITY_PRESET, AMF_PARAM_STATIC, L"Quality Preset (BALANCED, SPEED, QUALITY default = depends on USAGE)");

    SetParamType(AMF_VIDEO_ENCODER_COLOR_BIT_DEPTH, AMF_PARAM_STATIC, L"amf_int64(AMF_COLOR_BIT_DEPTH_ENUM); default = AMF_COLOR_BIT_DEPTH_8");

    SetParamType(AMF_VIDEO_ENCODER_INPUT_COLOR_PROFILE, AMF_PARAM_STATIC, L"amf_int64(AMF_VIDEO_CONVERTER_COLOR_PROFILE_ENUM); default = AMF_VIDEO_CONVERTER_COLOR_PROFILE_UNKNOWN");
    SetParamType(AMF_VIDEO_ENCODER_INPUT_TRANSFER_CHARACTERISTIC, AMF_PARAM_STATIC, L"amf_int64(AMF_COLOR_TRANSFER_CHARACTERISTIC_ENUM); default = AMF_COLOR_TRANSFER_CHARACTERISTIC_UNDEFINED");
    SetParamType(AMF_VIDEO_ENCODER_INPUT_COLOR_PRIMARIES, AMF_PARAM_STATIC, L"amf_int64(AMF_COLOR_PRIMARIES_ENUM); default = AMF_COLOR_PRIMARIES_UNDEFINED");
    SetParamType(AMF_VIDEO_ENCODER_INPUT_HDR_METADATA, AMF_PARAM_STATIC, L"AMFBuffer containing AMFHDRMetadata; default NULL");

    SetParamType(AMF_VIDEO_ENCODER_OUTPUT_COLOR_PROFILE, AMF_PARAM_STATIC, L"amf_int64(AMF_VIDEO_CONVERTER_COLOR_PROFILE_ENUM); default = AMF_VIDEO_CONVERTER_COLOR_PROFILE_UNKNOWN");
    SetParamType(AMF_VIDEO_ENCODER_OUTPUT_TRANSFER_CHARACTERISTIC, AMF_PARAM_STATIC, L"amf_int64(AMF_COLOR_TRANSFER_CHARACTERISTIC_ENUM); default = AMF_COLOR_TRANSFER_CHARACTERISTIC_UNDEFINED");
    SetParamType(AMF_VIDEO_ENCODER_OUTPUT_COLOR_PRIMARIES, AMF_PARAM_STATIC, L"amf_int64(AMF_COLOR_PRIMARIES_ENUM); default = AMF_COLOR_PRIMARIES_UNDEFINED");
    SetParamType(AMF_VIDEO_ENCODER_OUTPUT_HDR_METADATA, AMF_PARAM_STATIC, L"AMFBuffer containing AMFHDRMetadata; default NULL");

    // ------------- Encoder params dynamic ---------------
    SetParamType(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP, AMF_PARAM_DYNAMIC, L"B-picture Delta  (integer, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP, AMF_PARAM_DYNAMIC, L"Reference B-picture Delta  (integer, default = depends on USAGE)");

    SetParamType(AMF_VIDEO_ENCODER_ENFORCE_HRD, AMF_PARAM_DYNAMIC, L"Enforce HRD (true, false default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE, AMF_PARAM_DYNAMIC, L"Filler Data Enable (true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_ENABLE_VBAQ, AMF_PARAM_DYNAMIC, L"Eanble VBAQ (integer, default = 0)");
    SetParamType(AMF_VIDEO_ENCODER_HIGH_MOTION_QUALITY_BOOST_ENABLE, AMF_PARAM_DYNAMIC, L"bool; default = depends on USAGE; Enable High motion quality boost mode");

    SetParamType(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE, AMF_PARAM_DYNAMIC, L"VBV Buffer Size (in bits, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS, AMF_PARAM_DYNAMIC, L"Initial VBV Buffer Fullness (integer, 0=0% 64=100% , default = 64)");

    SetParamType(AMF_VIDEO_ENCODER_MAX_AU_SIZE, AMF_PARAM_DYNAMIC, L"Max AU Size (in bits, default = 0)");

    SetParamType(AMF_VIDEO_ENCODER_MIN_QP, AMF_PARAM_DYNAMIC, L"Min QP (integer 0-51, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_MAX_QP, AMF_PARAM_DYNAMIC, L"Max QP (integer 0-51, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_QP_I, AMF_PARAM_DYNAMIC, L"QP I (integer 0-51, default = 22)");
    SetParamType(AMF_VIDEO_ENCODER_QP_P, AMF_PARAM_DYNAMIC, L"QP P (integer 0-51, default = 22)");
    SetParamType(AMF_VIDEO_ENCODER_QP_B, AMF_PARAM_DYNAMIC, L"QP B (integer 0-51, default = 22)");
    SetParamType(AMF_VIDEO_ENCODER_TARGET_BITRATE, AMF_PARAM_DYNAMIC, L"Target bit rate (in bits, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_PEAK_BITRATE, AMF_PARAM_DYNAMIC, L"Peak bit rate (in bits, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE, AMF_PARAM_DYNAMIC, L"Rate Control Based Frame Skip (true, false default =  depends on USAGE)");

    SetParamType(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING, AMF_PARAM_DYNAMIC, L"Insertion spacing");
    SetParamType(AMF_VIDEO_ENCODER_B_PIC_PATTERN, AMF_PARAM_DYNAMIC, L"B-picture Pattern (number of B-Frames, default = 3)");
    SetParamType(AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER, AMF_PARAM_DYNAMIC, L"De-blocking Filter (true, false default =  depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE, AMF_PARAM_DYNAMIC, L"Enable B Refrence (true, false default =  true)");
    SetParamType(AMF_VIDEO_ENCODER_IDR_PERIOD, AMF_PARAM_DYNAMIC, L"IDR Period, (in frames, default = depends on USAGE) ");
    SetParamType(AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT, AMF_PARAM_DYNAMIC, L"Intra Refresh MBs Number Per Slot (in Macroblocks, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_SLICES_PER_FRAME, AMF_PARAM_DYNAMIC, L"Slices Per Frame (integer, default = 1)");
    SetParamType(AMF_VIDEO_ENCODER_CABAC_ENABLE, AMF_PARAM_DYNAMIC, L"Encoding method (UNDEFINED, CAABC, CALV) default =UNDEFINED");

    SetParamType(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL, AMF_PARAM_DYNAMIC, L"Half Pixel (true, false default =  true)");
    SetParamType(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL, AMF_PARAM_DYNAMIC, L"Quarter Pixel (true, false default =  true");

    //SetParamType(AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS, AMF_PARAM_DYNAMIC, L"Num Of Temporal Enhancment Layers (SVC) (integer, default = 0, range = 0, min(2, caps->GetMaxNumOfTemporalLayers())");

    // ------------- Encoder params per frame ---------------
    SetParamType(AMF_VIDEO_ENCODER_END_OF_SEQUENCE, AMF_PARAM_FRAME, L"bool; default = false; generate end of sequence");
    SetParamType(AMF_VIDEO_ENCODER_END_OF_STREAM, AMF_PARAM_FRAME, L"bool; default = false; generate end of stream");
    SetParamType(AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE, AMF_PARAM_FRAME, L"amf_int64(AMF_VIDEO_ENCODER_PICTURE_TYPE_ENUM); default = AMF_VIDEO_ENCODER_PICTURE_TYPE_NONE; generate particular picture type");
    SetParamType(AMF_VIDEO_ENCODER_INSERT_AUD, AMF_PARAM_FRAME, L"Insert AUD (true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_INSERT_SPS, AMF_PARAM_FRAME, L"Insert SPS (true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_INSERT_PPS, AMF_PARAM_FRAME, L"Insert PPS (true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_PICTURE_STRUCTURE, AMF_PARAM_FRAME, L"amf_int64(AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_ENUM); default = AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_FRAME; indicate picture type");
    SetParamType(AMF_VIDEO_ENCODER_MARK_CURRENT_WITH_LTR_INDEX, AMF_PARAM_FRAME, L"Mark With LTR Index (integer, default -1)");
    SetParamType(AMF_VIDEO_ENCODER_FORCE_LTR_REFERENCE_BITFIELD, AMF_PARAM_FRAME, L"Force LTR Reference Bitfield (bitfield default = 0)");
    SetParamType(AMF_VIDEO_ENCODER_ROI_DATA, AMF_PARAM_FRAME, L"2D AMFSurface, surface format: AMF_SURFACE_GRAY32");
    SetParamType(AMF_VIDEO_ENCODER_REFERENCE_PICTURE, AMF_PARAM_FRAME, L"AMFInterface(AMFSurface); surface used for frame injection");
    SetParamType(AMF_VIDEO_ENCODER_PSNR_FEEDBACK, AMF_PARAM_FRAME, L"amf_bool; default = false; Signal encoder to calculate PSNR score");
    SetParamType(AMF_VIDEO_ENCODER_SSIM_FEEDBACK, AMF_PARAM_FRAME, L"amf_bool; default = false; Signal encoder to calculate SSIM score");
    SetParamType(AMF_VIDEO_ENCODER_STATISTICS_FEEDBACK, AMF_PARAM_FRAME, L"amf_bool; default = false; Signal encoder to collect and feedback statistics");
    SetParamType(AMF_VIDEO_ENCODER_BLOCK_QP_FEEDBACK, AMF_PARAM_FRAME, L"amf_bool; default = false; Signal encoder to collect and feedback block level QP values");

    SetParamType(AMF_VIDEO_ENCODER_ENABLE_SMART_ACCESS_VIDEO, AMF_PARAM_STATIC, L"amf_bool; default = false; true = enables smart access video feature");

    SetParamTypePA();

    return RGY_ERR_NONE;
}

RGY_ERR AMFParams::SetParamTypeHEVC() {
    SetParamType(SETFRAMEPARAMFREQ_PARAM_NAME, AMF_PARAM_COMMON, L"Frequency of applying frame parameters (in frames, default = 0 )");
    SetParamType(SETDYNAMICPARAMFREQ_PARAM_NAME, AMF_PARAM_COMMON, L"Frequency of applying dynamic parameters. (in frames, default = 0 )");

    SetParamType(AMF_VIDEO_ENCODER_HEVC_ASPECT_RATIO, AMF_PARAM_STATIC, L"");

    // ------------- Encoder params usage---------------
    SetParamType(AMF_VIDEO_ENCODER_HEVC_USAGE, AMF_PARAM_ENCODER_USAGE, L"Encoder usage type. Set many default parameters. (TRANSCONDING, ULTRALOWLATENCY, LOWLATENCY, WEBCAM, default = N/A)");
    // ------------- Encoder params static---------------
    SetParamType(AMF_VIDEO_ENCODER_HEVC_PROFILE, AMF_PARAM_STATIC, L"HEVC profile (Main, default = Main");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_TIER, AMF_PARAM_STATIC, L"HEVC tier (Main, High, default = Main");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_PROFILE_LEVEL, AMF_PARAM_STATIC, L"HEVC profile level (float or integer, default = based on HW");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MAX_LTR_FRAMES, AMF_PARAM_STATIC, L"Max Of LTR frames (integer, default = 0)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_LTR_MODE, AMF_PARAM_STATIC, L"mf_int64(AMF_VIDEO_ENCODER_HEVC_LTR_MODE_ENUM); default = AMF_VIDEO_ENCODER_HEVC_LTR_MODE_RESET_UNUSED; remove/keep unused LTRs (not specified in property AMF_VIDEO_ENCODER_HEVC_FORCE_LTR_REFERENCE_BITFIELD)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MAX_NUM_REFRAMES, AMF_PARAM_STATIC, L" Maximum number of reference frames default = 1");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET, AMF_PARAM_STATIC, L"Quality Preset (BALANCED, SPEED, QUALITY default = depends on USAGE)");
    //SetParamType(AMF_VIDEO_ENCODER_HEVC_EXTRADATA, AMF_PARAM_STATIC, L"AMFInterface* - > AMFBuffer*; SPS/PPS buffer - read-only");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_ASPECT_RATIO, AMF_PARAM_STATIC, L"AMFRatio; default = 1, 1");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_LOWLATENCY_MODE, AMF_PARAM_STATIC, L"bool; default = false, enables low latency mode");
#pragma warning(push)
#pragma warning(disable:4995) //warning C4995: 'AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_PREANALYSIS_ENABLE': 名前が避けられた #pragma として記述されています。
    SetParamType(AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_PREANALYSIS_ENABLE, AMF_PARAM_STATIC, L"Enable Preanalysis(true, false default = depends on USAGE)");
#pragma warning(pop)
    SetParamType(AMF_VIDEO_ENCODER_HEVC_NOMINAL_RANGE, AMF_PARAM_STATIC, L"amf_int64(AMF_VIDEO_ENCODER_HEVC_NOMINAL_RANGE_ENUM) default = AMF_VIDEO_ENCODER_HEVC_NOMINAL_RANGE_FULL");

    // Picture control properties
    SetParamType(AMF_VIDEO_ENCODER_HEVC_NUM_GOPS_PER_IDR, AMF_PARAM_STATIC, L"The frequency to insert IDR as start of a GOP. 0 means no IDR will be inserted (in frames, default= 60 )");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_GOP_SIZE, AMF_PARAM_STATIC, L"GOP Size (in frames, default= 60 )");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_DE_BLOCKING_FILTER_DISABLE, AMF_PARAM_STATIC, L"De-blocking Filter(true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_SLICES_PER_FRAME, AMF_PARAM_STATIC, L"Slices Per Frame (integer, default = 1)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_HEADER_INSERTION_MODE, AMF_PARAM_STATIC, L"insertion mode (none, gop, idr default = none");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_INTRA_REFRESH_NUM_CTBS_PER_SLOT, AMF_PARAM_STATIC, L"amf_int64; default = depends on USAGE; Intra Refresh CTBs Number Per Slot in 64x64 CTB");


    // Rate control properties
    SetParamType(AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD, AMF_PARAM_STATIC, L"Rate Control Method (CQP, CBR, VBR, VBR_LAT default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_QVBR_QUALITY_LEVEL, AMF_PARAM_STATIC, L"amf_int64; default = 23; QVBR quality level; range = 1-51");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_FRAMERATE, AMF_PARAM_STATIC, L"Frame Rate (num,den), default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_VBV_BUFFER_SIZE, AMF_PARAM_STATIC, L"VBV Buffer Size (in bits, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_INITIAL_VBV_BUFFER_FULLNESS, AMF_PARAM_STATIC, L"Initial VBV Buffer Fullness (integer, 0=0% 64=100% , default = 64)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_ENABLE_VBAQ, AMF_PARAM_STATIC, L"Enable VBAQ(true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_HIGH_MOTION_QUALITY_BOOST_ENABLE, AMF_PARAM_DYNAMIC, L"bool; default = depends on USAGE; Enable High motion quality boost mode");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_PREENCODE_ENABLE, AMF_PARAM_STATIC, L"enables pre-encode assisted rate control");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_PRE_ANALYSIS_ENABLE, AMF_PARAM_STATIC, L"enables the pre-analysis module");
    // Motion estimation
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MOTION_HALF_PIXEL, AMF_PARAM_STATIC, L"Half Pixel (true, false default =  true)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MOTION_QUARTERPIXEL, AMF_PARAM_STATIC, L"Quarter Pixel (true, false default =  true");

    SetParamType(AMF_VIDEO_ENCODER_HEVC_COLOR_BIT_DEPTH, AMF_PARAM_STATIC, L"amf_int64(AMF_COLOR_BIT_DEPTH_ENUM); default = AMF_COLOR_BIT_DEPTH_8");

    SetParamType(AMF_VIDEO_ENCODER_HEVC_INPUT_COLOR_PROFILE, AMF_PARAM_STATIC, L"amf_int64(AMF_VIDEO_CONVERTER_COLOR_PROFILE_ENUM); default = AMF_VIDEO_CONVERTER_COLOR_PROFILE_UNKNOWN");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_INPUT_TRANSFER_CHARACTERISTIC, AMF_PARAM_STATIC, L"amf_int64(AMF_COLOR_TRANSFER_CHARACTERISTIC_ENUM); default = AMF_COLOR_TRANSFER_CHARACTERISTIC_UNDEFINED");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_INPUT_COLOR_PRIMARIES, AMF_PARAM_STATIC, L"amf_int64(AMF_COLOR_PRIMARIES_ENUM); default = AMF_COLOR_PRIMARIES_UNDEFINED");

    SetParamType(AMF_VIDEO_ENCODER_HEVC_OUTPUT_COLOR_PROFILE, AMF_PARAM_STATIC, L"amf_int64(AMF_VIDEO_CONVERTER_COLOR_PROFILE_ENUM); default = AMF_VIDEO_CONVERTER_COLOR_PROFILE_UNKNOWN");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_OUTPUT_TRANSFER_CHARACTERISTIC, AMF_PARAM_STATIC, L"amf_int64(AMF_COLOR_TRANSFER_CHARACTERISTIC_ENUM); default = AMF_COLOR_TRANSFER_CHARACTERISTIC_UNDEFINED");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_OUTPUT_COLOR_PRIMARIES, AMF_PARAM_STATIC, L"amf_int64(AMF_COLOR_PRIMARIES_ENUM); default = AMF_COLOR_PRIMARIES_UNDEFINED");

    // ------------- Encoder params dynamic ---------------
//    SetParamType(AMF_VIDEO_ENCODER_HEVC_WIDTH, AMF_PARAM_DYNAMIC, L"Frame width (integer, default = 0)");
//    SetParamType(AMF_VIDEO_ENCODER_HEVC_HEIGHT, AMF_PARAM_DYNAMIC, L"Frame height (integer, default = 0)");


// Rate control properties
    SetParamType(AMF_VIDEO_ENCODER_HEVC_ENFORCE_HRD, AMF_PARAM_DYNAMIC, L"Enforce HRD (true, false default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_FILLER_DATA_ENABLE, AMF_PARAM_DYNAMIC, L"Filler Data Enable (true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_TARGET_BITRATE, AMF_PARAM_DYNAMIC, L"Target bit rate (in bits, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_PEAK_BITRATE, AMF_PARAM_DYNAMIC, L"Peak bit rate (in bits, default = depends on USAGE)");

    SetParamType(AMF_VIDEO_ENCODER_HEVC_MAX_AU_SIZE, AMF_PARAM_DYNAMIC, L"Max AU Size (in bits, default = 0)");

    SetParamType(AMF_VIDEO_ENCODER_HEVC_MIN_QP_I, AMF_PARAM_DYNAMIC, L"Min QP I frame (integer 0-51, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MAX_QP_I, AMF_PARAM_DYNAMIC, L"Max QP I frame (integer 0-51, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MIN_QP_P, AMF_PARAM_DYNAMIC, L"Min QP I frame (integer 0-51, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MAX_QP_P, AMF_PARAM_DYNAMIC, L"Max QP I frame (integer 0-51, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_QP_I, AMF_PARAM_DYNAMIC, L"QP I (integer 0-51, default = 22)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_QP_P, AMF_PARAM_DYNAMIC, L"QP P (integer 0-51, default = 22)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_SKIP_FRAME_ENABLE, AMF_PARAM_DYNAMIC, L"Rate Control Based Frame Skip (true, false default =  depends on USAGE)");

    SetParamType(AMF_VIDEO_ENCODER_HEVC_INPUT_HDR_METADATA, AMF_PARAM_DYNAMIC, L"AMFBuffer containing AMFHDRMetadata; default NULL");

    // ------------- Encoder params per frame ---------------
    SetParamType(AMF_VIDEO_ENCODER_HEVC_END_OF_SEQUENCE, AMF_PARAM_FRAME, L"bool; default = false; generate end of sequence");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_FORCE_PICTURE_TYPE, AMF_PARAM_FRAME, L"amf_int64(AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_ENUM); default = AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_NONE; generate particular picture type");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_INSERT_AUD, AMF_PARAM_FRAME, L"Insert AUD (true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_INSERT_HEADER, AMF_PARAM_FRAME, L"Insert Header (true, false default =  false)");

    SetParamType(AMF_VIDEO_ENCODER_HEVC_MARK_CURRENT_WITH_LTR_INDEX, AMF_PARAM_FRAME, L"Mark With LTR Index (integer, default -1)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_FORCE_LTR_REFERENCE_BITFIELD, AMF_PARAM_FRAME, L"Force LTR Reference Bitfield (bitfield default = 0)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_ROI_DATA, AMF_PARAM_FRAME, L"2D AMFSurface, surface format: AMF_SURFACE_GRAY32");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_REFERENCE_PICTURE, AMF_PARAM_FRAME, L"AMFInterface(AMFSurface); surface used for frame injection");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_PSNR_FEEDBACK, AMF_PARAM_FRAME, L"amf_bool; default = false; Signal encoder to calculate PSNR score");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_SSIM_FEEDBACK, AMF_PARAM_FRAME, L"amf_bool; default = false; Signal encoder to calculate SSIM score");

    SetParamType(AMF_VIDEO_ENCODER_HEVC_ENABLE_SMART_ACCESS_VIDEO, AMF_PARAM_STATIC, L"amf_bool; default = false; true = enables smart access video feature");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MULTI_HW_INSTANCE_ENCODE, AMF_PARAM_STATIC, L"amf_bool; flag to enable multi VCN encode.");

    SetParamTypePA();

    return RGY_ERR_NONE;
}

RGY_ERR AMFParams::SetParamTypeAV1() {
    // Encoder Engine Settings
    SetParamType(AMF_VIDEO_ENCODER_AV1_ENCODER_INSTANCE_INDEX,                 AMF_PARAM_STATIC, L"amf_int64; default = 0; selected HW instance idx. The number of instances is queried by using AMF_VIDEO_ENCODER_AV1_CAP_NUM_OF_HW_INSTANCES");
    SetParamType(AMF_VIDEO_ENCODER_AV1_ENCODING_LATENCY_MODE,                  AMF_PARAM_STATIC, L"amf_int64(AMF_VIDEO_ENCODER_AV1_ENCODING_LATENCY_MODE_ENUM); default = depends on USAGE; The encoding latency mode.");
    SetParamType(AMF_VIDEO_ENCODER_AV1_QUERY_TIMEOUT,                          AMF_PARAM_STATIC, L"amf_int64; default = 0 (no wait); timeout for QueryOutput call in ms.");

    // Usage Settings
    SetParamType(AMF_VIDEO_ENCODER_AV1_USAGE,                                  AMF_PARAM_ENCODER_USAGE, L"amf_int64(AMF_VIDEO_ENCODER_AV1_USAGE_ENUM); default = N/A; Encoder usage. fully configures parameter set.");

    // Session Configuration
    SetParamType(AMF_VIDEO_ENCODER_AV1_FRAMESIZE,                              AMF_PARAM_STATIC, L"AMFSize; default = 0,0; Frame size");
    SetParamType(AMF_VIDEO_ENCODER_AV1_COLOR_BIT_DEPTH,                        AMF_PARAM_STATIC, L"amf_int64(AMF_COLOR_BIT_DEPTH_ENUM); default = AMF_COLOR_BIT_DEPTH_8");
    SetParamType(AMF_VIDEO_ENCODER_AV1_PROFILE,                                AMF_PARAM_STATIC, L"amf_int64(AMF_VIDEO_ENCODER_AV1_PROFILE_ENUM) ; default = depends on USAGE; the codec profile of the coded bitstream");
    SetParamType(AMF_VIDEO_ENCODER_AV1_LEVEL,                                  AMF_PARAM_STATIC, L"amf_int64 (AMF_VIDEO_ENCODER_AV1_LEVEL_ENUM); default = depends on USAGE; the codec level of the coded bitstream");
    SetParamType(AMF_VIDEO_ENCODER_AV1_TILES_PER_FRAME,                        AMF_PARAM_STATIC, L"amf_int64; default = 1; Number of tiles Per Frame. This is treated as suggestion. The actual number of tiles might be different due to compliance or encoder limitation.");
    SetParamType(AMF_VIDEO_ENCODER_AV1_QUALITY_PRESET,                         AMF_PARAM_STATIC, L"amf_int64(AMF_VIDEO_ENCODER_AV1_QUALITY_PRESET_ENUM); default = depends on USAGE; Quality Preset");

    // Codec Configuration
    SetParamType(AMF_VIDEO_ENCODER_AV1_SCREEN_CONTENT_TOOLS,                   AMF_PARAM_STATIC, L"bool; default = depends on USAGE; If true, allow enabling screen content tools by AMF_VIDEO_ENCODER_AV1_PALETTE_MODE_ENABLE and AMF_VIDEO_ENCODER_AV1_FORCE_INTEGER_MV; if false, all screen content tools are disabled.");
    SetParamType(AMF_VIDEO_ENCODER_AV1_ORDER_HINT,                             AMF_PARAM_STATIC, L"bool; default = depends on USAGE; If true, code order hint; if false, don't code order hint");
    SetParamType(AMF_VIDEO_ENCODER_AV1_FRAME_ID,                               AMF_PARAM_STATIC, L"bool; default = depends on USAGE; If true, code frame id; if false, don't code frame id");
    SetParamType(AMF_VIDEO_ENCODER_AV1_TILE_GROUP_OBU,                         AMF_PARAM_STATIC, L"bool; default = depends on USAGE; If true, code FrameHeaderObu + TileGroupObu and each TileGroupObu contains one tile; if false, code FrameObu.");
    SetParamType(AMF_VIDEO_ENCODER_AV1_CDEF_MODE,                              AMF_PARAM_STATIC, L"amd_int64(AMF_VIDEO_ENCODER_AV1_CDEF_MODE_ENUM); default = depends on USAGE; Cdef mode");
    SetParamType(AMF_VIDEO_ENCODER_AV1_ERROR_RESILIENT_MODE,                   AMF_PARAM_STATIC, L"bool; default = depends on USAGE; If true, enable error resilient mode; if false, disable error resilient mode");

    // Rate Control and Quality Enhancement
    SetParamType(AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD,                    AMF_PARAM_STATIC, L"amf_int64(AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_METHOD_ENUM); default = depends on USAGE; Rate Control Method");
    SetParamType(AMF_VIDEO_ENCODER_AV1_QVBR_QUALITY_LEVEL,                     AMF_PARAM_STATIC, L"amf_int64; default = 23; QVBR quality level; range = 1-51");
    SetParamType(AMF_VIDEO_ENCODER_AV1_INITIAL_VBV_BUFFER_FULLNESS,            AMF_PARAM_STATIC, L"amf_int64; default = depends on USAGE; Initial VBV Buffer Fullness 0=0% 64=100%");

    // Alignment Mode Configuration
    SetParamType(AMF_VIDEO_ENCODER_AV1_ALIGNMENT_MODE,                         AMF_PARAM_STATIC, L"amf_int64(AMF_VIDEO_ENCODER_AV1_ALIGNMENT_MODE_ENUM); default = AMF_VIDEO_ENCODER_AV1_ALIGNMENT_MODE_64X16_ONLY; Alignment Mode.");

    SetParamType(AMF_VIDEO_ENCODER_AV1_PRE_ANALYSIS_ENABLE,                    AMF_PARAM_STATIC, L"bool; default = depends on USAGE; If true, enables the pre-analysis module. Refer to AMF Video PreAnalysis API reference for more details. If false, disable the pre-analysis module.");
    SetParamType(AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_PREENCODE,                 AMF_PARAM_STATIC, L"bool; default = depends on USAGE; If true, enables pre-encode assist in rate control; if false, disables pre-encode assist in rate control.");
    SetParamType(AMF_VIDEO_ENCODER_AV1_HIGH_MOTION_QUALITY_BOOST,              AMF_PARAM_STATIC, L"bool; default = depends on USAGE; If true, enable high motion quality boost mode; if false, disable high motion quality boost mode.");
    SetParamType(AMF_VIDEO_ENCODER_AV1_AQ_MODE,                                AMF_PARAM_STATIC, L"amd_int64(AMF_VIDEO_ENCODER_AV1_AQ_MODE_ENUM); default = depends on USAGE; AQ mode");

    // Picture Management Configuration
    SetParamType(AMF_VIDEO_ENCODER_AV1_MAX_NUM_TEMPORAL_LAYERS,                AMF_PARAM_STATIC, L"amf_int64; default = depends on USAGE; Max number of temporal layers might be enabled. The maximum value can be queried from AMF_VIDEO_ENCODER_AV1_CAP_MAX_NUM_TEMPORAL_LAYERS");
    SetParamType(AMF_VIDEO_ENCODER_AV1_MAX_LTR_FRAMES,                         AMF_PARAM_STATIC, L"amf_int64; default = depends on USAGE; Max number of LTR frames. The maximum value can be queried from AMF_VIDEO_ENCODER_AV1_CAP_MAX_NUM_LTR_FRAMES");
#define AMF_VIDEO_ENCODER_AV1_LTR_MODE                              L"Av1LTRMode"                       // amf_int64(AMF_VIDEO_ENCODER_AV1_LTR_MODE_ENUM); default = AMF_VIDEO_ENCODER_AV1_LTR_MODE_RESET_UNUSED; remove/keep unused LTRs (not specified in property AMF_VIDEO_ENCODER_AV1_FORCE_LTR_REFERENCE_BITFIELD)
    SetParamType(AMF_VIDEO_ENCODER_AV1_MAX_NUM_REFRAMES,                       AMF_PARAM_STATIC, L"amf_int64; default = 1; Maximum number of reference frames");

    // color conversion
    SetParamType(AMF_VIDEO_ENCODER_AV1_INPUT_HDR_METADATA,                     AMF_PARAM_STATIC, L"AMFBuffer containing AMFHDRMetadata; default NULL");

    // Miscellaneous
    SetParamType(AMF_VIDEO_ENCODER_AV1_EXTRA_DATA,                             AMF_PARAM_STATIC, L"AMFInterface* - > AMFBuffer*; buffer to retrieve coded sequence header");

    // Dynamic properties - can be set anytime ***

    // Codec Configuration
    SetParamType(AMF_VIDEO_ENCODER_AV1_PALETTE_MODE,                           AMF_PARAM_DYNAMIC, L"bool; default = depends on USAGE; If true, enable palette mode; if false, disable palette mode. Valid only when AMF_VIDEO_ENCODER_AV1_SCREEN_CONTENT_TOOLS is true.");
    SetParamType(AMF_VIDEO_ENCODER_AV1_FORCE_INTEGER_MV,                       AMF_PARAM_DYNAMIC, L"bool; default = depends on USAGE; If true, enable force integer MV; if false, disable force integer MV. Valid only when AMF_VIDEO_ENCODER_AV1_SCREEN_CONTENT_TOOLS is true.");
    SetParamType(AMF_VIDEO_ENCODER_AV1_CDF_UPDATE,                             AMF_PARAM_DYNAMIC, L"bool; default = depends on USAGE; If true, enable CDF update; if false, disable CDF update.");
    SetParamType(AMF_VIDEO_ENCODER_AV1_CDF_FRAME_END_UPDATE_MODE,              AMF_PARAM_DYNAMIC, L"amd_int64(AMF_VIDEO_ENCODER_AV1_CDF_FRAME_END_UPDATE_MODE_ENUM); default = depends on USAGE; CDF frame end update mode");


    // Rate Control and Quality Enhancement
    SetParamType(AMF_VIDEO_ENCODER_AV1_VBV_BUFFER_SIZE,                        AMF_PARAM_DYNAMIC, L"amf_int64; default = depends on USAGE; VBV Buffer Size in bits");
    SetParamType(AMF_VIDEO_ENCODER_AV1_FRAMERATE,                              AMF_PARAM_DYNAMIC, L"AMFRate; default = depends on usage; Frame Rate");
    SetParamType(AMF_VIDEO_ENCODER_AV1_ENFORCE_HRD,                            AMF_PARAM_DYNAMIC, L"bool; default = depends on USAGE; If true, enforce HRD; if false, HRD is not enforced.");
    SetParamType(AMF_VIDEO_ENCODER_AV1_FILLER_DATA,                            AMF_PARAM_DYNAMIC, L"bool; default = depends on USAGE; If true, code filler data when needed; if false, don't code filler data.");
    SetParamType(AMF_VIDEO_ENCODER_AV1_TARGET_BITRATE,                         AMF_PARAM_DYNAMIC, L"amf_int64; default = depends on USAGE; Target bit rate in bits");
    SetParamType(AMF_VIDEO_ENCODER_AV1_PEAK_BITRATE,                           AMF_PARAM_DYNAMIC, L"amf_int64; default = depends on USAGE; Peak bit rate in bits");

    SetParamType(AMF_VIDEO_ENCODER_AV1_MAX_COMPRESSED_FRAME_SIZE,              AMF_PARAM_DYNAMIC, L"amf_int64; default = 0; Max compressed frame Size in bits. 0 - no limit");
    SetParamType(AMF_VIDEO_ENCODER_AV1_MIN_Q_INDEX_INTRA,                      AMF_PARAM_DYNAMIC, L"amf_int64; default = depends on USAGE; Min QIndex for intra frames; range = 1-255");
    SetParamType(AMF_VIDEO_ENCODER_AV1_MAX_Q_INDEX_INTRA,                      AMF_PARAM_DYNAMIC, L"amf_int64; default = depends on USAGE; Max QIndex for intra frames; range = 1-255");
    SetParamType(AMF_VIDEO_ENCODER_AV1_MIN_Q_INDEX_INTER,                      AMF_PARAM_DYNAMIC, L"amf_int64; default = depends on USAGE; Min QIndex for inter frames; range = 1-255");
    SetParamType(AMF_VIDEO_ENCODER_AV1_MAX_Q_INDEX_INTER,                      AMF_PARAM_DYNAMIC, L"amf_int64; default = depends on USAGE; Max QIndex for inter frames; range = 1-255");
    SetParamType(AMF_VIDEO_ENCODER_AV1_MIN_Q_INDEX_INTER_B,                    AMF_PARAM_DYNAMIC, L"amf_int64; default = depends on USAGE; Min QIndex for inter frames (bi-directional); range = 1-255");
    SetParamType(AMF_VIDEO_ENCODER_AV1_MAX_Q_INDEX_INTER_B,                    AMF_PARAM_DYNAMIC, L"amf_int64; default = depends on USAGE; Max QIndex for inter frames (bi-directional); range = 1-255");

    SetParamType(AMF_VIDEO_ENCODER_AV1_Q_INDEX_INTRA,                          AMF_PARAM_DYNAMIC, L"amf_int64; default = depends on USAGE; intra-frame QIndex; range = 1-255");
    SetParamType(AMF_VIDEO_ENCODER_AV1_Q_INDEX_INTER,                          AMF_PARAM_DYNAMIC, L"amf_int64; default = depends on USAGE; inter-frame QIndex; range = 1-255");
    SetParamType(AMF_VIDEO_ENCODER_AV1_Q_INDEX_INTER_B,                        AMF_PARAM_DYNAMIC, L"amf_int64; default = depends on USAGE; inter-frame QIndex (bi-directional); range = 1-255");

    SetParamType(AMF_VIDEO_ENCODER_AV1_RATE_CONTROL_SKIP_FRAME,                AMF_PARAM_DYNAMIC, L"bool; default = depends on USAGE; If true, rate control may code skip frame when needed; if false, rate control will not code skip frame.");

    // Picture Management Configuration
    SetParamType(AMF_VIDEO_ENCODER_AV1_GOP_SIZE,                               AMF_PARAM_DYNAMIC, L"amf_int64; default = depends on USAGE; GOP Size (distance between automatically inserted key frames). If 0, key frame will be inserted at first frame only. Note that GOP may be interrupted by AMF_VIDEO_ENCODER_AV1_FORCE_FRAME_TYPE.");
    SetParamType(AMF_VIDEO_ENCODER_AV1_HEADER_INSERTION_MODE,                  AMF_PARAM_DYNAMIC, L"amf_int64(AMF_VIDEO_ENCODER_AV1_HEADER_INSERTION_MODE_ENUM); default = depends on USAGE; sequence header insertion mode");
    SetParamType(AMF_VIDEO_ENCODER_AV1_SWITCH_FRAME_INSERTION_MODE,            AMF_PARAM_DYNAMIC, L"amf_int64(AMF_VIDEO_ENCODER_AV1_SWITCH_FRAME_INSERTION_MODE_ENUM); default = depends on USAGE; switch frame insertin mode");
    SetParamType(AMF_VIDEO_ENCODER_AV1_SWITCH_FRAME_INTERVAL,                  AMF_PARAM_DYNAMIC, L"amf_int64; default = depends on USAGE; the interval between two inserted switch frames. Valid only when AMF_VIDEO_ENCODER_AV1_SWITCH_FRAME_INSERTION_MODE is AMF_VIDEO_ENCODER_AV1_SWITCH_FRAME_INSERTION_MODE_FIXED_INTERVAL.");
    SetParamType(AMF_VIDEO_ENCODER_AV1_NUM_TEMPORAL_LAYERS,                    AMF_PARAM_DYNAMIC, L"amf_int64; default = depends on USAGE; Number of temporal layers. Can be changed at any time but the change is only applied when encoding next base layer frame.");

    SetParamType(AMF_VIDEO_ENCODER_AV1_INTRA_REFRESH_MODE,                     AMF_PARAM_DYNAMIC, L"amf_int64(AMF_VIDEO_ENCODER_AV1_INTRA_REFRESH_MODE_ENUM); default AMF_VIDEO_ENCODER_AV1_INTRA_REFRESH_MODE__DISABLED");
    SetParamType(AMF_VIDEO_ENCODER_AV1_INTRAREFRESH_STRIPES,                   AMF_PARAM_DYNAMIC, L"amf_int64; default = N/A; Valid only when intra refresh is enabled.");

    // color conversion
    SetParamType(AMF_VIDEO_ENCODER_AV1_INPUT_COLOR_PROFILE,                    AMF_PARAM_DYNAMIC, L"amf_int64(AMF_VIDEO_CONVERTER_COLOR_PROFILE_ENUM); default = AMF_VIDEO_CONVERTER_COLOR_PROFILE_UNKNOWN - mean AUTO by size");
    SetParamType(AMF_VIDEO_ENCODER_AV1_INPUT_TRANSFER_CHARACTERISTIC,          AMF_PARAM_DYNAMIC, L"amf_int64(AMF_COLOR_TRANSFER_CHARACTERISTIC_ENUM); default = AMF_COLOR_TRANSFER_CHARACTERISTIC_UNDEFINED, ISO/IEC 23001-8_2013 section 7.2 See VideoDecoderUVD.h for enum");
    SetParamType(AMF_VIDEO_ENCODER_AV1_INPUT_COLOR_PRIMARIES,                  AMF_PARAM_DYNAMIC, L"amf_int64(AMF_COLOR_PRIMARIES_ENUM); default = AMF_COLOR_PRIMARIES_UNDEFINED, ISO/IEC 23001-8_2013 section 7.1 See ColorSpace.h for enum");

    SetParamType(AMF_VIDEO_ENCODER_AV1_OUTPUT_COLOR_PROFILE,                   AMF_PARAM_DYNAMIC, L"amf_int64(AMF_VIDEO_CONVERTER_COLOR_PROFILE_ENUM); default = AMF_VIDEO_CONVERTER_COLOR_PROFILE_UNKNOWN - mean AUTO by size");
    SetParamType(AMF_VIDEO_ENCODER_AV1_OUTPUT_TRANSFER_CHARACTERISTIC,         AMF_PARAM_DYNAMIC, L"amf_int64(AMF_COLOR_TRANSFER_CHARACTERISTIC_ENUM); default = AMF_COLOR_TRANSFER_CHARACTERISTIC_UNDEFINED, ISO/IEC 23001-8_2013 ?7.2 See VideoDecoderUVD.h for enum");
    SetParamType(AMF_VIDEO_ENCODER_AV1_OUTPUT_COLOR_PRIMARIES,                 AMF_PARAM_DYNAMIC, L"amf_int64(AMF_COLOR_PRIMARIES_ENUM); default = AMF_COLOR_PRIMARIES_UNDEFINED, ISO/IEC 23001-8_2013 section 7.1 See ColorSpace.h for enum");


    // Frame encode parameters
    SetParamType(AMF_VIDEO_ENCODER_AV1_FORCE_FRAME_TYPE,                       AMF_PARAM_FRAME, L"amf_int64(AMF_VIDEO_ENCODER_AV1_FORCE_FRAME_TYPE_ENUM); default = AMF_VIDEO_ENCODER_AV1_FORCE_FRAME_TYPE_NONE; generate particular frame type");
    SetParamType(AMF_VIDEO_ENCODER_AV1_FORCE_INSERT_SEQUENCE_HEADER,           AMF_PARAM_FRAME, L"bool; default = false; If true, force insert sequence header with current frame;");
    SetParamType(AMF_VIDEO_ENCODER_AV1_MARK_CURRENT_WITH_LTR_INDEX,            AMF_PARAM_FRAME, L"amf_int64; default = N/A; Mark current frame with LTR index");
    SetParamType(AMF_VIDEO_ENCODER_AV1_FORCE_LTR_REFERENCE_BITFIELD,           AMF_PARAM_FRAME, L"amf_int64; default = 0; force LTR bit-field");
    SetParamType(AMF_VIDEO_ENCODER_AV1_ROI_DATA,                               AMF_PARAM_FRAME, L"2D AMFSurface, surface format: AMF_SURFACE_GRAY32");

    // Encode output parameters
    SetParamType(AMF_VIDEO_ENCODER_AV1_OUTPUT_FRAME_TYPE,                      AMF_PARAM_FRAME, L"amf_int64(AMF_VIDEO_ENCODER_AV1_OUTPUT_FRAME_TYPE_ENUM); default = N/A");
    SetParamType(AMF_VIDEO_ENCODER_AV1_OUTPUT_MARKED_LTR_INDEX,                AMF_PARAM_FRAME, L"amf_int64; default = N/A; Marked LTR index");
    SetParamType(AMF_VIDEO_ENCODER_AV1_OUTPUT_REFERENCED_LTR_INDEX_BITFIELD,   AMF_PARAM_FRAME, L"amf_int64; default = N/A; referenced LTR bit-field");

    SetParamType(AMF_VIDEO_ENCODER_AV1_ENABLE_SMART_ACCESS_VIDEO,              AMF_PARAM_STATIC, L"amf_bool; default = false; true = enables smart access video feature");
    SetParamType(AMF_VIDEO_ENCODER_AV1_MULTI_HW_INSTANCE_ENCODE,               AMF_PARAM_STATIC, L"amf_bool; flag to enable multi VCN encode.");


    SetParamType(AMF_VIDEO_ENCODER_AV1_MAX_CONSECUTIVE_BPICTURES,              AMF_PARAM_STATIC, L"amf_int64; default 0 or 127, determined by AMF_VIDEO_ENCODER_AV1_CAP_BFRAMES. Maximum number of consecutive B frames for B frame encoding");
    SetParamType(AMF_VIDEO_ENCODER_AV1_B_PIC_PATTERN,                          AMF_PARAM_STATIC, L"amf_int64; default 0. Number of B frames for B frame encoding.");
    SetParamType(AMF_VIDEO_ENCODER_AV1_ADAPTIVE_MINIGOP,                       AMF_PARAM_STATIC, L"amf_bool; default false. Adaptive miniGOP size for B frame encoding.");

    SetParamTypePA();

    return RGY_ERR_NONE;
}
