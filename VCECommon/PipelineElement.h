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
 * @file <PipelineElement.h>
 *
 * @brief Header file for Pipeline Element
 *
 *******************************************************************************
 */

#pragma once

#include "AMFPlatform.h"
#include "Thread.h"
#include "CmdLogger.h"
#include "Surface.h"
#include "Context.h"
#include "Component.h"
#include <vector>

#pragma warning(push)
#pragma warning(disable: 4100)

class Pipeline;
class PipelineElement
{
public:
    virtual amf_int32 GetInputSlotCount()
    {
        return 1;
    }
    virtual amf_int32 GetOutputSlotCount()
    {
        return 1;
    }
    virtual AMF_RESULT SubmitInput(amf::AMFData* pData, amf_int32 slot)
    {
        return SubmitInput(pData);
    }
    virtual AMF_RESULT QueryOutput(amf::AMFData** ppData, amf_int32 slot)
    {
        return QueryOutput(ppData);
    }
    virtual AMF_RESULT SubmitInput(amf::AMFData* pData)
    {
        return AMF_NOT_SUPPORTED;
    }
    virtual AMF_RESULT QueryOutput(amf::AMFData** ppData)
    {
        return AMF_NOT_SUPPORTED;
    }

    virtual AMF_RESULT Drain()
    {
        return AMF_NOT_SUPPORTED;
    }
    virtual std::wstring GetDisplayResult()
    {
        return std::wstring();
    }

    virtual ~PipelineElement()
    {
    }
protected:
    PipelineElement() :
        m_host(0)
    {
    }

    Pipeline* m_host;
};

typedef std::shared_ptr<PipelineElement> PipelineElementPtr;

class StreamWriter: public PipelineElement
{
public:
    StreamWriter(AMFDataStreamPtr pDataStream) :
        m_pDataStream(pDataStream), m_framesWritten(0), m_maxSize(0),
                        m_totalSize(0)
    {
    }

    virtual ~StreamWriter()
    {
        //        LOG_DEBUG(L"Stream Writer: written frames:" << m_framesWritten << L"\n");
    }

    virtual amf_int32 GetOutputSlotCount()
    {
        return 0;
    }

    virtual AMF_RESULT SubmitInput(amf::AMFData* pData)
    {
        AMF_RESULT res = AMF_OK;
        if (pData)
        {
            amf::AMFBufferPtr pBuffer(pData);

            amf_size towrite = pBuffer->GetSize();
            m_pDataStream->Write(pBuffer->GetNative(), towrite);
            m_framesWritten++;
            if (m_maxSize < towrite)
            {
                m_maxSize = towrite;
            }
            m_totalSize += towrite;
        }
        else
        {
            res = AMF_EOF;
        }
        return res;
    }
    virtual AMF_RESULT QueryOutput(amf::AMFData** ppData)
    {
        return AMF_NOT_SUPPORTED;
    }
    virtual std::wstring GetDisplayResult()
    {
        std::wstring ret;

        if (m_framesWritten > 0)
        {
            std::wstringstream messageStream;
            messageStream << L" Average (Max) Frame size: " << m_totalSize
                            / m_framesWritten << L" bytes (" << m_maxSize
                            << " bytes)";
            ret = messageStream.str();
        }
        return ret;
    }

private:
    AMFDataStreamPtr m_pDataStream;
    amf_int m_framesWritten;
    amf_size m_maxSize;
    amf_int64 m_totalSize;
};

typedef std::shared_ptr<StreamWriter> StreamWriterPtr;

class SurfaceWriter: public PipelineElement
{
public:
    SurfaceWriter(AMFDataStreamPtr pDataStream) :
        m_pDataStream(pDataStream), m_framesWritten(0)
    {
    }

    virtual ~SurfaceWriter()
    {
        //        LOG_DEBUG(L"Stream Writer: written frames:" << m_framesWritten << L"\n");
    }

    virtual amf_int32 GetOutputSlotCount()
    {
        return 0;
    }

    virtual AMF_RESULT SubmitInput(amf::AMFData* pData)
    {
        AMF_RESULT res = AMF_OK;
        if (pData)
        {
            amf::AMFSurfacePtr pSurface(pData);
            pSurface->Convert(amf::AMF_MEMORY_HOST);

            for (amf_size i = 0; i < pSurface->GetPlanesCount(); i++)
            {
                amf::AMFPlanePtr pPlane = pSurface->GetPlaneAt(i);
                amf_size towrite = (pPlane->GetOffsetY() + pPlane->GetHeight())
                                * pPlane->GetHPitch();
                m_pDataStream->Write(pPlane->GetNative(), towrite);
            }
            m_framesWritten++;
        }
        else
        {
            res = AMF_EOF;
        }
        return res;
    }
    virtual AMF_RESULT QueryOutput(amf::AMFData** ppData)
    {
        return AMF_NOT_SUPPORTED;
    }

private:
    AMFDataStreamPtr m_pDataStream;
    amf_int m_framesWritten;
};

typedef std::shared_ptr<SurfaceWriter> SurfaceWriterPtr;

class DummyWriter: public PipelineElement
{
public:
    DummyWriter()
    {
    }
    virtual ~DummyWriter()
    {
    }

    virtual amf_int32 GetOutputSlotCount()
    {
        return 0;
    }

    virtual AMF_RESULT SubmitInput(amf::AMFData* pData)
    {
        if (!pData)
        {
            return AMF_EOF;
        }
        return AMF_OK;
    }
    virtual AMF_RESULT QueryOutput(amf::AMFData** ppData)
    {
        return AMF_NOT_SUPPORTED;
    }
};
typedef std::shared_ptr<DummyWriter> DummyWriterPtr;

class Splitter: public PipelineElement
{
protected:
    struct Data
    {
        amf::AMFDataPtr data;
        std::vector<bool> slots;
    };
public:
    Splitter(bool bCopyData = false, amf_int32 outputCount = 2,
                    amf_size queueSize = 1) :
        m_bCopyData(bCopyData), m_iOutputCount(outputCount), m_QueueSize(
                        queueSize), m_bEof(false)
    {
    }
    virtual ~Splitter()
    {
    }
    virtual amf_int32 GetInputSlotCount()
    {
        return 1;
    }
    virtual amf_int32 GetOutputSlotCount()
    {
        return m_iOutputCount;
    }
    virtual AMF_RESULT SubmitInput(amf::AMFData* pData)
    {
        AMFLock lock(&m_cs);
        if (m_Queue.size() >= m_QueueSize)
        {
            return AMF_INPUT_FULL;
        }
        Data data;
        data.data = pData;
        data.slots.resize(m_iOutputCount, false); // set all slots to false
        m_Queue.push_back(data);

        return AMF_OK;
    }
    virtual AMF_RESULT QueryOutput(amf::AMFData** ppData, amf_int32 slot)
    {
        AMFLock lock(&m_cs);
        if (slot >= m_iOutputCount)
        {
            //LOG_ERROR(L"Bad slot=" << slot);
            return AMF_INVALID_ARG;
        }
        AMF_RESULT res = m_bEof ? AMF_EOF : AMF_REPEAT;
        for (std::list<Data>::iterator it = m_Queue.begin(); it
                        != m_Queue.end(); it++)
        {
            if (!it->slots[slot])
            {
                it->slots[slot] = true;
                if (it->data == NULL)
                {
                    res = AMF_EOF;
                }
                else
                {
                    res = AMF_OK;
                    if (m_bCopyData)
                    {
                        res = it->data->Duplicate(it->data->GetMemoryType(),
                                        ppData);
                    }
                    else
                    {
                        *ppData = it->data;
                        (*ppData)->Acquire();
                    }
                }
                //check if we delivered data to all slots and can erase
                bool bUnserved = false;
                for (amf_int32 i = 0; i < m_iOutputCount; i++)
                {
                    if (!it->slots[i])
                    {
                        bUnserved = true;
                        break;
                    }
                }
                if (!bUnserved)
                {
                    m_Queue.erase(it);
                }
                break;
            }
        }
        return res;
    }
    virtual AMF_RESULT Drain()
    {
        AMFLock lock(&m_cs);
        m_bEof = true;
        return AMF_OK;
    }
protected:
    bool m_bCopyData;
    amf_int32 m_iOutputCount;
    amf_size m_QueueSize;
    std::list<Data> m_Queue;
    AMFCriticalSection m_cs;
    bool m_bEof;
};
typedef std::shared_ptr<Splitter> SplitterPtr;

#pragma warning(pop)
