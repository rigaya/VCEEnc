// -----------------------------------------------------------------------------------------
//     QSVEnc/VCEEnc/rkmppenc by rigaya
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

#pragma once
#ifndef __RGY_FILTER_ONNX_H__
#define __RGY_FILTER_ONNX_H__

#include "rgy_filter_cl.h"
#include "rgy_prm.h"
#include "rgy_onnxrt_dml.h"
#include <vector>
#include <memory>
#include <deque>

class RGYFilterResize; // opt-in end-of-chain resize sub-filter (out_res=/resize=)

class RGYFilterParamOnnx : public RGYFilterParam {
public:
    VppOnnx onnx;
    tstring modelDir;
    int sar[2] = { 0, 0 };  // input SAR (set by pipeline) -- resolves a negative out_res= (auto-aspect) DAR-correctly
    // LUID of the GPU adapter VCEEnc selected, split for a windows.h-free header.
    // DirectML is vendor agnostic, so this is used to bind inference to the same
    // adapter as the encoder rather than DirectML's default (adapter 0).
    uint32_t adapterLuidLow  = 0;
    int32_t  adapterLuidHigh = 0;
    RGYFilterParamOnnx() : onnx(), modelDir() {};
    virtual ~RGYFilterParamOnnx() {};
    virtual tstring print() const override;
};

// The pre/post a model needs, inferred from its input/output channel count.
// ONNX Runtime DirectML runs the network with no per-model code; this enum only
// selects how pixels are packed into / unpacked from the tensor.
enum class OnnxIO {
    LumaSR,     // in1  -> out1 : Y plane through the net, integer scale, chroma bilinear-resampled (ArtCNN, vgg7-Y)
    GrayNoise,  // in2  -> out1 : [Y, sigma] through the net, scale=1, chroma copied (DRUNet gray)
    Chroma,     // in3  -> out2 : [Y, Cb, Cr] -> refined [Cb, Cr], scale=1, luma copied (ArtCNN Chroma)
    RGB,        // in3  -> out3 : YUV<->RGB bookend, integer scale (Real-ESRGAN / waifu2x / Real-CUGAN / BSRGAN / ArtCNN RGB)
    RGBNoise,   // in4  -> out3 : [R, G, B, sigma] -> RGB, integer scale (DPSR, DRUNet color)
};

// Standalone ONNX Runtime DirectML-backed CNN VPP filter. The network's I/O
// convention is inferred from its channel count (OnnxIO), so the same
// load-and-run covers every model family with no per-model code.
//
// The host-readback path (map input to host, build the input tensor, run the
// network, write the result back) handles all I/O modes.
class RGYFilterOnnx : public RGYFilter {
public:
    RGYFilterOnnx(shared_ptr<RGYOpenCLContext> context);
    virtual ~RGYFilterOnnx();
    virtual RGY_ERR init(shared_ptr<RGYFilterParam> pParam, shared_ptr<RGYLog> pPrintMes) override;
protected:
    RGY_ERR checkParam(const std::shared_ptr<RGYFilterParamOnnx> prm);
    virtual RGY_ERR run_filter(const RGYFrameInfo *pInputFrame, RGYFrameInfo **ppOutputFrames, int *pOutputFrameNum,
        RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event) override;
    virtual void close() override;

    // host-readback path: map the input to the host, build the input tensor, run
    // the network on DirectML, write the result back. Works for every OnnxIO mode
    // and is the only path for the DirectML backend.
    RGY_ERR runHost(const RGYFrameInfo *in, RGYFrameInfo *out,
        RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event);

    // host pre/post, dispatched on m_io. fillInputHost packs the mapped input
    // frame into m_inBuf (inC*inW*inH, CHW); writeOutputHost unpacks m_outBuf
    // (outC*outW*outH, CHW) into the mapped output frame.
    void fillInputHost(const RGYFrameInfo &hin);
    void writeOutputHost(const RGYFrameInfo &hout, const RGYFrameInfo &hin);
    // compute the YUV<->RGB matrix + range coefficients.
    void setupColorCoeffs(int matrixSelIn, int matrixSelOut, bool rangeTV, int pixMax);

    // --- multi-frame temporal window (frames= > 1) ---------------------------
    // Pack one mapped input frame's YUV as planar RGB [0,1] CHW
    // (3*inW*inH floats) into dst.
    void packFrameRGB(const RGYFrameInfo &hin, float *dst);
    // Temporal run path: buffer the last T RGB frames and emit a centred window
    // with edge replication (1-in-1-out with (T-1)/2 delay); flush drains the tail.
    RGY_ERR runTemporal(const RGYFrameInfo *pInputFrame, RGYFrameInfo **ppOutputFrames, int *pOutputFrameNum,
        RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event);
    RGY_ERR emitTemporalOutput(int64_t outIdx, RGYFrameInfo **ppOutputFrames, int *pOutputFrameNum,
        RGYOpenCLQueue &queue, RGYOpenCLEvent *event);

    // --- two-input inpainting mode (mask=) -----------------------------------
    RGY_ERR initMask(std::shared_ptr<RGYFilterParamOnnx> prm, const int inW, const int inH, const RGY_CSP inCsp);
    RGY_ERR runMask(const RGYFrameInfo *pInputFrame, RGYFrameInfo **ppOutputFrames, int *pOutputFrameNum,
        RGYOpenCLQueue &queue, const std::vector<RGYOpenCLEvent> &wait_events, RGYOpenCLEvent *event);

    std::unique_ptr<RGYOnnxRTDML> m_ov;
    OnnxIO m_io;                          // I/O convention inferred from channel counts
    int   m_inC, m_outC;                        // model input / output channel counts
    int   m_scale;                              // integer upscale factor from the model (out/in)
    float m_maxval;                             // (1<<bitdepth)-1
    bool  m_ycbcr;                              // 3ch model fed as planar YCbCr instead of RGB
    float m_sigmaNorm;                          // noise sigma / 255 for the conditioning channel

    // colour coefficients (computed once at init)
    float m_yOff, m_yScale, m_yRange, m_cOff, m_cScale, m_cRange;
    float m_matVR, m_matUG, m_matVG, m_matUB;                                   // YUV -> RGB
    float m_matRY, m_matGY, m_matBY, m_matRU, m_matGU, m_matBU, m_matRV, m_matGV, m_matBV; // RGB -> YUV

    // host-readback path scratch
    std::unique_ptr<RGYCLFrame>  m_inStaging;   // host-mappable copy of the input frame
    std::unique_ptr<RGYCLFrame>  m_outStaging;  // host-mappable scratch for the output frame
    std::vector<float>           m_inBuf;       // network input tensor  (inC*inW*inH, CHW)
    std::vector<float>           m_outBuf;      // network output tensor (outC*outW*outH, CHW)
    std::vector<float>           m_u444, m_v444;// normalised chroma at output luma res (for 4:2:0 downsample)

    // --- multi-frame temporal window state (only used when m_temporalT > 1) ---
    int m_temporalT;
    struct RingFrame {
        std::vector<float> rgb;
        int64_t         timestamp;
        int64_t         duration;
        RGY_PICSTRUCT   picstruct;
        RGY_FRAME_FLAGS flags;
        int             inputFrameId;
    };
    std::deque<RingFrame> m_ring;
    int64_t m_ringBaseIdx;
    int64_t m_recvCount;
    int64_t m_emitCount;

    // --- two-input inpainting mode state (only used when mask= is set) --------
    std::unique_ptr<RGYOnnxRTDMLMultiIO> m_ovm;
    int m_maskModelW, m_maskModelH;
    int m_maskFrameW, m_maskFrameH;
    int m_imgPortIdx, m_mskPortIdx;
    float m_outScale;
    std::vector<float> m_maskFrame;
    std::vector<float> m_maskModel;
    std::vector<float> m_frameRGB;
    std::vector<float> m_modelIn;
    std::vector<float> m_modelOut;

    // opt-in end-of-chain resize (out_res=): runs after the network core, fitting
    // the integer-scaled output to the requested final resolution. Reuses the
    // shared resampler family; null when out_res= is not used.
    std::unique_ptr<RGYFilterResize>  m_postResize;
};

#endif //__RGY_FILTER_ONNX_H__
