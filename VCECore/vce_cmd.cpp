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
#include "core/Version.h"
#include "rgy_cmd.h"
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
        option_name = _T("input");
        break;
    case _T('o'):
        option_name = _T("output");
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

#if 0
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
#endif

int parse_one_option(const TCHAR *option_name, const TCHAR* strInput[], int& i, int nArgNum, VCEParam *pParams, sArgsData *argData, ParseCmdError& err) {

    if (IS_OPTION("device")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->deviceID = value;
        return 0;
    }
    if (IS_OPTION("interlop-d3d9")) {
        pParams->interlopD3d9 = true;
        return 0;
    }
    if (IS_OPTION("no-interlop-d3d9")) {
        pParams->interlopD3d9 = false;
        return 0;
    }
    if (IS_OPTION("interlop-d3d11")) {
        pParams->interlopD3d11 = true;
        return 0;
    }
    if (IS_OPTION("no-interlop-d3d11")) {
        pParams->interlopD3d11 = false;
        return 0;
    }
    if (IS_OPTION("quality")) {
        i++;
        int value = AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_vce_quality_preset, strInput[i]))) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->qualityPreset = value;
        return 0;
    }
    if (IS_OPTION("codec")) {
        i++;
        int value = 0;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_codec_all, strInput[i]))) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
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
            CMD_PARSE_SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("profile")) {
        if (i+1 < nArgNum) {
            i++;
            argData->cachedprofile = strInput[i];
        } else {
            CMD_PARSE_SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("tier")) {
        if (i+1 < nArgNum) {
            i++;
            argData->cachedtier = strInput[i];
        } else {
            CMD_PARSE_SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
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
            pParams->input.sar[0] = 0;
            pParams->input.sar[1] = 0;
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        if (IS_OPTION("dar")) {
            value[0] = -value[0];
            value[1] = -value[1];
        }
        pParams->input.sar[0] = value[0];
        pParams->input.sar[1] = value[1];
        return 0;
    }
    if (IS_OPTION("cqp")) {
        i++;
        int a = 0, b = 0, c = 0;
        if (   3 != _stscanf_s(strInput[i], _T("%d:%d:%d"), &a, &b, &c)
            && 3 != _stscanf_s(strInput[i], _T("%d/%d/%d"), &a, &b, &c)
            && 3 != _stscanf_s(strInput[i], _T("%d,%d,%d"), &a, &b, &c)) {
            if (1 != _stscanf_s(strInput[i], _T("%d"), &a)) {
                CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
                return -1;
            }
            b = a;
            c = a;
        }
        if (a < 0 || b < 0 || c < 0) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
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
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
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
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
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
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nQPMax = value;
        return 0;
    }
    if (IS_OPTION("qp-min")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nQPMin = value;
        return 0;
    }
    if (IS_OPTION("bframes")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
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
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nDeltaQPBFrame = value;
        return 0;
    }
    if (IS_OPTION("bref-deltaqp")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nDeltaQPBFrameRef = value;
        return 0;
    }
    if (IS_OPTION("ref")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nRefFrames = value;
        return 0;
    }
    if (IS_OPTION("ltr")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nLTRFrames = value;
        return 0;
    }
    if (IS_OPTION("max-bitrate")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nMaxBitrate = value;
        return 0;
    }
    if (IS_OPTION("vbv-bufsize")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nVBVBufferSize = value;
        return 0;
    }
    if (IS_OPTION("slices")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
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
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
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
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Invalid value"), option_name, strInput[i]);
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
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("fullrange")) {
        pParams->vui.fullrange = TRUE;
        return 0;
    }
    if (IS_OPTION("colormatrix")) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG != (value = get_value_from_chr(list_colormatrix, strInput[i]))) {
            pParams->vui.matrix = (CspMatrix)value;
        }
        return 0;
    }
    if (IS_OPTION("colorprim")) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG != (value = get_value_from_chr(list_colorprim, strInput[i]))) {
            pParams->vui.colorprim = (CspColorprim)value;
        }
        return 0;
    }
    if (IS_OPTION("transfer")) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG != (value = get_value_from_chr(list_transfer, strInput[i]))) {
            pParams->vui.transfer = (CspTransfer)value;
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
    if (0 == _tcscmp(option_name, _T("chromaloc"))) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_chromaloc, strInput[i]))) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return 1;
        }
        pParams->vui.chromaloc = value;
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
    if (0 == _tcscmp(option_name, _T("vpp-resize"))) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_vpp_resize, strInput[i]))) {
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return 1;
        }
        pParams->vpp.resize = (RGY_VPP_RESIZE_ALGO)value;
        return 0;
    }

    auto ret = parse_one_input_option(option_name, strInput, i, nArgNum, &pParams->input, argData, err);
    if (ret >= 0) return ret;

    ret = parse_one_common_option(option_name, strInput, i, nArgNum, &pParams->common, argData, err);
    if (ret >= 0) return ret;

    ret = parse_one_ctrl_option(option_name, strInput, i, nArgNum, &pParams->ctrl, argData, err);
    if (ret >= 0) return ret;

    tstring mes = _T("Unknown option: --");
    mes += option_name;
    CMD_PARSE_SET_ERR(strInput[0], (TCHAR *)mes.c_str(), NULL, strInput[i]);
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
                    CMD_PARSE_SET_ERR(strInput[0], strsprintf(_T("Unknown options: \"%s\""), strInput[i]).c_str(), nullptr, nullptr);
                    return -1;
                }
            } else {
                if (ignore_parse_err) continue;
                CMD_PARSE_SET_ERR(strInput[0], strsprintf(_T("Invalid options: \"%s\""), strInput[i]).c_str(), nullptr, nullptr);
                return -1;
            }
        }

        if (nullptr == option_name) {
            if (ignore_parse_err) continue;
            CMD_PARSE_SET_ERR(strInput[0], strsprintf(_T("Unknown option: \"%s\""), strInput[i]).c_str(), nullptr, nullptr);
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
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), _T("level"), argsData.cachedlevel.c_str());
            return -1;
        }
    }
    if (argsData.cachedprofile.length() > 0) {
        const auto desc = get_profile_list(pParams->codec);
        int value = 0;
        if (desc != nullptr && PARSE_ERROR_FLAG != (value = get_value_from_chr(desc, argsData.cachedprofile.c_str()))) {
            pParams->codecParam[pParams->codec].nProfile = (int16_t)value;
        } else {
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), _T("profile"), argsData.cachedprofile.c_str());
            return -1;
        }
    }
    if (argsData.cachedtier.length() > 0) {
        const auto desc = get_tier_list(pParams->codec);
        int value = 0;
        if (desc != nullptr && PARSE_ERROR_FLAG != (value = get_value_from_chr(desc, argsData.cachedtier.c_str()))) {
            pParams->codecParam[pParams->codec].nTier = (int16_t)value;
        } else {
            CMD_PARSE_SET_ERR(strInput[0], _T("Unknown value"), _T("tier"), argsData.cachedtier.c_str());
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

    cmd << gen_cmd(&pParams->input, &encPrmDefault.input, save_disabled_prm);

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
    if (pParams->input.sar[0] > 0 && pParams->input.sar[1] > 0) {
        cmd << _T(" --sar ") << pParams->input.sar[0] << _T(":") << pParams->input.sar[1];
    } else if (pParams->input.sar[0] < 0 && pParams->input.sar[1] < 0) {
        cmd << _T(" --dar ") << -1 * pParams->input.sar[0] << _T(":") << -1 * pParams->input.sar[1];
    }
    OPT_BOOL(_T("--filler"), _T(""), bFiller);
    OPT_BOOL(_T("--enforce-hrd"), _T(""), bEnforceHRD);
    OPT_BOOL(_T("--fullrange"), _T(""), vui.fullrange);
    OPT_LST(_T("--videoformat"), vui.format, list_videoformat);
    OPT_LST(_T("--colormatrix"), vui.matrix, list_colormatrix);
    OPT_LST(_T("--colorprim"), vui.colorprim, list_colorprim);
    OPT_LST(_T("--chromaloc"), vui.chromaloc, list_chromaloc);
    OPT_LST(_T("--transfer"), vui.transfer, list_transfer);

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

    cmd << gen_cmd(&pParams->common, &encPrmDefault.common, save_disabled_prm);

    std::basic_stringstream<TCHAR> tmp;

    OPT_LST(_T("--vpp-resize"), vpp.resize, list_vpp_resize);

    cmd << gen_cmd(&pParams->ctrl, &encPrmDefault.ctrl, save_disabled_prm);

    return cmd.str();
}
#pragma warning (pop)

#undef CMD_PARSE_SET_ERR