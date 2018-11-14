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

#include "PipelineElement.h"

#include "rgy_util.h"
#include "vce_param.h"
#include "rgy_log.h"
#include "rgy_status.h"
#include "convert_csp.h"
#pragma warning(pop)
#include "rgy_input.h"
#include "rgy_err.h"

class VCEInput : public PipelineElement {
public:
    VCEInput() : m_inputFrames(0), m_input(), m_pContext(), m_bitstream(RGYBitstreamInit()) { }
    VCEInput(shared_ptr<RGYInput> input, amf::AMFContextPtr pContext) : m_input(input), m_pContext(pContext), m_bitstream(RGYBitstreamInit()) {}
    virtual ~VCEInput() {
        m_input.reset();
        m_bitstream.clear();
    }

    virtual AMF_RESULT init(shared_ptr<RGYInput> input, amf::AMFContextPtr pContext) {
        m_input = input;
        m_pContext = pContext;
        return AMF_OK;
    }

    virtual amf_int32 GetInputSlotCount() const override {
        return 0;
    }
    virtual amf_int32 GetOutputSlotCount() const override {
        return 1;
    }
    virtual AMF_RESULT SubmitInput(amf::AMFData *pData) override {
        return AMF_NOT_SUPPORTED;
    }

    virtual AMF_RESULT QueryOutput(amf::AMFData ** ppData) override;

    shared_ptr<RGYInput> input() {
        return m_input;
    }
protected:
    int m_inputFrames;
    shared_ptr<RGYInput> m_input;
    amf::AMFContextPtr m_pContext;
    RGYBitstream m_bitstream;
};
