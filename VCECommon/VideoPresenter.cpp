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
 * @file <VideoPresenter.cpp>
 *
 * @brief Source file for Video Presenter
 *
 *******************************************************************************
 */

#include "VideoPresenter.h"
#include "VideoPresenterDX11.h"
#include "VideoPresenterDX9.h"
#include "VideoPresenterOpenGL.h"

VideoPresenter::VideoPresenter(HWND hwnd, amf::AMFContext* pContext) :
    m_hwnd(hwnd), m_pContext(pContext), m_lastPTS(-1L), m_lastTime(0), m_state(
                    ModePlaying)
{
    // increase timer precision for the app
    amf_increase_timer_precision();
}
VideoPresenter::~VideoPresenter()
{
}

VideoPresenterPtr VideoPresenter::Create(amf::AMF_MEMORY_TYPE type, HWND hwnd,
                amf::AMFContext* pContext)
{
    AMF_RESULT res = AMF_OK;
    VideoPresenterPtr pPresenter;
    switch (type)
    {
    case amf::AMF_MEMORY_DX9:
        pPresenter = VideoPresenterPtr(new VideoPresenterDX9(hwnd, pContext));
        break;
    case amf::AMF_MEMORY_OPENGL:
        pPresenter = VideoPresenterPtr(new VideoPresenterOpenGL(hwnd, pContext));
        break;
    case amf::AMF_MEMORY_DX11:
        pPresenter = VideoPresenterPtr(new VideoPresenterDX11(hwnd, pContext));
        break;
    default:
        return NULL;
    }

    res = pPresenter->Init();

    if (res != AMF_OK)
    {
        pPresenter = NULL;
    }
    return pPresenter;
}

AMF_RESULT VideoPresenter::Init()
{
    m_lastPTS = -1L;
    m_lastTime = 0;
    return AMF_OK;
}

AMF_RESULT VideoPresenter::SubmitInput(amf::AMFData* pData)
{
    if (pData)
    {
        switch (m_state)
        {
        case ModeStep:
            m_state = ModePaused;
            return Present(amf::AMFSurfacePtr(pData));
        case ModePlaying:
            return Present(amf::AMFSurfacePtr(pData));
        case ModePaused:
            return AMF_INPUT_FULL;
        }
        return AMF_WRONG_STATE;
    }
    return AMF_OK;
}

AMF_RESULT VideoPresenter::CalcOutputRect(const AMFRect* pSrcRect,
                const AMFRect* pDstRect, AMFRect* pTargetRect)
{
    amf_double dDstRatio = pDstRect->Height() / (amf_double) pDstRect->Width();
    amf_double dSrcRatio = pSrcRect->Height() / (amf_double) pSrcRect->Width();

    // TODO : add aspect ratio from frame
    //dSrcRatio /= frameApectRatioFromDecoder;

    if (dDstRatio > dSrcRatio)
    { // empty areas on top and bottom
        pTargetRect->left = 0;
        pTargetRect->right = pDstRect->Width();
        LONG lViewHeight = amf_int(pDstRect->Width() * dSrcRatio);
        pTargetRect->top = (pDstRect->Height() - lViewHeight) / 2;
        pTargetRect->bottom = pTargetRect->top + lViewHeight;
    }
    else
    { // empty areas on left and right
        pTargetRect->bottom = pDstRect->Height();
        pTargetRect->top = 0;
        LONG lViewWidth = amf_int(pDstRect->Height() / dSrcRatio);
        pTargetRect->left = (pDstRect->Width() - lViewWidth) / 2;
        pTargetRect->right = pTargetRect->left + lViewWidth;
    }
    return AMF_OK;
}

AMF_RESULT VideoPresenter::WaitForPTS(amf_pts pts)
{
    amf_pts currTime = amf_high_precision_clock();
    if (m_lastPTS != -1L)
    {
        amf_pts duration = pts - m_lastPTS;
        amf_pts elapsed = currTime - m_lastTime;
        amf_pts delay = duration - elapsed;
        if (delay > AMF_SECOND / 1000) // ignore delays < 1 ms or negative
        {
            m_waiter.Wait(delay);
        }

    }
    m_lastTime = amf_high_precision_clock();
    m_lastPTS = pts;
    return AMF_OK;
}
