// -----------------------------------------------------------------------------------------
//     VCEEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2021 rigaya
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

#include "rgy_device_vulkan.h"

#if ENABLE_VULKAN

#include <unordered_set>

DeviceVulkan::DeviceVulkan() :
    m_name(_T("devVulkan")),
    m_vkInstance(nullptr),
    m_vkPhysicalDevice(nullptr),
    m_vkDevice(nullptr),
    m_displayDeviceName(),
    m_uuid(),
    m_vendorID(0),
    m_deviceID(0),
    m_vk(),
    m_uQueueGraphicsFamilyIndex(-1),
    m_uQueueComputeFamilyIndex(-1),
    m_hQueueGraphics(NULL),
    m_hQueueCompute(NULL),
    m_log() {
}
DeviceVulkan::~DeviceVulkan() {
    Terminate();
}

#if ENCODER_VCEENC
amf::AMFVulkanDevice* DeviceVulkan::GetAMFDevice() {
    m_VulkanDev.hDevice = m_vkDevice;
    m_VulkanDev.hInstance = m_vkInstance;
    m_VulkanDev.hPhysicalDevice = m_vkPhysicalDevice;
    m_VulkanDev.cbSizeof = sizeof(amf::AMFVulkanDevice);
    m_VulkanDev.pNext = nullptr;
    return &m_VulkanDev;
}
#endif

int DeviceVulkan::adapterCount() {
    return (int)adapterList().size();
}

std::vector<RGYVulkanAdapterInfo> DeviceVulkan::adapterList() {
    std::vector<RGYVulkanAdapterInfo> adapters;
    RGY_ERR res = RGY_ERR_NONE;
    if (m_vk.init() != 0) {
        return adapters;
    }

    if ((res = CreateInstance({})) != RGY_ERR_NONE) {
        return adapters;
    }

    uint32_t physicalDeviceCount = 0;
    if ((res = err_to_rgy(GetVulkan()->vkEnumeratePhysicalDevices(m_vkInstance, &physicalDeviceCount, nullptr))) != RGY_ERR_NONE) {
        return adapters;
    }
    std::vector<VkPhysicalDevice> physicalDevices{ physicalDeviceCount };
    if ((res = err_to_rgy(GetVulkan()->vkEnumeratePhysicalDevices(m_vkInstance, &physicalDeviceCount, physicalDevices.data()))) != RGY_ERR_NONE) {
        return adapters;
    }

    for (const auto& physicalDevice : physicalDevices) {
        VkPhysicalDeviceProperties props = {};
        GetVulkan()->vkGetPhysicalDeviceProperties(physicalDevice, &props);
        adapters.push_back({ props.vendorID, props.deviceID, props.deviceName });
    }
    return adapters;
}

RGY_ERR DeviceVulkan::Init(int adapterID, const std::vector<const char*> &extInstance, const std::vector<const char*> &extDevice, std::shared_ptr<RGYLog> log, bool logTryMode, amf::AMFContext *pContext) {
    RGY_ERR res = RGY_ERR_NONE;
    m_log = log;
    m_logTryMode = logTryMode;
    AddMessage(RGY_LOG_DEBUG, _T("Init Vulkan device...\n"));

    if (m_vk.init() != 0) {
        AddMessage(RGY_LOG_ERROR, _T("LoadFunctionsTable() failed - check if the proper Vulkan SDK is installed\n"));
        return RGY_ERR_NULL_PTR;
    }
    AddMessage(RGY_LOG_DEBUG, _T("LoadFunctionsTable() success.\n"));

    if ((res = CreateInstance(extInstance)) != RGY_ERR_NONE) {
        AddMessage(RGY_LOG_ERROR, _T("CreateInstance() failed\n"));
        return res;
    }
    AddMessage(RGY_LOG_DEBUG, _T("CreateInstance() success.\n"));

#if defined(_DEBUG) && defined(ENABLE_VALIDATION)
    const bool bDebug = true;
#else
    const bool bDebug = false;
#endif
    if (m_vk.load(m_vkInstance, bDebug) != 0) {
        AddMessage(RGY_LOG_ERROR, _T("LoadInstanceFunctionsTableExt() failed - check if the proper Vulkan SDK is installed\n"));
        return RGY_ERR_NULL_PTR;
    }
    AddMessage(RGY_LOG_DEBUG, _T("LoadInstanceFunctionsTableExt() success.\n"));

    // load instance based functions
    if (CreateDeviceAndFindQueues(adapterID, extDevice, pContext) != 0) {
        AddMessage(RGY_LOG_ERROR, _T("CreateDeviceAndFindQueues() failed\n"));
        return RGY_ERR_NULL_PTR;
    }
    AddMessage(RGY_LOG_DEBUG, _T("CreateDeviceAndFindQueues() success.\n"));

    if (m_vk.load(m_vkDevice) != 0) {
        AddMessage(RGY_LOG_ERROR, _T("LoadDeviceFunctionsTableExt() failed - check if the proper Vulkan SDK is installed\n"));
        return RGY_ERR_NULL_PTR;
    }
    AddMessage(RGY_LOG_DEBUG, _T("LoadDeviceFunctionsTableExt() success.\n"));

    return RGY_ERR_NONE;
}

RGY_ERR DeviceVulkan::Terminate() {
    m_hQueueGraphics = NULL;
    m_hQueueCompute = NULL;
    m_uQueueGraphicsFamilyIndex = -1;
    m_uQueueComputeFamilyIndex = -1;

    if (m_vkDevice != VK_NULL_HANDLE) {
        GetVulkan()->vkDestroyDevice(m_vkDevice, nullptr);
        m_vkDevice = VK_NULL_HANDLE;
    }
    if (m_vkInstance != VK_NULL_HANDLE) {
        GetVulkan()->vkDestroyInstance(m_vkInstance, nullptr);
        m_vkInstance = VK_NULL_HANDLE;
    }
    m_vkPhysicalDevice = VK_NULL_HANDLE;
    m_displayDeviceName.clear();
    memset(m_uuid, 0, sizeof(m_uuid));
    m_vendorID = 0;
    m_deviceID = 0;
    return RGY_ERR_NONE;
}

std::vector<const char*> DeviceVulkan::GetDebugInstanceExtensionNames() {
    uint32_t extensionCount = 0;
    GetVulkan()->vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> instanceExtensions{ extensionCount };
    GetVulkan()->vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, instanceExtensions.data());

    std::vector<const char*> result;
    for (const VkExtensionProperties& e : instanceExtensions) {
        if (strcmp(e.extensionName, "VK_EXT_debug_report") == 0) {
            result.push_back("VK_EXT_debug_report");
        }
    }
    return result;
}

std::vector<const char*> DeviceVulkan::GetDebugInstanceLayerNames() {
    uint32_t layerCount = 0;
    GetVulkan()->vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> instanceLayers{ layerCount };
    GetVulkan()->vkEnumerateInstanceLayerProperties(&layerCount, instanceLayers.data());
    std::vector<const char*> result;
    for (const VkLayerProperties& p : instanceLayers) {
        if (strcmp(p.layerName, "VK_LAYER_LUNARG_standard_validation") == 0) {
            result.push_back("VK_LAYER_LUNARG_standard_validation");
        }
    }
    return result;
}

std::vector<const char*> DeviceVulkan::GetDebugDeviceLayerNames(VkPhysicalDevice device) {
    uint32_t layerCount = 0;
    GetVulkan()->vkEnumerateDeviceLayerProperties(device, &layerCount, nullptr);
    std::vector<VkLayerProperties> deviceLayers{ layerCount };
    GetVulkan()->vkEnumerateDeviceLayerProperties(device, &layerCount, deviceLayers.data());
    std::vector<const char*> result;
    for (const VkLayerProperties& p : deviceLayers) {
        if (strcmp(p.layerName, "VK_LAYER_LUNARG_standard_validation") == 0) {
            result.push_back("VK_LAYER_LUNARG_standard_validation");
        }
    }
    return result;
}
RGY_ERR DeviceVulkan::CreateInstance(const std::vector<const char*> &extInstance) {
    RGY_ERR res = RGY_ERR_NONE;

    if (m_vkInstance != VK_NULL_HANDLE) {
        return RGY_ERR_NONE;
    }

    // Create the Vulkan instance
    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    std::vector<const char*> instanceExtensions = extInstance;
    instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    instanceExtensions.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);
    instanceExtensions.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);
#if defined(_WIN32)
    instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__linux)
    instanceExtensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

#if defined(_DEBUG) && defined(ENABLE_VALIDATION)
    std::vector<const char*> debugInstanceExtensionNames = GetDebugInstanceExtensionNames();

    instanceExtensions.insert(instanceExtensions.end(),
        debugInstanceExtensionNames.begin(),
        debugInstanceExtensionNames.end());
#endif

    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    instanceCreateInfo.enabledExtensionCount = (decltype(instanceCreateInfo.enabledExtensionCount))(instanceExtensions.size());

    std::vector<const char*> instanceLayers;

#if defined(_DEBUG) && defined(ENABLE_VALIDATION)
	std::vector<const char*> debugInstanceLayerNames = GetDebugInstanceLayerNames();

    instanceLayers.insert(instanceLayers.end(),
        debugInstanceLayerNames.begin(),
        debugInstanceLayerNames.end());
#endif

    instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();
    instanceCreateInfo.enabledLayerCount = (decltype(instanceCreateInfo.enabledLayerCount))(instanceLayers.size());

    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.apiVersion          = VK_API_VERSION_1_3;
    applicationInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pApplicationName    = ENCODER_NAME;
    applicationInfo.pEngineName         = ENCODER_NAME;

    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    
    if ((res = err_to_rgy(GetVulkan()->vkCreateInstance(&instanceCreateInfo, nullptr, &m_vkInstance))) != RGY_ERR_NONE) {
        AddMessage(RGY_LOG_ERROR, _T("CreateInstance() failed to vkCreateInstance, Error=%s\n"), get_err_mes(res));
        return res;
    }

    return RGY_ERR_NONE;
}

RGY_ERR DeviceVulkan::CreateDeviceAndFindQueues(int adapterID, const std::vector<const char*> &extDevice, amf::AMFContext *pContext) {
    RGY_ERR res = RGY_ERR_NONE;
    uint32_t physicalDeviceCount = 0;
    if ((res = err_to_rgy(GetVulkan()->vkEnumeratePhysicalDevices(m_vkInstance, &physicalDeviceCount, nullptr))) != RGY_ERR_NONE) {
        AddMessage(RGY_LOG_ERROR, _T("CreateDeviceAndQueues() failed to vkEnumeratePhysicalDevices, Error=%s\n"), get_err_mes(res));
        return res;
    }

    std::vector<VkPhysicalDevice> physicalDevices{ physicalDeviceCount };
    if ((res = err_to_rgy(GetVulkan()->vkEnumeratePhysicalDevices(m_vkInstance, &physicalDeviceCount, physicalDevices.data()))) != RGY_ERR_NONE) {
        AddMessage(RGY_LOG_ERROR, _T("CreateDeviceAndQueues() failed to vkEnumeratePhysicalDevices, Error=%s\n"), get_err_mes(res));
        return res;
    }

    if (adapterID < 0) {
        adapterID = 0;
    }
    if (adapterID >= (int)physicalDeviceCount) {
        AddMessage(RGY_LOG_ERROR, _T("Invalid Adapter ID: %d"), adapterID);
        return RGY_ERR_UNKNOWN;
    }

    m_vkPhysicalDevice = physicalDevices[adapterID];
    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfoItems;
    std::vector<float> queuePriorities;

    uint32_t queueFamilyPropertyCount = 0;
    GetVulkan()->vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamilyPropertyCount, nullptr);
    if (queueFamilyPropertyCount == 0) {
        AddMessage(RGY_LOG_ERROR, _T("CreateDeviceAndQueues() queueFamilyPropertyCount = 0\n"));
        return RGY_ERR_UNKNOWN;
    }

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
    GetVulkan()->vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

    m_uQueueGraphicsFamilyIndex = -1;
    m_uQueueComputeFamilyIndex = -1;
    uint32_t uQueueDecodeFamilyIndex = std::numeric_limits<uint32_t>::max();

    uint32_t uQueueGraphicsIndex = std::numeric_limits<uint32_t>::max();
    uint32_t uQueueComputeIndex = std::numeric_limits<uint32_t>::max();
    for (uint32_t i = 0; i < queueFamilyPropertyCount; i++) {
        VkQueueFamilyProperties &queueFamilyProperty = queueFamilyProperties[i];
        AddMessage(RGY_LOG_DEBUG, _T("Queue family #%d: flags=0x%08x, count=%d.\n"),
            i, (uint32_t)queueFamilyProperty.queueFlags, (int)queueFamilyProperty.queueCount);
        if (queuePriorities.size() < queueFamilyProperty.queueCount) {
            queuePriorities.resize(queueFamilyProperty.queueCount, 1.0f);
        }
    }
    for (uint32_t i = 0; i < queueFamilyPropertyCount; i++) {
        VkQueueFamilyProperties &queueFamilyProperty = queueFamilyProperties[i];
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.pQueuePriorities = &queuePriorities[0];
        queueCreateInfo.queueFamilyIndex = i;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

        if ((queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT)
            && (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0
            && m_uQueueComputeFamilyIndex < 0) {
            m_uQueueComputeFamilyIndex = i;
            uQueueComputeIndex = 0;

            amf_int64 queueComputeIndex = -1;
            if (pContext != nullptr && pContext->GetProperty(AMF_CONTEXT_VULKAN_COMPUTE_QUEUE, &queueComputeIndex) == AMF_OK) {
                if (queueComputeIndex < 0 || queueComputeIndex >= queueFamilyProperty.queueCount) {
                    AddMessage(RGY_LOG_ERROR, _T("CreateDeviceAndFindQueues() invalid VulkanComputeQueue index %d not in range [%d,%d]\n"),
                        (int)queueComputeIndex, 0, (int)queueFamilyProperty.queueCount - 1);
                    return RGY_ERR_UNKNOWN;
                }
                uQueueComputeIndex = (uint32_t)queueComputeIndex;
                queueCreateInfo.queueCount = uQueueComputeIndex + 1;
            }
            deviceQueueCreateInfoItems.push_back(queueCreateInfo);
        }
        if ((queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            && m_uQueueGraphicsFamilyIndex < 0) {
            m_uQueueGraphicsFamilyIndex = i;
            uQueueGraphicsIndex = 0;
            deviceQueueCreateInfoItems.push_back(queueCreateInfo);
        }
        if ((queueFamilyProperty.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)
            && uQueueDecodeFamilyIndex == std::numeric_limits<uint32_t>::max()) {
            uQueueDecodeFamilyIndex = i;
            deviceQueueCreateInfoItems.push_back(queueCreateInfo);
        }
    }
    VkDeviceCreateInfo deviceCreateInfo = {};

    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = (decltype(deviceCreateInfo.queueCreateInfoCount))deviceQueueCreateInfoItems.size();
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfoItems[0];

    VkPhysicalDeviceFeatures features = {};
    GetVulkan()->vkGetPhysicalDeviceFeatures(m_vkPhysicalDevice, &features);
    deviceCreateInfo.pEnabledFeatures = &features;

    VkPhysicalDeviceSynchronization2Features syncFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES };
    syncFeatures.synchronization2 = VK_TRUE;
    deviceCreateInfo.pNext = &syncFeatures;

    VkPhysicalDeviceCoherentMemoryFeaturesAMD coherentMemoryFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD };
    coherentMemoryFeatures.deviceCoherentMemory = VK_TRUE;
    syncFeatures.pNext = &coherentMemoryFeatures;

    VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES };
    coherentMemoryFeatures.pNext = &indexingFeatures;
    indexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
    indexingFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
    indexingFeatures.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
    indexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
    indexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE;
    indexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE;

    VkPhysicalDeviceShaderAtomicInt64FeaturesKHR atomicInt64Features = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES_KHR };
    atomicInt64Features.shaderBufferInt64Atomics = VK_TRUE;
    indexingFeatures.pNext = &atomicInt64Features;

    std::vector<const char*> deviceLayers;

#if defined(_DEBUG) && defined(ENABLE_VALIDATION)
	std::vector<const char*> debugDeviceLayerNames = GetDebugDeviceLayerNames(physicalDevices[0]);

    deviceLayers.insert(deviceLayers.end(),
        debugDeviceLayerNames.begin(),
        debugDeviceLayerNames.end());
#endif

    deviceCreateInfo.ppEnabledLayerNames = deviceLayers.data();
    deviceCreateInfo.enabledLayerCount = (decltype(deviceCreateInfo.enabledLayerCount))(deviceLayers.size());

    std::vector<const char*> deviceExt = extDevice;
    deviceExt.insert(deviceExt.begin(), "VK_KHR_shader_atomic_int64");
    deviceExt.insert(deviceExt.begin(), "VK_KHR_swapchain");

    uint32_t extensionCount = 0;
    if ((res = err_to_rgy(GetVulkan()->vkEnumerateDeviceExtensionProperties(m_vkPhysicalDevice, nullptr, &extensionCount, nullptr))) != RGY_ERR_NONE) {
        AddMessage(RGY_LOG_ERROR, _T("vkEnumerateDeviceExtensionProperties() failed, err = %s\n"), get_err_mes(res));
        return res;
    }
    std::vector<VkExtensionProperties> deviceExtensionProperties(extensionCount);
    if ((res = err_to_rgy(GetVulkan()->vkEnumerateDeviceExtensionProperties(m_vkPhysicalDevice, nullptr, &extensionCount, deviceExtensionProperties.data()))) != RGY_ERR_NONE) {
        AddMessage(RGY_LOG_ERROR, _T("vkEnumerateDeviceExtensionProperties() failed, err = %s\n"), get_err_mes(res));
        return res;
    }
    std::unordered_set<std::string> deviceExtensionLookup;
    for (const auto& ext : deviceExtensionProperties) {
        deviceExtensionLookup.insert(ext.extensionName);
    }
    std::vector<const char*> supportedDeviceExtensions;
    for (const auto& ext : deviceExt) {
        if (deviceExtensionLookup.count(ext) > 0) {
            supportedDeviceExtensions.push_back(ext);
        } else {
            AddMessage(RGY_LOG_WARN, _T("Extension %s is not available. Some Vulkan features may not work correctly.\n"), char_to_tstring(ext).c_str());
        }
    }
    deviceCreateInfo.ppEnabledExtensionNames = supportedDeviceExtensions.data();
    deviceCreateInfo.enabledExtensionCount = (decltype(deviceCreateInfo.enabledExtensionCount))(supportedDeviceExtensions.size());

    if ((res = err_to_rgy(GetVulkan()->vkCreateDevice(m_vkPhysicalDevice, &deviceCreateInfo, nullptr, &m_vkDevice)))!= RGY_ERR_NONE) {
        AddMessage(RGY_LOG_ERROR, _T("CreateDeviceAndQueues() vkCreateDevice() failed, Error=%s\n"), get_err_mes(res));
        return res;
    }
    if (m_vkDevice == nullptr) {
        AddMessage(RGY_LOG_ERROR, _T("CreateDeviceAndQueues() vkCreateDevice() returned nullptr\n"));
        return RGY_ERR_NULL_PTR;
    }
    AddMessage(RGY_LOG_DEBUG, _T("vkCreateDevice #%d success.\n"), adapterID);
    
    GetVulkan()->vkGetDeviceQueue(m_vkDevice, m_uQueueGraphicsFamilyIndex, uQueueGraphicsIndex, &m_hQueueGraphics);
    GetVulkan()->vkGetDeviceQueue(m_vkDevice, m_uQueueComputeFamilyIndex, uQueueComputeIndex, &m_hQueueCompute);
    AddMessage(RGY_LOG_DEBUG, _T("Selected Vulkan queues: graphicsFamily=%d graphicsQueue=%p computeFamily=%d computeQueue=%p.\n"),
        m_uQueueGraphicsFamilyIndex,
        m_hQueueGraphics,
        m_uQueueComputeFamilyIndex,
        m_hQueueCompute);

    VkPhysicalDeviceProperties physicalDeviceProperties = {};
    GetVulkan()->vkGetPhysicalDeviceProperties(m_vkPhysicalDevice, &physicalDeviceProperties);
    m_displayDeviceName = physicalDeviceProperties.deviceName;
    m_vendorID = physicalDeviceProperties.vendorID;
    m_deviceID = physicalDeviceProperties.deviceID;

    VkPhysicalDeviceIDProperties physicalDeviceIDProperties = {};
    physicalDeviceIDProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES;
    physicalDeviceIDProperties.pNext = nullptr;

    VkPhysicalDeviceProperties2 physicalDeviceProperties2 = {};
    physicalDeviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    physicalDeviceProperties2.pNext = &physicalDeviceIDProperties;

    GetVulkan()->vkGetPhysicalDeviceProperties2(m_vkPhysicalDevice, &physicalDeviceProperties2);
    AddMessage(RGY_LOG_DEBUG, _T("vkGetPhysicalDeviceProperties2 #%d success.\n"), adapterID);
    memcpy(m_uuid, physicalDeviceIDProperties.deviceUUID, VK_UUID_SIZE);

    return RGY_ERR_NONE;
}

RGYVulkanFuncs *DeviceVulkan::GetVulkan() {
    return &m_vk;
}

void DeviceVulkan::AddMessage(RGYLogLevel log_level, const tstring &str) {
    if (m_log == nullptr || log_level < m_log->getLogLevel(RGY_LOGT_DEV)) {
        return;
    }
    auto lines = split(str, _T("\n"));
    for (const auto &line : lines) {
        if (line[0] != _T('\0')) {
            m_log->write((m_logTryMode && log_level > RGY_LOG_DEBUG) ? RGY_LOG_DEBUG : log_level, RGY_LOGT_DEV, (m_name + _T(": ") + line + _T("\n")).c_str());
        }
    }
}

void DeviceVulkan::AddMessage(RGYLogLevel log_level, const TCHAR *format, ...) {
    if (m_log == nullptr || log_level < m_log->getLogLevel(RGY_LOGT_DEV)) {
        return;
    }

    va_list args;
    va_start(args, format);
    int len = _vsctprintf(format, args) + 1; // _vscprintf doesn't count terminating '\0'
    tstring buffer;
    buffer.resize(len, _T('\0'));
    _vstprintf_s(&buffer[0], len, format, args);
    va_end(args);
    AddMessage(log_level, buffer);
}

#if defined(_WIN32) || defined(_WIN64)
HANDLE DeviceVulkan::getMemHandle(VkDeviceMemory memory) {
    HANDLE                        handle = NULL;
    VkMemoryGetWin32HandleInfoKHR win32_handle_info{};
    win32_handle_info.sType      = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;
    win32_handle_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT_KHR;
    win32_handle_info.memory     = memory;
    GetVulkan()->vkGetMemoryWin32HandleKHR(m_device, &win32_handle_info, &handle);
    return handle;
}

HANDLE DeviceVulkan::getSemaphoreHandle(VkSemaphore &sempahore) {
    HANDLE                           handle = NULL;
    VkSemaphoreGetWin32HandleInfoKHR win32_handle_info{};
    win32_handle_info.sType      = VK_STRUCTURE_TYPE_SEMAPHORE_GET_WIN32_HANDLE_INFO_KHR;
    win32_handle_info.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    win32_handle_info.semaphore  = sempahore;
    GetVulkan()->vkGetSemaphoreWin32HandleKHR(m_device, &win32_handle_info, &handle);
    return handle;
}
#else
int DeviceVulkan::getMemHandle(VkDeviceMemory memory) {
    int fd = 0;
    VkMemoryGetFdInfoKHR fd_info{};
    fd_info.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
    fd_info.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
    fd_info.memory = memory;
    GetVulkan()->vkGetMemoryFdKHR(m_vkDevice, &fd_info, &fd);
    return fd;
}

int DeviceVulkan::getSemaphoreHandle(VkSemaphore &sempahore) {
    int fd = 0;
    VkSemaphoreGetFdInfoKHR fd_info{};
    fd_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_GET_FD_INFO_KHR;
    fd_info.handleType = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;
    fd_info.semaphore = sempahore;
    GetVulkan()->vkGetSemaphoreFdKHR(m_vkDevice, &fd_info, &fd);
    return fd;
}
#endif

#endif
