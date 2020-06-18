// -----------------------------------------------------------------------------------------
// QSVEnc/NVEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2011-2016 rigaya
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// -------------------------------------------------------------------------------------------

#pragma once
#ifndef __RGY_DEVICE_H__
#define __RGY_DEVICE_H__

#include "rgy_osdep.h"
#include <SDKDDKVer.h>
#include <atlbase.h>
#include <string>
#include <memory>
#include <d3d9.h>
#include <d3d11.h>
#include "rgy_err.h"

class RGYLog;

class DeviceDX9 {
public:
    DeviceDX9();
    virtual ~DeviceDX9();

    RGY_ERR Init(bool dx9ex, int adapterID, bool bFullScreen, int width, int height, std::shared_ptr<RGYLog> log);
    RGY_ERR Terminate();

    bool isValid() const;
    ATL::CComPtr<IDirect3DDevice9> GetDevice();
    LUID getLUID() const { return m_devLUID; };
    std::wstring GetDisplayDeviceName() const { return m_displayDeviceName; }
    static int adapterCount();
private:
    RGY_ERR EnumerateAdapters();

    ATL::CComPtr<IDirect3D9>            m_pD3D;
    ATL::CComPtr<IDirect3DDevice9>      m_pD3DDevice;
    LUID                                m_devLUID;

    static const int             MAXADAPTERS = 128;
    int                          m_adaptersCount;
    int                          m_adaptersIndexes[MAXADAPTERS];
    std::wstring                 m_displayDeviceName;
    std::shared_ptr<RGYLog>      m_log;
};


class DeviceDX11 {
public:
    DeviceDX11();
    virtual ~DeviceDX11();

    RGY_ERR Init(int adapterID, bool onlyWithOutputs, std::shared_ptr<RGYLog> log);
    RGY_ERR Terminate();

    bool isValid() const;
    ATL::CComPtr<ID3D11Device>      GetDevice();
    LUID getLUID() const { return m_devLUID; };
    std::wstring GetDisplayDeviceName() const { return m_displayDeviceName; }
    static int adapterCount();
private:
    void EnumerateAdapters(bool onlyWithOutputs);

    ATL::CComPtr<ID3D11Device>   m_pD3DDevice;
    LUID                         m_devLUID;

    static const int             MAXADAPTERS = 128;
    int                          m_adaptersCount;
    int                          m_adaptersIndexes[MAXADAPTERS];
    std::wstring                 m_displayDeviceName;
    std::shared_ptr<RGYLog>      m_log;
};

#endif //__RGY_DEVICE_H__
