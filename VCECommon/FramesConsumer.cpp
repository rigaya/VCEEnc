/*******************************************************************************
 Copyright ｩ2014 Advanced Micro Devices, Inc. All rights reserved.

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
 * @file <FramesConsumer.cpp>
 *
 * @brief Source file for the FramesConsumer
 *
 *******************************************************************************
 */
#include "FramesConsumer.h"
#include "CmdLogger.h"
#include "FileHelper.h"

FramesConsumer::FramesConsumer()
{
}

FramesConsumer::~FramesConsumer()
{
    Terminate();
}

AMF_RESULT FramesConsumer::Init(ParametersStorage* pParams,
                amf::AMFContext* pContext)
{
    pParams->GetParamWString(PARAM_NAME_OUTPUT, m_output);
    if (amf_path_is_relative(m_output.c_str()))
    {
        std::wstring dir;
        if (pParams->GetParamWString(PARAM_NAME_OUTPUT_DIR, dir) == AMF_OK)
        {
            m_output = dir + PATH_SEPARATOR_WSTR + m_output;
        }
    }

    amf_int width = 0; // if 0 - no scaling
    amf_int height = 0; // if 0 - no scaling

    pParams->GetParam(PARAM_NAME_OUTPUT_WIDTH, width);
    pParams->GetParam(PARAM_NAME_OUTPUT_HEIGHT, height);

    amf::AMF_SURFACE_FORMAT format = amf::AMF_SURFACE_UNKNOWN;
    std::wstring outputFormatStr;
    if (pParams->GetParamWString(PARAM_NAME_OUTPUT_FORMAT, outputFormatStr)
                    == AMF_OK)
    {
        format = GetFormatFromString(outputFormatStr.c_str());
    }

    m_width = width;
    m_height = height;
    m_format = format;

    if (m_width == 0 || m_height == 0 || m_format == amf::AMF_SURFACE_UNKNOWN)
    {
        ParseRawFileFormat(m_output.c_str(), m_width, m_height, m_format);
    }

    if (m_format == amf::AMF_SURFACE_UNKNOWN)
    {
        LOG_ERROR(L"Unknown file format: inputFileName - " << m_output);
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

    return AMF_OK;
}

AMF_RESULT FramesConsumer::Terminate()
{
    AMF_RESULT res = AMF_OK;
    return res;
}

