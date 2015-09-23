//  -----------------------------------------------------------------------------------------
//    VCEEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

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

VCEStatus::VCEStatus() :
    m_inputFrames(0), m_bStdErrWriteToConsole(true),
    m_sStartTime(), m_nTotalOutFrames(0), m_OutputFps({ 0, 0 }),
    m_sData(), m_pVCELog() {

}

VCEStatus::~VCEStatus() {
    close();
}

AMF_RESULT VCEStatus::init(shared_ptr<VCELog> pLog, VCERational outputFps, int totalOutFrames) {
    m_pVCELog = pLog;
    m_OutputFps = outputFps;
    m_nTotalOutFrames = totalOutFrames;
    DWORD mode = 0;
    m_bStdErrWriteToConsole = 0 != GetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), &mode); //stderrの出力先がコンソールかどうか
    return AMF_OK;
}

void VCEStatus::SetStart() {
    m_sData.tmStart = timeGetTime();
    GetProcessTime(GetCurrentProcess(), &m_sStartTime);
}

void VCEStatus::close() {
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

void VCEStatus::UpdateDisplay(const TCHAR *mes, int drop_frames) {
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

void VCEStatus::UpdateDisplay(uint32_t tm, int drop_frames, double progressPercent) {
    if (m_pVCELog != nullptr && m_pVCELog->getLogLevel() > VCE_LOG_INFO) {
        return;
    }
    if (m_sData.nProcessedFramesNum + drop_frames) {
        TCHAR mes[256];
        m_sData.fEncodeFps = (m_sData.nProcessedFramesNum + drop_frames) * 1000.0 / (double)(tm - m_sData.tmStart);
        m_sData.fBitrateKbps = (double)m_sData.nWrittenBytes * (m_OutputFps.num / (double)m_OutputFps.den) / ((1000 / 8) * (m_sData.nProcessedFramesNum + drop_frames));
        if (m_nTotalOutFrames || progressPercent > 0.0) {
            if (progressPercent == 0.0) {
                progressPercent = (m_sData.nProcessedFramesNum + drop_frames) * 100 / (double)m_nTotalOutFrames;
            }
            progressPercent = min(progressPercent, 100.0);
            uint32_t remaining_time = (uint32_t)((double)(tm - m_sData.tmStart) * (100.0 - progressPercent) / progressPercent + 0.5);
            int hh = remaining_time / (60*60*1000);
            remaining_time -= hh * (60*60*1000);
            int mm = remaining_time / (60*1000);
            remaining_time -= mm * (60*1000);
            int ss = (remaining_time + 500) / 1000;

            int len = _stprintf_s(mes, _countof(mes), _T("[%.1lf%%] %d frames: %.2lf fps, %0.2lf kb/s, remain %d:%02d:%02d  "),
                progressPercent,
                m_sData.nProcessedFramesNum + drop_frames,
                m_sData.fEncodeFps,
                m_sData.fBitrateKbps,
                hh, mm, ss);
            if (drop_frames)
                _stprintf_s(mes + len - 2, _countof(mes) - len + 2, _T(", afs drop %d/%d  "), drop_frames, (m_sData.nProcessedFramesNum + drop_frames));
        } else {
            _stprintf_s(mes, _countof(mes), _T("%d frames: %0.2lf fps, %0.2lf kbps  "),
                (m_sData.nProcessedFramesNum + drop_frames),
                m_sData.fEncodeFps,
                m_sData.fBitrateKbps
                );
        }
        UpdateDisplay(mes, drop_frames);
    }
}

void VCEStatus::WriteLine(const TCHAR *mes) {
    if (m_pVCELog != nullptr && m_pVCELog->getLogLevel() > VCE_LOG_INFO) {
        return;
    }
    (*m_pVCELog)(VCE_LOG_INFO, _T("%s\n"), mes);
}

void VCEStatus::WriteFrameTypeResult(const TCHAR *header, uint32_t count, uint32_t maxCount, uint64_t frameSize, uint64_t maxFrameSize) {
    if (count) {
        TCHAR mes[512] = { 0 };
        int mes_len = 0;
        const int header_len = (int)_tcslen(header);
        memcpy(mes, header, header_len * sizeof(mes[0]));
        mes_len += header_len;

        for (int i = max(0, (int)log10((double)count)); i < (int)log10((double)maxCount) && mes_len < _countof(mes); i++, mes_len++)
            mes[mes_len] = _T(' ');
        mes_len += _stprintf_s(mes + mes_len, _countof(mes) - mes_len, _T("%u"), count);

        if (frameSize > 0) {
            const TCHAR *TOTAL_SIZE = _T(",  total size  ");
            memcpy(mes + mes_len, TOTAL_SIZE, _tcslen(TOTAL_SIZE) * sizeof(mes[0]));
            mes_len += (int)_tcslen(TOTAL_SIZE);

            for (int i = max(0, (int)log10((double)frameSize / (double)(1024 * 1024))); i < (int)log10((double)maxFrameSize / (double)(1024 * 1024)) && mes_len < _countof(mes); i++, mes_len++)
                mes[mes_len] = _T(' ');

            mes_len += _stprintf_s(mes + mes_len, _countof(mes) - mes_len, _T("%.2f MB"), (double)frameSize / (double)(1024 * 1024));
        }

        WriteLine(mes);
    }
}

void VCEStatus::WriteResults() {
    uint32_t tm_result = timeGetTime();
    uint32_t time_elapsed = tm_result - m_sData.tmStart;
    m_sData.fEncodeFps = m_sData.nProcessedFramesNum * 1000.0 / (double)time_elapsed;
    m_sData.fBitrateKbps = (double)(m_sData.nWrittenBytes * 8) *  (m_OutputFps.num / (double)m_OutputFps.den) / (1000.0 * m_sData.nProcessedFramesNum);

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

    int hh = time_elapsed / (60*60*1000);
    time_elapsed -= hh * (60*60*1000);
    int mm = time_elapsed / (60*1000);
    time_elapsed -= mm * (60*1000);
    int ss = (time_elapsed + 500) / 1000;
    m_sData.fCPUUsagePercent = GetProcessAvgCPUUsage(GetCurrentProcess(), &m_sStartTime);
    _stprintf_s(mes, _T("encode time %d:%02d:%02d / CPU Usage: %.2f%%\n"), hh, mm, ss, m_sData.fCPUUsagePercent);
    WriteLine(mes);

    uint32_t maxCount = MAX3(m_sData.nICount, m_sData.nPCount, m_sData.nBCount);
    uint64_t maxFrameSize = MAX3(m_sData.nIFrameSize, m_sData.nPFrameSize, m_sData.nBFrameSize);

    WriteFrameTypeResult(_T("frame type IDR "), m_sData.nIDRCount, maxCount, 0, maxFrameSize);
    WriteFrameTypeResult(_T("frame type I   "), m_sData.nICount, maxCount, m_sData.nIFrameSize, maxFrameSize);
    WriteFrameTypeResult(_T("frame type P   "), m_sData.nPCount, maxCount, m_sData.nPFrameSize, maxFrameSize);
    WriteFrameTypeResult(_T("frame type B   "), m_sData.nBCount, maxCount, m_sData.nBFrameSize, maxFrameSize);
}
