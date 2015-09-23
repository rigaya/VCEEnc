//  -----------------------------------------------------------------------------------------
//    VCEEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <cstdio>
#include <tchar.h>
#include "VCEVersion.h"
#include "VCEParam.h"
#include "VCECore.h"

static void print_version() {
    static const TCHAR *const ENABLED_INFO[] = { _T("disabled"), _T("enabled") };
    _ftprintf(stdout, _T("VCEEncC (%s) %s by rigaya, build %s %s\n"), BUILD_ARCH_STR, VER_STR_FILEVERSION_TCHAR, _T(__DATE__), _T(__TIME__));
    _ftprintf(stdout, _T("  avs reader:   %s\n"), ENABLED_INFO[!!ENABLE_AVISYNTH_READER]);
    _ftprintf(stdout, _T("  vpy reader:   %s\n"), ENABLED_INFO[!!ENABLE_VAPOURSYNTH_READER]);
}

static bool check_if_vce_supported() {
    tstring mes;
    if (!check_if_vce_available(mes)) {
        _ftprintf(stderr, _T("%s\n"), mes.c_str());
        return false;
    }
    return true;
}

static void print_help() {
    print_version();

    _ftprintf(stdout,
        _T("Usage: VCEEncC [Options] -i <filename> -o <filename>\n"));
    _ftprintf(stdout, _T("\n")
        _T("input can be %s%sraw YUV or YUV4MPEG2(y4m) format.\n")
        _T("when raw(default), fps, input-res are also necessary.\n")
        _T("\n")
        _T("output format will be raw H.264/AVC ES.\n")
        _T("\n")
        _T("Example:\n")
        _T("  VCEEncC -i \"<avsfilename>\" -o \"<outfilename>\"\n")
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
        _T("\n")
        _T(" Input formats (will be estimated from extension if not set.)\n")
        _T("   --raw                        set input as raw format\n")
        _T("   --y4m                        set input as y4m format\n")
        _T("   --avs                        set input as avs format\n")
        _T("   --vpy                        set input as vpy format\n")
        _T("   --vpy-mt                     set input as vpy format in multi-thread\n")
        _T("\n")
        _T("   --input-res <int>x<int>      set input resolution\n")
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
        _T("   --gop-len <int>              set length of gop (default: auto)\n")
        _T("   --log-level <int>            set log level\n")
        _T("                                 error, warn, info(default), debug\n"),
        (ENABLE_AVISYNTH_READER)    ? _T("avs, ") : _T(""),
        (ENABLE_VAPOURSYNTH_READER) ? _T("vpy, ") : _T(""),
        VCE_DEFAULT_QPI, VCE_DEFAULT_QPP, VCE_DEFAULT_QPB, VCE_DEFAULT_BFRAMES,
        VCE_DEFAULT_MAX_BITRATE, VCE_DEFAULT_VBV_BUFSIZE, VCE_DEFAULT_SLICES
        );
}

int parse_arg_line(VCEParam *pParam, VCEInputInfo *pInputInfo, const TCHAR *option, int i, const TCHAR * const*strInput) {
#define IS_OPTION(x) (0 == _tcsicmp(option, _T(x)))
    if (IS_OPTION("help")) {
        print_help();
        return -1;
    }
    if (IS_OPTION("version")) {
        print_version();
        return -1;
    }
    if (IS_OPTION("check-vce")) {
        if (check_if_vce_supported()) {
            _ftprintf(stderr, _T("VCE available.\n"));
            exit(0);
        }
        exit(1);
    }
    if (IS_OPTION("input-file")) {
        pParam->pInputFile = strInput[i+1];
        return 1;
    }
    if (IS_OPTION("output-file")) {
        pParam->pOutputFile = strInput[i+1];
        return 1;
    }
    if (IS_OPTION("raw")) {
        pParam->nInputType = VCE_INPUT_RAW;
        return 0;
    }
    if (IS_OPTION("y4m")) {
        pParam->nInputType = VCE_INPUT_Y4M;
        return 0;
    }
    if (IS_OPTION("avs")) {
        pParam->nInputType = VCE_INPUT_AVS;
        return 0;
    }
    if (IS_OPTION("vpy")) {
        pParam->nInputType = VCE_INPUT_VPY;
        return 0;
    }
    if (IS_OPTION("vpy-mt")) {
        pParam->nInputType = VCE_INPUT_VPY_MT;
        return 0;
    }
    if (IS_OPTION("quality")) {
        i++;
        int value = AMF_VIDEO_ENCODER_QUALITY_PRESET_BALANCED;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_vce_quality_preset, strInput[i]))) {
            _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
            return -1;
        }
        pParam->nQualityPreset = value;
        return 1;
    }
    if (IS_OPTION("input-res")) {
        i++;
        int width = 0, height = 0;
        if (   2 != _stscanf_s(strInput[i], _T("%dx%d"), &width, &height)
            && 2 != _stscanf_s(strInput[i], _T("%d,%d"), &width, &height)
            && 2 != _stscanf_s(strInput[i], _T("%d/%d"), &width, &height)
            && 2 != _stscanf_s(strInput[i], _T("%d:%d"), &width, &height)) {
            _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
            return -1;
        }
        pInputInfo->srcWidth  = width;
        pInputInfo->srcHeight = height;
        return 1;
    }
    if (IS_OPTION("fps")) {
        i++;
        VCERational framerate;
        if (   2 != _stscanf_s(strInput[i], _T("%dx%d"), &framerate.num, &framerate.den)
            && 2 != _stscanf_s(strInput[i], _T("%d,%d"), &framerate.num, &framerate.den)
            && 2 != _stscanf_s(strInput[i], _T("%d/%d"), &framerate.num, &framerate.den)
            && 2 != _stscanf_s(strInput[i], _T("%d:%d"), &framerate.num, &framerate.den)) {
            if (1 != _stscanf_s(strInput[i], _T("%d"), &framerate.num)) {
                _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
                return -1;
            }
            framerate.den = 1;
        }
        pInputInfo->fps = framerate;
        return 1;
    }
    if (IS_OPTION("crop")) {
        i++;
        int crop[4] = { 0 };
        if (   4 != _stscanf_s(strInput[i], _T("%d,%d,%d,%d"), &crop[0], &crop[1], &crop[2], &crop[3])
            && 4 != _stscanf_s(strInput[i], _T("%d:%d:%d:%d"), &crop[0], &crop[1], &crop[2], &crop[3])) {
            _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
            return -1;
        }
        memcpy(pInputInfo->crop.c, crop, sizeof(crop));
        return 1;
    }
    if (IS_OPTION("cqp")) {
        i++;
        int a = 0, b = 0, c = 0;
        if (   3 != _stscanf_s(strInput[i], _T("%d:%d:%d"), &a, &b, &c)
            && 3 != _stscanf_s(strInput[i], _T("%d/%d/%d"), &a, &b, &c)
            && 3 != _stscanf_s(strInput[i], _T("%d,%d,%d"), &a, &b, &c)) {
            if (1 != _stscanf_s(strInput[i], _T("%d"), &a)) {
                _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
                return -1;
            }
            b = a;
            c = a;
        }
        if (a < 0 || b < 0 || c < 0) {
            _ftprintf(stderr, _T("\"%s\" requires non-negative value, input value \"%s\"\n"), strInput[i-1], strInput[i]);
            return -1;
        }
        pParam->nRateControl = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CONSTRAINED_QP;
        pParam->nQPI = a;
        pParam->nQPP = b;
        pParam->nQPB = c;
        return 1;
    }
    if (IS_OPTION("vbr")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
            return -1;
        } else if (value < 0) {
            _ftprintf(stderr, _T("\"%s\" requires non-negative value, input value \"%s\"\n"), strInput[i-1], strInput[i]);
            return -1;
        }
        pParam->nRateControl = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_PEAK_CONSTRAINED_VBR;
        pParam->nBitrate = value;
        return 1;
    }
    if (IS_OPTION("cbr")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
            return -1;
        } else if (value < 0) {
            _ftprintf(stderr, _T("\"%s\" requires non-negative value, input value \"%s\"\n"), strInput[i-1], strInput[i]);
            return -1;
        }
        pParam->nRateControl = AMF_VIDEO_ENCODER_RATE_CONTROL_METHOD_CBR;
        pParam->nBitrate = value;
        return 1;
    }
    if (IS_OPTION("qp-max")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
            return -1;
        } else if (value < 0) {
            _ftprintf(stderr, _T("\"%s\" requires non-negative value, input value \"%s\"\n"), strInput[i-1], strInput[i]);
            return -1;
        }
        pParam->nQPMax = value;
        return 1;
    }
    if (IS_OPTION("qp-min")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
            return -1;
        } else if (value < 0) {
            _ftprintf(stderr, _T("\"%s\" requires non-negative value, input value \"%s\"\n"), strInput[i-1], strInput[i]);
            return -1;
        }
        pParam->nQPMin = value;
        return 1;
    }
    if (IS_OPTION("bframes")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
            return -1;
        } else if (value < 0) {
            _ftprintf(stderr, _T("\"%s\" requires non-negative value, input value \"%s\"\n"), strInput[i-1], strInput[i]);
            return -1;
        }
        pParam->nBframes = value;
        return 1;
    }
    if (IS_OPTION("b-pyramid")) {
        pParam->bBPyramid = true;
        return 0;
    }
    if (IS_OPTION("no-b-pyramid")) {
        pParam->bBPyramid = false;
        return 0;
    }
    if (IS_OPTION("b-deltaqp")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
            return -1;
        }
        pParam->nDeltaQPBFrame = value;
        return 1;
    }
    if (IS_OPTION("bref-deltaqp")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
            return -1;
        }
        pParam->nDeltaQPBFrameRef = value;
        return 1;
    }
    if (IS_OPTION("max-bitrate")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
            return -1;
        } else if (value < 0) {
            _ftprintf(stderr, _T("\"%s\" requires non-negative value, input value \"%s\"\n"), strInput[i-1], strInput[i]);
            return -1;
        }
        pParam->nMaxBitrate = value;
        return 1;
    }
    if (IS_OPTION("vbv-bufsize")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
            return -1;
        } else if (value < 0) {
            _ftprintf(stderr, _T("\"%s\" requires non-negative value, input value \"%s\"\n"), strInput[i-1], strInput[i]);
            return -1;
        }
        pParam->nVBVBufferSize = value;
        return 1;
    }
    if (IS_OPTION("slices")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
            return -1;
        } else if (value < 0) {
            _ftprintf(stderr, _T("\"%s\" requires non-negative value, input value \"%s\"\n"), strInput[i-1], strInput[i]);
            return -1;
        }
        pParam->nSlices = value;
        return 1;
    }
    if (IS_OPTION("skip-frame")) {
        pParam->bEnableSkipFrame = true;
        return 0;
    }
    if (IS_OPTION("no-skip-frame")) {
        pParam->bEnableSkipFrame = false;
        return 0;
    }
    if (IS_OPTION("motion-est")) {
        i++;
        int value = VCE_MOTION_EST_QUATER | VCE_MOTION_EST_HALF;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_mv_presicion, strInput[i]))) {
            _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
            return -1;
        }
        pParam->nMotionEst = value;
        return 1;
    }
    if (IS_OPTION("gop-len")) {
        i++;
        int value = 0;
        if (1 != _stscanf_s(strInput[i], _T("%d"), &value)) {
            _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
            return -1;
        } else if (value < 0) {
            _ftprintf(stderr, _T("\"%s\" requires non-negative value, input value \"%s\"\n"), strInput[i-1], strInput[i]);
            return -1;
        }
        pParam->nGOPLen = value;
        return 1;
    }
    if (IS_OPTION("log-level")) {
        i++;
        int value = VCE_LOG_INFO;
        if (PARSE_ERROR_FLAG == (value = get_value_from_chr(list_log_level, strInput[i]))) {
            _ftprintf(stderr, _T("Unknown value \"%s\" for \"%s\"\n"), strInput[i], strInput[i-1]);
            return -1;
        }
        pParam->nLogLevel = value;
        return 1;
    }
    _ftprintf(stderr, _T("Unknow Options: \"%s\"\n"), strInput[i]);
    return -1;
#undef IS_OPTION
}

int parse_args(VCEParam *pParam, VCEInputInfo *pInputInfo, int argc, const TCHAR * const*argv) {
    for (int i = 1; i < argc; i++) {
        const TCHAR *option_name = NULL;

        if (argv[i][0] == _T('-')) {
            if (argv[i][1] == _T('-')) {
                option_name = &argv[i][2];
            } else if (argv[i][2] == _T('\0')) {
                switch (argv[i][1]) {
                case _T('b'):
                    option_name = _T("bframes");
                    break;
                case _T('c'):
                    option_name = _T("codec");
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
                case _T('v'):
                    option_name = _T("version");
                    break;
                case _T('h'):
                case _T('?'):
                    option_name = _T("help");
                    break;
                default:
                    print_help();
                    _ftprintf(stderr, _T("Unknow Options: \"%s\"\n"), argv[i]);
                    return -1;
                }
            } else {
                print_help();
                _ftprintf(stderr, _T("Invalid options: \"%s\"\n"), argv[i]);
                return -1;
            }

            int ret = parse_arg_line(pParam, pInputInfo, option_name, i, argv);
            if (ret < 0) {
                return -1;
            }
            i += ret;
        } else {
            _ftprintf(stderr, _T("Invalid options: \"%s\"\n"), argv[i]);
            return -1;
        }
    }
    return 0;
}

int vce_run(VCEParam *pParam, VCEInputInfo *pInputInfo) {
    unique_ptr<VCECore> vce(new VCECore());
    if (AMF_OK != vce->init(pParam, pInputInfo)) {
        return 1;
    }
    vce->PrintEncoderParam();

    try {
        if (AMF_OK != vce->run()) {
            return 1;
        }

        while (vce->GetState() != PipelineStateEof) {
            amf_sleep(100);
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
        print_help();
        return 1;
    }

    if (check_locale_is_ja()) {
        _tsetlocale(LC_ALL, _T("Japanese"));
    }

    VCEInputInfo inputInfo = { 0 };
    inputInfo.format = amf::AMF_SURFACE_NV12;

    VCEParam prm;
    init_vce_param(&prm);
    if (parse_args(&prm, &inputInfo, argc, argv)) {
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
