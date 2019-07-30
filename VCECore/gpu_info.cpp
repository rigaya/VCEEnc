// -----------------------------------------------------------------------------------------
// QSVEnc/NVEnc by rigaya
// -----------------------------------------------------------------------------------------
//
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
// ------------------------------------------------------------------------------------------

#include "rgy_tchar.h"
#include <string>
#include <vector>
#include <random>
#include <future>
#include <algorithm>
#include "rgy_opencl.h"
#include "rgy_osdep.h"
#include "rgy_util.h"

#if ENABLE_OPENCL

static std::basic_string<TCHAR> to_tchar(const char *string) {
#if UNICODE
    int required_length = MultiByteToWideChar(CP_ACP, 0, string, -1, NULL, 0);
    std::basic_string<TCHAR> str(1+required_length, _T('\0'));
    MultiByteToWideChar(CP_ACP, 0, string, -1, &str[0], (int)str.size());
#else
    std::basic_string<char> str = string;
#endif
    return str;
};

#if 0
static cl_int cl_create_info_string(cl_data_t *cl_data, const cl_func_t *cl, TCHAR *buffer, unsigned int buffer_size) {
    cl_int ret = cl_get_device_name(cl_data, cl, buffer, buffer_size);
    if (ret != CL_SUCCESS) {
        return ret;
    }
    const int device_cu = cl_get_device_max_compute_units(cl_data, cl);
    if (device_cu > 0) {
        _stprintf_s(buffer + _tcslen(buffer), buffer_size - _tcslen(buffer), _T(" (%d EU)"), device_cu);
    }
    const int max_device_frequency = cl_get_device_max_clock_frequency_mhz(cl_data, cl);
    if (max_device_frequency) {
        _stprintf_s(buffer + _tcslen(buffer), buffer_size - _tcslen(buffer), _T(" @ %d MHz"), max_device_frequency);
    }
    TCHAR driver_ver[256] = { 0 };
    if (CL_SUCCESS == cl_get_driver_version(cl_data, cl, driver_ver, _countof(driver_ver))) {
        _stprintf_s(buffer + _tcslen(buffer), buffer_size - _tcslen(buffer), _T(" (%s)"), driver_ver);
    }
    return ret;
}
#endif

#endif //ENABLE_OPENCL

#if ENCODER_NVENC && !FOR_AUO
#include "NVEncCore.h"
#endif //#if ENCODER_NVENC
#if ENCODER_VCEENC && !FOR_AUO
#include "rgy_device.h"
#endif //#if ENCODER_VCEENC && !FOR_AUO

#pragma warning (push)
#pragma warning (disable: 4100)
int getGPUInfo(const char *VendorName, TCHAR *buffer, unsigned int buffer_size, int device_id, bool driver_version_only) {
#if ENCODER_NVENC && !FOR_AUO
    NVEncoderGPUInfo nvGPUInfo(device_id, true);
    const auto gpulist = nvGPUInfo.getGPUList();
    if (gpulist.size() > 0) {
        tstring gpu_info;
        const auto& gpuInfo = std::find_if(gpulist.begin(), gpulist.end(), [device_id](const NVGPUInfo& info) { return info.id == device_id; });
        if (gpuInfo != gpulist.end()) {
            if (driver_version_only) {
                if (gpuInfo->nv_driver_version) {
                    gpu_info = strsprintf(_T("%d.%d"), gpuInfo->nv_driver_version / 1000, (gpuInfo->nv_driver_version % 1000) / 10);
                }
            } else {
                gpu_info = strsprintf(_T("#%d: %s"), gpuInfo->id, gpuInfo->name.c_str());
                if (gpuInfo->cuda_cores > 0) {
                    gpu_info += strsprintf(_T(" (%d cores"), gpuInfo->cuda_cores);
                    if (gpuInfo->clock_rate > 0) {
                        gpu_info += strsprintf(_T(", %d MHz"), gpuInfo->clock_rate / 1000);
                    }
                    gpu_info += strsprintf(_T(")"));
                }
                if (gpuInfo->pcie_gen > 0 && gpuInfo->pcie_link > 0) {
                    gpu_info += strsprintf(_T("[PCIe%dx%d]"), gpuInfo->pcie_gen, gpuInfo->pcie_link);
                }
                if (gpuInfo->nv_driver_version) {
                    gpu_info += strsprintf(_T("[%d.%d]"), gpuInfo->nv_driver_version / 1000, (gpuInfo->nv_driver_version % 1000) / 10);
                }
            }
            _tcscpy_s(buffer, buffer_size, gpu_info.c_str());
            return 0;
        }
    }
#endif  //#if ENCODER_NVENC && !FOR_AUO
#if ENCODER_VCEENC && !FOR_AUO
    DeviceDX11 dx11;
    auto err = dx11.Init(device_id, false, nullptr);
    if (err == RGY_ERR_NONE) {
        auto str = dx11.GetDisplayDeviceName();
        str = str_replace(str, L" (TM)", L"");
        str = str_replace(str, L" (R)", L"");
        str = str_replace(str, L" Series", L"");
        str = str_replace(str, L" Graphics", L"");
        _tcsncpy(buffer, wstring_to_tstring(str).c_str(), buffer_size);
        return 0;
    }
#endif //#if ENCODER_VCEENC && !FOR_AUO
#if !ENABLE_OPENCL
    _stprintf_s(buffer, buffer_size, _T("Unknown (not compiled with OpenCL support)"));
    return 0;
#else
    if (initOpenCLGlobal()) {
        _tcscpy_s(buffer, buffer_size, _T("Unknown (Failed to load OpenCL.dll)"));
        return 1;
    }
    RGYOpenCL cl;
    auto platforms = cl.getPlatforms(VendorName);
    if (platforms.size() == 0) {
        _stprintf_s(buffer, buffer_size, _T("Unknown (Failed to find OpenCL %s platform)"), to_tchar(VendorName).c_str());
        return 1;
    }
    const auto& platform = platforms[0];
    if (platform->createDeviceList(CL_DEVICE_TYPE_GPU) != CL_SUCCESS || platform->devs().size() <= device_id) {
        _stprintf_s(buffer, buffer_size, _T("Unknown (Failed to find %s GPU)"), to_tchar(VendorName).c_str());
        return 1;
    }
    _tcscpy_s(buffer, buffer_size, RGYOpenCLDevice(platform->dev(device_id)).infostr().c_str());
    return 0;
#endif // !ENABLE_OPENCL
}

#pragma warning (pop)
