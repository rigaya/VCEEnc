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
 * @file <EncoderPipeline.h>
 *
 * @brief Header file for Encoder Pipeline
 *
 *******************************************************************************
 */

#pragma once

#include "VideoEncoderHW_AVC.h"
#include "EncoderStatistic.h"
#include "FrameProvider.h"

class EncoderPipeline
{
public:

    enum PipelineStatus
    {
        PS_UnInitialized = 0, PS_Initialized, PS_Running, PS_Eof,
    };

    EncoderPipeline(ParametersManagerPtr pParams);
    virtual ~EncoderPipeline();

    AMF_RESULT Init(int threadID = -1);
    AMF_RESULT Run();
    AMF_RESULT Terminate();
    PipelineStatus GetStatus()
    {
        return m_eStatus;
    }
    double GetFPS();
    void LogStat();

protected:
    void PushProperties(ParametersManager::ParamType ptype,
                    amf::AMFPropertyStorage *storage);
    void SetStatus(PipelineStatus status)
    {
        m_eStatus = status;
    }

    class RenderThread: public AMFThread
    {
    protected:
        EncoderPipeline *m_pHost;
    public:
        RenderThread(EncoderPipeline *host) :
            m_pHost(host)
        {
        }
        virtual void Run();
    };
    class PollingThread: public AMFThread
    {
    protected:
        EncoderPipeline *m_pHost;
    public:
        PollingThread(EncoderPipeline *host) :
            m_pHost(host)
        {
        }
        virtual void Run();
    };

    FrameProviderPtr m_pFrameProvider;
    ParametersManagerPtr m_params;
    amf::AMFContextPtr m_context;
    amf::AMFComponentPtr m_encoder;
    AMFDataStreamPtr m_stream;

    RenderThread m_RenderThread;
    PollingThread m_PollingThread;
    amf_int64 m_framesRequested;
    amf_int64 m_framesSubmitted;
    amf_int64 m_framesEncoded;
    amf_int64 m_frameParameterFreq;
    amf_int64 m_dynamicParameterFreq;
    PipelineStatus m_eStatus;

    EncoderStat m_stat;
};