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
 * @file <Pipeline.h>
 *
 * @brief Header file for Pipeline
 *
 *******************************************************************************
 */

#pragma once

#include "PipelineElement.h"
#include <vector>

enum PipelineState
{
    PipelineStateNotReady,
    PipelineStateReady,
    PipelineStateRunning,
    PipelineStateEof,
    PipelineStateError,
};

class Pipeline
{
    friend class PipelineConnector;
    typedef std::shared_ptr<PipelineConnector> PipelineConnectorPtr;
public:
    Pipeline();
    virtual ~Pipeline();

    AMF_RESULT Connect(PipelineElementPtr pElement, amf_int32 queueSize,
                    bool syncronized = false);
    AMF_RESULT Connect(PipelineElementPtr pElement, amf_int32 slot,
                    PipelineElementPtr upstreamElement, amf_int32 upstreamSlot,
                    amf_int32 queueSize, bool syncronized = false);

    virtual AMF_RESULT Start();
    virtual AMF_RESULT Stop();

    PipelineState GetState();

    virtual void DisplayResult();
    double GetFPS();
    double GetProcessingTime();
    amf_int64 GetNumberOfProcessedFrames();

private:
    void OnEof();

    amf_int64 m_startTime;
    amf_int64 m_stopTime;

    typedef std::vector<PipelineConnectorPtr> ConnectorList;
    ConnectorList m_connectors;
    PipelineState m_state;
    AMFCriticalSection m_cs;
};