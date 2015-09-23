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
 * @file <RawStreamWriter.cpp>
 *
 * @brief Source file for Raw Stream Writer
 *
 *******************************************************************************
 */

#include "RawStreamWriter.h"

#include "CmdLogger.h"
#include "FileHelper.h"

RawStreamWriter::RawStreamWriter()
{
}

RawStreamWriter::~RawStreamWriter()
{
    Terminate();
}

AMF_RESULT RawStreamWriter::Init(ParametersStorage* pParams,
                amf::AMFContext* pContext)
{
    AMF_RESULT res = AMF_OK;
    res = FramesConsumer::Init(pParams, pContext);
    CHECK_AMF_ERROR_RETURN(res, L"FramesConsumer::Init failed");

    m_pDataStream = AMFDataStream::Create(m_output.c_str(), AMF_FileWrite);
    CHECK_RETURN(m_pDataStream != NULL, AMF_FILE_NOT_OPEN, "Open File");

    return res;
}

AMF_RESULT RawStreamWriter::Terminate()
{
    AMF_RESULT res = AMF_OK;
    m_pDataStream = NULL;

    return res;
}

AMF_RESULT RawStreamWriter::SubmitInput(amf::AMFData* pData)
{
    if (pData->GetDataType() == amf::AMF_DATA_SURFACE)
    {
        amf::AMFSurfacePtr pSurface(pData);
        pSurface->Convert(amf::AMF_MEMORY_HOST);

        for (amf_size i = 0; i < pSurface->GetPlanesCount(); i++)
        {
            amf::AMFPlane* plane = pSurface->GetPlaneAt(i);
            int width = plane->GetWidth();
            int hPitch = plane->GetHPitch();
            int height = plane->GetHeight();
            int offsetX = plane->GetOffsetX();
            int offsetY = plane->GetOffsetY();
            int pixelInBytes = plane->GetPixelSizeInBytes();
            unsigned char* ptr = (unsigned char*) plane->GetNative();
            for (int h = 0; h < height; h++)
            {
                unsigned char* write_ptr = ptr + hPitch * (h + offsetY)
                                + offsetX * pixelInBytes;
                amf_size size = width * pixelInBytes;
                m_pDataStream->Write(write_ptr, size);
            }
        }
        return AMF_OK;
    }

    if (pData->GetDataType() == amf::AMF_DATA_BUFFER)
    {
        amf::AMFBufferPtr pBuffer(pData);
        pBuffer->Convert(amf::AMF_MEMORY_HOST);

        unsigned char* write_ptr = (unsigned char*) pBuffer->GetNative();
        amf_size size = pBuffer->GetSize();
        m_pDataStream->Write(write_ptr, size);
        return AMF_OK;
    }
    return AMF_INVALID_DATA_TYPE;
}
