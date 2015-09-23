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
 * @file <VideoPresenterDX9.cpp>
 *
 * @brief Source file for DX9 Video Presenter
 *
 *******************************************************************************
 */

#include "VideoPresenterDX9.h"

VideoPresenterDX9::VideoPresenterDX9(HWND hwnd, amf::AMFContext* pContext) :
    VideoPresenter(hwnd, pContext)
{

}

VideoPresenterDX9::~VideoPresenterDX9()
{
    Terminate();
}

AMF_RESULT VideoPresenterDX9::Init()
{
    AMF_RESULT err = AMF_OK;

    VideoPresenter::Init();

    m_pDevice = static_cast<IDirect3DDevice9Ex*> (m_pContext->GetDX9Device());
    if (m_pDevice == NULL)
    {
        err = AMF_NO_DEVICE;
    }

    if (err == AMF_OK)
    {
        err = CreatePresentationSwapChain();
    }

    return err;
}

AMF_RESULT VideoPresenterDX9::Terminate()
{
    m_pContext = NULL;
    m_hwnd = NULL;

    m_pDevice = NULL;
    m_pSwapChain = NULL;
    return AMF_OK;
}

AMF_RESULT VideoPresenterDX9::CreatePresentationSwapChain()
{
    AMF_RESULT err = AMF_OK;
    HRESULT hr = S_OK;

    // setup params

    D3DPRESENT_PARAMETERS pp;
    ZeroMemory(&pp, sizeof(pp));
    pp.BackBufferWidth = 0; // will get from window
    pp.BackBufferHeight = 0; // will get from window
    pp.BackBufferCount = 4; // 4 buffers to flip - 4 buffers get the best performance
    pp.Windowed = TRUE;
    //    pp.SwapEffect = D3DSWAPEFFECT_FLIP;
    pp.SwapEffect = D3DSWAPEFFECT_FLIPEX;
    pp.BackBufferFormat = D3DFMT_A8R8G8B8;
    pp.hDeviceWindow = (HWND) m_hwnd;
    pp.Flags = D3DPRESENTFLAG_VIDEO;
    pp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    D3DDEVICE_CREATION_PARAMETERS params;
    hr = m_pDevice->GetCreationParameters(&params);

    if (FAILED(hr))
    {
        return AMF_DIRECTX_FAILED;
    }

    IDirect3DSwapChain9Ptr pSwapChain;

    hr = m_pDevice->CreateAdditionalSwapChain(&pp, &pSwapChain);
    if (FAILED(hr))
    {
        return AMF_DIRECTX_FAILED;
    }
    m_pSwapChain = pSwapChain;

    m_pDevice->SetGPUThreadPriority(7);

    return err;
}

AMF_RESULT VideoPresenterDX9::Present(amf::AMFSurface* pSurface)
{
    AMF_RESULT err = AMF_OK;

    if (m_pDevice == NULL)
    {
        err = AMF_NO_DEVICE;
    }

    if (err == AMF_OK)
    {
        err = pSurface->GetFormat() == GetInputFormat() ? AMF_OK
                        : AMF_INVALID_FORMAT;
    }

    if (err == AMF_OK)
    {
        err = pSurface->Convert(GetMemoryType());
    }

    D3DPRESENT_PARAMETERS presentationParameters;
    if (err == AMF_OK)
    {
        err = SUCCEEDED(m_pSwapChain->GetPresentParameters(
                        &presentationParameters)) ? AMF_OK : AMF_DIRECTX_FAILED;
    }

    AMFRect rectClient;
    if (err == AMF_OK)
    {
        RECT tmpRectClient = { 0, 0, 500, 500 };
        if (!GetClientRect((HWND) m_hwnd, &tmpRectClient))
        {
            err = AMF_FAIL;
        }
        rectClient = AMFConstructRect(tmpRectClient.left, tmpRectClient.top,
                        tmpRectClient.right, tmpRectClient.bottom);
    }

    if (err == AMF_OK)
    {
        if ((presentationParameters.BackBufferHeight != rectClient.bottom
                        - rectClient.top)
                        || (presentationParameters.BackBufferWidth
                                        != rectClient.right - rectClient.left))
        {
            m_pSwapChain = NULL;
            err = CreatePresentationSwapChain();
        }
    }

    amf::AMFPlane* pPlane = pSurface->GetPlane(amf::AMF_PLANE_PACKED);
    IDirect3DSurface9Ptr pSrcDxSurface =
                    (IDirect3DSurface9*) pPlane->GetNative();
    if (pSrcDxSurface == NULL)
    {
        err = AMF_INVALID_POINTER;
    }

    IDirect3DSurface9Ptr pDestDxSurface;
    if (err == AMF_OK)
    {
        err = SUCCEEDED(m_pSwapChain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO,
                        &pDestDxSurface)) ? AMF_OK : AMF_DIRECTX_FAILED;
    }
    AMFRect srcRect = { pPlane->GetOffsetX(), pPlane->GetOffsetY(),
                        pPlane->GetOffsetX() + pPlane->GetWidth(),
                        pPlane->GetOffsetY() + pPlane->GetHeight() };
    AMFRect outputRect;

    if (err == AMF_OK)
    {
        err = CalcOutputRect(&srcRect, &rectClient, &outputRect);
    }
    if (err == AMF_OK)
    {
        m_pDevice->SetRenderTarget(0, pDestDxSurface);
        //in case of ROI we should specify SrcRect

        WaitForPTS(pSurface->GetPts());

        err = BitBlt(pSrcDxSurface, &srcRect, pDestDxSurface, &outputRect);
    }
    for (int i = 0; i < 100; i++)
    {
        HRESULT hr = m_pSwapChain->Present(NULL, NULL, NULL, NULL, 0);
        if (SUCCEEDED(hr))
        {
            break;
        }
        if (hr != D3DERR_WASSTILLDRAWING)
        {
            return AMF_DIRECTX_FAILED;
        }
        amf_sleep(1);
    }
    return err;
}

AMF_RESULT VideoPresenterDX9::BitBlt(IDirect3DSurface9* pSrcSurface,
                AMFRect* pSrcRect, IDirect3DSurface9* pDstSurface,
                AMFRect* pDstRect)
{
    RECT srcRect = { pSrcRect->left, pSrcRect->top, pSrcRect->right,
                     pSrcRect->bottom };
    RECT dstRect = { pDstRect->left, pDstRect->top, pDstRect->right,
                     pDstRect->bottom };
    return SUCCEEDED(m_pDevice->StretchRect(pSrcSurface, &srcRect, pDstSurface,
                    &dstRect, D3DTEXF_LINEAR)) ? AMF_OK : AMF_DIRECTX_FAILED;
}

