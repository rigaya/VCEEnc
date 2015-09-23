//  -----------------------------------------------------------------------------------------
//    VCEEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

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
