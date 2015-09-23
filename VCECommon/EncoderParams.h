/*******************************************************************************
 Copyright 息2014 Advanced Micro Devices, Inc. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1   Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 2   Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

/**
 *******************************************************************************
 * @file <EncoderParams.h>
 *
 * @brief Header file for Encoder Parameters
 *
 *******************************************************************************
 */

#pragma once

#include "AMFPlatform.h"
#include "VideoEncoderVCE.h"
#include "CmdLogger.h"
#include "ParametersStorage.h"

#include <algorithm>
#include <iterator>
#include <cctype>

static AMF_RESULT ParamConverterUsage(const std::wstring& value,
                amf::AMFVariant& valueOut)
{
    AMF_VIDEO_ENCODER_USAGE_ENUM paramValue;

    std::wstring uppValue = toUpper(value);
    if (uppValue == L"TRANSCODING" || uppValue == L"0")
    {
        paramValue = AMF_VIDEO_ENCODER_USAGE_TRANSCONDING;
    }
    else if (uppValue == L"ULTRALOWLATENCY" || uppValue == L"1")
    {
        paramValue = AMF_VIDEO_ENCODER_USAGE_ULTRA_LOW_LATENCY;
    }
    else if (uppValue == L"LOWLATENCY" || uppValue == L"2")
    {
        paramValue = AMF_VIDEO_ENCODER_USAGE_LOW_LATENCY;
    }
    else if (uppValue == L"WEBCAM" || uppValue == L"3")
    {
        paramValue = AMF_VIDEO_ENCODER_USAGE_WEBCAM;
    }
    else
    {
        LOG_ERROR(L"AMF_VIDEO_ENCODER_USAGE_ENUM hasn't \"" << value
                        << L"\" value.");
        return AMF_INVALID_ARG;
    }
    valueOut = amf_int64(paramValue);
    return AMF_OK;
}
static AMF_RESULT ParamConverterQuality(const std::wstring& value,
                amf::AMFVariant& valueOut)
{
    AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM paramValue;

    std::wstring uppValue = toUpper(value);
    if (uppValue == L"BALANCED" || uppValue == L"0")
    {
        paramValue = AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED;
    }
    else if (uppValue == L"SPEED" || uppValue == L"1")
    {
        paramValue = AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED;
    }
    else if (uppValue == L"QUALITY" || uppValue == L"2")
    {
        paramValue = AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY;
    }
    else
    {
        LOG_ERROR(L"AMF_VIDEO_ENCODER_QUALITY_PRESET_ENUM hasn't \"" << value
                        << L"\" value.");
        return AMF_INVALID_ARG;
    }
    valueOut = amf_int64(paramValue);
    return AMF_OK;
}
static AMF_RESULT ParamConverterProfile(const std::wstring& value,
                amf::AMFVariant& valueOut)
{
    AMF_VIDEO_ENCODER_PROFILE_ENUM paramValue;
    std::wstring uppValue = toUpper(value);
    if (uppValue == L"BASELINE" || uppValue == L"66")
    {
        paramValue = AMF_VIDEO_ENCODER_PROFILE_BASELINE;
    }
    else if (uppValue == L"MAIN" || uppValue == L"77")
    {
        paramValue = AMF_VIDEO_ENCODER_PROFILE_MAIN;
    }
    else if (uppValue == L"HIGH" || uppValue == L"100")
    {
        paramValue = AMF_VIDEO_ENCODER_PROFILE_HIGH;
    }
    else
    {
        LOG_ERROR(L"AMF_VIDEO_ENCODER_PROFILE_ENUM hasn't \"" << value
                        << L"\" value.");
        return AMF_INVALID_ARG;
    }
    valueOut = amf_int64(paramValue);
    return AMF_OK;
}
enum ProfileLevel
{
    PV1 = 10,
    PV11 = 11,
    PV12 = 12,
    PV13 = 13,
    PV2 = 20,
    PV21 = 21,
    PV22 = 22,
    PV3 = 30,
    PV31 = 31,
    PV32 = 32,
    PV4 = 40,
    PV41 = 41,
    PV42 = 42
};

static AMF_RESULT ParamConverterProfileLevel(const std::wstring& value,
                amf::AMFVariant& valueOut)
{
    ProfileLevel paramValue;
    std::wstring uppValue = toUpper(value);
    if (uppValue == L"1")
    {
        paramValue = PV1;
    }
    else if (uppValue == L"1.1")
    {
        paramValue = PV11;
    }
    else if (uppValue == L"1.2")
    {
        paramValue = PV12;
    }
    else if (uppValue == L"1.3")
    {
        paramValue = PV13;
    }
    else if (uppValue == L"2")
    {
        paramValue = PV2;
    }
    else if (uppValue == L"2.1")
    {
        paramValue = PV21;
    }
    else if (uppValue == L"2.2")
    {
        paramValue = PV22;
    }
    else if (uppValue == L"3")
    {
        paramValue = PV3;
    }
    else if (uppValue == L"3.1")
    {
        paramValue = PV31;
    }
    else if (uppValue == L"3.2")
    {
        paramValue = PV32;
    }
    else if (uppValue == L"4")
    {
        paramValue = PV4;
    }
    else if (uppValue == L"4.1")
    {
        paramValue = PV41;
    }
    else if (uppValue == L"4.2")
    {
        paramValue = PV42;
    }
    else
    {
        LOG_ERROR(L"ProfileLevel hasn't \"" << value << L"\" value.");
        return AMF_INVALID_ARG;
    }
    valueOut = amf_int64(paramValue);
    return AMF_OK;
}
static AMF_RESULT ParamConverterScanType(const std::wstring& value,
                amf::AMFVariant& valueOut)
{
    AMF_VIDEO_ENCODER_SCANTYPE_ENUM paramValue;
    std::wstring uppValue = toUpper(value);
    if (uppValue == L"PROGRESSIVE" || uppValue == L"0")
    {
        paramValue = AMF_VIDEO_ENCODER_SCANTYPE_PROGRESSIVE;
    }
    else if (uppValue == L"INTERLACED" || uppValue == L"1")
    {
        paramValue = AMF_VIDEO_ENCODER_SCANTYPE_INTERLACED;
    }
    else
    {
        LOG_ERROR(L"AMF_VIDEO_ENCODER_SCANTYPE_ENUM hasn't \"" << value
                        << L"\" value.");
        return AMF_INVALID_ARG;
    }
    valueOut = amf_int64(paramValue);
    return AMF_OK;
}

static AMF_RESULT ParamConverterRateControl(const std::wstring& value,
                amf::AMFVariant& valueOut)
{
    AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_ENUM paramValue;
    std::wstring uppValue = toUpper(value);
    if (uppValue == L"CQP" || uppValue == L"0")
    {
        paramValue = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTRAINED_QP;
    }
    else if (uppValue == L"CBR" || uppValue == L"1")
    {
        paramValue = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR;
    }
    else if (uppValue == L"VBR" || uppValue == L"2")
    {
        paramValue = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
    }
    else if (uppValue == L"VBR_LAT" || uppValue == L"3")
    {
        paramValue
                        = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR;
    }
    else
    {
        LOG_ERROR(L"AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD hasn't \"" << value
                        << L"\" value.");
        return AMF_INVALID_ARG;
    }
    valueOut = amf_int64(paramValue);
    return AMF_OK;
}

static AMF_RESULT ParamConverterPictureType(const std::wstring& value,
                amf::AMFVariant& valueOut)
{
    AMF_VIDEO_ENCODER_PICTURE_TYPE_ENUM paramValue;
    std::wstring uppValue = toUpper(value);
    if (uppValue == L"NONE" || uppValue == L"0")
    {
        paramValue = AMF_VIDEO_ENCODER_PICTURE_TYPE_NONE;
    }
    else if (uppValue == L"SKIP" || uppValue == L"1")
    {
        paramValue = AMF_VIDEO_ENCODER_PICTURE_TYPE_SKIP;
    }
    else if (uppValue == L"IDR" || uppValue == L"2")
    {
        paramValue = AMF_VIDEO_ENCODER_PICTURE_TYPE_IDR;
    }
    else if (uppValue == L"I" || uppValue == L"3")
    {
        paramValue = AMF_VIDEO_ENCODER_PICTURE_TYPE_I;
    }
    else if (uppValue == L"P" || uppValue == L"4")
    {
        paramValue = AMF_VIDEO_ENCODER_PICTURE_TYPE_P;
    }
    else if (uppValue == L"B" || uppValue == L"5")
    {
        paramValue = AMF_VIDEO_ENCODER_PICTURE_TYPE_B;
    }
    else
    {
        LOG_ERROR(L"AMF_VIDEO_ENCODER_PICTURE_TYPE_ENUM hasn't \"" << value
                        << L"\" value.");
        return AMF_INVALID_ARG;
    }
    valueOut = amf_int64(paramValue);
    return AMF_OK;
}

static AMF_RESULT ParamConverterPictureStructure(const std::wstring& value,
                amf::AMFVariant& valueOut)
{
    AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_ENUM paramValue;
    std::wstring uppValue = toUpper(value);
    if (uppValue == L"NONE" || uppValue == L"0")
    {
        paramValue = AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_NONE;
    }
    else if (uppValue == L"FRAME" || uppValue == L"1")
    {
        paramValue = AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_FRAME;
    }
    else if (uppValue == L"TOP_FIELD" || uppValue == L"2")
    {
        paramValue = AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_TOP_FIELD;
    }
    else if (uppValue == L"BOTTOM_FIELD" || uppValue == L"3")
    {
        paramValue = AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_BOTTOM_FIELD;
    }
    else
    {
        LOG_ERROR(L"AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_ENUM hasn't \""
                        << value << L"\" value.");
        return AMF_INVALID_ARG;
    }
    valueOut = amf_int64(paramValue);
    return AMF_OK;
}

static const wchar_t* SETFRAMEPARAMFREQ_PARAM_NAME = L"SETFRAMEPARAMFREQ";
static const wchar_t* SETDYNAMICPARAMFREQ_PARAM_NAME = L"SETDYNAMICPARAMFREQ";

static AMF_RESULT RegisterEncoderParams(ParametersStorage* pParams)
{
    pParams->SetParamDescription(SETFRAMEPARAMFREQ_PARAM_NAME, ParamCommon,
                    L"Frequency of applying frame parameters (in frames, default = 0 )");
    pParams->SetParamDescription(SETDYNAMICPARAMFREQ_PARAM_NAME, ParamCommon,
                    L"Frequency of applying dynamic parameters. (in frames, default = 0 )");

    // ------------- Encoder params usage---------------
    pParams->SetParamDescription(
                    AMF_VIDEO_ENCODER_USAGE,
                    ParamEncoderUsage,
                    L"Encoder usage type. Set many default parameters. (TRANSCONDING, ULTRALOWLATENCY, LOWLATENCY, WEBCAM, default = N/A)",
                    ParamConverterUsage);
    // ------------- Encoder params static---------------
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_PROFILE, ParamEncoderStatic,
                    L"H264 profile (Main, Baseline,High, default = Main",
                    ParamConverterProfile);
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_PROFILE_LEVEL,
                    ParamEncoderStatic,
                    L"H264 profile level (integer, default = 42",
                    ParamConverterProfileLevel);
    pParams->SetParamDescription(
                    AMF_VIDEO_ENCODER_QUALITY_PRESET,
                    ParamEncoderStatic,
                    L"Quality Preset (BALANCED, SPEED, QUALITY default = depends on USAGE)",
                    ParamConverterQuality);
    pParams->SetParamDescription(
                    AMF_VIDEO_ENCODER_SCANTYPE,
                    ParamEncoderStatic,
                    L"Scan Type (PROGRESSIVE, INTERLACED, default = PROGRESSIVE)",
                    ParamConverterScanType);
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_MAX_LTR_FRAMES,
                    ParamEncoderStatic,
                    L"Max Of LTR frames (integer, default = 0)");

    // ------------- Encoder params dynamic ---------------
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_B_PIC_DELTA_QP,
                    ParamEncoderDynamic,
                    L"B-picture Delta  (integer, default = depends on USAGE)");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_REF_B_PIC_DELTA_QP,
                    ParamEncoderDynamic,
                    L"Reference B-picture Delta  (integer, default = depends on USAGE)");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_FRAMERATE,
                    ParamEncoderDynamic,
                    L"Frame Rate (num,den), default = depends on USAGE)");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_GOP_SIZE,
                    ParamEncoderDynamic, L"GOP Size (in frames, default=60 )");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_MAX_AU_SIZE,
                    ParamEncoderDynamic, L"Max AU Size (in bits, default = 0)");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_TARGET_BITRATE,
                    ParamEncoderDynamic,
                    L"Target bit rate (in bits, default = depends on USAGE)");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_PEAK_BITRATE,
                    ParamEncoderDynamic,
                    L"Peak bit rate (in bits, default = depends on USAGE)");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_B_PIC_PATTERN,
                    ParamEncoderDynamic,
                    L"B-picture Pattern (number of B-Frames, default = 3)");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_IDR_PERIOD,
                    ParamEncoderDynamic,
                    L"IDR Period, (in frames, default = depends on USAGE) ");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_SLICES_PER_FRAME,
                    ParamEncoderDynamic,
                    L"Slices Per Frame (integer, default = 1)");
    pParams->SetParamDescription(
                    AMF_VIDEO_ENCODER_INTRA_REFRESH_NUM_MBS_PER_SLOT,
                    ParamEncoderDynamic,
                    L"Intra Refresh MBs Number Per Slot (in Macroblocks, default = depends on USAGE)");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_INITIAL_VBV_BUFFER_FULLNESS,
                    ParamEncoderDynamic,
                    L"Initial VBV Buffer Fullness (integer, 0=0% 64=100% , default = 64)");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_VBV_BUFFER_SIZE,
                    ParamEncoderDynamic,
                    L"VBV Buffer Size (in bits, default = depends on USAGE)");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_MIN_QP, ParamEncoderDynamic,
                    L"Min QP (integer 0-51, default = depends on USAGE)");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_MAX_QP, ParamEncoderDynamic,
                    L"Max QP (integer 0-51, default = 51)");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_QP_I, ParamEncoderDynamic,
                    L"QP I (integer 0-51, default = 22)");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_QP_P, ParamEncoderDynamic,
                    L"QP P (integer 0-51, default = 22)");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_QP_B, ParamEncoderDynamic,
                    L"QP B (integer 0-51, default = 22)");
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_HEADER_INSERTION_SPACING,
                    ParamEncoderDynamic,
                    L"Sets the header insertion spacing (integer 0 - 1000, default = 0)");

    pParams->SetParamDescription(AMF_VIDEO_ENCODER_ENFORCE_HRD,
                    ParamEncoderDynamic,
                    L"Enforce HRD (true, false default = depends on USAGE)",
                    ParamConverterBoolean);
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_FILLER_DATA_ENABLE,
                    ParamEncoderDynamic,
                    L"Filler Data Enable (true, false default =  false)",
                    ParamConverterBoolean);
    pParams->SetParamDescription(
                    AMF_VIDEO_ENCODER_RATE_CONTROL_SKIP_FRAME_ENABLE,
                    ParamEncoderDynamic,
                    L"Rate Control Based Frame Skip (true, false default =  depends on USAGE)",
                    ParamConverterBoolean);
    pParams->SetParamDescription(
                    AMF_VIDEO_ENCODER_DE_BLOCKING_FILTER,
                    ParamEncoderDynamic,
                    L"De-blocking Filter (true, false default =  depends on USAGE)",
                    ParamConverterBoolean);
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_B_REFERENCE_ENABLE,
                    ParamEncoderDynamic,
                    L"Enable B Refrence (true, false default =  true)",
                    ParamConverterBoolean);
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_MOTION_HALF_PIXEL,
                    ParamEncoderDynamic,
                    L"Half Pixel (true, false default =  true)",
                    ParamConverterBoolean);
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_MOTION_QUARTERPIXEL,
                    ParamEncoderDynamic,
                    L"Quarter Pixel (true, false default =  true",
                    ParamConverterBoolean);
    pParams->SetParamDescription(
                    AMF_VIDEO_ENCODER_NUM_TEMPORAL_ENHANCMENT_LAYERS,
                    ParamEncoderDynamic,
                    L"Num Of Temporal Enhancment Layers (SVC) (integer, default = 0, range = 0, min(2, caps->GetMaxNumOfTemporalLayers())");

    pParams->SetParamDescription(
                    AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD,
                    ParamEncoderDynamic,
                    L"Rate Control Method (CQP, CBR, VBR, VBR_LAT default = depends on USAGE)",
                    ParamConverterRateControl);

    // ------------- Encoder params per frame ---------------
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_END_OF_SEQUENCE,
                    ParamEncoderFrame,
                    L"Bool; default = false; Generate end of sequence",
                    ParamConverterBoolean);
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_END_OF_STREAM,
                    ParamEncoderFrame,
                    L"Bool; default = false; Generate end of stream",
                    ParamConverterBoolean);
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_INSERT_SPS,
                    ParamEncoderFrame,
                    L"Insert SPS (true, false default =  false)",
                    ParamConverterBoolean);
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_INSERT_PPS,
                    ParamEncoderFrame,
                    L"Insert PPS (true, false default =  false)",
                    ParamConverterBoolean);
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_INSERT_AUD,
                    ParamEncoderFrame,
                    L"Insert AUD (true, false default =  false)",
                    ParamConverterBoolean);
    pParams->SetParamDescription(AMF_VIDEO_ENCODER_MARK_CURRENT_WITH_LTR_INDEX,
                    ParamEncoderFrame,
                    L"Mark With LTR Index (integer, default -1)");
    pParams->SetParamDescription(
                    AMF_VIDEO_ENCODER_FORCE_LTR_REFERENCE_BITFIELD,
                    ParamEncoderFrame,
                    L"Force LTR Reference Bitfield (bitfield default = 0)");
    pParams->SetParamDescription(
                    AMF_VIDEO_ENCODER_FORCE_PICTURE_TYPE,
                    ParamEncoderFrame,
                    L"Force Picture Type (NONE, SKIP, IDR, I, P, B, default = NONE)",
                    ParamConverterPictureType);
    pParams->SetParamDescription(
                    AMF_VIDEO_ENCODER_PICTURE_STRUCTURE,
                    ParamEncoderFrame,
                    L"Indicates picture type.  amf_int64(AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_ENUM); default = AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_NONE",
                    ParamConverterPictureStructure);
    return AMF_OK;
}
