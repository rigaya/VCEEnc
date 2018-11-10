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
#include "vce_util.h"
#include "vce_param.h"
#pragma warning(pop)

class VCEInput : public PipelineElement {
public:
    VCEInput();
    virtual ~VCEInput();

    virtual AMF_RESULT init(shared_ptr<VCELog> pLog, shared_ptr<VCEStatus> pStatus, VCEInputInfo *pInfo, amf::AMFContextPtr m_pContext) = 0;

    virtual tstring getMessage() {
        return m_message;
    }

    virtual amf::AMF_SURFACE_FORMAT GetFormat() {
        return m_inputFrameInfo.format;
    }
    virtual amf_int32 GetWidth() {
        return m_inputFrameInfo.srcWidth;
    }
    virtual amf_int32 GetHeight() {
        return m_inputFrameInfo.srcHeight;
    }
    virtual VCEInputInfo GetInputFrameInfo() {
        return m_inputFrameInfo;
    }

#pragma warning (push)
#pragma warning (disable: 4100)
    //動画ストリームの1フレーム分のデータをbitstreamに追加する (リーダー側のデータは消す)
    virtual AMF_RESULT GetNextBitstream(amf::AMFData **ppData) {
        return AMF_OK;
    }
    //動画ストリームの1フレーム分のデータをbitstreamに追加する (リーダー側のデータは残す)
    virtual AMF_RESULT GetNextBitstreamNoDelete(amf::AMFData **ppData) {
        return AMF_OK;
    }
    //ストリームのヘッダ部分を取得する
    virtual AMF_RESULT GetHeader(sBitstream *bitstream) {
        return AMF_OK;
    }
#pragma warning (pop)

    virtual amf_int32 GetInputSlotCount() override {
        return 0;
    }
    virtual amf_int32 GetOutputSlotCount() override {
        return 1;
    }
    virtual tstring GetInputInfoStr() {
        return m_strInputInfo;
    }

    void SetTrimParam(const sTrimParam& trim) {
        m_sTrimParam = trim;
    }

    sTrimParam *GetTrimParam() {
        return &m_sTrimParam;
    }
    void GetInputCropInfo(sInputCrop *cropInfo) {
        memcpy(cropInfo, &m_sInputCrop, sizeof(m_sInputCrop));
    }
    //QSVデコードを行う場合のコーデックを返す
    //行わない場合は0を返す
    uint32_t getInputCodec() {
        return m_nInputCodec;
    }

    //入力ファイルに存在する音声のトラック数を返す
    virtual int GetAudioTrackCount() {
        return 0;
    }

    //入力ファイルに存在する字幕のトラック数を返す
    virtual int GetSubtitleTrackCount() {
        return 0;
    }
    //virtual amf_double GetPosition() {
    //    return static_cast<amf_double> (m_framesCountRead) / m_framesCount;
    //}
    virtual AMF_RESULT SubmitInput(amf::AMFData *pData) override;
    virtual AMF_RESULT Terminate();
protected:
    void AddMessage(int log_level, const tstring& str) {
        if (m_pPrintMes == nullptr || log_level < m_pPrintMes->getLogLevel()) {
            return;
        }
        auto lines = split(str, _T("\n"));
        for (const auto& line : lines) {
            if (line[0] != _T('\0')) {
                m_pPrintMes->write(log_level, (m_strReaderName + _T(": ") + line + _T("\n")).c_str());
            }
        }
    }
    void AddMessage(int log_level, const TCHAR *format, ...) {
        if (m_pPrintMes == nullptr || log_level < m_pPrintMes->getLogLevel()) {
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
    //trim listを参照し、動画の最大フレームインデックスを取得する
    int getVideoTrimMaxFramIdx() {
        if (m_sTrimParam.list.size() == 0) {
            return INT_MAX;
        }
        return m_sTrimParam.list[m_sTrimParam.list.size()-1].fin;
    }
    uint32_t m_nInputCodec;
    sInputCrop m_sInputCrop;
    VCEInputInfo m_inputFrameInfo;
    shared_ptr<VCELog> m_pPrintMes;
    shared_ptr<VCEStatus> m_pEncSatusInfo;
    tstring m_message;
    tstring m_strReaderName;
    tstring m_strInputInfo;
    amf::AMFContextPtr m_pContext;
    const ConvertCSP *m_sConvert;
    sTrimParam m_sTrimParam;
};
