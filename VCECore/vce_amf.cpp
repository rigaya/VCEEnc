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
// IABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// ------------------------------------------------------------------------------------------

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>
#include "vce_amf.h"
#include "vce_util.h"
#include "rgy_filesystem.h"
#if defined(_WIN32) || defined(_WIN64)
#include <atlbase.h>
#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")
#endif

#if defined(_WIN32) || defined(_WIN64)
// 指定DXGIアダプタ(論理AMDアダプタ番号)に対応するDriverStoreディレクトリを取得する
static tstring getAMFDriverStoreDirForAdapter(int logicalAdapterId) {
    const auto& adapterIndexes = DX11AdapterManager::getInstance(nullptr)->getAdapterIndexes();
    if (logicalAdapterId < 0 || logicalAdapterId >= (int)adapterIndexes.size()) {
        return tstring();
    }
    ATL::CComPtr<IDXGIFactory> pFactory;
    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pFactory))) {
        return tstring();
    }
    ATL::CComPtr<IDXGIAdapter> pAdapter;
    if (pFactory->EnumAdapters(adapterIndexes[logicalAdapterId], &pAdapter) == DXGI_ERROR_NOT_FOUND) {
        return tstring();
    }
    DXGI_ADAPTER_DESC targetDesc = {};
    if (FAILED(pAdapter->GetDesc(&targetDesc))) {
        return tstring();
    }

    const auto deviceIdToken = strsprintf(_T("DEV_%04X"), targetDesc.DeviceId);
    const TCHAR *kDisplayClass = _T("SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}");
    HKEY hClassKey = nullptr;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, kDisplayClass, 0, KEY_READ, &hClassKey) != ERROR_SUCCESS) {
        return tstring();
    }

    tstring driverStoreDir;
    for (DWORD idx = 0; ; idx++) {
        TCHAR subKeyName[256] = {};
        DWORD subKeyNameLen = _countof(subKeyName);
        if (RegEnumKeyEx(hClassKey, idx, subKeyName, &subKeyNameLen, nullptr, nullptr, nullptr, nullptr) != ERROR_SUCCESS) {
            break;
        }
        HKEY hAdapterKey = nullptr;
        if (RegOpenKeyEx(hClassKey, subKeyName, 0, KEY_READ, &hAdapterKey) != ERROR_SUCCESS) {
            continue;
        }
        TCHAR matchingId[256] = {};
        DWORD matchingIdSize = sizeof(matchingId);
        DWORD typ = 0;
        const auto regRet = RegQueryValueEx(hAdapterKey, _T("MatchingDeviceId"), nullptr, &typ, (LPBYTE)matchingId, &matchingIdSize);
        if (regRet != ERROR_SUCCESS || (typ != REG_SZ && typ != REG_MULTI_SZ)) {
            RegCloseKey(hAdapterKey);
            continue;
        }
        const tstring matchingIdStr(matchingId);
        if (matchingIdStr.find(_T("VEN_1002")) == tstring::npos
            || matchingIdStr.find(deviceIdToken) == tstring::npos) {
            RegCloseKey(hAdapterKey);
            continue;
        }

        // OpenGLDriverName / UserModeDriverName からDriverStoreパスを得る
        auto tryExtractDir = [](HKEY hKey, const TCHAR *valueName) -> tstring {
            DWORD typ2 = 0;
            DWORD size = 0;
            if (RegQueryValueEx(hKey, valueName, nullptr, &typ2, nullptr, &size) != ERROR_SUCCESS || size == 0) {
                return tstring();
            }
            std::vector<TCHAR> buf(size / sizeof(TCHAR) + 1, _T('\0'));
            if (RegQueryValueEx(hKey, valueName, nullptr, &typ2, (LPBYTE)buf.data(), &size) != ERROR_SUCCESS) {
                return tstring();
            }
            tstring path = buf.data();
            // REG_MULTI_SZや "{path}" 形式に対応
            path = str_replace(path, _T("{"), _T(""));
            path = str_replace(path, _T("}"), _T(""));
            // 複数パスがスペース区切りの場合は先頭を使う
            const auto spacePos = path.find(_T(' '));
            if (spacePos != tstring::npos) {
                path = path.substr(0, spacePos);
            }
            if (path.empty()) {
                return tstring();
            }
            return PathRemoveFileSpecFixed(path).second;
        };

        driverStoreDir = tryExtractDir(hAdapterKey, _T("OpenGLDriverName"));
        if (driverStoreDir.empty()) {
            driverStoreDir = tryExtractDir(hAdapterKey, _T("UserModeDriverName"));
        }
        RegCloseKey(hAdapterKey);
        if (!driverStoreDir.empty()) {
            break;
        }
    }
    RegCloseKey(hClassKey);
    return driverStoreDir;
}

static tstring resolveAMFRuntimeDllPath(int deviceId) {
    // 対象アダプタが決まっている場合はそのDriverStoreを優先
    std::vector<int> adaptersToTry;
    if (deviceId >= 0) {
        adaptersToTry.push_back(deviceId);
    } else {
        // 自動選択時は全AMDアダプタを新しい順に試し、amfrtがあるものを選ぶ
        const int count = DeviceDX11::adapterCount(nullptr);
        for (int i = count - 1; i >= 0; i--) {
            adaptersToTry.push_back(i);
        }
    }
    for (const int adapterId : adaptersToTry) {
        const auto dir = getAMFDriverStoreDirForAdapter(adapterId);
        if (dir.empty()) {
            continue;
        }
        const auto dllPath = dir + _T("\\") + wstring_to_tstring(AMF_DLL_NAME);
        if (rgy_file_exists(dllPath)) {
            return dllPath;
        }
    }
    return wstring_to_tstring(AMF_DLL_NAME);
}
#endif // Windows


void VCEAMF::PrintMes(RGYLogLevel log_level, const TCHAR *format, ...) {
    if (m_pLog.get() == nullptr || log_level < m_pLog->getLogLevel(RGY_LOGT_CORE)) {
        return;
    }

    va_list args;
    va_start(args, format);

    int len = _vsctprintf(format, args) + 1; // _vscprintf doesn't count terminating '\0'
    vector<TCHAR> buffer(len, 0);
    _vstprintf_s(buffer.data(), len, format, args);
    va_end(args);

    m_pLog->write(log_level, RGY_LOGT_CORE, buffer.data());
}

VCEAMF::VCEAMF() :
    m_pLog(),
    m_dll(),
    m_pFactory(nullptr),
    m_pDebug(nullptr),
    m_pTrace(nullptr),
    m_tracer(),
    m_AMFRuntimeVersion(0) {
}

VCEAMF::~VCEAMF() {
    Terminate();
}

RGY_ERR VCEAMF::initLogLevel(RGYLogLevel loglevel) {
    m_pLog.reset(new RGYLog(nullptr, loglevel));
    return RGY_ERR_NONE;
}

RGY_ERR VCEAMF::initLogLevel(const RGYParamLogLevel& loglevel) {
    m_pLog.reset(new RGYLog(nullptr, loglevel));
    return RGY_ERR_NONE;
}

void VCEAMF::Terminate() {
    m_tracer.reset();

    PrintMes(RGY_LOG_DEBUG, _T("Closing logger...\n"));
    m_pLog.reset();
}

RGY_ERR VCEAMF::initAMFFactory(int deviceId) {
#if defined(_WIN32) || defined(_WIN64)
    // 複数世代のAMDドライバが同居していると、System32のamfrt64が
    // 古い方のamfrtdrv64を掴んでしまい新しいdGPUでエンコーダ作成に失敗する。
    // 対象GPUのDriverStore上のamfrt64をLOAD_WITH_ALTERED_SEARCH_PATHで読む。
    const auto amfDllPath = resolveAMFRuntimeDllPath(deviceId);
    PrintMes(RGY_LOG_DEBUG, _T("Loading AMF runtime: %s (deviceId=%d)\n"), amfDllPath.c_str(), deviceId);
    m_dll = std::unique_ptr<std::remove_pointer_t<HMODULE>, module_deleter>(
        LoadLibraryEx(amfDllPath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH));
    if (!m_dll) {
        // フォールバック: 通常の検索パス
        m_dll = std::unique_ptr<std::remove_pointer_t<HMODULE>, module_deleter>(RGY_LOAD_LIBRARY(wstring_to_tstring(AMF_DLL_NAME).c_str()));
    }
#else
    (void)deviceId;
    m_dll = std::unique_ptr<std::remove_pointer_t<HMODULE>, module_deleter>(RGY_LOAD_LIBRARY(wstring_to_tstring(AMF_DLL_NAME).c_str()));
#endif
    if (!m_dll) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to load %s.\n"), wstring_to_tstring(AMF_DLL_NAME).c_str());
        return RGY_ERR_NOT_FOUND;
    }
    AMFInit_Fn initFun = (AMFInit_Fn)RGY_GET_PROC_ADDRESS(m_dll.get(), AMF_INIT_FUNCTION_NAME);
    if (initFun == NULL) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to load %s.\n"), AMF_INIT_FUNCTION_NAME);
        return RGY_ERR_NOT_FOUND;
    }
    AMF_RESULT res = initFun(AMF_FULL_VERSION, &m_pFactory);
    if (res != AMF_OK) {
        PrintMes(RGY_LOG_ERROR, _T("Failed AMFInit: %s.\n"), AMFRetString(res));
        return err_to_rgy(res);
    }
    AMFQueryVersion_Fn versionFun = (AMFQueryVersion_Fn)RGY_GET_PROC_ADDRESS(m_dll.get(), AMF_QUERY_VERSION_FUNCTION_NAME);
    if (versionFun == NULL) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to load %s.\n"), AMF_QUERY_VERSION_FUNCTION_NAME);
        return RGY_ERR_NOT_FOUND;
    }
    res = versionFun(&m_AMFRuntimeVersion);
    if (res != AMF_OK) {
        return err_to_rgy(res);
    }
    m_pFactory->GetTrace(&m_pTrace);
    m_pFactory->GetDebug(&m_pDebug);
    PrintMes(RGY_LOG_DEBUG, _T("Loaded %s: ver %d.%d.%d.\n"),
        wstring_to_tstring(AMF_DLL_NAME).c_str(),
        (int)AMF_GET_MAJOR_VERSION(m_AMFRuntimeVersion), (int)AMF_GET_MINOR_VERSION(m_AMFRuntimeVersion), (int)AMF_GET_SUBMINOR_VERSION(m_AMFRuntimeVersion));
    return RGY_ERR_NONE;
}

RGY_ERR VCEAMF::initTracer(int log_level) {
    m_pTrace->EnableWriter(AMF_TRACE_WRITER_DEBUG_OUTPUT, log_level < RGY_LOG_INFO);
    if (log_level < RGY_LOG_INFO)
        m_pTrace->SetWriterLevel(AMF_TRACE_WRITER_DEBUG_OUTPUT, loglevel_rgy_to_enc(log_level));
    m_pTrace->EnableWriter(AMF_TRACE_WRITER_CONSOLE, false);
    m_pTrace->SetGlobalLevel(loglevel_rgy_to_enc(log_level));

    m_tracer.init(m_pLog);
    m_pTrace->RegisterWriter(L"RGYLOGWriter", &m_tracer, log_level < RGY_LOG_INFO);
    m_pTrace->SetWriterLevel(L"RGYLOGWriter", loglevel_rgy_to_enc(log_level));
    return RGY_ERR_NONE;
}

std::vector<std::unique_ptr<VCEDevice>> VCEAMF::createDeviceList(bool interopD3d9, bool interopD3d11, RGYParamInitVulkan interopVulkan, bool enableOpenCL, bool enableVppPerfMonitor, bool enableAV1HWDec, int openCLBuildThreads, int targetDeviceId, const tstring& clPerfDumpDir, const double clPerfTimelineSec) {
    std::vector<std::unique_ptr<VCEDevice>> devs;
    int adapterCount = 0;
#if ENABLE_D3D11
    adapterCount = DeviceDX11::adapterCount(m_pLog.get());
#elif ENABLE_VULKAN
    std::vector<int> adapterIndices;
    if (VULKAN_DEFAULT_DEVICE_ONLY == 0) {
        auto devVk = std::make_unique<DeviceVulkan>();
        const auto adapters = devVk->adapterList();
        if (interopVulkan == RGYParamInitVulkan::TargetVendor) {
            for (int i = 0; i < (int)adapters.size(); i++) {
                if (adapters[i].vendorID == 0x1002) {
                    adapterIndices.push_back(i);
                }
            }
        } else {
            for (int i = 0; i < (int)adapters.size(); i++) {
                adapterIndices.push_back(i);
            }
        }
        devVk.reset(); // VCEDevice::init()を呼ぶ前に開放しないとなぜか処理がうまく進まない
    } else {
        adapterIndices.push_back(0);
    }
    if (targetDeviceId >= 0) {
        adapterIndices.erase(
            std::remove_if(adapterIndices.begin(), adapterIndices.end(), [targetDeviceId](const int adapterIndex) {
                return adapterIndex != targetDeviceId;
            }),
            adapterIndices.end());
    }
    adapterCount = (int)adapterIndices.size();
#else
    RGYOpenCL cl(m_pLog);
    auto platforms = cl.getPlatforms("AMD");
    adapterCount = std::accumulate(platforms.begin(), platforms.end(), 0, [](int acc, std::shared_ptr<RGYOpenCLPlatform>& p) {
        if (p->createDeviceList(CL_DEVICE_TYPE_GPU) == RGY_ERR_NONE) {
            acc += (int)p->devs().size();
        }
        return acc;
    });
#endif
    PrintMes(RGY_LOG_DEBUG, _T("adapterCount %d.\n"), adapterCount);

    std::vector<int> adaptersToInit;
#if ENABLE_VULKAN && !ENABLE_D3D11
    adaptersToInit = adapterIndices;
#else
    for (int i = 0; i < adapterCount; i++) {
        if (targetDeviceId >= 0 && i != targetDeviceId) {
            continue;
        }
        adaptersToInit.push_back(i);
    }
#endif
    // 複数GPUを同時に初期化すると、古いiGPUドライバのAMFコンポーネントが先に
    // ロードされ、新しいdGPUでエンコーダ作成に失敗することがある。
    // 新しいアダプタから初期化することで、新しいドライバ側を優先する。
    if (adaptersToInit.size() > 1) {
        std::reverse(adaptersToInit.begin(), adaptersToInit.end());
    }

    for (const int adapterIndex : adaptersToInit) {
        auto dev = std::make_unique<VCEDevice>(m_pLog, m_pFactory, m_pTrace);
        PrintMes(RGY_LOG_DEBUG, _T("Init adaptor #%d.\n"), adapterIndex);
        if (dev->init(adapterIndex, interopD3d9, interopD3d11, interopVulkan, enableOpenCL, enableVppPerfMonitor, enableAV1HWDec, openCLBuildThreads, clPerfDumpDir, clPerfTimelineSec) == RGY_ERR_NONE) {
            devs.push_back(std::move(dev));
        }
    }
    // デバイスID順に戻す（新しいアダプタから初期化したため）
    std::sort(devs.begin(), devs.end(), [](const std::unique_ptr<VCEDevice>& a, const std::unique_ptr<VCEDevice>& b) {
        return a->id() < b->id();
    });
    return devs;
}
