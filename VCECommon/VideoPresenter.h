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
 * @file <VideoPresenter.h>
 *
 * @brief Header file for VideoPresenter
 *
 *******************************************************************************
 */

#pragma once

#include "AMFPlatform.h"
#include "PipelineElement.h"
#include "Context.h"

/*
 struct AMFRect : public RECT
 {
 AMFRect(LONG l, LONG t, LONG r, LONG b)
 {
 left = l;
 top = t;
 right = r;
 bottom = b;
 }
 AMFRect()
 {
 left = 0;
 top = 0;
 right = 0;
 bottom = 0;
 }
 LONG Width() const { return right - left; }
 LONG Height() const { return bottom - top; }
 };
 */
class VideoPresenter;
typedef std::shared_ptr<VideoPresenter> VideoPresenterPtr;

class VideoPresenter: public PipelineElement
{
    enum Mode
    {
        ModePlaying, ModeStep, ModePaused
    };

public:
    virtual ~VideoPresenter();

    virtual AMF_RESULT SubmitInput(amf::AMFData* pData);

    virtual AMF_RESULT Present(amf::AMFSurface* pSurface) = 0;
    virtual amf::AMF_MEMORY_TYPE GetMemoryType() = 0;
    virtual amf::AMF_SURFACE_FORMAT GetInputFormat() = 0;

    virtual AMF_RESULT Init();
    virtual AMF_RESULT Terminate() = 0;

    AMF_RESULT Resume()
    {
        m_state = ModePlaying;
        return AMF_OK;
    }
    AMF_RESULT Pause()
    {
        m_state = ModePaused;
        return AMF_OK;
    }
    AMF_RESULT Step()
    {
        m_state = ModeStep;
        return AMF_OK;
    }

public:
    static VideoPresenterPtr Create(amf::AMF_MEMORY_TYPE type, HWND hwnd,
                    amf::AMFContext* pContext);

protected:
    VideoPresenter(HWND hwnd, amf::AMFContext* pContext);

    AMF_RESULT CalcOutputRect(const AMFRect* pSrcRect, const AMFRect* pDstRect,
                    AMFRect* pTargetRect);
    AMF_RESULT WaitForPTS(amf_pts pts);

    HWND m_hwnd;
    amf::AMFContext* m_pContext;
    amf_pts m_lastPTS;
    amf_pts m_lastTime;
    AMFPreciseWaiter m_waiter;
private:
    Mode m_state;
};