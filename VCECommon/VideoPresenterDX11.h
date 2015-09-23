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
 * @file <VideoPresenterDX11.h>
 *
 * @brief Header file for VideoPresenterDX11
 *
 *******************************************************************************
 */

#pragma once

#include "VideoPresenter.h"

#include <atlbase.h>
#include <d3d11.h>
#include <DXGI1_2.h>

#include <DirectXMath.h>
using namespace DirectX;

struct SimpleVertex
{
    XMFLOAT3 position;
    XMFLOAT2 texture;
};
struct CBNeverChanges
{
    XMMATRIX mView;
};

class VideoPresenterDX11: public VideoPresenter
{
public:
    VideoPresenterDX11(HWND hwnd, amf::AMFContext* pContext);

    virtual ~VideoPresenterDX11();

    virtual AMF_RESULT Present(amf::AMFSurface* pSurface);

    virtual amf::AMF_MEMORY_TYPE GetMemoryType()
    {
        return amf::AMF_MEMORY_DX11;
    }
    virtual amf::AMF_SURFACE_FORMAT GetInputFormat()
    {
        return amf::AMF_SURFACE_BGRA;
    }

    virtual AMF_RESULT Init();
    virtual AMF_RESULT Terminate();

private:
    AMF_RESULT BitBlt(amf::AMF_FRAME_TYPE eFrameType,
                    ID3D11Texture2D* pSrcSurface, AMFRect* pSrcRect,
                    ID3D11Texture2D* pDstSurface, AMFRect* pDstRect);
    AMF_RESULT BitBltRender(amf::AMF_FRAME_TYPE eFrameType,
                    ID3D11Texture2D* pSrcSurface, AMFRect* pSrcRect,
                    ID3D11Texture2D* pDstSurface, AMFRect* pDstRect);
    AMF_RESULT BitBltCopy(ID3D11Texture2D* pSrcSurface, AMFRect* pSrcRect,
                    ID3D11Texture2D* pDstSurface, AMFRect* pDstRect);
    AMF_RESULT CompileShaders();
    AMF_RESULT PrepareStates();
    AMF_RESULT CheckForResize(bool bForce);
    AMF_RESULT UpdateVertices(AMFRect* pSrcRect, ID3D11Texture2D * textureSrc);
    AMF_RESULT DrawFrame(ID3D11Texture2D* pSrcSurface, bool bLeft);
    AMF_RESULT CopySurface(amf::AMF_FRAME_TYPE eFrameType,
                    ID3D11Texture2D* pSrcSurface, AMFRect* pSrcRect);

    AMF_RESULT CreatePresentationSwapChain();

    CComPtr<ID3D11Device> m_pDevice;
    CComQIPtr<IDXGISwapChain> m_pSwapChain;
    CComPtr<IDXGISwapChain1> m_pSwapChain1;
    bool m_stereo;
    CComPtr<ID3D11Texture2D> m_pCopyTexture_L;
    CComPtr<ID3D11Texture2D> m_pCopyTexture_R;

    CComPtr<ID3D11Buffer> m_pVertexBuffer;
    CComPtr<ID3D11Buffer> m_pCBChangesOnResize;
    CComPtr<ID3D11VertexShader> m_pVertexShader;
    CComPtr<ID3D11PixelShader> m_pPixelShader;
    CComPtr<ID3D11InputLayout> m_pVertexLayout;
    CComPtr<ID3D11SamplerState> m_pSampler;
    CComPtr<ID3D11DepthStencilState> m_pDepthStencilState;
    CComPtr<ID3D11RasterizerState> m_pRasterizerState;
    CComPtr<ID3D11BlendState> m_pBlendState;
    D3D11_VIEWPORT m_CurrentViewport;

    CComPtr<ID3D11RenderTargetView> m_pRenderTargetView_L;
    CComPtr<ID3D11RenderTargetView> m_pRenderTargetView_R;

    float m_fScale;
    float m_fPixelAspectRatio;
    float m_fOffsetX;
    float m_fOffsetY;

    AMFRect m_sourceRect;
    amf_int m_dstWidth;
    amf_int m_dstHeight;
};