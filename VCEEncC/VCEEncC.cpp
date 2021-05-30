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
#include <filesystem>
#include <cstdio>
#include <cassert>
#include <tchar.h>
#include "rgy_version.h"
#include "rgy_codepage.h"
#include "rgy_resource.h"
#include "rgy_env.h"
#include "vce_param.h"
#include "vce_core.h"
#include "vce_cmd.h"
#include "rgy_util.h"
#include "rgy_avutil.h"

static void show_version() {
    _ftprintf(stdout, _T("%s\n"), GetVCEEncVersion().c_str());
}

static void show_help() {
    _ftprintf(stdout, _T("%s\n"), encoder_help().c_str());
}

static void show_hw(int deviceid) {
    show_version();
    auto loglevel = RGY_LOG_ERROR;
    if (check_if_vce_available(deviceid, loglevel)) {
        auto core = std::make_unique<VCECore>();
        auto err = RGY_ERR_NONE;
        if ((err = core->initLog(loglevel)) == RGY_ERR_NONE
            && (err = core->initAMFFactory()) == RGY_ERR_NONE
            && (err = core->initTracer(loglevel)) == RGY_ERR_NONE) {
            const auto devList = core->createDeviceList(false, true);
            if (devList.size() > 0) {
                _ftprintf(stdout, _T("VCE available\n"));
                for (auto &dev : devList) {
                    if (deviceid < 0 || dev->id() == deviceid) {
                        _ftprintf(stdout, _T("device #%d: %s\n"), dev->id(), dev->name().c_str());
                    }
                }
                exit(0);
            }
        }
    }
    _ftprintf(stdout, _T("VCE unavailable.\n"));
    exit(1);
}

static void show_vce_features(int deviceid) {
    const auto codecs = std::vector<RGY_CODEC>{RGY_CODEC_H264, RGY_CODEC_HEVC};
    _ftprintf(stdout, _T("%s\n"), check_vce_enc_features(codecs, deviceid, RGY_LOG_ERROR).c_str());
    _ftprintf(stdout, _T("\n%s\n"), check_vce_dec_features(deviceid, RGY_LOG_ERROR).c_str());
    exit(0);
}

static void show_environment_info() {
    _ftprintf(stderr, _T("%s\n"), getEnviromentInfo().c_str());
}

static void show_option_list() {
    show_version();

    std::vector<std::string> optList;
    for (const auto &optHelp : createOptionList()) {
        optList.push_back(optHelp.first);
    }
    std::sort(optList.begin(), optList.end());

    _ftprintf(stdout, _T("Option List:\n"));
    for (const auto &optHelp : optList) {
        _ftprintf(stdout, _T("--%s\n"), char_to_tstring(optHelp).c_str());
    }
}

int parse_print_options(const TCHAR *option_name, const TCHAR *arg1) {

#define IS_OPTION(x) (0 == _tcscmp(option_name, _T(x)))

    if (IS_OPTION("help")) {
        show_version();
        show_help();
        return 1;
    }
    if (IS_OPTION("version")) {
        show_version();
        return 1;
    }
    if (IS_OPTION("option-list")) {
        show_option_list();
        return 1;
    }
#if 0
    if (IS_OPTION("check-device")) {
        show_device_list();
        return 1;
    }
#endif
    if (IS_OPTION("check-hw")) {
        int deviceid = -1;
        if (arg1 && arg1[0] != '-') {
            int value = 0;
            if (1 == _stscanf_s(arg1, _T("%d"), &value)) {
                deviceid = value;
            }
        }
        show_hw(deviceid);
        return 1;
    }
    if (IS_OPTION("check-environment")) {
        show_environment_info();
        return 1;
    }
    if (IS_OPTION("check-features")) {
        int deviceid = 0;
        if (arg1 && arg1[0] != '-') {
            int value = 0;
            if (1 == _stscanf_s(arg1, _T("%d"), &value)) {
                deviceid = value;
            }
        }
        show_vce_features(deviceid);
        return 1;
    }
#if ENABLE_AVSW_READER
    if (0 == _tcscmp(option_name, _T("check-avversion"))) {
        _ftprintf(stdout, _T("%s\n"), getAVVersions().c_str());
        return 1;
    }
    if (0 == _tcscmp(option_name, _T("check-codecs"))) {
        _ftprintf(stdout, _T("%s\n"), getAVCodecs((RGYAVCodecType)(RGY_AVCODEC_DEC | RGY_AVCODEC_ENC)).c_str());
        return 1;
    }
    if (0 == _tcscmp(option_name, _T("check-encoders"))) {
        _ftprintf(stdout, _T("%s\n"), getAVCodecs(RGY_AVCODEC_ENC).c_str());
        return 1;
    }
    if (0 == _tcscmp(option_name, _T("check-decoders"))) {
        _ftprintf(stdout, _T("%s\n"), getAVCodecs(RGY_AVCODEC_DEC).c_str());
        return 1;
    }
    if (0 == _tcscmp(option_name, _T("check-profiles"))) {
        auto list = getAudioPofileList(arg1);
        if (list.size() == 0) {
            _ftprintf(stdout, _T("Failed to find codec name \"%s\"\n"), arg1);
        } else {
            _ftprintf(stdout, _T("profile name for \"%s\"\n"), arg1);
            for (const auto &name : list) {
                _ftprintf(stdout, _T("  %s\n"), name.c_str());
            }
        }
        return 1;
    }
    if (0 == _tcscmp(option_name, _T("check-protocols"))) {
        _ftprintf(stdout, _T("%s\n"), getAVProtocols().c_str());
        return 1;
    }
    if (0 == _tcscmp(option_name, _T("check-formats"))) {
        _ftprintf(stdout, _T("%s\n"), getAVFormats((RGYAVFormatType)(RGY_AVFORMAT_DEMUX | RGY_AVFORMAT_MUX)).c_str());
        return 1;
    }
    if (0 == _tcscmp(option_name, _T("check-filters"))) {
        _ftprintf(stdout, _T("%s\n"), getAVFilters().c_str());
        return 1;
    }
#endif //#if ENABLE_AVSW_READER
#undef IS_OPTION
    return 0;
}

static bool check_locale_is_ja() {
    const WORD LangID_ja_JP = MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN);
    return GetUserDefaultLangID() == LangID_ja_JP;
}

#if defined(_WIN32) || defined(_WIN64)
static tstring getErrorFmtStr(uint32_t err) {
    TCHAR errmes[4097];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, NULL, errmes, _countof(errmes), NULL);
    return errmes;
}

static int run_on_os_codepage() {
    auto exepath = getExePath();
    auto tmpexe = std::filesystem::path(PathRemoveExtensionS(exepath));
    tmpexe += strsprintf(_T("A_%x"), GetCurrentProcessId());
    tmpexe += std::filesystem::path(exepath).extension();
    std::filesystem::copy_file(exepath, tmpexe, std::filesystem::copy_options::overwrite_existing);

    SetLastError(0);
    HANDLE handle = BeginUpdateResourceW(tmpexe.wstring().c_str(), FALSE);
    if (handle == NULL) {
        auto lasterr = GetLastError();
        _ftprintf(stderr, _T("Failed to create temporary exe file: [%d] %s.\n"), lasterr, getErrorFmtStr(lasterr).c_str());
        return 1;
    }
    void *manifest = nullptr;
    int size = getEmbeddedResource(&manifest,_T("APP_OSCODEPAGE_MANIFEST"), _T("EXE_DATA"));
    if (size == 0) {
        _ftprintf(stderr, _T("Failed to load manifest for OS codepage mode.\n"));
        return 1;
    }
    SetLastError(0);
    if (!UpdateResourceW(handle, RT_MANIFEST, CREATEPROCESS_MANIFEST_RESOURCE_ID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), manifest, size)) {
        auto lasterr = GetLastError();
        _ftprintf(stderr, _T("Failed to update manifest for ansi mode: [%d] %s.\n"), lasterr, getErrorFmtStr(lasterr).c_str());
        return 1;
    }
    SetLastError(0);
    if (!EndUpdateResourceW(handle, FALSE)) {
        auto lasterr = GetLastError();
        _ftprintf(stderr, _T("Failed to finish update manifest for OS codepage mode: [%d] %s.\n"), lasterr, getErrorFmtStr(lasterr).c_str());
        return 1;
    }

    const auto commandline = str_replace(str_replace(GetCommandLineW(),
        std::filesystem::path(exepath).filename(), std::filesystem::path(tmpexe).filename()),
        CODEPAGE_CMDARG, CODEPAGE_CMDARG_APPLIED);

    int ret = 0;
    try {
        DWORD flags = 0; // CREATE_NO_WINDOW;

        HANDLE hStdIn, hStdOut, hStdErr;
        DuplicateHandle(GetCurrentProcess(), GetStdHandle(STD_INPUT_HANDLE),  GetCurrentProcess(), &hStdIn,  0, TRUE, DUPLICATE_SAME_ACCESS);
        DuplicateHandle(GetCurrentProcess(), GetStdHandle(STD_OUTPUT_HANDLE), GetCurrentProcess(), &hStdOut, 0, TRUE, DUPLICATE_SAME_ACCESS);
        DuplicateHandle(GetCurrentProcess(), GetStdHandle(STD_ERROR_HANDLE),  GetCurrentProcess(), &hStdErr, 0, TRUE, DUPLICATE_SAME_ACCESS);

        SECURITY_ATTRIBUTES sa;
        memset(&sa, 0, sizeof(SECURITY_ATTRIBUTES));
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE; //TRUEでハンドルを引き継ぐ

        STARTUPINFO si;
        memset(&si, 0, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        //si.dwFlags |= STARTF_USESHOWWINDOW;
        si.dwFlags |= STARTF_USESTDHANDLES;
        //si.wShowWindow |= SW_SHOWMINNOACTIVE;
        si.hStdInput = hStdIn;
        si.hStdOutput = hStdOut;
        si.hStdError = hStdErr;

        PROCESS_INFORMATION pi;
        memset(&pi, 0, sizeof(PROCESS_INFORMATION));

        SetLastError(0);
        if (CreateProcess(nullptr, (LPWSTR)commandline.c_str(), &sa, nullptr, TRUE, flags, nullptr, nullptr, &si, &pi) == 0) {
            auto lasterr = GetLastError();
            _ftprintf(stderr, _T("Failed to run process in OS codepage mode: [%d] %s.\n"), lasterr, getErrorFmtStr(lasterr).c_str());
            ret = 1;
        } else {
            WaitForSingleObject(pi.hProcess, INFINITE);
            DWORD proc_ret = 0;
            if (GetExitCodeProcess(pi.hProcess, &proc_ret)) {
                ret = (int)proc_ret;
            }
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    } catch (...) {
        ret = 1;
    }
    std::filesystem::remove(tmpexe);
    return ret;
}
#endif //#if defined(_WIN32) || defined(_WIN64)

int vce_run(VCEParam *pParams) {
    unique_ptr<VCECore> vce(new VCECore());
    if (AMF_OK != vce->init(pParams)) {
        return 1;
    }
    vce->PrintEncoderParam();

    try {
        if (AMF_OK != vce->run()) {
            return 1;
        }
    } catch (...) {
        _ftprintf(stderr, _T("fatal error in encoding pipeline.\n"));
        return 1;
    }
    return 0;
}

int _tmain(int argc, TCHAR **argv) {
#if defined(_WIN32) || defined(_WIN64)
    if (check_locale_is_ja()) {
        _tsetlocale(LC_ALL, _T("Japanese"));
    }
#endif //#if defined(_WIN32) || defined(_WIN64)

    if (argc == 1) {
        show_version();
        show_help();
        return 1;
    }

#if defined(_WIN32) || defined(_WIN64)
    if (GetACP() == CODE_PAGE_UTF8) {
        bool switch_to_os_cp = false;
        for (int iarg = 1; iarg < argc; iarg++) {
            if (iarg + 1 < argc
                && _tcscmp(argv[iarg + 0], CODEPAGE_CMDARG) == 0) {
                if (_tcscmp(argv[iarg + 1], _T("os")) == 0) {
                    switch_to_os_cp = true;
                } else if (_tcscmp(argv[iarg + 1], _T("utf8")) == 0) {
                    switch_to_os_cp = false;
                } else {
                    _ftprintf(stderr, _T("Unknown option for %s.\n"), CODEPAGE_CMDARG);
                    return 1;
                }
            }
        }
        if (switch_to_os_cp) {
            return run_on_os_codepage();
        }
    }
#endif //#if defined(_WIN32) || defined(_WIN64)

    for (int iarg = 1; iarg < argc; iarg++) {
        const TCHAR *option_name = nullptr;
        if (argv[iarg][0] == _T('-')) {
            if (argv[iarg][1] == _T('\0')) {
                continue;
            } else if (argv[iarg][1] == _T('-')) {
                option_name = &argv[iarg][2];
            } else if (argv[iarg][2] == _T('\0')) {
                if (nullptr == (option_name = cmd_short_opt_to_long(argv[iarg][1]))) {
                    continue;
                }
            }
        }
        if (option_name != nullptr) {
            int ret = parse_print_options(option_name, (iarg+1 < argc) ? argv[iarg+1] : _T(""));
            if (ret != 0) {
                return ret == 1 ? 0 : 1;
            }
        }
    }
    
    //optionファイルの読み取り
    std::vector<tstring> argvCnfFile;
    for (int iarg = 1; iarg < argc; iarg++) {
        const TCHAR *option_name = nullptr;
        if (argv[iarg][0] == _T('-')) {
            if (argv[iarg][1] == _T('\0')) {
                continue;
            } else if (argv[iarg][1] == _T('-')) {
                option_name = &argv[iarg][2];
            }
        }
        if (option_name != nullptr
            && tstring(option_name) == _T("option-file")) {
            if (iarg + 1 >= argc) {
                _ftprintf(stderr, _T("option file name is not specified.\n"));
                return -1;
            }
            tstring cnffile = argv[iarg + 1];
            vector_cat(argvCnfFile, cmd_from_config_file(argv[iarg + 1]));
        }
    }

    VCEParam prm;
    std::vector<const TCHAR *> argvCopy(argv, argv + argc);
    //optionファイルのパラメータを追加
    for (size_t i = 0; i < argvCnfFile.size(); i++) {
        if (argvCnfFile[i].length() > 0) {
            argvCopy.push_back(argvCnfFile[i].c_str());
        }
    }
    argvCopy.push_back(_T(""));
    if (parse_cmd(&prm, (int)argvCopy.size()-1, argvCopy.data())) {
        return 1;
    }

    if (vce_run(&prm)) {
        fprintf(stderr, "Finished with error in VCEEncC.\n");
        return 1;
    }
    return 0;
}
