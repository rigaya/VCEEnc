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
 * @file <RawStreamReader.cpp>
 *
 * @brief Source file for Raw Stream Reader
 *
 *******************************************************************************
 */

#include "RawStreamReader.h"
#include "CmdLogger.h"
#include "FileHelper.h"

static bool PicGetStride(amf::AMF_SURFACE_FORMAT eFormat, amf_int32 width,
                amf_int32& stride)
{
    stride = 0;
    switch (eFormat)
    {
    case amf::AMF_SURFACE_NV12:
        stride = width % 2 ? width + 1 : width;
        break;
    case amf::AMF_SURFACE_YUV420P:
    case amf::AMF_SURFACE_YV12:
        stride = width;
        break;
    case amf::AMF_SURFACE_BGRA:
    case amf::AMF_SURFACE_RGBA:
    case amf::AMF_SURFACE_ARGB:
        stride = width * 4;
        break;
    default:
        LOG_ERROR("Stride cannot be calculated, unknown pixel format");
        return false;
    }
    return true;
}

static bool PicGetFrameSize(amf::AMF_SURFACE_FORMAT format, amf_int32 width,
                amf_int32 height, int& size)
{
    size = 0;
    amf_int32 stride;
    if (!PicGetStride(format, width, stride))
    {
        return false;
    }

    switch (format)
    {
    case amf::AMF_SURFACE_YUV420P:
    case amf::AMF_SURFACE_YV12:
    case amf::AMF_SURFACE_NV12:
        size = stride * height + stride / 2 * height / 2 * 2; // 2 planes
        break;
    case amf::AMF_SURFACE_BGRA:
    case amf::AMF_SURFACE_RGBA:
    case amf::AMF_SURFACE_ARGB:
        size = stride * height;
        break;
    default:
        LOG_ERROR("Frame size cannot be calculated, unknown pixel format");
        return false;
    }
    return true;
}

static void PlaneCopy(const amf_uint8 *src, amf_int32 srcStride,
                amf_int32 srcHeight, amf_uint8 *dst, amf_int32 dstStride,
                amf_int32 dstHeight)
{
    amf_int32 minHeight = AMF_MIN(srcHeight, dstHeight);
    if (srcStride == dstStride)
    {
        memcpy(dst, src, minHeight * srcStride);
    }
    else
    {
        int minStride = AMF_MIN(srcStride, dstStride);
        for (int y = 0; y < minHeight; y++)
        {
            memcpy(dst + dstStride * y, src + srcStride * y, minStride);
        }
    }
}

static void NV12PicCopy(const amf_uint8 *src, amf_int32 srcStride,
                amf_int32 srcHeight, amf_uint8 *dst, amf_int32 dstStride,
                amf_int32 dstHeight)
{
    // Y- plane
    PlaneCopy(src, srcStride, srcHeight, dst, dstStride, dstHeight);
    // UV - plane
    amf_int32 srcYSize = srcHeight * srcStride;
    amf_int32 dstYSize = dstHeight * dstStride;
    PlaneCopy(src + srcYSize, srcStride, srcHeight / 2, dst + dstYSize,
                    dstStride, dstHeight / 2);
}

static void YUV420PicCopy(const amf_uint8 *src, amf_int32 srcStride,
                amf_int32 srcHeight, amf_uint8 *dst, amf_int32 dstStride,
                amf_int32 dstHeight)
{
    // Y- plane
    PlaneCopy(src, srcStride, srcHeight, dst, dstStride, dstHeight);

    // U - plane
    amf_int32 srcYSize = srcHeight * srcStride;
    amf_int32 dstYSize = dstHeight * dstStride;
    PlaneCopy(src + srcYSize, srcStride / 2, srcHeight / 2, dst + dstYSize,
                    dstStride / 2, dstHeight / 2);

    // V Plane
    amf_int32 srcUSize = srcHeight / 2 * srcStride / 2;
    amf_int32 dstUSize = dstHeight / 2 * dstStride / 2;
    PlaneCopy(src + srcYSize + srcUSize, srcStride / 2, srcHeight / 2, dst
                    + dstYSize + dstUSize, dstStride / 2, dstHeight / 2);
}

RawStreamReader::RawStreamReader() :
    m_pDataStream(), m_format(amf::AMF_SURFACE_UNKNOWN), m_memoryType(
                    amf::AMF_MEMORY_UNKNOWN), m_width(0), m_height(0),
                    m_stride(0), m_framesCount(0), m_framesCountRead(0),
                    m_frame()
{
}

RawStreamReader::~RawStreamReader()
{
    Terminate();
}

AMF_RESULT RawStreamReader::Init(ParametersStorage* pParams,
                amf::AMFContext* pContext)
{
    AMF_RESULT res = AMF_OK;
    m_pContext = pContext;
    std::wstring path;
    res = pParams->GetParamWString(PARAM_NAME_INPUT, path);

    if (amf_path_is_relative(path.c_str()))
    {
        std::wstring dir;
        if (pParams->GetParamWString(PARAM_NAME_INPUT_DIR, dir) == AMF_OK)
        {
            path = dir + PATH_SEPARATOR_WSTR + path;
        }
    }

    amf_int width = 0;
    amf_int height = 0;

    pParams->GetParam(PARAM_NAME_INPUT_WIDTH, width);
    pParams->GetParam(PARAM_NAME_INPUT_HEIGHT, height);
    amf::AMF_SURFACE_FORMAT inputFormat = amf::AMF_SURFACE_UNKNOWN;
    std::wstring inputFormatStr;
    if (pParams->GetParamWString(PARAM_NAME_INPUT_FORMAT, inputFormatStr)
                    == AMF_OK)
    {
        inputFormat = GetFormatFromString(inputFormatStr.c_str());
    }

    amf_int frames = 0;
    pParams->GetParam(PARAM_NAME_INPUT_FRAMES, frames);
    m_width = width;
    m_height = height;
    m_format = inputFormat;

    if (m_width == 0 || m_height == 0 || m_format == amf::AMF_SURFACE_UNKNOWN)
    {
        ParseRawFileFormat(path, m_width, m_height, m_format);
    }

    if (m_format == amf::AMF_SURFACE_UNKNOWN)
    {
        LOG_ERROR(L"Unknown file format: inputFileName - " << path);
        return AMF_FAIL;
    }

    if (m_format != amf::AMF_SURFACE_YUV420P && m_format
                    != amf::AMF_SURFACE_BGRA && m_format
                    != amf::AMF_SURFACE_RGBA && m_format
                    != amf::AMF_SURFACE_NV12 && m_format
                    != amf::AMF_SURFACE_ARGB && m_format
                    != amf::AMF_SURFACE_YV12)
    {
        LOG_ERROR(
                        "Only YUV420P or BGRA or NV12 or RGBA or ARGB or YV12 picture format supported");
        return AMF_FAIL;
    }

    m_pDataStream = AMFDataStream::Create(path.c_str(), AMF_FileRead);
    if (!m_pDataStream)
    {
        LOG_ERROR("Cannot open input file: " << path.c_str());
        return AMF_FAIL;
    }

    if (!PicGetStride(m_format, m_width, m_stride))
    {
        LOG_ERROR("Wrong format:" << m_format);
        return AMF_FAIL;
    }
    int frameSize = 0;
    if (!PicGetFrameSize(m_format, m_width, m_height, frameSize))
    {
        LOG_ERROR("Wrong format:" << m_format);
        return AMF_FAIL;
    }
    m_frame.SetSize(frameSize);

    if (!m_stride || !frameSize)
    {
        LOG_ERROR("Could not define frame size for current frame format");
        return AMF_FAIL;
    }
    m_framesCount = static_cast<int> (m_pDataStream->Size() / frameSize);
    if(frames)
    {
        m_framesCount = AMF_MIN(frames, m_framesCount);
    }
    return AMF_OK;
}

AMF_RESULT RawStreamReader::Terminate()
{
    AMF_RESULT res = AMF_OK;
    m_pDataStream = NULL;
    m_pContext = NULL;
    return res;
}

AMF_RESULT RawStreamReader::SubmitInput(amf::AMFData* pData)
{
    return AMF_NOT_SUPPORTED;
}

AMF_RESULT RawStreamReader::QueryOutput(amf::AMFData** ppData)
{
    AMF_RESULT res = AMF_OK;
    amf::AMFSurfacePtr pSurface;
    res = m_pContext->AllocSurface(amf::AMF_MEMORY_HOST, m_format, m_width,
                    m_height, &pSurface);
    CHECK_AMF_ERROR_RETURN(res,
                    L"AMFContext::AllocSurface(amf::AMF_MEMORY_HOST) failed");

    amf::AMFPlanePtr plane = pSurface->GetPlaneAt(0);
    res = ReadNextFrame(plane->GetHPitch(), m_height, plane->GetVPitch(),
                    static_cast<unsigned char*> (plane->GetNative()));
    if (res == AMF_EOF)
    {
        return res;
    }
    CHECK_AMF_ERROR_RETURN(res, L"ReadNextFrame() failed");
    *ppData = pSurface.Detach();
    return AMF_OK;
}

AMF_RESULT RawStreamReader::ReadNextFrame(int dstStride, int dstHeight,
                int valignment, unsigned char* pDstBits)
{
    if (m_framesCountRead == m_framesCount)
    {
        return AMF_EOF;
    }

    {
        amf_size read = m_pDataStream->Read(m_frame.GetData(),
                        m_frame.GetSize());
        if (read != m_frame.GetSize())
        {
            return AMF_EOF;
        }
        m_framesCountRead++;
    }

    switch (m_format)
    {
    case amf::AMF_SURFACE_BGRA:
    case amf::AMF_SURFACE_RGBA:
    case amf::AMF_SURFACE_ARGB:
        PlaneCopy(m_frame.GetData(), m_stride, m_height, pDstBits, dstStride,
                        valignment);
        break;
    case amf::AMF_SURFACE_YUV420P:
    case amf::AMF_SURFACE_YV12:
        YUV420PicCopy(m_frame.GetData(), m_stride, m_height, pDstBits,
                        dstStride, valignment);
        break;
    case amf::AMF_SURFACE_NV12:
        NV12PicCopy(m_frame.GetData(), m_stride, m_height, pDstBits, dstStride,
                        valignment);
        break;
    default:
        LOG_ERROR("Format reading is not supported");
        return AMF_FAIL;
    }
    return AMF_OK;
}
