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
#include "vce_param.h"
#include "rgy_frame.h"
#include "rgy_bitstream.h"
#include "vce_util.h"

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

// VA-APIのcompression_levelは値が大きいほど高速側のため、fast=7、balanced=4、slow=2、slower=1に対応させる。
constexpr int VA_PRESET_COMPRESSION_FAST = 7;
constexpr int VA_PRESET_COMPRESSION_BALANCED = 4;
constexpr int VA_PRESET_COMPRESSION_SLOW = 2;
constexpr int VA_PRESET_COMPRESSION_SLOWER = 1;
constexpr int VA_DEFAULT_FRAME_RATE = 30;
constexpr int VA_DEFAULT_BIT_DEPTH = 8;
constexpr int VA_DEFAULT_GOP_LENGTH = 30;
constexpr int VA_DEFAULT_GOP_SECONDS = 2;
constexpr int VA_DEFAULT_BITRATE = 1000000;
constexpr int VA_PROBE_WIDTH = 640;
constexpr int VA_PROBE_HEIGHT = 360;
constexpr int VA_PROBE_FRAME_POOL_SIZE = 2;
constexpr int VA_ENCODER_FRAME_POOL_SIZE = 8;

int va_compression_level(const RGY_CODEC codec, const int preset) {
    switch (codec) {
    case RGY_CODEC_H264:
        switch (preset) {
        case AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED: return VA_PRESET_COMPRESSION_FAST;
        case AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY: return VA_PRESET_COMPRESSION_SLOW;
        case AMF_VIDEO_ENCODER_QUALITY_PRESET_HIGH_QUALITY: return VA_PRESET_COMPRESSION_SLOWER;
        default: return VA_PRESET_COMPRESSION_BALANCED;
        }
    case RGY_CODEC_HEVC:
        switch (preset) {
        case AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_SPEED: return VA_PRESET_COMPRESSION_FAST;
        case AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_QUALITY: return VA_PRESET_COMPRESSION_SLOW;
        case AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_HIGH_QUALITY: return VA_PRESET_COMPRESSION_SLOWER;
        default: return VA_PRESET_COMPRESSION_BALANCED;
        }
    case RGY_CODEC_AV1:
        switch (preset) {
        case AMF_VIDEO_ENCODER_AV1_QUALITY_PRESET_SPEED: return VA_PRESET_COMPRESSION_FAST;
        case AMF_VIDEO_ENCODER_AV1_QUALITY_PRESET_QUALITY: return VA_PRESET_COMPRESSION_SLOW;
        case AMF_VIDEO_ENCODER_AV1_QUALITY_PRESET_HIGH_QUALITY: return VA_PRESET_COMPRESSION_SLOWER;
        default: return VA_PRESET_COMPRESSION_BALANCED;
        }
    default:
        return VA_PRESET_COMPRESSION_BALANCED;
    }
}

const TCHAR *va_preset_name(const int preset) {
    switch (preset) {
    case VA_PRESET_COMPRESSION_FAST: return _T("fast");
    case VA_PRESET_COMPRESSION_SLOW: return _T("slow");
    case VA_PRESET_COMPRESSION_SLOWER: return _T("slower");
    default: return _T("balanced");
    }
}

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
    frames->width = VA_PROBE_WIDTH;
    frames->height = VA_PROBE_HEIGHT;
    frames->initial_pool_size = VA_PROBE_FRAME_POOL_SIZE;
    if (av_hwframe_ctx_init(framesRef.get()) < 0) return false;

    AVCodecContext *contextRaw = avcodec_alloc_context3(codec);
    if (contextRaw == nullptr) return false;
    std::unique_ptr<AVCodecContext, RGYAVDeleter<AVCodecContext>> context(contextRaw, RGYAVDeleter<AVCodecContext>(avcodec_free_context));
    context->width = frames->width;
    context->height = frames->height;
    context->time_base = AVRational{ 1, VA_DEFAULT_FRAME_RATE };
    context->framerate = AVRational{ VA_DEFAULT_FRAME_RATE, 1 };
    context->bit_rate = VA_DEFAULT_BITRATE;
    context->gop_size = VA_DEFAULT_GOP_LENGTH;
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

VCEEncoderVA::VCEEncoderVA() :
    m_codecCtx(nullptr, RGYAVDeleter<AVCodecContext>(avcodec_free_context)),
    m_hwframes(nullptr, RGYAVDeleter<AVBufferRef>(av_buffer_unref)),
    m_frameHW(nullptr, RGYAVDeleter<AVFrame>(av_frame_free)),
    m_frameSW(nullptr, RGYAVDeleter<AVFrame>(av_frame_free)),
    m_pkt(nullptr, RGYAVDeleter<AVPacket>(av_packet_free)),
    m_log(), m_codec(RGY_CODEC_UNKNOWN), m_width(0), m_height(0), m_bitdepth(VA_DEFAULT_BIT_DEPTH), m_rateControl(VCE_RC_CQP),
    m_qp(0), m_bframes(0), m_refs(0), m_preset(VA_PRESET_COMPRESSION_BALANCED) {
}

VCEEncoderVA::~VCEEncoderVA() = default;

RGY_ERR VCEEncoderVA::init(VCEDeviceVA *dev, const VCEParam *prm, int width, int height,
    rgy_rational<int> sar, rgy_rational<int> fps, rgy_rational<int> timebase, std::shared_ptr<RGYLog> log) {
    if (dev == nullptr || prm == nullptr || dev->hwdevice() == nullptr || width <= 0 || height <= 0) return RGY_ERR_INVALID_PARAM;
    m_log = std::move(log);
    m_codec = prm->codec;
    m_width = width;
    m_height = height;
    m_bitdepth = prm->outputDepth;
    m_rateControl = prm->rateControl;
    const AVRational avSar{ sar.n(), sar.d() };
    const AVRational avFps{ fps.n(), fps.d() };
    const AVRational avTimebase{ timebase.n(), timebase.d() };
    const bool qvbr = prm->rateControl == get_codec_qvbr(prm->codec);
    const bool hqvbr = prm->rateControl == get_codec_hqvbr(prm->codec);
    const bool hqcbr = prm->rateControl == get_codec_hqcbr(prm->codec);
    const auto& caps = dev->encCaps(prm->codec);
    const auto defaultParam = VCEParam();
    auto warnUnsupported = [&](const TCHAR *option, const bool changed) {
        if (changed && m_log) m_log->write(RGY_LOG_WARN, RGY_LOGT_DEV,
            _T("WARN: %s is not supported with --backend vaapi, ignored.\n"), option);
    };
    warnUnsupported(_T("--pe"), prm->pe != defaultParam.pe);
    warnUnsupported(_T("--vbaq"), prm->bVBAQ != defaultParam.bVBAQ);
    warnUnsupported(_T("--pa"), prm->pa != defaultParam.pa);
    warnUnsupported(_T("--b-pyramid"), prm->bPyramid != defaultParam.bPyramid);
    warnUnsupported(_T("--adapt-minigop"), prm->adaptMiniGOP != defaultParam.adaptMiniGOP);
    warnUnsupported(_T("--slices"), prm->nSlices != defaultParam.nSlices);
    warnUnsupported(_T("--ltr"), prm->LTRFrames != defaultParam.LTRFrames);
    warnUnsupported(_T("--filler"), prm->bFiller != defaultParam.bFiller);
    warnUnsupported(_T("--tiles"), prm->tiles != defaultParam.tiles);
    warnUnsupported(_T("--deblock"), prm->deblockFilter != defaultParam.deblockFilter);
    warnUnsupported(_T("--skip-frame"), prm->enableSkipFrame != defaultParam.enableSkipFrame);
    warnUnsupported(_T("--motion-est"), prm->nMotionEst != defaultParam.nMotionEst);
    warnUnsupported(_T("--enforce-hrd"), prm->bEnforceHRD != defaultParam.bEnforceHRD);
    warnUnsupported(_T("--b-deltaqp"), prm->deltaQPBFrame != defaultParam.deltaQPBFrame);
    warnUnsupported(_T("--bref-deltaqp"), prm->deltaQPBFrameRef != defaultParam.deltaQPBFrameRef);
    warnUnsupported(_T("--repeat-headers"), prm->repeatHeaders != defaultParam.repeatHeaders);
    warnUnsupported(_T("--temporal-layers"), prm->temporalLayers != defaultParam.temporalLayers);
    warnUnsupported(_T("--cdef-mode"), prm->cdefMode != defaultParam.cdefMode);
    warnUnsupported(_T("--cdf-update"), prm->cdfUpdate != defaultParam.cdfUpdate);
    warnUnsupported(_T("--cdf-frame-end-update"), prm->cdfFrameEndUpdate != defaultParam.cdfFrameEndUpdate);
    warnUnsupported(_T("--aq-mode"), prm->aqMode != defaultParam.aqMode);
    warnUnsupported(_T("--qp-min/--qp-max inter"), prm->nQPMinInter != prm->nQPMin || prm->nQPMaxInter != prm->nQPMax);
    warnUnsupported(_T("--smart-access-video"), prm->smartAccessVideo != defaultParam.smartAccessVideo);
    warnUnsupported(_T("--multi-instance"), prm->multiInstance != defaultParam.multiInstance);
    warnUnsupported(_T("--screen-content-tools"), prm->screenContentTools != defaultParam.screenContentTools);
    warnUnsupported(_T("--palette-mode"), prm->paletteMode != defaultParam.paletteMode);
    warnUnsupported(_T("--force-integer-mv"), prm->forceIntegerMV != defaultParam.forceIntegerMV);
    if (prm->ctrl.parallelEnc.isEnabled()) {
        if (m_log) m_log->write(RGY_LOG_ERROR, RGY_LOGT_DEV, _T("--parallel is not supported with --backend vaapi.\n"));
        return RGY_ERR_UNSUPPORTED;
    }
    if (prm->vppamf.pp.enable || prm->vppamf.enhancer.enable || prm->vppamf.frc.enable) {
        if (m_log) m_log->write(RGY_LOG_ERROR, RGY_LOGT_DEV, _T("AMF preprocess/enhancer/FRC filters are not supported with --backend vaapi.\n"));
        return RGY_ERR_UNSUPPORTED;
    }
    if (prm->rateControl == get_codec_qvbr(prm->codec) && !(caps.rcModes & VCE_VA_RC_QVBR)) {
        if (m_log) m_log->write(RGY_LOG_ERROR, RGY_LOGT_DEV, _T("QVBR is not supported by this VA-API device.\n"));
        return RGY_ERR_UNSUPPORTED;
    }
    const char *name = codec_name(prm->codec);
    const AVCodec *codec = name ? avcodec_find_encoder_by_name(name) : nullptr;
    if (codec == nullptr) {
        if (m_log) m_log->write(RGY_LOG_ERROR, RGY_LOGT_DEV, _T("VA-API encoder %s was not found.\n"), char_to_tstring(name ? name : "unknown").c_str());
        return RGY_ERR_UNSUPPORTED;
    }
    int maxBFrames = prm->bframes.value_or(defaultParam.bframes.value_or(VCE_DEFAULT_BFRAMES));
    if (maxBFrames > 0 && caps.maxRefL1 <= 0) {
        if (m_log) m_log->write(RGY_LOG_WARN, RGY_LOGT_DEV, _T("WARN: --bframes is not supported with --backend vaapi, ignored (device reports maxRefL1=0).\n"));
        maxBFrames = 0;
    }
    m_bframes = maxBFrames;
    m_refs = prm->refFrames.value_or(caps.maxRefL0);
    if (m_refs > caps.maxRefL0) {
        if (m_log) m_log->write(RGY_LOG_WARN, RGY_LOGT_DEV, _T("WARN: --ref %d exceeds the VA-API device limit (%d), using %d.\n"), m_refs, caps.maxRefL0, caps.maxRefL0);
        m_refs = caps.maxRefL0;
    }
    m_qp = qvbr ? prm->qvbrLevel : prm->qp.qpP;

    AVBufferRef *framesRaw = av_hwframe_ctx_alloc(dev->hwdevice());
    if (framesRaw == nullptr) return RGY_ERR_NULL_PTR;
    m_hwframes.reset(framesRaw);
    auto *frames = (AVHWFramesContext *)m_hwframes->data;
    frames->format = AV_PIX_FMT_VAAPI;
    frames->sw_format = (prm->outputDepth > 8) ? AV_PIX_FMT_P010 : AV_PIX_FMT_NV12;
    frames->width = width;
    frames->height = height;
    frames->initial_pool_size = VA_ENCODER_FRAME_POOL_SIZE;
    int ret = av_hwframe_ctx_init(m_hwframes.get());
    if (ret < 0) return RGY_ERR_DEVICE_FAILED;

    AVCodecContext *ctxRaw = avcodec_alloc_context3(codec);
    if (ctxRaw == nullptr) return RGY_ERR_NULL_PTR;
    m_codecCtx.reset(ctxRaw);
    auto *ctx = m_codecCtx.get();
    ctx->width = width;
    ctx->height = height;
    ctx->bit_rate = (int64_t)(prm->nBitrate > 0 ? prm->nBitrate : (qvbr ? defaultParam.nBitrate : 0)) * 1000;
    ctx->rc_max_rate = (int64_t)prm->nMaxBitrate * 1000;
    ctx->rc_buffer_size = prm->nVBVBufferSize * 1000;
    ctx->time_base = avTimebase;
    ctx->framerate = avFps;
    ctx->pix_fmt = AV_PIX_FMT_VAAPI;
    ctx->gop_size = prm->nGOPLen > 0 ? prm->nGOPLen : avFps.num * VA_DEFAULT_GOP_SECONDS / avFps.den;
    ctx->max_b_frames = maxBFrames;
    ctx->refs = m_refs;
    ctx->sample_aspect_ratio = avSar;
    if (prm->outputDepth > 8 && prm->codec == RGY_CODEC_HEVC) ctx->profile = AV_PROFILE_HEVC_MAIN_10;
    else if (prm->codec == RGY_CODEC_H264 && prm->codecParam[RGY_CODEC_H264].nProfile != defaultParam.codecParam[RGY_CODEC_H264].nProfile) ctx->profile = prm->codecParam[RGY_CODEC_H264].nProfile;
    else if (prm->codec == RGY_CODEC_HEVC) ctx->profile = prm->codecParam[RGY_CODEC_HEVC].nProfile == AMF_VIDEO_ENCODER_HEVC_PROFILE_MAIN_10 ? AV_PROFILE_HEVC_MAIN_10 : AV_PROFILE_HEVC_MAIN;
    else if (prm->codec == RGY_CODEC_AV1) ctx->profile = AV_PROFILE_AV1_MAIN;
    if (prm->codecParam[prm->codec].nLevel > 0) ctx->level = prm->codecParam[prm->codec].nLevel;
    if (prm->nQPMin.has_value()) ctx->qmin = prm->nQPMin.value();
    if (prm->nQPMax.has_value()) ctx->qmax = prm->nQPMax.value();
    ctx->color_primaries = (AVColorPrimaries)prm->common.out_vui.colorprim;
    ctx->color_trc = (AVColorTransferCharacteristic)prm->common.out_vui.transfer;
    ctx->colorspace = (AVColorSpace)prm->common.out_vui.matrix;
    ctx->color_range = (AVColorRange)prm->common.out_vui.colorrange;
    ctx->hw_frames_ctx = av_buffer_ref(m_hwframes.get());
    if (ctx->hw_frames_ctx == nullptr) return RGY_ERR_NULL_PTR;

    AVDictionary *opts = nullptr;
    const bool cbr = prm->rateControl == get_codec_cbr(prm->codec) || hqcbr;
    const char *rcMode = prm->rateControl == get_codec_cqp(prm->codec) ? "CQP" : qvbr ? "QVBR" : cbr ? "CBR" : "VBR";
    av_dict_set(&opts, "rc_mode", rcMode, 0);
    if (prm->refFrames.has_value()) av_dict_set_int(&opts, "refs", prm->refFrames.value(), 0);
    if (hqvbr && m_log) m_log->write(RGY_LOG_WARN, RGY_LOGT_DEV, _T("WARN: --hqvbr is not supported with --backend vaapi, ignored (using VBR).\n"));
    if (hqcbr && m_log) m_log->write(RGY_LOG_WARN, RGY_LOGT_DEV, _T("WARN: --hqcbr is not supported with --backend vaapi, ignored (using CBR).\n"));
    if (prm->qualityPreset != defaultParam.qualityPreset) {
        const int compressionLevel = va_compression_level(prm->codec, prm->qualityPreset);
        ctx->compression_level = compressionLevel;
        m_preset = compressionLevel;
    }
    if (prm->codec == RGY_CODEC_H264 && prm->aud) av_dict_set(&opts, "aud", "1", 0);
    if (prm->codec == RGY_CODEC_HEVC && prm->aud) av_dict_set(&opts, "aud", "1", 0);
    if (prm->codec != RGY_CODEC_H264) {
        const int tier = prm->codecParam[prm->codec].nTier == AMF_VIDEO_ENCODER_HEVC_TIER_HIGH ? 1 : 0;
        av_dict_set_int(&opts, "tier", tier, 0);
    }
    if (prm->rateControl == get_codec_cqp(prm->codec)) {
        ctx->flags |= AV_CODEC_FLAG_QSCALE;
        ctx->global_quality = prm->qp.qpP * FF_QP2LAMBDA;
        ctx->i_quant_factor = (float)prm->qp.qpI / (float)(std::max)(1, prm->qp.qpP);
        ctx->b_quant_factor = (float)prm->qp.qpB / (float)(std::max)(1, prm->qp.qpP);
    } else if (qvbr) {
        av_dict_set_int(&opts, "qp", prm->qvbrLevel, 0);
    }
    ret = avcodec_open2(ctx, codec, &opts);
    av_dict_free(&opts);
    if (ret < 0) {
        char errbuf[AV_ERROR_MAX_STRING_SIZE] = {};
        av_strerror(ret, errbuf, sizeof(errbuf));
        if (m_log) m_log->write(RGY_LOG_ERROR, RGY_LOGT_DEV, _T("Failed to open VA-API encoder %s: %s.\n"), char_to_tstring(name).c_str(), char_to_tstring(errbuf).c_str());
        return RGY_ERR_UNSUPPORTED;
    }
    m_frameHW.reset(av_frame_alloc());
    m_frameSW.reset(av_frame_alloc());
    m_pkt.reset(av_packet_alloc());
    if (!m_frameHW || !m_frameSW || !m_pkt) return RGY_ERR_NULL_PTR;
    m_frameHW->format = AV_PIX_FMT_VAAPI;
    m_frameHW->width = width;
    m_frameHW->height = height;
    if (m_log) m_log->write(RGY_LOG_INFO, RGY_LOGT_DEV, _T("VA-API encoder initialized: %s %dx%d, %dbit, %d/%d fps.\n"),
        char_to_tstring(name).c_str(), width, height, m_bitdepth, avFps.num, avFps.den);
    return RGY_ERR_NONE;
}

RGY_ERR VCEEncoderVA::submit(RGYFrame *frame) {
    if (!m_codecCtx) return RGY_ERR_NOT_INITIALIZED;
    if (frame == nullptr) {
        const int ret = avcodec_send_frame(m_codecCtx.get(), nullptr);
        return ret == AVERROR(EAGAIN) ? RGY_ERR_MORE_DATA : (ret < 0 ? RGY_ERR_DEVICE_FAILED : RGY_ERR_NONE);
    }
    auto *sys = dynamic_cast<RGYSysFrame *>(frame);
    if (sys == nullptr) return RGY_ERR_UNSUPPORTED;
    const auto& info = sys->frameInfo();
    if (info.width != m_width || info.height != m_height) return RGY_ERR_INVALID_VIDEO_PARAM;
    av_frame_unref(m_frameSW.get());
    m_frameSW->format = (m_bitdepth > 8) ? AV_PIX_FMT_P010 : AV_PIX_FMT_NV12;
    m_frameSW->width = m_width;
    m_frameSW->height = m_height;
    for (int plane = 0; plane < 4; plane++) {
        m_frameSW->data[plane] = info.ptr[plane];
        m_frameSW->linesize[plane] = info.pitch[plane];
    }
    m_frameSW->pts = info.timestamp;
    m_frameSW->duration = (int64_t)info.duration;
    m_frameSW->pict_type = AV_PICTURE_TYPE_NONE;
    av_frame_unref(m_frameHW.get());
    m_frameHW->format = AV_PIX_FMT_VAAPI;
    m_frameHW->width = m_width;
    m_frameHW->height = m_height;
    int ret = av_hwframe_get_buffer(m_hwframes.get(), m_frameHW.get(), 0);
    if (ret < 0) return RGY_ERR_DEVICE_FAILED;
    ret = av_hwframe_transfer_data(m_frameHW.get(), m_frameSW.get(), 0);
    if (ret < 0) return RGY_ERR_DEVICE_FAILED;
    m_frameHW->pts = info.timestamp;
    m_frameHW->duration = (int64_t)info.duration;
    ret = avcodec_send_frame(m_codecCtx.get(), m_frameHW.get());
    return ret == AVERROR(EAGAIN) ? RGY_ERR_MORE_DATA : (ret < 0 ? RGY_ERR_DEVICE_FAILED : RGY_ERR_NONE);
}

RGY_ERR VCEEncoderVA::receive(std::shared_ptr<RGYBitstream>& bs) {
    bs.reset();
    if (!m_codecCtx || !m_pkt) return RGY_ERR_NOT_INITIALIZED;
    const int ret = avcodec_receive_packet(m_codecCtx.get(), m_pkt.get());
    if (ret == AVERROR(EAGAIN)) return RGY_ERR_MORE_DATA;
    if (ret == AVERROR_EOF) return RGY_ERR_MORE_BITSTREAM;
    if (ret < 0) return RGY_ERR_DEVICE_FAILED;
    auto output = std::make_shared<RGYBitstream>(RGYBitstreamInit());
    const int64_t pts = m_pkt->pts == AV_NOPTS_VALUE ? 0 : m_pkt->pts;
    const int64_t dts = m_pkt->dts == AV_NOPTS_VALUE ? pts : m_pkt->dts;
    const auto duration = m_pkt->duration;
    const auto copyErr = output->copy(m_pkt->data, m_pkt->size, pts, dts, duration);
    if (copyErr != RGY_ERR_NONE) return copyErr;
    if (m_codec == RGY_CODEC_AV1) {
        const auto units = parse_unit_av1(output->data(), output->size());
        const auto hasTemporalDelimiter = std::find_if(units.begin(), units.end(), [](const auto& unit) {
            return unit->type == OBU_TEMPORAL_DELIMITER;
        }) != units.end();
        if (!hasTemporalDelimiter) {
            // 後段はTemporal DelimiterでAV1フレームを分割するため、VAAPI出力に無い場合は補う。
            std::vector<uint8_t> packetWithTemporalDelimiter{ 0x12, 0x00 };
            packetWithTemporalDelimiter.insert(packetWithTemporalDelimiter.end(), output->data(), output->data() + output->size());
            const auto prependErr = output->copy(packetWithTemporalDelimiter.data(), packetWithTemporalDelimiter.size(), pts, dts, duration);
            if (prependErr != RGY_ERR_NONE) return prependErr;
        }
    }
    output->setFrametype((m_pkt->flags & AV_PKT_FLAG_KEY) ? RGY_FRAMETYPE_IDR : RGY_FRAMETYPE_P);
    bs = std::move(output);
    av_packet_unref(m_pkt.get());
    return RGY_ERR_NONE;
}

tstring VCEEncoderVA::paramString() const {
    if (!m_codecCtx) return _T("VA-API encoder is not initialized.");
    const TCHAR *rc = m_rateControl == get_codec_cqp(m_codec) ? _T("CQP") : m_rateControl == get_codec_qvbr(m_codec) ? _T("QVBR") : m_rateControl == get_codec_cbr(m_codec) || m_rateControl == get_codec_hqcbr(m_codec) ? _T("CBR") : _T("VBR");
    const TCHAR *preset = va_preset_name(m_preset);
    const tstring level = m_codecCtx->level == AV_LEVEL_UNKNOWN ? tstring(_T("auto")) : strsprintf(_T("%d"), m_codecCtx->level);
    tstring rcDetails;
    if (m_rateControl == get_codec_cqp(m_codec)) {
        const auto qpP = m_codecCtx->global_quality / FF_QP2LAMBDA;
        const auto qpI = (int)(qpP * m_codecCtx->i_quant_factor);
        const auto qpB = (int)(qpP * m_codecCtx->b_quant_factor);
        rcDetails = strsprintf(_T("QP (I/P/B):    %d/%d/%d"), qpI, qpP, qpB);
    } else if (m_rateControl == get_codec_qvbr(m_codec)) {
        rcDetails = strsprintf(_T("QVBR QP:       %d\nBitrate:       %lld kbps"), m_qp, (long long)(m_codecCtx->bit_rate / 1000));
    } else {
        rcDetails = strsprintf(_T("Bitrate:       %lld kbps\nMax bitrate:   %lld kbps\nVBV buffer:    %d kbit"),
            (long long)(m_codecCtx->bit_rate / 1000), (long long)(m_codecCtx->rc_max_rate / 1000), m_codecCtx->rc_buffer_size / 1000);
    }
    return strsprintf(_T("Codec:         %s\nResolution:    %dx%d\nFrame rate:    %d/%d\nRate control:  %s\n%s\nB frames:      %d\nRef frames:    %d\nProfile:       %d\nLevel:         %s\nPreset:        %s (compression_level=%d)\nGOP:           %d"),
        CodecToStr(m_codec).c_str(), m_width, m_height, m_codecCtx->framerate.num, m_codecCtx->framerate.den,
        rc, rcDetails.c_str(), m_bframes, m_refs, m_codecCtx->profile, level.c_str(), preset, m_preset, m_codecCtx->gop_size);
}

#endif // ENABLE_VAAPI
