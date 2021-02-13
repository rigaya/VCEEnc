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
#include "vce_util.h"
#include "vce_param.h"
#include "VideoEncoderVCE.h"
#include "VideoEncoderHEVC.h"
#include "VideoDecoderUVD.h"
#include "Trace.h"
#include "Surface.h"
#include "rgy_frame.h"

static const auto RGY_CODEC_TO_VCE = make_array<std::pair<RGY_CODEC, const wchar_t *>>(
    std::make_pair(RGY_CODEC_UNKNOWN, nullptr),
    std::make_pair(RGY_CODEC_H264,    AMFVideoEncoderVCE_AVC),
    std::make_pair(RGY_CODEC_HEVC,    AMFVideoEncoder_HEVC)
    );
MAP_PAIR_0_1(codec, rgy, RGY_CODEC, enc, const wchar_t *, RGY_CODEC_TO_VCE, RGY_CODEC_UNKNOWN, nullptr);

static const auto VCE_CODEC_UVD_NAME = make_array<std::pair<RGY_CODEC, const wchar_t *>>(
    std::make_pair(RGY_CODEC_H264,  AMFVideoDecoderUVD_H264_AVC ),
    std::make_pair(RGY_CODEC_HEVC,  AMFVideoDecoderHW_H265_HEVC ),
    std::make_pair(RGY_CODEC_VC1,   AMFVideoDecoderUVD_VC1 ),
    //std::make_pair( RGY_CODEC_WMV3,  AMFVideoDecoderUVD_WMV3 ),
    std::make_pair(RGY_CODEC_VP9,   AMFVideoDecoderHW_VP9),
    std::make_pair(RGY_CODEC_MPEG2, AMFVideoDecoderUVD_MPEG2 )
);

MAP_PAIR_0_1(codec, rgy, RGY_CODEC, dec, const wchar_t *, VCE_CODEC_UVD_NAME, RGY_CODEC_UNKNOWN, nullptr);

const wchar_t * codec_rgy_to_dec_10bit(const RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_HEVC: return AMFVideoDecoderHW_H265_MAIN10;
    case RGY_CODEC_VP9:  return AMFVideoDecoderHW_VP9_10BIT;
    default:
        return nullptr;
    }
}

static const auto RGY_CSP_TO_VCE = make_array<std::pair<RGY_CSP, amf::AMF_SURFACE_FORMAT>>(
    std::make_pair(RGY_CSP_NA,        amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_NV12,      amf::AMF_SURFACE_NV12),
    std::make_pair(RGY_CSP_YV12,      amf::AMF_SURFACE_YV12),
    std::make_pair(RGY_CSP_YUY2,      amf::AMF_SURFACE_YUY2),
    std::make_pair(RGY_CSP_YUV422,    amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_YUV444,    amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_YV12_09,   amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_YV12_10,   amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_YV12_12,   amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_YV12_14,   amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_YV12_16,   amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_P010,      amf::AMF_SURFACE_P010),
    std::make_pair(RGY_CSP_YUV422_09, amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_YUV422_10, amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_YUV422_12, amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_YUV422_14, amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_YUV422_16, amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_P210,      amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_YUV444_09, amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_YUV444_10, amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_YUV444_12, amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_YUV444_14, amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_YUV444_16, amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_RGB24R,    amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_RGB32R,    amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_RGB24,     amf::AMF_SURFACE_UNKNOWN),
    std::make_pair(RGY_CSP_RGB32,     amf::AMF_SURFACE_ARGB),
    std::make_pair(RGY_CSP_YC48,      amf::AMF_SURFACE_UNKNOWN)
    );

MAP_PAIR_0_1(csp, rgy, RGY_CSP, enc, amf::AMF_SURFACE_FORMAT, RGY_CSP_TO_VCE, RGY_CSP_NA, amf::AMF_SURFACE_UNKNOWN);

static const auto RGY_LOGLEVEL_TO_VCE = make_array<std::pair<int, int>>(
    std::make_pair(RGY_LOG_TRACE, AMF_TRACE_TRACE),
    std::make_pair(RGY_LOG_DEBUG, AMF_TRACE_DEBUG),
    std::make_pair(RGY_LOG_INFO,  AMF_TRACE_INFO),
    std::make_pair(RGY_LOG_WARN,  AMF_TRACE_WARNING),
    std::make_pair(RGY_LOG_ERROR, AMF_TRACE_ERROR)
    );
MAP_PAIR_0_1(loglevel, rgy, int, enc, int, RGY_LOGLEVEL_TO_VCE, RGY_LOG_INFO, AMF_TRACE_INFO);


static const auto RGY_PICSTRUCT_TO_VCE = make_array<std::pair<RGY_PICSTRUCT, amf::AMF_FRAME_TYPE>>(
    std::make_pair(RGY_PICSTRUCT_UNKNOWN,      amf::AMF_FRAME_UNKNOWN),
    std::make_pair(RGY_PICSTRUCT_FRAME,        amf::AMF_FRAME_PROGRESSIVE),
    std::make_pair(RGY_PICSTRUCT_FRAME_TFF,    amf::AMF_FRAME_INTERLEAVED_EVEN_FIRST),
    std::make_pair(RGY_PICSTRUCT_FRAME_BFF,    amf::AMF_FRAME_INTERLEAVED_ODD_FIRST),
    std::make_pair(RGY_PICSTRUCT_TFF,          amf::AMF_FRAME_INTERLEAVED_EVEN_FIRST),
    std::make_pair(RGY_PICSTRUCT_BFF,          amf::AMF_FRAME_INTERLEAVED_ODD_FIRST),
    std::make_pair(RGY_PICSTRUCT_FIELD_TOP,    amf::AMF_FRAME_FIELD_SINGLE_EVEN),
    std::make_pair(RGY_PICSTRUCT_FIELD_BOTTOM, amf::AMF_FRAME_FIELD_SINGLE_ODD)
    );
MAP_PAIR_0_1(frametype, rgy, RGY_PICSTRUCT, enc, amf::AMF_FRAME_TYPE, RGY_PICSTRUCT_TO_VCE, RGY_PICSTRUCT_UNKNOWN, amf::AMF_FRAME_UNKNOWN);

__declspec(noinline)
AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_ENUM picstruct_rgy_to_enc(RGY_PICSTRUCT picstruct) {
    if (picstruct & RGY_PICSTRUCT_TFF) return AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_TOP_FIELD;
    if (picstruct & RGY_PICSTRUCT_BFF) return AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_BOTTOM_FIELD;
    return AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_FRAME;
}

__declspec(noinline)
RGY_PICSTRUCT picstruct_enc_to_rgy(AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_ENUM picstruct) {
    if (picstruct == AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_TOP_FIELD) return RGY_PICSTRUCT_FRAME_TFF;
    if (picstruct == AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_BOTTOM_FIELD) return RGY_PICSTRUCT_FRAME_BFF;
    return RGY_PICSTRUCT_FRAME;
}

const TCHAR *AMFRetString(AMF_RESULT ret) {
#define AMFRESULT_TO_STR(x) case x: return _T( #x );
    switch (ret) {
        AMFRESULT_TO_STR(AMF_OK)
        AMFRESULT_TO_STR(AMF_FAIL)

        // common errors
        AMFRESULT_TO_STR(AMF_UNEXPECTED)

        AMFRESULT_TO_STR(AMF_ACCESS_DENIED)
        AMFRESULT_TO_STR(AMF_INVALID_ARG)
        AMFRESULT_TO_STR(AMF_OUT_OF_RANGE)

        AMFRESULT_TO_STR(AMF_OUT_OF_MEMORY)
        AMFRESULT_TO_STR(AMF_INVALID_POINTER)

        AMFRESULT_TO_STR(AMF_NO_INTERFACE)
        AMFRESULT_TO_STR(AMF_NOT_IMPLEMENTED)
        AMFRESULT_TO_STR(AMF_NOT_SUPPORTED)
        AMFRESULT_TO_STR(AMF_NOT_FOUND)

        AMFRESULT_TO_STR(AMF_ALREADY_INITIALIZED)
        AMFRESULT_TO_STR(AMF_NOT_INITIALIZED)

        AMFRESULT_TO_STR(AMF_INVALID_FORMAT)// invalid data format

        AMFRESULT_TO_STR(AMF_WRONG_STATE)
        AMFRESULT_TO_STR(AMF_FILE_NOT_OPEN)// cannot open file

                                            // device common codes
        AMFRESULT_TO_STR(AMF_NO_DEVICE)

        // device directx
        AMFRESULT_TO_STR(AMF_DIRECTX_FAILED)
        // device opencl
        AMFRESULT_TO_STR(AMF_OPENCL_FAILED)
        // device opengl
        AMFRESULT_TO_STR(AMF_GLX_FAILED)//failed to use GLX
                                        // device XV
        AMFRESULT_TO_STR(AMF_XV_FAILED) //failed to use Xv extension
                                        // device alsa
        AMFRESULT_TO_STR(AMF_ALSA_FAILED)//failed to use ALSA

                                            // component common codes

                                            //result codes
        AMFRESULT_TO_STR(AMF_EOF)
        AMFRESULT_TO_STR(AMF_REPEAT)
        AMFRESULT_TO_STR(AMF_INPUT_FULL)//returned by AMFComponent::SubmitInput if input queue is full
        AMFRESULT_TO_STR(AMF_RESOLUTION_CHANGED)//resolution changed client needs to Drain/Terminate/Init
        AMFRESULT_TO_STR(AMF_RESOLUTION_UPDATED)//resolution changed in adaptive mode. New ROI will be set on output on newly decoded frames

                                                //error codes
        AMFRESULT_TO_STR(AMF_INVALID_DATA_TYPE)//invalid data type
        AMFRESULT_TO_STR(AMF_INVALID_RESOLUTION)//invalid resolution (width or height)
        AMFRESULT_TO_STR(AMF_CODEC_NOT_SUPPORTED)//codec not supported
        AMFRESULT_TO_STR(AMF_SURFACE_FORMAT_NOT_SUPPORTED)//surface format not supported
        AMFRESULT_TO_STR(AMF_SURFACE_MUST_BE_SHARED)//surface should be shared (DX11: (MiscFlags & D3D11_RESOURCE_MISC_SHARED) == 0) DX9: No shared handle found)

                                                    // component video decoder
        AMFRESULT_TO_STR(AMF_DECODER_NOT_PRESENT)//failed to create the decoder
        AMFRESULT_TO_STR(AMF_DECODER_SURFACE_ALLOCATION_FAILED)//failed to create the surface for decoding
        AMFRESULT_TO_STR(AMF_DECODER_NO_FREE_SURFACES)

        // component video encoder
        AMFRESULT_TO_STR(AMF_ENCODER_NOT_PRESENT)//failed to create the encoder

                                                    // component video processor

                                                    // component video conveter

                                                    // component dem
        AMFRESULT_TO_STR(AMF_DEM_ERROR)
        AMFRESULT_TO_STR(AMF_DEM_PROPERTY_READONLY)
        AMFRESULT_TO_STR(AMF_DEM_REMOTE_DISPLAY_CREATE_FAILED)
        AMFRESULT_TO_STR(AMF_DEM_START_ENCODING_FAILED)
        AMFRESULT_TO_STR(AMF_DEM_QUERY_OUTPUT_FAILED)

        // component TAN
        AMFRESULT_TO_STR(AMF_TAN_CLIPPING_WAS_REQUIRED) // Resulting data was truncated to meet output type's value limits.
        AMFRESULT_TO_STR(AMF_TAN_UNSUPPORTED_VERSION) // Not supported version requested) solely for TANCreateContext().

        AMFRESULT_TO_STR(AMF_NEED_MORE_INPUT)//returned by AMFComponent::SubmitInput did not produce buffer
    default:
        return _T("Unknown");
    }
#undef AMFRESULT_TO_STR
}

tstring AccelTypeToString(amf::AMF_ACCELERATION_TYPE accelType) {
    tstring strValue;
    switch (accelType) {
    case amf::AMF_ACCEL_NOT_SUPPORTED:
        strValue = _T("Not supported");
        break;
    case amf::AMF_ACCEL_HARDWARE:
        strValue = _T("Hardware-accelerated");
        break;
    case amf::AMF_ACCEL_GPU:
        strValue = _T("GPU-accelerated");
        break;
    case amf::AMF_ACCEL_SOFTWARE:
        strValue = _T("Not accelerated (software)");
        break;
    }
    return strValue;
}

void RGYBitstream::addFrameData(RGYFrameData *frameData) {
    if (frameData != nullptr) {
        frameDataNum++;
        frameDataList = (RGYFrameData **)realloc(frameDataList, frameDataNum * sizeof(frameDataList[0]));
        frameDataList[frameDataNum - 1] = frameData;
    }
}

void RGYBitstream::clearFrameDataList() {
    frameDataNum = 0;
    if (frameDataList) {
        for (int i = 0; i < frameDataNum; i++) {
            if (frameDataList[i]) {
                delete frameDataList[i];
            }
        }
        free(frameDataList);
        frameDataList = nullptr;
    }
}
std::vector<RGYFrameData *> RGYBitstream::getFrameDataList() {
    return make_vector(frameDataList, frameDataNum);
}

__declspec(noinline)
VideoInfo videooutputinfo(
    RGY_CODEC codec,
    amf::AMF_SURFACE_FORMAT encFormat,
    const AMFParams& prm,
    RGY_PICSTRUCT picstruct,
    const VideoVUIInfo& vui) {

    const int bframes = (codec == RGY_CODEC_H264) ? prm.get<int>(AMF_VIDEO_ENCODER_B_PIC_PATTERN) : 0;

    VideoInfo info;
    memset(&info, 0, sizeof(info));
    info.codec = codec;
    info.codecLevel = prm.get<int>(AMF_PARAM_PROFILE_LEVEL(codec));
    info.codecProfile = prm.get<int>(AMF_PARAM_PROFILE(codec));
    info.videoDelay = ((bframes > 0) + (bframes > 2));
    info.dstWidth = prm.get<int>(VCE_PARAM_KEY_OUTPUT_WIDTH);
    info.dstHeight = prm.get<int>(VCE_PARAM_KEY_OUTPUT_HEIGHT);
    info.fpsN = prm.get<AMFRate>(AMF_PARAM_FRAMERATE(codec)).num;
    info.fpsD = prm.get<AMFRate>(AMF_PARAM_FRAMERATE(codec)).den;
    info.sar[0] = prm.get<AMFRatio>(AMF_PARAM_ASPECT_RATIO(codec)).num;
    info.sar[1] = prm.get<AMFRatio>(AMF_PARAM_ASPECT_RATIO(codec)).den;
    adjust_sar(&info.sar[0], &info.sar[1], info.dstWidth, info.dstHeight);
    info.picstruct = picstruct;
    info.csp = csp_enc_to_rgy(encFormat);
    info.vui = vui;
    return info;
}

//TODO: ちゃんと動的にチェックする
CodecCsp getHWDecCodecCsp(const bool skipHWDecodeCheck) {
#if ENABLE_AVSW_READER
    std::vector<RGY_CSP> supportedCsp = { RGY_CSP_NV12, RGY_CSP_YV12 };
    CodecCsp codecCsp;
    for (int i = 0; i < _countof(HW_DECODE_LIST); i++) {
        codecCsp[HW_DECODE_LIST[i].rgy_codec] = supportedCsp;
    }
    codecCsp[RGY_CODEC_HEVC].push_back(RGY_CSP_YV12_10);
    codecCsp[RGY_CODEC_HEVC].push_back(RGY_CSP_YV12_12);
    codecCsp[RGY_CODEC_VP9].push_back(RGY_CSP_YV12_10);
    codecCsp[RGY_CODEC_VP9].push_back(RGY_CSP_YV12_12);
    return codecCsp;
#else
    return CodecCsp();
#endif
}


#if !ENABLE_AVSW_READER
#define TTMATH_NOASM
#pragma warning(push)
#pragma warning(disable: 4244)
#include "ttmath/ttmath.h"
#pragma warning(pop)

int64_t rational_rescale(int64_t v, rgy_rational<int> from, rgy_rational<int> to) {
    auto mul = rgy_rational<int64_t>((int64_t)from.n() * (int64_t)to.d(), (int64_t)from.d() * (int64_t)to.n());

#if _M_IX86
#define RESCALE_INT_SIZE 4
#else
#define RESCALE_INT_SIZE 2
#endif
    ttmath::Int<RESCALE_INT_SIZE> tmp1 = v;
    tmp1 *= mul.n();
    ttmath::Int<RESCALE_INT_SIZE> tmp2 = mul.d();

    tmp1 = (tmp1 + tmp2 - 1) / tmp2;
    int64_t ret;
    tmp1.ToInt(ret);
    return ret;
}

#endif