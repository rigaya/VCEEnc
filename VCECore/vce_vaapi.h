// -----------------------------------------------------------------------------------------
//     VCEEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2014-2017 rigaya
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
// ------------------------------------------------------------------------------------------

#pragma once

#include "rgy_version.h"

#if ENABLE_VAAPI

#include "rgy_libavcodec_vaapi.h"

struct VCEParam;

// VA-API の実装は QSVEnc と共通の rgy_libavcodec_vaapi.* にある。
// ここには VCEParam からの変換、AMF 専用オプションの警告、Mesa 用の preset 対応だけを置く。
static constexpr uint32_t VCE_VA_VENDOR_ID_AMD = 0x1002;
using VCEVADeviceInfo = RGYVADeviceInfo;
using VCEDeviceVA = RGYDeviceVA;

class VCEEncoderVA : public RGYEncoderVA {
public:
    VCEEncoderVA();
    RGY_ERR init(VCEDeviceVA *dev, const VCEParam *prm, int width, int height,
        rgy_rational<int> sar, rgy_rational<int> fps, rgy_rational<int> timebase, std::shared_ptr<RGYLog> log);
    // 従来の VCEEnc の表示 (preset 名と level の表記) を保つため、共通側の表示を置き換える。
    tstring paramString() const;
    tstring levelString() const;
protected:
    int m_preset;
};

#endif // ENABLE_VAAPI
