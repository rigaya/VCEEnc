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

#pragma once

#include "rgy_version.h"

#if ENABLE_VAAPI

#include <unordered_map>
#include <vector>
#include "rgy_avutil.h"
#include "rgy_def.h"
#include "rgy_err.h"
#include "rgy_log.h"

class RGYFrame;
struct RGYBitstream;
struct VCEParam;

enum VCEVAEncRCMode : uint32_t {
    VCE_VA_RC_CBR  = 1u << 0,
    VCE_VA_RC_VBR  = 1u << 1,
    VCE_VA_RC_CQP  = 1u << 2,
    VCE_VA_RC_QVBR = 1u << 3,
};

struct VCEVADeviceInfo {
    int         id = -1;
    tstring     renderNode;
    std::string pciBusId;
    tstring     name;
};

struct VCEVAEncCaps {
    bool     available;
    bool     support10bit;
    uint32_t rcModes;
    int      maxRefL0;
    int      maxRefL1;
    int      maxWidth;
    int      maxHeight;

    VCEVAEncCaps();
};

std::vector<VCEVADeviceInfo> enumerateVADevices(RGYLog *log);

class VCEDeviceVA {
public:
    VCEDeviceVA();
    ~VCEDeviceVA();
    RGY_ERR open(const VCEVADeviceInfo& info, std::shared_ptr<RGYLog> log);
    const VCEVAEncCaps& encCaps(RGY_CODEC codec);
    tstring capsString(RGY_CODEC codec);
    const VCEVADeviceInfo& info() const { return m_info; }
    AVBufferRef *hwdevice() { return m_hwdevice.get(); }
protected:
    VCEVADeviceInfo m_info;
    std::unique_ptr<AVBufferRef, RGYAVDeleter<AVBufferRef>> m_hwdevice;
    void *m_display;
    std::unordered_map<RGY_CODEC, VCEVAEncCaps> m_encCaps;
    std::shared_ptr<RGYLog> m_log;
};

class VCEEncoderVA {
public:
    VCEEncoderVA();
    ~VCEEncoderVA();
    RGY_ERR init(VCEDeviceVA *dev, const VCEParam *prm, int width, int height,
        AVRational fps, AVRational timebase, std::shared_ptr<RGYLog> log);
    RGY_ERR submit(RGYFrame *frame);
    RGY_ERR receive(std::shared_ptr<RGYBitstream>& bs);
    tstring paramString() const;
    int width() const { return m_width; }
    int height() const { return m_height; }
    int bitdepth() const { return m_bitdepth; }
    int videoDelay() const { return (m_codec != RGY_CODEC_AV1 && m_bframes > 0) ? 1 : 0; }
protected:
    std::unique_ptr<AVCodecContext, RGYAVDeleter<AVCodecContext>> m_codecCtx;
    std::unique_ptr<AVBufferRef, RGYAVDeleter<AVBufferRef>> m_hwframes;
    std::unique_ptr<AVFrame, RGYAVDeleter<AVFrame>> m_frameHW;
    std::unique_ptr<AVFrame, RGYAVDeleter<AVFrame>> m_frameSW;
    std::unique_ptr<AVPacket, RGYAVDeleter<AVPacket>> m_pkt;
    std::shared_ptr<RGYLog> m_log;
    RGY_CODEC m_codec;
    int m_width, m_height, m_bitdepth;
    int m_rateControl;
    int m_qp;
    int m_bframes;
    int m_refs;
    int m_preset;
    AVRational m_timebase;
};

#endif // ENABLE_VAAPI
