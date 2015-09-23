//  -----------------------------------------------------------------------------------------
//    VCEEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#pragma once

#include "VideoEncoderVCE.h"
#include "VCEUtil.h"
#include "VCEParam.h"

bool is_interlaced(VCEParam *prm) {
    return prm->nInterlaced == AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_TOP_FIELD
        || prm->nInterlaced == AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_BOTTOM_FIELD;
}

void init_vce_param(VCEParam *prm) {
    memset(prm, 0, sizeof(prm[0]));
    prm->memoryTypeIn = amf::AMF_MEMORY_DX9;
    prm->nCodecId     = 0;
    prm->nUsage       = AMF_VIDEO_ENCODER_USAGE_TRANSCONDING;
    prm->codecParam[prm->nCodecId].nLevel   = list_avc_level[0].value;
    prm->codecParam[prm->nCodecId].nProfile = list_avc_profile[2].value;
    prm->nInterlaced = AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_FRAME;

    prm->bTimerPeriodTuning = TRUE;

    prm->nBitrate    = 5000;
    prm->nMaxBitrate = VCE_DEFAULT_MAX_BITRATE;
    prm->nVBVBufferSize = VCE_DEFAULT_VBV_BUFSIZE;
    prm->nInitialVBVPercent = 64;

    prm->nQPI = VCE_DEFAULT_QPI;
    prm->nQPP = VCE_DEFAULT_QPP;
    prm->nQPB = VCE_DEFAULT_QPB;
    prm->nQPMin = 0;
    prm->nQPMax = 51;
    prm->nDeltaQPBFrame = 6;
    prm->nDeltaQPBFrameRef = 4;

    prm->nBframes = VCE_DEFAULT_BFRAMES;
    prm->bBPyramid = FALSE;
    prm->nRateControl = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTRAINED_QP;
    prm->bDeblockFilter = TRUE;
    prm->nGOPLen = 0;
    prm->nIDRPeriod = 1;
    prm->nSlices = 1;
    prm->nMotionEst = VCE_MOTION_EST_QUATER | VCE_MOTION_EST_HALF;
}
