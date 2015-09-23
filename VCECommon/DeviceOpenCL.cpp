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
 * @file <DeviceOpenCL.cpp>
 *
 * @brief Source file for OpenCL Device
 *
 *******************************************************************************
 */

#include "DeviceOpenCL.h"
#include "CmdLogger.h"
#include <CL/cl_d3d11.h>
#include <CL/cl_dx9_media_sharing.h>
#include <CL/cl_gl.h>

#pragma comment(lib, "opencl.lib")

#pragma warning(disable: 4996)
DeviceOpenCL::DeviceOpenCL() :
    m_hCommandQueue(0), m_hContext(0)
{
}

DeviceOpenCL::~DeviceOpenCL()
{
    Terminate();
}

AMF_RESULT DeviceOpenCL::Init(IDirect3DDevice9* pD3DDevice9,
                ID3D11Device* pD3DDevice11, HGLRC hContextOGL, HDC hDC)
{
    AMF_RESULT res = AMF_OK;
    cl_int status = 0;
    cl_platform_id platformID = 0;
    cl_device_id interoppedDeviceID = 0;
    // get AMD platform:
    res = FindPlatformID(platformID);
    CHECK_AMF_ERROR_RETURN(res, L"FindPlatformID() failed");

    std::vector < cl_context_properties > cps;

    // add devices if needed
    if (pD3DDevice11 != NULL)
    {
        clGetDeviceIDsFromD3D11KHR_fn
                        pClGetDeviceIDsFromD3D11KHR =
                                        static_cast<clGetDeviceIDsFromD3D11KHR_fn> (clGetExtensionFunctionAddressForPlatform(
                                                        platformID,
                                                        "clGetDeviceIDsFromD3D11KHR"));
        if (pClGetDeviceIDsFromD3D11KHR == NULL)
        {
            LOG_ERROR(L"Cannot resolve ClGetDeviceIDsFromD3D11KHR function");
            return AMF_FAIL;
        }

        cl_device_id deviceDX11 = 0;
        status = (*pClGetDeviceIDsFromD3D11KHR)(platformID,
                        CL_D3D11_DEVICE_KHR, (void*) pD3DDevice11,
                        CL_PREFERRED_DEVICES_FOR_D3D11_KHR, 1, &deviceDX11,
                        NULL);
        CHECK_OPENCL_ERROR_RETURN(status,
                        L"pClGetDeviceIDsFromD3D11KHR() failed");
        interoppedDeviceID = deviceDX11;
        m_hDeviceIDs.push_back(deviceDX11);
        cps.push_back(CL_CONTEXT_D3D11_DEVICE_KHR);
        cps.push_back((cl_context_properties) pD3DDevice11);
    }
    if (pD3DDevice9 != NULL)
    {
        clGetDeviceIDsFromDX9MediaAdapterKHR_fn
                        pclGetDeviceIDsFromDX9MediaAdapterKHR =
                                        static_cast<clGetDeviceIDsFromDX9MediaAdapterKHR_fn> (clGetExtensionFunctionAddressForPlatform(
                                                        platformID,
                                                        "clGetDeviceIDsFromDX9MediaAdapterKHR"));
        if (pclGetDeviceIDsFromDX9MediaAdapterKHR == NULL)
        {
            LOG_ERROR(
                            L"Cannot resolve clGetDeviceIDsFromDX9MediaAdapterKHR function");
            return AMF_FAIL;
        }
        cl_dx9_media_adapter_type_khr mediaAdapterType = CL_ADAPTER_D3D9EX_KHR;
        cl_device_id deviceDX9 = 0;
        status = (*pclGetDeviceIDsFromDX9MediaAdapterKHR)(platformID, 1,
                        &mediaAdapterType, &pD3DDevice9,
                        CL_PREFERRED_DEVICES_FOR_DX9_MEDIA_ADAPTER_KHR, 1,
                        &deviceDX9, NULL);
        CHECK_OPENCL_ERROR_RETURN(status,
                        L"pclGetDeviceIDsFromDX9MediaAdapterKHR() failed");

        interoppedDeviceID = deviceDX9;
        m_hDeviceIDs.push_back(deviceDX9);

        cps.push_back(CL_CONTEXT_ADAPTER_D3D9EX_KHR);
        cps.push_back((cl_context_properties) pD3DDevice9);
    }
    if (hContextOGL != NULL)
    {
        clGetGLContextInfoKHR_fn
                        pclGetGLContextInfoKHR =
                                        static_cast<clGetGLContextInfoKHR_fn> (clGetExtensionFunctionAddressForPlatform(
                                                        platformID,
                                                        "clGetGLContextInfoKHR"));
        if (pclGetGLContextInfoKHR == NULL)
        {
            LOG_ERROR(L"Cannot resolve clGetGLContextInfoKHR function");
            return AMF_FAIL;
        }
        std::vector < cl_context_properties > gl_cps;
        gl_cps.push_back(CL_CONTEXT_PLATFORM);
        gl_cps.push_back((cl_context_properties) platformID);
        gl_cps.push_back(CL_GL_CONTEXT_KHR);
        gl_cps.push_back((cl_context_properties) hContextOGL);
        gl_cps.push_back(CL_WGL_HDC_KHR);
        gl_cps.push_back((cl_context_properties) hDC);
        gl_cps.push_back(0);

        cl_device_id deviceGL = NULL;
        status = pclGetGLContextInfoKHR(&gl_cps[0],
                        CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
                        sizeof(cl_device_id), &deviceGL, NULL);
        CHECK_OPENCL_ERROR_RETURN(status, L"clGetGLContextInfoKHR() failed");
        interoppedDeviceID = deviceGL;
        m_hDeviceIDs.push_back(deviceGL);

        cps.push_back(CL_GL_CONTEXT_KHR);
        cps.push_back((cl_context_properties) hContextOGL);

        cps.push_back(CL_WGL_HDC_KHR);
        cps.push_back((cl_context_properties) hDC);
    }
    cps.push_back(CL_CONTEXT_INTEROP_USER_SYNC);
    cps.push_back(CL_TRUE);

    cps.push_back(CL_CONTEXT_PLATFORM);
    cps.push_back((cl_context_properties) platformID);
    cps.push_back(0);

    if (interoppedDeviceID == NULL)
    {
        status = clGetDeviceIDs((cl_platform_id) platformID,
                        CL_DEVICE_TYPE_GPU, 1,
                        (cl_device_id*) &interoppedDeviceID, NULL);
        CHECK_OPENCL_ERROR_RETURN(status, L"clGetDeviceIDs() failed");
        m_hDeviceIDs.push_back(interoppedDeviceID);
    }
    if (hContextOGL != 0)
    {
        wglMakeCurrent(hDC, hContextOGL);
    }
    m_hContext = clCreateContext(&cps[0], 1, &interoppedDeviceID, NULL, NULL,
                    &status);
    if (hContextOGL != 0)
    {
        wglMakeCurrent(NULL, NULL);
    }
    CHECK_OPENCL_ERROR_RETURN(status, L"clCreateContext() failed");

    m_hCommandQueue = clCreateCommandQueue(m_hContext, interoppedDeviceID,
                    (cl_command_queue_properties) NULL, &status);
    CHECK_OPENCL_ERROR_RETURN(status, L"clCreateCommandQueue() failed");
    return AMF_OK;
}

AMF_RESULT DeviceOpenCL::Terminate()
{
    if (m_hCommandQueue != 0)
    {
        clReleaseCommandQueue( m_hCommandQueue);
        m_hCommandQueue = NULL;
    }
    if (m_hDeviceIDs.size() != 0)
    {
        for (std::vector<cl_device_id>::iterator it = m_hDeviceIDs.begin(); it
                        != m_hDeviceIDs.end(); it++)
        {
            clReleaseDevice(*it);
        }
        m_hDeviceIDs.clear();
        ;
    }
    if (m_hContext != 0)
    {
        clReleaseContext( m_hContext);
        m_hContext = NULL;
    }
    return AMF_OK;
}
AMF_RESULT DeviceOpenCL::FindPlatformID(cl_platform_id &platform)
{
    cl_int status = 0;
    cl_uint numPlatforms = 0;
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    CHECK_OPENCL_ERROR_RETURN(status, L"clGetPlatformIDs() failed");
    if (numPlatforms == 0)
    {
        LOG_ERROR("clGetPlatformIDs() returned 0 platforms: ");
        return AMF_FAIL;
    }
    std::vector < cl_platform_id > platforms;
    platforms.resize(numPlatforms);
    status = clGetPlatformIDs(numPlatforms, &platforms[0], NULL);
    CHECK_OPENCL_ERROR_RETURN(status, L"clGetPlatformIDs() failed");
    bool bFound = false;
    for (cl_uint i = 0; i < numPlatforms; ++i)
    {
        char pbuf[1000];
        status = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR,
                        sizeof(pbuf), pbuf, NULL);
        CHECK_OPENCL_ERROR_RETURN(status, L"clGetPlatformInfo() failed");
        if (!strcmp(pbuf, "Advanced Micro Devices, Inc."))
        {
            platform = platforms[i];
            bFound = true;
            return AMF_OK;
        }
    }
    return AMF_FAIL;
}
