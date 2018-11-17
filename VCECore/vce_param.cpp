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
    memoryTypeIn(amf::AMF_MEMORY_UNKNOWN),
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
}

VCEParam::~VCEParam() {

}