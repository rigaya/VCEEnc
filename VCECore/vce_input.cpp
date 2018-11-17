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

#include "rgy_osdep.h"
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
#include "rgy_input_avcodec.h"
#include "rgy_err.h"
#include "vce_input.h"

AMF_RESULT VCEInput::QueryOutput(amf::AMFData ** ppData) {
    const auto frameInfo = m_input->GetInputFrameInfo();
    RGYFrame frame;
    if (frameInfo.codec == RGY_CODEC_UNKNOWN) {
        //動画のデコード/フレームの取得を行う
        amf::AMFSurfacePtr pSurface;
        auto res = m_pContext->AllocSurface(amf::AMF_MEMORY_HOST, csp_rgy_to_enc(frameInfo.csp),
            frameInfo.srcWidth - frameInfo.crop.e.left - frameInfo.crop.e.right,
            frameInfo.srcHeight - frameInfo.crop.e.bottom - frameInfo.crop.e.up,
            &pSurface);
        if (res != AMF_OK) return res;
        frame = RGYFrameFromSurface(pSurface);
        auto ret = m_input->LoadNextFrame(&frame);
        if (ret == RGY_ERR_NONE) {
            const auto VCE_TIMEBASE = rgy_rational<int>(1, AMF_SECOND);
#if ENABLE_AVSW_READER
            auto pAVCodecReader = std::dynamic_pointer_cast<RGYInputAvcodec>(m_input);
            if (pAVCodecReader != nullptr) {
                const auto vid_timebase = to_rgy(pAVCodecReader->GetInputVideoStream()->time_base);
                pSurface->SetDuration(rgy_change_scale(frame.duration(), vid_timebase, VCE_TIMEBASE));
                pSurface->SetPts(rgy_change_scale(frame.timestamp(), vid_timebase, VCE_TIMEBASE));
            } else {
#endif //#if ENABLE_AVSW_READER
                const auto fps_timebase = rgy_rational<int>(frameInfo.fpsN, frameInfo.fpsD).inv();
                pSurface->SetDuration(rgy_change_scale(1, fps_timebase, VCE_TIMEBASE));
                pSurface->SetPts(rgy_change_scale(m_inputFrames, fps_timebase, VCE_TIMEBASE));
#if ENABLE_AVSW_READER
            }
#endif //#if ENABLE_AVSW_READER
            *ppData = pSurface.Detach();
            m_inputFrames++;
        }
        return err_to_amf(ret);
    } else {
#if ENABLE_AVSW_READER
        auto err = m_input->LoadNextFrame(&frame); //進捗表示のため

        err = m_input->GetNextBitstream(&m_bitstream);
        if (err == RGY_ERR_NONE) {
            amf::AMFBufferPtr pictureBuffer;
            AMF_RESULT ar = m_pContext->AllocBuffer(amf::AMF_MEMORY_HOST, m_bitstream.size(), &pictureBuffer);
            if (ar != AMF_OK) {
                return ar;
            }
            auto pAVCodecReader = std::dynamic_pointer_cast<RGYInputAvcodec>(m_input);
            if (pAVCodecReader == nullptr) {
                return AMF_UNEXPECTED;
            }
            memcpy(pictureBuffer->GetNative(), m_bitstream.data(), m_bitstream.size());

            const auto VCE_TIMEBASE = rgy_rational<int>(1, AMF_SECOND); // In 100 NanoSeconds
            const auto vid_timebase = to_rgy(pAVCodecReader->GetInputVideoStream()->time_base);
            const auto duration = rgy_change_scale(m_bitstream.duration(), vid_timebase, VCE_TIMEBASE);
            const auto pts = rgy_change_scale(m_bitstream.pts(), vid_timebase, VCE_TIMEBASE);
            pictureBuffer->SetDuration(duration);
            pictureBuffer->SetPts(pts);
            *ppData = pictureBuffer.Detach();
            m_bitstream.clear();
        }
        return err_to_amf(err);
#else
        return err_to_amf(RGY_ERR_UNSUPPORTED);
#endif //#if ENABLE_AVSW_READER
    }
}
