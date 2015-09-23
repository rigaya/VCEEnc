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
 * @file <VideoPresenterDX11.cpp>
 *
 * @brief Source file for DX11 Video Presenter
 *
 *******************************************************************************
 */

#include "VideoPresenterDX11.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

const char
                DX11_FullScreenQuad[] =
                                "//--------------------------------------------------------------------------------------\n"
                                    "// Constant Buffer Variables                                                            \n"
                                    "//--------------------------------------------------------------------------------------\n"
                                    "Texture2D txDiffuse : register( t0 );                                                   \n"
                                    "SamplerState samplerState : register( s0 );                                             \n"
                                    "//--------------------------------------------------------------------------------------\n"
                                    "cbuffer cbNeverChanges : register( b0 )                                                 \n"
                                    "{                                                                                       \n"
                                    "    matrix View;                                                                        \n"
                                    "};                                                                                      \n"
                                    "//--------------------------------------------------------------------------------------\n"
                                    "struct VS_INPUT                                                                         \n"
                                    "{                                                                                       \n"
                                    "    float4 Pos : POSITION;                                                              \n"
                                    "    float2 Tex : TEXCOORD0;                                                             \n"
                                    "};                                                                                      \n"
                                    "//--------------------------------------------------------------------------------------\n"
                                    "struct PS_INPUT                                                                         \n"
                                    "{                                                                                       \n"
                                    "    float4 Pos : SV_POSITION;                                                           \n"
                                    "    float2 Tex : TEXCOORD0;                                                             \n"
                                    "};                                                                                      \n"
                                    "//--------------------------------------------------------------------------------------\n"
                                    "// Vertex Shader                                                                        \n"
                                    "//--------------------------------------------------------------------------------------\n"
                                    "PS_INPUT VS( VS_INPUT input )                                                           \n"
                                    "{                                                                                       \n"
                                    "    PS_INPUT output = (PS_INPUT)0;                                                      \n"
                                    "    output.Pos = mul( input.Pos, View );                                                \n"
                                    "//    output.Pos = mul( output.Pos, Projection );                                       \n"
                                    "    output.Tex = input.Tex;                                                             \n"
                                    "                                                                                        \n"
                                    "    return output;                                                                      \n"
                                    "}                                                                                       \n"
                                    "//--------------------------------------------------------------------------------------\n"
                                    "// Pixel Shader passing texture color                                                   \n"
                                    "//--------------------------------------------------------------------------------------\n"
                                    "float4 PS( PS_INPUT input) : SV_Target                                                  \n"
                                    "{                                                                                       \n"
                                    "    return txDiffuse.Sample( samplerState, input.Tex );                                 \n"
                                    "}                                                                                       \n";

VideoPresenterDX11::VideoPresenterDX11(HWND hwnd, amf::AMFContext* pContext) :
    VideoPresenter(hwnd, pContext), m_stereo(false), m_fScale(1.0f),
                    m_fPixelAspectRatio(1.0f), m_fOffsetX(0.0f), m_fOffsetY(
                                    0.0f), m_dstWidth(0), m_dstHeight(0)
{
}

VideoPresenterDX11::~VideoPresenterDX11()
{
    Terminate();
}

AMF_RESULT VideoPresenterDX11::Present(amf::AMFSurface* pSurface)
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

    amf::AMFPlane* pPlane = pSurface->GetPlane(amf::AMF_PLANE_PACKED);

    bool bForce = m_sourceRect.left != pPlane->GetOffsetX() || m_sourceRect.top
                    != pPlane->GetOffsetY() || m_sourceRect.right
                    != pPlane->GetWidth() || m_sourceRect.bottom
                    != pPlane->GetHeight();

    CheckForResize(bForce);

    CComPtr < ID3D11Texture2D > pSrcDxSurface
                    = (ID3D11Texture2D*) pPlane->GetNative();
    if (pSrcDxSurface == NULL)
    {
        err = AMF_INVALID_POINTER;
    }

    CComPtr < ID3D11Texture2D > pDestDxSurface;
    if (err == AMF_OK)
    {
        err = SUCCEEDED(m_pSwapChain->GetBuffer(0, __uuidof(pDestDxSurface),
                        reinterpret_cast<void**> (&pDestDxSurface))) ? AMF_OK
                        : AMF_DIRECTX_FAILED;
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
    AMFRect srcRect = { pPlane->GetOffsetX(), pPlane->GetOffsetY(),
                        pPlane->GetOffsetX() + pPlane->GetWidth(),
                        pPlane->GetOffsetY() + pPlane->GetHeight() };
    AMFRect outputRect;

    if (err == AMF_OK)
    {
        err = CalcOutputRect(&srcRect, &rectClient, &outputRect);
    }
    WaitForPTS(pSurface->GetPts());
    if (err == AMF_OK)
    {
        //in case of ROI we should specify SrcRect
        err = BitBlt(pSurface->GetFrameType(), pSrcDxSurface, &srcRect,
                        pDestDxSurface, &outputRect);
    }
    if (err == AMF_OK)
    {
        for (int i = 0; i < 100; i++)
        {
            HRESULT hr = m_pSwapChain->Present(0, DXGI_PRESENT_DO_NOT_WAIT);
            if (hr != DXGI_ERROR_WAS_STILL_DRAWING)
            {
                //ASSERT_RETURN_IF_HR_FAILED(hr,AMF_DIRECTX_FAILED,L"Present() - Present() failed");
                break;
            }
            amf_sleep(1);
        }
    }

    return err;
}

AMF_RESULT VideoPresenterDX11::BitBlt(amf::AMF_FRAME_TYPE eFrameType,
                ID3D11Texture2D* pSrcSurface, AMFRect* pSrcRect,
                ID3D11Texture2D* pDstSurface, AMFRect* pDstRect)
{
    //return BitBltCopy(pSrcSurface, pSrcRect, pDstSurface, pDstRect);
    return BitBltRender(eFrameType, pSrcSurface, pSrcRect, pDstSurface,
                    pDstRect);
}

AMF_RESULT VideoPresenterDX11::BitBltCopy(ID3D11Texture2D* pSrcSurface,
                AMFRect* pSrcRect, ID3D11Texture2D* pDstSurface,
                AMFRect* pDstRect)
{
    AMF_RESULT err = AMF_OK;

    CComPtr < ID3D11DeviceContext > spContext;
    m_pDevice->GetImmediateContext(&spContext);

    D3D11_TEXTURE2D_DESC srcDesc;
    pSrcSurface->GetDesc(&srcDesc);
    D3D11_TEXTURE2D_DESC dstDesc;
    pDstSurface->GetDesc(&dstDesc);

    D3D11_BOX box;
    box.left = pSrcRect->left;
    box.top = pSrcRect->top;
    box.right = pSrcRect->right;
    box.bottom = pSrcRect->bottom;
    box.front = 0;
    box.back = 1;

    spContext->CopySubresourceRegion(pDstSurface, 0, pDstRect->left,
                    pDstRect->top, 0, pSrcSurface, 0, &box);
    return err;
}

AMF_RESULT VideoPresenterDX11::BitBltRender(amf::AMF_FRAME_TYPE eFrameType,
                ID3D11Texture2D* pSrcSurface, AMFRect* pSrcRect,
                ID3D11Texture2D* pDstSurface, AMFRect* pDstRect)
{
    AMF_RESULT err = AMF_OK;
    HRESULT hr = S_OK;

    CComPtr < ID3D11DeviceContext > spContext;
    m_pDevice->GetImmediateContext(&spContext);

    D3D11_TEXTURE2D_DESC srcDesc;
    pSrcSurface->GetDesc(&srcDesc);

    if ((srcDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
                    != D3D11_BIND_SHADER_RESOURCE)
    {
        CopySurface(eFrameType, pSrcSurface, pSrcRect);
        AMFRect newSourceRect;

        newSourceRect.left = 0;
        newSourceRect.top = 0;
        newSourceRect.right = pSrcRect->Width();
        newSourceRect.bottom = pSrcRect->Height();

        UpdateVertices(&newSourceRect, m_pCopyTexture_L);
    }
    else
    {
        UpdateVertices(pSrcRect, pSrcSurface);
    }

    // setup all states

    spContext->OMSetDepthStencilState(m_pDepthStencilState, 1);
    spContext->RSSetState(m_pRasterizerState);
    spContext->IASetInputLayout(m_pVertexLayout);
    spContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    spContext->RSSetViewports(1, &m_CurrentViewport);
    float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    spContext->OMSetBlendState(m_pBlendState, blendFactor, 0xFFFFFFFF);

    // Set vertex buffer

    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    spContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer.p, &stride, &offset);

    // setup shaders
    spContext->VSSetShader(m_pVertexShader, NULL, 0);
    spContext->VSSetConstantBuffers(0, 1, &m_pCBChangesOnResize.p);
    spContext->PSSetShader(m_pPixelShader, NULL, 0);
    spContext->PSSetConstantBuffers(0, 1, &m_pCBChangesOnResize.p);
    spContext->PSSetSamplers(0, 1, &m_pSampler.p);

    // render left
    if ((eFrameType & amf::AMF_FRAME_LEFT_FLAG) == amf::AMF_FRAME_LEFT_FLAG
                    || (eFrameType & amf::AMF_FRAME_STEREO_FLAG) == 0)
    {
        DrawFrame(m_pCopyTexture_L != NULL ? m_pCopyTexture_L : pSrcSurface,
                        true);
    }

    // render right
    if ((eFrameType & amf::AMF_FRAME_RIGHT_FLAG) == amf::AMF_FRAME_RIGHT_FLAG)
    {
        DrawFrame(m_pCopyTexture_R != NULL ? m_pCopyTexture_R : pSrcSurface,
                        false);
    }
    return AMF_OK;
}

AMF_RESULT VideoPresenterDX11::CopySurface(amf::AMF_FRAME_TYPE eFrameType,
                ID3D11Texture2D* pSrcSurface, AMFRect* pSrcRect)
{
    HRESULT hr = S_OK;
    D3D11_TEXTURE2D_DESC srcDesc;
    pSrcSurface->GetDesc(&srcDesc);

    CComPtr < ID3D11DeviceContext > spContext;
    m_pDevice->GetImmediateContext(&spContext);

    if (m_pCopyTexture_L == NULL)
    {
        D3D11_TEXTURE2D_DESC Desc = { 0 };
        Desc.ArraySize = 1;
        Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        Desc.Usage = D3D11_USAGE_DEFAULT;
        Desc.Format = srcDesc.Format;
        Desc.Width = pSrcRect->Width();
        Desc.Height = pSrcRect->Height();
        Desc.MipLevels = 1;
        Desc.SampleDesc.Count = 1;
        Desc.CPUAccessFlags = 0;
        hr = m_pDevice->CreateTexture2D(&Desc, NULL, &m_pCopyTexture_L);
        if (m_stereo)
        {
            hr = m_pDevice->CreateTexture2D(&Desc, NULL, &m_pCopyTexture_R);
        }
    }
    D3D11_BOX box;
    box.left = pSrcRect->left;
    box.top = pSrcRect->top;
    box.right = pSrcRect->right;
    box.bottom = pSrcRect->bottom;
    box.front = 0;
    box.back = 1;

    switch (eFrameType)
    {
    case amf::AMF_FRAME_STEREO_RIGHT:
        spContext->CopySubresourceRegion(m_pCopyTexture_R, 0, 0, 0, 0,
                        pSrcSurface, 0, &box); // we expect that texture comes as a single slice
        break;
    case amf::AMF_FRAME_STEREO_BOTH:
        spContext->CopySubresourceRegion(m_pCopyTexture_L, 0, 0, 0, 0,
                        pSrcSurface, 0, &box); // we expect that texture comes as array of two silces
        spContext->CopySubresourceRegion(m_pCopyTexture_R, 0, 0, 0, 0,
                        pSrcSurface, 1, &box);
        break;
    default:
        spContext->CopySubresourceRegion(m_pCopyTexture_L, 0, 0, 0, 0,
                        pSrcSurface, 0, &box);
        break;
    }
    return AMF_OK;
}

AMF_RESULT VideoPresenterDX11::DrawFrame(ID3D11Texture2D* pSrcSurface,
                bool bLeft)
{
    HRESULT hr = S_OK;
    CComPtr < ID3D11DeviceContext > spContext;
    m_pDevice->GetImmediateContext(&spContext);

    D3D11_TEXTURE2D_DESC srcDesc;
    pSrcSurface->GetDesc(&srcDesc);

    CComPtr < ID3D11ShaderResourceView > pTextureRV;
    if (srcDesc.ArraySize == 1)
    {
        hr
                        = m_pDevice->CreateShaderResourceView(pSrcSurface,
                                        NULL, &pTextureRV);
    }
    else
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        ZeroMemory(&viewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
        viewDesc.Format = srcDesc.Format;
        // For stereo support
        viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY; // we expect that texture comes as array of two silces
        viewDesc.Texture2DArray.ArraySize = srcDesc.ArraySize;
        viewDesc.Texture2DArray.FirstArraySlice = bLeft ? 0 : 1; // left or right
        viewDesc.Texture2DArray.MipLevels = 1;
        viewDesc.Texture2DArray.MostDetailedMip = 0;
        hr = m_pDevice->CreateShaderResourceView(pSrcSurface, &viewDesc,
                        &pTextureRV);

    }
    spContext->PSSetShaderResources(0, 1, &pTextureRV.p);

    CComPtr < ID3D11RenderTargetView > pRenderTargetView
                    = bLeft ? m_pRenderTargetView_L : m_pRenderTargetView_R;
    float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    spContext->ClearRenderTargetView(pRenderTargetView, ClearColor);
    spContext->OMSetRenderTargets(1, &pRenderTargetView.p, NULL);

    spContext->Draw(4, 0);

    return AMF_OK;
}

AMF_RESULT VideoPresenterDX11::Init()
{
    AMF_RESULT err = AMF_OK;

    VideoPresenter::Init();

    m_pDevice = static_cast<ID3D11Device*> (m_pContext->GetDX11Device());
    if (m_pDevice == NULL)
    {
        err = AMF_NO_DEVICE;
    }

    if (err == AMF_OK)
    {
        err = CreatePresentationSwapChain();
    }
    err = CompileShaders();
    PrepareStates();

    return err;
}

AMF_RESULT VideoPresenterDX11::Terminate()
{
    m_pRenderTargetView_L = NULL;
    m_pRenderTargetView_R = NULL;

    m_pVertexBuffer = NULL;
    m_pCBChangesOnResize = NULL;
    m_pVertexShader = NULL;
    m_pPixelShader = NULL;
    m_pVertexLayout = NULL;
    m_pSampler = NULL;
    m_pDepthStencilState = NULL;
    m_pRasterizerState = NULL;
    m_pBlendState = NULL;

    m_pCopyTexture_L = NULL;
    m_pCopyTexture_R = NULL;

    m_pContext = NULL;
    m_hwnd = NULL;

    m_pDevice = NULL;
    m_pSwapChain.Release();
    return AMF_OK;
}

AMF_RESULT VideoPresenterDX11::CreatePresentationSwapChain()
{
    AMF_RESULT err = AMF_OK;
    HRESULT hr = S_OK;

    CComQIPtr < IDXGIDevice > spDXGIDevice = m_pDevice;

    CComPtr < IDXGIAdapter > spDXGIAdapter;
    hr = spDXGIDevice->GetParent(__uuidof(IDXGIAdapter),
                    (void **) &spDXGIAdapter);

    CComPtr < IDXGIFactory2 > spIDXGIFactory2;
    spDXGIAdapter->GetParent(__uuidof(IDXGIFactory2),
                    (void **) &spIDXGIFactory2);

    CComPtr < IDXGIOutput > spOutput;

    m_stereo = false;//for future

    if (spIDXGIFactory2 != NULL)
    {
        // clean context if swap chain was created on the same window before
        CComPtr < ID3D11DeviceContext > spContext;
        m_pDevice->GetImmediateContext(&spContext);
        spContext->ClearState();
        spContext->Flush();

        DXGI_SWAP_CHAIN_DESC1 swapChainDescription = { 0 };
        swapChainDescription.Width = 0; // use automatic sizing
        swapChainDescription.Height = 0;
        swapChainDescription.Format = DXGI_FORMAT_B8G8R8A8_UNORM;// DXGI_FORMAT_R8G8B8A8_UNORM;           // this is the most common swapchain format
        swapChainDescription.Stereo = m_stereo ? TRUE : FALSE; // create swapchain in stereo if stereo is enabled
        swapChainDescription.SampleDesc.Count = 1; // don't use multi-sampling
        swapChainDescription.SampleDesc.Quality = 0;
        swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDescription.BufferCount = 3; // use a single buffer and render to it as a FS quad
        swapChainDescription.Scaling = DXGI_SCALING_NONE; // set scaling to none//DXGI_SCALING_STRETCH;//
        swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // we recommend using this swap effect for all applications - only this works for stereo
        swapChainDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        CComQIPtr < IDXGIDevice1 > spDXGIDevice1 = spDXGIDevice;

        {
            hr = spIDXGIFactory2->CreateSwapChainForHwnd(m_pDevice,
                            (HWND) m_hwnd, &swapChainDescription, NULL,
                            spOutput, &m_pSwapChain1);
        }

        if (FAILED(hr) && m_stereo)
        {
            return AMF_FAIL;
        }
    }
    m_pSwapChain = m_pSwapChain1;
    if (m_pSwapChain1 == NULL)
    {
        m_stereo = false;
        CComPtr < IDXGIFactory > spIDXGIFactory;
        spDXGIAdapter->GetParent(__uuidof(IDXGIFactory),
                        (void **) &spIDXGIFactory);

        // setup params
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 1; // use a single buffer and render to it as a FS quad
        sd.BufferDesc.Width = 0; // will get fr0m window
        sd.BufferDesc.Height = 0; // will get fr0m window
        sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = (HWND) m_hwnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        hr = spIDXGIFactory->CreateSwapChain(m_pDevice, &sd, &m_pSwapChain);
        //ASSERT_RETURN_IF_HR_FAILED(hr,AMF_DIRECTX_FAILED,L"CreatePresentationSwapChain() - CreateSwapChain() failed");
    }

    return err;
}
AMF_RESULT VideoPresenterDX11::CompileShaders()
{
    HRESULT hr = S_OK;
    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( _DEBUG )
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
    CComPtr < ID3DBlob > pBlobVertexShader;
    CComPtr < ID3DBlob > pBlobPixelShader;

    CComPtr < ID3DBlob > pErrorBlob;
    hr = D3DCompile(DX11_FullScreenQuad, sizeof(DX11_FullScreenQuad),
                    "DX11_FullScreenQuad", NULL, NULL, "VS", "vs_4_0",
                    dwShaderFlags, 0, &pBlobVertexShader, &pErrorBlob);
    if (FAILED(hr))
    {
        char *data = (char *) pErrorBlob->GetBufferPointer();
        return AMF_FAIL;
    }
    hr = m_pDevice->CreateVertexShader(pBlobVertexShader->GetBufferPointer(),
                    pBlobVertexShader->GetBufferSize(), NULL, &m_pVertexShader);
    if (FAILED(hr))
    {
        return AMF_FAIL;
    }
    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
                    { { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                         D3D11_INPUT_PER_VERTEX_DATA, 0 },
                       { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
                         D3D11_INPUT_PER_VERTEX_DATA, 0 }, };
    UINT numElements = ARRAYSIZE(layout);
    // Create the input layout
    hr = m_pDevice->CreateInputLayout(layout, numElements,
                    pBlobVertexShader->GetBufferPointer(),
                    pBlobVertexShader->GetBufferSize(), &m_pVertexLayout);

    hr = D3DCompile(DX11_FullScreenQuad, sizeof(DX11_FullScreenQuad),
                    "DX11_FullScreenQuad", NULL, NULL, "PS", "ps_4_0",
                    dwShaderFlags, 0, &pBlobPixelShader, &pErrorBlob);
    if (FAILED(hr))
    {
        char *data = (char *) pErrorBlob->GetBufferPointer();
        return AMF_FAIL;
    }
    hr = m_pDevice->CreatePixelShader(pBlobPixelShader->GetBufferPointer(),
                    pBlobPixelShader->GetBufferSize(), NULL, &m_pPixelShader);
    if (FAILED(hr))
    {
        return AMF_FAIL;
    }

    return AMF_OK;
}

AMF_RESULT VideoPresenterDX11::PrepareStates()
{
    HRESULT hr = S_OK;
    // Create vertex buffer (quad will be set later)
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 4; // 4 - for video frame
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    hr = m_pDevice->CreateBuffer(&bd, NULL, &m_pVertexBuffer);

    // Initialize the view matrix
    XMMATRIX worldViewProjection = XMMatrixIdentity();
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(CBNeverChanges);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = { 0 };
    InitData.pSysMem = &worldViewProjection;

    hr = m_pDevice->CreateBuffer(&bd, &InitData, &m_pCBChangesOnResize);

    // Create the sample state

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = m_pDevice->CreateSamplerState(&sampDesc, &m_pSampler);

    // set Z-buffer off

    D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc = { 0 };
    depthDisabledStencilDesc.DepthEnable = FALSE;
    depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthDisabledStencilDesc.StencilEnable = TRUE;
    depthDisabledStencilDesc.StencilReadMask = 0xFF;
    depthDisabledStencilDesc.StencilWriteMask = 0xFF;
    depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.FrontFace.StencilDepthFailOp
                    = D3D11_STENCIL_OP_INCR;
    depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.BackFace.StencilDepthFailOp
                    = D3D11_STENCIL_OP_DECR;
    depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    hr = m_pDevice->CreateDepthStencilState(&depthDisabledStencilDesc,
                    &m_pDepthStencilState);

    // Create the rasterizer state which will determine how and what polygons will be drawn.
    D3D11_RASTERIZER_DESC rasterDesc;
    memset(&rasterDesc, 0, sizeof(rasterDesc));
    rasterDesc.AntialiasedLineEnable = false;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.DepthBias = 0;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = true;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.FrontCounterClockwise = false;
    rasterDesc.MultisampleEnable = false;
    rasterDesc.ScissorEnable = false;
    rasterDesc.SlopeScaledDepthBias = 0.0f;

    // Create the rasterizer state from the description we just filled out.
    hr = m_pDevice->CreateRasterizerState(&rasterDesc, &m_pRasterizerState);

    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(blendDesc));
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;

    hr = m_pDevice->CreateBlendState(&blendDesc, &m_pBlendState);

    return AMF_OK;
}

AMF_RESULT VideoPresenterDX11::CheckForResize(bool bForce)
{
    AMF_RESULT err = AMF_OK;
    HRESULT hr = S_OK;

    CComPtr < ID3D11Texture2D > spBuffer;
    hr = m_pSwapChain->GetBuffer(0, __uuidof(spBuffer),
                    reinterpret_cast<void**> (&spBuffer));

    D3D11_TEXTURE2D_DESC bufferDesc;
    spBuffer->GetDesc(&bufferDesc);

    RECT client;
    if (m_hwnd != NULL)
    {
        ::GetClientRect((HWND) m_hwnd, &client);
    }
    else if (m_pSwapChain1 != NULL)
    {
        DXGI_SWAP_CHAIN_DESC1 SwapDesc;
        m_pSwapChain1->GetDesc1(&SwapDesc);
        client.left = 0;
        client.top = 0;
        client.right = SwapDesc.Width;
        client.bottom = SwapDesc.Height;
    }
    amf_int width = client.right - client.left;
    amf_int height = client.bottom - client.top;

    if (!bForce && width == (amf_int) bufferDesc.Width && height
                    == (amf_int) bufferDesc.Height)
    {
        return AMF_OK;
    }
    // clear views and temp surfaces
    spBuffer = NULL;
    m_pRenderTargetView_L = NULL;
    m_pRenderTargetView_R = NULL;
    m_pCopyTexture_L = NULL;
    m_pCopyTexture_R = NULL;

    CComPtr < ID3D11DeviceContext > spContext;
    m_pDevice->GetImmediateContext(&spContext);

    //   spContext->ClearState();
    spContext->OMSetRenderTargets(0, 0, 0);

    // resize
    hr = m_pSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN,
                    DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

    // Create render target view
    CComPtr < ID3D11Texture2D > spBackBuffer;
    hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                    (LPVOID*) &spBackBuffer);

    D3D11_RENDER_TARGET_VIEW_DESC RenderTargetViewDescription;
    ZeroMemory(&RenderTargetViewDescription,
                    sizeof(RenderTargetViewDescription));
    RenderTargetViewDescription.Format = DXGI_FORMAT_B8G8R8A8_UNORM;//DXGI_FORMAT_R8G8B8A8_UNORM;

    RenderTargetViewDescription.ViewDimension
                    = D3D11_RTV_DIMENSION_TEXTURE2DARRAY; // render target view is a Texture2D array
    RenderTargetViewDescription.Texture2DArray.MipSlice = 0; // each array element is one Texture2D
    RenderTargetViewDescription.Texture2DArray.ArraySize = 1;
    RenderTargetViewDescription.Texture2DArray.FirstArraySlice = 0; // first Texture2D of the array is the left eye view

    hr = m_pDevice->CreateRenderTargetView(spBackBuffer,
                    &RenderTargetViewDescription, &m_pRenderTargetView_L);

    if (m_stereo)
    {
        RenderTargetViewDescription.Texture2DArray.FirstArraySlice = 1; // second Texture2D of the array is the right eye view
        hr = m_pDevice->CreateRenderTargetView(spBackBuffer,
                        &RenderTargetViewDescription, &m_pRenderTargetView_R);
    }

    m_CurrentViewport.TopLeftX = 0;
    m_CurrentViewport.TopLeftY = 0;
    m_CurrentViewport.Width = FLOAT(width);
    m_CurrentViewport.Height = FLOAT(height);
    m_CurrentViewport.MinDepth = 0.0f;
    m_CurrentViewport.MaxDepth = 1.0f;
    return AMF_OK;
}
AMF_RESULT VideoPresenterDX11::UpdateVertices(AMFRect* pSrcRect,
                ID3D11Texture2D *textureSrc)
{
    HRESULT hr = S_OK;
    CComPtr < ID3D11Texture2D > spBackBuffer;
    hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                    (LPVOID*) &spBackBuffer);

    D3D11_TEXTURE2D_DESC bufferDesc;
    spBackBuffer->GetDesc(&bufferDesc);

    if (m_sourceRect.left == pSrcRect->left && m_sourceRect.top
                    == pSrcRect->top && m_sourceRect.right == pSrcRect->right
                    && m_sourceRect.bottom == pSrcRect->bottom && m_dstWidth
                    == bufferDesc.Width && m_dstHeight == bufferDesc.Height)
    {
        return AMF_OK;
    }

    m_sourceRect = *pSrcRect;
    m_dstWidth = bufferDesc.Width;
    m_dstHeight = bufferDesc.Height;

    SimpleVertex vertices[4];

    D3D11_TEXTURE2D_DESC srcDesc;
    textureSrc->GetDesc(&srcDesc);

    // stretch video rect to back buffer
    FLOAT w = 2.f;
    FLOAT h = 2.f;

    w *= m_fScale;
    h *= m_fScale;

    FLOAT fVideoRatio = static_cast<FLOAT> (m_sourceRect.Width()
                    * m_fPixelAspectRatio / m_sourceRect.Height());
    FLOAT fScreenRatio = static_cast<FLOAT> (m_dstWidth) / m_dstHeight;

    if (fVideoRatio > fScreenRatio)
    {
        h *= fScreenRatio;
        h /= fVideoRatio;
    }
    else
    {
        w /= fScreenRatio;
        w *= fVideoRatio;
    }

    FLOAT centerX = m_fOffsetX * 2.f / m_dstWidth;
    FLOAT centerY = -m_fOffsetY * 2.f / m_dstHeight;

    FLOAT leftDst = centerX - w / 2;
    FLOAT rightDst = leftDst + w;
    FLOAT topDst = centerY - h / 2;
    FLOAT bottomDst = topDst + h;

    centerX = (m_sourceRect.left + m_sourceRect.right) / 2.f / srcDesc.Width;
    centerY = (m_sourceRect.top + m_sourceRect.bottom) / 2.f / srcDesc.Height;

    w = (FLOAT) m_sourceRect.Width() / srcDesc.Width;
    h = (FLOAT) m_sourceRect.Height() / srcDesc.Height;

    FLOAT leftSrc = centerX - w / 2;
    FLOAT rightSrc = leftSrc + w;
    FLOAT topSrc = centerY - h / 2;
    FLOAT bottomSrc = topSrc + h;

    vertices[0].position = XMFLOAT3(leftDst, bottomDst, 0.0f);
    vertices[0].texture = XMFLOAT2(leftSrc, topSrc);

    vertices[1].position = XMFLOAT3(rightDst, bottomDst, 0.0f);
    vertices[1].texture = XMFLOAT2(rightSrc, topSrc);

    vertices[2].position = XMFLOAT3(leftDst, topDst, 0.0f);
    vertices[2].texture = XMFLOAT2(leftSrc, bottomSrc);

    // Second triangle.
    vertices[3].position = XMFLOAT3(rightDst, topDst, 0.0f);
    vertices[3].texture = XMFLOAT2(rightSrc, bottomSrc);

    CComPtr < ID3D11DeviceContext > spContext;
    m_pDevice->GetImmediateContext(&spContext);
    spContext->UpdateSubresource(m_pVertexBuffer, 0, NULL, vertices, 0, 0);

    return AMF_OK;
}