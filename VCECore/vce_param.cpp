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

ColorspaceConv::ColorspaceConv() :
    from(),
    to(),
    sdr_source_peak(FILTER_DEFAULT_COLORSPACE_NOMINAL_SOURCE_PEAK),
    approx_gamma(false),
    scene_ref(false) {

}
bool ColorspaceConv::operator==(const ColorspaceConv &x) const {
    return from == x.from
        && to == x.to
        && sdr_source_peak == x.sdr_source_peak
        && approx_gamma == x.approx_gamma
        && scene_ref == x.scene_ref;
}
bool ColorspaceConv::operator!=(const ColorspaceConv &x) const {
    return !(*this == x);
}

TonemapHable::TonemapHable() :
    a(FILTER_DEFAULT_HDR2SDR_HABLE_A),
    b(FILTER_DEFAULT_HDR2SDR_HABLE_B),
    c(FILTER_DEFAULT_HDR2SDR_HABLE_C),
    d(FILTER_DEFAULT_HDR2SDR_HABLE_D),
    e(FILTER_DEFAULT_HDR2SDR_HABLE_E),
    f(FILTER_DEFAULT_HDR2SDR_HABLE_F) {}

bool TonemapHable::operator==(const TonemapHable &x) const {
    return a == x.a
        && b == x.b
        && c == x.c
        && d == x.d
        && e == x.e
        && f == x.f;
}
bool TonemapHable::operator!=(const TonemapHable &x) const {
    return !(*this == x);
}
TonemapMobius::TonemapMobius() :
    transition(FILTER_DEFAULT_HDR2SDR_MOBIUS_TRANSITION),
    peak(FILTER_DEFAULT_HDR2SDR_MOBIUS_PEAK) {
}
bool TonemapMobius::operator==(const TonemapMobius &x) const {
    return transition == x.transition
        &&peak == x.peak;
}
bool TonemapMobius::operator!=(const TonemapMobius &x) const {
    return !(*this == x);
}
TonemapReinhard::TonemapReinhard() :
    contrast(FILTER_DEFAULT_HDR2SDR_REINHARD_CONTRAST),
    peak(FILTER_DEFAULT_HDR2SDR_REINHARD_PEAK) {
}
bool TonemapReinhard::operator==(const TonemapReinhard &x) const {
    return contrast == x.contrast
        &&peak == x.peak;
}
bool TonemapReinhard::operator!=(const TonemapReinhard &x) const {
    return !(*this == x);
}

HDR2SDRParams::HDR2SDRParams() :
    tonemap(HDR2SDR_DISABLED),
    hable(),
    mobius(),
    reinhard(),
    ldr_nits(FILTER_DEFAULT_COLORSPACE_LDRNITS),
    hdr_source_peak(FILTER_DEFAULT_COLORSPACE_HDR_SOURCE_PEAK),
    desat_base(FILTER_DEFAULT_HDR2SDR_DESAT_BASE),
    desat_strength(FILTER_DEFAULT_HDR2SDR_DESAT_STRENGTH),
    desat_exp(FILTER_DEFAULT_HDR2SDR_DESAT_EXP) {

}
bool HDR2SDRParams::operator==(const HDR2SDRParams &x) const {
    return tonemap == x.tonemap
        && hable == x.hable
        && mobius == x.mobius
        && reinhard == x.reinhard;
}
bool HDR2SDRParams::operator!=(const HDR2SDRParams &x) const {
    return !(*this == x);
}

VppColorspace::VppColorspace() :
    enable(false),
    hdr2sdr(),
    convs() {

}

bool VppColorspace::operator==(const VppColorspace &x) const {
    if (enable != x.enable
        || x.hdr2sdr != this->hdr2sdr
        || x.convs.size() != this->convs.size()) {
        return false;
    }
    for (size_t i = 0; i < x.convs.size(); i++) {
        if (x.convs[i].from != this->convs[i].from
            || x.convs[i].to != this->convs[i].to) {
            return false;
        }
    }
    return true;
}
bool VppColorspace::operator!=(const VppColorspace &x) const {
    return !(*this == x);
}

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
    if (!rgy_file_exists(inifile)) {
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

VppNnedi::VppNnedi() :
    enable(false),
    field(VPP_NNEDI_FIELD_USE_AUTO),
    nns(32),
    nsize(VPP_NNEDI_NSIZE_32x4),
    quality(VPP_NNEDI_QUALITY_FAST),
    precision(VPP_FP_PRECISION_AUTO),
    pre_screen(VPP_NNEDI_PRE_SCREEN_NEW_BLOCK),
    errortype(VPP_NNEDI_ETYPE_ABS),
    weightfile(_T("")) {

}

bool VppNnedi::isbob() {
    return field == VPP_NNEDI_FIELD_BOB_AUTO
        || field == VPP_NNEDI_FIELD_BOB_BOTTOM_TOP
        || field == VPP_NNEDI_FIELD_BOB_TOP_BOTTOM;
}

bool VppNnedi::operator==(const VppNnedi &x) const {
    return enable == x.enable
        && field == x.field
        && nns == x.nns
        && nsize == x.nsize
        && quality == x.quality
        && pre_screen == x.pre_screen
        && errortype == x.errortype
        && precision == x.precision
        && weightfile == x.weightfile;
}
bool VppNnedi::operator!=(const VppNnedi &x) const {
    return !(*this == x);
}

tstring VppNnedi::print() const {
    return strsprintf(
        _T("nnedi: field %s, nns %d, nsize %s, quality %s, prec %s\n")
        _T("                       pre_screen %s, errortype %s, weight \"%s\""),
        get_cx_desc(list_vpp_nnedi_field, field),
        nns,
        get_cx_desc(list_vpp_nnedi_nsize, nsize),
        get_cx_desc(list_vpp_nnedi_quality, quality),
        get_cx_desc(list_vpp_fp_prec, precision),
        get_cx_desc(list_vpp_nnedi_pre_screen, pre_screen),
        get_cx_desc(list_vpp_nnedi_error_type, errortype),
        ((weightfile.length()) ? weightfile.c_str() : _T("internal")));
}

VppDecimate::VppDecimate() :
    enable(false),
    cycle(FILTER_DEFAULT_DECIMATE_CYCLE),
    threDuplicate(FILTER_DEFAULT_DECIMATE_THRE_DUP),
    threSceneChange(FILTER_DEFAULT_DECIMATE_THRE_SC),
    blockX(FILTER_DEFAULT_DECIMATE_BLOCK_X),
    blockY(FILTER_DEFAULT_DECIMATE_BLOCK_Y),
    preProcessed(FILTER_DEFAULT_DECIMATE_PREPROCESSED),
    chroma(FILTER_DEFAULT_DECIMATE_CHROMA),
    log(FILTER_DEFAULT_DECIMATE_LOG) {

}

bool VppDecimate::operator==(const VppDecimate &x) const {
    return enable == x.enable
        && cycle == x.cycle
        && threDuplicate == x.threDuplicate
        && threSceneChange == x.threSceneChange
        && blockX == x.blockX
        && blockY == x.blockY
        && preProcessed == x.preProcessed
        && chroma == x.chroma
        && log == x.log;
}
bool VppDecimate::operator!=(const VppDecimate &x) const {
    return !(*this == x);
}

tstring VppDecimate::print() const {
    return strsprintf(_T("decimate: cycle %d, threDup %.2f, threSC %.2f\n")
        _T("                         block %dx%d, chroma %s, log %s"),
        cycle,
        threDuplicate, threSceneChange,
        blockX, blockY,
        /*preProcessed ? _T("on") : _T("off"),*/
        chroma ? _T("on") : _T("off"),
        log ? _T("on") : _T("off"));
}

VppPad::VppPad() :
    enable(false),
    left(0),
    top(0),
    right(0),
    bottom(0) {

}

bool VppPad::operator==(const VppPad& x) const {
    return enable == x.enable
        && left == x.left
        && top == x.top
        && right == x.right
        && bottom == x.bottom;
}
bool VppPad::operator!=(const VppPad& x) const {
    return !(*this == x);
}

tstring VppPad::print() const {
    return strsprintf(_T("(right=%d, left=%d, top=%d, bottom=%d)"),
        right, left, top, bottom);
}

VppKnn::VppKnn() :
    enable(false),
    radius(FILTER_DEFAULT_KNN_RADIUS),
    strength(FILTER_DEFAULT_KNN_STRENGTH),
    lerpC(FILTER_DEFAULT_KNN_LERPC),
    weight_threshold(FILTER_DEFAULT_KNN_WEIGHT_THRESHOLD),
    lerp_threshold(FILTER_DEFAULT_KNN_LERPC_THRESHOLD) {
}

bool VppKnn::operator==(const VppKnn &x) const {
    return enable == x.enable
        && radius == x.radius
        && strength == x.strength
        && lerpC == x.lerpC
        && weight_threshold == x.weight_threshold
        && lerp_threshold == x.lerp_threshold;
}
bool VppKnn::operator!=(const VppKnn &x) const {
    return !(*this == x);
}

tstring VppKnn::print() const {
    return strsprintf(
        _T("denoise(knn): radius %d, strength %.2f, lerp %.2f\n")
        _T("                              th_weight %.2f, th_lerp %.2f"),
        radius, strength, lerpC,
        weight_threshold, lerp_threshold);
}

VppPmd::VppPmd() :
    enable(false),
    strength(FILTER_DEFAULT_PMD_STRENGTH),
    threshold(FILTER_DEFAULT_PMD_THRESHOLD),
    applyCount(FILTER_DEFAULT_PMD_APPLY_COUNT),
    useExp(FILTER_DEFAULT_PMD_USE_EXP) {

}

bool VppPmd::operator==(const VppPmd& x) const {
    return enable == x.enable
        && strength == x.strength
        && threshold == x.threshold
        && applyCount == x.applyCount
        && useExp == x.useExp;
}
bool VppPmd::operator!=(const VppPmd& x) const {
    return !(*this == x);
}

tstring VppPmd::print() const {
    return strsprintf(_T("denoise(pmd): strength %d, threshold %d, apply %d, exp %d"),
        (int)strength, (int)threshold, applyCount, useExp);
}

VppSmooth::VppSmooth() :
    enable(false),
    quality(FILTER_DEFAULT_SMOOTH_QUALITY),
    qp(FILTER_DEFAULT_SMOOTH_QP),
    prec(VPP_FP_PRECISION_AUTO),
    useQPTable(false),
    strength(FILTER_DEFAULT_SMOOTH_STRENGTH),
    threshold(FILTER_DEFAULT_SMOOTH_THRESHOLD),
    bratio(FILTER_DEFAULT_SMOOTH_B_RATIO),
    maxQPTableErrCount(FILTER_DEFAULT_SMOOTH_MAX_QPTABLE_ERR) {

}

bool VppSmooth::operator==(const VppSmooth &x) const {
    return enable == x.enable
        && quality == x.quality
        && qp == x.qp
        && prec == x.prec
        && useQPTable == x.useQPTable
        && strength == x.strength
        && threshold == x.threshold
        && bratio == x.bratio
        && maxQPTableErrCount == x.maxQPTableErrCount;
}
bool VppSmooth::operator!=(const VppSmooth &x) const {
    return !(*this == x);
}

tstring VppSmooth::print() const {
    //return strsprintf(_T("smooth: quality %d, qp %d, threshold %.1f, strength %.1f, mode %d, use_bframe_qp %s"), quality, qp, threshold, strength, mode, use_bframe_qp ? _T("yes") : _T("no"));
    tstring str = strsprintf(_T("smooth: quality %d, qp %d, prec %s"), quality, qp, get_cx_desc(list_vpp_fp_prec, prec));
    if (useQPTable) {
        str += strsprintf(_T(", use QP table on"));
    }
    return str;
}

VppSubburn::VppSubburn() :
    enable(false),
    filename(),
    charcode(),
    fontsdir(),
    trackId(0),
    assShaping(1),
    scale(0.0),
    transparency_offset(0.0),
    brightness(FILTER_DEFAULT_TWEAK_BRIGHTNESS),
    contrast(FILTER_DEFAULT_TWEAK_CONTRAST),
    ts_offset(0.0),
    vid_ts_offset(true) {
}

bool VppSubburn::operator==(const VppSubburn &x) const {
    return enable == x.enable
        && filename == x.filename
        && charcode == x.charcode
        && fontsdir == x.fontsdir
        && trackId == x.trackId
        && assShaping == x.assShaping
        && scale == x.scale
        && transparency_offset == x.transparency_offset
        && brightness == x.brightness
        && contrast == x.contrast
        && ts_offset == x.ts_offset
        && vid_ts_offset == x.vid_ts_offset;
}
bool VppSubburn::operator!=(const VppSubburn &x) const {
    return !(*this == x);
}

tstring VppSubburn::print() const {
    tstring str = strsprintf(_T("subburn: %s, scale x%.2f"),
        (filename.length() > 0)
        ? filename.c_str()
        : strsprintf(_T("track #%d"), trackId).c_str(),
        scale);
    if (transparency_offset != 0.0) {
        str += strsprintf(_T(", transparency %.2f"), transparency_offset);
    }
    if (brightness != FILTER_DEFAULT_TWEAK_BRIGHTNESS) {
        str += strsprintf(_T(", brightness %.2f"), brightness);
    }
    if (contrast != FILTER_DEFAULT_TWEAK_CONTRAST) {
        str += strsprintf(_T(", contrast %.2f"), contrast);
    }
    if (ts_offset != 0.0) {
        str += strsprintf(_T(", ts_offset %.2f"), ts_offset);
    }
    if (!vid_ts_offset) {
        str += _T(", vid_ts_offset off");
    }
    return str;
}

VppUnsharp::VppUnsharp() :
    enable(false),
    radius(FILTER_DEFAULT_UNSHARP_RADIUS),
    weight(FILTER_DEFAULT_UNSHARP_WEIGHT),
    threshold(FILTER_DEFAULT_UNSHARP_THRESHOLD) {

}

bool VppUnsharp::operator==(const VppUnsharp &x) const {
    return enable == x.enable
        && radius == x.radius
        && weight == x.weight
        && threshold == x.threshold;
}
bool VppUnsharp::operator!=(const VppUnsharp &x) const {
    return !(*this == x);
}

tstring VppUnsharp::print() const {
    return strsprintf(_T("unsharp: radius %d, weight %.1f, threshold %.1f"),
        radius, weight, threshold);
}

VppEdgelevel::VppEdgelevel() :
    enable(false),
    strength(FILTER_DEFAULT_EDGELEVEL_STRENGTH),
    threshold(FILTER_DEFAULT_EDGELEVEL_THRESHOLD),
    black(FILTER_DEFAULT_EDGELEVEL_BLACK),
    white(FILTER_DEFAULT_EDGELEVEL_WHITE) {
}

bool VppEdgelevel::operator==(const VppEdgelevel &x) const {
    return enable == x.enable
        && strength == x.strength
        && threshold == x.threshold
        && black == x.black
        && white == x.white;
}
bool VppEdgelevel::operator!=(const VppEdgelevel &x) const {
    return !(*this == x);
}

tstring VppEdgelevel::print() const {
    return strsprintf(_T("edgelevel: strength %.1f, threshold %.1f, black %.1f, white %.1f"),
        strength, threshold, black, white);
}

VppWarpsharp::VppWarpsharp() :
    enable(false),
    threshold(FILTER_DEFAULT_WARPSHARP_THRESHOLD),
    blur(FILTER_DEFAULT_WARPSHARP_BLUR),
    type(FILTER_DEFAULT_WARPSHARP_TYPE),
    depth(FILTER_DEFAULT_WARPSHARP_DEPTH),
    chroma(FILTER_DEFAULT_WARPSHARP_CHROMA) {
}

bool VppWarpsharp::operator==(const VppWarpsharp& x) const {
    return enable == x.enable
        && threshold == x.threshold
        && blur == x.blur
        && type == x.type
        && depth == x.depth
        && chroma == x.chroma;
}
bool VppWarpsharp::operator!=(const VppWarpsharp& x) const {
    return !(*this == x);
}

tstring VppWarpsharp::print() const {
    return strsprintf(_T("warpsharp: threshold %.1f, blur %d, type %d, depth %.1f, chroma %d"),
        threshold, blur, type, depth, chroma);
}

VppTweak::VppTweak() :
    enable(false),
    brightness(FILTER_DEFAULT_TWEAK_BRIGHTNESS),
    contrast(FILTER_DEFAULT_TWEAK_CONTRAST),
    gamma(FILTER_DEFAULT_TWEAK_GAMMA),
    saturation(FILTER_DEFAULT_TWEAK_SATURATION),
    hue(FILTER_DEFAULT_TWEAK_HUE) {
}

bool VppTweak::operator==(const VppTweak &x) const {
    return enable == x.enable
        && brightness == x.brightness
        && contrast == x.contrast
        && gamma == x.gamma
        && saturation == x.saturation
        && hue == x.hue;
}
bool VppTweak::operator!=(const VppTweak &x) const {
    return !(*this == x);
}

tstring VppTweak::print() const {
    return strsprintf(_T("tweak: brightness %.2f, contrast %.2f, saturation %.2f, gamma %.2f, hue %.2f"),
        brightness, contrast, saturation, gamma, hue);
}

VppTransform::VppTransform() :
    enable(false),
    transpose(false),
    flipX(false),
    flipY(false) {
}

int VppTransform::rotate() const {
    if (transpose) {
        if (!flipY && flipX) {
            return 270;
        } else if (flipY && !flipX) {
            return 90;
        }
    } else if (flipY && flipX) {
        return 180;
    }
    return 0;
}

bool VppTransform::setRotate(int rotate) {
    switch (rotate) {
    case 90:
        transpose = true;
        flipY = true;
        break;
    case 180:
        flipX = true;
        flipY = true;
        break;
    case 270:
        transpose = true;
        flipX = true;
        break;
    default:
        return false;
    }
    return true;
}

bool VppTransform::operator==(const VppTransform &x) const {
    return enable == x.enable
        && transpose == x.transpose
        && flipX == x.flipX
        && flipY == x.flipY;
}
bool VppTransform::operator!=(const VppTransform &x) const {
    return !(*this == x);
}

tstring VppTransform::print() const {
#define ON_OFF(b) ((b) ? _T("on") : _T("off"))
    const auto rotation = rotate();
    if (rotation) {
        return strsprintf(_T("rotate: %d"), rotation);
    } else {
        return strsprintf(_T("transform: transpose %s, flipX %s, flipY %s"),
            ON_OFF(transpose), ON_OFF(flipX), ON_OFF(flipY));
    }
#undef ON_OFF
}

VppDeband::VppDeband() :
    enable(false),
    range(FILTER_DEFAULT_DEBAND_RANGE),
    threY(FILTER_DEFAULT_DEBAND_THRE_Y),
    threCb(FILTER_DEFAULT_DEBAND_THRE_CB),
    threCr(FILTER_DEFAULT_DEBAND_THRE_CR),
    ditherY(FILTER_DEFAULT_DEBAND_DITHER_Y),
    ditherC(FILTER_DEFAULT_DEBAND_DITHER_C),
    sample(FILTER_DEFAULT_DEBAND_MODE),
    seed(FILTER_DEFAULT_DEBAND_SEED),
    blurFirst(FILTER_DEFAULT_DEBAND_BLUR_FIRST),
    randEachFrame(FILTER_DEFAULT_DEBAND_RAND_EACH_FRAME) {

}

bool VppDeband::operator==(const VppDeband &x) const {
    return enable == x.enable
        && range == x.range
        && threY == x.threY
        && threCb == x.threCb
        && threCr == x.threCr
        && ditherY == x.ditherY
        && ditherC == x.ditherC
        && sample == x.sample
        && seed == x.seed
        && blurFirst == x.blurFirst
        && randEachFrame == x.randEachFrame;
}
bool VppDeband::operator!=(const VppDeband &x) const {
    return !(*this == x);
}

tstring VppDeband::print() const {
    return strsprintf(_T("deband: mode %d, range %d, threY %d, threCb %d, threCr %d\n")
        _T("                       ditherY %d, ditherC %d, blurFirst %s, randEachFrame %s"),
        sample, range,
        threY, threCb, threCr,
        ditherY, ditherC,
        blurFirst ? _T("yes") : _T("no"),
        randEachFrame ? _T("yes") : _T("no"));
}

VCEVppParam::VCEVppParam() :
    resize(RGY_VPP_RESIZE_AUTO),
    colorspace(),
    afs(),
    nnedi(),
    decimate(),
    pad(),
    knn(),
    pmd(),
    smooth(),
    subburn(),
    unsharp(),
    edgelevel(),
    warpsharp(),
    tweak(),
    transform(),
    deband() {

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
    CAQStrength(AMF_PA_CAQ_STRENGTH_MEDIUM) {

};


VCEParam::VCEParam() :
    input(),
    common(),
    ctrl(),
    codec(RGY_CODEC_H264),
    codecParam(),
    deviceID(-1),
    interopD3d9(false),
    interopD3d11(true),
    outputDepth(8),
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
    bEnforceHRD(false),
    nIDRPeriod(1),
    nGOPLen(0),
    nRefFrames(VCE_DEFAULT_REF_FRAMES),
    nLTRFrames(0),
    bFiller(false),
    pa(),
    pe(),
    bVBAQ(false),
    ssim(false),
    psnr(false),
    vpp() {
    codecParam[RGY_CODEC_H264].nLevel   = 0;
    codecParam[RGY_CODEC_H264].nProfile = list_avc_profile[2].value;
    codecParam[RGY_CODEC_HEVC].nLevel   = 0;
    codecParam[RGY_CODEC_HEVC].nProfile = AMF_VIDEO_ENCODER_HEVC_PROFILE_MAIN;
    codecParam[RGY_CODEC_HEVC].nTier    = AMF_VIDEO_ENCODER_HEVC_TIER_MAIN;
    par[0] = par[1] = 0;
    input.vui = VideoVUIInfo();
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
    SetParamType(AMF_VIDEO_ENCODER_MAX_LTR_FRAMES, AMF_PARAM_STATIC, L"Max Of LTR frames (integer, default = 0)");
    SetParamType(AMF_VIDEO_ENCODER_SCANTYPE, AMF_PARAM_STATIC, L"Scan Type (PROGRESSIVE, INTERLACED, default = PROGRESSIVE)");
    SetParamType(AMF_VIDEO_ENCODER_MAX_NUM_REFRAMES, AMF_PARAM_STATIC, L"Max Of Reference frames (integer, default = 4)");
    SetParamType(AMF_VIDEO_ENCODER_ASPECT_RATIO, AMF_PARAM_STATIC, L"Controls aspect ratio, defulat (1,1)");
    SetParamType(AMF_VIDEO_ENCODER_FULL_RANGE_COLOR, AMF_PARAM_STATIC, L"Inidicates that YUV input is (0,255) (bool, default = false)");
    SetParamType(AMF_VIDEO_ENCODER_LOWLATENCY_MODE, AMF_PARAM_STATIC, L"bool; default = false, enables low latency mode");
#pragma warning(push)
#pragma warning(disable:4995) //warning C4995: 'AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_PREANALYSIS_ENABLE': 名前が避けられた #pragma として記述されています。
    SetParamType(AMF_VIDEO_ENCODER_RATE_CONTROL_PREANALYSIS_ENABLE, AMF_PARAM_STATIC, L"Rate Control Preanalysis Enabled (EANBLED, DISABLED, default = DISABLED)");
#pragma warning(pop)

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
    SetParamType(AMF_VIDEO_ENCODER_FRAMERATE, AMF_PARAM_DYNAMIC, L"Frame Rate (num,den), default = depends on USAGE)");
    //SetParamType(AMF_VIDEO_ENCODER_WIDTH, AMF_PARAM_DYNAMIC, L"Frame width (integer, default = 0)");
    //SetParamType(AMF_VIDEO_ENCODER_HEIGHT, AMF_PARAM_DYNAMIC, L"Frame height (integer, default = 0)");

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
    SetParamType(AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD, AMF_PARAM_DYNAMIC, L"Rate Control Method (CQP, CBR, VBR, VBR_LAT default = depends on USAGE)");

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
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MAX_LTR_FRAMES, AMF_PARAM_STATIC, L"Max Of LTR frames (integer, default = 0)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_MAX_NUM_REFRAMES, AMF_PARAM_STATIC, L" Maximum number of reference frames default = 1");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET, AMF_PARAM_STATIC, L"Quality Preset (BALANCED, SPEED, QUALITY default = depends on USAGE)");
    //SetParamType(AMF_VIDEO_ENCODER_HEVC_EXTRADATA, AMF_PARAM_STATIC, L"AMFInterface* - > AMFBuffer*; SPS/PPS buffer - read-only");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_ASPECT_RATIO, AMF_PARAM_STATIC, L"AMFRatio; default = 1, 1");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_LOWLATENCY_MODE, AMF_PARAM_STATIC, L"bool; default = false, enables low latency mode");
#pragma warning(push)
#pragma warning(disable:4995) //warning C4995: 'AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_PREANALYSIS_ENABLE': 名前が避けられた #pragma として記述されています。
    SetParamType(AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_PREANALYSIS_ENABLE, AMF_PARAM_STATIC, L"Enable Preanalysis(true, false default = depends on USAGE)");
#pragma warning(pop)

    // Picture control properties
    SetParamType(AMF_VIDEO_ENCODER_HEVC_NUM_GOPS_PER_IDR, AMF_PARAM_STATIC, L"The frequency to insert IDR as start of a GOP. 0 means no IDR will be inserted (in frames, default= 60 )");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_GOP_SIZE, AMF_PARAM_STATIC, L"GOP Size (in frames, default= 60 )");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_DE_BLOCKING_FILTER_DISABLE, AMF_PARAM_STATIC, L"De-blocking Filter(true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_SLICES_PER_FRAME, AMF_PARAM_STATIC, L"Slices Per Frame (integer, default = 1)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_HEADER_INSERTION_MODE, AMF_PARAM_STATIC, L"insertion mode (none, gop, idr default = none");


    // Rate control properties
    SetParamType(AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD, AMF_PARAM_STATIC, L"Rate Control Method (CQP, CBR, VBR, VBR_LAT default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_FRAMERATE, AMF_PARAM_STATIC, L"Frame Rate (num,den), default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_VBV_BUFFER_SIZE, AMF_PARAM_STATIC, L"VBV Buffer Size (in bits, default = depends on USAGE)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_INITIAL_VBV_BUFFER_FULLNESS, AMF_PARAM_STATIC, L"Initial VBV Buffer Fullness (integer, 0=0% 64=100% , default = 64)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_ENABLE_VBAQ, AMF_PARAM_STATIC, L"Enable VBAQ(true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_HIGH_MOTION_QUALITY_BOOST_ENABLE, AMF_PARAM_DYNAMIC, L"bool; default = depends on USAGE; Enable High motion quality boost mode");

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

    // ------------- Encoder params per frame ---------------
    SetParamType(AMF_VIDEO_ENCODER_HEVC_END_OF_SEQUENCE, AMF_PARAM_FRAME, L"bool; default = false; generate end of sequence");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_FORCE_PICTURE_TYPE, AMF_PARAM_FRAME, L"amf_int64(AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_ENUM); default = AMF_VIDEO_ENCODER_HEVC_PICTURE_TYPE_NONE; generate particular picture type");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_INSERT_AUD, AMF_PARAM_FRAME, L"Insert AUD (true, false default =  false)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_INSERT_HEADER, AMF_PARAM_FRAME, L"Insert Header (true, false default =  false)");

    SetParamType(AMF_VIDEO_ENCODER_HEVC_MARK_CURRENT_WITH_LTR_INDEX, AMF_PARAM_FRAME, L"Mark With LTR Index (integer, default -1)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_FORCE_LTR_REFERENCE_BITFIELD, AMF_PARAM_FRAME, L"Force LTR Reference Bitfield (bitfield default = 0)");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_ROI_DATA, AMF_PARAM_FRAME, L"2D AMFSurface, surface format: AMF_SURFACE_GRAY32");
    SetParamType(AMF_VIDEO_ENCODER_HEVC_REFERENCE_PICTURE, AMF_PARAM_FRAME, L"AMFInterface(AMFSurface); surface used for frame injection");

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
