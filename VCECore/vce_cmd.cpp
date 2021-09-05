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

#include <set>
#include <sstream>
#include <iomanip>
#include <numeric>
#include "rgy_osdep.h"
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
    tstring version = get_encoder_version() + tstring(_T("\n"));
    version += strsprintf(_T("AMF Version %d.%d.%d\n"), AMF_VERSION_MAJOR, AMF_VERSION_MINOR, AMF_VERSION_RELEASE);
    version += _T("reader: raw");
    if (ENABLE_AVISYNTH_READER)    version += _T(", avs");
    if (ENABLE_VAPOURSYNTH_READER) version += _T(", vpy");
#if ENABLE_AVSW_READER
    version += strsprintf(_T(", avhw [%s]"), getHWDecSupportedCodecList().c_str());
#endif //#if ENABLE_AVSW_READER
    return version;
}

typedef struct ListData {
    const TCHAR *name;
    const CX_DESC *list;
    int default_index;
} ListData;

static tstring PrintMultipleListOptions(const TCHAR *option_name, const TCHAR *option_desc, const vector<ListData>& listDatas) {
    tstring str;
    const TCHAR *indent_space = _T("                                ");
    const int indent_len = (int)_tcslen(indent_space);
    const int max_len = 79;
    str += strsprintf(_T("   %s "), option_name);
    while ((int)str.length() < indent_len) {
        str += _T(" ");
    }
    str += strsprintf(_T("%s\n"), option_desc);
    const auto data_name_max_len = indent_len + 4 + std::accumulate(listDatas.begin(), listDatas.end(), 0,
        [](const int max_len, const ListData data) { return (std::max)(max_len, (int)_tcslen(data.name)); });

    for (const auto& data : listDatas) {
        tstring line = strsprintf(_T("%s- %s: "), indent_space, data.name);
        while ((int)line.length() < data_name_max_len) {
            line += strsprintf(_T(" "));
        }
        for (int i = 0; data.list[i].desc; i++) {
            if (i > 0 && data.list[i].value == data.list[i - 1].value) {
                continue; //連続で同じ値を示す文字列があるときは、先頭のみ表示する
            }
            const int desc_len = (int)(_tcslen(data.list[i].desc) + _tcslen(_T(", ")) + ((i == data.default_index) ? _tcslen(_T("(default)")) : 0));
            if (line.length() + desc_len >= max_len) {
                str += line + _T("\n");
                line = indent_space;
                while ((int)line.length() < data_name_max_len) {
                    line += strsprintf(_T(" "));
                }
            } else {
                if (i) {
                    line += strsprintf(_T(", "));
                }
            }
            line += strsprintf(_T("%s%s"), data.list[i].desc, (i == data.default_index) ? _T("(default)") : _T(""));
        }
        str += line + _T("\n");
    }
    return str;
}

tstring encoder_help() {
    tstring str;
    str += _T("Usage: VCEEncC [Options] -i <filename> -o <filename>\n");
    str += strsprintf(_T("\n")
#if ENABLE_AVSW_READER
        _T("When video codec could be decoded by VCE, any format or protocol supported\n")
        _T("by ffmpeg library could be used as a input.\n")
#endif
        _T("%s input can be %s%s%sraw YUV or YUV4MPEG2(y4m) format.\n")
        _T("when raw, fps, input-res are also necessary.\n")
        _T("\n")
        _T("output format will be automatically set by the output extension.\n")
        _T("when output filename is set to \"-\", ES output is thrown to stdout.\n")
        _T("\n")
        _T("Example:\n")
        _T("  VCEEncC -i \"<infilename>\" -o \"<outfilename>\"\n")
        _T("  avs2pipemod -y4mp \"<avsfile>\" | VCEEncC --y4m -i - -o \"<outfilename>\"\n")
        ,
        (ENABLE_AVSW_READER) ? _T("Also, ") : _T(""),
        (ENABLE_AVI_READER)         ? _T("avi, ") : _T(""),
        (ENABLE_AVISYNTH_READER)    ? _T("avs, ") : _T(""),
        (ENABLE_VAPOURSYNTH_READER) ? _T("vpy, ") : _T(""));
    str += strsprintf(_T("\n")
        _T("Information Options: \n")
        _T("-h,-? --help                    show help\n")
        _T("-v,--version                    show version info\n")
        _T("\n")
        _T("   --check-hw [<int>]           check for vce support for default device.\n")
        _T("                                 as an option, you can specify device id to check.\n")
        _T("   --check-features [<int>]     check features of vce support for default device.\n")
        _T("                                 as an option, you can specify device id to check.\n")
#if ENABLE_AVSW_READER
        _T("   --check-avversion            show dll version\n")
        _T("   --check-codecs               show codecs available\n")
        _T("   --check-encoders             show audio encoders available\n")
        _T("   --check-decoders             show audio decoders available\n")
        _T("   --check-profiles <string>    show profile names available for specified codec\n")
        _T("   --check-formats              show in/out formats available\n")
        _T("   --check-protocols            show in/out protocols available\n")
        _T("   --check-filters              show filters available\n")
        _T("   --option-list                show option list\n")
#endif
        _T("\n"));
    str += strsprintf(_T("\n")
        _T("Basic Encoding Options: \n")
        _T("-d,--device <int>               set device id to use, default = 0\n"));
    str += gen_cmd_help_input();
    str += strsprintf(_T("\n")
        _T("-c,--codec <string>             set encode codec\n")
        _T("                                 - h264(default), hevc\n")
        _T("-u,--preset <string>            set quality preset\n")
        _T("                                 balanced(default), fast, slow\n")
        _T("   --cqp <int> or               encode in Constant QP, default %d:%d:%d\n")
        _T("         <int>:<int>:<int>      set qp value for i:p:b frame\n")
        _T("   --cbr <int>                  set bitrate in CBR mode (kbps)\n")
        _T("   --vbr <int>                  set bitrate in VBR mode (kbps)\n")
        _T("   --qvbr <int>                 set bitrate in QVBR mode (kbps)\n")
        _T("   --output-depth <int>         set output bit depth\n")
        _T("   --qp-max <int>               set max qp\n")
        _T("   --qp-min <int>               set min qp\n")
        _T("-b,--bframes <int>              set consecutive b frames (default: %d)\n")
        _T("   --(no-)b-pyramid             enable b-pyramid feature\n")
        _T("   --b-deltaqp <int>            set qp offset for non-ref b frames\n")
        _T("   --bref-deltaqp <int>         set qp offset for ref b frames\n")
        _T("   --ref <int>                  set num of reference frames (default: %d)\n")
        _T("   --ltr <int>                  set num of long term reference frames (default: %d)\n")
        _T("   --max-bitrate <int>          set max bitrate (kbps) (default: %d)\n")
        _T("   --vbv-bufsize <int>          set vbv buffer size (kbps) (default: %d)\n")
        _T("   --slices <int>               set num of slices per frame (default: %d)\n")
        _T("   --(no-)skip-frame            enable skip frame feature\n")
        _T("   --motion-est                 set motion estimation precision\n")
        _T("                                 - full-pel (fast)\n")
        _T("                                 - half-pel\n")
        _T("                                 - q-pel (best) = default\n")
        _T("   --gop-len <int>              set length of gop (default: auto)\n")
        _T("   --vbaq                       enable VBAQ\n")
        _T("   --pe                         enable Pre Encode\n")
        _T("   --pa                         enable Pre Analysis\n")
        _T("   --pa-sc <string>             sensitivity of scenechange detection\n")
        _T("                                 - none, low, medium(default), high\n")
        _T("   --pa-ss <string>             sensitivity of static scene detection\n")
        _T("                                 - none, low, medium(default), high\n")
        _T("   --pa-activity-type <string>  block activity calcualtion mode\n")
        _T("                                 - y (default, yuv)\n")
        _T("   --pa-caq-strength <string>   Content Adaptive Quantization (CAQ) strength\n")
        _T("                                 - low, medium(default), high\n")
        _T("   --pa-initqpsc <int>          initial qp after scene change\n")
        _T("   --pa-fskip-maxqp <int>       threshold to insert skip frame on static scene\n"),
        VCE_DEFAULT_QPI, VCE_DEFAULT_QPP, VCE_DEFAULT_QPB, VCE_DEFAULT_BFRAMES,
        VCE_DEFAULT_REF_FRAMES, VCE_DEFAULT_LTR_FRAMES,
        VCE_DEFAULT_MAX_BITRATE, VCE_DEFAULT_VBV_BUFSIZE, VCE_DEFAULT_SLICES
    );

    str += PrintMultipleListOptions(_T("--level <string>"), _T("set codec level"),
        { { _T("H.264"), list_avc_level,   0 },
          { _T("HEVC"),  list_hevc_level,  0 },
        });
    str += PrintMultipleListOptions(_T("--profile <string>"), _T("set codec profile"),
        { { _T("H.264"), list_avc_profile,   0 },
          { _T("HEVC"),  list_hevc_profile,  0 },
        });
    str += PrintMultipleListOptions(_T("--tier <string>"), _T("set codec tier"),
        { { _T("HEVC"),  list_hevc_tier, 0 },
        });

    str += strsprintf(_T("\n")
        _T("   --sar <int>:<int>            set Sample Aspect Ratio\n")
        _T("   --dar <int>:<int>            set Display Aspect Ratio\n")
        _T("\n")
        _T("   --crop <int>,<int>,<int>,<int>\n")
        _T("                                set crop pixels of left, up, right, bottom.\n")
        _T("\n")
        _T("   --enforce-hrd                enforce hrd compatibility of bitstream\n")
        _T("   --filler                     use filler data\n")
    );
    str += _T("\n");
    str += gen_cmd_help_common();
    str += _T("\n");
    str += gen_cmd_help_vpp();
    str += _T("\n");
    str += gen_cmd_help_ctrl();
    return str;
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
        option_name = _T("preset");
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

int parse_one_option(const TCHAR *option_name, const TCHAR* strInput[], int& i, int nArgNum, VCEParam *pParams, sArgsData *argData) {

    if (IS_OPTION("device")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        }
        pParams->deviceID = value;
        return 0;
    }
    if (IS_OPTION("interop-d3d9")) {
        pParams->interopD3d9 = true;
        return 0;
    }
    if (IS_OPTION("no-interop-d3d9")) {
        pParams->interopD3d9 = false;
        return 0;
    }
    if (IS_OPTION("interop-d3d11")) {
        pParams->interopD3d11 = true;
        return 0;
    }
    if (IS_OPTION("no-interop-d3d11")) {
        pParams->interopD3d11 = false;
        return 0;
    }
    if (IS_OPTION("preset") || IS_OPTION("quality")) {
        i++;
        int value = AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_vce_quality_preset_h264, strInput[i]))) {
            print_cmd_error_invalid_value(option_name, strInput[i], list_vce_quality_preset_h264);
            return 1;
        }
        pParams->qualityPreset = value;
        return 0;
    }
    if (IS_OPTION("codec")) {
        i++;
        int value = 0;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_codec_all, strInput[i]))) {
            print_cmd_error_invalid_value(option_name, strInput[i], list_codec_all);
            return 1;
        }
        pParams->codec = (RGY_CODEC)value;
        return 0;
    }
    if (IS_OPTION("output-depth")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        } else if (value < 0) {
            print_cmd_error_invalid_value(option_name, strInput[i], _T("output-depth should be positive value."));
            return 1;
        }
        pParams->outputDepth = value;
        return 0;
    }
    if (IS_OPTION("level")) {
        if (i+1 < nArgNum) {
            i++;
            argData->cachedlevel = strInput[i];
        } else {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        }
        return 0;
    }
    if (IS_OPTION("profile")) {
        if (i+1 < nArgNum) {
            i++;
            argData->cachedprofile = strInput[i];
        } else {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        }
        return 0;
    }
    if (IS_OPTION("tier")) {
        if (i+1 < nArgNum) {
            i++;
            argData->cachedtier = strInput[i];
        } else {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
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
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
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
        int qp[3];
        int ret = parse_qp(qp, strInput[i]);
        if (ret == 0) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        }
        pParams->rateControl = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP;
        pParams->nQPI = qp[0];
        pParams->nQPP = (ret > 1) ? qp[1] : qp[ret - 1];
        pParams->nQPB = (ret > 2) ? qp[2] : qp[ret - 1];
        return 0;
    }
    if (IS_OPTION("vbr")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        } else if (value < 0) {
            print_cmd_error_invalid_value(option_name, strInput[i], _T("bitrate should be positive value."));
            return 1;
        }
        pParams->rateControl = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
        pParams->nBitrate = value;
        return 0;
    }
    if (IS_OPTION("cbr")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        } else if (value < 0) {
            print_cmd_error_invalid_value(option_name, strInput[i], _T("bitrate should be positive value."));
            return 1;
        }
        pParams->rateControl = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR;
        pParams->nBitrate = value;
        return 0;
    }
    if (IS_OPTION("qvbr")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        } else if (value < 0) {
            print_cmd_error_invalid_value(option_name, strInput[i], _T("bitrate should be positive value."));
            return 1;
        }
        pParams->rateControl = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_QUALITY_VBR;
        pParams->nBitrate = value;
        return 0;
    }
    if (IS_OPTION("qp-max")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        } else if (value < 0) {
            print_cmd_error_invalid_value(option_name, strInput[i], _T("qp-max should be positive value."));
            return 1;
        }
        pParams->nQPMax = value;
        return 0;
    }
    if (IS_OPTION("qp-min")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        } else if (value < 0) {
            print_cmd_error_invalid_value(option_name, strInput[i], _T("qp-min should be positive value."));
            return 1;
        }
        pParams->nQPMin = value;
        return 0;
    }
    if (IS_OPTION("bframes")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        } else if (value < 0) {
            print_cmd_error_invalid_value(option_name, strInput[i], _T("bframes should be positive value."));
            return 1;
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
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        }
        pParams->nDeltaQPBFrame = value;
        return 0;
    }
    if (IS_OPTION("bref-deltaqp")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        }
        pParams->nDeltaQPBFrameRef = value;
        return 0;
    }
    if (IS_OPTION("ref")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        }
        pParams->nRefFrames = value;
        return 0;
    }
    if (IS_OPTION("ltr")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        }
        pParams->nLTRFrames = value;
        return 0;
    }
    if (IS_OPTION("max-bitrate")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        } else if (value < 0) {
            print_cmd_error_invalid_value(option_name, strInput[i], _T("max-bitrate should be positive value."));
            return 1;
        }
        pParams->nMaxBitrate = value;
        return 0;
    }
    if (IS_OPTION("vbv-bufsize")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        } else if (value < 0) {
            print_cmd_error_invalid_value(option_name, strInput[i], _T("vbv-bufsize should be positive value."));
            return 1;
        }
        pParams->nVBVBufferSize = value;
        return 0;
    }
    if (IS_OPTION("slices")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        } else if (value < 0) {
            print_cmd_error_invalid_value(option_name, strInput[i], _T("slices should be positive value."));
            return 1;
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
            print_cmd_error_invalid_value(option_name, strInput[i], list_mv_presicion);
            return 1;
        }
        pParams->nMotionEst = value;
        return 0;
    }
    if (IS_OPTION("vbaq")) {
        pParams->bVBAQ = true;
        return 0;
    }
    if (IS_OPTION("gop-len")) {
        i++;
        int value = 0;
        if (_tcscmp(strInput[i], _T("auto")) == 0) {
            value = 0;
        } else if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        } else if (value < 0) {
            print_cmd_error_invalid_value(option_name, strInput[i], _T("gop-len should be positive value."));
            return 1;
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
            print_cmd_error_invalid_value(option_name, strInput[i], list_interlaced);
            return 1;
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
    if (IS_OPTION("no-pe")) {
        pParams->pe = false;
        return 0;
    }
    if (IS_OPTION("pe")) {
        pParams->pe = true;
        return 0;
    }
    if (IS_OPTION("no-pa")
        || IS_OPTION("no-pre-analysis")) {
        pParams->pa.enable = false;
        return 0;
    }
    if (IS_OPTION("pa")
        || IS_OPTION("pre-analysis")) {
        pParams->pa.enable = true;
        return 0;
    }
    if (IS_OPTION("pa-sc")) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_pa_sc_sensitivity, strInput[i]))) {
            print_cmd_error_invalid_value(option_name, strInput[i], list_pa_sc_sensitivity);
            return 1;
        }
        if (value == AMF_PA_STATIC_SCENE_DETECTION_NONE) {
            pParams->pa.sc = false;
        } else {
            pParams->pa.sc = true;
            pParams->pa.scSensitivity = (AMF_PA_SCENE_CHANGE_DETECTION_SENSITIVITY_ENUM)value;
        }
        return 0;
    }
    if (IS_OPTION("pa-ss")) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_pa_ss_sensitivity, strInput[i]))) {
            print_cmd_error_invalid_value(option_name, strInput[i], list_pa_ss_sensitivity);
            return 1;
        }
        if (value == AMF_PA_STATIC_SCENE_DETECTION_NONE) {
            pParams->pa.ss = false;
        } else {
            pParams->pa.ss = true;
            pParams->pa.ssSensitivity = (AMF_PA_STATIC_SCENE_DETECTION_SENSITIVITY_ENUM)value;
        }
        return 0;
    }
    if (IS_OPTION("pa-activity-type")) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_pa_activity, strInput[i]))) {
            print_cmd_error_invalid_value(option_name, strInput[i], list_pa_activity);
            return 1;
        }
        pParams->pa.activityType = (AMF_PA_ACTIVITY_TYPE_ENUM)value;
        return 0;
    }
    if (IS_OPTION("pa-caq-strength")) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_pa_caq_strength, strInput[i]))) {
            print_cmd_error_invalid_value(option_name, strInput[i], list_pa_caq_strength);
            return 1;
        }
        pParams->pa.CAQStrength = (AMF_PA_CAQ_STRENGTH_ENUM)value;
        return 0;
    }
    if (IS_OPTION("pa-initqpsc")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        } else if (value < 0) {
            print_cmd_error_invalid_value(option_name, strInput[i], _T("pa-initqpsc should be positive value."));
            return 1;
        }
        pParams->pa.initQPSC = value;
        return 0;
    }
    if (IS_OPTION("pa-fskip-maxqp")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            print_cmd_error_invalid_value(option_name, strInput[i]);
            return 1;
        } else if (value < 0) {
            print_cmd_error_invalid_value(option_name, strInput[i], _T("pa-fskip-maxqp should be positive value."));
            return 1;
        }
        pParams->pa.maxQPBeforeForceSkip = value;
        return 0;
    }

    auto ret = parse_one_input_option(option_name, strInput, i, nArgNum, &pParams->input, argData);
    if (ret >= 0) return ret;

    ret = parse_one_common_option(option_name, strInput, i, nArgNum, &pParams->common, argData);
    if (ret >= 0) return ret;

    ret = parse_one_ctrl_option(option_name, strInput, i, nArgNum, &pParams->ctrl, argData);
    if (ret >= 0) return ret;

    ret = parse_one_vpp_option(option_name, strInput, i, nArgNum, &pParams->vpp, argData);
    if (ret >= 0) return ret;

    print_cmd_error_unknown_opt(strInput[i]);
    return 1;
}
#undef IS_OPTION

int parse_cmd(VCEParam *pParams, int nArgNum, const TCHAR **strInput, bool ignore_parse_err) {
    sArgsData argsData;

    bool debug_cmd_parser = false;
    for (int i = 1; i < nArgNum; i++) {
        if (tstring(strInput[i]) == _T("--debug-cmd-parser")) {
            debug_cmd_parser = true;
            break;
        }
    }

    if (debug_cmd_parser) {
        for (int i = 1; i < nArgNum; i++) {
            _ftprintf(stderr, _T("arg[%3d]: %s\n"), i, strInput[i]);
        }
    }

    for (int i = 1; i < nArgNum; i++) {
        if (strInput[i] == nullptr) {
            return 1;
        }
        const TCHAR *option_name = nullptr;
        if (strInput[i][0] == _T('-')) {
            if (strInput[i][1] == _T('-')) {
                option_name = &strInput[i][2];
            } else if (strInput[i][2] == _T('\0')) {
                if (nullptr == (option_name = cmd_short_opt_to_long(strInput[i][1]))) {
                    print_cmd_error_invalid_value(tstring(), tstring(), strsprintf(_T("Unknown option: \"%s\""), strInput[i]));
                    return 1;
                }
            } else {
                if (ignore_parse_err) continue;
                print_cmd_error_invalid_value(tstring(), tstring(), strsprintf(_T("Invalid option: \"%s\""), strInput[i]));
                return 1;
            }
        }

        if (option_name == nullptr) {
            if (ignore_parse_err) continue;
            print_cmd_error_unknown_opt(strInput[i]);
            return 1;
        }
        if (debug_cmd_parser) {
            _ftprintf(stderr, _T("parsing %3d: %s\n"), i, strInput[i]);
        }
        auto sts = parse_one_option(option_name, strInput, i, nArgNum, pParams, &argsData);
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
                    for (int i = 0; desc[i].desc; i++) {
                        try {
                            const int target_val = (int)(std::stod(desc[i].desc) * 1000 + 0.5);
                            if (   (int)(val_float * 1000 + 0.5) == target_val
                                || (int)(val_float * 100 + 0.5) == target_val) {
                                pParams->codecParam[pParams->codec].nLevel = desc[i].value;
                                bParsed = true;
                                break;
                            }
                        } catch (...) {
                            continue;
                        }
                    }
                }
            }
        }
        if (!bParsed) {
            print_cmd_error_invalid_value(_T("level"), argsData.cachedlevel.c_str(), std::vector<std::pair<RGY_CODEC, const CX_DESC *>>{
                { RGY_CODEC_H264, list_avc_level },
                { RGY_CODEC_HEVC, list_hevc_level }
            });
            return 1;
        }
    }
    if (argsData.cachedprofile.length() > 0) {
        const auto desc = get_profile_list(pParams->codec);
        int value = 0;
        if (desc != nullptr && PARSE_ERROR_FLAG != (value = get_value_from_chr(desc, argsData.cachedprofile.c_str()))) {
            pParams->codecParam[pParams->codec].nProfile = value;
        } else {
            print_cmd_error_invalid_value(_T("profile"), argsData.cachedprofile.c_str(), std::vector<std::pair<RGY_CODEC, const CX_DESC *>>{
                { RGY_CODEC_H264, list_avc_profile },
                { RGY_CODEC_HEVC, list_hevc_profile }
            });
            return 1;
        }
    }
    if (argsData.cachedtier.length() > 0) {
        const auto desc = get_tier_list(pParams->codec);
        int value = 0;
        if (desc != nullptr && PARSE_ERROR_FLAG != (value = get_value_from_chr(desc, argsData.cachedtier.c_str()))) {
            pParams->codecParam[pParams->codec].nTier = value;
        } else {
            print_cmd_error_invalid_value(_T("tier"), argsData.cachedtier.c_str(), std::vector<std::pair<RGY_CODEC, const CX_DESC *>>{
                { RGY_CODEC_HEVC, list_hevc_tier }
            });
            return 1;
        }
    }
    if (pParams->codec != RGY_CODEC_H264) {
        if (pParams->codec == RGY_CODEC_HEVC) {
            int h264RateControl = pParams->rateControl;
            switch (h264RateControl) {
            case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR:
                pParams->rateControl = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CBR;
                break;
            case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR:
                pParams->rateControl = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
                break;
            case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_QUALITY_VBR:
                fprintf(stderr, "QVBR unsupported for HEVC encoding!\n");
                abort();
            case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP:
            default:
                pParams->rateControl = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CONSTANT_QP;
                break;
            }
            int h264qualityPreset = pParams->qualityPreset;
            switch (h264qualityPreset) {
            case AMF_VIDEO_ENCODER_QUALITY_PRESET_SPEED:
                pParams->qualityPreset = AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_SPEED;
                break;
            case AMF_VIDEO_ENCODER_QUALITY_PRESET_QUALITY:
                pParams->qualityPreset = AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_QUALITY;
                break;
            case AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED:
            default:
                pParams->qualityPreset = AMF_VIDEO_ENCODER_HEVC_QUALITY_PRESET_BALANCED;
                break;
            }
        } else {
            fprintf(stderr, "Unsupported codec!\n");
            abort();
        }
    }

    return 0;
}

#if defined(_WIN32) || defined(_WIN64)
int parse_cmd(VCEParam *pParams, const char *cmda, bool ignore_parse_err) {
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
    int ret = parse_cmd(pParams, argc, strInput, ignore_parse_err);
    return ret;
}
#endif //#if defined(_WIN32) || defined(_WIN64)

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
        if (pParams->rateControl == get_codec_cqp(pParams->codec)) {
            cmd << _T(" --vbr ") << pParams->nBitrate;
        } else {
            OPT_QP(_T("--cqp"), nQPI, nQPP, nQPB, true, true);
        }
    }
    cmd << _T(" --preset ") << get_chr_from_value(get_quality_preset(pParams->codec), (pParams->qualityPreset));
    if (pParams->rateControl == get_codec_cbr(pParams->codec)) {
        cmd << _T(" --cbr ") << pParams->nBitrate;
    } else if (pParams->rateControl == get_codec_vbr(pParams->codec)
        || pParams->rateControl == get_codec_vbr_lat(pParams->codec)) {
        cmd << _T(" --vbr ") << pParams->nBitrate;
    } else if (pParams->rateControl == get_codec_cqp(pParams->codec)) {
        OPT_QP(_T("--cqp"), nQPI, nQPP, nQPB, true, true);
    } else if (pParams->rateControl == get_codec_qvbr(pParams->codec)) {
        cmd << _T(" --qvbr ") << pParams->nBitrate;
    }
    OPT_NUM(_T("--output-depth"), outputDepth);
    if (pParams->rateControl != get_codec_cqp(pParams->codec) || save_disabled_prm) {
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

    if (pParams->codec == RGY_CODEC_HEVC || save_disabled_prm) {
        OPT_LST_HEVC(_T("--level"), _T(""), nLevel, list_hevc_level);
        OPT_LST_HEVC(_T("--profile"), _T(""), nProfile, list_hevc_profile);
        OPT_LST_HEVC(_T("--tier"), _T(""), nTier, list_hevc_tier);
    }
    if (pParams->codec == RGY_CODEC_H264 || save_disabled_prm) {
        OPT_LST_H264(_T("--level"), _T(""), nLevel, list_avc_level);
        OPT_LST_H264(_T("--profile"), _T(""), nProfile, list_avc_profile);
    }

    OPT_BOOL(_T("--pe"), _T("--no-pe"), pe);
    OPT_BOOL(_T("--pa"), _T("--no-pa"), pa.enable);
    if (pParams->pa.sc) {
        OPT_LST(_T("pa-sc"), pa.scSensitivity, list_pa_sc_sensitivity);
    } else {
        cmd << _T("--pa-sc ") << get_chr_from_value(list_pa_sc_sensitivity, AMF_PA_SCENE_CHANGE_DETECTION_NONE);
    }
    if (pParams->pa.ss) {
        OPT_LST(_T("pa-ss"), pa.ssSensitivity, list_pa_ss_sensitivity);
    } else {
        cmd << _T("--pa-ss ") << get_chr_from_value(list_pa_ss_sensitivity, AMF_PA_STATIC_SCENE_DETECTION_NONE);
    }
    OPT_LST(_T("pa-activity-type"), pa.activityType, list_pa_activity);
    OPT_LST(_T("pa-caq-strength"), pa.CAQStrength, list_pa_caq_strength);
    OPT_NUM(_T("pa-initqpsc"), pa.initQPSC);
    OPT_NUM(_T("pa-fskip-maxqp"), pa.maxQPBeforeForceSkip);

    cmd << gen_cmd(&pParams->common, &encPrmDefault.common, save_disabled_prm);

    cmd << gen_cmd(&pParams->ctrl, &encPrmDefault.ctrl, save_disabled_prm);

    cmd << gen_cmd(&pParams->vpp, &encPrmDefault.vpp, save_disabled_prm);

    return cmd.str();
}
#pragma warning (pop)

#undef CMD_PARSE_SET_ERR