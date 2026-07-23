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

#include "rgy_onnxrt_dml.h"

#if ENABLE_ONNXRUNTIME

#include <cstring>
#include <vector>
#include <string>
#include <mutex>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include "rgy_onnxruntime.h"
#include "rgy_util.h"

using Microsoft::WRL::ComPtr;

// ------- one-time dynamic load of ONNX Runtime + Ort C++ API init -------------

namespace {
    std::once_flag    s_ortInitOnce;
    bool              s_ortReady = false;
    tstring           s_ortError;

    RGYOnnxRuntimeLoader& onnxRuntime() {
        static RGYOnnxRuntimeLoader loader;
        return loader;
    }

    void loadOrtOnce() {
        std::call_once(s_ortInitOnce, []() {
            if (!onnxRuntime().load()) {
                s_ortError = onnxRuntime().errMessage();
                return;
            }
            s_ortReady = true;
        });
    }

    // Map an adapter LUID to its DirectML device_id, which is the DXGI adapter
    // enumeration index (DirectML enumerates adapters in DXGI order). Returns 0
    // (DirectML's default) when no adapter matches or the LUID is zero.
    int dxgiIndexForLuid(uint32_t luidLow, int32_t luidHigh, tstring &adapterName) {
        adapterName.clear();
        if (luidLow == 0 && luidHigh == 0) return 0;
        ComPtr<IDXGIFactory1> factory;
        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) return 0;
        ComPtr<IDXGIAdapter1> adapter;
        for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
            DXGI_ADAPTER_DESC1 desc{};
            if (SUCCEEDED(adapter->GetDesc1(&desc))
                && desc.AdapterLuid.LowPart == (DWORD)luidLow
                && desc.AdapterLuid.HighPart == (LONG)luidHigh) {
                adapterName = wstring_to_tstring(desc.Description);
                return (int)i;
            }
            adapter.Reset();
        }
        return 0;
    }

}

// ------------------------------- pimpl ---------------------------------------

class RGYOnnxRTDML::Impl {
public:
    Impl() {}
    // env / alloc construct the ONNX Runtime C++ objects, which require the API
    // to be initialised first (loadOrtOnce in init), so they are created lazily
    // in init() rather than in this constructor.
    std::unique_ptr<Ort::Env> env;
    std::unique_ptr<Ort::AllocatorWithDefaultOptions> alloc;
    std::unique_ptr<Ort::Session> session{ nullptr };
    std::string inName, outName;     // owned copies of the model's first I/O names
    int inC = 0, inH = 0, inW = 0;
    int outC = 0, outH = 0, outW = 0;
    tstring adapterName;
    tstring precision = _T("f32");
};

RGYOnnxRTDML::RGYOnnxRTDML() : m_impl(std::make_unique<Impl>()) {}
RGYOnnxRTDML::~RGYOnnxRTDML() {}

RGY_ERR RGYOnnxRTDML::init(const tstring &modelPath, const uint32_t luidLow, const int32_t luidHigh,
                           const int height, const int width, tstring &errMessage) {
    loadOrtOnce();
    if (!s_ortReady) {
        errMessage = s_ortError;
        return RGY_ERR_UNSUPPORTED;
    }
    try {
        auto &I = *m_impl;
        // create the ORT env / allocator now that the API is initialised
        if (!I.env)   I.env   = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "vceenc-onnx");
        if (!I.alloc) I.alloc = std::make_unique<Ort::AllocatorWithDefaultOptions>();

        Ort::SessionOptions opts;
        // DirectML requires sequential execution and disabled memory pattern.
        opts.DisableMemPattern();
        opts.SetExecutionMode(ORT_SEQUENTIAL);
        opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        const int deviceId = dxgiIndexForLuid(luidLow, luidHigh, I.adapterName);
        OrtStatus *st = onnxRuntime().p_OrtSessionOptionsAppendExecutionProviderDML()(static_cast<OrtSessionOptions*>(opts), deviceId);
        if (st != nullptr) {
            errMessage = tstring(_T("AppendExecutionProvider_DML failed: "))
                       + char_to_tstring(Ort::GetApi().GetErrorMessage(st));
            Ort::GetApi().ReleaseStatus(st);
            return RGY_ERR_UNSUPPORTED;
        }

        I.session = std::make_unique<Ort::Session>(*I.env, modelPath.c_str(), opts);

        if (I.session->GetInputCount() < 1 || I.session->GetOutputCount() < 1) {
            errMessage = _T("model has no input/output tensor.");
            return RGY_ERR_UNSUPPORTED;
        }
        // names (own the strings; the AllocatedStringPtr frees on scope exit)
        {
            auto inN  = I.session->GetInputNameAllocated(0, *I.alloc);
            auto outN = I.session->GetOutputNameAllocated(0, *I.alloc);
            I.inName  = inN.get();
            I.outName = outN.get();
        }
        // input channel count from the model (dim 1); N/H/W are pinned by us
        auto inInfo  = I.session->GetInputTypeInfo(0).GetTensorTypeAndShapeInfo();
        auto inShape = inInfo.GetShape(); // may contain -1 for dynamic dims
        I.inC = (inShape.size() >= 2 && inShape[1] > 0) ? (int)inShape[1] : 1;
        I.inH = height;
        I.inW = width;

        // Probe inference with a zero input to discover the output shape and warm
        // the DirectML graph compile (the first run JITs the DML operators).
        std::vector<int64_t> inDims = { 1, I.inC, I.inH, I.inW };
        std::vector<float> zero((size_t)I.inC * I.inH * I.inW, 0.0f);
        Ort::MemoryInfo memCpu = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        Ort::Value inT = Ort::Value::CreateTensor<float>(memCpu, zero.data(), zero.size(),
                                                         inDims.data(), inDims.size());
        const char *inNames[]  = { I.inName.c_str() };
        const char *outNames[] = { I.outName.c_str() };
        auto outs = I.session->Run(Ort::RunOptions{ nullptr }, inNames, &inT, 1, outNames, 1);
        auto oShape = outs[0].GetTensorTypeAndShapeInfo().GetShape();
        if (oShape.size() != 4) {
            errMessage = _T("model output is not a 4D NCHW tensor.");
            return RGY_ERR_UNSUPPORTED;
        }
        I.outC = (int)oShape[1];
        I.outH = (int)oShape[2];
        I.outW = (int)oShape[3];
    } catch (const Ort::Exception &e) {
        errMessage = char_to_tstring(e.what());
        return RGY_ERR_UNKNOWN;
    } catch (const std::exception &e) {
        errMessage = char_to_tstring(e.what());
        return RGY_ERR_UNKNOWN;
    }
    return RGY_ERR_NONE;
}

RGY_ERR RGYOnnxRTDML::infer(const float *in, float *out) {
    if (!m_impl->session) return RGY_ERR_NULL_PTR;
    try {
        auto &I = *m_impl;
        std::vector<int64_t> inDims = { 1, I.inC, I.inH, I.inW };
        const size_t inCount = (size_t)I.inC * I.inH * I.inW;
        Ort::MemoryInfo memCpu = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        Ort::Value inT = Ort::Value::CreateTensor<float>(memCpu, const_cast<float*>(in), inCount,
                                                         inDims.data(), inDims.size());
        const char *inNames[]  = { I.inName.c_str() };
        const char *outNames[] = { I.outName.c_str() };
        auto outs = I.session->Run(Ort::RunOptions{ nullptr }, inNames, &inT, 1, outNames, 1);
        const float *res = outs[0].GetTensorData<float>();
        std::memcpy(out, res, outElemCount() * sizeof(float));
    } catch (const Ort::Exception &e) {
        (void)e;
        return RGY_ERR_UNKNOWN;
    }
    return RGY_ERR_NONE;
}

int RGYOnnxRTDML::inChannels()  const { return m_impl->inC; }
int RGYOnnxRTDML::inHeight()    const { return m_impl->inH; }
int RGYOnnxRTDML::inWidth()     const { return m_impl->inW; }
int RGYOnnxRTDML::outChannels() const { return m_impl->outC; }
int RGYOnnxRTDML::outHeight()   const { return m_impl->outH; }
int RGYOnnxRTDML::outWidth()    const { return m_impl->outW; }
size_t RGYOnnxRTDML::outElemCount() const {
    return (size_t)m_impl->outC * m_impl->outH * m_impl->outW;
}
tstring RGYOnnxRTDML::deviceFullName() const { return m_impl->adapterName; }
tstring RGYOnnxRTDML::inferencePrecision() const { return m_impl->precision; }

class RGYOnnxRTDMLMultiIO::Impl {
public:
    std::unique_ptr<Ort::Env> env;
    std::unique_ptr<Ort::AllocatorWithDefaultOptions> alloc;
    std::unique_ptr<Ort::Session> session{ nullptr };
    std::vector<std::string> inNames, outNames;
    std::vector<std::vector<int64_t>> inShapes, outShapes;
    tstring adapterName;
    tstring precision = _T("f32");
};

RGYOnnxRTDMLMultiIO::RGYOnnxRTDMLMultiIO() : m_impl(std::make_unique<Impl>()) {}
RGYOnnxRTDMLMultiIO::~RGYOnnxRTDMLMultiIO() {}

RGY_ERR RGYOnnxRTDMLMultiIO::init(const tstring &modelPath, const uint32_t luidLow, const int32_t luidHigh, tstring &errMessage) {
    loadOrtOnce();
    if (!s_ortReady) { errMessage = s_ortError; return RGY_ERR_UNSUPPORTED; }
    try {
        auto &I = *m_impl;
        I.env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "vceenc-onnx-multiio");
        I.alloc = std::make_unique<Ort::AllocatorWithDefaultOptions>();
        Ort::SessionOptions opts;
        opts.DisableMemPattern();
        opts.SetExecutionMode(ORT_SEQUENTIAL);
        opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        const int deviceId = dxgiIndexForLuid(luidLow, luidHigh, I.adapterName);
        OrtStatus *st = onnxRuntime().p_OrtSessionOptionsAppendExecutionProviderDML()(static_cast<OrtSessionOptions*>(opts), deviceId);
        if (st != nullptr) {
            errMessage = tstring(_T("AppendExecutionProvider_DML failed: ")) + char_to_tstring(Ort::GetApi().GetErrorMessage(st));
            Ort::GetApi().ReleaseStatus(st);
            return RGY_ERR_UNSUPPORTED;
        }
        I.session = std::make_unique<Ort::Session>(*I.env, modelPath.c_str(), opts);
        const size_t nin = I.session->GetInputCount();
        const size_t nout = I.session->GetOutputCount();
        if (nin == 0 || nout == 0) { errMessage = _T("model has no input/output tensor."); return RGY_ERR_UNSUPPORTED; }
        for (size_t i = 0; i < nin; i++) {
            auto name = I.session->GetInputNameAllocated(i, *I.alloc);
            I.inNames.emplace_back(name.get());
            auto shape = I.session->GetInputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
            if (shape.size() != 4) { errMessage = _T("model input is not a 4D NCHW tensor."); return RGY_ERR_UNSUPPORTED; }
            for (auto &dim : shape) if (dim < 0) dim = 1;
            I.inShapes.push_back(std::move(shape));
        }
        for (size_t i = 0; i < nout; i++) {
            auto name = I.session->GetOutputNameAllocated(i, *I.alloc);
            I.outNames.emplace_back(name.get());
            auto shape = I.session->GetOutputTypeInfo(i).GetTensorTypeAndShapeInfo().GetShape();
            if (shape.size() != 4) { errMessage = _T("model output is not a 4D NCHW tensor."); return RGY_ERR_UNSUPPORTED; }
            for (auto &dim : shape) if (dim < 0) dim = 1;
            I.outShapes.push_back(std::move(shape));
        }
    } catch (const Ort::Exception &e) {
        errMessage = char_to_tstring(e.what());
        return RGY_ERR_UNKNOWN;
    } catch (const std::exception &e) {
        errMessage = char_to_tstring(e.what());
        return RGY_ERR_UNKNOWN;
    }
    return RGY_ERR_NONE;
}

RGY_ERR RGYOnnxRTDMLMultiIO::infer(const std::vector<const float *> &inputs, const std::vector<float *> &outputs, tstring &errMessage) {
    if (!m_impl->session || inputs.size() != m_impl->inNames.size() || outputs.size() != m_impl->outNames.size()) return RGY_ERR_INVALID_PARAM;
    try {
        auto &I = *m_impl;
        Ort::MemoryInfo memCpu = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        std::vector<Ort::Value> tensors;
        std::vector<const char *> inNames, outNames;
        tensors.reserve(inputs.size());
        for (size_t i = 0; i < inputs.size(); i++) {
            size_t count = 1; for (auto dim : I.inShapes[i]) count *= (size_t)dim;
            tensors.emplace_back(Ort::Value::CreateTensor<float>(memCpu, const_cast<float *>(inputs[i]), count,
                I.inShapes[i].data(), I.inShapes[i].size()));
            inNames.push_back(I.inNames[i].c_str());
        }
        for (auto &name : I.outNames) outNames.push_back(name.c_str());
        auto result = I.session->Run(Ort::RunOptions{ nullptr }, inNames.data(), tensors.data(), tensors.size(), outNames.data(), outNames.size());
        for (size_t i = 0; i < result.size(); i++) {
            size_t count = 1; for (auto dim : I.outShapes[i]) count *= (size_t)dim;
            std::memcpy(outputs[i], result[i].GetTensorData<float>(), count * sizeof(float));
        }
    } catch (const Ort::Exception &e) {
        errMessage = char_to_tstring(e.what());
        return RGY_ERR_UNKNOWN;
    }
    return RGY_ERR_NONE;
}
const std::vector<std::string>& RGYOnnxRTDMLMultiIO::inputNames() const { return m_impl->inNames; }
const std::vector<std::string>& RGYOnnxRTDMLMultiIO::outputNames() const { return m_impl->outNames; }
const std::vector<int64_t>& RGYOnnxRTDMLMultiIO::inputShape(size_t index) const { return m_impl->inShapes.at(index); }
const std::vector<int64_t>& RGYOnnxRTDMLMultiIO::outputShape(size_t index) const { return m_impl->outShapes.at(index); }
tstring RGYOnnxRTDMLMultiIO::deviceFullName() const { return m_impl->adapterName; }
tstring RGYOnnxRTDMLMultiIO::inferencePrecision() const { return m_impl->precision; }

#else // !ENABLE_ONNXRUNTIME

class RGYOnnxRTDML::Impl {};
RGYOnnxRTDML::RGYOnnxRTDML() : m_impl(nullptr) {}
RGYOnnxRTDML::~RGYOnnxRTDML() {}
RGY_ERR RGYOnnxRTDML::init(const tstring &, const uint32_t, const int32_t, const int, const int, tstring &errMessage) {
    errMessage = _T("this build of VCEEnc has no ONNX Runtime DirectML support.");
    return RGY_ERR_UNSUPPORTED;
}
RGY_ERR RGYOnnxRTDML::infer(const float *, float *) { return RGY_ERR_UNSUPPORTED; }
int RGYOnnxRTDML::inChannels()  const { return 0; }
int RGYOnnxRTDML::inHeight()    const { return 0; }
int RGYOnnxRTDML::inWidth()     const { return 0; }
int RGYOnnxRTDML::outChannels() const { return 0; }
int RGYOnnxRTDML::outHeight()   const { return 0; }
int RGYOnnxRTDML::outWidth()    const { return 0; }
size_t RGYOnnxRTDML::outElemCount() const { return 0; }
tstring RGYOnnxRTDML::deviceFullName() const { return tstring(); }
tstring RGYOnnxRTDML::inferencePrecision() const { return tstring(); }

class RGYOnnxRTDMLMultiIO::Impl {};
RGYOnnxRTDMLMultiIO::RGYOnnxRTDMLMultiIO() : m_impl(nullptr) {}
RGYOnnxRTDMLMultiIO::~RGYOnnxRTDMLMultiIO() {}
RGY_ERR RGYOnnxRTDMLMultiIO::init(const tstring &, const uint32_t, const int32_t, tstring &errMessage) { errMessage = _T("this build of VCEEnc has no ONNX Runtime DirectML support."); return RGY_ERR_UNSUPPORTED; }
RGY_ERR RGYOnnxRTDMLMultiIO::infer(const std::vector<const float *> &, const std::vector<float *> &, tstring &) { return RGY_ERR_UNSUPPORTED; }
const std::vector<std::string>& RGYOnnxRTDMLMultiIO::inputNames() const { static const std::vector<std::string> v; return v; }
const std::vector<std::string>& RGYOnnxRTDMLMultiIO::outputNames() const { static const std::vector<std::string> v; return v; }
const std::vector<int64_t>& RGYOnnxRTDMLMultiIO::inputShape(size_t) const { static const std::vector<int64_t> v; return v; }
const std::vector<int64_t>& RGYOnnxRTDMLMultiIO::outputShape(size_t) const { static const std::vector<int64_t> v; return v; }
tstring RGYOnnxRTDMLMultiIO::deviceFullName() const { return tstring(); }
tstring RGYOnnxRTDMLMultiIO::inferencePrecision() const { return tstring(); }

#endif // ENABLE_ONNXRUNTIME
