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

#include <vector>
#include <set>
#include <numeric>
#include <cstdio>
#include <cassert>
#include <tchar.h>
#include "VCEVersion.h"
#include "VCEParam.h"
#include "VCECore.h"
#include "avcodec_vce.h"

static tstring GetVCEEncVersion() {
    static const TCHAR *const ENABLED_INFO[] = { _T("disabled"), _T("enabled") };
    tstring version = strsprintf(_T("VCEEncC (%s) %s by rigaya, build %s %s\n"), BUILD_ARCH_STR, VER_STR_FILEVERSION_TCHAR, _T(__DATE__), _T(__TIME__));
    if (ENABLE_AVISYNTH_READER)    version += _T(", avs");
    if (ENABLE_VAPOURSYNTH_READER) version += _T(", vpy");
    if (ENABLE_AVCODEC_VCE_READER) version += strsprintf(_T(", avvce [%s]"), getAVVCESupportedCodecList().c_str());
    return version;
}

static void PrintVersion() {
    _ftprintf(stdout, _T("%s"), GetVCEEncVersion().c_str());
}

//適当に改行しながら表示する
static tstring PrintListOptions(const TCHAR *option_name, const CX_DESC *list, int default_index) {
    const TCHAR *indent_space = _T("                                ");
    const int indent_len = (int)_tcslen(indent_space);
    const int max_len = 77;
    tstring str = strsprintf(_T("   %s "), option_name);
    while ((int)str.length() < indent_len)
        str += _T(" ");
    int line_len = (int)str.length();
    for (int i = 0; list[i].desc; i++) {
        if (line_len + _tcslen(list[i].desc) + _tcslen(_T(", ")) >= max_len) {
            str += strsprintf(_T("\n%s"), indent_space);
            line_len = indent_len;
        } else {
            if (i) {
                str += strsprintf(_T(", "));
                line_len += 2;
            }
        }
        str += strsprintf(_T("%s"), list[i].desc);
        line_len += (int)_tcslen(list[i].desc);
    }
    str += strsprintf(_T("\n%s default: %s\n"), indent_space, list[default_index].desc);
    return str;
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

static bool check_if_vce_supported() {
    if (!check_if_vce_available()) {
        _ftprintf(stderr, _T("VCE check failed!\n"));
        return false;
    }
    return true;
}


static const TCHAR *short_opt_to_long(TCHAR short_opt) {
    const TCHAR *option_name = nullptr;
    switch (short_opt) {
    case _T('b'):
        option_name = _T("bframes");
        break;
    case _T('c'):
        option_name = _T("codec");
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

static tstring help() {
    tstring str;
    str += _T("Usage: VCEEncC [Options] -i <filename> -o <filename>\n");
    str += strsprintf(_T("\n")
        _T("input can be %s%sraw YUV or YUV4MPEG2(y4m) format.\n")
        _T("when raw(default), fps, input-res are also necessary.\n")
        _T("\n")
        _T("output format will be automatically set by the output extension.\n")
        _T("\n")
        _T("Example:\n")
        _T("  VCEEncC -i \"<infilename>\" -o \"<outfilename>\"\n")
        _T("  avs2pipemod -y4mp \"<avsfile>\" | VCEEncC --y4m -i - -o \"<outfilename>\"\n")
        _T("\n")
        _T("Options: \n")
        _T("-h,-? --help                    show help\n")
        _T("-v,--version                    show version info\n")
        _T("\n")
        _T("-i,--input-file <filename>      set input file name\n")
        _T("-o,--output-file <filename>     set ouput file name\n")
        _T("\n")
        _T("   --check-vce                  check for vce support on system.\n")
#if ENABLE_AVCODEC_VCE_READER
        _T("   --check-avversion            show dll version\n")
        _T("   --check-codecs               show codecs available\n")
        _T("   --check-encoders             show audio encoders available\n")
        _T("   --check-decoders             show audio decoders available\n")
        _T("   --check-formats              show in/out formats available\n")
        _T("   --check-protocols            show in/out protocols available\n")
        _T("   --check-filters              show filters available\n")
#endif
        _T("\n"),
        (ENABLE_AVISYNTH_READER)    ? _T("avs, ") : _T(""),
        (ENABLE_VAPOURSYNTH_READER) ? _T("vpy, ") : _T(""));
    str += strsprintf(_T("\n")
        _T(" Input formats (will be estimated from extension if not set.)\n")
        _T("   --raw                        set input as raw format\n")
        _T("   --y4m                        set input as y4m format\n")
#if ENABLE_AVISYNTH_READER
        _T("   --avs                        set input as avs format\n")
#endif
#if ENABLE_VAPOURSYNTH_READER
        _T("   --vpy                        set input as vpy format\n")
        _T("   --vpy-mt                     set input as vpy format in multi-thread\n")
#endif
        );
#if ENABLE_AVCODEC_VCE_READER
    str += strsprintf(
        _T("   --avvce                      set input to use avcodec + vce\n")
        _T("   --avvce-analyze <int>        set time (sec) which reader analyze input file.\n")
        _T("                                 default: 5 (seconds).\n")
        _T("                                 could be only used with avvce reader.\n")
        _T("                                 use if reader fails to detect audio stream.\n")
        _T("   --video-track <int>          set video track to encode in track id\n")
        _T("                                 1 (default)  highest resolution video track\n")
        _T("                                 2            next high resolution video track\n")
        _T("                                   ... \n")
        _T("                                 -1           lowest resolution video track\n")
        _T("                                 -2           next low resolution video track\n")
        _T("                                   ... \n")
        _T("   --video-streamid <int>       set video track to encode in stream id\n")
        _T("   --audio-source <string>      input extra audio file\n")
        _T("   --audio-file [<int>?][<string>:]<string>\n")
        _T("                                extract audio into file.\n")
        _T("                                 could be only used with avvce reader.\n")
        _T("                                 below are optional,\n")
        _T("                                  in [<int>?], specify track number to extract.\n")
        _T("                                  in [<string>?], specify output format.\n")
#if 0
        _T("   --trim <int>:<int>[,<int>:<int>]...\n")
        _T("                                trim video for the frame range specified.\n")
        _T("                                 frame range should not overwrap each other.\n")
#endif
        _T("   --seek [<int>:][<int>:]<int>[.<int>] (hh:mm:ss.ms)\n")
        _T("                                skip video for the time specified,\n")
        _T("                                 seek will be inaccurate but fast.\n")
        _T("-f,--format <string>            set output format of output file.\n")
        _T("                                 if format is not specified, output format will\n")
        _T("                                 be guessed from output file extension.\n")
        _T("                                 set \"raw\" for H.264/ES output.\n")
        _T("   --audio-copy [<int>[,...]]   mux audio with video during output.\n")
        _T("                                 could be only used with\n")
        _T("                                 avvce reader and avcodec muxer.\n")
        _T("                                 by default copies all audio tracks.\n")
        _T("                                 \"--audio-copy 1,2\" will extract\n")
        _T("                                 audio track #1 and #2.\n")
        _T("   --audio-codec [<int>?]<string>\n")
        _T("                                encode audio to specified format.\n")
        _T("                                  in [<int>?], specify track number to encode.\n")
        _T("   --audio-bitrate [<int>?]<int>\n")
        _T("                                set encode bitrate for audio (kbps).\n")
        _T("                                  in [<int>?], specify track number of audio.\n")
        _T("   --audio-ignore-decode-error <int>  (default: %d)\n")
        _T("                                set numbers of continuous packets of audio\n")
        _T("                                 decode error to ignore, replaced by silence.\n")
        _T("   --audio-ignore-notrack-error ignore error when audio track is unfound.\n")
        _T("   --audio-samplerate [<int>?]<int>\n")
        _T("                                set sampling rate for audio (Hz).\n")
        _T("                                  in [<int>?], specify track number of audio.\n")
        _T("   --audio-resampler <string>   set audio resampler.\n")
        _T("                                  swr (swresampler: default), soxr (libsoxr)\n")
        _T("   --audio-stream [<int>?][<string1>][:<string2>][,[<string1>][:<string2>]][..\n")
        _T("       set audio streams in channels.\n")
        _T("         in [<int>?], specify track number to split.\n")
        _T("         in <string1>, set input channels to use from source stream.\n")
        _T("           if unset, all input channels will be used.\n")
        _T("         in <string2>, set output channels to mix.\n")
        _T("           if unset, all input channels will be copied without mixing.\n")
        _T("       example1: --audio-stream FL,FR\n")
        _T("         splitting dual mono audio to each stream.\n")
        _T("       example2: --audio-stream :stereo\n")
        _T("         mixing input channels to stereo.\n")
        _T("       example3: --audio-stream 5.1,5.1:stereo\n")
        _T("         keeping 5.1ch audio and also adding downmixed stereo stream.\n")
        _T("       usable simbols\n")
        _T("         mono       = FC\n")
        _T("         stereo     = FL + FR\n")
        _T("         2.1        = FL + FR + LFE\n")
        _T("         3.0        = FL + FR + FC\n")
        _T("         3.0(back)  = FL + FR + BC\n")
        _T("         3.1        = FL + FR + FC + LFE\n")
        _T("         4.0        = FL + FR + FC + BC\n")
        _T("         quad       = FL + FR + BL + BR\n")
        _T("         quad(side) = FL + FR + SL + SR\n")
        _T("         5.0        = FL + FR + FC + SL + SR\n")
        _T("         5.1        = FL + FR + FC + LFE + SL + SR\n")
        _T("         6.0        = FL + FR + FC + BC + SL + SR\n")
        _T("         6.0(front) = FL + FR + FLC + FRC + SL + SR\n")
        _T("         hexagonal  = FL + FR + FC + BL + BR + BC\n")
        _T("         6.1        = FL + FR + FC + LFE + BC + SL + SR\n")
        _T("         6.1(front) = FL + FR + LFE + FLC + FRC + SL + SR\n")
        _T("         7.0        = FL + FR + FC + BL + BR + SL + SR\n")
        _T("         7.0(front) = FL + FR + FC + FLC + FRC + SL + SR\n")
        _T("         7.1        = FL + FR + FC + LFE + BL + BR + SL + SR\n")
        _T("         7.1(wide)  = FL + FR + FC + LFE + FLC + FRC + SL + SR\n")
        _T("   --audio-filter [<int>?]<string>\n")
        _T("                                set audio filter.\n")
        _T("                                  in [<int>?], specify track number of audio.\n")
        _T("   --chapter-copy               copy chapter to output file.\n")
        _T("   --chapter <string>           set chapter from file specified.\n")
        _T("   --sub-copy [<int>[,...]]     copy subtitle to output file.\n")
        _T("                                 these could be only used with\n")
        _T("                                 avvce reader and avcodec muxer.\n")
        _T("                                 below are optional,\n")
        _T("                                  in [<int>?], specify track number to copy.\n")
        _T("\n")
#if 0
        _T("   --avsync <string>            method for AV sync (default: through)\n")
        _T("                                 through  ... assume cfr, no check but fast\n")
        _T("                                 forcecfr ... check timestamp and force cfr.\n")
#endif
        _T("-m,--mux-option <string1>:<string2>\n")
        _T("                                set muxer option name and value.\n")
        _T("                                 these could be only used with\n")
        _T("                                 avvce reader and avcodec muxer.\n"),
        VCE_DEFAULT_AUDIO_IGNORE_DECODE_ERROR);
#endif
    str += strsprintf(_T("\n")
        _T("-c,--codec <string>             set codec: h264(default), hevc\n")
        _T("   --input-res <int>x<int>      set input resolution\n")
        _T("   --output-res <int>x<int>     output resolution\n")
        _T("                                if different from input, uses vpp resizing\n")
        _T("                                if not set, output resolution will be same\n")
        _T("                                as input (no resize will be done).\n")
        _T("   --fps <int>/<int>            set input framerate\n")
        _T("   --crop <int>,<int>,<int>,<int>\n")
        _T("                                set crop pixels of left, up, right, bottom.\n")
        _T("\n")
        _T("-u,--quality <string>           set quality preset\n")
        _T("                                 balanced(default), fast, slow\n")
        _T("   --cqp <int> or               encode in Constant QP, default %d:%d:%d\n")
        _T("         <int>:<int>:<int>      set qp value for i:p:b frame\n")
        _T("   --cbr <int>                  set bitrate in CBR mode (kbps)\n")
        _T("   --vbr <int>                  set bitrate in VBR mode (kbps)\n")
        _T("   --qp-max <int>               set max qp\n")
        _T("   --qp-min <int>               set min qp\n")
        _T("-b,--bframes <int>              set consecutive b frames (default: %d)\n")
        _T("   --(no-)b-pyramid             enable b-pyramid feature\n")
        _T("   --b-deltaqp <int>            set qp offset for non-ref b frames\n")
        _T("   --bref-deltaqp <int>         set qp offset for ref b frames\n")
        _T("   --max-bitrate <int>          set max bitrate (kbps) (default: %d)\n")
        _T("   --vbv-bufsize <int>          set vbv buffer size (kbps) (default: %d)\n")
        _T("   --slices <int>               set number of slices per frame (default: %d)\n")
        _T("   --(no-)skip-frame            enable skip frame feature\n")
        _T("   --motion-est                 set motion estimation precision\n")
        _T("                                 full-pel, half-pel, q-pel(default)\n")
        _T("   --vbaq                       enable VBAQ (H.264 only)\n")
        _T("   --pre-analysis <string>      set pre-analysis mode\n")
        _T("                      H.264: none (default), full (best), half, quater (fast)\n")
        _T("                      HEVC:  none (default), auto\n")
        _T("   --gop-len <int>              set length of gop (default: auto)\n")
        _T("   --tff                        set input as interlaced (tff)\n")
        _T("   --bff                        set input as interlaced (bff)\n"),
        VCE_DEFAULT_QPI, VCE_DEFAULT_QPP, VCE_DEFAULT_QPB, VCE_DEFAULT_BFRAMES,
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

    str += strsprintf(_T("\n")
        _T("   --sar <int>:<int>            set Sample Aspect Ratio\n")
        _T("   --dar <int>:<int>            set Display Aspect Ratio\n")
        _T("\n")
        _T("   --crop <int>,<int>,<int>,<int>\n")
        _T("                                set crop pixels of left, up, right, bottom.\n")
        //_T("\n")
        //_T("   --fullrange                  set stream as fullrange yuv\n")
    );
    //str += PrintListOptions(_T("--videoformat <string>"), list_videoformat, 0);
    //str += PrintListOptions(_T("--colormatrix <string>"), list_colormatrix, 0);
    //str += PrintListOptions(_T("--colorprim <string>"), list_colorprim, 0);
    //str += PrintListOptions(_T("--transfer <string>"), list_transfer, 0);
    str += strsprintf(_T("\n")
        _T("   --log <string>               output log to file (txt or html).\n")
        _T("   --log-level <int>            set log level\n")
        _T("                                 error, warn, info(default), debug\n")
        _T("   --log-framelist <string>     output frame info for avvce reader (for debug)\n")
        );
    return str;
}

static void PrintHelp(const TCHAR *strAppName, const TCHAR *strErrorMessage, const TCHAR *strOptionName, const TCHAR *strErrorValue = nullptr) {
    strAppName = strAppName;
    if (strErrorMessage) {
        if (strOptionName) {
            if (strErrorValue) {
                _ftprintf(stderr, _T("Error: %s \"%s\" for \"--%s\"\n"), strErrorMessage, strErrorValue, strOptionName);
                if (0 == _tcsnccmp(strErrorValue, _T("--"), _tcslen(_T("--")))
                    || (strErrorValue[0] == _T('-') && strErrorValue[2] == _T('\0') && short_opt_to_long(strErrorValue[1]) != nullptr)) {
                    _ftprintf(stderr, _T("       \"--%s\" requires value.\n\n"), strOptionName);
                }
            } else {
                _ftprintf(stderr, _T("Error: %s for --%s\n\n"), strErrorMessage, strOptionName);
            }
        } else {
            _ftprintf(stderr, _T("Error: %s\n\n"), strErrorMessage);
        }
    } else {
        PrintVersion();
        _ftprintf(stdout, _T("%s\n"), help().c_str());
    }
}

struct sArgsData {
    tstring cachedlevel, cachedprofile, cachedPreAnalysis;
    uint32_t nParsedAudioFile = 0;
    uint32_t nParsedAudioEncode = 0;
    uint32_t nParsedAudioCopy = 0;
    uint32_t nParsedAudioBitrate = 0;
    uint32_t nParsedAudioSamplerate = 0;
    uint32_t nParsedAudioSplit = 0;
    uint32_t nParsedAudioFilter = 0;
    uint32_t nTmpInputBuf = 0;
};

int ParseOneOption(const TCHAR *option_name, const TCHAR* strInput[], int& i, int nArgNum, VCEParam *pParams, VCEInputInfo *pInputInfo, sArgsData *argData) {
#define IS_OPTION(x) (0 == _tcsicmp(option_name, _T(x)))
    if (IS_OPTION("input-file")) {
        i++;
        pParams->pInputFile = _tcsdup(strInput[i]);
        return 0;
    }
    if (IS_OPTION("output-file")) {
        i++;
        pParams->pOutputFile = _tcsdup(strInput[i]);
        return 0;
    }
    if (IS_OPTION("raw")) {
        pParams->nInputType = VCE_INPUT_RAW;
        return 0;
    }
    if (IS_OPTION("y4m")) {
        pParams->nInputType = VCE_INPUT_Y4M;
        return 0;
    }
    if (IS_OPTION("avs")) {
        pParams->nInputType = VCE_INPUT_AVS;
        return 0;
    }
    if (IS_OPTION("vpy")) {
        pParams->nInputType = VCE_INPUT_VPY;
        return 0;
    }
    if (IS_OPTION("vpy-mt")) {
        pParams->nInputType = VCE_INPUT_VPY_MT;
        return 0;
    }
    if (IS_OPTION("avvce")) {
        pParams->nInputType = VCE_INPUT_AVCODEC_VCE;
        return 0;
    }
    if (IS_OPTION("avvce-analyze")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            PrintHelp(strInput[0], _T("avvce-analyze requires non-negative value."), option_name);
            return -1;
        } else {
            pParams->nAVDemuxAnalyzeSec = (std::min)(value, USHRT_MAX);
        }
        return 0;
    }
    if (IS_OPTION("quality")) {
        i++;
        int value = AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_vce_quality_preset, strInput[i]))) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nQualityPreset = value;
        return 0;
    }
    if (IS_OPTION("input-res")) {
        i++;
        int width = 0, height = 0;
        if (   2 != _stscanf_s(strInput[i], _T("%dx%d"), &width, &height)
            && 2 != _stscanf_s(strInput[i], _T("%d,%d"), &width, &height)
            && 2 != _stscanf_s(strInput[i], _T("%d/%d"), &width, &height)
            && 2 != _stscanf_s(strInput[i], _T("%d:%d"), &width, &height)) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pInputInfo->srcWidth  = width;
        pInputInfo->srcHeight = height;
        return 0;
    }
    if (0 == _tcscmp(option_name, _T("output-res"))) {
        i++;
        int width = 0, height = 0;
        if (   2 != _stscanf_s(strInput[i], _T("%dx%d"), &width, &height)
            && 2 != _stscanf_s(strInput[i], _T("%d,%d"), &width, &height)
            && 2 != _stscanf_s(strInput[i], _T("%d/%d"), &width, &height)
            && 2 != _stscanf_s(strInput[i], _T("%d:%d"), &width, &height)) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pInputInfo->dstWidth  = width;
        pInputInfo->dstHeight = height;
        return 0;
    }
    if (IS_OPTION("fps")) {
        i++;
        VCERational framerate;
        if (   2 != _stscanf_s(strInput[i], _T("%dx%d"), &framerate.num, &framerate.den)
            && 2 != _stscanf_s(strInput[i], _T("%d,%d"), &framerate.num, &framerate.den)
            && 2 != _stscanf_s(strInput[i], _T("%d/%d"), &framerate.num, &framerate.den)
            && 2 != _stscanf_s(strInput[i], _T("%d:%d"), &framerate.num, &framerate.den)) {
            if (1 != _stscanf_s(strInput[i], _T("%d"), &framerate.num)) {
                PrintHelp(strInput[0], _T("Invalid value"), option_name, strInput[i]);
                return -1;
            }
            framerate.den = 1;
        }
        pInputInfo->fps = framerate;
        return 0;
    }
    if (IS_OPTION("crop")) {
        i++;
        int crop[4] = { 0 };
        if (   4 != _stscanf_s(strInput[i], _T("%d,%d,%d,%d"), &crop[0], &crop[1], &crop[2], &crop[3])
            && 4 != _stscanf_s(strInput[i], _T("%d:%d:%d:%d"), &crop[0], &crop[1], &crop[2], &crop[3])) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        memcpy(pInputInfo->crop.c, crop, sizeof(crop));
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
                PrintHelp(strInput[0], _T("Invalid Value"), option_name);
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
    if (IS_OPTION("seek")) {
        i++;
        int ret = 0;
        int hh = 0, mm = 0;
        float sec = 0.0f;
        if (   3 != (ret = _stscanf_s(strInput[i], _T("%d:%d:%f"),    &hh, &mm, &sec))
            && 2 != (ret = _stscanf_s(strInput[i],    _T("%d:%f"),         &mm, &sec))
            && 1 != (ret = _stscanf_s(strInput[i],       _T("%f"),              &sec))) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        if (ret <= 2) {
            hh = 0;
        }
        if (ret <= 1) {
            mm = 0;
        }
        if (hh < 0 || mm < 0 || sec < 0) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name);
            return -1;
        }
        if (hh > 0 && mm >= 60) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name);
            return -1;
        }
        mm += hh * 60;
        if (mm > 0 && sec >= 60.0f) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name);
            return -1;
        }
        pParams->fSeekSec = sec + mm * 60;
        return 0;
    }
    if (IS_OPTION("video-track")) {
        i++;
        int v = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &v)) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        if (v == 0) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nVideoTrack = (int8_t)v;
        return 0;
    }
    if (IS_OPTION("video-streamid")) {
        i++;
        int v = 0;
        if (1 != _stscanf_s(strInput[i], _T("%i"), &v)) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nVideoStreamId = v;
        return 0;
    }
    if (IS_OPTION("audio-source")) {
        i++;
        size_t audioSourceLen = _tcslen(strInput[i]) + 1;
        TCHAR *pAudioSource = (TCHAR *)malloc(sizeof(strInput[i][0]) * audioSourceLen);
        memcpy(pAudioSource, strInput[i], sizeof(strInput[i][0]) * audioSourceLen);
        pParams->ppAudioSourceList = (TCHAR **)realloc(pParams->ppAudioSourceList, sizeof(pParams->ppAudioSourceList[0]) * (pParams->nAudioSourceCount + 1));
        pParams->ppAudioSourceList[pParams->nAudioSourceCount] = pAudioSource;
        pParams->nAudioSourceCount++;
        return 0;
    }
    if (IS_OPTION("audio-file")) {
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
            PrintHelp(strInput[0], _T("Invalid track number"), option_name);
            return -1;
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
            pAudioSelect->pAudioExtractFormat = alloc_str(ptr, qtr - ptr);
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
                PrintHelp(strInput[0], _T("Same output file name is used more than twice"), option_name);
                return -1;
            }
        }

        if (audioIdx < 0) {
            audioIdx = pParams->nAudioSelectCount;
            //新たに要素を追加
            pParams->ppAudioSelectList = (sAudioSelect **)realloc(pParams->ppAudioSelectList, sizeof(pParams->ppAudioSelectList[0]) * (pParams->nAudioSelectCount + 1));
            pParams->ppAudioSelectList[pParams->nAudioSelectCount] = pAudioSelect;
            pParams->nAudioSelectCount++;
        }
        pParams->ppAudioSelectList[audioIdx]->pAudioExtractFilename = alloc_str(ptr);
        argData->nParsedAudioFile++;
        return 0;
    }
    if (IS_OPTION("format")) {
        if (i+1 < nArgNum && strInput[i+1][0] != _T('-')) {
            i++;
            const int formatLen = (int)_tcslen(strInput[i]);
            pParams->pAVMuxOutputFormat = (TCHAR *)realloc(pParams->pAVMuxOutputFormat, sizeof(pParams->pAVMuxOutputFormat[0]) * (formatLen + 1));
            _tcscpy_s(pParams->pAVMuxOutputFormat, formatLen + 1, strInput[i]);
            if (0 != _tcsicmp(pParams->pAVMuxOutputFormat, _T("raw"))) {
                pParams->nAVMux |= VCEENC_MUX_VIDEO;
            }
        } else {
            PrintHelp(strInput[0], _T("Invalid value"), option_name);
            return -1;
        }
        return 0;
    }
#if ENABLE_AVCODEC_VCE_READER
    if (   IS_OPTION("audio-copy")
        || IS_OPTION("copy-audio")) {
        pParams->nAVMux |= (VCEENC_MUX_VIDEO | VCEENC_MUX_AUDIO);
        std::set<int> trackSet; //重複しないよう、setを使う
        if (i+1 < nArgNum && (strInput[i+1][0] != _T('-') && strInput[i+1][0] != _T('\0'))) {
            i++;
            auto trackListStr = split(strInput[i], _T(","));
            for (auto str : trackListStr) {
                int iTrack = 0;
                if (1 != _stscanf(str.c_str(), _T("%d"), &iTrack) || iTrack < 1) {
                    PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
                    return -1;
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
            pAudioSelect->pAVAudioEncodeCodec = _tcsdup(AVVCE_CODEC_COPY);

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
    if (IS_OPTION("audio-codec")) {
        pParams->nAVMux |= (VCEENC_MUX_VIDEO | VCEENC_MUX_AUDIO);
        if (i+1 < nArgNum) {
            const TCHAR *ptr = nullptr;
            const TCHAR *ptrDelim = nullptr;
            if (strInput[i+1][0] != _T('-') && strInput[i+1][0] != _T('\0')) {
                i++;
                ptrDelim = _tcschr(strInput[i], _T('?'));
                ptr = (ptrDelim == nullptr) ? strInput[i] : ptrDelim+1;
            }
            int trackId = 1;
            if (ptrDelim == nullptr) {
                trackId = argData->nParsedAudioEncode+1;
                int idx = getAudioTrackIdx(pParams, trackId);
                if (idx >= 0 && pParams->ppAudioSelectList[idx]->pAVAudioEncodeCodec != nullptr) {
                    trackId = getFreeAudioTrack(pParams);
                }
            } else {
                tstring temp = tstring(strInput[i]).substr(0, ptrDelim - strInput[i]);
                if (1 != _stscanf(temp.c_str(), _T("%d"), &trackId)) {
                    PrintHelp(strInput[0], _T("Invalid value"), option_name);
                    return -1;
                }
            }
            sAudioSelect *pAudioSelect = nullptr;
            int audioIdx = getAudioTrackIdx(pParams, trackId);
            if (audioIdx < 0) {
                pAudioSelect = (sAudioSelect *)calloc(1, sizeof(pAudioSelect[0]));
                pAudioSelect->nAudioSelect = trackId;
            } else {
                pAudioSelect = pParams->ppAudioSelectList[audioIdx];
            }
            pAudioSelect->pAVAudioEncodeCodec = _tcsdup((ptr) ? ptr : AVVCE_CODEC_COPY);

            if (audioIdx < 0) {
                audioIdx = pParams->nAudioSelectCount;
                //新たに要素を追加
                pParams->ppAudioSelectList = (sAudioSelect **)realloc(pParams->ppAudioSelectList, sizeof(pParams->ppAudioSelectList[0]) * (pParams->nAudioSelectCount + 1));
                pParams->ppAudioSelectList[pParams->nAudioSelectCount] = pAudioSelect;
                pParams->nAudioSelectCount++;
            }
            argData->nParsedAudioEncode++;
        } else {
            PrintHelp(strInput[0], _T("Invalid value"), option_name);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("audio-bitrate")) {
        if (i+1 < nArgNum) {
            i++;
            const TCHAR *ptr = _tcschr(strInput[i], _T('?'));
            int trackId = 1;
            if (ptr == nullptr) {
                trackId = argData->nParsedAudioBitrate+1;
                int idx = getAudioTrackIdx(pParams, trackId);
                if (idx >= 0 && pParams->ppAudioSelectList[idx]->nAVAudioEncodeBitrate > 0) {
                    trackId = getFreeAudioTrack(pParams);
                }
                ptr = strInput[i];
            } else {
                tstring temp = tstring(strInput[i]).substr(0, ptr - strInput[i]);
                if (1 != _stscanf(temp.c_str(), _T("%d"), &trackId)) {
                    PrintHelp(strInput[0], _T("Invalid value"), option_name);
                    return -1;
                }
                ptr++;
            }
            sAudioSelect *pAudioSelect = nullptr;
            int audioIdx = getAudioTrackIdx(pParams, trackId);
            if (audioIdx < 0) {
                pAudioSelect = (sAudioSelect *)calloc(1, sizeof(pAudioSelect[0]));
                pAudioSelect->nAudioSelect = trackId;
            } else {
                pAudioSelect = pParams->ppAudioSelectList[audioIdx];
            }
            int bitrate = 0;
            if (1 != _stscanf(ptr, _T("%d"), &bitrate)) {
                PrintHelp(strInput[0], _T("Invalid value"), option_name);
                return -1;
            }
            pAudioSelect->nAVAudioEncodeBitrate = bitrate;

            if (audioIdx < 0) {
                audioIdx = pParams->nAudioSelectCount;
                //新たに要素を追加
                pParams->ppAudioSelectList = (sAudioSelect **)realloc(pParams->ppAudioSelectList, sizeof(pParams->ppAudioSelectList[0]) * (pParams->nAudioSelectCount + 1));
                pParams->ppAudioSelectList[pParams->nAudioSelectCount] = pAudioSelect;
                pParams->nAudioSelectCount++;
            }
            argData->nParsedAudioBitrate++;
        } else {
            PrintHelp(strInput[0], _T("Invalid value"), option_name);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("audio-ignore-decode-error")) {
        i++;
        uint32_t value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nAudioIgnoreDecodeError = value;
        return 0;
    }
    if (IS_OPTION("audio-ignore-notrack-error")) {
        pParams->bAudioIgnoreNoTrackError = 1;
        return 0;
    }
    if (IS_OPTION("audio-samplerate")) {
        if (i+1 < nArgNum) {
            i++;
            const TCHAR *ptr = _tcschr(strInput[i], _T('?'));
            int trackId = 1;
            if (ptr == nullptr) {
                trackId = argData->nParsedAudioSamplerate+1;
                int idx = getAudioTrackIdx(pParams, trackId);
                if (idx >= 0 && pParams->ppAudioSelectList[idx]->nAudioSamplingRate > 0) {
                    trackId = getFreeAudioTrack(pParams);
                }
                ptr = strInput[i];
            } else {
                tstring temp = tstring(strInput[i]).substr(0, ptr - strInput[i]);
                if (1 != _stscanf(temp.c_str(), _T("%d"), &trackId)) {
                    PrintHelp(strInput[0], _T("Invalid value"), option_name);
                    return -1;
                }
                ptr++;
            }
            sAudioSelect *pAudioSelect = nullptr;
            int audioIdx = getAudioTrackIdx(pParams, trackId);
            if (audioIdx < 0) {
                pAudioSelect = (sAudioSelect *)calloc(1, sizeof(pAudioSelect[0]));
                pAudioSelect->nAudioSelect = trackId;
            } else {
                pAudioSelect = pParams->ppAudioSelectList[audioIdx];
            }
            int bitrate = 0;
            if (1 != _stscanf(ptr, _T("%d"), &bitrate)) {
                PrintHelp(strInput[0], _T("Invalid value"), option_name);
                return -1;
            }
            pAudioSelect->nAudioSamplingRate = bitrate;

            if (audioIdx < 0) {
                audioIdx = pParams->nAudioSelectCount;
                //新たに要素を追加
                pParams->ppAudioSelectList = (sAudioSelect **)realloc(pParams->ppAudioSelectList, sizeof(pParams->ppAudioSelectList[0]) * (pParams->nAudioSelectCount + 1));
                pParams->ppAudioSelectList[pParams->nAudioSelectCount] = pAudioSelect;
                pParams->nAudioSelectCount++;
            }
            argData->nParsedAudioSamplerate++;
        } else {
            PrintHelp(strInput[0], _T("Invalid value"), option_name);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("audio-resampler")) {
        i++;
        int v;
        if (PARSE_ERROR_FLAG != (v = get_value_from_chr(list_resampler, strInput[i]))) {
            pParams->nAudioResampler = v;
        } else if (1 == _stscanf_s(strInput[i], _T("%d"), &v) && 0 <= v && v < _countof(list_resampler) - 1) {
            pParams->nAudioResampler = v;
        } else {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("audio-stream")) {
        if (!check_avcodec_dll()) {
            _ftprintf(stderr, _T("%s\n--audio-stream could not be used.\n"), error_mes_avcodec_dll_not_found().c_str());
            return -1;
        }
        int trackId = -1;
        const TCHAR *ptr = nullptr;
        if (i+1 < nArgNum && strInput[i+1][0] != _T('-')) {
            i++;
            ptr = _tcschr(strInput[i], _T('?'));
            if (ptr != nullptr) {
                tstring temp = tstring(strInput[i]).substr(0, ptr - strInput[i]);
                if (1 != _stscanf(temp.c_str(), _T("%d"), &trackId)) {
                    PrintHelp(strInput[0], _T("Invalid value"), option_name);
                    return -1;
                }
                ptr++;
            } else {
                ptr = strInput[i];
            }
        }
        if (trackId < 0) {
            trackId = argData->nParsedAudioSplit+1;
            int idx = getAudioTrackIdx(pParams, trackId);
            if (idx >= 0 && bSplitChannelsEnabled(pParams->ppAudioSelectList[idx]->pnStreamChannelSelect)) {
                trackId = getFreeAudioTrack(pParams);
            }
        }
        sAudioSelect *pAudioSelect = nullptr;
        int audioIdx = getAudioTrackIdx(pParams, trackId);
        if (audioIdx < 0) {
            pAudioSelect = (sAudioSelect *)calloc(1, sizeof(pAudioSelect[0]));
            pAudioSelect->nAudioSelect = trackId;
        } else {
            pAudioSelect = pParams->ppAudioSelectList[audioIdx];
        }
        if (ptr == nullptr) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name);
            return -1;
        } else {
            auto streamSelectList = split(tchar_to_string(ptr), ",");
            if (streamSelectList.size() > _countof(pAudioSelect->pnStreamChannelSelect)) {
                PrintHelp(strInput[0], _T("Too much streams splitted"), option_name);
                return -1;
            }
            static const char *DELIM = ":";
            for (uint32_t j = 0; j < streamSelectList.size(); j++) {
                auto selectPtr = streamSelectList[j].c_str();
                auto selectDelimPos = strstr(selectPtr, DELIM);
                if (selectDelimPos == nullptr) {
                    auto channelLayout = av_get_channel_layout(selectPtr);
                    pAudioSelect->pnStreamChannelSelect[j] = channelLayout;
                    pAudioSelect->pnStreamChannelOut[j]    = VCE_CHANNEL_AUTO; //自動
                } else if (selectPtr == selectDelimPos) {
                    pAudioSelect->pnStreamChannelSelect[j] = VCE_CHANNEL_AUTO;
                    pAudioSelect->pnStreamChannelOut[j]    = av_get_channel_layout(selectDelimPos + strlen(DELIM));
                } else {
                    pAudioSelect->pnStreamChannelSelect[j] = av_get_channel_layout(streamSelectList[j].substr(0, selectDelimPos - selectPtr).c_str());
                    pAudioSelect->pnStreamChannelOut[j]    = av_get_channel_layout(selectDelimPos + strlen(DELIM));
                }
            }
        }
        if (audioIdx < 0) {
            audioIdx = pParams->nAudioSelectCount;
            //新たに要素を追加
            pParams->ppAudioSelectList = (sAudioSelect **)realloc(pParams->ppAudioSelectList, sizeof(pParams->ppAudioSelectList[0]) * (pParams->nAudioSelectCount + 1));
            pParams->ppAudioSelectList[pParams->nAudioSelectCount] = pAudioSelect;
            pParams->nAudioSelectCount++;
        }
        argData->nParsedAudioSplit++;
        return 0;
    }
    if (IS_OPTION("audio-filter")) {
        if (i+1 < nArgNum) {
            const TCHAR *ptr = nullptr;
            const TCHAR *ptrDelim = nullptr;
            if (strInput[i+1][0] != _T('-')) {
                i++;
                ptrDelim = _tcschr(strInput[i], _T('?'));
                ptr = (ptrDelim == nullptr) ? strInput[i] : ptrDelim+1;
            } else {
                PrintHelp(strInput[0], _T("Invalid value"), option_name);
                return -1;
            }
            int trackId = 1;
            if (ptrDelim == nullptr) {
                trackId = argData->nParsedAudioFilter+1;
                int idx = getAudioTrackIdx(pParams, trackId);
                if (idx >= 0 && pParams->ppAudioSelectList[idx]->pAudioFilter != nullptr) {
                    trackId = getFreeAudioTrack(pParams);
                }
            } else {
                tstring temp = tstring(strInput[i]).substr(0, ptrDelim - strInput[i]);
                if (1 != _stscanf(temp.c_str(), _T("%d"), &trackId)) {
                    PrintHelp(strInput[0], _T("Invalid value"), option_name);
                    return -1;
                }
            }
            sAudioSelect *pAudioSelect = nullptr;
            int audioIdx = getAudioTrackIdx(pParams, trackId);
            if (audioIdx < 0) {
                pAudioSelect = (sAudioSelect *)calloc(1, sizeof(pAudioSelect[0]));
                pAudioSelect->nAudioSelect = trackId;
            } else {
                pAudioSelect = pParams->ppAudioSelectList[audioIdx];
            }
            if (pAudioSelect->pAudioFilter) {
                free(pAudioSelect->pAudioFilter);
            }
            pAudioSelect->pAudioFilter = _tcsdup(ptr);

            if (audioIdx < 0) {
                audioIdx = pParams->nAudioSelectCount;
                //新たに要素を追加
                pParams->ppAudioSelectList = (sAudioSelect **)realloc(pParams->ppAudioSelectList, sizeof(pParams->ppAudioSelectList[0]) * (pParams->nAudioSelectCount + 1));
                pParams->ppAudioSelectList[pParams->nAudioSelectCount] = pAudioSelect;
                pParams->nAudioSelectCount++;
            }
            argData->nParsedAudioFilter++;
        } else {
            PrintHelp(strInput[0], _T("Invalid value"), option_name);
            return -1;
        }
        return 0;
    }
#endif //#if ENABLE_AVCODEC_VCE_READER
    if (   IS_OPTION("chapter-copy")
        || IS_OPTION("copy-chapter")) {
        pParams->bCopyChapter = TRUE;
        return 0;
    }
    if (IS_OPTION("chapter")) {
        if (i+1 < nArgNum && strInput[i+1][0] != _T('-')) {
            i++;
            pParams->pChapterFile = alloc_str(strInput[i]);
        } else {
            PrintHelp(strInput[0], _T("Invalid value"), option_name);
            return -1;
        }
        return 0;
    }
    if (   IS_OPTION("sub-copy")
        || IS_OPTION("copy-sub")) {
        pParams->nAVMux |= (VCEENC_MUX_VIDEO | VCEENC_MUX_SUBTITLE);
        std::set<int> trackSet; //重複しないよう、setを使う
        if (i+1 < nArgNum && (strInput[i+1][0] != _T('-') && strInput[i+1][0] != _T('\0'))) {
            i++;
            auto trackListStr = split(strInput[i], _T(","));
            for (auto str : trackListStr) {
                int iTrack = 0;
                if (1 != _stscanf(str.c_str(), _T("%d"), &iTrack) || iTrack < 1) {
                    PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
                    return -1;
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
#if 0
    if (IS_OPTION("avsync")) {
        int value = 0;
        i++;
        if (PARSE_ERROR_FLAG != (value = get_value_from_chr(list_avsync, strInput[i]))) {
            pParams->nAVSyncMode = (AVSync)value;
        } else {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        return 0;
    }
#endif
    if (IS_OPTION("mux-option")) {
        if (i+1 < nArgNum && strInput[i+1][0] != _T('-')) {
            i++;
            auto ptr = _tcschr(strInput[i], ':');
            if (ptr == nullptr) {
                PrintHelp(strInput[0], _T("invalid value"), option_name);
                return -1;
            } else {
                if (pParams->pMuxOpt == nullptr) {
                    pParams->pMuxOpt = new muxOptList();
                }
                pParams->pMuxOpt->push_back(std::make_pair<tstring, tstring>(tstring(strInput[i]).substr(0, ptr - strInput[i]), tstring(ptr+1)));
            }
        } else {
            PrintHelp(strInput[0], _T("invalid option"), option_name);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("codec")) {
        i++;
        int value = 0;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_codec_all, strInput[i]))) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nCodecId = value;
        return 0;
    }
    if (IS_OPTION("level")) {
        if (i+1 < nArgNum) {
            i++;
            argData->cachedlevel = strInput[i];
        } else {
            PrintHelp(strInput[0], _T("Invalid value"), option_name);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("profile")) {
        if (i+1 < nArgNum) {
            i++;
            argData->cachedprofile = strInput[i];
        } else {
            PrintHelp(strInput[0], _T("Invalid value"), option_name);
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
            pParams->nPAR[0] = 0;
            pParams->nPAR[1] = 0;
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        if (IS_OPTION("dar")) {
            value[0] = -value[0];
            value[1] = -value[1];
        }
        pParams->nPAR[0] = value[0];
        pParams->nPAR[1] = value[1];
        return 0;
    }
    if (IS_OPTION("cqp")) {
        i++;
        int a = 0, b = 0, c = 0;
        if (   3 != _stscanf_s(strInput[i], _T("%d:%d:%d"), &a, &b, &c)
            && 3 != _stscanf_s(strInput[i], _T("%d/%d/%d"), &a, &b, &c)
            && 3 != _stscanf_s(strInput[i], _T("%d,%d,%d"), &a, &b, &c)) {
            if (1 != _stscanf_s(strInput[i], _T("%d"), &a)) {
                PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
                return -1;
            }
            b = a;
            c = a;
        }
        if (a < 0 || b < 0 || c < 0) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nRateControl = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP;
        pParams->nQPI = a;
        pParams->nQPP = b;
        pParams->nQPB = c;
        return 0;
    }
    if (IS_OPTION("vbr")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nRateControl = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
        pParams->nBitrate = value;
        return 0;
    }
    if (IS_OPTION("cbr")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nRateControl = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR;
        pParams->nBitrate = value;
        return 0;
    }
    if (IS_OPTION("qp-max")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nQPMax = value;
        return 0;
    }
    if (IS_OPTION("qp-min")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nQPMin = value;
        return 0;
    }
    if (IS_OPTION("bframes")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name, strInput[i]);
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
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nDeltaQPBFrame = value;
        return 0;
    }
    if (IS_OPTION("bref-deltaqp")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nDeltaQPBFrameRef = value;
        return 0;
    }
    if (IS_OPTION("max-bitrate")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nMaxBitrate = value;
        return 0;
    }
    if (IS_OPTION("vbv-bufsize")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nVBVBufferSize = value;
        return 0;
    }
    if (IS_OPTION("slices")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name, strInput[i]);
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
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nMotionEst = value;
        return 0;
    }
    if (IS_OPTION("vbaq")) {
        pParams->bVBAQ = 1;
        return 0;
    }
    if (IS_OPTION("pre-analysis")) {
        if (i+1 < nArgNum) {
            i++;
            argData->cachedPreAnalysis = strInput[i];
        } else {
            PrintHelp(strInput[0], _T("Invalid value"), option_name);
            return -1;
        }
        return 0;
    }
    if (IS_OPTION("gop-len")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        } else if (value < 0) {
            PrintHelp(strInput[0], _T("Invalid value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nGOPLen = value;
        return 0;
    }
    if (IS_OPTION("tff")) {
        pParams->nPicStruct = AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_TOP_FIELD;
        return 0;
    }
    if (IS_OPTION("bff")) {
        pParams->nPicStruct = AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_BOTTOM_FIELD;
        return 0;
    }
#if 0
    if (IS_OPTION("colormatrix")) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG != (value = get_value_from_chr(list_colormatrix, strInput[i]))) {
            pParams->vui.colormatrix = value;
            pParams->vui.infoPresent = true;
        }
        return 0;
    }
    if (IS_OPTION("colorprim")) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG != (value = get_value_from_chr(list_colorprim, strInput[i]))) {
            pParams->vui.colorprim = value;
            pParams->vui.infoPresent = true;
        }
        return 0;
    }
    if (IS_OPTION("transfer")) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG != (value = get_value_from_chr(list_transfer, strInput[i]))) {
            pParams->vui.transfer = value;
            pParams->vui.infoPresent = true;
        }
        return 0;
    }
    if (IS_OPTION("videoformat")) {
        i++;
        int value;
        if (PARSE_ERROR_FLAG != (value = get_value_from_chr(list_videoformat, strInput[i]))) {
            pParams->vui.videoformat = value;
            pParams->vui.infoPresent = true;
        }
        return 0;
    }
    if (IS_OPTION("fullrange")) {
        pParams->vui.fullrange = true;
        pParams->vui.infoPresent = true;
        return 0;
    }
#endif
    if (IS_OPTION("log")) {
        i++;
        pParams->pStrLogFile = _tcsdup(strInput[i]);
        return 0;
    }
    if (IS_OPTION("log-framelist")) {
        i++;
        pParams->pFramePosListLog = _tcsdup(strInput[i]);
        return 0;
    }
    if (IS_OPTION("log-level")) {
        i++;
        int value = VCE_LOG_INFO;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_log_level, strInput[i]))) {
            PrintHelp(strInput[0], _T("Unknown value"), option_name, strInput[i]);
            return -1;
        }
        pParams->nLogLevel = value;
        return 0;
    }
    tstring mes = _T("Unknown option: --");
    mes += option_name;
    PrintHelp(strInput[0], (TCHAR *)mes.c_str(), NULL, strInput[i]);
    return -1;
}

int parse_args(VCEParam *pParams, VCEInputInfo *pInputInfo, int nArgNum, const TCHAR* strInput[]) {
    sArgsData argsData;
    for (int i = 1; i < nArgNum; i++) {
        if (strInput[i] == nullptr) {
            return -1;
        }
        const TCHAR *option_name = nullptr;

        if (strInput[i][0] == _T('|')) {
            break;
        } else if (strInput[i][0] == _T('-')) {
            if (strInput[i][1] == _T('-')) {
                option_name = &strInput[i][2];
            } else if (strInput[i][2] == _T('\0')) {
                if (nullptr == (option_name = short_opt_to_long(strInput[i][1]))) {
                    PrintHelp(strInput[0], strsprintf(_T("Unknown options: \"%s\""), strInput[i]).c_str(), NULL, NULL);
                    return -1;
                }
            } else {
                PrintHelp(strInput[0], strsprintf(_T("Invalid options: \"%s\""), strInput[i]).c_str(), NULL, NULL);
                return -1;
            }
        }

        if (option_name == nullptr) {
            PrintHelp(strInput[0], strsprintf(_T("Unknown option: \"%s\""), strInput[i]).c_str(), NULL, NULL);
            return -1;
        }

        // process multi-character options
        if (IS_OPTION("help")) {
            PrintHelp(strInput[0], NULL, NULL);
            return 1;
        }
        if (IS_OPTION("version")) {
            PrintVersion();
            return 1;
        }
        if (IS_OPTION("check-vce")) {
            if (check_if_vce_supported()) {
                _ftprintf(stderr, _T("VCE available.\n"));
                exit(0);
            }
            exit(1);
        }
#if ENABLE_AVCODEC_VCE_READER
        if (IS_OPTION("check-avversion")) {
            _ftprintf(stdout, _T("%s\n"), getAVVersions().c_str());
            return 1;
        }
        if (IS_OPTION("check-codecs")) {
            _ftprintf(stdout, _T("%s\n"), getAVCodecs((AVQSVCodecType)(AVQSV_CODEC_DEC | AVQSV_CODEC_ENC)).c_str());
            return 1;
        }
        if (IS_OPTION("check-encoders")) {
            _ftprintf(stdout, _T("%s\n"), getAVCodecs(AVQSV_CODEC_ENC).c_str());
            return 1;
        }
        if (IS_OPTION("check-decoders")) {
            _ftprintf(stdout, _T("%s\n"), getAVCodecs(AVQSV_CODEC_DEC).c_str());
            return 1;
        }
        if (IS_OPTION("check-protocols")) {
            _ftprintf(stdout, _T("%s\n"), getAVProtocols().c_str());
            return 1;
        }
        if (IS_OPTION("check-filters")) {
            _ftprintf(stdout, _T("%s\n"), getAVFilters().c_str());
            return 1;
        }
        if (IS_OPTION("check-formats")) {
            _ftprintf(stdout, _T("%s\n"), getAVFormats((AVQSVFormatType)(AVQSV_FORMAT_DEMUX | AVQSV_FORMAT_MUX)).c_str());
            return 1;
        }
#endif //ENABLE_AVCODEC_VCE_READER
        auto sts = ParseOneOption(option_name, strInput, i, nArgNum, pParams, pInputInfo, &argsData);
        if (sts != 0) {
            return sts;
        }
    }

    //parse cached profile and level
    if (argsData.cachedlevel.length() > 0) {
        const auto desc = get_level_list(pParams->nCodecId);
        int value = 0;
        bool bParsed = false;
        if (desc != nullptr) {
            if (PARSE_ERROR_FLAG != (value = get_value_from_chr(desc, argsData.cachedlevel.c_str()))) {
                pParams->codecParam[pParams->nCodecId].nLevel = value;
                bParsed = true;
            } else if (pParams->nCodecId == VCE_CODEC_H264) {
                double val_float = 0.0;
                if (1 == _stscanf_s(argsData.cachedlevel.c_str(), _T("%lf"), &val_float)) {
                    value = (int)(val_float * 10 + 0.5);
                    if (value == desc[get_cx_index(desc, value)].value) {
                        pParams->codecParam[pParams->nCodecId].nLevel = value;
                        bParsed = true;
                    }
                }
            }
        }
        if (!bParsed) {
            PrintHelp(strInput[0], _T("Unknown value"), _T("level"));
            return -1;
        }
    }
    if (argsData.cachedprofile.length() > 0) {
        const auto desc = get_profile_list(pParams->nCodecId);
        int value = 0;
        if (desc != nullptr && PARSE_ERROR_FLAG != (value = get_value_from_chr(desc, argsData.cachedprofile.c_str()))) {
            pParams->codecParam[pParams->nCodecId].nProfile = value;
        } else {
            PrintHelp(strInput[0], _T("Unknown value"), _T("profile"));
            return -1;
        }
    }
    if (argsData.cachedPreAnalysis.length() > 0) {
        const auto desc = get_pre_analysis_list(pParams->nCodecId);
        int value = 0;
        if (desc != nullptr && PARSE_ERROR_FLAG == (value = get_value_from_chr(desc, argsData.cachedPreAnalysis.c_str()))) {
            PrintHelp(strInput[0], _T("Unknown value"), _T("pre-analysis"));
            return -1;
        }
        pParams->nPreAnalysis = value;
    }
    if (pParams->nCodecId == VCE_CODEC_HEVC) {
        int nH264RateControl = pParams->nRateControl;
        switch (nH264RateControl) {
        case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR:
            pParams->nRateControl = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CBR;
            break;
        case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR:
            pParams->nRateControl = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_LATENCY_CONSTRAINED_VBR;
            break;
        case AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTANT_QP:
        default:
            pParams->nRateControl = AMF_VIDEO_ENCODER_HEVC_RATE_CONTROL_METHOD_CONSTANT_QP;
            break;
        }
    }
    return 0;
}

int vce_run(VCEParam *pParams, VCEInputInfo *pInputInfo) {
    unique_ptr<VCECore> vce(new VCECore());
    if (AMF_OK != vce->init(pParams, pInputInfo)) {
        return 1;
    }
    vce->PrintEncoderParam();

    try {
        if (AMF_OK != vce->run()) {
            return 1;
        }

        PipelineState state = PipelineStateRunning;
        while ((state = vce->GetState()) != PipelineStateEof && state != PipelineStateError) {
            amf_sleep(100);
        }
        if (state == PipelineStateError) {
            _ftprintf(stderr, _T("fatal error in encoding pipeline.\n"));
            return 1;
        }
        vce->PrintResult();
    } catch (...) {
        _ftprintf(stderr, _T("fatal error in encoding pipeline.\n"));
        return 1;
    }
    return 0;
}

static bool check_locale_is_ja() {
    const WORD LangID_ja_JP = MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN);
    return GetUserDefaultLangID() == LangID_ja_JP;
}

int _tmain(int argc, TCHAR **argv) {
    if (argc <= 1) {
        PrintHelp(argv[0], NULL, NULL);
        return 1;
    }

    if (check_locale_is_ja()) {
        _tsetlocale(LC_ALL, _T("Japanese"));
    }

    VCEInputInfo inputInfo = { 0 };
    inputInfo.format = amf::AMF_SURFACE_NV12;

    vector<const TCHAR *> argvCopy(argv, argv + argc);
    argvCopy.push_back(_T(""));

    VCEParam prm;
    init_vce_param(&prm);
    if (parse_args(&prm, &inputInfo, argc, argvCopy.data())) {
        return 1;
    }

    if (!check_if_vce_supported()) {
        return 1;
    }

    if (vce_run(&prm, &inputInfo)) {
        fprintf(stderr, "Finished with error in VCEEncC.\n");
        return 1;
    }
    return 0;
}
