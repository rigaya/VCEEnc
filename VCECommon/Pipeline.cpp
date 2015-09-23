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
 * @file <Pipeline.cpp>
 *
 * @brief Source file for the Pipeline
 *
 *******************************************************************************
 */

#include "Pipeline.h"
#include "Thread.h"
#include "CmdLogger.h"
#include <sstream>

#pragma warning(disable:4355)

typedef AMFQueue<amf::AMFDataPtr> DataQueue;

class PipelineConnector;
class InputSlot;
class OutputSlot;
//-------------------------------------------------------------------------------------------------
class Slot: public AMFThread
{
public:
    bool m_bThread;
    PipelineConnector *m_pConnector;
    amf_int32 m_iThisSlot;
    AMFPreciseWaiter m_waiter;

    Slot(bool bThread, PipelineConnector *connector, amf_int32 thisSlot);
    virtual ~Slot()
    {
    }

    void Stop();
    virtual bool StopRequested();
};
//-------------------------------------------------------------------------------------------------
class InputSlot: public Slot
{
public:
    OutputSlot *m_pUpstreamOutputSlot;

    InputSlot(bool bThread, PipelineConnector *connector, amf_int32 thisSlot);
    virtual ~InputSlot()
    {
    }

    // pull 
    virtual void Run();
    AMF_RESULT Drain();
    AMF_RESULT SubmitInput(amf::AMFData* pData, amf_ulong ulTimeout, bool poll);
};
typedef std::shared_ptr<InputSlot> InputSlotPtr;
//-------------------------------------------------------------------------------------------------
class OutputSlot: public Slot
{
public:
    DataQueue m_dataQueue;
    InputSlot *m_pDownstreamInputSlot;

    OutputSlot(bool bThread, PipelineConnector *connector, amf_int32 thisSlot,
                    amf_int32 queueSize);
    virtual ~OutputSlot()
    {
    }

    virtual void Run();
    AMF_RESULT QueryOutput(amf::AMFData** ppData, amf_ulong ulTimeout);
    AMF_RESULT Poll(bool bEof);
};
typedef std::shared_ptr<OutputSlot> OutputSlotPtr;
//-------------------------------------------------------------------------------------------------
class PipelineConnector
{
    friend class Pipeline;
    friend class InputSlot;
    friend class OutputSlot;
protected:

public:
    PipelineConnector(Pipeline *host, PipelineElementPtr element);
    virtual ~PipelineConnector();

    void Start();
    void Stop();
    bool StopRequested()
    {
        return m_bStop;
    }

    void NotLast()
    {
        m_bLast = false;
    }
    void NotPush()
    {
        m_bPush = false;
    }

    void OnEof();

    // a-sync operations from threads
    AMF_RESULT Poll(amf_int32 slot);
    AMF_RESULT PollAll(bool bEof);

    void AddInputSlot(InputSlotPtr pSlot);
    void AddOutputSlot(OutputSlotPtr pSlot);

    amf_int64 GetSubmitFramesProcessed()
    {
        return m_iSubmitFramesProcessed;
    }
    amf_int64 GetPollFramesProcessed()
    {
        return m_iPollFramesProcessed;
    }

protected:
    Pipeline* m_pHost;
    PipelineElementPtr m_pElement;
    bool m_bPush;
    bool m_bLast;
    bool m_bStop;
    amf_int64 m_iSubmitFramesProcessed;
    amf_int64 m_iPollFramesProcessed;

    std::vector<InputSlotPtr> m_InputSlots;
    std::vector<OutputSlotPtr> m_OutputSlots;
};
//-------------------------------------------------------------------------------------------------
// class Pipeline
//-------------------------------------------------------------------------------------------------
Pipeline::Pipeline() :
    m_state(PipelineStateNotReady), m_startTime(0), m_stopTime(0)
{
}
Pipeline::~Pipeline()
{
    Stop();
}

AMF_RESULT Pipeline::Connect(PipelineElementPtr pElement, amf_int32 queueSize,
                bool syncronized /*= false*/)
{
    AMFLock lock(&m_cs);
    PipelineConnectorPtr upstreamConnector;
    if (!m_connectors.empty())
    {
        upstreamConnector = *m_connectors.rbegin();
        (*m_connectors.rbegin())->NotLast();
    }
    return Connect(pElement, 0, upstreamConnector == NULL ? NULL
                    : upstreamConnector->m_pElement, 0, queueSize, syncronized);
}
AMF_RESULT Pipeline::Connect(PipelineElementPtr pElement, amf_int32 slot,
                PipelineElementPtr upstreamElement, amf_int32 upstreamSlot,
                amf_int32 queueSize, bool syncronized/* = false*/)
{
    AMFLock lock(&m_cs);

    PipelineConnectorPtr upstreamConnector;
    for (ConnectorList::iterator it = m_connectors.begin(); it
                    != m_connectors.end(); it++)
    {
        if (it->get()->m_pElement.get() == upstreamElement.get())
        {
            upstreamConnector = *it;
            break;
        }
    }
    if (upstreamConnector != NULL)
    {
        upstreamConnector->NotLast();
    }

    PipelineConnectorPtr connector(new PipelineConnector(this, pElement));
    if (upstreamConnector != NULL)
    {
        if (!syncronized)
        {
            upstreamConnector->NotPush();
        }
        OutputSlotPtr pOutoutSlot = OutputSlotPtr(new OutputSlot(!syncronized
                        || m_connectors.size() == 1, upstreamConnector.get(),
                        upstreamSlot, queueSize));
        InputSlotPtr pInputSlot = InputSlotPtr(new InputSlot(!syncronized,
                        connector.get(), slot));
        pOutoutSlot->m_pDownstreamInputSlot = pInputSlot.get();
        pInputSlot->m_pUpstreamOutputSlot = pOutoutSlot.get();
        upstreamConnector->AddOutputSlot(pOutoutSlot);
        connector->AddInputSlot(pInputSlot);
    }

    m_connectors.push_back(connector);

    m_state = PipelineStateReady;
    return AMF_OK;
}

AMF_RESULT Pipeline::Start()
{
    AMFLock lock(&m_cs);
    if (m_state != PipelineStateReady)
    {
        return AMF_WRONG_STATE;
    }
    m_startTime = amf_high_precision_clock();

    for (ConnectorList::iterator it = m_connectors.begin(); it
                    != m_connectors.end(); it++)
    {
        (*it)->Start();
    }
    m_state = PipelineStateRunning;
    return AMF_OK;
}

AMF_RESULT Pipeline::Stop()
{
    for (ConnectorList::iterator it = m_connectors.begin(); it
                    != m_connectors.end(); it++)
    {
        (*it)->Stop();
    }
    AMFLock lock(&m_cs);
    m_connectors.clear();
    m_state = PipelineStateNotReady;
    return AMF_OK;
}
void Pipeline::OnEof()
{
    AMFLock lock(&m_cs);
    m_state = PipelineStateEof;
    m_stopTime = amf_high_precision_clock();
}

PipelineState Pipeline::GetState()
{
    AMFLock lock(&m_cs);
    return m_state;
}

void Pipeline::DisplayResult()
{
    AMFLock lock(&m_cs);
    if (m_connectors.size())
    {

        PipelineConnectorPtr last = *m_connectors.rbegin();

        amf_int64 frameCount = last->GetSubmitFramesProcessed();
        amf_int64 startTime = m_startTime;
        amf_int64 stopTime = m_stopTime;

        std::wstringstream messageStream;
        messageStream.precision(1);
        messageStream.setf(std::ios::fixed, std::ios::floatfield);

        // trace individual connectors
        for (ConnectorList::iterator it = m_connectors.begin(); it
                        != m_connectors.end(); it++)
        {
            std::wstring text = (*it)->m_pElement->GetDisplayResult();
            LOG_SUCCESS(text);
        }

        messageStream << L" Frames processed: " << frameCount;

        double encodeTime = double(stopTime - startTime) / 10000. / frameCount;
        messageStream << L" Frame process time: " << encodeTime << L"ms";

        messageStream << L" FPS: " << double(frameCount) / (double(stopTime
                        - startTime) / double(AMF_SECOND));

        LOG_SUCCESS(messageStream.str());
    }
}

double Pipeline::GetFPS()
{
    AMFLock lock(&m_cs);
    if (m_connectors.size())
    {
        PipelineConnectorPtr first = *m_connectors.begin();
        amf_int64 frameCount = first->GetPollFramesProcessed();
        amf_int64 startTime = m_startTime;
        amf_int64 stopTime = m_stopTime;
        return double(frameCount) / (double(stopTime - startTime) / double(
                        AMF_SECOND));
    }
    return 0;
}
double Pipeline::GetProcessingTime()
{
    return double(m_stopTime - m_startTime) / 10000.;
}

amf_int64 Pipeline::GetNumberOfProcessedFrames()
{
    PipelineConnectorPtr last = *m_connectors.rbegin();
    return last->GetSubmitFramesProcessed();
}
//-------------------------------------------------------------------------------------------------
// class Slot
//-------------------------------------------------------------------------------------------------
Slot::Slot(bool bThread, PipelineConnector *connector, amf_int32 thisSlot) :
    m_bThread(bThread), m_pConnector(connector), m_iThisSlot(thisSlot)
{
}
void Slot::Stop()
{
    RequestStop();
    WaitForStop();
}
bool Slot::StopRequested()
{
    return m_bThread ? AMFThread::StopRequested()
                    : m_pConnector->StopRequested();
}

//-------------------------------------------------------------------------------------------------
// class InputSlot
//-------------------------------------------------------------------------------------------------
InputSlot::InputSlot(bool bThread, PipelineConnector *connector,
                amf_int32 thisSlot) :
    Slot(bThread, connector, thisSlot), m_pUpstreamOutputSlot(NULL)
{
}
// pull 
void InputSlot::Run()
{
    AMF_RESULT res = AMF_OK;
    while (!StopRequested())
    {
        if (res != AMF_EOF) // after EOF thread waits for stop
        {
            amf::AMFDataPtr data;

            res = m_pUpstreamOutputSlot->QueryOutput(&data, 50);
            if (res == AMF_OK || res == AMF_EOF)
            {
                res = SubmitInput(data, 50, false);
            }
            else
            {
                m_waiter.Wait(3);
            }
        }
        else
        {
            m_waiter.Wait(5);
        }

    }
}

AMF_RESULT InputSlot::Drain()
{
    AMF_RESULT res = AMF_OK;
    while (!StopRequested())
    {
        res = m_pConnector->m_pElement->Drain();
        if (res != AMF_INPUT_FULL)
        {

            break;
        }
        // LOG_INFO(L"m_pElement->Drain() returned AMF_INPUT_FULL");
        if (m_bThread)
        {
            m_waiter.Wait(5);
        }
        else
        {
            res = m_pConnector->PollAll(false);
        }
    }
    return AMF_EOF;
}

AMF_RESULT InputSlot::SubmitInput(amf::AMFData* pData, amf_ulong ulTimeout,
                bool poll)
{
    AMF_RESULT res = AMF_OK;
    if (pData == NULL)
    {
        res = Drain();

    }
    else
    {

        //push input
        while (!StopRequested())
        {
            res = m_pConnector->m_pElement->SubmitInput(pData, m_iThisSlot);
            if (res == AMF_INPUT_FULL || res == AMF_DECODER_NO_FREE_SURFACES)
            {
                if (poll)
                {
                    res = m_pConnector->PollAll(false); // no poll thread: poll right here
                    if(res != AMF_OK && res != AMF_REPEAT && res != AMF_RESOLUTION_UPDATED)
                    {
                        break;
                    }
                }
                else
                {
                    m_waiter.Wait(3); // wait till Poll thread clears input
                }
            }
            else if(res == AMF_REPEAT)
            {
                pData = NULL; // need to submit one more time
            }
            else
            {
                if(res == AMF_OK || res == AMF_RESOLUTION_UPDATED)
                {
                    if (pData != NULL)
                    {
                        m_pConnector->m_iSubmitFramesProcessed++;
                    }
                }
                else if (res != AMF_EOF)
                {
                    LOG_ERROR(L"SubmitInput() returned error: "
                                    << amf::AMFGetResultText(res));
                }

                break;
            }
        }
    }
    if (res != AMF_OK && res != AMF_EOF)
    {
        return res;
    }
    // poll output
    AMF_RESULT resPoll = m_pConnector->PollAll(res == AMF_EOF);
    if (pData == NULL && m_pConnector->m_bLast)
    {
        m_pConnector->OnEof();
    }

    return res;
}

//-------------------------------------------------------------------------------------------------
// class OutputSlot
//-------------------------------------------------------------------------------------------------
OutputSlot::OutputSlot(bool bThread, PipelineConnector *connector,
                amf_int32 thisSlot, amf_int32 queueSize) :
    Slot(bThread, connector, thisSlot), m_pDownstreamInputSlot(NULL)
{
    m_dataQueue.SetQueueSize(queueSize);
}

void OutputSlot::Run()
{
    AMF_RESULT res = AMF_OK;
    while (!StopRequested())
    {
        if (res != AMF_EOF) // after EOF thread waits for stop
        {
            res = Poll(false);
            if (res != AMF_OK) // 
            {
                m_waiter.Wait(3);
            }
        }
        else
        {
            m_waiter.Wait(5);
        }
    }
}

AMF_RESULT OutputSlot::QueryOutput(amf::AMFData** ppData, amf_ulong ulTimeout)
{
    AMF_RESULT res = AMF_OK;
    if (!m_bThread)
    {
        res = m_pConnector->m_pElement->QueryOutput(ppData, m_iThisSlot);

        if (res == AMF_EOF) // EOF is sent as NULL data to the next element
        {
            m_pConnector->OnEof();
        }

        return res;
    }
    amf::AMFDataPtr data;
    amf_ulong id = 0;
    if (m_dataQueue.Get(id, data, ulTimeout))
    {
        *ppData = data.Detach();
        if (*ppData == NULL)
        {
            m_pConnector->OnEof();
            return AMF_EOF;
        }
        return AMF_OK;
    }
    return AMF_REPEAT;
}
AMF_RESULT OutputSlot::Poll(bool bEof)
{
    AMF_RESULT res = AMF_OK;

    while (!StopRequested())
    {
        amf::AMFDataPtr data;

        res = m_pConnector->m_pElement->QueryOutput(&data, m_iThisSlot);
        if (data != NULL || res == AMF_EOF /*|| bEof*/) // EOF is sent as NULL data to the next element
        {
            if (res == AMF_EOF)
            {
                bEof = true;
            }
            if (data != NULL)
            {
                m_pConnector->m_iPollFramesProcessed++; // EOF is not included
            }
            // have data - send it

            if (!m_bThread || m_pConnector->m_bPush) // push mode
            {
                res = m_pDownstreamInputSlot->SubmitInput(data, 50, true);
            }
            else // pull mode
            {
                while (!StopRequested())
                {
                    amf_ulong id = 0;
                    if (m_dataQueue.Add(id, data, 50))
                    {
                        break;
                    }
                }
            }
        }
        else if (res == AMF_OK)
        {
            res = AMF_REPEAT;
        }
        if (data == NULL)
        {
            if (!bEof || res == AMF_EOF)
            {
                break; // nothing in component - exit
            }
            else
            {
                m_waiter.Wait(3);
            }
        }
    }
    if (bEof)
    {
        m_pConnector->OnEof();
    }
    return res;
}
//-------------------------------------------------------------------------------------------------
// class PipelineConnector
//-------------------------------------------------------------------------------------------------
PipelineConnector::PipelineConnector(Pipeline *host, PipelineElementPtr element) :
    m_pHost(host), m_pElement(element), m_bPush(true), m_bLast(true), m_bStop(
                    false), m_iSubmitFramesProcessed(0),
                    m_iPollFramesProcessed(0)
{
}
PipelineConnector::~PipelineConnector()
{
    Stop();
}
void PipelineConnector::Start()
{
    m_bStop = false;

    for (amf_size i = 0; i < m_InputSlots.size(); i++)
    {
        InputSlotPtr pSlot = m_InputSlots[i];
        if (m_pElement->GetInputSlotCount() == 0
                        || pSlot->m_pUpstreamOutputSlot == NULL)
        {
            pSlot->m_bThread = false; // do not start first submit thread 
        }
        if (pSlot->m_bThread)
        {
            pSlot->Start();
        }
    }
    for (amf_size i = 0; i < m_OutputSlots.size(); i++)
    {
        OutputSlotPtr pSlot = m_OutputSlots[i];
        if (m_bLast || m_pElement->GetOutputSlotCount() == 0
                        || pSlot->m_pDownstreamInputSlot == NULL)
        {
            pSlot->m_bThread = false; // do not start last polling thread 
        }
        if (pSlot->m_bThread)
        {
            pSlot->Start();
        }
    }

}
void PipelineConnector::Stop()
{
    m_bStop = true; // must be atomic but will work

    for (amf_size i = 0; i < m_InputSlots.size(); i++)
    {
        m_InputSlots[i]->Stop();
    }
    for (amf_size i = 0; i < m_OutputSlots.size(); i++)
    {
        m_OutputSlots[i]->Stop();
    }
    m_pElement->Drain();

}

void PipelineConnector::OnEof()
{
    if (m_bLast)
    {
        m_pHost->OnEof();
    }
}

// a-sync operations from threads
AMF_RESULT PipelineConnector::Poll(amf_int32 slot)
{
    return m_OutputSlots[slot]->Poll(false);
}
AMF_RESULT PipelineConnector::PollAll(bool bEof)
{
    AMF_RESULT res = AMF_OK;
    for (amf_size i = 0; i < m_OutputSlots.size(); i++)
    {
        if (!m_OutputSlots[i]->m_bThread && !m_bLast)
        {
            res = m_OutputSlots[i]->Poll(bEof);
        }
        if (res == AMF_EOF)
        {
            bEof = true;
        }
        else if (res != AMF_OK)
        {
            break;
        }
    }
    if (bEof)
    {
        OnEof();
    }
    return bEof ? AMF_EOF : res;
}
void PipelineConnector::AddInputSlot(InputSlotPtr pSlot)
{
    m_InputSlots.push_back(pSlot);
}
void PipelineConnector::AddOutputSlot(OutputSlotPtr pSlot)
{
    m_OutputSlots.push_back(pSlot);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
