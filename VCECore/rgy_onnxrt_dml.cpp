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
#include <limits>
#include <stdexcept>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dxgi1_4.h>
#include <d3d11_1.h>
#include <d3d12.h>
#include <DirectML.h>
#include <wrl/client.h>

#include "rgy_onnxruntime.h"
#include "rgy_util.h"
#include "rgy_opencl.h"

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


namespace {
    // 公開OrtDmlApiの先頭4エントリ。通常のDML初期化APIとは独立して取得する。
    // 定義元: https://github.com/microsoft/onnxruntime/blob/v1.23.0/include/onnxruntime/core/providers/dml/dml_provider_factory.h
    struct RGYOrtDmlSharedApi {
        OrtStatus *(ORT_API_CALL *append)(OrtSessionOptions *, int);
        OrtStatus *(ORT_API_CALL *appendWithQueue)(OrtSessionOptions *, IDMLDevice *, ID3D12CommandQueue *);
        OrtStatus *(ORT_API_CALL *createAllocation)(ID3D12Resource *, void **);
        OrtStatus *(ORT_API_CALL *freeAllocation)(void *);
    };

    struct RGYDmlSharedRuntime {
        using CreateD3D12 = HRESULT (WINAPI *)(IUnknown *, D3D_FEATURE_LEVEL, REFIID, void **);
        using CreateDML = HRESULT (WINAPI *)(ID3D12Device *, DML_CREATE_DEVICE_FLAGS, REFIID, void **);
        HMODULE d3d12Module = nullptr;
        HMODULE dmlModule = nullptr;
        CreateD3D12 createD3D12 = nullptr;
        CreateDML createDML = nullptr;
        RGYDmlSharedRuntime() {
            d3d12Module = LoadLibraryExW(L"d3d12.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            dmlModule = LoadLibraryW(L"DirectML.dll");
            if (d3d12Module) createD3D12 = reinterpret_cast<CreateD3D12>(GetProcAddress(d3d12Module, "D3D12CreateDevice"));
            if (dmlModule) createDML = reinterpret_cast<CreateDML>(GetProcAddress(dmlModule, "DMLCreateDevice"));
        }
        ~RGYDmlSharedRuntime() {
            if (dmlModule) FreeLibrary(dmlModule);
            if (d3d12Module) FreeLibrary(d3d12Module);
        }
    };

    RGYDmlSharedRuntime& dmlSharedRuntime() {
        static RGYDmlSharedRuntime runtime;
        return runtime;
    }

    void dmlSharedCheckHr(HRESULT result, const char *operation) {
        if (FAILED(result)) throw std::runtime_error(strsprintf("%s: HRESULT=0x%08x", operation, (unsigned)result));
    }

    void dmlSharedCheckCl(cl_int result, const char *operation) {
        if (result != CL_SUCCESS) throw std::runtime_error(strsprintf("%s: OpenCL=%d", operation, result));
    }

    struct RGYDmlSharedState {
        struct Buffer {
            ComPtr<ID3D12Resource> d3d12;
            ComPtr<ID3D12Resource> texture;
            ComPtr<ID3D11Texture2D> d3d11;
            cl_mem image = nullptr;
            cl_mem opencl = nullptr;
            void *allocation = nullptr;
        } buffers[2];
        std::shared_ptr<RGYOpenCLContext> context;
        const RGYOrtDmlSharedApi *api = nullptr;
        ComPtr<ID3D12Device> device;
        ComPtr<IDMLDevice> dml;
        ComPtr<ID3D12CommandQueue> queue;
        ComPtr<ID3D12CommandAllocator> commandAllocator;
        ComPtr<ID3D12GraphicsCommandList> commandList;
        ComPtr<ID3D12Fence> fence;
        HANDLE fenceEvent = nullptr;
        UINT64 fenceValue = 0;
        cl_command_queue lastQueue = nullptr;
        bool acquired = false;
        size_t imageWidth = 0;
        size_t imageHeight = 0;
        std::unique_ptr<Ort::Value> input;
        std::unique_ptr<Ort::Value> output;
        std::unique_ptr<Ort::IoBinding> binding;

        cl_mem inputMem() const { return buffers[0].opencl; }
        cl_mem outputMem() const { return buffers[1].opencl; }

        void useQueue(cl_command_queue nextQueue) {
            cl_context queueContext = nullptr;
            dmlSharedCheckCl(clGetCommandQueueInfo(nextQueue, CL_QUEUE_CONTEXT, sizeof(queueContext), &queueContext, nullptr), "clGetCommandQueueInfo");
            if (queueContext != context->context()) throw std::runtime_error("共有GPUバッファとOpenCL queueのcontextが一致しません。");
            cl_command_queue_properties queueProperties = 0;
            dmlSharedCheckCl(clGetCommandQueueInfo(nextQueue, CL_QUEUE_PROPERTIES, sizeof(queueProperties), &queueProperties, nullptr), "clGetCommandQueueInfo(properties)");
            if ((queueProperties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) != 0) throw std::runtime_error("共有GPU推論にはin-orderのOpenCL queueが必要です。");
            if (nextQueue != lastQueue) {
                // 前フレームの出力が別queueで参照中でも、DMLへ所有権を渡す前に完了させる。
                if (lastQueue) dmlSharedCheckCl(clFinish(lastQueue), "clFinish(previous)");
                dmlSharedCheckCl(clRetainCommandQueue(nextQueue), "clRetainCommandQueue");
                if (lastQueue) clReleaseCommandQueue(lastQueue);
                lastQueue = nextQueue;
            }
        }

        void acquire() {
            const cl_mem objects[] = { buffers[0].image, buffers[1].image };
            dmlSharedCheckCl(context->platform()->ext().clEnqueueAcquireD3D11ObjectsKHR(lastQueue, 2, objects, 0, nullptr, nullptr), "clEnqueueAcquireD3D11ObjectsKHR");
            acquired = true;
            dmlSharedCheckCl(clFinish(lastQueue), "clFinish(acquire)");
        }

        void release() {
            const cl_mem objects[] = { buffers[0].image, buffers[1].image };
            dmlSharedCheckCl(context->platform()->ext().clEnqueueReleaseD3D11ObjectsKHR(lastQueue, 2, objects, 0, nullptr, nullptr), "clEnqueueReleaseD3D11ObjectsKHR");
            acquired = false;
            dmlSharedCheckCl(clFinish(lastQueue), "clFinish(release)");
        }

        void waitD3D() {
            if (!queue || !fence || !fenceEvent) return;
            dmlSharedCheckHr(queue->Signal(fence.Get(), ++fenceValue), "D3D12 Queue::Signal");
            const auto completed = fence->GetCompletedValue();
            if (completed == UINT64_MAX) throw std::runtime_error("共有推論のD3D12デバイスが失われました。");
            if (completed < fenceValue) {
                dmlSharedCheckHr(fence->SetEventOnCompletion(fenceValue, fenceEvent), "D3D12 Fence::SetEventOnCompletion");
                if (WaitForSingleObject(fenceEvent, INFINITE) != WAIT_OBJECT_0) throw std::runtime_error("共有推論のD3D12 fence待機に失敗しました。");
            }
        }

        void copyTensor(bool toTensor) {
            dmlSharedCheckHr(commandAllocator->Reset(), "D3D12 CommandAllocator::Reset");
            dmlSharedCheckHr(commandList->Reset(commandAllocator.Get(), nullptr), "D3D12 CommandList::Reset");
            auto barrier = [&](ID3D12Resource *resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
                D3D12_RESOURCE_BARRIER item = {};
                item.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                item.Transition.pResource = resource;
                item.Transition.StateBefore = before;
                item.Transition.StateAfter = after;
                item.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                commandList->ResourceBarrier(1, &item);
            };
            auto& current = buffers[toTensor ? 0 : 1];
            // textureはD3D11/OpenCLとの共有用、DEFAULT bufferはDirectMLのtensor専用とする。
            barrier(current.texture.Get(), D3D12_RESOURCE_STATE_COMMON,
                toTensor ? D3D12_RESOURCE_STATE_COPY_SOURCE : D3D12_RESOURCE_STATE_COPY_DEST);
            barrier(current.d3d12.Get(), toTensor ? D3D12_RESOURCE_STATE_COMMON : D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                toTensor ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COPY_SOURCE);
            D3D12_TEXTURE_COPY_LOCATION textureLocation = {};
            textureLocation.pResource = current.texture.Get();
            textureLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            D3D12_TEXTURE_COPY_LOCATION bufferLocation = {};
            bufferLocation.pResource = current.d3d12.Get();
            bufferLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            bufferLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R32_FLOAT;
            bufferLocation.PlacedFootprint.Footprint.Width = (UINT)imageWidth;
            bufferLocation.PlacedFootprint.Footprint.Height = (UINT)imageHeight;
            bufferLocation.PlacedFootprint.Footprint.Depth = 1;
            bufferLocation.PlacedFootprint.Footprint.RowPitch = (UINT)(imageWidth * sizeof(float));
            commandList->CopyTextureRegion(toTensor ? &bufferLocation : &textureLocation, 0, 0, 0,
                toTensor ? &textureLocation : &bufferLocation, nullptr);
            barrier(current.texture.Get(), toTensor ? D3D12_RESOURCE_STATE_COPY_SOURCE : D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_COMMON);
            barrier(current.d3d12.Get(), toTensor ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COPY_SOURCE,
                toTensor ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : D3D12_RESOURCE_STATE_COMMON);
            barrier(buffers[toTensor ? 1 : 0].d3d12.Get(),
                toTensor ? D3D12_RESOURCE_STATE_COMMON : D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                toTensor ? D3D12_RESOURCE_STATE_UNORDERED_ACCESS : D3D12_RESOURCE_STATE_COMMON);
            dmlSharedCheckHr(commandList->Close(), "D3D12 CommandList::Close");
            ID3D12CommandList *commands[] = { commandList.Get() };
            queue->ExecuteCommandLists(1, commands);
            // allocatorの再利用とAPI間の所有権移行に必要なGPU完了だけを待つ。
            waitD3D();
        }

        ~RGYDmlSharedState() {
            // フィルターの最後の並べ替えが未完了でも、共有資源を先に解放しない。
            if (lastQueue) {
                clFinish(lastQueue);
                if (acquired) {
                    const cl_mem objects[] = { buffers[0].image, buffers[1].image };
                    context->platform()->ext().clEnqueueReleaseD3D11ObjectsKHR(lastQueue, 2, objects, 0, nullptr, nullptr);
                    clFinish(lastQueue);
                }
            }
            try { waitD3D(); } catch (...) {}
            binding.reset();
            input.reset();
            output.reset();
            for (auto& buffer : buffers) {
                if (buffer.allocation && api) {
                    auto status = api->freeAllocation(buffer.allocation);
                    if (status) Ort::GetApi().ReleaseStatus(status);
                }
                if (buffer.opencl) clReleaseMemObject(buffer.opencl);
                if (buffer.image) clReleaseMemObject(buffer.image);
            }
            if (lastQueue) clReleaseCommandQueue(lastQueue);
            if (fenceEvent) CloseHandle(fenceEvent);
        }
    };
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
    std::unique_ptr<RGYDmlSharedState> shared;
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
        // 共有入出力を使った後でも、旧bindingを旧sessionより先に解放して通常経路へ初期化し直す。
        if (m_impl->shared) m_impl = std::make_unique<Impl>();
        auto &I = *m_impl;
        // create the ORT env / allocator now that the API is initialised
        if (!I.env)   I.env   = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "vceenc-onnx");
        if (!I.alloc) I.alloc = std::make_unique<Ort::AllocatorWithDefaultOptions>();

        const int deviceId = dxgiIndexForLuid(luidLow, luidHigh, I.adapterName);
        auto buildDmlOptions = [&](const GraphOptimizationLevel level, Ort::SessionOptions& opts) {
            // DirectMLでは逐次実行とメモリパターンの無効化が必要。
            opts.DisableMemPattern();
            opts.SetExecutionMode(ORT_SEQUENTIAL);
            opts.SetGraphOptimizationLevel(level);
            auto status = onnxRuntime().p_OrtSessionOptionsAppendExecutionProviderDML()(static_cast<OrtSessionOptions*>(opts), deviceId);
            if (status != nullptr) {
                errMessage = tstring(_T("AppendExecutionProvider_DML failed: "))
                           + char_to_tstring(Ort::GetApi().GetErrorMessage(status));
                Ort::GetApi().ReleaseStatus(status);
                return false;
            }
            return true;
        };

        Ort::SessionOptions opts;
        if (!buildDmlOptions(GraphOptimizationLevel::ORT_ENABLE_ALL, opts)) {
            return RGY_ERR_UNSUPPORTED;
        }
        try {
            I.session = std::make_unique<Ort::Session>(*I.env, modelPath.c_str(), opts);
        } catch (const Ort::Exception& allError) {
            // DirectMLの拡張グラフ最適化が一部のINT8 QDQモデルを壊す場合は、
            // 基本最適化へ下げてセッション生成を一度だけ再試行する。
            Ort::SessionOptions basicOpts;
            if (!buildDmlOptions(GraphOptimizationLevel::ORT_ENABLE_BASIC, basicOpts)) {
                return RGY_ERR_UNSUPPORTED;
            }
            try {
                I.session = std::make_unique<Ort::Session>(*I.env, modelPath.c_str(), basicOpts);
            } catch (const Ort::Exception& basicError) {
                errMessage = tstring(_T("DirectML session creation failed with all optimizations: "))
                    + char_to_tstring(allError.what())
                    + _T("; retry with basic optimizations failed: ")
                    + char_to_tstring(basicError.what());
                return RGY_ERR_UNKNOWN;
            }
        }

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


RGY_ERR RGYOnnxRTDML::initShared(const tstring &modelPath, uint32_t luidLow, int32_t luidHigh,
    std::shared_ptr<RGYOpenCLContext> context, int height, int width, tstring &errMessage) {
    loadOrtOnce();
    if (!s_ortReady) { errMessage = s_ortError; return RGY_ERR_UNSUPPORTED; }
    if (!context || !context->platform() || height <= 0 || width <= 0
        || (size_t)height > std::numeric_limits<size_t>::max() / sizeof(float) / 3 / (size_t)width) {
        errMessage = _T("共有GPU推論のcontextまたはRGB寸法が無効です。");
        return RGY_ERR_INVALID_PARAM;
    }
    try {
        // 通常のhost経路のsessionやallocatorを再利用せず、明示したD3D12 queueだけで実行する。
        m_impl = std::make_unique<Impl>();
        // 途中で失敗したsession/bufferを共有APIから再利用できる状態に残さない。
        struct InitRollback {
            Impl *state;
            bool completed = false;
            ~InitRollback() {
                if (!completed) {
                    state->shared.reset();
                    state->session.reset();
                    state->inC = state->inH = state->inW = 0;
                    state->outC = state->outH = state->outW = 0;
                }
            }
        } rollback{ m_impl.get() };
        auto& I = *m_impl;
        I.shared = std::make_unique<RGYDmlSharedState>();
        auto& S = *I.shared;
        S.context = context;
        const auto& extensions = context->platform()->ext();
        if (!context->platform()->d3d11dev() || !extensions.clCreateFromD3D11Texture2DKHR
            || !extensions.clEnqueueAcquireD3D11ObjectsKHR || !extensions.clEnqueueReleaseD3D11ObjectsKHR) {
            errMessage = _T("共有GPU推論には同一D3D11デバイスのOpenCL共有contextとcl_khr_d3d11_sharingが必要です。");
            return RGY_ERR_UNSUPPORTED;
        }
        // 既存OpenCL wrapperにない照会APIだけを局所取得し、追加のimport libraryを要求しない。
        const auto getContextInfo = reinterpret_cast<decltype(&::clGetContextInfo)>(
            RGYOpenCL::openCLHandle ? GetProcAddress(RGYOpenCL::openCLHandle, "clGetContextInfo") : nullptr);
        if (!getContextInfo) {
            errMessage = _T("OpenCL contextの共有設定を照会できません。");
            return RGY_ERR_UNSUPPORTED;
        }
        size_t propertyBytes = 0;
        dmlSharedCheckCl(getContextInfo(context->context(), CL_CONTEXT_PROPERTIES, 0, nullptr, &propertyBytes), "clGetContextInfo(size)");
        std::vector<cl_context_properties> properties(propertyBytes / sizeof(cl_context_properties));
        dmlSharedCheckCl(getContextInfo(context->context(), CL_CONTEXT_PROPERTIES, propertyBytes, properties.data(), nullptr), "clGetContextInfo(properties)");
        const void *contextD3D11 = nullptr;
        for (size_t i = 0; i + 1 < properties.size() && properties[i] != 0; i += 2) {
            if (properties[i] == CL_CONTEXT_D3D11_DEVICE_KHR) contextD3D11 = reinterpret_cast<const void *>(properties[i + 1]);
        }
        if (contextD3D11 != context->platform()->d3d11dev()) {
            errMessage = _T("OpenCL contextのD3D11共有デバイスが一致しません。");
            return RGY_ERR_UNSUPPORTED;
        }
        auto d3d11 = static_cast<ID3D11Device *>(const_cast<void *>(contextD3D11));
        ComPtr<ID3D11Device1> d3d11Device;
        dmlSharedCheckHr(d3d11->QueryInterface(IID_PPV_ARGS(&d3d11Device)), "D3D11 Device1");
        ComPtr<IDXGIDevice> dxgiDevice;
        ComPtr<IDXGIAdapter> adapter;
        dmlSharedCheckHr(d3d11->QueryInterface(IID_PPV_ARGS(&dxgiDevice)), "D3D11 DXGI device");
        dmlSharedCheckHr(dxgiDevice->GetAdapter(&adapter), "DXGI GetAdapter");
        DXGI_ADAPTER_DESC description = {};
        dmlSharedCheckHr(adapter->GetDesc(&description), "DXGI GetDesc");
        if (description.AdapterLuid.LowPart != luidLow || description.AdapterLuid.HighPart != luidHigh) {
            errMessage = _T("OpenCLとDirectMLに指定されたGPUのLUIDが一致しません。");
            return RGY_ERR_UNSUPPORTED;
        }
        I.adapterName = wstring_to_tstring(description.Description);
        auto& runtime = dmlSharedRuntime();
        if (!runtime.createD3D12 || !runtime.createDML) {
            errMessage = _T("共有GPU推論に必要なD3D12またはDirectMLのAPIを読み込めません。");
            return RGY_ERR_UNSUPPORTED;
        }
        const void *dmlApi = nullptr;
        Ort::ThrowOnError(Ort::GetApi().GetExecutionProviderApi("DML", ORT_API_VERSION, &dmlApi));
        S.api = reinterpret_cast<const RGYOrtDmlSharedApi *>(dmlApi);
        if (!S.api || !S.api->appendWithQueue || !S.api->createAllocation || !S.api->freeAllocation) {
            errMessage = _T("ONNX RuntimeがDirectMLの共有GPU入出力APIに対応していません。");
            return RGY_ERR_UNSUPPORTED;
        }
        dmlSharedCheckHr(runtime.createD3D12(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&S.device)), "D3D12CreateDevice");
        D3D12_COMMAND_QUEUE_DESC queueDescription = {};
        queueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        dmlSharedCheckHr(S.device->CreateCommandQueue(&queueDescription, IID_PPV_ARGS(&S.queue)), "D3D12 CreateCommandQueue");
        dmlSharedCheckHr(runtime.createDML(S.device.Get(), DML_CREATE_DEVICE_FLAG_NONE, IID_PPV_ARGS(&S.dml)), "DMLCreateDevice");
        dmlSharedCheckHr(S.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&S.commandAllocator)), "D3D12 CreateCommandAllocator");
        dmlSharedCheckHr(S.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, S.commandAllocator.Get(), nullptr, IID_PPV_ARGS(&S.commandList)), "D3D12 CreateCommandList");
        dmlSharedCheckHr(S.commandList->Close(), "D3D12 CommandList::Close(initial)");
        dmlSharedCheckHr(S.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&S.fence)), "D3D12 CreateFence");
        S.fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
        if (!S.fenceEvent) throw std::runtime_error("共有推論のD3D12 fenceイベントを作成できません。");

        I.env = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "vceenc-onnx-shared");
        I.alloc = std::make_unique<Ort::AllocatorWithDefaultOptions>();
        Ort::SessionOptions options;
        options.DisableMemPattern();
        options.SetExecutionMode(ORT_SEQUENTIAL);
        options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        // 共有GPU必須の経路では、未対応演算をCPUで代行するsessionも作らない。
        options.AddConfigEntry("session.disable_cpu_ep_fallback", "1");
        Ort::ThrowOnError(S.api->appendWithQueue(options, S.dml.Get(), S.queue.Get()));
        I.session = std::make_unique<Ort::Session>(*I.env, modelPath.c_str(), options);
        if (I.session->GetInputCount() != 1 || I.session->GetOutputCount() != 1) {
            errMessage = _T("共有GPU推論はRGBの1入力1出力モデルだけに対応します。");
            return RGY_ERR_UNSUPPORTED;
        }
        const std::vector<int64_t> dimensions = { 1, 3, height, width };
        for (int output = 0; output < 2; output++) {
            auto info = output ? I.session->GetOutputTypeInfo(0) : I.session->GetInputTypeInfo(0);
            auto tensor = info.GetTensorTypeAndShapeInfo();
            const auto shape = tensor.GetShape();
            if (tensor.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT || shape.size() != dimensions.size()) {
                errMessage = _T("共有GPU推論はFP32・NCHWのRGB等倍モデルだけに対応します。");
                return RGY_ERR_UNSUPPORTED;
            }
            for (size_t i = 0; i < shape.size(); i++) {
                if (shape[i] > 0 && shape[i] != dimensions[i]) {
                    errMessage = _T("共有GPU推論のRGBモデル入出力寸法が指定寸法と一致しません。");
                    return RGY_ERR_UNSUPPORTED;
                }
            }
        }
        auto inputName = I.session->GetInputNameAllocated(0, *I.alloc);
        auto outputName = I.session->GetOutputNameAllocated(0, *I.alloc);
        I.inName = inputName.get();
        I.outName = outputName.get();
        I.inC = I.outC = 3;
        I.inH = I.outH = height;
        I.inW = I.outW = width;
        const size_t bytes = (size_t)3 * height * width * sizeof(float);
        const size_t elements = bytes / sizeof(float);
        // CopyTextureRegionのrow pitchを256byte境界へ揃え、CHWを連続したfloat列として保存する。
        S.imageWidth = std::min<size_t>(D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION, std::max<size_t>(256, ((size_t)width + 63) & ~(size_t)63));
        S.imageHeight = (elements + S.imageWidth - 1) / S.imageWidth;
        if (S.imageHeight > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION) {
            errMessage = _T("共有GPU推論の画像要素数がD3D11共有textureの上限を超えています。");
            return RGY_ERR_UNSUPPORTED;
        }
        const size_t capacityBytes = S.imageWidth * S.imageHeight * sizeof(float);
        D3D11_TEXTURE2D_DESC texture = {};
        texture.Width = (UINT)S.imageWidth;
        texture.Height = (UINT)S.imageHeight;
        texture.MipLevels = 1;
        texture.ArraySize = 1;
        texture.Format = DXGI_FORMAT_R32_FLOAT;
        texture.SampleDesc.Count = 1;
        texture.Usage = D3D11_USAGE_DEFAULT;
        texture.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
        // NTHANDLEだけでは共有資源にならない。D3D11起点のtextureをD3D12へ開く。
        texture.MiscFlags = D3D11_RESOURCE_MISC_SHARED | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
        D3D12_HEAP_PROPERTIES heap = {};
        heap.Type = D3D12_HEAP_TYPE_DEFAULT;
        D3D12_RESOURCE_DESC tensorResource = {};
        tensorResource.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        tensorResource.Width = capacityBytes;
        tensorResource.Height = 1;
        tensorResource.DepthOrArraySize = 1;
        tensorResource.MipLevels = 1;
        tensorResource.SampleDesc.Count = 1;
        tensorResource.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        tensorResource.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        for (auto& buffer : S.buffers) {
            dmlSharedCheckHr(d3d11Device->CreateTexture2D(&texture, nullptr, &buffer.d3d11), "D3D11 CreateTexture2D(shared)");
            ComPtr<IDXGIResource1> sharedResource;
            dmlSharedCheckHr(buffer.d3d11.As(&sharedResource), "D3D11 IDXGIResource1");
            HANDLE handle = nullptr;
            dmlSharedCheckHr(sharedResource->CreateSharedHandle(nullptr, DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE,
                nullptr, &handle), "DXGI CreateSharedHandle");
            const auto openResult = S.device->OpenSharedHandle(handle, IID_PPV_ARGS(&buffer.texture));
            CloseHandle(handle);
            dmlSharedCheckHr(openResult, "D3D12 OpenSharedHandle(texture)");
            dmlSharedCheckHr(S.device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &tensorResource,
                D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer.d3d12)), "D3D12 CreateCommittedResource(tensor)");
            cl_int clStatus = CL_SUCCESS;
            buffer.image = extensions.clCreateFromD3D11Texture2DKHR(context->context(), CL_MEM_READ_WRITE,
                buffer.d3d11.Get(), 0, &clStatus);
            dmlSharedCheckCl(clStatus, "clCreateFromD3D11Texture2DKHR");
            if (!buffer.image) throw std::runtime_error("共有OpenCL imageの作成結果がnullです。");
            buffer.opencl = clCreateBuffer(context->context(), CL_MEM_READ_WRITE, capacityBytes, nullptr, &clStatus);
            dmlSharedCheckCl(clStatus, "clCreateBuffer(shared bridge)");
            if (!buffer.opencl) throw std::runtime_error("共有経路のOpenCL bufferの作成結果がnullです。");
            Ort::ThrowOnError(S.api->createAllocation(buffer.d3d12.Get(), &buffer.allocation));
        }
        Ort::MemoryInfo gpuMemory("DML", OrtDeviceAllocator, 0, OrtMemTypeDefault);
        S.input = std::make_unique<Ort::Value>(Ort::Value::CreateTensor(gpuMemory, S.buffers[0].allocation, bytes,
            dimensions.data(), dimensions.size(), ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT));
        S.output = std::make_unique<Ort::Value>(Ort::Value::CreateTensor(gpuMemory, S.buffers[1].allocation, bytes,
            dimensions.data(), dimensions.size(), ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT));
        S.binding = std::make_unique<Ort::IoBinding>(*I.session);
        S.binding->BindInput(I.inName.c_str(), *S.input);
        S.binding->BindOutput(I.outName.c_str(), *S.output);
        S.useQueue(context->queue().get());
        S.acquire();
        const float zero = 0.0f;
        dmlSharedCheckCl(clEnqueueFillBuffer(S.lastQueue, S.inputMem(), &zero, sizeof(zero), 0, capacityBytes, 0, nullptr, nullptr), "clEnqueueFillBuffer(warmup)");
        // 出力shapeも固定RGB契約でbindし、GPU上のゼロ入力だけで最初の推論を検証する。
        const auto result = inferShared(context->queue(), errMessage);
        if (result != RGY_ERR_NONE) return result;
        rollback.completed = true;
    } catch (const Ort::Exception& e) {
        errMessage = char_to_tstring(e.what());
        return RGY_ERR_UNSUPPORTED;
    } catch (const std::exception& e) {
        errMessage = char_to_tstring(e.what());
        return RGY_ERR_UNSUPPORTED;
    }
    return RGY_ERR_NONE;
}

void *RGYOnnxRTDML::sharedInput() const {
    return m_impl && m_impl->shared ? m_impl->shared->inputMem() : nullptr;
}

void *RGYOnnxRTDML::sharedOutput() const {
    return m_impl && m_impl->shared ? m_impl->shared->outputMem() : nullptr;
}

RGY_ERR RGYOnnxRTDML::inferShared(RGYOpenCLQueue& queue, tstring& errMessage) {
    if (!m_impl || !m_impl->session || !m_impl->shared || !m_impl->shared->binding) return RGY_ERR_NOT_INITIALIZED;
    try {
        auto& I = *m_impl;
        auto& S = *I.shared;
        S.useQueue(queue.get());
        if (!S.acquired) throw std::runtime_error("共有GPU bufferのOpenCL所有権が失われています。");
        const size_t origin[] = { 0, 0, 0 };
        const size_t region[] = { S.imageWidth, S.imageHeight, 1 };
        dmlSharedCheckCl(clEnqueueCopyBufferToImage(S.lastQueue, S.inputMem(), S.buffers[0].image,
            0, origin, region, 0, nullptr, nullptr), "clEnqueueCopyBufferToImage");
        S.release();
        S.copyTensor(true);
        I.session->Run(Ort::RunOptions{ nullptr }, *S.binding);
        S.copyTensor(false);
        S.acquire();
        dmlSharedCheckCl(clEnqueueCopyImageToBuffer(S.lastQueue, S.buffers[1].image, S.outputMem(),
            origin, region, 0, 0, nullptr, nullptr), "clEnqueueCopyImageToBuffer");
        dmlSharedCheckCl(clFinish(S.lastQueue), "clFinish(output bridge)");
    } catch (const Ort::Exception& e) {
        errMessage = char_to_tstring(e.what());
        return RGY_ERR_UNKNOWN;
    } catch (const std::exception& e) {
        errMessage = char_to_tstring(e.what());
        return RGY_ERR_UNKNOWN;
    }
    return RGY_ERR_NONE;
}

RGY_ERR RGYOnnxRTDML::infer(const float *in, float *out) {
    if (!m_impl->session) return RGY_ERR_NULL_PTR;
    if (m_impl->shared) return RGY_ERR_UNSUPPORTED; // 共有初期化後にhost入出力へ切り替えない。
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
        const int deviceId = dxgiIndexForLuid(luidLow, luidHigh, I.adapterName);
        auto buildDmlOptions = [&](const GraphOptimizationLevel level, Ort::SessionOptions& opts) {
            opts.DisableMemPattern();
            opts.SetExecutionMode(ORT_SEQUENTIAL);
            opts.SetGraphOptimizationLevel(level);
            auto status = onnxRuntime().p_OrtSessionOptionsAppendExecutionProviderDML()(static_cast<OrtSessionOptions*>(opts), deviceId);
            if (status != nullptr) {
                errMessage = tstring(_T("AppendExecutionProvider_DML failed: ")) + char_to_tstring(Ort::GetApi().GetErrorMessage(status));
                Ort::GetApi().ReleaseStatus(status);
                return false;
            }
            return true;
        };
        Ort::SessionOptions opts;
        if (!buildDmlOptions(GraphOptimizationLevel::ORT_ENABLE_ALL, opts)) {
            return RGY_ERR_UNSUPPORTED;
        }
        try {
            I.session = std::make_unique<Ort::Session>(*I.env, modelPath.c_str(), opts);
        } catch (const Ort::Exception& allError) {
            // 複数入出力モデルでも同じ条件で基本最適化へフォールバックする。
            Ort::SessionOptions basicOpts;
            if (!buildDmlOptions(GraphOptimizationLevel::ORT_ENABLE_BASIC, basicOpts)) {
                return RGY_ERR_UNSUPPORTED;
            }
            try {
                I.session = std::make_unique<Ort::Session>(*I.env, modelPath.c_str(), basicOpts);
            } catch (const Ort::Exception& basicError) {
                errMessage = tstring(_T("DirectML session creation failed with all optimizations: "))
                    + char_to_tstring(allError.what())
                    + _T("; retry with basic optimizations failed: ")
                    + char_to_tstring(basicError.what());
                return RGY_ERR_UNKNOWN;
            }
        }
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
RGY_ERR RGYOnnxRTDML::initShared(const tstring &, uint32_t, int32_t,
    std::shared_ptr<RGYOpenCLContext>, int, int, tstring &errMessage) {
    errMessage = _T("このビルドではDirectMLの共有GPU推論を使用できません。");
    return RGY_ERR_UNSUPPORTED;
}
void *RGYOnnxRTDML::sharedInput() const { return nullptr; }
void *RGYOnnxRTDML::sharedOutput() const { return nullptr; }
RGY_ERR RGYOnnxRTDML::inferShared(RGYOpenCLQueue &, tstring &) { return RGY_ERR_UNSUPPORTED; }
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
