// -----------------------------------------------------------------------------------------
// VCEEnc by rigaya
// -----------------------------------------------------------------------------------------
//
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
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// ------------------------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <set>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <shellapi.h>
#include "rgy_version.h"
#include "rgy_perf_monitor.h"
#include "rgy_caption.h"
#include "vce_param.h"
#include "vce_core.h"
#include "vce_cmd.h"
#include "rgy_avutil.h"

tstring GetVCEEncVersion() {
    static const TCHAR *const ENABLED_INFO[] = { _T("disabled"), _T("enabled") };
    tstring version = strsprintf(_T("VCEEncC (%s) %s by rigaya, build %s %s\n"), BUILD_ARCH_STR, VER_STR_FILEVERSION_TCHAR, _T(__DATE__), _T(__TIME__));
    version += strsprintf(_T("AMF Version %d.%d.%d\n"), AMF_VERSION_MAJOR, AMF_VERSION_MINOR, AMF_VERSION_RELEASE);
    version += _T("reader: raw");
    if (ENABLE_AVISYNTH_READER)    version += _T(", avs");
    if (ENABLE_VAPOURSYNTH_READER) version += _T(", vpy");
#if ENABLE_AVSW_READER
    version += strsprintf(_T(", avhw [%s]"), getHWDecSupportedCodecList().c_str());
#endif //#if ENABLE_AVSW_READER
    return version;
}

const TCHAR *cmd_short_opt_to_long(TCHAR short_opt) {
    const TCHAR *option_name = nullptr;
    switch (short_opt) {
    case _T('b'):
        option_name = _T("bframes");
        break;
    case _T('c'):
        option_name = _T("codec");
        break;
    case _T('d'):
        option_name = _T("device");
        break;
    case _T('f'):
        option_name = _T("format");
        break;
    case _T('u'):
        option_name = _T("quality");
        break;
    case _T('i'):
        option_name = _T("input-file");
        break;
    case _T('o'):
        option_name = _T("output-file");
        break;
    case _T('m'):
        option_name = _T("mux-option");
        break;
    case _T('v'):
        option_name = _T("version");
        break;
    case _T('h'):
    case _T('?'):
        option_name = _T("help");
        break;
    default:
        break;
    }
    return option_name;
}

#define IS_OPTION(x) (0 == _tcscmp(option_name, _T(x)))

static int getAudioTrackIdx(const VCEParam *pParams, int iTrack) {
    for (int i = 0; i < pParams->nAudioSelectCount; i++) {
        if (iTrack == pParams->ppAudioSelectList[i]->nAudioSelect) {
            return i;
        }
    }
    return -1;
}

static int getFreeAudioTrack(const VCEParam *pParams) {
    for (int iTrack = 1;; iTrack++) {
        if (0 > getAudioTrackIdx(pParams, iTrack)) {
            return iTrack;
        }
    }
#ifndef _MSC_VER
    return -1;
#endif //_MSC_VER
}

bool get_list_value(const CX_DESC * list, const TCHAR *chr, int *value) {
    for (int i = 0; list[i].desc; i++) {
        if (0 == _tcsicmp(list[i].desc, chr)) {
            *value = list[i].value;
            return true;
        }
    }
    return false;
};

struct sArgsData {
    tstring cachedlevel, cachedprofile, cachedtier;
    uint32_t nParsedAudioFile = 0;
    uint32_t nParsedAudioEncode = 0;
    uint32_t nParsedAudioCopy = 0;
    uint32_t nParsedAudioBitrate = 0;
    uint32_t nParsedAudioSamplerate = 0;
    uint32_t nParsedAudioSplit = 0;
    uint32_t nParsedAudioFilter = 0;
    uint32_t nTmpInputBuf = 0;
    int nBframes = -1;
};

int parse_one_option(const TCHAR *option_name, const TCHAR* strInput[], int& i, int nArgNum, VCEParam *pParams, sArgsData *argData, ParseCmdError& err) {
#define SET_ERR(app_name, errmes, opt_name, err_val) \
    err.strAppName = (app_name) ? app_name : _T(""); \
    err.strErrorMessage = (errmes) ? errmes : _T(""); \
    err.strOptionName = (opt_name) ? opt_name : _T(""); \
    err.strErrorValue = (err_val) ? err_val : _T("");

    if (IS_OPTION("input") || IS_OPTION("input-file")) {
        i++;
        pParams->inputFilename = strInput[i];
        return 0;
    }
    if (IS_OPTION("output") || IS_OPTION("output-file")) {
        i++;
        pParams->outputFilename = strInput[i];
        return 0;
    }
    if (IS_OPTION("device")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->deviceID = value;
        return 0;
    }
    if (IS_OPTION("raw")) {
        pParams->input.type = RGY_INPUT_FMT_RAW;
        return 0;
    }
    if (IS_OPTION("y4m")) {
        pParams->input.type = RGY_INPUT_FMT_Y4M;
#if ENABLE_AVI_READER
        return 0;
    }
    if (IS_OPTION("avi")) {
        pParams->input.type = RGY_INPUT_FMT_AVI;
#endif
#if ENABLE_AVISYNTH_READER
        return 0;
    }
    if (IS_OPTION("avs")) {
        pParams->input.type = RGY_INPUT_FMT_AVS;
#endif
#if ENABLE_VAPOURSYNTH_READER
        return 0;
    }
    if (IS_OPTION("vpy")) {
        pParams->input.type = RGY_INPUT_FMT_VPY;
        return 0;
    }
    if (IS_OPTION("vpy-mt")) {
        pParams->input.type = RGY_INPUT_FMT_VPY_MT;
#endif
#if ENABLE_AVSW_READER
        return 0;
    }
    if (IS_OPTION("avvce")
        || IS_OPTION("avhw")) {
        pParams->input.type = RGY_INPUT_FMT_AVHW;
        return 0;
    }
    if (IS_OPTION("avsw")) {
        pParams->input.type = RGY_INPUT_FMT_AVSW;
#endif
        return 0;
    }
    if (IS_OPTION("fps")) {
        i++;
        int a[2] = { 0 };
        if (   2 == _stscanf_s(strInput[i], _T("%d/%d"), &a[0], &a[1])
            || 2 == _stscanf_s(strInput[i], _T("%d:%d"), &a[0], &a[1])
            || 2 == _stscanf_s(strInput[i], _T("%d,%d"), &a[0], &a[1])) {
            pParams->input.fpsN = a[0];
            pParams->input.fpsD = a[1];
        } else {
            double d;
            if (1 == _stscanf_s(strInput[i], _T("%lf"), &d)) {
                int rate = (int)(d * 1001.0 + 0.5);
                if (rate % 1000 == 0) {
                    pParams->input.fpsN = rate;
                    pParams->input.fpsD = 1001;
                } else {
                    pParams->input.fpsD = 100000;
                    pParams->input.fpsN = (int)(d * pParams->input.fpsD + 0.5);
                    rgy_reduce(pParams->input.fpsN, pParams->input.fpsD);
                }
            } else  {
                SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
                return -1;
            }
        }
        return 0;
    }
    if (IS_OPTION("crop")) {
        i++;
        sInputCrop a = { 0 };
        if (   4 == _stscanf_s(strInput[i], _T("%d,%d,%d,%d"), &a.c[0], &a.c[1], &a.c[2], &a.c[3])
            || 4 == _stscanf_s(strInput[i], _T("%d:%d:%d:%d"), &a.c[0], &a.c[1], &a.c[2], &a.c[3])) {
            memcpy(&pParams->input.crop, &a, sizeof(a));
        } else {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("input-res")) {
        i++;
        int a[2] = { 0 };
        if (   2 == _stscanf_s(strInput[i], _T("%dx%d"), &a[0], &a[1])
            || 2 == _stscanf_s(strInput[i], _T("%d:%d"), &a[0], &a[1])
            || 2 == _stscanf_s(strInput[i], _T("%d,%d"), &a[0], &a[1])) {
            pParams->input.srcWidth  = a[0];
            pParams->input.srcHeight = a[1];
        } else {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("output-res")) {
        i++;
        int a[2] = { 0 };
        if (   2 == _stscanf_s(strInput[i], _T("%dx%d"), &a[0], &a[1])
            || 2 == _stscanf_s(strInput[i], _T("%d:%d"), &a[0], &a[1])
            || 2 == _stscanf_s(strInput[i], _T("%d,%d"), &a[0], &a[1])) {
            pParams->input.dstWidth  = a[0];
            pParams->input.dstHeight = a[1];
        } else {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("input-analyze")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return 1;
        } else if (value < 0) {
            SET_ERR(strInput[0], _T("input-analyze requires non-negative value."), option_name, strInput[i]);
            return 1;
        } else {
            pParams->nAVDemuxAnalyzeSec = (int)((std::min)(value, USHRT_MAX));
        }
        return 0;
    }
    if (IS_OPTION("video-track")) {
        i++;
        int v = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &v)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return 1;
        }
        if (v == 0) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return 1;
        }
        pParams->nVideoTrack = v;
        return 0;
    }
    if (IS_OPTION("video-streamid")) {
        i++;
        int v = 0;
        if (1 != _stscanf_s(strInput[i], _T("%i"), &v)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return 1;
        }
        pParams->nVideoStreamId = v;
        return 0;
    }
#if 0
    if (IS_OPTION("trim")) {
        i++;
        auto trim_str_list = split(strInput[i], _T(","));
        std::vector<sTrim> trim_list;
        for (auto trim_str : trim_str_list) {
            sTrim trim;
            if (2 != _stscanf_s(trim_str.c_str(), _T("%d:%d"), &trim.start, &trim.fin) || (trim.fin > 0 && trim.fin < trim.start)) {
                SET_ERR(strInput[0], _T("Invalid Value"), option_name);
                return -1;
            }
            if (trim.fin == 0) {
                trim.fin = TRIM_MAX;
            } else if (trim.fin < 0) {
                trim.fin = trim.start - trim.fin - 1;
            }
            trim_list.push_back(trim);
        }
        if (trim_list.size()) {
            std::sort(trim_list.begin(), trim_list.end(), [](const sTrim& trimA, const sTrim& trimB) { return trimA.start < trimB.start; });
            for (int j = (int)trim_list.size() - 2; j >= 0; j--) {
                if (trim_list[j].fin > trim_list[j+1].start) {
                    trim_list[j].fin = trim_list[j+1].fin;
                    trim_list.erase(trim_list.begin() + j+1);
                }
            }
            pParams->nTrimCount = (int)trim_list.size();
            pParams->pTrimList = (sTrim *)malloc(sizeof(pParams->pTrimList[0]) * trim_list.size());
            memcpy(pParams->pTrimList, &trim_list[0], sizeof(pParams->pTrimList[0]) * trim_list.size());
        }
        return 0;
    }
#endif
    if (0 == _tcscmp(option_name, _T("seek"))) {
        i++;
        int ret = 0;
        int hh = 0, mm = 0;
        float sec = 0.0f;
        if (   3 != (ret = _stscanf_s(strInput[i], _T("%d:%d:%f"),    &hh, &mm, &sec))
            && 2 != (ret = _stscanf_s(strInput[i],    _T("%d:%f"),         &mm, &sec))
            && 1 != (ret = _stscanf_s(strInput[i],       _T("%f"),              &sec))) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return 1;
        }
        if (ret <= 2) {
            hh = 0;
        }
        if (ret <= 1) {
            mm = 0;
        }
        if (hh < 0 || mm < 0 || sec < 0) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return 1;
        }
        if (hh > 0 && mm >= 60) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return 1;
        }
        mm += hh * 60;
        if (mm > 0 && sec >= 60.0f) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return 1;
        }
        pParams->fSeekSec = sec + mm * 60;
        return 0;
    }
    if (IS_OPTION("video-track")) {
        i++;
        int v = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &v)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        if (v == 0) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nVideoTrack = (int8_t)v;
        return 0;
    }
    if (IS_OPTION("video-streamid")) {
        i++;
        int v = 0;
        if (1 != _stscanf_s(strInput[i], _T("%i"), &v)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nVideoStreamId = v;
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("audio-source"))) {
        i++;
        pParams->nAVMux |= (RGY_MUX_VIDEO | RGY_MUX_AUDIO);
        size_t audioSourceLen = _tcslen(strInput[i]) + 1;
        TCHAR *pAudioSource = (TCHAR *)malloc(sizeof(strInput[i][0]) * audioSourceLen);
        memcpy(pAudioSource, strInput[i], sizeof(strInput[i][0]) * audioSourceLen);
        pParams->ppAudioSourceList = (TCHAR **)realloc(pParams->ppAudioSourceList, sizeof(pParams->ppAudioSourceList[0]) * (pParams->nAudioSourceCount + 1));
        pParams->ppAudioSourceList[pParams->nAudioSourceCount] = pAudioSource;
        pParams->nAudioSourceCount++;
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("audio-file"))) {
        i++;
        const TCHAR *ptr = strInput[i];
        sAudioSelect *pAudioSelect = nullptr;
        int audioIdx = -1;
        int trackId = 0;
        if (_tcschr(ptr, '?') == nullptr || 1 != _stscanf(ptr, _T("%d?"), &trackId)) {
            //トラック番号を適当に発番する (カウントは1から)
            trackId = argData->nParsedAudioFile+1;
            audioIdx = getAudioTrackIdx(pParams, trackId);
            if (audioIdx < 0 || pParams->ppAudioSelectList[audioIdx]->pAudioExtractFilename != nullptr) {
                trackId = getFreeAudioTrack(pParams);
                pAudioSelect = (sAudioSelect *)calloc(1, sizeof(pAudioSelect[0]));
                pAudioSelect->nAudioSelect = trackId;
            } else {
                pAudioSelect = pParams->ppAudioSelectList[audioIdx];
            }
        } else if (i <= 0) {
            //トラック番号は1から連番で指定
            SET_ERR(strInput[0], _T("Invalid track number"), option_name, strInput[i]);
            return 1;
        } else {
            audioIdx = getAudioTrackIdx(pParams, trackId);
            if (audioIdx < 0) {
                pAudioSelect = (sAudioSelect *)calloc(1, sizeof(pAudioSelect[0]));
                pAudioSelect->nAudioSelect = trackId;
            } else {
                pAudioSelect = pParams->ppAudioSelectList[audioIdx];
            }
            ptr = _tcschr(ptr, '?') + 1;
        }
        assert(pAudioSelect != nullptr);
        const TCHAR *qtr = _tcschr(ptr, ':');
        if (qtr != NULL && !(ptr + 1 == qtr && qtr[1] == _T('\\'))) {
            pAudioSelect->pAudioExtractFormat = _tcsdup(ptr);
            ptr = qtr + 1;
        }
        size_t filename_len = _tcslen(ptr);
        //ファイル名が""でくくられてたら取り除く
        if (ptr[0] == _T('\"') && ptr[filename_len-1] == _T('\"')) {
            filename_len -= 2;
            ptr++;
        }
        //ファイル名が重複していないかを確認する
        for (int j = 0; j < pParams->nAudioSelectCount; j++) {
            if (pParams->ppAudioSelectList[j]->pAudioExtractFilename != nullptr
                && 0 == _tcsicmp(pParams->ppAudioSelectList[j]->pAudioExtractFilename, ptr)) {
                SET_ERR(strInput[0], _T("Same output file name is used more than twice"), option_name, nullptr);
                return 1;
            }
        }

        if (audioIdx < 0) {
            audioIdx = pParams->nAudioSelectCount;
            //新たに要素を追加
            pParams->ppAudioSelectList = (sAudioSelect **)realloc(pParams->ppAudioSelectList, sizeof(pParams->ppAudioSelectList[0]) * (pParams->nAudioSelectCount + 1));
            pParams->ppAudioSelectList[pParams->nAudioSelectCount] = pAudioSelect;
            pParams->nAudioSelectCount++;
        }
        pParams->ppAudioSelectList[audioIdx]->pAudioExtractFilename = _tcsdup(ptr);
        argData->nParsedAudioFile++;
        return 0;
    }
    if (   0 == _tcscmp(option_name, _T("format"))
        || 0 == _tcscmp(option_name, _T("output-format"))) {
        if (i+1 < nArgNum && strInput[i+1][0] != _T('-')) {
            i++;
            pParams->AVMuxOutputFormat = strInput[i];
            if (0 != _tcsicmp(strInput[i], _T("raw"))) {
                pParams->nAVMux |= RGY_MUX_VIDEO;
            }
        }
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("input-format"))) {
        if (i+1 < nArgNum && strInput[i+1][0] != _T('-')) {
            pParams->pAVInputFormat = _tcsdup(strInput[i]);
        } else {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return 1;
        }
        return 0;
    }
#if ENABLE_AVSW_READER
    auto set_audio_prm = [&](std::function<void(sAudioSelect *pAudioSelect, int trackId, const TCHAR *prmstr)> func_set) {
        const TCHAR *ptr = nullptr;
        const TCHAR *ptrDelim = nullptr;
        int trackId = 0;
        if (i+1 < nArgNum) {
            if (strInput[i+1][0] != _T('-') && strInput[i+1][0] != _T('\0')) {
                i++;
                ptrDelim = _tcschr(strInput[i], _T('?'));
                ptr = (ptrDelim == nullptr) ? strInput[i] : ptrDelim+1;
            }
            if (ptrDelim != nullptr) {
                tstring temp = tstring(strInput[i]).substr(0, ptrDelim - strInput[i]);
                trackId = std::stoi(temp);
            }
        }
        sAudioSelect *pAudioSelect = nullptr;
        int audioIdx = getAudioTrackIdx(pParams, trackId);
        if (audioIdx < 0) {
            pAudioSelect = (sAudioSelect *)calloc(1, sizeof(pAudioSelect[0]));
            if (trackId != 0) {
                //もし、trackID=0以外の指定であれば、
                //これまでalltrackに指定されたパラメータを探して引き継ぐ
                sAudioSelect *pAudioSelectAll = nullptr;
                for (int itrack = 0; itrack < pParams->nAudioSelectCount; itrack++) {
                    if (pParams->ppAudioSelectList[itrack]->nAudioSelect == 0) {
                        pAudioSelectAll = pParams->ppAudioSelectList[itrack];
                    }
                }
                if (pAudioSelectAll) {
                    *pAudioSelect = *pAudioSelectAll;
                }
            }
            pAudioSelect->nAudioSelect = trackId;
        } else {
            pAudioSelect = pParams->ppAudioSelectList[audioIdx];
        }
        func_set(pAudioSelect, trackId, ptr);
        if (trackId == 0) {
            for (int itrack = 0; itrack < pParams->nAudioSelectCount; itrack++) {
                func_set(pParams->ppAudioSelectList[itrack], trackId, ptr);
            }
        }

        if (audioIdx < 0) {
            audioIdx = pParams->nAudioSelectCount;
            //新たに要素を追加
            pParams->ppAudioSelectList = (sAudioSelect **)realloc(pParams->ppAudioSelectList, sizeof(pParams->ppAudioSelectList[0]) * (pParams->nAudioSelectCount + 1));
            pParams->ppAudioSelectList[pParams->nAudioSelectCount] = pAudioSelect;
            pParams->nAudioSelectCount++;
        }
        return 0;
    };
    if (   0 == _tcscmp(option_name, _T("audio-copy"))
        || 0 == _tcscmp(option_name, _T("copy-audio"))) {
        pParams->nAVMux |= (RGY_MUX_VIDEO | RGY_MUX_AUDIO);
        std::set<int> trackSet; //重複しないよう、setを使う
        if (i+1 < nArgNum && strInput[i+1][0] != _T('-')) {
            i++;
            auto trackListStr = split(strInput[i], _T(","));
            for (auto str : trackListStr) {
                int iTrack = 0;
                if (1 != _stscanf(str.c_str(), _T("%d"), &iTrack) || iTrack < 1) {
                    SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
                    return 1;
                } else {
                    trackSet.insert(iTrack);
                }
            }
        } else {
            trackSet.insert(0);
        }

        for (auto it = trackSet.begin(); it != trackSet.end(); it++) {
            int trackId = *it;
            sAudioSelect *pAudioSelect = nullptr;
            int audioIdx = getAudioTrackIdx(pParams, trackId);
            if (audioIdx < 0) {
                pAudioSelect = (sAudioSelect *)calloc(1, sizeof(pAudioSelect[0]));
                pAudioSelect->nAudioSelect = trackId;
            } else {
                pAudioSelect = pParams->ppAudioSelectList[audioIdx];
            }
            pAudioSelect->pAVAudioEncodeCodec = _tcsdup(RGY_AVCODEC_COPY);

            if (audioIdx < 0) {
                audioIdx = pParams->nAudioSelectCount;
                //新たに要素を追加
                pParams->ppAudioSelectList = (sAudioSelect **)realloc(pParams->ppAudioSelectList, sizeof(pParams->ppAudioSelectList[0]) * (pParams->nAudioSelectCount + 1));
                pParams->ppAudioSelectList[pParams->nAudioSelectCount] = pAudioSelect;
                pParams->nAudioSelectCount++;
            }
            argData->nParsedAudioCopy++;
        }
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("audio-codec"))) {
        pParams->nAVMux |= (RGY_MUX_VIDEO | RGY_MUX_AUDIO);
        auto ret = set_audio_prm([](sAudioSelect *pAudioSelect, int trackId, const TCHAR *prmstr) {
            if (trackId != 0 || pAudioSelect->pAVAudioEncodeCodec == nullptr) {
                if (prmstr == nullptr) {
                    pAudioSelect->pAVAudioEncodeCodec = _tcsdup(RGY_AVCODEC_AUTO);
                } else {
                    pAudioSelect->pAVAudioEncodeCodec = _tcsdup(prmstr);
                    auto delim = _tcschr(pAudioSelect->pAVAudioEncodeCodec, _T(':'));
                    if (delim != nullptr) {
                        pAudioSelect->pAVAudioEncodeCodecPrm = _tcsdup(delim+1);
                        delim[0] = _T('\0');
                    } else {
                        pAudioSelect->pAVAudioEncodeCodecPrm = nullptr;
                    }
                }
            }
        });
        if (ret) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return ret;
        }
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("audio-profile"))) {
        pParams->nAVMux |= (RGY_MUX_VIDEO | RGY_MUX_AUDIO);
        auto ret = set_audio_prm([](sAudioSelect *pAudioSelect, int trackId, const TCHAR *prmstr) {
            if (trackId != 0 || pAudioSelect->pAVAudioEncodeCodecProfile == nullptr) {
                pAudioSelect->pAVAudioEncodeCodecProfile = _tcsdup(prmstr);
            }
        });
        if (ret) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return ret;
        }
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("audio-bitrate"))) {
        try {
            auto ret = set_audio_prm([](sAudioSelect *pAudioSelect, int trackId, const TCHAR *prmstr) {
                if (trackId != 0 || pAudioSelect->nAVAudioEncodeBitrate == 0) {
                    pAudioSelect->nAVAudioEncodeBitrate = std::stoi(prmstr);
                }
            });
            return ret;
        } catch (...) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return 1;
        }
    }
    if (0 == _tcscmp(option_name, _T("audio-ignore-decode-error"))) {
        i++;
        uint32_t value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return 1;
        }
        pParams->nAudioIgnoreDecodeError = value;
        return 0;
    }
    //互換性のため残す
    if (0 == _tcscmp(option_name, _T("audio-ignore-notrack-error"))) {
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("audio-samplerate"))) {
        try {
            auto ret = set_audio_prm([](sAudioSelect *pAudioSelect, int trackId, const TCHAR *prmstr) {
                if (trackId != 0 || pAudioSelect->nAudioSamplingRate == 0) {
                    pAudioSelect->nAudioSamplingRate = std::stoi(prmstr);
                }
            });
            return ret;
        } catch (...) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return 1;
        }
    }
    if (0 == _tcscmp(option_name, _T("audio-resampler"))) {
        i++;
        int v = 0;
        if (PARSE_ERROR_FLAG != (v = get_value_from_chr(list_resampler, strInput[i]))) {
            pParams->nAudioResampler = v;
        } else if (1 == _stscanf_s(strInput[i], _T("%d"), &v) && 0 <= v && v < _countof(list_resampler) - 1) {
            pParams->nAudioResampler = v;
        } else {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return 1;
        }
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("audio-stream"))) {
        //ここで、av_get_channel_layout()を使うため、チェックする必要がある
        if (!check_avcodec_dll()) {
            _ftprintf(stderr, _T("%s\n--audio-stream could not be used.\n"), error_mes_avcodec_dll_not_found().c_str());
            return 1;
        }

        try {
            auto ret = set_audio_prm([](sAudioSelect *pAudioSelect, int trackId, const TCHAR *prmstr) {
                if (trackId != 0 || (pAudioSelect->pnStreamChannelSelect[0] == 0 && pAudioSelect->pnStreamChannelOut[0] == 0)) {
                    auto streamSelectList = split(tchar_to_string(prmstr), ",");
                    if (streamSelectList.size() > _countof(pAudioSelect->pnStreamChannelSelect)) {
                        return 1;
                    }
                    static const char *DELIM = ":";
                    for (uint32_t j = 0; j < streamSelectList.size(); j++) {
                        auto selectPtr = streamSelectList[j].c_str();
                        auto selectDelimPos = strstr(selectPtr, DELIM);
                        if (selectDelimPos == nullptr) {
                            auto channelLayout = av_get_channel_layout(selectPtr);
                            pAudioSelect->pnStreamChannelSelect[j] = channelLayout;
                            pAudioSelect->pnStreamChannelOut[j]    = RGY_CHANNEL_AUTO; //自動
                        } else if (selectPtr == selectDelimPos) {
                            pAudioSelect->pnStreamChannelSelect[j] = RGY_CHANNEL_AUTO;
                            pAudioSelect->pnStreamChannelOut[j]    = av_get_channel_layout(selectDelimPos + strlen(DELIM));
                        } else {
                            pAudioSelect->pnStreamChannelSelect[j] = av_get_channel_layout(streamSelectList[j].substr(0, selectDelimPos - selectPtr).c_str());
                            pAudioSelect->pnStreamChannelOut[j]    = av_get_channel_layout(selectDelimPos + strlen(DELIM));
                        }
                    }
                }
                return 0;
            });
            if (ret) {
                SET_ERR(strInput[0], _T("Too much streams splitted"), option_name, strInput[i]);
                return ret;
            }
            return ret;
        } catch (...) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return 1;
        }
    }
    if (0 == _tcscmp(option_name, _T("audio-filter"))) {
        try {
            auto ret = set_audio_prm([](sAudioSelect *pAudioSelect, int trackId, const TCHAR *prmstr) {
                if (trackId != 0 || (pAudioSelect->pAudioFilter == nullptr)) {
                    if (pAudioSelect->pAudioFilter) {
                        free(pAudioSelect->pAudioFilter);
                    }
                    pAudioSelect->pAudioFilter = _tcsdup(prmstr);
                }
            });
            return ret;
        } catch (...) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return 1;
        }
    }
#endif //#if ENABLE_AVCODEC_QSV_READER
    if (   0 == _tcscmp(option_name, _T("chapter-copy"))
        || 0 == _tcscmp(option_name, _T("copy-chapter"))) {
        pParams->bCopyChapter = TRUE;
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("chapter"))) {
        if (i+1 < nArgNum && strInput[i+1][0] != _T('-')) {
            i++;
            pParams->sChapterFile = strInput[i];
        } else {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i+1]);
            return 1;
        }
        return 0;
    }
    if (   0 == _tcscmp(option_name, _T("sub-copy"))
        || 0 == _tcscmp(option_name, _T("copy-sub"))) {
        pParams->nAVMux |= (RGY_MUX_VIDEO | RGY_MUX_SUBTITLE);
        std::set<int> trackSet; //重複しないよう、setを使う
        if (i+1 < nArgNum && strInput[i+1][0] != _T('-')) {
            i++;
            auto trackListStr = split(strInput[i], _T(","));
            for (auto str : trackListStr) {
                int iTrack = 0;
                if (1 != _stscanf(str.c_str(), _T("%d"), &iTrack) || iTrack < 1) {
                    SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
                    return 1;
                } else {
                    trackSet.insert(iTrack);
                }
            }
        } else {
            trackSet.insert(0);
        }
        for (int iTrack = 0; iTrack < pParams->nSubtitleSelectCount; iTrack++) {
            trackSet.insert(pParams->pSubtitleSelect[iTrack]);
        }
        if (pParams->pSubtitleSelect) {
            free(pParams->pSubtitleSelect);
        }

        pParams->pSubtitleSelect = (int *)malloc(sizeof(pParams->pSubtitleSelect[0]) * trackSet.size());
        pParams->nSubtitleSelectCount = (int)trackSet.size();
        int iTrack = 0;
        for (auto it = trackSet.begin(); it != trackSet.end(); it++, iTrack++) {
            pParams->pSubtitleSelect[iTrack] = *it;
        }
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("caption2ass"))) {
        if (i+1 < nArgNum && strInput[i+1][0] != _T('-')) {
            i++;
            C2AFormat format = FORMAT_INVALID;
            if (PARSE_ERROR_FLAG != (format = (C2AFormat)get_value_from_chr(list_caption2ass, strInput[i]))) {
                pParams->caption2ass = format;
            } else {
                SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
                return 1;
            }
        } else {
            pParams->caption2ass = FORMAT_SRT;
        }
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("no-caption2ass"))) {
        pParams->caption2ass = FORMAT_INVALID;
        return 0;
    }
#if 0
    if (0 == _tcscmp(option_name, _T("avsync"))) {
        int value = 0;
        i++;
        if (PARSE_ERROR_FLAG != (value = get_value_from_chr(list_avsync, strInput[i]))) {
            pParams->nAVSyncMode = (RGYAVSync)value;
        } else {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return 1;
        }
        return 0;
    }
#endif
    if (0 == _tcscmp(option_name, _T("mux-option"))) {
        if (i+1 < nArgNum && strInput[i+1][0] != _T('-')) {
            i++;
            auto ptr = _tcschr(strInput[i], ':');
            if (ptr == nullptr) {
                SET_ERR(strInput[0], _T("invalid value"), option_name, nullptr);
                return 1;
            } else {
                if (pParams->pMuxOpt == nullptr) {
                    pParams->pMuxOpt = new muxOptList();
                }
                pParams->pMuxOpt->push_back(std::make_pair<tstring, tstring>(tstring(strInput[i]).substr(0, ptr - strInput[i]), tstring(ptr+1)));
            }
        } else {
            SET_ERR(strInput[0], _T("invalid option"), option_name, nullptr);
            return 1;
        }
        return 0;
    }
    if (IS_OPTION("quality")) {
        i++;
        int value = AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_vce_quality_preset, strInput[i]))) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->qualityPreset = value;
        return 0;
    }
    if (IS_OPTION("codec")) {
        i++;
        int value = 0;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_codec_all, strInput[i]))) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->codec = (RGY_CODEC)value;
        return 0;
    }
    if (IS_OPTION("level")) {
        if (i+1 < nArgNum) {
            i++;
            argData->cachedlevel = strInput[i];
        } else {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("profile")) {
        if (i+1 < nArgNum) {
            i++;
            argData->cachedprofile = strInput[i];
        } else {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("tier")) {
        if (i+1 < nArgNum) {
            i++;
            argData->cachedtier = strInput[i];
        } else {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("sar") || IS_OPTION("dar")) {
        i++;
        int value[2] = { 0 };
        if (   2 != _stscanf_s(strInput[i], _T("%dx%d"), &value[0], &value[1])
            && 2 != _stscanf_s(strInput[i], _T("%d,%d"), &value[0], &value[1])
            && 2 != _stscanf_s(strInput[i], _T("%d/%d"), &value[0], &value[1])
            && 2 != _stscanf_s(strInput[i], _T("%d:%d"), &value[0], &value[1])) {
            pParams->par[0] = 0;
            pParams->par[1] = 0;
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        if (IS_OPTION("dar")) {
            value[0] = -value[0];
            value[1] = -value[1];
        }
        pParams->par[0] = value[0];
        pParams->par[1] = value[1];
        return 0;
    }
    if (IS_OPTION("cqp")) {
        i++;
        int a = 0, b = 0, c = 0;
        if (   3 != _stscanf_s(strInput[i], _T("%d:%d:%d"), &a, &b, &c)
            && 3 != _stscanf_s(strInput[i], _T("%d/%d/%d"), &a, &b, &c)
            && 3 != _stscanf_s(strInput[i], _T("%d,%d,%d"), &a, &b, &c)) {
            if (1 != _stscanf_s(strInput[i], _T("%d"), &a)) {
                SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
                return -1;
            }
            b = a;
            c = a;
        }
        if (a < 0 || b < 0 || c < 0) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->rateControl = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP;
        pParams->nQPI = a;
        pParams->nQPP = b;
        pParams->nQPB = c;
        return 0;
    }
    if (IS_OPTION("vbr")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->rateControl = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
        pParams->nBitrate = value;
        return 0;
    }
    if (IS_OPTION("cbr")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->rateControl = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR;
        pParams->nBitrate = value;
        return 0;
    }
    if (IS_OPTION("qp-max")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nQPMax = value;
        return 0;
    }
    if (IS_OPTION("qp-min")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nQPMin = value;
        return 0;
    }
    if (IS_OPTION("bframes")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nBframes = value;
        return 0;
    }
    if (IS_OPTION("b-pyramid")) {
        pParams->bBPyramid = true;
        return 0;
    }
    if (IS_OPTION("no-b-pyramid")) {
        pParams->bBPyramid = false;
        return 0;
    }
    if (IS_OPTION("b-deltaqp")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nDeltaQPBFrame = value;
        return 0;
    }
    if (IS_OPTION("bref-deltaqp")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nDeltaQPBFrameRef = value;
        return 0;
    }
    if (IS_OPTION("ref")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nRefFrames = value;
        return 0;
    }
    if (IS_OPTION("ltr")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nLTRFrames = value;
        return 0;
    }
    if (IS_OPTION("max-bitrate")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nMaxBitrate = value;
        return 0;
    }
    if (IS_OPTION("vbv-bufsize")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nVBVBufferSize = value;
        return 0;
    }
    if (IS_OPTION("slices")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nSlices = value;
        return 0;
    }
    if (IS_OPTION("skip-frame")) {
        pParams->bEnableSkipFrame = true;
        return 0;
    }
    if (IS_OPTION("no-skip-frame")) {
        pParams->bEnableSkipFrame = false;
        return 0;
    }
    if (IS_OPTION("motion-est")) {
        i++;
        int value = VCE_MOTION_EST_QUATER | VCE_MOTION_EST_HALF;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_mv_presicion, strInput[i]))) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nMotionEst = value;
        return 0;
    }
    if (IS_OPTION("vbaq")) {
        pParams->bVBAQ = true;
        return 0;
    }
    if (IS_OPTION("no-pre-analysis")) {
        pParams->preAnalysis = false;
        return 0;
    }
    if (IS_OPTION("pre-analysis")) {
        pParams->preAnalysis = true;
        return 0;
    }
    if (IS_OPTION("gop-len")) {
        i++;
        int value = 0;
        if (_tcscmp(strInput[i], _T("auto")) == 0) {
            value = 0;
        } else if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nGOPLen = value;
        return 0;
    }
    if (IS_OPTION("tff")) {
        pParams->input.picstruct = RGY_PICSTRUCT_FRAME_TFF;
        return 0;
    }
    if (IS_OPTION("bff")) {
        pParams->input.picstruct = RGY_PICSTRUCT_FRAME_BFF;
        return 0;
    }
    if (IS_OPTION("interlace") || IS_OPTION("interlaced")) {
        i++;
        int value = 0;
        if (get_list_value(list_interlaced, strInput[i], &value)) {
            pParams->input.picstruct = (RGY_PICSTRUCT)value;
        } else {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("fullrange")) {
        pParams->vui.fullrange = TRUE;
        return 0;
    }
    if (IS_OPTION("max-cll")) {
        i++;
        pParams->sMaxCll = tchar_to_string(strInput[i]);
        return 0;
    }
    if (IS_OPTION("master-display")) {
        i++;
        pParams->sMasterDisplay = tchar_to_string(strInput[i]);
        return 0;
    }
    if (IS_OPTION("colormatrix")) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG != (value = get_value_from_chr(list_colormatrix, strInput[i]))) {
            pParams->vui.matrix = value;
        }
        return 0;
    }
    if (IS_OPTION("colorprim")) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG != (value = get_value_from_chr(list_colorprim, strInput[i]))) {
            pParams->vui.colorprim = value;
        }
        return 0;
    }
    if (IS_OPTION("transfer")) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG != (value = get_value_from_chr(list_transfer, strInput[i]))) {
            pParams->vui.transfer = value;
        }
        return 0;
    }
    if (IS_OPTION("videoformat")) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG != (value = get_value_from_chr(list_videoformat, strInput[i]))) {
            pParams->vui.format = value;
        }
        return 0;
    }
    if (IS_OPTION("filler")) {
        pParams->bFiller = TRUE;
        return 0;
    }
    if (IS_OPTION("enforce-hrd")) {
        pParams->bEnforceHRD = TRUE;
        return 0;
    }
    if (IS_OPTION("log")) {
        i++;
        pParams->logfile = strInput[i];
        return 0;
    }
    if (IS_OPTION("log-level")) {
        i++;
        int value = 0;
        if (get_list_value(list_log_level, strInput[i], &value)) {
            pParams->loglevel = value;
        } else {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("log-framelist")) {
        i++;
        pParams->sFramePosListLog = strInput[i];
        return 0;
    }
    if (IS_OPTION("log-mux-ts")) {
        i++;
        pParams->pMuxVidTsLogFile = _tcsdup(strInput[i]);
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("output-buf"))) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return 1;
        }
        if (value < 0) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return 1;
        }
        pParams->nOutputBufSizeMB = (std::min)(value, RGY_OUTPUT_BUF_MB_MAX);
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("input-thread"))) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return 1;
        }
        if (value < -1 || value >= 2) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return 1;
        }
        pParams->nInputThread = (int8_t)value;
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("no-output-thread"))) {
        pParams->nOutputThread = 0;
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("output-thread"))) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return 1;
        }
        if (value < -1 || value >= 2) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return 1;
        }
        pParams->nOutputThread = value;
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("audio-thread"))) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return 1;
        }
        if (value < -1 || value >= 3) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return 1;
        }
        pParams->nAudioThread = value;
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("max-procfps"))) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return 1;
        }
        if (value < 0) {
            SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return 1;
        }
        pParams->nProcSpeedLimit = (uint16_t)(std::min)(value, (int)UINT16_MAX);
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("perf-monitor"))) {
        if (strInput[i+1][0] == _T('-') || _tcslen(strInput[i+1]) == 0) {
            pParams->nPerfMonitorSelect = (int)PERF_MONITOR_ALL;
        } else {
            i++;
            auto items = split(strInput[i], _T(","));
            for (const auto& item : items) {
                int value = 0;
                if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_pref_monitor, item.c_str()))) {
                    SET_ERR(strInput[0], _T("Unknown value"), option_name, item.c_str());
                    return 1;
                }
                pParams->nPerfMonitorSelect |= value;
            }
        }
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("perf-monitor-interval"))) {
        i++;
        int v;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &v)) {
            SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return 1;
        }
        pParams->nPerfMonitorInterval = std::max(50, v);
        return 0;
    }
    tstring mes = _T("Unknown option: --");
    mes += option_name;
    SET_ERR(strInput[0], (TCHAR *)mes.c_str(), NULL, strInput[i]);
    return -1;
}
#undef IS_OPTION

int parse_cmd(VCEParam *pParams, int nArgNum, const TCHAR **strInput, ParseCmdError& err, bool ignore_parse_err) {
    sArgsData argsData;

    for (int i = 1; i < nArgNum; i++) {
        if (strInput[i] == nullptr) {
            return -1;
        }
        const TCHAR *option_name = nullptr;
        if (strInput[i][0] == _T('-')) {
            if (strInput[i][1] == _T('-')) {
                option_name = &strInput[i][2];
            } else if (strInput[i][2] == _T('\0')) {
                if (nullptr == (option_name = cmd_short_opt_to_long(strInput[i][1]))) {
                    SET_ERR(strInput[0], strsprintf(_T("Unknown options: \"%s\""), strInput[i]).c_str(), nullptr, nullptr);
                    return -1;
                }
            } else {
                if (ignore_parse_err) continue;
                SET_ERR(strInput[0], strsprintf(_T("Invalid options: \"%s\""), strInput[i]).c_str(), nullptr, nullptr);
                return -1;
            }
        }

        if (nullptr == option_name) {
            if (ignore_parse_err) continue;
            SET_ERR(strInput[0], strsprintf(_T("Unknown option: \"%s\""), strInput[i]).c_str(), nullptr, nullptr);
            return -1;
        }
        auto sts = parse_one_option(option_name, strInput, i, nArgNum, pParams, &argsData, err);
        if (!ignore_parse_err && sts != 0) {
            return sts;
        }
    }

    //parse cached profile and level
    if (argsData.cachedlevel.length() > 0) {
        const auto desc = get_level_list(pParams->codec);
        int value = 0;
        bool bParsed = false;
        if (desc != nullptr) {
            if (PARSE_ERROR_FLAG != (value = get_value_from_chr(desc, argsData.cachedlevel.c_str()))) {
                pParams->codecParam[pParams->codec].nLevel = value;
                bParsed = true;
            } else {
                double val_float = 0.0;
                if (1 == _stscanf_s(argsData.cachedlevel.c_str(), _T("%lf"), &val_float)) {
                    value = (int)(val_float * 10 + 0.5);
                    if (value == desc[get_cx_index(desc, value)].value) {
                        pParams->codecParam[pParams->codec].nLevel = value;
                        bParsed = true;
                    } else {
                        value = (int)(val_float + 0.5);
                        if (value == desc[get_cx_index(desc, value)].value) {
                            pParams->codecParam[pParams->codec].nLevel = value;
                            bParsed = true;
                        }
                    }
                }
            }
        }
        if (!bParsed) {
            SET_ERR(strInput[0], _T("Unknown value"), _T("level"), argsData.cachedlevel.c_str());
            return -1;
        }
    }
    if (argsData.cachedprofile.length() > 0) {
        const auto desc = get_profile_list(pParams->codec);
        int value = 0;
        if (desc != nullptr && PARSE_ERROR_FLAG != (value = get_value_from_chr(desc, argsData.cachedprofile.c_str()))) {
            pParams->codecParam[pParams->codec].nProfile = (int16_t)value;
        } else {
            SET_ERR(strInput[0], _T("Unknown value"), _T("profile"), argsData.cachedprofile.c_str());
            return -1;
        }
    }
    if (argsData.cachedtier.length() > 0) {
        const auto desc = get_tier_list(pParams->codec);
        int value = 0;
        if (desc != nullptr && PARSE_ERROR_FLAG != (value = get_value_from_chr(desc, argsData.cachedtier.c_str()))) {
            pParams->codecParam[pParams->codec].nTier = (int16_t)value;
        } else {
            SET_ERR(strInput[0], _T("Unknown value"), _T("tier"), argsData.cachedtier.c_str());
            return -1;
        }
    }
    if (pParams->codec == RGY_CODEC_HEVC) {
        int nH264RateControl = pParams->rateControl;
        switch (nH264RateControl) {
        case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR:
            pParams->rateControl = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CBR;
            break;
        case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR:
            pParams->rateControl = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
            break;
        case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP:
        default:
            pParams->rateControl = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CONSTANT_QP;
            break;
        }
    }

    return 0;
}

int parse_cmd(VCEParam *pParams, const char *cmda, ParseCmdError& err, bool ignore_parse_err) {
    if (cmda == nullptr) {
        return 0;
    }
    std::wstring cmd = char_to_wstring(cmda);
    int argc = 0;
    auto argvw = CommandLineToArgvW(cmd.c_str(), &argc);
    if (argc <= 1) {
        return 0;
    }
    vector<tstring> argv_tstring;
    for (int i = 0; i < argc; i++) {
        argv_tstring.push_back(wstring_to_tstring(argvw[i]));
    }
    LocalFree(argvw);

    vector<TCHAR *> argv_tchar;
    for (int i = 0; i < argc; i++) {
        argv_tchar.push_back((TCHAR *)argv_tstring[i].data());
    }
    argv_tchar.push_back(_T(""));
    const TCHAR **strInput = (const TCHAR **)argv_tchar.data();
    int ret = parse_cmd(pParams, argc, strInput, err, ignore_parse_err);
    return ret;
}

#pragma warning (push)
#pragma warning (disable: 4127)
tstring gen_cmd(const VCEParam *pParams, bool save_disabled_prm) {
    std::basic_stringstream<TCHAR> cmd;
    VCEParam encPrmDefault;

#define OPT_FLOAT(str, opt, prec) if ((pParams->opt) != (encPrmDefault.opt)) cmd << _T(" ") << (str) << _T(" ") << std::setprecision(prec) << (pParams->opt);
#define OPT_NUM(str, opt) if ((pParams->opt) != (encPrmDefault.opt)) cmd << _T(" ") << (str) << _T(" ") << (int)(pParams->opt);
#define OPT_NUM_HEVC(str, codec, opt) if ((pParams->codecParam[RGY_CODEC_HEVC].opt) != (encPrmDefault.codecParam[RGY_CODEC_HEVC].opt)) cmd << _T(" ") << (str) << ((save_disabled_prm) ? codec : _T("")) << _T(" ") << (int)(pParams->codecParam[RGY_CODEC_HEVC].opt);
#define OPT_NUM_H264(str, codec, opt) if ((pParams->codecParam[RGY_CODEC_H264].opt) != (encPrmDefault.codecParam[RGY_CODEC_H264].opt)) cmd << _T(" ") << (str) << ((save_disabled_prm) ? codec : _T("")) << _T(" ") << (int)(pParams->codecParam[RGY_CODEC_H264].opt);
#define OPT_GUID(str, opt, list) if ((pParams->opt) != (encPrmDefault.opt)) cmd << _T(" ") << (str) << _T(" ") << get_name_from_guid((pParams->opt), list);
#define OPT_GUID_HEVC(str, codec, opt, list) if ((pParams->codecParam[RGY_CODEC_HEVC].opt) != (encPrmDefault.codecParam[RGY_CODEC_HEVC].opt)) cmd << _T(" ") << (str) << ((save_disabled_prm) ? codec : _T("")) << _T(" ") << get_name_from_value((pParams->codecParam[RGY_CODEC_HEVC].opt), list);
#define OPT_LST(str, opt, list) if ((pParams->opt) != (encPrmDefault.opt)) cmd << _T(" ") << (str) << _T(" ") << get_chr_from_value(list, (pParams->opt));
#define OPT_LST_HEVC(str, codec, opt, list) if ((pParams->codecParam[RGY_CODEC_HEVC].opt) != (encPrmDefault.codecParam[RGY_CODEC_HEVC].opt)) cmd << _T(" ") << (str) << ((save_disabled_prm) ? codec : _T("")) << _T(" ") << get_chr_from_value(list, (pParams->codecParam[RGY_CODEC_HEVC].opt));
#define OPT_LST_H264(str, codec, opt, list) if ((pParams->codecParam[RGY_CODEC_H264].opt) != (encPrmDefault.codecParam[RGY_CODEC_H264].opt)) cmd << _T(" ") << (str) << ((save_disabled_prm) ? codec : _T("")) << _T(" ") << get_chr_from_value(list, (pParams->codecParam[RGY_CODEC_H264].opt));
#define OPT_QP(str, qpi, qpp, qpb, enable, force) { \
    if ((force) || (enable) \
    || (pParams->qpi) != (encPrmDefault.qpi) \
    || (pParams->qpp) != (encPrmDefault.qpp) \
    || (pParams->qpb) != (encPrmDefault.qpb)) { \
        if (enable) { \
            cmd << _T(" ") << (str) << _T(" "); \
        } else { \
            cmd << _T(" ") << (str) << _T(" 0;"); \
        } \
        if ((pParams->qpi) == (pParams->qpp) && (pParams->qpi) == (pParams->qpb)) { \
            cmd << (int)(pParams->qpi); \
        } else { \
            cmd << (int)(pParams->qpi) << _T(":") << (int)(pParams->qpp) << _T(":") << (int)(pParams->qpb); \
        } \
    } \
}
#define OPT_BOOL(str_true, str_false, opt) if ((pParams->opt) != (encPrmDefault.opt)) cmd << _T(" ") << ((pParams->opt) ? (str_true) : (str_false));
#define OPT_BOOL_HEVC(str_true, str_false, codec, opt) \
    if ((pParams->codecParam[RGY_CODEC_HEVC].opt) != (encPrmDefault.codecParam[RGY_CODEC_HEVC].opt)) { \
        cmd << _T(" "); \
        if ((pParams->codecParam[RGY_CODEC_HEVC].opt)) { \
            if (_tcslen(str_true)) { cmd << (str_true) << ((save_disabled_prm) ? (codec) : _T("")); } \
        } else { \
            if (_tcslen(str_false)) { cmd << (str_false) << ((save_disabled_prm) ? (codec) : _T("")); } \
        } \
    }
#define OPT_BOOL_H264(str_true, str_false, codec, opt) \
    if ((pParams->codecParam[RGY_CODEC_H264].opt) != (encPrmDefault.codecParam[RGY_CODEC_H264].opt)) { \
        cmd << _T(" "); \
        if ((pParams->codecParam[RGY_CODEC_H264].opt)) { \
            if (_tcslen(str_true)) { cmd << (str_true) << ((save_disabled_prm) ? (codec) : _T("")); }\
        } else { \
            if (_tcslen(str_false)) { cmd << (str_false) << ((save_disabled_prm) ? (codec) : _T("")); }\
        } \
    }
#define OPT_CHAR(str, opt) if ((pParams->opt) && _tcslen(pParams->opt)) cmd << _T(" ") << str << _T(" ") << (pParams->opt);
#define OPT_STR(str, opt) if (pParams->opt.length() > 0) cmd << _T(" ") << str << _T(" ") << (pParams->opt.c_str());
#define OPT_CHAR_PATH(str, opt) if ((pParams->opt) && _tcslen(pParams->opt)) cmd << _T(" ") << str << _T(" \"") << (pParams->opt) << _T("\"");
#define OPT_STR_PATH(str, opt) if (pParams->opt.length() > 0) cmd << _T(" ") << str << _T(" \"") << (pParams->opt.c_str()) << _T("\"");

    OPT_NUM(_T("-d"), deviceID);
    cmd << _T(" -c ") << get_chr_from_value(list_codec, pParams->codec);
    OPT_STR_PATH(_T("-i"), inputFilename);
    OPT_STR_PATH(_T("-o"), outputFilename);
    switch (pParams->input.type) {
    case RGY_INPUT_FMT_RAW:    cmd << _T(" --raw"); break;
    case RGY_INPUT_FMT_Y4M:    cmd << _T(" --y4m"); break;
    case RGY_INPUT_FMT_AVI:    cmd << _T(" --avi"); break;
    case RGY_INPUT_FMT_AVS:    cmd << _T(" --avs"); break;
    case RGY_INPUT_FMT_VPY:    cmd << _T(" --vpy"); break;
    case RGY_INPUT_FMT_VPY_MT: cmd << _T(" --vpy-mt"); break;
    case RGY_INPUT_FMT_AVHW:   cmd << _T(" --avhw"); break;
    case RGY_INPUT_FMT_AVSW:   cmd << _T(" --avsw"); break;
    default: break;
    }
    if (save_disabled_prm || pParams->input.picstruct != RGY_PICSTRUCT_FRAME) {
        OPT_LST(_T("--interlace"), input.picstruct, list_interlaced);
    }
    if (cropEnabled(pParams->input.crop)) {
        cmd << _T(" --crop ") << pParams->input.crop.e.left << _T(",") << pParams->input.crop.e.up
            << _T(",") << pParams->input.crop.e.right << _T(",") << pParams->input.crop.e.bottom;
    }
    if (pParams->input.fpsN * pParams->input.fpsD > 0) {
        cmd << _T(" --fps ") << pParams->input.fpsN << _T("/") << pParams->input.fpsD;
    }
    if (pParams->input.srcWidth * pParams->input.srcHeight > 0) {
        cmd << _T(" --input-res ") << pParams->input.srcWidth << _T("x") << pParams->input.srcHeight;
    }
    if (pParams->input.dstWidth * pParams->input.dstHeight > 0) {
        cmd << _T(" --output-res ") << pParams->input.dstWidth << _T("x") << pParams->input.dstHeight;
    }
    if (save_disabled_prm) {
        switch (pParams->rateControl) {
        case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR:
        case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR:
        case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR: {
            OPT_QP(_T("--cqp"), nQPI, nQPP, nQPB, true, true);
        } break;
        case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP:
        default: {
            cmd << _T(" --vbr ") << pParams->nBitrate;
        } break;
        }
    }
    OPT_LST(_T("--quality"), qualityPreset, list_vce_quality_preset);
    switch (pParams->rateControl) {
    case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR: {
        cmd << _T(" --cbr ") << pParams->nBitrate;
    } break;
    case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR:
    case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR: {
        cmd << _T(" --vbr ") << pParams->nBitrate;
    } break;
    case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP:
    default: {
        OPT_QP(_T("--cqp"), nQPI, nQPP, nQPB, true, true);
    } break;
    }
    if (pParams->rateControl != AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP || save_disabled_prm) {
        OPT_NUM(_T("--vbv-bufsize"), nVBVBufferSize);
        OPT_NUM(_T("--max-bitrate"), nMaxBitrate);
    }
    if (pParams->nQPMin || save_disabled_prm) {
        OPT_NUM(_T("--qp-min"), nQPMin);
    }
    if (pParams->nQPMax || save_disabled_prm) {
        OPT_NUM(_T("--qp-max"), nQPMax);
    }
    if (pParams->nGOPLen == 0) {
        cmd << _T(" --gop-len auto");
    } else {
        OPT_NUM(_T("--gop-len"), nGOPLen);
    }
    OPT_NUM(_T("-b"), nBframes);
    OPT_BOOL(_T("--b-pyramid"), _T("--no-b-pyramid"), bBPyramid);
    OPT_NUM(_T("--b-deltaqp"), nDeltaQPBFrame);
    OPT_NUM(_T("--bref-deltaqp"), nDeltaQPBFrameRef);
    OPT_NUM(_T("--ref"), nRefFrames);
    OPT_NUM(_T("--ltr"), nLTRFrames);
    OPT_NUM(_T("--slices"), nSlices);
    OPT_BOOL(_T("--skip-frame"), _T("--no-skip-frame"), bEnableSkipFrame);
    OPT_BOOL(_T("--vbaq"), _T(""), bVBAQ);
    OPT_LST(_T("--motion-est"), nMotionEst, list_mv_presicion);
    if (pParams->par[0] > 0 && pParams->par[1] > 0) {
        cmd << _T(" --sar ") << pParams->par[0] << _T(":") << pParams->par[1];
    } else if (pParams->par[0] < 0 && pParams->par[1] < 0) {
        cmd << _T(" --dar ") << -1 * pParams->par[0] << _T(":") << -1 * pParams->par[1];
    }
    OPT_BOOL(_T("--filler"), _T(""), bFiller);
    OPT_BOOL(_T("--enforce-hrd"), _T(""), bEnforceHRD);
    OPT_BOOL(_T("--fullrange"), _T(""), vui.fullrange);
    OPT_LST(_T("--videoformat"), vui.format, list_videoformat);
    OPT_LST(_T("--colormatrix"), vui.matrix, list_colormatrix);
    OPT_LST(_T("--colorprim"), vui.colorprim, list_colorprim);
    //OPT_LST(_T("--chromaloc"), h264VUIParameters.chromaSampleLocationTop, list_chromaloc);
    OPT_LST(_T("--transfer"), vui.transfer, list_transfer);
    OPT_STR(_T("--max-cll"), sMaxCll);
    OPT_STR(_T("--master-display"), sMasterDisplay);

    if (pParams->codec == RGY_CODEC_HEVC || save_disabled_prm) {
        OPT_LST_HEVC(_T("--level"), _T(""), nLevel, list_hevc_level);
        OPT_LST_HEVC(_T("--profile"), _T(""), nProfile, list_hevc_profile);
        OPT_LST_HEVC(_T("--tier"), _T(""), nTier, list_hevc_tier);
    }
    if (pParams->codec == RGY_CODEC_H264 || save_disabled_prm) {
        OPT_LST_H264(_T("--level"), _T(""), nLevel, list_avc_level);
        OPT_LST_H264(_T("--profile"), _T(""), nProfile, list_avc_profile);
    }
    OPT_BOOL(_T("--pre-analysis"), _T("--no-pre-analysis"), preAnalysis);

    std::basic_stringstream<TCHAR> tmp;
#if ENABLE_AVSW_READER
    OPT_NUM(_T("--input-analyze"), nAVDemuxAnalyzeSec);
    if (pParams->nTrimCount > 0) {
        cmd << _T(" --trim ");
        for (int i = 0; i < pParams->nTrimCount; i++) {
            if (i > 0) cmd << _T(",");
            cmd << pParams->pTrimList[i].start << _T(":") << pParams->pTrimList[i].fin;
        }
    }
    OPT_FLOAT(_T("--seek"), fSeekSec, 2);
    OPT_CHAR(_T("--input-format"), pAVInputFormat);
    OPT_STR(_T("--output-format"), AVMuxOutputFormat);
    OPT_NUM(_T("--video-track"), nVideoTrack);
    OPT_NUM(_T("--video-streamid"), nVideoStreamId);
    if (pParams->pMuxOpt) {
        for (uint32_t i = 0; i < pParams->pMuxOpt->size(); i++) {
            cmd << _T(" -m ") << pParams->pMuxOpt->at(i).first << _T(":") << pParams->pMuxOpt->at(i).second;
        }
    }
    tmp.str(tstring());
    for (int i = 0; i < pParams->nAudioSelectCount; i++) {
        const sAudioSelect *pAudioSelect = pParams->ppAudioSelectList[i];
        if (_tcscmp(pAudioSelect->pAVAudioEncodeCodec, RGY_AVCODEC_COPY) == 0) {
            if (pAudioSelect->nAudioSelect == 0) {
                tmp << _T(","); // --audio-copy のみの指定 (トラックIDを省略)
            } else {
                tmp << _T(",") << pAudioSelect->nAudioSelect;
            }
        }
    }
    if (!tmp.str().empty()) {
        cmd << _T(" --audio-copy ") << tmp.str().substr(1);
    }
    tmp.str(tstring());

    for (int i = 0; i < pParams->nAudioSelectCount; i++) {
        const sAudioSelect *pAudioSelect = pParams->ppAudioSelectList[i];
        if (_tcscmp(pAudioSelect->pAVAudioEncodeCodec, RGY_AVCODEC_COPY) != 0) {
            cmd << _T(" --audio-codec ") << pAudioSelect->nAudioSelect;
            if (_tcscmp(pAudioSelect->pAVAudioEncodeCodec, RGY_AVCODEC_AUTO) != 0) {
                cmd << _T("?") << pAudioSelect->pAVAudioEncodeCodec;
            }
            if (pAudioSelect->pAVAudioEncodeCodecPrm) {
                cmd << _T(":") << pAudioSelect->pAVAudioEncodeCodecPrm;
            }
        }
    }

    for (int i = 0; i < pParams->nAudioSelectCount; i++) {
        const sAudioSelect *pAudioSelect = pParams->ppAudioSelectList[i];
        if (_tcscmp(pAudioSelect->pAVAudioEncodeCodec, RGY_AVCODEC_COPY) != 0
            && pAudioSelect->pAVAudioEncodeCodecProfile != nullptr) {
            cmd << _T(" --audio-profile ") << pAudioSelect->nAudioSelect << _T("?") << pAudioSelect->pAVAudioEncodeCodecProfile;
        }
    }

    for (int i = 0; i < pParams->nAudioSelectCount; i++) {
        const sAudioSelect *pAudioSelect = pParams->ppAudioSelectList[i];
        if (_tcscmp(pAudioSelect->pAVAudioEncodeCodec, RGY_AVCODEC_COPY) != 0) {
            cmd << _T(" --audio-bitrate ") << pAudioSelect->nAudioSelect << _T("?") << pAudioSelect->nAVAudioEncodeBitrate;
        }
    }

    for (int i = 0; i < pParams->nAudioSelectCount; i++) {
        tmp.str(tstring());
        const sAudioSelect *pAudioSelect = pParams->ppAudioSelectList[i];
        for (int j = 0; j < MAX_SPLIT_CHANNELS; j++) {
            if (pAudioSelect->pnStreamChannelSelect[j] == 0) {
                break;
            }
            if (j > 0) cmd << _T(",");
            if (pAudioSelect->pnStreamChannelSelect[j] != RGY_CHANNEL_AUTO) {
                char buf[256];
                av_get_channel_layout_string(buf, _countof(buf), 0, pAudioSelect->pnStreamChannelOut[j]);
                cmd << char_to_tstring(buf);
            }
            if (pAudioSelect->pnStreamChannelOut[j] != RGY_CHANNEL_AUTO) {
                cmd << _T(":");
                char buf[256];
                av_get_channel_layout_string(buf, _countof(buf), 0, pAudioSelect->pnStreamChannelOut[j]);
                cmd << char_to_tstring(buf);
            }
        }
        if (!tmp.str().empty()) {
            cmd << _T(" --audio-stream ") << pAudioSelect->nAudioSelect << _T("?") << tmp.str();
        }
    }
    tmp.str(tstring());

    for (int i = 0; i < pParams->nAudioSelectCount; i++) {
        const sAudioSelect *pAudioSelect = pParams->ppAudioSelectList[i];
        if (_tcscmp(pAudioSelect->pAVAudioEncodeCodec, RGY_AVCODEC_COPY) != 0) {
            cmd << _T(" --audio-samplerate ") << pAudioSelect->nAudioSelect << _T("?") << pAudioSelect->nAudioSamplingRate;
        }
    }
    OPT_LST(_T("--audio-resampler"), nAudioResampler, list_resampler);

    for (int i = 0; i < pParams->nAudioSelectCount; i++) {
        const sAudioSelect *pAudioSelect = pParams->ppAudioSelectList[i];
        if (_tcscmp(pAudioSelect->pAVAudioEncodeCodec, RGY_AVCODEC_COPY) != 0) {
            cmd << _T(" --audio-filter ") << pAudioSelect->nAudioSelect << _T("?") << pAudioSelect->pAudioFilter;
        }
    }
    for (int i = 0; i < pParams->nAudioSelectCount; i++) {
        const sAudioSelect *pAudioSelect = pParams->ppAudioSelectList[i];
        if (pAudioSelect->pAudioExtractFilename) {
            cmd << _T(" --audio-file ") << pAudioSelect->nAudioSelect << _T("?");
            if (pAudioSelect->pAudioExtractFormat) {
                cmd << pAudioSelect->pAudioExtractFormat << _T(":");
            }
            cmd << _T("\"") << pAudioSelect->pAudioExtractFilename << _T("\"");
        }
    }
    for (int i = 0; i < pParams->nAudioSourceCount; i++) {
        cmd << _T(" --audio-source ") << _T("\"") << pParams->ppAudioSourceList[i] << _T("\"");
    }
    OPT_NUM(_T("--audio-ignore-decode-error"), nAudioIgnoreDecodeError);
    if (pParams->pMuxOpt) {
        for (uint32_t i = 0; i < pParams->pMuxOpt->size(); i++) {
            cmd << _T(" -m ") << (*pParams->pMuxOpt)[i].first << _T(":") << (*pParams->pMuxOpt)[i].second;
        }
    }

    tmp.str(tstring());
    for (int i = 0; i < pParams->nSubtitleSelectCount; i++) {
        tmp << _T(",") << pParams->pSubtitleSelect[i];
    }
    if (!tmp.str().empty()) {
        cmd << _T(" --sub-copy ") << tmp.str().substr(1);
    }
    tmp.str(tstring());
    OPT_LST(_T("--caption2ass"), caption2ass, list_caption2ass);
    OPT_STR_PATH(_T("--chapter"), sChapterFile);
    OPT_BOOL(_T("--chapter-copy"), _T(""), bCopyChapter);
    //OPT_BOOL(_T("--chapter-no-trim"), _T(""), bChapterNoTrim);
    OPT_LST(_T("--avsync"), nAVSyncMode, list_avsync);
#endif //#if ENABLE_AVSW_READER
    OPT_NUM(_T("--output-buf"), nOutputBufSizeMB);
    OPT_NUM(_T("--output-thread"), nOutputThread);
    OPT_NUM(_T("--input-thread"), nInputThread);
    OPT_NUM(_T("--audio-thread"), nAudioThread);
    OPT_NUM(_T("--max-procfps"), nProcSpeedLimit);
    OPT_STR_PATH(_T("--log"), logfile);
    OPT_LST(_T("--log-level"), loglevel, list_log_level);
    OPT_STR_PATH(_T("--log-framelist"), sFramePosListLog);
    OPT_CHAR_PATH(_T("--log-mux-ts"), pMuxVidTsLogFile);
    if (pParams->nPerfMonitorSelect != encPrmDefault.nPerfMonitorSelect) {
        auto select = (int)pParams->nPerfMonitorSelect;
        tmp.str(tstring());
        for (int i = 0; list_pref_monitor[i].desc; i++) {
            auto check = list_pref_monitor[i].value;
            if ((select & check) == check) {
                tmp << _T(",") << list_pref_monitor[i].desc;
                select &= (~check);
            }
        }
        if (tmp.str().empty()) {
            cmd << _T(" --perf-monitor");
        } else {
            cmd << _T(" --perf-monitor ") << tmp.str().substr(1);
        }
    }
    OPT_NUM(_T("--perf-monitor-interval"), nPerfMonitorInterval);
    return cmd.str();
}
#pragma warning (pop)

#undef SET_ERR