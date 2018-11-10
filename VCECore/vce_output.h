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

#pragma once

#include <d3d9.h>
#include <d3d11.h>
#pragma warning(push)
#pragma warning(disable:4100)
#include "VideoEncoderVCE.h"

#include "DataStream.h"
#include "PipelineElement.h"

#include "rgy_output.h"
#pragma warning(pop)

class VCEOutput : public PipelineElement {
public:
    VCEOutput() : m_output() {};
    VCEOutput(shared_ptr<RGYOutput> output, rgy_rational<int> outFps, rgy_rational<int> outTimebase)
        : m_output(output), m_outFps(outFps), m_outTimebase(outTimebase) {};
    virtual ~VCEOutput() {
        m_output.reset();
    }

    virtual AMF_RESULT Init(shared_ptr<RGYOutput> output) {
        m_output = output;
        return AMF_OK;
    }
    virtual void close() {
        m_output->Close();
    }

    virtual amf_int32 GetInputSlotCount() override {
        return 1;
    }
    virtual amf_int32 GetOutputSlotCount() override {
        return 0;
    }
    virtual AMF_RESULT SubmitInput(amf::AMFData* pData) override {
        amf::AMFBufferPtr pBuffer(pData);

        const auto VCE_TIMEBASE = rgy_rational<int>(1, HW_TIMEBASE);
        RGYBitstream data = RGYBitstreamInit();
        int64_t pts = rgy_change_scale(pData->GetPts(), m_outFps.inv(), m_outTimebase);
        int64_t duration = rgy_change_scale(pData->GetDuration(), VCE_TIMEBASE, m_outTimebase);
        data.ref((uint8_t *)pBuffer->GetNative(), pBuffer->GetSize(), pts, 0, duration);
        return err_to_amf(m_output->WriteNextFrame(&data));
    }
    virtual AMF_RESULT QueryOutput(amf::AMFData** ppData) override {
        return AMF_NOT_SUPPORTED;
    }
    virtual std::wstring GetDisplayResult() override {
        return std::wstring(L"");
    }
    virtual void WaitFin() { }

    shared_ptr<RGYOutput> output() {
        return m_output;
    }
protected:
    shared_ptr<RGYOutput> m_output;;
    rgy_rational<int> m_outFps;
    rgy_rational<int> m_outTimebase;
};
