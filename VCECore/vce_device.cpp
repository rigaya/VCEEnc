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

#include "vce_device.h"
#include "vce_util.h"

const wchar_t *VCEDevice::CAP_10BITDEPTH = L"CAP_10BITDEPTH";

VCEDevice::VCEDevice(shared_ptr<RGYLog> &log, amf::AMFFactory *factory, amf::AMFTrace *trace) :
    m_log(log),
    m_id(-1),
    m_devName(),
    m_d3d9interlop(false),
#if ENABLE_D3D9
    m_dx9(),
#endif //#if ENABLE_D3D9
    m_d3d11interlop(false),
#if ENABLE_D3D11
    m_dx11(),
#endif //#if ENABLE_D3D11
    m_vulkaninterlop(false),
#if ENABLE_VULKAN
    m_vk(),
#endif
    m_cl(),
    m_context(),
    m_factory(factory),
    m_trace(trace),
    m_gotCaps(false),
    m_encCaps(),
    m_decCaps() {

}

VCEDevice::~VCEDevice() {
    m_context.Release();
    m_factory = nullptr;
    m_trace = nullptr;

    m_cl.reset();
#if ENABLE_VULKAN
    m_vk.Terminate();
#endif
#if ENABLE_D3D11
    m_dx11.Terminate();
#endif //#if ENABLE_D3D11
#if ENABLE_D3D9
    m_dx9.Terminate();
#endif //#if ENABLE_D3D9
    m_devName.clear();
    m_log.reset();
}

RGY_ERR VCEDevice::CreateContext() {
    auto res = m_factory->CreateContext(&m_context);
    if (res != AMF_OK) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to CreateContext(): %s.\n"), get_err_mes(err_to_rgy(res)));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("CreateContext() Success.\n"));
    return RGY_ERR_NONE;
}

RGY_ERR VCEDevice::init(const int deviceId, const bool interopD3d9, const bool interopD3d11, const bool interopVulkan, const bool enableOpenCL, const bool enableVppPerfMonitor) {
    m_devName = strsprintf(_T("device #%d"), deviceId);
    m_id = deviceId;
    {
        auto err = CreateContext();
        if (err != RGY_ERR_NONE) {
            return err;
        }
    }
    m_d3d9interlop = interopD3d9;
    m_d3d11interlop = interopD3d11;
    m_vulkaninterlop = interopVulkan;
#if ENABLE_VULKAN
    if (interopVulkan) {
        auto amferr = AMF_OK;
        if (VULKAN_DEFAULT_DEVICE_ONLY) {
            amferr = amf::AMFContext1Ptr(m_context)->InitVulkan(NULL);
        } else {
            //現状これをやるとなぜか異常終了してしまう
            auto err = m_vk.Init(deviceId, m_context.GetPtr(), m_log);
            if (err != RGY_ERR_NONE) {
                PrintMes(RGY_LOG_ERROR, _T("Failed to get vulkan device.\n"));
                return RGY_ERR_DEVICE_LOST;
            }
            amferr = amf::AMFContext1Ptr(m_context)->InitVulkan(m_vk.GetDevice());
        }
        if (amferr != AMF_OK) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to init AMF context by vulkan.\n"));
            return err_to_rgy(amferr);
        }
    }
#endif
#if ENABLE_D3D9
    if (interopD3d9) {
        auto err = m_dx9.Init(true, deviceId, false, 1280, 720, m_log);
        if (err != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to get directX9 device.\n"));
            return RGY_ERR_DEVICE_LOST;
        }
        PrintMes(RGY_LOG_DEBUG, _T("dx9.Init() Success.\n"));

        auto amferr = m_context->InitDX9(m_dx9.GetDevice());
        if (amferr != AMF_OK) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to init AMF context by directX9.\n"));
            return err_to_rgy(amferr);
        }
        PrintMes(RGY_LOG_DEBUG, _T("init AMF context by directX9: Success.\n"));
    }
#endif //#if ENABLE_D3D9
#if ENABLE_D3D11
    if (interopD3d11 || !interopD3d9) {
        auto err = m_dx11.Init(deviceId, false, m_log);
        if (err != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to get directX11 device.\n"));
            return RGY_ERR_DEVICE_LOST;
        }
        PrintMes(RGY_LOG_DEBUG, _T("dx11.Init() Success.\n"));

        auto amferr = m_context->InitDX11(m_dx11.GetDevice());
        if (amferr != AMF_OK) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to init AMF context by directX11.\n"));
            return err_to_rgy(amferr);
        }
        PrintMes(RGY_LOG_DEBUG, _T("init AMF context by directX11: Success.\n"));
    }
#endif //#if ENABLE_D3D11

    if (enableOpenCL) {
        RGYOpenCL cl(m_log);
        auto platforms = cl.getPlatforms("AMD");
        if (platforms.size() == 0) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to find AMD OpenCL platforms.\n"));
            return RGY_ERR_DEVICE_LOST;
        }
        int totalDevices = 0;
        int selectCLDevice = 0;
        std::shared_ptr<RGYOpenCLPlatform> selectedPlatform;
        for (auto& platform : platforms) {
            PrintMes(RGY_LOG_DEBUG, _T("Checking platform %s...\n"), char_to_tstring(platform->info().name).c_str());
#if ENABLE_D3D9
            if (interopD3d9) {
                if (platform->createDeviceListD3D9(CL_DEVICE_TYPE_GPU, (void *)m_dx9.GetDevice()) != RGY_ERR_NONE || platform->devs().size() == 0) {
                    PrintMes(RGY_LOG_ERROR, _T("Failed to find d3d9 device.\n"));
                    return RGY_ERR_DEVICE_LOST;
                }
            } else
#endif //#if ENABLE_D3D9
#if ENABLE_D3D11
            if (interopD3d11 || !interopD3d9) {
                if (platform->createDeviceListD3D11(CL_DEVICE_TYPE_GPU, (void *)m_dx11.GetDevice()) != RGY_ERR_NONE || platform->devs().size() == 0) {
                    PrintMes(RGY_LOG_ERROR, _T("Failed to find d3d11 device.\n"));
                    return RGY_ERR_DEVICE_LOST;
                }
            } else
#endif //#if ENABLE_D3D11
            {
                auto ret = platform->createDeviceList(CL_DEVICE_TYPE_GPU);
                if (ret != RGY_ERR_NONE || platform->devs().size() == 0) {
                    if (ret == RGY_ERR_DEVICE_NOT_FOUND) {
                        PrintMes(RGY_LOG_INFO, _T("GPU device not found on platform %s.\n"), char_to_tstring(platform->info().name).c_str());
                        continue;
                    }
                    PrintMes(RGY_LOG_ERROR, _T("Failed to find gpu device.\n"));
                    return RGY_ERR_DEVICE_LOST;
                }
            }
            selectCLDevice = (interopD3d9 || interopD3d11) ? 0 : deviceId - totalDevices;
            auto devices = platform->devs();
            totalDevices += (int)devices.size();
            if (selectCLDevice < (int)devices.size()) {
                selectedPlatform = platform;
                break;
            }
            PrintMes(RGY_LOG_DEBUG, _T("Target device #%d not found in platform %s.\n"), deviceId, char_to_tstring(platform->info().name).c_str());
        }
        if (!selectedPlatform) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to find device #%d.\n"), deviceId);
            return RGY_ERR_DEVICE_LOST;
        }
        selectedPlatform->setDev(selectedPlatform->devs()[selectCLDevice],
#if ENABLE_D3D9
            (interopD3d9) ? m_dx9.GetDevice() :
#endif //#if ENABLE_D3D9
            nullptr,
#if ENABLE_D3D11
            (interopD3d11) ? m_dx11.GetDevice() :
#endif //#if ENABLE_D3D11
            nullptr);

        m_cl = std::make_shared<RGYOpenCLContext>(selectedPlatform, m_log);
        if (m_cl->createContext((enableVppPerfMonitor) ? CL_QUEUE_PROFILING_ENABLE : 0) != CL_SUCCESS) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to create OpenCL context.\n"));
            return RGY_ERR_UNKNOWN;
        }
        auto amferr = m_context->InitOpenCL(m_cl->queue().get());
        if (amferr != AMF_OK) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to init AMF context by OpenCL.\n"));
            return err_to_rgy(amferr);
        }
    }

    m_devName = getGPUInfo();
    return RGY_ERR_NONE;
}

void VCEDevice::getAllCaps() {
    //エンコーダの情報
    for (int i = 0; list_codec[i].desc; i++) {
        const auto codec = (RGY_CODEC)list_codec[i].value;
        getEncCaps(codec);
    }
    //デコーダの情報
    for (size_t i = 0; i < _countof(HW_DECODE_LIST); i++) {
        const auto codec = HW_DECODE_LIST[i].rgy_codec;
        getDecCaps(codec);
    }
}

amf::AMFCapsPtr VCEDevice::getEncCaps(RGY_CODEC codec) {
    if (m_encCaps.count(codec) == 0) {
        m_encCaps[codec] = amf::AMFCapsPtr();
        amf::AMFCapsPtr encoderCaps;
        auto ret = AMF_OK;
        {   amf::AMFComponentPtr p_encoder;
            ret = m_factory->CreateComponent(m_context, codec_rgy_to_enc(codec), &p_encoder);
            if (ret == AMF_OK) {
                //HEVCでのAMFComponent::GetCaps()は、AMFComponent::Init()を呼んでおかないと成功しない
                p_encoder->Init(amf::AMF_SURFACE_NV12, 1280, 720);
                ret = p_encoder->GetCaps(&encoderCaps);
                encoderCaps->SetProperty(CAP_10BITDEPTH, false);
                p_encoder->Terminate();
            }
        }
        //10bit深度のチェック
        if (ret == AMF_OK
            && (codec == RGY_CODEC_HEVC || codec == RGY_CODEC_AV1)) {
            amf::AMFComponentPtr p_encoder;
            if (m_factory->CreateComponent(m_context, codec_rgy_to_enc(codec), &p_encoder) == AMF_OK) {
                const int dummy_width  = 1920;
                const int dummy_height = 1080;

                AMFParams params;
                params.SetParamTypeCodec(codec);
                params.SetParam(VCE_PARAM_KEY_INPUT_WIDTH, dummy_width);
                params.SetParam(VCE_PARAM_KEY_INPUT_HEIGHT, dummy_height);
                params.SetParam(VCE_PARAM_KEY_OUTPUT_WIDTH, dummy_width);
                params.SetParam(VCE_PARAM_KEY_OUTPUT_HEIGHT, dummy_height);
                params.SetParam(AMF_PARAM_FRAMESIZE(codec), AMFConstructSize(dummy_width, dummy_height));
                params.SetParam(AMF_PARAM_FRAMERATE(codec), AMFConstructRate(30, 1));
                params.SetParam(AMF_PARAM_COLOR_BIT_DEPTH(codec), (amf_int64)10);
                if (codec == RGY_CODEC_AV1) {
                    //これをいれないと、1920x1080などの解像度が正常に扱えない
                    params.SetParam(AMF_VIDEO_ENCODER_AV1_ALIGNMENT_MODE, (amf_int64)AMF_VIDEO_ENCODER_AV1_ALIGNMENT_MODE_NO_RESTRICTIONS);
                }

                // Usage is preset that will set many parameters
                params.Apply(p_encoder, AMF_PARAM_ENCODER_USAGE);
                // override some usage parameters
                params.Apply(p_encoder, AMF_PARAM_STATIC);
                //HEVCでのAMFComponent::GetCaps()は、AMFComponent::Init()を呼んでおかないと成功しない
                if (p_encoder->Init(amf::AMF_SURFACE_P010, dummy_width, dummy_height) == AMF_OK
                    && p_encoder->GetCaps(&encoderCaps) == AMF_OK) {
                    encoderCaps->SetProperty(CAP_10BITDEPTH, true);
                    p_encoder->Terminate();
                }
            }
        }
        m_encCaps[codec] = encoderCaps;
    }
    return m_encCaps[codec];
}

amf::AMFCapsPtr VCEDevice::getDecCaps(RGY_CODEC codec) {
    if (m_decCaps.count(codec) == 0) {
        const auto codec_uvd_name = codec_rgy_to_dec(codec);
        m_decCaps[codec] = amf::AMFCapsPtr();
        if (codec_uvd_name != nullptr) {
            amf::AMFCapsPtr decodeCaps;
            amf::AMFComponentPtr p_decode;
            if (m_factory->CreateComponent(m_context, codec_uvd_name, &p_decode) == AMF_OK
                && p_decode->GetCaps(&decodeCaps) == AMF_OK) {
                // 10bit深度のサポートのチェック
                decodeCaps->SetProperty(CAP_10BITDEPTH, false);
                const auto codec_uvd_10bit_name = codec_rgy_to_dec_10bit(codec);
                if (codec_uvd_10bit_name != nullptr) {
                    amf::AMFCapsPtr decodeCaps10bit;
                    amf::AMFComponentPtr p_decode10bit;
                    if (m_factory->CreateComponent(m_context, codec_uvd_10bit_name, &p_decode10bit) == AMF_OK
                        && p_decode10bit->GetCaps(&decodeCaps10bit) == AMF_OK) {
                        decodeCaps->SetProperty(CAP_10BITDEPTH, true);
                        p_decode10bit->Terminate();
                    }
                }
                m_decCaps[codec] = decodeCaps;
                p_decode->Terminate();
            }
        }
    }
    return m_decCaps[codec];
}

std::vector<RGY_CSP> VCEDevice::getIOCspSupport(amf::AMFIOCapsPtr& ioCaps) const {
    std::vector<RGY_CSP> csps;
    const auto numOfFormats = ioCaps->GetNumOfFormats();
    for (int ifmt = 0; ifmt < numOfFormats; ifmt++) {
        amf::AMF_SURFACE_FORMAT format = amf::AMF_SURFACE_UNKNOWN;
        amf_bool native = false;
        if (ioCaps->GetFormatAt(ifmt, &format, &native) == AMF_OK && native) {
            auto csp = csp_enc_to_rgy(format);
            if (csp != RGY_CSP_NA) {
                csps.push_back(csp);
            }
        }
    }
    return csps;
}

tstring VCEDevice::QueryIOCaps(amf::AMFIOCapsPtr& ioCaps) {
    tstring str;
    bool result = true;
    if (ioCaps != NULL) {
        amf_int32 minWidth, maxWidth;
        ioCaps->GetWidthRange(&minWidth, &maxWidth);
        str += strsprintf(_T("Width:       %d - %d\n"), minWidth, maxWidth);

        amf_int32 minHeight, maxHeight;
        ioCaps->GetHeightRange(&minHeight, &maxHeight);
        str += strsprintf(_T("Height:      %d - %d\n"), minHeight, maxHeight);

        amf_int32 vertAlign = ioCaps->GetVertAlign();
        str += strsprintf(_T("alignment:   %d\n"), vertAlign);

        amf_bool interlacedSupport = ioCaps->IsInterlacedSupported();
        str += strsprintf(_T("Interlace:   %s\n"), interlacedSupport ? _T("yes") : _T("no"));

        amf_int32 numOfFormats = ioCaps->GetNumOfFormats();
        str += _T("pix format:  ");
        for (amf_int32 i = 0; i < numOfFormats; i++) {
            amf::AMF_SURFACE_FORMAT format = amf::AMF_SURFACE_UNKNOWN;
            amf_bool native = false;
            if (ioCaps->GetFormatAt(i, &format, &native) == AMF_OK) {
                if (i) str += _T(", ");
                str += wstring_to_tstring(m_trace->SurfaceGetFormatName(format)) + strsprintf(_T("[%d]"), (int)format) + ((native) ? _T("(native)") : _T(""));
            }
        }
        str += _T("\n");

        if (result == true) {
            amf_int32 numOfMemTypes = ioCaps->GetNumOfMemoryTypes();
            str += _T("memory type: ");
            for (amf_int32 i = 0; i < numOfMemTypes; i++) {
                amf::AMF_MEMORY_TYPE memType = amf::AMF_MEMORY_UNKNOWN;
                amf_bool native = false;
                if (ioCaps->GetMemoryTypeAt(i, &memType, &native) == AMF_OK) {
                    if (i) str += _T(", ");
                    str += wstring_to_tstring(m_trace->GetMemoryTypeName(memType)) + ((native) ? _T("(native)") : _T(""));
                }
            }
        }
        str += _T("\n");
    } else {
        str += _T("failed to get io capability\n");
    }
    return str;
}

tstring VCEDevice::QueryInputCaps(RGY_CODEC codec, amf::AMFCapsPtr& caps) {
    tstring str;
    if (caps == NULL) {
        str += _T("failed to get input capability\n");
    }
    str += strsprintf(_T("\n%s input:\n"), CodecToStr(codec).c_str());
    amf::AMFIOCapsPtr inputCaps;
    if (caps->GetInputCaps(&inputCaps) == AMF_OK) {
        str += QueryIOCaps(inputCaps);
    }
    return str;
}

tstring VCEDevice::QueryOutputCaps(RGY_CODEC codec, amf::AMFCapsPtr& caps) {
    tstring str;
    if (caps == NULL) {
        str += _T("failed to get output capability\n");
    }

    str += strsprintf(_T("\n%s output:\n"), CodecToStr(codec).c_str());
    amf::AMFIOCapsPtr outputCaps;
    if (caps->GetOutputCaps(&outputCaps) == AMF_OK) {
        str += QueryIOCaps(outputCaps);
    }
    return str;
}

tstring VCEDevice::QueryIOCaps(RGY_CODEC codec, amf::AMFCapsPtr& caps) {
    return QueryInputCaps(codec, caps) + QueryOutputCaps(codec, caps);
}

tstring VCEDevice::QueryEncCaps(RGY_CODEC codec, amf::AMFCapsPtr& encoderCaps) {
    tstring str;
    if (encoderCaps == NULL) {
        str += _T("failed to get encoder capability\n");
    }

    bool Support10bitDepth = false;
    if (encoderCaps->GetProperty(CAP_10BITDEPTH, &Support10bitDepth) == AMF_OK) {
        str += strsprintf(_T("10bit depth:     %s\n"), (Support10bitDepth) ? _T("yes") : _T("no"));
    }

    amf::AMF_ACCELERATION_TYPE accelType = encoderCaps->GetAccelerationType();
    str += _T("acceleration:    ") + AccelTypeToString(accelType) + _T("\n");

    amf_uint32 maxProfile = 0;
    encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_PROFILE(codec), &maxProfile);
    auto profile_desc = get_cx_desc(get_profile_list(codec), maxProfile);
    str += _T("max profile:     ") + tstring(profile_desc ? profile_desc : _T("unknown")) + _T("\n");

    amf_uint32 maxLevel = 0;
    encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_LEVEL(codec), &maxLevel);
    auto level_desc = get_cx_desc(get_profile_list(codec), maxLevel);
    str += _T("max level:       ") + tstring(level_desc ? level_desc : _T("unknown")) + _T("\n");

    amf_uint32 maxBitrate = 0;
    encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_BITRATE(codec), &maxBitrate);
    str += strsprintf(_T("max bitrate:     %d kbps\n"), maxBitrate / 1000);

    if (codec == RGY_CODEC_H264 || codec == RGY_CODEC_HEVC) {
        amf_uint32 maxRef = 0, minRef = 0;
        encoderCaps->GetProperty(AMF_PARAM_CAP_MIN_REFERENCE_FRAMES(codec), &minRef);
        encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_REFERENCE_FRAMES(codec), &maxRef);
        str += strsprintf(_T("ref frames:      %d-%d\n"), minRef, maxRef);
    }

    if (codec == RGY_CODEC_H264) {
        //amf_uint32 maxTemporalLayers = 0;
        //encoderCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_MAX_TEMPORAL_LAYERS, &maxTemporalLayers);
        //str += strsprintf(_T("max temp layers:  %d\n"), maxTemporalLayers);

        bool bBPictureSupported = false;
        encoderCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_BFRAMES, &bBPictureSupported);
        str += strsprintf(_T("Bframe support:  %s\n"), (bBPictureSupported) ? _T("yes") : _T("no"));

        amf_uint32 NumOfHWInstances = 0;
        encoderCaps->GetProperty(AMF_VIDEO_ENCODER_CAP_NUM_OF_HW_INSTANCES, &NumOfHWInstances);
        str += strsprintf(_T("HW instances:    %d\n"), NumOfHWInstances);
    } else if (codec == RGY_CODEC_HEVC) {
        //いまは特になし
    } else if (codec == RGY_CODEC_AV1) {
        amf_uint32 maxTemporalLayers = 0;
        encoderCaps->GetProperty(AMF_VIDEO_ENCODER_AV1_CAP_MAX_NUM_TEMPORAL_LAYERS, &maxTemporalLayers);
        str += strsprintf(_T("Temporal layers: %d\n"), maxTemporalLayers);
    }

    amf_bool preAnalysis = 0;
    encoderCaps->GetProperty(AMF_PARAM_CAP_PRE_ANALYSIS(codec), &preAnalysis);
    str += strsprintf(_T("pre analysis:    %s\n"), (preAnalysis) ? _T("yes") : _T("no"));

    amf_uint32 maxNumOfStreams = 0;
    encoderCaps->GetProperty(AMF_PARAM_CAP_NUM_OF_STREAMS(codec), &maxNumOfStreams);
    str += strsprintf(_T("max streams:     %d\n"), maxNumOfStreams);

    //amf_uint32 throughputMax = 0;
    //encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_THROUGHPUT(codec), &throughputMax);
    //str += strsprintf(_T("max throughput:    %d 16x16MB\n"), throughputMax);

    //amf_uint32 throughputRequested = 0;
    //encoderCaps->GetProperty(AMF_PARAM_CAP_REQUESTED_THROUGHPUT(codec), &throughputRequested);
    //str += strsprintf(_T("requested throughput:    %d 16x16 MB\n"), throughputRequested);

    bool queryTimeout = false;
    encoderCaps->GetProperty(AMF_PARAM_CAPS_QUERY_TIMEOUT_SUPPORT(codec), &queryTimeout);
    str += strsprintf(_T("timeout support: %s\n"), (queryTimeout) ? _T("yes") : _T("no"));

    str += QueryIOCaps(codec, encoderCaps);

    return str;
}

tstring VCEDevice::QueryDecCaps(RGY_CODEC codec, amf::AMFCapsPtr& decoderCaps) {
    tstring str;
    if (decoderCaps == NULL) {
        str += _T("failed to get decoder capability\n");
    }

    bool Support10bitDepth = false;
    if (decoderCaps->GetProperty(CAP_10BITDEPTH, &Support10bitDepth) == AMF_OK) {
        str += strsprintf(_T("10bit depth:     %s\n"), (Support10bitDepth) ? _T("yes") : _T("no"));
    }

    amf::AMF_ACCELERATION_TYPE accelType = decoderCaps->GetAccelerationType();
    str += _T("acceleration:    ") + AccelTypeToString(accelType) + _T("\n");

    amf_uint32 maxNumOfStreams = 0;
    decoderCaps->GetProperty(AMF_PARAM_CAP_NUM_OF_STREAMS(codec), &maxNumOfStreams);
    str += strsprintf(_T("max streams:     %d\n"), maxNumOfStreams);

    //amf_uint32 throughputMax = 0;
    //encoderCaps->GetProperty(AMF_PARAM_CAP_MAX_THROUGHPUT(codec), &throughputMax);
    //str += strsprintf(_T("max throughput:    %d 16x16MB\n"), throughputMax);

    //amf_uint32 throughputRequested = 0;
    //encoderCaps->GetProperty(AMF_PARAM_CAP_REQUESTED_THROUGHPUT(codec), &throughputRequested);
    //str += strsprintf(_T("requested throughput:    %d 16x16 MB\n"), throughputRequested);

    str += QueryOutputCaps(codec, decoderCaps);

    return str;
}

tstring VCEDevice::getGPUInfo() const {
#if ENABLE_D3D11
    if (m_dx11.isValid()) {
        auto str = m_dx11.GetDisplayDeviceName();
        str = str_replace(str, L"(TM)", L"");
        str = str_replace(str, L"(R)", L"");
        str = str_replace(str, L" Series", L"");
        str = str_replace(str, L" Graphics", L"");
        return wstring_to_tstring(str);
    }
#endif //#if ENABLE_D3D11
    if (m_cl) {
        return RGYOpenCLDevice(m_cl->platform()->dev(0)).infostr();
    }
    return _T("");
}

CodecCsp VCEDevice::getHWDecCodecCsp() {
#if ENABLE_AVSW_READER
    CodecCsp codecCsp;
    for (int i = 0; i < _countof(HW_DECODE_LIST); i++) {
        amf::AMFCapsPtr decCaps = getDecCaps(HW_DECODE_LIST[i].rgy_codec);
        if (decCaps != nullptr) {
#if defined(_WIN32) || defined(_WIN64)
            amf::AMFIOCapsPtr outputCaps;
            if (decCaps->GetOutputCaps(&outputCaps) == AMF_OK) {
                auto csps = getIOCspSupport(outputCaps);
                // 10bitサポートのチェック
                if (std::find(csps.begin(), csps.end(), RGY_CSP_P010) == csps.end()) {
                    bool Support10bitDepth = false;
                    if (decCaps->GetProperty(CAP_10BITDEPTH, &Support10bitDepth) == AMF_OK && Support10bitDepth) {
                        csps.push_back(RGY_CSP_P010);
                        csps.push_back(RGY_CSP_YV12_10);
                    }
                }
                if (std::find(csps.begin(), csps.end(), RGY_CSP_NV12) != csps.end()) {
                    csps.push_back(RGY_CSP_YV12);
                }
                codecCsp[HW_DECODE_LIST[i].rgy_codec] = csps;
            }
#else
            //現状まともにoutputCapsが得られないので、適当に作る
            std::vector<RGY_CSP> csps = { RGY_CSP_NV12, RGY_CSP_YV12 };
            bool Support10bitDepth = false;
            if (decCaps->GetProperty(CAP_10BITDEPTH, &Support10bitDepth) == AMF_OK && Support10bitDepth) {
                csps.push_back(RGY_CSP_P010);
                csps.push_back(RGY_CSP_YV12_10);
            }
            codecCsp[HW_DECODE_LIST[i].rgy_codec] = csps;
#endif
        }
    }
    return codecCsp;
#else
    return CodecCsp();
#endif
}

LUID VCEDevice::luid() const {
    return
#if ENABLE_D3D11
    (m_dx11.isValid()) ? m_dx11.getLUID() :
#endif //#if ENABLE_D3D11
#if ENABLE_D3D9
    m_dx9.getLUID();
#else
    LUID();
#endif
}
