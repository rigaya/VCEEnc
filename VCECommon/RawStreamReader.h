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
 * @file <RawStreamReader.h>
 *
 * @brief Header file for Raw Stream Reader
 *
 *******************************************************************************
 */

#pragma once

#include "PipelineElement.h"
#include "ParametersStorage.h"
#include "ByteArray.h"

class RawStreamReader: public PipelineElement
{
public:
    RawStreamReader();
    virtual ~RawStreamReader();

    virtual AMF_RESULT Init(ParametersStorage* pParams,
                    amf::AMFContext* pContext);

    virtual amf::AMF_SURFACE_FORMAT GetFormat()
    {
        return m_format;
    }
    virtual amf_int32 GetWidth()
    {
        return m_width;
    }
    virtual amf_int32 GetHeight()
    {
        return m_height;
    }
    virtual amf::AMF_MEMORY_TYPE GetMemoryType()
    {
        return m_memoryType;
    }

    virtual amf_int32 GetInputSlotCount()
    {
        return 0;
    }
    virtual amf_double GetPosition()
    {
        return static_cast<amf_double> (m_framesCountRead) / m_framesCount;
    }
private:
    virtual AMF_RESULT SubmitInput(amf::AMFData* pData);
    virtual AMF_RESULT QueryOutput(amf::AMFData** ppData);

    virtual AMF_RESULT Terminate();
    AMF_RESULT ReadNextFrame(int dstStride, int dstHeight, int valignment,
                    unsigned char* pDstBits);

    amf::AMFContextPtr m_pContext;
    AMFDataStreamPtr m_pDataStream;

    amf::AMF_SURFACE_FORMAT m_format;
    amf::AMF_MEMORY_TYPE m_memoryType;
    amf_int32 m_width;
    amf_int32 m_height;
    amf_int32 m_stride;

    amf_int64 m_framesCount;
    amf_int64 m_framesCountRead;

    AMFByteArray m_frame;

    amf::AMF_SURFACE_FORMAT m_fileFormat;
};

typedef std::shared_ptr<RawStreamReader> RawStreamReaderPtr;
