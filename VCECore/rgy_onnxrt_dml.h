// -----------------------------------------------------------------------------------------
//     VCEEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2019-2021 rigaya
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
// IABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// ------------------------------------------------------------------------------------------

#pragma once
#ifndef __RGY_ONNXRT_DML_H__
#define __RGY_ONNXRT_DML_H__

#include <memory>
#include <cstdint>
#include "rgy_err.h"

// Set to 1 by the build (preprocessor define) when VCEEnc is built with the
// ONNX Runtime DirectML backend wired in. When 0 the wrapper still compiles but
// every call returns RGY_ERR_UNSUPPORTED and --vpp-onnx reports that this build
// has no DirectML support. onnxruntime.dll / libonnxruntime.so (a
// DirectML-enabled build) is loaded at runtime; no import library is linked,
// so the exact runtime library can be dropped next to the executable.
#ifndef ENABLE_ONNXRUNTIME
#define ENABLE_ONNXRUNTIME 0
#endif

// Thin wrapper over an ONNX Runtime session that runs on the DirectML execution
// provider. This is the VCEEnc counterpart of the QSVEnc RGYOpenVINO backend:
// one generic load-and-run covers every model family with no per-network code.
// DirectML is vendor agnostic, so on a multi-GPU machine its default adapter
// (index 0) may not be the GPU VCEEnc selected. init() therefore takes the LUID
// of VCEEnc's chosen adapter and binds the DirectML provider to the DXGI adapter
// whose LUID matches it, so inference runs on the same GPU as the encoder.
//
// The whole ONNX Runtime / DirectML include surface is confined to the .cpp via
// the pimpl below, so no other translation unit pulls in those headers.
class RGYOnnxRTDML {
public:
    RGYOnnxRTDML();
    ~RGYOnnxRTDML();

    // Load an ONNX model, create a DirectML session bound to the adapter whose
    // LUID matches (luidLow, luidHigh) -- pass VCEEnc's selected adapter LUID
    // split into its LowPart (uint32) and HighPart (int32) so this header needs
    // no windows.h. If no adapter matches, falls back to DirectML adapter 0. The
    // input is treated as [1, channels, height, width] (channels read from the
    // model); a probe inference discovers the output shape (and warms the DML
    // graph compile). On failure errMessage carries the ONNX Runtime error text.
    RGY_ERR init(const tstring &modelPath, const uint32_t luidLow, const int32_t luidHigh,
                 const int height, const int width, tstring &errMessage);

    // Synchronous inference. in points to inChannels()*inHeight()*inWidth()
    // floats (CHW); out receives outChannels()*outHeight()*outWidth() floats
    // (CHW). Blocking; in only needs to stay valid for the call.
    RGY_ERR infer(const float *in, float *out);

    int inChannels()  const;
    int inHeight()    const;
    int inWidth()     const;
    int outChannels() const;
    int outHeight()   const;
    int outWidth()    const;
    size_t outElemCount() const; // outChannels()*outHeight()*outWidth()

    tstring deviceFullName() const;     // DXGI adapter description bound to
    tstring inferencePrecision() const; // "f32" (DirectML runs the graph as authored)

    static bool available() { return ENABLE_ONNXRUNTIME != 0; }

private:
    RGYOnnxRTDML(const RGYOnnxRTDML &) = delete;
    void operator=(const RGYOnnxRTDML &) = delete;

    class Impl;
    std::unique_ptr<Impl> m_impl;
};

#endif //__RGY_ONNXRT_DML_H__
