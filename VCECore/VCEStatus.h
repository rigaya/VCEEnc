// -----------------------------------------------------------------------------------------
//     VCEEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2014-2016 rigaya
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
#include "VideoEncoderVCE.h"
#include "AMFPlatform.h"
#include "PlatformWindows.h"
#include "Thread.h"

#include "VCEUtil.h"
#include "VCEParam.h"
#include "VCELog.h"
#include "cpu_info.h"

typedef struct sEncodeStatusData {
    uint32_t nProcessedFramesNum;
    uint32_t nIDRCount;
    uint32_t nICount;
    uint32_t nPCount;
    uint32_t nBCount;
    uint64_t nIFrameSize;
    uint64_t nPFrameSize;
    uint64_t nBFrameSize;
    uint64_t nWrittenBytes;
    uint32_t tmStart;
    double fEncodeFps;
    double fBitrateKbps;
    double fCPUUsagePercent;
} sEncodeStatusData;

class VCEStatus {
public:
    VCEStatus();
    ~VCEStatus();

    virtual AMF_RESULT init(shared_ptr<VCELog> pLog, VCERational outputFps, int totalOutFrames);
    virtual void close();

    virtual void setOutputData(uint64_t nBytesWritten, uint32_t frameType);
    virtual void UpdateDisplay(const TCHAR *mes, int drop_frames);
    virtual void UpdateDisplay(uint32_t tm, int drop_frames, double progressPercent = 0.0);
    virtual void WriteLine(const TCHAR *mes);
    virtual void WriteFrameTypeResult(const TCHAR *header, uint32_t count, uint32_t maxCount, uint64_t frameSize, uint64_t maxFrameSize);
    virtual void WriteResults();
    virtual void SetStart();
    uint32_t m_inputFrames;
protected:
    sEncodeStatusData getStatus() {
        return m_sData;
    }

    bool m_bStdErrWriteToConsole;
    PROCESS_TIME m_sStartTime;
    int m_nTotalOutFrames;
    VCERational m_OutputFps;
    sEncodeStatusData m_sData;
    shared_ptr<VCELog> m_pVCELog;
};
