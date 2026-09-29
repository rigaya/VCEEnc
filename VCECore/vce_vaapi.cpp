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
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// ------------------------------------------------------------------------------------------

#include "vce_vaapi.h"
#include "vce_param.h"
#include "vce_util.h"

#if ENABLE_VAAPI

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace {

// MesaのvlVaQualityBits: bit0=valid、bit1-2=preset、bit3=pre_encode、bit4=vbaq。
// 0はspeed/付加機能なし、1はbalanced/付加機能autoの特別値。明示presetはvalidを立て、付加機能は立てない。
constexpr int VA_PRESET_COMPRESSION_FAST = 0;
constexpr int VA_PRESET_COMPRESSION_BALANCED = 3;
constexpr int VA_PRESET_COMPRESSION_SLOW = 5;
constexpr int VA_PRESET_COMPRESSION_SLOWER = 7;
int va_compression_level(const RGY_CODEC codec, const int preset) {
    switch (codec) {
    case RGY_CODEC_H264:
        switch (preset) {
        case AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED: return VA_PRESET_COMPRESSION_FAST;
        case AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY: return VA_PRESET_COMPRESSION_SLOW;
        case AMF_VIDEO_ENCODER_QUALITY_PRESET_HIGH_QUALITY: return VA_PRESET_COMPRESSION_SLOWER;
        default: return VA_PRESET_COMPRESSION_BALANCED;
        }
    case RGY_CODEC_HEVC:
        switch (preset) {
        case AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_SPEED: return VA_PRESET_COMPRESSION_FAST;
        case AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_QUALITY: return VA_PRESET_COMPRESSION_SLOW;
        case AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_HIGH_QUALITY: return VA_PRESET_COMPRESSION_SLOWER;
        default: return VA_PRESET_COMPRESSION_BALANCED;
        }
    case RGY_CODEC_AV1:
        switch (preset) {
        case AMF_VIDEO_ENCODER_AV1_QUALITY_PRESET_SPEED: return VA_PRESET_COMPRESSION_FAST;
        case AMF_VIDEO_ENCODER_AV1_QUALITY_PRESET_QUALITY: return VA_PRESET_COMPRESSION_SLOW;
        case AMF_VIDEO_ENCODER_AV1_QUALITY_PRESET_HIGH_QUALITY: return VA_PRESET_COMPRESSION_SLOWER;
        default: return VA_PRESET_COMPRESSION_BALANCED;
        }
    default:
        return VA_PRESET_COMPRESSION_BALANCED;
    }
}

const TCHAR *va_preset_name(const int preset) {
    switch (preset) {
    case VA_PRESET_COMPRESSION_FAST: return _T("speed");
    case VA_PRESET_COMPRESSION_SLOW: return _T("quality");
    case VA_PRESET_COMPRESSION_SLOWER: return _T("high quality");
    default: return _T("balanced");
    }
}

} // namespace

VCEEncoderVA::VCEEncoderVA() : RGYEncoderVA(), m_preset(VA_PRESET_COMPRESSION_BALANCED) {
}

RGY_ERR VCEEncoderVA::init(VCEDeviceVA *dev, const VCEParam *prm, int width, int height,
    rgy_rational<int> sar, rgy_rational<int> fps, rgy_rational<int> timebase, std::shared_ptr<RGYLog> log) {
    if (prm == nullptr) return RGY_ERR_INVALID_PARAM;
    const bool qvbr = prm->rateControl == get_codec_qvbr(prm->codec);
    const bool hqvbr = prm->rateControl == get_codec_hqvbr(prm->codec);
    const bool hqcbr = prm->rateControl == get_codec_hqcbr(prm->codec);
    const auto defaultParam = VCEParam();
    // 共通側の init() より前にログを出すため、ここでは引数の log を直接使う。
    m_log = log;
    auto warnUnsupported = [&](const TCHAR *option, const bool changed) {
        if (changed && m_log) m_log->write(RGY_LOG_WARN, RGY_LOGT_DEV,
            _T("WARN: %s is not supported with --backend vaapi, ignored.\n"), option);
    };
    warnUnsupported(_T("--pe"), prm->pe != defaultParam.pe);
    warnUnsupported(_T("--vbaq"), prm->bVBAQ != defaultParam.bVBAQ);
    warnUnsupported(_T("--pa"), prm->pa != defaultParam.pa);
    warnUnsupported(_T("--b-pyramid"), prm->bPyramid != defaultParam.bPyramid);
    warnUnsupported(_T("--adapt-minigop"), prm->adaptMiniGOP != defaultParam.adaptMiniGOP);
    warnUnsupported(_T("--slices"), prm->nSlices != defaultParam.nSlices);
    warnUnsupported(_T("--ltr"), prm->LTRFrames != defaultParam.LTRFrames);
    warnUnsupported(_T("--filler"), prm->bFiller != defaultParam.bFiller);
    warnUnsupported(_T("--tiles"), prm->tiles != defaultParam.tiles);
    warnUnsupported(_T("--deblock"), prm->deblockFilter != defaultParam.deblockFilter);
    warnUnsupported(_T("--skip-frame"), prm->enableSkipFrame != defaultParam.enableSkipFrame);
    warnUnsupported(_T("--motion-est"), prm->nMotionEst != defaultParam.nMotionEst);
    warnUnsupported(_T("--enforce-hrd"), prm->bEnforceHRD != defaultParam.bEnforceHRD);
    warnUnsupported(_T("--b-deltaqp"), prm->deltaQPBFrame != defaultParam.deltaQPBFrame);
    warnUnsupported(_T("--bref-deltaqp"), prm->deltaQPBFrameRef != defaultParam.deltaQPBFrameRef);
    warnUnsupported(_T("--repeat-headers"), prm->repeatHeaders != defaultParam.repeatHeaders);
    warnUnsupported(_T("--temporal-layers"), prm->temporalLayers != defaultParam.temporalLayers);
    warnUnsupported(_T("--cdef-mode"), prm->cdefMode != defaultParam.cdefMode);
    warnUnsupported(_T("--cdf-update"), prm->cdfUpdate != defaultParam.cdfUpdate);
    warnUnsupported(_T("--cdf-frame-end-update"), prm->cdfFrameEndUpdate != defaultParam.cdfFrameEndUpdate);
    warnUnsupported(_T("--aq-mode"), prm->aqMode != defaultParam.aqMode);
    warnUnsupported(_T("--qp-min/--qp-max inter"), prm->nQPMinInter != prm->nQPMin || prm->nQPMaxInter != prm->nQPMax);
    warnUnsupported(_T("--smart-access-video"), prm->smartAccessVideo != defaultParam.smartAccessVideo);
    warnUnsupported(_T("--multi-instance"), prm->multiInstance != defaultParam.multiInstance);
    warnUnsupported(_T("--screen-content-tools"), prm->screenContentTools != defaultParam.screenContentTools);
    warnUnsupported(_T("--palette-mode"), prm->paletteMode != defaultParam.paletteMode);
    warnUnsupported(_T("--force-integer-mv"), prm->forceIntegerMV != defaultParam.forceIntegerMV);
    if (prm->vppamf.pp.enable || prm->vppamf.enhancer.enable || prm->vppamf.frc.enable) {
        if (m_log) m_log->write(RGY_LOG_ERROR, RGY_LOGT_DEV, _T("AMF preprocess/enhancer/FRC filters are not supported with --backend vaapi.\n"));
        return RGY_ERR_UNSUPPORTED;
    }
    if (hqvbr && m_log) m_log->write(RGY_LOG_WARN, RGY_LOGT_DEV, _T("WARN: --hqvbr is not supported with --backend vaapi, ignored (using VBR).\n"));
    if (hqcbr && m_log) m_log->write(RGY_LOG_WARN, RGY_LOGT_DEV, _T("WARN: --hqcbr is not supported with --backend vaapi, ignored (using CBR).\n"));

    RGYVAEncParam va;
    va.codec = prm->codec;
    va.width = width;
    va.height = height;
    va.bitdepth = prm->outputDepth;
    // sar の未指定は 0/0 で来る。共通側は分母 0 を不正とするので、同じく「未指定」を表す 0/1 にする。
    va.sar = sar.d() > 0 ? sar : rgy_rational<int>(0, 1);
    va.fps = fps;
    va.timebase = timebase;
    va.rc = prm->rateControl == get_codec_cqp(prm->codec) ? RGY_VA_RC_CQP
        : qvbr ? RGY_VA_RC_QVBR
        : (prm->rateControl == get_codec_cbr(prm->codec) || hqcbr) ? RGY_VA_RC_CBR : RGY_VA_RC_VBR;
    va.qp = prm->qp;
    va.quality = prm->qvbrLevel;
    va.bitrateKbps = prm->nBitrate > 0 ? prm->nBitrate : (qvbr ? defaultParam.nBitrate : 0);
    va.maxBitrateKbps = prm->nMaxBitrate;
    va.vbvBufKbits = prm->nVBVBufferSize;
    va.gopLen = prm->nGOPLen;
    va.bframes = prm->bframes.value_or(defaultParam.bframes.value_or(VCE_DEFAULT_BFRAMES));
    va.refs = prm->refFrames.value_or(-1);
    if (prm->outputDepth > 8 && prm->codec == RGY_CODEC_HEVC) va.profile = AV_PROFILE_HEVC_MAIN_10;
    else if (prm->codec == RGY_CODEC_H264 && prm->codecParam[RGY_CODEC_H264].nProfile != defaultParam.codecParam[RGY_CODEC_H264].nProfile) va.profile = prm->codecParam[RGY_CODEC_H264].nProfile;
    else if (prm->codec == RGY_CODEC_HEVC) va.profile = prm->codecParam[RGY_CODEC_HEVC].nProfile == AMF_VIDEO_ENCODER_HEVC_PROFILE_MAIN_10 ? AV_PROFILE_HEVC_MAIN_10 : AV_PROFILE_HEVC_MAIN;
    else if (prm->codec == RGY_CODEC_AV1) va.profile = AV_PROFILE_AV1_MAIN;
    if (prm->codecParam[prm->codec].nLevel > 0) va.level = prm->codecParam[prm->codec].nLevel;
    // H.264 以外は、従来どおり AMF の tier の値を FFmpeg の tier に写す。
    if (prm->codec != RGY_CODEC_H264) va.tier = prm->codecParam[prm->codec].nTier == AMF_VIDEO_ENCODER_HEVC_TIER_HIGH ? 1 : 0;
    va.qpMin = prm->nQPMin;
    va.qpMax = prm->nQPMax;
    m_preset = va_compression_level(prm->codec, prm->qualityPreset);
    va.compressionLevel = m_preset;
    va.aud = prm->aud;
    va.vui = prm->common.out_vui;
    return RGYEncoderVA::init(dev, va, log);
}

tstring VCEEncoderVA::levelString() const {
    if (!m_codecCtx || m_codecCtx->level == AV_LEVEL_UNKNOWN) return _T("auto");
    const TCHAR *level = get_cx_desc(get_level_list(m_codec), m_codecCtx->level);
    return level && level[0] ? tstring(level) : strsprintf(_T("%d"), m_codecCtx->level);
}

tstring VCEEncoderVA::paramString() const {
    if (!m_codecCtx) return _T("VA-API encoder is not initialized.\n");
    tstring mes = strsprintf(_T("Quality:       %s\n"), va_preset_name(m_preset));
    if (m_rateControl == RGY_VA_RC_CQP) {
        const int qpP = m_codecCtx->global_quality / FF_QP2LAMBDA;
        const int qpI = (int)(qpP * m_codecCtx->i_quant_factor);
        const int qpB = (int)(qpP * m_codecCtx->b_quant_factor);
        mes += strsprintf(_T("CQP:           %s:%d, %s:%d"),
            m_codec == RGY_CODEC_AV1 ? _T("Intra") : _T("I"), qpI,
            m_codec == RGY_CODEC_AV1 ? _T("Inter") : _T("P"), qpP);
        if (m_bframes > 0) {
            mes += strsprintf(_T(", %s:%d"), m_codec == RGY_CODEC_AV1 ? _T("InterB") : _T("B"), qpB);
        }
        mes += _T("\n");
    } else {
        const TCHAR *rc = m_rateControl == RGY_VA_RC_QVBR ? _T("QVBR") : m_rateControl == RGY_VA_RC_CBR ? _T("CBR") : _T("VBR");
        mes += strsprintf(_T("%s:           %lld kbps\n"), rc, (long long)(m_codecCtx->bit_rate / 1000));
        if (m_rateControl == RGY_VA_RC_QVBR) {
            mes += strsprintf(_T("QVBR level:    %d\n"), m_qp);
        }
        if (m_codecCtx->rc_max_rate > 0) {
            mes += strsprintf(_T("Max bitrate:   %lld kbps\n"), (long long)(m_codecCtx->rc_max_rate / 1000));
        }
        if (m_qpMin.has_value() || m_qpMax.has_value()) {
            const auto qmin = m_qpMin.has_value() ? strsprintf(_T("%d"), m_qpMin.value()) : tstring(_T("auto"));
            const auto qmax = m_qpMax.has_value() ? strsprintf(_T("%d"), m_qpMax.value()) : tstring(_T("auto"));
            mes += strsprintf(_T("QP:            Min: %s:%s, Max: %s:%s\n"), qmin.c_str(), qmin.c_str(), qmax.c_str(), qmax.c_str());
        }
    }
    if (m_codecCtx->rc_buffer_size > 0) {
        mes += strsprintf(_T("VBV Bufsize:   %d kb\n"), m_codecCtx->rc_buffer_size / 1000);
    }
    // AMFの表示に合わせ、H.264でBフレームを使わない場合はBframesの行を出さない
    if (m_bframes > 0 || m_codec != RGY_CODEC_H264) {
        mes += strsprintf(_T("Bframes:       %d frames\n"), m_bframes);
    }
    mes += strsprintf(_T("Ref frames:    %d frames\nGOP Len:       %d frames\n"), m_refs, m_codecCtx->gop_size);
    return mes;
}

#endif // ENABLE_VAAPI
