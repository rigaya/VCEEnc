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

// ONNX Runtime is loaded dynamically (onnxruntime.dll dropped next to the exe),
// so the C++ API must be initialised by hand rather than linking the import lib.
#define ORT_API_MANUAL_INIT
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4244 4267 4127 4100)
#endif
#include "onnxruntime_cxx_api.h"
// The DirectML provider's append function is resolved dynamically by name
// (PFN_AppendDML below), so dml_provider_factory.h -- which pulls in <DirectML.h>
// and <d3d12.h> -- is not needed here.
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

using Microsoft::WRL::ComPtr;

// ------- one-time dynamic load of onnxruntime.dll + Ort C++ API init ----------

// DML provider factory C entry point (exported by a DirectML-enabled
// onnxruntime.dll). Resolved by name so no import library is needed.
typedef OrtStatus*(ORT_API_CALL *PFN_AppendDML)(OrtSessionOptions *options, int device_id);

namespace {
    std::once_flag    s_ortInitOnce;
    bool              s_ortReady = false;
    std::string       s_ortError;
    PFN_AppendDML     s_appendDML = nullptr;

    void loadOrtOnce() {
        std::call_once(s_ortInitOnce, []() {
            HMODULE h = LoadLibraryW(L"onnxruntime.dll");
            if (!h) {
                s_ortError = "could not load onnxruntime.dll (a DirectML-enabled ONNX Runtime). "
                             "place onnxruntime.dll and DirectML.dll next to the executable.";
                return;
            }
            auto pGetApiBase = reinterpret_cast<const OrtApiBase*(ORT_API_CALL*)()>(
                GetProcAddress(h, "OrtGetApiBase"));
            if (!pGetApiBase) {
                s_ortError = "onnxruntime.dll is missing OrtGetApiBase (not an ONNX Runtime DLL?).";
                return;
            }
            const OrtApi *api = pGetApiBase()->GetApi(ORT_API_VERSION);
            if (!api) {
                s_ortError = "this onnxruntime.dll is older than the headers VCEEnc was built against.";
                return;
            }
            Ort::InitApi(api);
            s_appendDML = reinterpret_cast<PFN_AppendDML>(
                GetProcAddress(h, "OrtSessionOptionsAppendExecutionProvider_DML"));
            if (!s_appendDML) {
                s_ortError = "onnxruntime.dll has no DirectML provider "
                             "(install the DirectML build of onnxruntime).";
                return;
            }
            s_ortReady = true;
        });
    }

    // Map an adapter LUID to its DirectML device_id, which is the DXGI adapter
    // enumeration index (DirectML enumerates adapters in DXGI order). Returns 0
    // (DirectML's default) when no adapter matches or the LUID is zero.
    int dxgiIndexForLuid(uint32_t luidLow, int32_t luidHigh, std::string &adapterName) {
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
                char nm[256] = { 0 };
                WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, nm, sizeof(nm) - 1, nullptr, nullptr);
                adapterName = nm;
                return (int)i;
            }
            adapter.Reset();
        }
        return 0;
    }

    std::wstring utf8ToWide(const std::string &s) {
        if (s.empty()) return std::wstring();
        int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
        std::wstring w(n, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], n);
        return w;
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
    std::string adapterName;
    std::string precision = "f32";
};

RGYOnnxRTDML::RGYOnnxRTDML() : m_impl(std::make_unique<Impl>()) {}
RGYOnnxRTDML::~RGYOnnxRTDML() {}

RGY_ERR RGYOnnxRTDML::init(const std::string &modelPath, const uint32_t luidLow, const int32_t luidHigh,
                           const int height, const int width, std::string &errMessage) {
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
        OrtStatus *st = s_appendDML(static_cast<OrtSessionOptions*>(opts), deviceId);
        if (st != nullptr) {
            errMessage = std::string("AppendExecutionProvider_DML failed: ")
                       + Ort::GetApi().GetErrorMessage(st);
            Ort::GetApi().ReleaseStatus(st);
            return RGY_ERR_UNSUPPORTED;
        }

        const std::wstring wpath = utf8ToWide(modelPath);
        I.session = std::make_unique<Ort::Session>(*I.env, wpath.c_str(), opts);

        if (I.session->GetInputCount() < 1 || I.session->GetOutputCount() < 1) {
            errMessage = "model has no input/output tensor.";
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
            errMessage = "model output is not a 4D NCHW tensor.";
            return RGY_ERR_UNSUPPORTED;
        }
        I.outC = (int)oShape[1];
        I.outH = (int)oShape[2];
        I.outW = (int)oShape[3];
    } catch (const Ort::Exception &e) {
        errMessage = e.what();
        return RGY_ERR_UNKNOWN;
    } catch (const std::exception &e) {
        errMessage = e.what();
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
std::string RGYOnnxRTDML::deviceFullName() const { return m_impl->adapterName; }
std::string RGYOnnxRTDML::inferencePrecision() const { return m_impl->precision; }

#else // !ENABLE_ONNXRUNTIME

class RGYOnnxRTDML::Impl {};
RGYOnnxRTDML::RGYOnnxRTDML() : m_impl(nullptr) {}
RGYOnnxRTDML::~RGYOnnxRTDML() {}
RGY_ERR RGYOnnxRTDML::init(const std::string &, const uint32_t, const int32_t, const int, const int, std::string &errMessage) {
    errMessage = "this build of VCEEnc has no ONNX Runtime DirectML support.";
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
std::string RGYOnnxRTDML::deviceFullName() const { return std::string(); }
std::string RGYOnnxRTDML::inferencePrecision() const { return std::string(); }

#endif // ENABLE_ONNXRUNTIME
