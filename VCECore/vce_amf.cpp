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

#include <cmath>
#include <numeric>
#include "vce_amf.h"
#include "vce_util.h"

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

RGY_ERR VCEAMF::initAMFFactory() {
    m_dll = std::unique_ptr<std::remove_pointer_t<HMODULE>, module_deleter>(RGY_LOAD_LIBRARY(wstring_to_tstring(AMF_DLL_NAME).c_str()));
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

std::vector<std::unique_ptr<VCEDevice>> VCEAMF::createDeviceList(bool interopD3d9, bool interopD3d11, RGYParamInitVulkan interopVulkan, bool enableOpenCL, bool enableVppPerfMonitor, bool enableAV1HWDec) {
    std::vector<std::unique_ptr<VCEDevice>> devs;
#if ENABLE_D3D11
    const int adapterCount = DeviceDX11::adapterCount();
#elif ENABLE_VULKAN
#if !(defined(_WIN32) || defined(_WIN64))
    if (interopVulkan == RGYParamInitVulkan::TargetVendor) {
        setenv("VK_LOADER_DRIVERS_SELECT", "*amd*", 1);
    }
#endif // #if !(defined(_WIN32) || defined(_WIN64))
    int adapterCount = 1;
    if (VULKAN_DEFAULT_DEVICE_ONLY == 0) {
        auto devVk = std::make_unique<DeviceVulkan>();
        adapterCount = devVk->adapterCount();
        devVk.reset(); // VCEDevice::init()を呼ぶ前に開放しないとなぜか処理がうまく進まない
    }
#else
    RGYOpenCL cl(m_pLog);
    auto platforms = cl.getPlatforms("AMD");
    const int adapterCount = std::accumulate(platforms.begin(), platforms.end(), 0, [](int acc, std::shared_ptr<RGYOpenCLPlatform>& p) {
        if (p->createDeviceList(CL_DEVICE_TYPE_GPU) == RGY_ERR_NONE) {
            acc += (int)p->devs().size();
        }
        return acc;
    });
#endif
    PrintMes(RGY_LOG_DEBUG, _T("adapterCount %d.\n"), adapterCount);

    for (int i = 0; i < adapterCount; i++) {
        auto dev = std::make_unique<VCEDevice>(m_pLog, m_pFactory, m_pTrace);
        PrintMes(RGY_LOG_DEBUG, _T("Init adaptor #%d.\n"), i);
        if (dev->init(i, interopD3d9, interopD3d11, interopVulkan, enableOpenCL, enableVppPerfMonitor, enableAV1HWDec) == RGY_ERR_NONE) {
            devs.push_back(std::move(dev));
        }
    }
    return devs;
}
