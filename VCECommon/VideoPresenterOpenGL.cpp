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
 * @file <VideoPresenterOpenGL.cpp>
 *
 * @brief Source file for OpenGL Video Presenter
 *
 *******************************************************************************
 */

#include "VideoPresenterOpenGL.h"

#include <gl\GL.h>
#include <gl\GLU.h>
#pragma comment(lib, "opengl32.lib")

VideoPresenterOpenGL::VideoPresenterOpenGL(HWND hwnd, amf::AMFContext* pContext) :
    VideoPresenter(hwnd, pContext), m_hdc(NULL), m_hglrc(NULL), m_initialized(
                    false)
{

}

VideoPresenterOpenGL::~VideoPresenterOpenGL()
{
    Terminate();
}

AMF_RESULT VideoPresenterOpenGL::Present(amf::AMFSurface* pSurface)
{
    AMF_RESULT err = AMF_OK;

    const amf::AMF_SURFACE_FORMAT surfaceType = pSurface->GetFormat();

    err = pSurface->Convert(amf::AMF_MEMORY_OPENGL);
    if (err != AMF_OK)
    {
        return err;
    }

    amf::AMFContext::AMFOpenGLLocker glLocker(m_pContext);

    RECT tmpRectClient = { 0, 0, 500, 500 };
    BOOL getWindowRectResult = GetClientRect(m_hwnd, &tmpRectClient);
    AMFRect rectClient = AMFConstructRect(tmpRectClient.left,
                    tmpRectClient.top, tmpRectClient.right,
                    tmpRectClient.bottom);

    glViewport(0, 0, rectClient.right, rectClient.bottom);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear( GL_COLOR_BUFFER_BIT);

    amf::AMFPlane* pPlane = pSurface->GetPlane(amf::AMF_PLANE_PACKED);

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
        WaitForPTS(pSurface->GetPts());

        const float left = 2.f / rectClient.Width()
                        * ((float) rectClient.Width() / 2
                                        - (float) outputRect.left);
        const float right = 2.f / rectClient.Width()
                        * ((float) rectClient.Width() / 2
                                        - (float) outputRect.right);
        const float top = 2.f / rectClient.Height()
                        * ((float) rectClient.Height() / 2
                                        - (float) outputRect.top);
        const float bottom = 2.f / rectClient.Height()
                        * ((float) rectClient.Height() / 2
                                        - (float) outputRect.bottom);

        glEnable( GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, (GLuint) (long) (pPlane->GetNative()));
        glBegin( GL_QUADS);
        glTexCoord2f(1.0, 1.0);
        glVertex2f(left, bottom);
        glTexCoord2f(0.0, 1.0);
        glVertex2f(right, bottom);
        glTexCoord2f(0.0, 0.0);
        glVertex2f(right, top);
        glTexCoord2f(1.0, 0.0);
        glVertex2f(left, top);
        glEnd();

    }

    BOOL swapResult = ::SwapBuffers(m_hdc);
    return AMF_OK;
}

AMF_RESULT VideoPresenterOpenGL::Init()
{
    AMF_RESULT res = AMF_OK;

    VideoPresenter::Init();

    m_hdc = GetDC(m_hwnd);
    if (m_hdc, NULL)
    {
        res = AMF_FAIL;
    }

    if (res == AMF_OK)
    {
        PIXELFORMATDESCRIPTOR pfd = { 0 };
        pfd.nSize = sizeof(pfd);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL
                        | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 24;
        pfd.cDepthBits = 16;
        pfd.iLayerType = PFD_MAIN_PLANE;

        const int pixelFormatIdx = ChoosePixelFormat(m_hdc, &pfd);
        //THROW_SAMPLES_EXCEPTION_IF_EQUAL("ChoosePixelFormat failed", pixelFormatIdx, 0);

        BOOL setPixelFormatResult = SetPixelFormat(m_hdc, pixelFormatIdx, &pfd);
        //THROW_SAMPLES_EXCEPTION_IF_NOT_EQUAL("SetPixelFormat failed", setPixelFormatResult, TRUE);

        m_hglrc = wglCreateContext(m_hdc);
        //THROW_SAMPLES_EXCEPTION_IF_EQUAL("wglCreateContext failed", _hglrc, NULL);

        BOOL makeCurrentResult = wglMakeCurrent(m_hdc, m_hglrc);
        //THROW_SAMPLES_EXCEPTION_IF_NOT_EQUAL("wglMakeCurrent failed", makeCurrentResult, TRUE);

        glEnable( GL_TEXTURE_2D);

        m_initialized = true;
    }
    return res;
}

AMF_RESULT VideoPresenterOpenGL::Terminate()
{
    if (m_hglrc != NULL)
    {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext( m_hglrc);
    }

    if (m_hdc != NULL)
    {
        ReleaseDC(m_hwnd, m_hdc);
    }

    return AMF_OK;
}
