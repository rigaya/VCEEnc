///-------------------------------------------------------------------------
/*
 ***************************************************************************************************
 *
 * Copyright (c) 2013 Advanced Micro Devices, Inc. (unpublished)
 *
 *  All rights reserved.  This notice is intended as a precaution against inadvertent publication and
 *  does not imply publication or any waiver of confidentiality.  The year included in the foregoing
 *  notice is the year of creation of the work.
 *
 ***************************************************************************************************
 */
/**
 ***************************************************************************************************
 * @file  EncoderVCEDEM.h
 * @brief AMD Media Framework (AMF) Display Encode Mode (DEM)
 ***************************************************************************************************
 */
#ifndef __AMFEncoderVCEDEM_h__
#define __AMFEncoderVCEDEM_h__
#pragma once

#include "../core/PropertyStorageEx.h"

namespace amf
{
    // Read-only properties
    static const wchar_t* DEM_GENERIC_SUPPORT = L"GenericSupport";
    static const wchar_t* DEM_WFD_SUPPORT = L"WirelessDisplaySupport";
    static const wchar_t* DEM_LOWLATENCY_SUPPORT = L"LowLatencySupport";

    static const wchar_t* DEM_WIDTH = L"Width";
    static const wchar_t* DEM_HEIGHT = L"Height";
    static const wchar_t* DEM_FPS_NUM = L"FPSNumerator";
    static const wchar_t* DEM_FPS_DENOM = L"FPSDenominator";

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Static Properties: The following properties must be set prior to the CreateEncoder() call

    // Usage: The usage parameter tailors the encoding and rate control parameters for the intended use case
    static const wchar_t* DEM_USAGE = L"Usage";
    enum DemUsage
    {
        DEM_USAGE_GENERIC = 0x01,         // Generic usage provides the application with the most control over encoding/rate control parameters
        DEM_USAGE_WIRELESS_DISPLAY = 0x02, // Configures encoding parameters for typical Wireless Display use-case. Minimize latency while preserving quality
        DEM_USAGE_LOW_LATENCY = 0x04       // Configures parameters for low latency, high-interactivity use-case such as remote gaming.
    };

    static const amf_uint32 DEM_USAGE_DEFAULT = DEM_USAGE_WIRELESS_DISPLAY;

    // Profile - AVC profile
    static const wchar_t* DEM_PROFILE = L"Profile";
    enum DemProfileType
    {
        DEM_PROFILE_CONSTRAINED_BASELINE = 66,
        DEM_PROFILE_MAIN = 77,
        DEM_PROFILE_HIGH = 100
    };
    static const amf_uint32 DEM_PROFILE_DEFAULT = DEM_PROFILE_CONSTRAINED_BASELINE;

    // Output type
    static const wchar_t* DEM_OUTPUT_TYPE = L"OutputType";
    enum DemOutputType
    {
        DEM_AV_TS = 0, // Audio and video transport stream
        DEM_AV_ES,    // Audio and video elementary stream
        DEM_V_TS,     // Video transport stream
        DEM_V_ES,     // Video elementary stream
        DEM_A_ES      // Audio elementary stream (Note: this value isn't supported as input and used only as AMFDemBuffer::GetDataType() return value)
    };
    static const DemOutputType DEM_OUTPUT_TYPE_DEFAULT = DEM_AV_TS;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    // Dynamic Properties: The following properties can be adjusted during encoding.
    //
    // Note: Make sure to call FlushConfiguration() after adjusting these to have them sent to the encoder


    // IDR period
    // This property is used to configure the interval between Intra Display Refresh pictures.
    // Value 0 means no IDR
    static const wchar_t* DEM_IDR_PERIOD = L"IDRPeriod";
    static const amf_uint32 DEM_IDR_PERIOD_MIN = 0;
    static const amf_uint32 DEM_IDR_PERIOD_MAX = 6000;
    static const amf_uint32 DEM_IDR_PERIOD_DEFAULT = 300;

    // Skipped PIC period
    // Value 0 means no skip, 1 means skip of evert second frame, 2 means skip every 2 of three frames etc.
    static const wchar_t* DEM_SKIPPED_PIC_PERIOD = L"SkippedPICPeriod";
    static const amf_uint32 DEM_SKIPPED_PIC_PERIOD_MIN = 0;
    static const amf_uint32 DEM_SKIPPED_PIC_PERIOD_MAX = 7;
    static const amf_uint32 DEM_SKIPPED_PIC_PERIOD_DEFAULT = 0;

    // Rate control method
    static const wchar_t* DEM_RATE_CONTROL_METHOD = L"RateControlMethod";
    enum DemRateControlMethod
    {
        DEM_PEAK_CONSTRAINED_VBR = 0, // Peak Constrained Variable Bitrate
        DEM_LATENCY_CONSTRAINED_VBR, // Latency Constrained Variable Bitrate favours latency over quality
        DEM_CBR,                     // Constant Bitrate
        DEM_NO_RC                    // No Rate Control
    };
    static const DemRateControlMethod DEM_RATE_CONTROL_METHOD_DEFAULT = DEM_PEAK_CONSTRAINED_VBR;

    // Bitrate and peak bitrate.
    // Max value depends on resolution and frame rate:
    //   - 10 MBits/sec for 720p and frame rate less than or equal to 30 FPS
    //   - 20 MBits/sec for 720p and frame rate less than or equal to 60 FPS
    //   - 20 MBits/sec for 1080p and frame rate less than or equal to 30 FPS
    //   - 50 MBits/sec for 1080p and frame rate less than or equal to 60 FPS
    static const wchar_t* DEM_TARGET_BITRATE = L"TargetBitrate";
    static const wchar_t* DEM_PEAK_BITRATE = L"PeakBitrate";

    // Intra refresh macroblocks per slot.
    // The intra refresh slot mechanism allows an application to insert intra macroblocks without having to encode an entire IDR picture.
    static const wchar_t* DEM_INTRA_REFRESH_MB_PER_SLOT = L"IntraRefreshMBsNumberPerSlot";

    // Initial VBV buffer fullness in percents
    static const wchar_t* DEM_INITIAL_VBV_BUFFER_FULLNESS = L"InitialVBVBufferFullness";
    static const amf_uint32 DEM_INITIAL_VBV_BUFFER_FULLNESS_DEFAULT = 100;
    static const amf_uint32 DEM_INITIAL_VBV_BUFFER_FULLNESS_MIN = 0;
    static const amf_uint32 DEM_INITIAL_VBV_BUFFER_FULLNESS_MAX = 100;

    // VBV buffer size.
    // Specifies the buffer size of the leaky bucket model used with the CBR and Peak Constrained VBR rate control modes
    static const wchar_t* DEM_VBV_BUFFER_SIZE = L"VBVBufferSize";

    // Min and max QP.
    static const wchar_t* DEM_MIN_QP = L"MinQP";
    static const wchar_t* DEM_MAX_QP = L"MaxQP";
    static const amf_uint32 DEM_MIN_QP_DEFAULT = 22;
    static const amf_uint32 DEM_MAX_QP_DEFAULT = 51;
    static const amf_uint32 DEM_QP_MIN = 0;
    static const amf_uint32 DEM_QP_MAX = 51;

    // I frame quantization only if rate control is disabled
    static const wchar_t* DEM_QP_I = L"QPI";
    static const amf_uint32 DEM_QP_I_DEFAULT = 22;
    static const amf_uint32 DEM_QP_I_MIN = 0;
    static const amf_uint32 DEM_QP_I_MAX = 51;

    // P frame quantization if rate control is disabled
    static const wchar_t* DEM_QP_P = L"QPP";
    static const amf_uint32 DEM_QP_P_DEFAULT = 22;
    static const amf_uint32 DEM_QP_P_MIN = 0;
    static const amf_uint32 DEM_QP_P_MAX = 51;

    // Number of feedback slots
    static const wchar_t* DEM_FEEDBACKS = L"Feedbacks";
    static const amf_uint32 DEM_FEEDBACKS_DEFAULT = 16;
    static const amf_uint32 DEM_FEEDBACKS_MIN = 3;
    static const amf_uint32 DEM_FEEDBACKS_MAX = 32;

    // Inloop Deblocking Filter Enable
    static const wchar_t* DEM_INLOOP_DEBLOCKING = L"DeBlockingFilter";
    static const amf_bool DEM_INLOOP_DEBLOCKING_DEFAULT = true;

    // FillerData for RC modes
    static const wchar_t* DEM_FILLER_DATA = L"FillerDataEnable";
    static const amf_bool DEM_FILLER_DATA_DEFAULT = false;

    // Insert AUD for Transport Stream mode
    static const wchar_t* DEM_INSERT_AUD = L"InsertAUD";
    static const amf_bool DEM_INSERT_AUD_DEFAULT = true;

    // Half pel motion estimation
    static const wchar_t* DEM_MOTION_EST_HALF_PIXEL = L"HalfPixel";
    static const amf_bool DEM_MOTION_EST_HALF_PIXEL_DEFAULT = true;

    // Quarter pel motion estimation
    static const wchar_t* DEM_MOTION_EST_QUARTER_PIXEL = L"QuarterPixel";
    static const amf_bool DEM_MOTION_EST_QUARTER_PIXEL_DEFAULT = true;

    // Timing Info Presence
    static const wchar_t* DEM_TIMING_INFO_PRESENT = L"TimingInfo";
    static const amf_bool DEM_TIMING_INFO_PRESENT_DEFAULT = true;

    // Slices per frame
    static const wchar_t* DEM_SLICES_PER_FRAME = L"SlicesPerFrame";
    static const amf_uint32 DEM_SLICES_PER_FRAME_DEFAULT = 1;

    // Defines maximum time in miliseconds while GetNextFrame will wait for next frame.
    static const wchar_t* DEM_WAIT_FRAME_MAX = L"WaitFrameMax";
    static const amf_uint32 DEM_WAIT_FRAME_MAX_DEFAULT = 500;
    static const amf_uint32 DEM_WAIT_FRAME_UNLIMITED = ~0;

    //----------------------------------------------------------------------------------------------
    // VCE-DEM buffer interface
    //----------------------------------------------------------------------------------------------
    class AMFDemBuffer : public virtual AMFInterface
    {
    public:
        AMF_DECLARE_IID(0x482e2939, 0x88f3, 0x4083, 0x91, 0xb1, 0xa6, 0x88, 0xe6, 0xa4, 0x54, 0xe1)

        virtual AMF_RESULT AMF_STD_CALL GetMemory(void** ppMem) = 0;
        virtual DemOutputType AMF_STD_CALL GetDataType() = 0;
        virtual amf_size AMF_STD_CALL GetMemorySize() = 0;
        virtual amf_int64 AMF_STD_CALL GetTimeStamp() = 0;
    };
    typedef AMFInterfacePtr_T<AMFDemBuffer> AMFDemBufferPtr;

    //----------------------------------------------------------------------------------------------
    // VCE-DEM Decoder interface
    //----------------------------------------------------------------------------------------------
    class AMFEncoderVCEDEM : public virtual AMFPropertyStorageEx
    {
    public:
        AMF_DECLARE_IID(0x4024dc39, 0xfefd, 0x4078, 0x91, 0x93, 0xaf, 0xed, 0xf8, 0x28, 0x6c, 0x61)

        // Initialization and termination
        virtual AMF_RESULT AMF_STD_CALL AcquireRemoteDisplay() = 0;
        virtual AMF_RESULT AMF_STD_CALL CreateEncoder() = 0;
        virtual AMF_RESULT AMF_STD_CALL StartEncoding() = 0;
        virtual AMF_RESULT AMF_STD_CALL StopEncoding() = 0;
        virtual AMF_RESULT AMF_STD_CALL DestroyEncoder() = 0;
        virtual AMF_RESULT AMF_STD_CALL ReleaseRemoteDisplay() = 0;

        // Buffer retrieving
        virtual AMF_RESULT AMF_STD_CALL GetNextFrame(AMFDemBuffer** buff) = 0;

        // This method must to be called from another thread
        // to terminate waiting inside GetNextThread
        virtual AMF_RESULT AMF_STD_CALL CancelGetNextFrame() = 0;

        // This method flushes configuration to encoder
        virtual AMF_RESULT AMF_STD_CALL FlushConfiguration() = 0;
    };
    typedef AMFInterfacePtr_T<AMFEncoderVCEDEM> AMFEncoderVCEDEMPtr;

}

//define export declaration
#ifdef _WIN32
    #if defined(AMF_VCEDEM_STATIC)
        #define AMF_VCEDEM_LINK
    #else
        #if defined(AMF_VCEDEM_EXPORTS)
            #define AMF_VCEDEM_LINK __declspec(dllexport)
        #else
            #define AMF_VCEDEM_LINK __declspec(dllimport)
        #endif
    #endif
#else // #ifdef _WIN32
    #define AMF_VCEDEM_LINK
#endif // #ifdef _WIN32



extern "C"
{
    AMF_VCEDEM_LINK AMF_RESULT AMF_CDECL_CALL AMFCreateEncoderVCEDEM(amf::AMFEncoderVCEDEM** encoder);
}

#endif //#ifndef __AMFEncoderVCEDEM_h__
