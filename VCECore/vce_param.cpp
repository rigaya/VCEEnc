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
#include "afs_stg.h"


VppAfs::VppAfs() :
    enable(false),
    tb_order(FILTER_DEFAULT_AFS_TB_ORDER),
    clip(scan_clip(FILTER_DEFAULT_AFS_CLIP_TB, FILTER_DEFAULT_AFS_CLIP_TB, FILTER_DEFAULT_AFS_CLIP_LR, FILTER_DEFAULT_AFS_CLIP_LR)),
    method_switch(FILTER_DEFAULT_AFS_METHOD_SWITCH),
    coeff_shift(FILTER_DEFAULT_AFS_COEFF_SHIFT),
    thre_shift(FILTER_DEFAULT_AFS_THRE_SHIFT),
    thre_deint(FILTER_DEFAULT_AFS_THRE_DEINT),
    thre_Ymotion(FILTER_DEFAULT_AFS_THRE_YMOTION),
    thre_Cmotion(FILTER_DEFAULT_AFS_THRE_CMOTION),
    analyze(FILTER_DEFAULT_AFS_ANALYZE),
    shift(FILTER_DEFAULT_AFS_SHIFT),
    drop(FILTER_DEFAULT_AFS_DROP),
    smooth(FILTER_DEFAULT_AFS_SMOOTH),
    force24(FILTER_DEFAULT_AFS_FORCE24),
    tune(FILTER_DEFAULT_AFS_TUNE),
    rff(FILTER_DEFAULT_AFS_RFF),
    timecode(FILTER_DEFAULT_AFS_TIMECODE),
    log(FILTER_DEFAULT_AFS_LOG) {
    check();
}

bool VppAfs::operator==(const VppAfs &x) const {
    return enable == x.enable
        && tb_order == x.tb_order
        && clip.bottom == x.clip.bottom
        && clip.left == x.clip.left
        && clip.top == x.clip.top
        && clip.right == x.clip.right
        && method_switch == x.method_switch
        && coeff_shift == x.coeff_shift
        && thre_shift == x.thre_shift
        && thre_deint == x.thre_deint
        && thre_Ymotion == x.thre_Ymotion
        && thre_Cmotion == x.thre_Cmotion
        && analyze == x.analyze
        && shift == x.shift
        && drop == x.drop
        && smooth == x.smooth
        && force24 == x.force24
        && tune == x.tune
        && rff == x.rff
        && timecode == x.timecode
        && log == x.log;
}
bool VppAfs::operator!=(const VppAfs &x) const {
    return !(*this == x);
}

void VppAfs::check() {
    if (!shift) {
        method_switch = 0;
        coeff_shift = 0;
    }
    drop &= shift;
    smooth &= drop;
}

void VppAfs::set_preset(int preset) {
    switch (preset) {
    case AFS_PRESET_DEFAULT: //デフォルト
        method_switch = FILTER_DEFAULT_AFS_METHOD_SWITCH;
        coeff_shift   = FILTER_DEFAULT_AFS_COEFF_SHIFT;
        thre_shift    = FILTER_DEFAULT_AFS_THRE_SHIFT;
        thre_deint    = FILTER_DEFAULT_AFS_THRE_DEINT;
        thre_Ymotion  = FILTER_DEFAULT_AFS_THRE_YMOTION;
        thre_Cmotion  = FILTER_DEFAULT_AFS_THRE_CMOTION;
        analyze       = FILTER_DEFAULT_AFS_ANALYZE;
        shift         = FILTER_DEFAULT_AFS_SHIFT;
        drop          = FILTER_DEFAULT_AFS_DROP;
        smooth        = FILTER_DEFAULT_AFS_SMOOTH;
        force24       = FILTER_DEFAULT_AFS_FORCE24;
        tune          = FILTER_DEFAULT_AFS_TUNE;
        break;
    case AFS_PRESET_TRIPLE: //動き重視
        method_switch = 0;
        coeff_shift   = 192;
        thre_shift    = 128;
        thre_deint    = 48;
        thre_Ymotion  = 112;
        thre_Cmotion  = 224;
        analyze       = 1;
        shift         = false;
        drop          = false;
        smooth        = false;
        force24       = false;
        tune          = false;
        break;
    case AFS_PRESET_DOUBLE://二重化
        method_switch = 0;
        coeff_shift   = 192;
        thre_shift    = 128;
        thre_deint    = 48;
        thre_Ymotion  = 112;
        thre_Cmotion  = 224;
        analyze       = 2;
        shift         = true;
        drop          = true;
        smooth        = true;
        force24       = false;
        tune          = false;
        break;
    case AFS_PRESET_ANIME: //映画/アニメ
        method_switch = 64;
        coeff_shift   = 128;
        thre_shift    = 128;
        thre_deint    = 48;
        thre_Ymotion  = 112;
        thre_Cmotion  = 224;
        analyze       = 3;
        shift         = true;
        drop          = true;
        smooth        = true;
        force24       = false;
        tune          = false;
        break;
    case AFS_PRESET_MIN_AFTERIMG:      //残像最小化
        method_switch = 0;
        coeff_shift   = 192;
        thre_shift    = 128;
        thre_deint    = 48;
        thre_Ymotion  = 112;
        thre_Cmotion  = 224;
        analyze       = 4;
        shift         = true;
        drop          = true;
        smooth        = true;
        force24       = false;
        tune          = false;
        break;
    case AFS_PRESET_FORCE24_SD:        //24fps固定
        method_switch = 64;
        coeff_shift   = 128;
        thre_shift    = 128;
        thre_deint    = 48;
        thre_Ymotion  = 112;
        thre_Cmotion  = 224;
        analyze       = 3;
        shift         = true;
        drop          = true;
        smooth        = false;
        force24       = true;
        tune          = false;
        break;
    case AFS_PRESET_FORCE24_HD:        //24fps固定 (HD)
        method_switch = 92;
        coeff_shift   = 192;
        thre_shift    = 448;
        thre_deint    = 48;
        thre_Ymotion  = 112;
        thre_Cmotion  = 224;
        analyze       = 3;
        shift         = true;
        drop          = true;
        smooth        = true;
        force24       = true;
        tune          = false;
        break;
    case AFS_PRESET_FORCE30:           //30fps固定
        method_switch = 92;
        coeff_shift   = 192;
        thre_shift    = 448;
        thre_deint    = 48;
        thre_Ymotion  = 112;
        thre_Cmotion  = 224;
        analyze       = 3;
        shift         = false;
        drop          = false;
        smooth        = false;
        force24       = false;
        tune          = false;
        break;
    default:
        break;
    }
}

int VppAfs::read_afs_inifile(const TCHAR *inifile) {
    if (!PathFileExists(inifile)) {
        return 1;
    }
    const auto filename = tchar_to_string(inifile);
    const auto section = AFS_STG_SECTION;

    clip.top      = GetPrivateProfileIntA(section, AFS_STG_UP, clip.top, filename.c_str());
    clip.bottom   = GetPrivateProfileIntA(section, AFS_STG_BOTTOM, clip.bottom, filename.c_str());
    clip.left     = GetPrivateProfileIntA(section, AFS_STG_LEFT, clip.left, filename.c_str());
    clip.right    = GetPrivateProfileIntA(section, AFS_STG_RIGHT, clip.right, filename.c_str());
    method_switch = GetPrivateProfileIntA(section, AFS_STG_METHOD_WATERSHED, method_switch, filename.c_str());
    coeff_shift   = GetPrivateProfileIntA(section, AFS_STG_COEFF_SHIFT, coeff_shift, filename.c_str());
    thre_shift    = GetPrivateProfileIntA(section, AFS_STG_THRE_SHIFT, thre_shift, filename.c_str());
    thre_deint    = GetPrivateProfileIntA(section, AFS_STG_THRE_DEINT, thre_deint, filename.c_str());
    thre_Ymotion  = GetPrivateProfileIntA(section, AFS_STG_THRE_Y_MOTION, thre_Ymotion, filename.c_str());
    thre_Cmotion  = GetPrivateProfileIntA(section, AFS_STG_THRE_C_MOTION, thre_Cmotion, filename.c_str());
    analyze       = GetPrivateProfileIntA(section, AFS_STG_MODE, analyze, filename.c_str());

    shift    = 0 != GetPrivateProfileIntA(section, AFS_STG_FIELD_SHIFT, shift, filename.c_str());
    drop     = 0 != GetPrivateProfileIntA(section, AFS_STG_DROP, drop, filename.c_str());
    smooth   = 0 != GetPrivateProfileIntA(section, AFS_STG_SMOOTH, smooth, filename.c_str());
    force24  = 0 != GetPrivateProfileIntA(section, AFS_STG_FORCE24, force24, filename.c_str());
    rff      = 0 != GetPrivateProfileIntA(section, AFS_STG_RFF, rff, filename.c_str());
    log      = 0 != GetPrivateProfileIntA(section, AFS_STG_LOG, log, filename.c_str());
    // GetPrivateProfileIntA(section, AFS_STG_DETECT_SC, fp->check[4], filename.c_str());
    tune     = 0 != GetPrivateProfileIntA(section, AFS_STG_TUNE_MODE, tune, filename.c_str());
    // GetPrivateProfileIntA(section, AFS_STG_LOG_SAVE, fp->check[6], filename.c_str());
    // GetPrivateProfileIntA(section, AFS_STG_TRACE_MODE, fp->check[7], filename.c_str());
    // GetPrivateProfileIntA(section, AFS_STG_REPLAY_MODE, fp->check[8], filename.c_str());
    // GetPrivateProfileIntA(section, AFS_STG_YUY2UPSAMPLE, fp->check[9], filename.c_str());
    // GetPrivateProfileIntA(section, AFS_STG_THROUGH_MODE, fp->check[10], filename.c_str());

    // GetPrivateProfileIntA(section, AFS_STG_PROC_MODE, g_afs.ex_data.proc_mode, filename.c_str());
    return 0;
}

tstring VppAfs::print() const {
#define ON_OFF(b) ((b) ? _T("on") : _T("off"))
    return strsprintf(
        _T("afs: clip(T %d, B %d, L %d, R %d), switch %d, coeff_shift %d\n")
        _T("                    thre(shift %d, deint %d, Ymotion %d, Cmotion %d)\n")
        _T("                    level %d, shift %s, drop %s, smooth %s, force24 %s\n")
        _T("                    tune %s, tb_order %d(%s), rff %s, timecode %s, log %s"),
        clip.top, clip.bottom, clip.left, clip.right,
        method_switch, coeff_shift,
        thre_shift, thre_deint, thre_Ymotion, thre_Cmotion,
        analyze, ON_OFF(shift), ON_OFF(drop), ON_OFF(smooth), ON_OFF(force24),
        ON_OFF(tune), tb_order, tb_order ? _T("tff") : _T("bff"), ON_OFF(rff), ON_OFF(timecode), ON_OFF(log));
#undef ON_OFF
}

VCEVppParam::VCEVppParam() :
    resize(RGY_VPP_RESIZE_AUTO),
    afs() {

}

VCEParamPA::VCEParamPA() :
    sc(true),
    scSensitivity(AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY_MEDIUM),
    ss(true),
    ssSensitivity(AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY_HIGH),
    activityType(AMF_PA_ACTIVITY_Y),
    initQPSC(AMF_PA_INITQPSC_AUTO),
    maxQPBeforeForceSkip(35),
    CAQStrength(AMF_PA_CAQ_STRENGTH_MEDIUM) {

};


VCEParam::VCEParam() :
    input(),
    common(),
    ctrl(),
    codec(RGY_CODEC_H264),
    codecParam(),
    deviceID(0),
    interopD3d9(false),
    interopD3d11(true),
    par(),
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
    pa(),
    vui(),
    bVBAQ(false),
    preAnalysis(false),
    vpp() {
    codecParam[RGY_CODEC_H264].nLevel   = 0;
    codecParam[RGY_CODEC_H264].nProfile = list_avc_profile[2].value;
    codecParam[RGY_CODEC_HEVC].nLevel   = 0;
    codecParam[RGY_CODEC_HEVC].nProfile = AMF_VIDEO_ENCODER_HEVC_PROFILE_MAIN;
    codecParam[RGY_CODEC_HEVC].nTier    = AMF_VIDEO_ENCODER_HEVC_TIER_MAIN;
    par[0] = par[1] = 0;
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
            const auto ret = storage->SetProperty(name.c_str(), value);
            if (ret != AMF_OK) {
                if (pLog) {
                    pLog->write(RGY_LOG_ERROR, _T("storage->SetProperty(%s)=%s failed: %s.\n"),
                        wstring_to_tstring(name).c_str(),
                        wstring_to_tstring(value.ToWString().c_str()).c_str(),
                        get_err_mes(err_to_rgy(ret)));
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


    // ------------- PA parameters ---------------
    SetParamType(AMF_VIDEO_ENCODER_PRE_ANALYSIS_ENABLE, AMF_PARAM_STATIC, L"Enable PA (true, false default =  false)");

    SetParamType(AMF_PA_ENGINE_TYPE, AMF_PARAM_STATIC, L"Engine Type (DX11, OPENCL default = OPENCL)");

    SetParamType(AMF_PA_SCENE_CHANGE_DETECTION_ENABLE, AMF_PARAM_DYNAMIC, L"Scene Change Detection Enable (true, false default =  true)");
    SetParamType(AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY, AMF_PARAM_DYNAMIC, L"Scene Change Detection Sensitivity (LOW, MEDIUM, HIGH default = MEDIUM)");
    SetParamType(AMF_PA_STATIC_SCENE_DETECTION_ENABLE, AMF_PARAM_DYNAMIC, L"Static Scene Detection Enable (true, false default =  true)");
    SetParamType(AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY, AMF_PARAM_DYNAMIC, L"Scene Change Detection Sensitivity (LOW, MEDIUM, HIGH default = HIGH)");
    SetParamType(AMF_PA_ACTIVITY_TYPE, AMF_PARAM_DYNAMIC, L"Activity Type (Y, YUV default = Y)");

    SetParamType(AMF_PA_INITIAL_QP_AFTER_SCENE_CHANGE, AMF_PARAM_DYNAMIC, L"QP After Scene Change (integer 0-51, default = 0)");
    SetParamType(AMF_PA_MAX_QP_BEFORE_FORCE_SKIP, AMF_PARAM_DYNAMIC, L"Max QP Before Force Skip (integer 0-51, default = 35)");
    //  SetParamType(AMF_IN_FRAME_QP_FILTERING_STRENGTH, AMF_PARAM_DYNAMIC, L"In Frame QP Filtering Strength (integer 0-255, default = 0)");
    //  SetParamType(AMF_BETWEEN_FRAME_QP_FILTERING_STRENGTH, AMF_PARAM_DYNAMIC, L"Between Frame QP Filtering Strength (integer 0-255, default = 0)");
    SetParamType(AMF_PA_CAQ_STRENGTH, AMF_PARAM_DYNAMIC, L"CAQ Strength (LOW, MEDIUM, HIGH default = MEDIUM)");

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

    // ------------- PA parameters ---------------
    SetParamType(AMF_VIDEO_ENCODER_HEVC_PRE_ANALYSIS_ENABLE, AMF_PARAM_STATIC, L"Enable PA (true, false default =  false)");

    SetParamType(AMF_PA_ENGINE_TYPE, AMF_PARAM_STATIC, L"Engine Type (DX11, OPENCL default = OPENCL)");

    SetParamType(AMF_PA_SCENE_CHANGE_DETECTION_ENABLE, AMF_PARAM_DYNAMIC, L"Scene Change Detection Enable (true, false default =  true)");
    SetParamType(AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY, AMF_PARAM_DYNAMIC, L"Scene Change Detection Sensitivity (LOW, MEDIUM, HIGH default = MEDIUM)");
    SetParamType(AMF_PA_STATIC_SCENE_DETECTION_ENABLE, AMF_PARAM_DYNAMIC, L"Static Scene Detection Enable (true, false default =  true)");
    SetParamType(AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY, AMF_PARAM_DYNAMIC, L"Scene Change Detection Sensitivity (LOW, MEDIUM, HIGH default = HIGH)");
    SetParamType(AMF_PA_ACTIVITY_TYPE, AMF_PARAM_DYNAMIC, L"Activity Type (Y, YUV default = Y)");

    SetParamType(AMF_PA_INITIAL_QP_AFTER_SCENE_CHANGE, AMF_PARAM_DYNAMIC, L"QP After Scene Change (integer 0-51, default = 0)");
    SetParamType(AMF_PA_MAX_QP_BEFORE_FORCE_SKIP, AMF_PARAM_DYNAMIC, L"Max QP Before Force Skip (integer 0-51, default = 35)");
    //  SetParamType(AMF_IN_FRAME_QP_FILTERING_STRENGTH, AMF_PARAM_DYNAMIC, L"In Frame QP Filtering Strength (integer 0-255, default = 0)");
    //  SetParamType(AMF_BETWEEN_FRAME_QP_FILTERING_STRENGTH, AMF_PARAM_DYNAMIC, L"Between Frame QP Filtering Strength (integer 0-255, default = 0)");
    SetParamType(AMF_PA_CAQ_STRENGTH, AMF_PARAM_DYNAMIC, L"CAQ Strength (LOW, MEDIUM, HIGH default = MEDIUM)");
    return RGY_ERR_NONE;
}
