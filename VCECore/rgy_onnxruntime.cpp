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
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// ------------------------------------------------------------------------------------------

#include "rgy_onnxruntime.h"

#if ENABLE_ONNXRUNTIME

#include <vector>

#include "rgy_util.h"

#if defined(_WIN32) || defined(_WIN64)
const TCHAR *RGY_ONNXRUNTIME_DLL_NAME = _T("onnxruntime.dll");
#else
const TCHAR *RGY_ONNXRUNTIME_DLL_NAME = _T("libonnxruntime.so");
#endif

#if defined(_WIN32) || defined(_WIN64)
static HMODULE loadOnnxRuntimeLibrary() {
    std::vector<tstring> dirs;
    std::vector<TCHAR> pathBuf(MAX_PATH + 1, 0);
    const auto exePathLen = GetModuleFileName(nullptr, pathBuf.data(), (DWORD)pathBuf.size());
    if (exePathLen > 0 && exePathLen < pathBuf.size()) {
        tstring exePath(pathBuf.data());
        const auto pos = exePath.find_last_of(_T("\\/"));
        if (pos != tstring::npos) {
            dirs.push_back(exePath.substr(0, pos));
        }
    }

    const auto envLen = GetEnvironmentVariable(_T("PATH"), nullptr, 0);
    if (envLen > 0) {
        std::vector<TCHAR> envPath(envLen, 0);
        if (GetEnvironmentVariable(_T("PATH"), envPath.data(), envLen) > 0) {
            for (const auto& dir : split(tstring(envPath.data()), _T(";"))) {
                if (dir.length() > 0) {
                    dirs.push_back(dir);
                }
            }
        }
    }

    for (auto dir : dirs) {
        if (dir.back() != _T('\\') && dir.back() != _T('/')) {
            dir += _T("\\");
        }
        const auto dllPath = dir + RGY_ONNXRUNTIME_DLL_NAME;
        if (GetFileAttributes(dllPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
            if (auto hModule = LoadLibrary(dllPath.c_str()); hModule != nullptr) {
                return hModule;
            }
        }
    }
    return LoadLibrary(RGY_ONNXRUNTIME_DLL_NAME);
}
#else
static HMODULE loadOnnxRuntimeLibrary() {
    return RGY_LOAD_LIBRARY(RGY_ONNXRUNTIME_DLL_NAME);
}
#endif

RGYOnnxRuntimeLoader::RGYOnnxRuntimeLoader() :
    m_hModule(nullptr),
    m_loaded(false),
    m_errMessage(),
    m_OrtGetApiBase(nullptr),
    m_OrtSessionOptionsAppendExecutionProviderDML(nullptr),
    m_OrtDmlApi(nullptr) {
}

RGYOnnxRuntimeLoader::~RGYOnnxRuntimeLoader() {
    close();
}

bool RGYOnnxRuntimeLoader::load() {
    if (m_loaded) {
        return true;
    }
    m_errMessage.clear();

    if ((m_hModule = loadOnnxRuntimeLibrary()) == nullptr) {
        m_errMessage = strsprintf(_T("could not load %s (a DirectML-enabled ONNX Runtime). ")
                                  _T("place it and DirectML.dll next to the executable or in the library search path."),
                                  RGY_ONNXRUNTIME_DLL_NAME);
        return false;
    }

    auto loadFunc = [this](const char *funcName, void **func) {
        if ((*func = RGY_GET_PROC_ADDRESS(m_hModule, funcName)) == nullptr) {
            m_errMessage = strsprintf(_T("%s is missing %s (not a compatible ONNX Runtime library?)."),
                                      RGY_ONNXRUNTIME_DLL_NAME, char_to_tstring(funcName).c_str());
            close();
            return false;
        }
        return true;
    };

    if (!loadFunc("OrtGetApiBase", (void **)&m_OrtGetApiBase)) {
        return false;
    }

    const OrtApi *api = nullptr;
    for (int v = ORT_API_VERSION; v >= 11; --v) {
        api = m_OrtGetApiBase()->GetApi((uint32_t)v);
        if (api) {
            break;
        }
    }
    if (!api) {
        m_errMessage = strsprintf(_T("%s is too old (no compatible ONNX Runtime API version)."),
                                  RGY_ONNXRUNTIME_DLL_NAME);
        close();
        return false;
    }
    Ort::InitApi(api);

    m_OrtSessionOptionsAppendExecutionProviderDML = (PFN_OrtSessionOptionsAppendExecutionProviderDML)RGY_GET_PROC_ADDRESS(m_hModule, "OrtSessionOptionsAppendExecutionProvider_DML");
    if (m_OrtSessionOptionsAppendExecutionProviderDML == nullptr) {
        const void *dmlApi = nullptr;
        if (auto st = api->GetExecutionProviderApi("DML", ORT_API_VERSION, &dmlApi); st != nullptr) {
            m_errMessage = tstring(_T("could not get ONNX Runtime DirectML API: "))
                         + char_to_tstring(api->GetErrorMessage(st));
            api->ReleaseStatus(st);
            close();
            return false;
        }
        m_OrtDmlApi = reinterpret_cast<const RGYOrtDmlApi *>(dmlApi);
        if (m_OrtDmlApi == nullptr || m_OrtDmlApi->SessionOptionsAppendExecutionProvider_DML == nullptr) {
            m_errMessage = strsprintf(_T("%s does not provide DirectML execution provider API."),
                                      RGY_ONNXRUNTIME_DLL_NAME);
            close();
            return false;
        }
        m_OrtSessionOptionsAppendExecutionProviderDML = m_OrtDmlApi->SessionOptionsAppendExecutionProvider_DML;
    }

    m_loaded = true;
    return true;
}

void RGYOnnxRuntimeLoader::close() {
    if (m_hModule) {
        RGY_FREE_LIBRARY(m_hModule);
        m_hModule = nullptr;
    }
    m_loaded = false;
    m_OrtGetApiBase = nullptr;
    m_OrtSessionOptionsAppendExecutionProviderDML = nullptr;
    m_OrtDmlApi = nullptr;
}

#endif // ENABLE_ONNXRUNTIME
