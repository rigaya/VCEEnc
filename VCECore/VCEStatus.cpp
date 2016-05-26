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

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <math.h>
#include <tchar.h>
#include <process.h>

#include "VCEParam.h"
#include "VCEVersion.h"
#include "VCELog.h"
#include "VCEStatus.h"
#include "cpu_info.h"
#include "gpuz_info.h"

VCEStatus::VCEStatus() :
    m_nInputFrames(0),
    m_tmStart(std::chrono::system_clock::now()),
    m_tmLastUpdate(std::chrono::system_clock::now()),
    m_sStartTime({ 0 }),
    m_nTotalOutFrames(0),
    m_nOutputFPSRate(0),
    m_nOutputFPSScale(0),
    m_sData({ 0 }),
    m_pVCELog(nullptr),
    m_bStdErrWriteToConsole(true),
    m_bEncStarted(false) {

}

VCEStatus::~VCEStatus() {
    m_pVCELog.reset();
    close();
}

AMF_RESULT VCEStatus::init(shared_ptr<VCELog> pLog, VCERational outputFps, int totalOutFrames) {
    m_pVCELog = pLog;
    m_nOutputFPSRate = outputFps.num;
    m_nOutputFPSScale = outputFps.den;
    m_nTotalOutFrames = totalOutFrames;
    DWORD mode = 0;
    m_bStdErrWriteToConsole = 0 != GetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), &mode); //stderrの出力先がコンソールかどうか
    return AMF_OK;
}

void VCEStatus::SetStart() {
    m_tmStart = std::chrono::system_clock::now();
    GetProcessTime(GetCurrentProcess(), &m_sStartTime);
}

void VCEStatus::close() {
    m_pVCELog.reset();
    memset(&m_sData, 0, sizeof(m_sData));
}

#pragma warning(push)
#pragma warning(disable: 4100)
void VCEStatus::setOutputData(uint64_t nBytesWritten, uint32_t frameType) {
    m_sData.nProcessedFramesNum++;
    m_sData.nWrittenBytes += nBytesWritten;
    //m_sData.nIDRCount     += ((frameType & MFX_FRAMETYPE_IDR) >> 7);
    //m_sData.nICount       +=  (frameType & MFX_FRAMETYPE_I);
    //m_sData.nPCount       += ((frameType & MFX_FRAMETYPE_P) >> 1);
    //m_sData.nBCount       += ((frameType & MFX_FRAMETYPE_B) >> 2);
    //m_sData.nIFrameSize   += nBytesWritten *  (frameType & MFX_FRAMETYPE_I);
    //m_sData.nPFrameSize   += nBytesWritten * ((frameType & MFX_FRAMETYPE_P) >> 1);
    //m_sData.nBFrameSize   += nBytesWritten * ((frameType & MFX_FRAMETYPE_B) >> 2);
}

void VCEStatus::UpdateDisplay(const TCHAR *mes, int drop_frames, double progressPercent) {
    if (m_pVCELog != nullptr && m_pVCELog->getLogLevel() > VCE_LOG_INFO) {
        return;
    }
#if UNICODE
    char *mes_char = NULL;
    if (!m_bStdErrWriteToConsole) {
        //コンソールへの出力でなければ、ANSIに変換する
        const int buf_length = (int)(wcslen(mes) + 1) * 2;
        if (NULL != (mes_char = (char *)calloc(buf_length, 1))) {
            WideCharToMultiByte(CP_THREAD_ACP, 0, mes, -1, mes_char, buf_length, NULL, NULL);
            fprintf(stderr, "%s\r", mes_char);
            free(mes_char);
        }
    } else
#endif
        _ftprintf(stderr, _T("%s\r"), mes);

    fflush(stderr); //リダイレクトした場合でもすぐ読み取れるようflush
}
#pragma warning(pop)

AMF_RESULT VCEStatus::UpdateDisplay(int drop_frames, double progressPercent) {
    if (m_pVCELog != nullptr && m_pVCELog->getLogLevel() > VCE_LOG_INFO) {
        return AMF_OK;
    }
    if (m_sData.nProcessedFramesNum + drop_frames <= 0) {
        return AMF_OK;
    }
    auto tm = std::chrono::system_clock::now();
    if (duration_cast<std::chrono::milliseconds>(tm - m_tmLastUpdate).count() < UPDATE_INTERVAL) {
        return AMF_OK;
    }
    m_tmLastUpdate = tm;
    bool bGPUUsage = false;
    int gpuusage = 0;
    GPUZ_SH_MEM gpu_info = { 0 };
    if (0 == get_gpuz_info(&gpu_info)) {
        bGPUUsage = true;
        m_sData.nGPUInfoCountSuccess++;
        m_sData.fGPULoadPercentTotal += gpu_load(&gpu_info);
        m_sData.fGPUClockTotal += gpu_core_clock(&gpu_info);
        gpuusage = (int)gpu_load(&gpu_info);
    } else {
        m_sData.nGPUInfoCountFail++;
    }
    TCHAR mes[256] = { 0 };
    double elapsedTime = (double)duration_cast<std::chrono::milliseconds>(tm - m_tmStart).count();
    m_sData.fEncodeFps = (m_sData.nProcessedFramesNum + drop_frames) * 1000.0 / elapsedTime;
    m_sData.fBitrateKbps = (double)m_sData.nWrittenBytes * (m_nOutputFPSRate / (double)m_nOutputFPSScale) / ((1000 / 8) * (m_sData.nProcessedFramesNum + drop_frames));
    if (m_nTotalOutFrames || progressPercent > 0.0) {
        if (progressPercent == 0.0) {
            progressPercent = (m_sData.nProcessedFramesNum + drop_frames) * 100 / (double)m_nTotalOutFrames;
        }
        progressPercent = (std::min)(progressPercent, 100.0);
        uint32_t remaining_time = (uint32_t)(elapsedTime * (100.0 - progressPercent) / progressPercent + 0.5);
        int hh = remaining_time / (60*60*1000);
        remaining_time -= hh * (60*60*1000);
        int mm = remaining_time / (60*1000);
        remaining_time -= mm * (60*1000);
        int ss = remaining_time / 1000;

        int len = _stprintf_s(mes, _countof(mes), _T("[%.1lf%%] %d frames: %.2lf fps, %0d kb/s, remain %d:%02d:%02d"),
            progressPercent,
            m_sData.nProcessedFramesNum + drop_frames,
            m_sData.fEncodeFps,
            (int)(m_sData.fBitrateKbps + 0.5),
            hh, mm, ss );
        if (drop_frames) {
            len += _stprintf_s(mes + len, _countof(mes) - len, _T(", afs drop %d/%d  "), drop_frames, (m_sData.nProcessedFramesNum + drop_frames));
        } else if (bGPUUsage) {
            len += _stprintf_s(mes + len, _countof(mes) - len, _T(", EU %d%%"), gpuusage);
        }
        for (; len < 79; len++) {
            mes[len] = _T(' ');
        }
        mes[len] = _T('\0');
    } else {
        int len = _stprintf_s(mes, _countof(mes), _T("%d frames: %0.2lf fps, %d kbps"), 
            (m_sData.nProcessedFramesNum + drop_frames),
            m_sData.fEncodeFps,
            (int)(m_sData.fBitrateKbps + 0.5)
        );
        if (bGPUUsage) {
            len += _stprintf_s(mes + len, _countof(mes) - len, _T(", EU %d%%"), gpuusage);
        }
        for (; len < 79; len++) {
            mes[len] = _T(' ');
        }
        mes[len] = _T('\0');
    }
    UpdateDisplay(mes, drop_frames, progressPercent);
    return AMF_OK;
}

void VCEStatus::WriteLine(const TCHAR *mes) {
    if (m_pVCELog != nullptr && m_pVCELog->getLogLevel() > VCE_LOG_INFO) {
        return;
    }
    m_pVCELog->write(VCE_LOG_INFO, _T("%s\n"), mes);
}
void VCEStatus::WriteLineDirect(TCHAR *mes) {
    if (m_pVCELog != nullptr && m_pVCELog->getLogLevel() > VCE_LOG_INFO) {
        return;
    }
    m_pVCELog->write_log(VCE_LOG_INFO, mes);
}

void VCEStatus::WriteFrameTypeResult(const TCHAR *header, uint32_t count, uint32_t maxCount, uint64_t frameSize, uint64_t maxFrameSize) {
    if (count) {
        TCHAR mes[512] = { 0 };
        int mes_len = 0;
        const int header_len = (int)_tcslen(header);
        memcpy(mes, header, header_len * sizeof(mes[0]));
        mes_len += header_len;

        for (int i = std::max(0, (int)log10((double)count)); i < (int)log10((double)maxCount) && mes_len < _countof(mes); i++, mes_len++)
            mes[mes_len] = _T(' ');
        mes_len += _stprintf_s(mes + mes_len, _countof(mes) - mes_len, _T("%u"), count);

        if (frameSize > 0) {
            const TCHAR *TOTAL_SIZE = _T(",  total size  ");
            memcpy(mes + mes_len, TOTAL_SIZE, _tcslen(TOTAL_SIZE) * sizeof(mes[0]));
            mes_len += (int)_tcslen(TOTAL_SIZE);

            for (int i = std::max(0, (int)log10((double)frameSize / (double)(1024 * 1024))); i < (int)log10((double)maxFrameSize / (double)(1024 * 1024)) && mes_len < _countof(mes); i++, mes_len++)
                mes[mes_len] = _T(' ');

            mes_len += _stprintf_s(mes + mes_len, _countof(mes) - mes_len, _T("%.2f MB"), (double)frameSize / (double)(1024 * 1024));
        }

        WriteLine(mes);
    }
}

void VCEStatus::WriteResults() {
    auto tm_result = std::chrono::system_clock::now();
    const auto time_elapsed64 = std::chrono::duration_cast<std::chrono::milliseconds>(tm_result - m_tmStart).count();
    m_sData.fEncodeFps = m_sData.nProcessedFramesNum * 1000.0 / (double)time_elapsed64;
    m_sData.fBitrateKbps = (double)(m_sData.nWrittenBytes * 8) *  (m_nOutputFPSRate / (double)m_nOutputFPSScale) / (1000.0 * m_sData.nProcessedFramesNum);

    TCHAR mes[512] = { 0 };
    for (int i = 0; i < 79; i++)
        mes[i] = _T(' ');
    WriteLine(mes);

    _stprintf_s(mes, _T("encoded %d frames, %.2f fps, %.2f kbps, %.2f MB"),
        m_sData.nProcessedFramesNum,
        m_sData.fEncodeFps,
        m_sData.fBitrateKbps,
        (double)m_sData.nWrittenBytes / (double)(1024 * 1024)
    );
    WriteLine(mes);

    int hh = (int)(time_elapsed64 / (60*60*1000));
    int time_elapsed = (int)(time_elapsed64 - hh * (60*60*1000));
    int mm = time_elapsed / (60*1000);
    time_elapsed -= mm * (60*1000);
    int ss = (time_elapsed + 500) / 1000;
    m_sData.fCPUUsagePercent = GetProcessAvgCPUUsage(&m_sStartTime);
    if (m_sData.nGPUInfoCountSuccess > m_sData.nGPUInfoCountFail) {
        double gpu_load = m_sData.fGPULoadPercentTotal / m_sData.nGPUInfoCountSuccess;
        int gpu_clock_avg = (int)(m_sData.fGPUClockTotal / m_sData.nGPUInfoCountSuccess + 0.5);
        _stprintf_s(mes, _T("encode time %d:%02d:%02d, CPU: %.2f%%, EU: %.2f%%, GPUClockAvg: %dMHz\n"), hh, mm, ss, m_sData.fCPUUsagePercent, gpu_load, gpu_clock_avg);
    } else {
        _stprintf_s(mes, _T("encode time %d:%02d:%02d, CPULoad: %.2f%%\n"), hh, mm, ss, m_sData.fCPUUsagePercent);
    }
    WriteLineDirect(mes);

    uint32_t maxCount = MAX3(m_sData.nICount, m_sData.nPCount, m_sData.nBCount);
    uint64_t maxFrameSize = MAX3(m_sData.nIFrameSize, m_sData.nPFrameSize, m_sData.nBFrameSize);

    WriteFrameTypeResult(_T("frame type IDR "), m_sData.nIDRCount, maxCount, 0, maxFrameSize);
    WriteFrameTypeResult(_T("frame type I   "), m_sData.nICount, maxCount, m_sData.nIFrameSize, maxFrameSize);
    WriteFrameTypeResult(_T("frame type P   "), m_sData.nPCount, maxCount, m_sData.nPFrameSize, maxFrameSize);
    WriteFrameTypeResult(_T("frame type B   "), m_sData.nBCount, maxCount, m_sData.nBFrameSize, maxFrameSize);
}
