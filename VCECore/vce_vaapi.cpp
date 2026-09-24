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

#include "vce_vaapi.h"

#if ENABLE_VAAPI

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <fcntl.h>
#include <string>
#include <unistd.h>

extern "C" {
#include <libavutil/hwcontext.h>
#include <libavutil/hwcontext_vaapi.h>
}

#include <va/va.h>
#include <va/va_drm.h>

namespace {

const char *codec_name(const RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_H264: return "h264_vaapi";
    case RGY_CODEC_HEVC: return "hevc_vaapi";
    case RGY_CODEC_AV1:  return "av1_vaapi";
    default:             return nullptr;
    }
}

VAProfile va_profile(const RGY_CODEC codec) {
    switch (codec) {
    case RGY_CODEC_H264: return VAProfileH264High;
    case RGY_CODEC_HEVC: return VAProfileHEVCMain;
    case RGY_CODEC_AV1:  return VAProfileAV1Profile0;
    default:             return VAProfileNone;
    }
}

bool has_encoding_entrypoint(VADisplay display, const VAProfile profile, VAEntrypoint& selected) {
    const int maxEntrypoints = vaMaxNumEntrypoints(display);
    if (maxEntrypoints <= 0) return false;
    std::vector<VAEntrypoint> entrypoints(maxEntrypoints);
    int numEntrypoints = 0;
    if (vaQueryConfigEntrypoints(display, profile, entrypoints.data(), &numEntrypoints) != VA_STATUS_SUCCESS) {
        return false;
    }
    for (int i = 0; i < numEntrypoints; i++) {
        if (entrypoints[i] == VAEntrypointEncSlice) {
            selected = VAEntrypointEncSlice;
            return true;
        }
    }
    for (int i = 0; i < numEntrypoints; i++) {
        if (entrypoints[i] == VAEntrypointEncSliceLP) {
            selected = VAEntrypointEncSliceLP;
            return true;
        }
    }
    return false;
}

bool query_profile_attributes(VADisplay display, const VAProfile profile, const VAEntrypoint entrypoint,
    uint32_t& rcModes, int& maxRefL0, int& maxRefL1, int& maxWidth, int& maxHeight, uint32_t& rtFormat) {
    std::array<VAConfigAttrib, 5> attrs = {{
        { VAConfigAttribRateControl, VA_ATTRIB_NOT_SUPPORTED },
        { VAConfigAttribEncMaxRefFrames, VA_ATTRIB_NOT_SUPPORTED },
        { VAConfigAttribMaxPictureWidth, VA_ATTRIB_NOT_SUPPORTED },
        { VAConfigAttribMaxPictureHeight, VA_ATTRIB_NOT_SUPPORTED },
        { VAConfigAttribRTFormat, VA_ATTRIB_NOT_SUPPORTED },
    }};
    if (vaGetConfigAttributes(display, profile, entrypoint, attrs.data(), (int)attrs.size()) != VA_STATUS_SUCCESS) {
        return false;
    }
    if (attrs[0].value != VA_ATTRIB_NOT_SUPPORTED) {
        if (attrs[0].value & VA_RC_CBR)  rcModes |= VCE_VA_RC_CBR;
        if (attrs[0].value & VA_RC_VBR)  rcModes |= VCE_VA_RC_VBR;
        if (attrs[0].value & VA_RC_CQP)  rcModes |= VCE_VA_RC_CQP;
        if (attrs[0].value & VA_RC_QVBR) rcModes |= VCE_VA_RC_QVBR;
    }
    if (attrs[1].value != VA_ATTRIB_NOT_SUPPORTED) {
        maxRefL0 = (int)(attrs[1].value & 0xffffu);
        maxRefL1 = (int)((attrs[1].value >> 16) & 0xffffu);
    }
    if (attrs[2].value != VA_ATTRIB_NOT_SUPPORTED) maxWidth = (int)attrs[2].value;
    if (attrs[3].value != VA_ATTRIB_NOT_SUPPORTED) maxHeight = (int)attrs[3].value;
    if (attrs[4].value != VA_ATTRIB_NOT_SUPPORTED) rtFormat = attrs[4].value;
    return true;
}

bool supports_10bit(VADisplay display, const RGY_CODEC codec, const VAEntrypoint entrypoint) {
    if (codec == RGY_CODEC_H264) return false;
    VAProfile profile = va_profile(codec);
    if (codec == RGY_CODEC_HEVC) {
        profile = VAProfileHEVCMain10;
    }
    uint32_t rcModes = 0;
    uint32_t rtFormat = 0;
    int maxRefL0 = 0, maxRefL1 = 0, maxWidth = 0, maxHeight = 0;
    return query_profile_attributes(display, profile, entrypoint, rcModes, maxRefL0, maxRefL1, maxWidth, maxHeight, rtFormat)
        && (rtFormat & VA_RT_FORMAT_YUV420_10) != 0;
}

tstring get_device_name(const char *vendorString) {
    if (vendorString == nullptr || vendorString[0] == '\0') {
        return _T("AMD VA-API device");
    }
    std::string name(vendorString);
    const auto forPos = name.find(" for ");
    if (forPos != std::string::npos) {
        name = name.substr(forPos + 5);
        const auto detailPos = name.find(" (");
        if (detailPos != std::string::npos) name.resize(detailPos);
    }
    return char_to_tstring(name);
}

std::string read_pci_bus_id(const std::filesystem::path& renderNode) {
    const auto sysfsDevice = std::filesystem::path("/sys/class/drm") / renderNode.filename() / "device";
    std::error_code ec;
    const auto resolved = std::filesystem::canonical(sysfsDevice, ec);
    return ec ? std::string() : resolved.filename().string();
}

void va_info_callback(void *userContext, const char *message) {
    auto *log = (RGYLog *)userContext;
    if (log != nullptr && message != nullptr && message[0] != '\0') {
        log->write(RGY_LOG_DEBUG, RGY_LOGT_DEV, _T("libva: %s\n"), char_to_tstring(message).c_str());
    }
}

void va_error_callback(void *userContext, const char *message) {
    auto *log = (RGYLog *)userContext;
    if (log != nullptr && message != nullptr && message[0] != '\0') {
        log->write(RGY_LOG_WARN, RGY_LOGT_DEV, _T("libva: %s\n"), char_to_tstring(message).c_str());
    }
}

bool is_amd_render_node(const std::filesystem::path& renderNode) {
    const auto vendorPath = std::filesystem::path("/sys/class/drm") / renderNode.filename() / "device/vendor";
    std::ifstream vendorFile(vendorPath);
    std::string value;
    if (!vendorFile || !(vendorFile >> value)) {
        return false;
    }
    try {
        return std::stoul(value, nullptr, 0) == 0x1002;
    } catch (...) {
        return false;
    }
}

bool probe_va_device(const std::filesystem::path& renderNode, tstring& name, RGYLog *log) {
    const int fd = open(renderNode.c_str(), O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        return false;
    }
    VADisplay display = vaGetDisplayDRM(fd);
    if (display != nullptr) {
        vaSetInfoCallback(display, va_info_callback, log);
        vaSetErrorCallback(display, va_error_callback, log);
    }
    int major = 0, minor = 0;
    const auto status = display ? vaInitialize(display, &major, &minor) : VA_STATUS_ERROR_INVALID_DISPLAY;
    if (status == VA_STATUS_SUCCESS) {
        name = get_device_name(vaQueryVendorString(display));
    }
    // vaInitialize に失敗した場合も vaGetDisplayDRM で確保した display は vaTerminate で解放する
    if (display != nullptr) {
        vaTerminate(display);
    }
    close(fd);
    return status == VA_STATUS_SUCCESS;
}

bool test_encoder_open(AVBufferRef *hwdevice, const AVCodec *codec, const bool tenBit) {
    if (hwdevice == nullptr || codec == nullptr) return false;
    AVBufferRef *framesRefRaw = av_hwframe_ctx_alloc(hwdevice);
    if (framesRefRaw == nullptr) return false;
    std::unique_ptr<AVBufferRef, RGYAVDeleter<AVBufferRef>> framesRef(framesRefRaw, RGYAVDeleter<AVBufferRef>(av_buffer_unref));
    auto *frames = (AVHWFramesContext *)framesRef->data;
    frames->format = AV_PIX_FMT_VAAPI;
    frames->sw_format = tenBit ? AV_PIX_FMT_P010 : AV_PIX_FMT_NV12;
    frames->width = 640;
    frames->height = 360;
    frames->initial_pool_size = 2;
    if (av_hwframe_ctx_init(framesRef.get()) < 0) return false;

    AVCodecContext *contextRaw = avcodec_alloc_context3(codec);
    if (contextRaw == nullptr) return false;
    std::unique_ptr<AVCodecContext, RGYAVDeleter<AVCodecContext>> context(contextRaw, RGYAVDeleter<AVCodecContext>(avcodec_free_context));
    context->width = frames->width;
    context->height = frames->height;
    context->time_base = AVRational{ 1, 30 };
    context->framerate = AVRational{ 30, 1 };
    context->bit_rate = 1000000;
    context->gop_size = 30;
    context->max_b_frames = 0;
    context->pix_fmt = AV_PIX_FMT_VAAPI;
    if (tenBit && codec->id == AV_CODEC_ID_HEVC) {
        context->profile = AV_PROFILE_HEVC_MAIN_10;
    }
    context->hw_frames_ctx = av_buffer_ref(framesRef.get());
    if (context->hw_frames_ctx == nullptr) return false;

    const int previousLogLevel = av_log_get_level();
    av_log_set_level(AV_LOG_QUIET);
    struct AvLogLevelRestorer { int prev; ~AvLogLevelRestorer() { av_log_set_level(prev); } } avGuard{ previousLogLevel };
    const int openResult = avcodec_open2(context.get(), codec, nullptr);
    return openResult >= 0;
}

} // namespace

VCEVAEncCaps::VCEVAEncCaps() :
    available(false),
    support10bit(false),
    rcModes(0),
    maxRefL0(0),
    maxRefL1(0),
    maxWidth(0),
    maxHeight(0) {
}

std::vector<VCEVADeviceInfo> enumerateVADevices(RGYLog *log) {
    std::vector<std::filesystem::path> nodes;
    std::error_code ec;
    const std::filesystem::path drmPath("/dev/dri");
    for (std::filesystem::directory_iterator it(drmPath, ec), end; !ec && it != end; it.increment(ec)) {
        const auto name = it->path().filename().string();
        if (name.compare(0, 7, "renderD") == 0 && name.size() > 7
            && std::all_of(name.begin() + 7, name.end(), [](const char c) { return c >= '0' && c <= '9'; })) {
            nodes.push_back(it->path());
        }
    }
    std::sort(nodes.begin(), nodes.end(), [](const auto& a, const auto& b) {
        return std::stoi(a.filename().string().substr(7)) < std::stoi(b.filename().string().substr(7));
    });

    std::vector<VCEVADeviceInfo> devices;
    for (const auto& node : nodes) {
        if (!is_amd_render_node(node)) continue;
        tstring name;
        if (!probe_va_device(node, name, log)) continue;
        VCEVADeviceInfo info;
        info.id = (int)devices.size();
        info.renderNode = char_to_tstring(node.string());
        info.pciBusId = read_pci_bus_id(node);
        info.name = std::move(name);
        devices.push_back(std::move(info));
        if (log != nullptr) {
            log->write(RGY_LOG_DEBUG, RGY_LOGT_DEV, _T("VA-API device #%d: %s (%s, PCI %s)\n"),
                devices.back().id, devices.back().name.c_str(), devices.back().renderNode.c_str(), char_to_tstring(devices.back().pciBusId).c_str());
        }
    }
    return devices;
}

VCEDeviceVA::VCEDeviceVA() :
    m_info(),
    m_hwdevice(nullptr, RGYAVDeleter<AVBufferRef>(av_buffer_unref)),
    m_display(nullptr),
    m_encCaps(),
    m_log() {
}

VCEDeviceVA::~VCEDeviceVA() {
}

RGY_ERR VCEDeviceVA::open(const VCEVADeviceInfo& info, std::shared_ptr<RGYLog> log) {
    m_info = info;
    m_log = std::move(log);
    AVBufferRef *deviceRaw = nullptr;
    const auto devicePath = tchar_to_string(m_info.renderNode);
    const int result = av_hwdevice_ctx_create(&deviceRaw, AV_HWDEVICE_TYPE_VAAPI, devicePath.c_str(), nullptr, 0);
    if (result < 0 || deviceRaw == nullptr) {
        if (m_log != nullptr) {
            char errbuf[AV_ERROR_MAX_STRING_SIZE] = {};
            av_strerror(result, errbuf, sizeof(errbuf));
            m_log->write(RGY_LOG_ERROR, RGY_LOGT_DEV, _T("Failed to open VA-API device %s (%s).\n"),
                m_info.renderNode.c_str(), char_to_tstring(errbuf).c_str());
        }
        return RGY_ERR_DEVICE_NOT_FOUND;
    }
    m_hwdevice.reset(deviceRaw);
    const auto *deviceCtx = (const AVHWDeviceContext *)m_hwdevice->data;
    const auto *vaCtx = (const AVVAAPIDeviceContext *)deviceCtx->hwctx;
    m_display = vaCtx->display;
    if (m_display == nullptr) {
        m_hwdevice.reset();
        return RGY_ERR_DEVICE_NOT_FOUND;
    }
    return RGY_ERR_NONE;
}

const VCEVAEncCaps& VCEDeviceVA::encCaps(RGY_CODEC codec) {
    const auto cached = m_encCaps.find(codec);
    if (cached != m_encCaps.end()) return cached->second;

    VCEVAEncCaps caps;
    const auto name = codec_name(codec);
    VAEntrypoint entrypoint = VAEntrypointEncSlice;
    if (m_display != nullptr && name != nullptr && has_encoding_entrypoint((VADisplay)m_display, va_profile(codec), entrypoint)) {
        uint32_t rtFormat = 0;
        query_profile_attributes((VADisplay)m_display, va_profile(codec), entrypoint,
            caps.rcModes, caps.maxRefL0, caps.maxRefL1, caps.maxWidth, caps.maxHeight, rtFormat);
        const bool supports10bit = supports_10bit((VADisplay)m_display, codec, entrypoint);

        const AVCodec *avcodec = avcodec_find_encoder_by_name(name);
        const bool opened8bit = test_encoder_open(m_hwdevice.get(), avcodec, false);
        const bool opened10bit = supports10bit && test_encoder_open(m_hwdevice.get(), avcodec, true);
        caps.support10bit = opened8bit && opened10bit;
        caps.available = avcodec != nullptr && opened8bit;
        if (m_log != nullptr) {
            m_log->write(RGY_LOG_DEBUG, RGY_LOGT_DEV, _T("VA-API encoder trial open %s (8-bit): %s\n"),
                char_to_tstring(name).c_str(), opened8bit ? _T("succeeded") : _T("failed"));
            if (supports10bit) {
                m_log->write(RGY_LOG_DEBUG, RGY_LOGT_DEV, _T("VA-API encoder trial open %s (10-bit): %s\n"),
                    char_to_tstring(name).c_str(), opened10bit ? _T("succeeded") : _T("failed"));
            }
        }
    }
    return m_encCaps.emplace(codec, caps).first->second;
}

tstring VCEDeviceVA::capsString(RGY_CODEC codec) {
    const auto& caps = encCaps(codec);
    tstring rcModes;
    const auto appendMode = [&rcModes](const uint32_t flag, const TCHAR *name) {
        if (flag) {
            if (!rcModes.empty()) rcModes += _T(", ");
            rcModes += name;
        }
    };
    appendMode(caps.rcModes & VCE_VA_RC_CBR, _T("CBR"));
    appendMode(caps.rcModes & VCE_VA_RC_VBR, _T("VBR"));
    appendMode(caps.rcModes & VCE_VA_RC_CQP, _T("CQP"));
    appendMode(caps.rcModes & VCE_VA_RC_QVBR, _T("QVBR"));
    if (rcModes.empty()) rcModes = _T("none");
    return strsprintf(_T("  available: %s\n  10-bit: %s\n  rate control: %s\n  max ref (L0/L1): %d/%d\n  max resolution: %dx%d"),
        caps.available ? _T("yes") : _T("no"), caps.support10bit ? _T("yes") : _T("no"), rcModes.c_str(), caps.maxRefL0, caps.maxRefL1, caps.maxWidth, caps.maxHeight);
}

#endif // ENABLE_VAAPI
