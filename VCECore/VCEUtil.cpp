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

#include <tchar.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <intrin.h>

#include "VCEUtil.h"
#include "VCEParam.h"

#pragma warning (push)
#pragma warning (disable: 4100)
unsigned int wstring_to_string(const WCHAR *wstr, std::string& str, DWORD codepage) {
    DWORD flags = (codepage == CP_UTF8) ? 0 : WC_NO_BEST_FIT_CHARS;
    int multibyte_length = WideCharToMultiByte(codepage, flags, wstr, -1, nullptr, 0, nullptr, nullptr);
    str.resize(multibyte_length, 0);
    if (0 == WideCharToMultiByte(codepage, flags, wstr, -1, &str[0], multibyte_length, nullptr, nullptr)) {
        str.clear();
        return 0;
    }
    return multibyte_length;
}

std::string wstring_to_string(const WCHAR *wstr, DWORD codepage) {
    std::string str;
    wstring_to_string(wstr, str, codepage);
    return str;
}

std::string wstring_to_string(const std::wstring& wstr, DWORD codepage) {
    std::string str;
    wstring_to_string(wstr.c_str(), str, codepage);
    return str;
}

unsigned int tchar_to_string(const TCHAR *tstr, std::string& str, DWORD codepage) {
#if UNICODE
    return wstring_to_string(tstr, str, codepage);
#else
    str = std::string(tstr);
    return (unsigned int)str.length();
#endif
}

unsigned int tchar_to_string(const tstring& tstr, std::string& str, DWORD codepage) {
    return tchar_to_string(tstr.c_str(), str, codepage);
}

std::string tchar_to_string(const TCHAR *tstr, DWORD codepage) {
    std::string str;
    tchar_to_string(tstr, str, codepage);
    return str;
}

std::string tchar_to_string(const tstring& tstr, DWORD codepage) {
    std::string str;
    tchar_to_string(tstr.c_str(), str, codepage);
    return str;
}

unsigned int char_to_wstring(std::wstring& wstr, const char *str, DWORD codepage) {
    int widechar_length = MultiByteToWideChar(codepage, 0, str, -1, nullptr, 0);
    wstr.resize(widechar_length, 0);
    if (0 == MultiByteToWideChar(codepage, 0, str, -1, &wstr[0], (int)wstr.size())) {
        wstr.clear();
        return 0;
    }
    return widechar_length;
}

std::wstring char_to_wstring(const char *str, DWORD codepage) {
    std::wstring wstr;
    char_to_wstring(wstr, str, codepage);
    return wstr;
}
std::wstring char_to_wstring(const std::string& str, DWORD codepage) {
    std::wstring wstr;
    char_to_wstring(wstr, str.c_str(), codepage);
    return wstr;
}

std::wstring tchar_to_wstring(const TCHAR *str, DWORD codepage) {
#if UNICODE
    return std::wstring(str);
#else
    return char_to_wstring(str);
#endif
}
std::wstring tchar_to_wstring(const tstring& str, DWORD codepage) {
#if UNICODE
    return str;
#else
    return char_to_wstring(str);
#endif
}

unsigned int wstring_to_tstring(const WCHAR *wstr, tstring& tstr, uint32_t codepage) {
    if (wstr == nullptr) {
        tstr = _T("");
        return 0;
    }
#if UNICODE
    tstr = std::wstring(wstr);
#else
    return wstring_to_string(wstr, tstr, codepage);
#endif
    return (unsigned int)tstr.length();
}

tstring wstring_to_tstring(const WCHAR *wstr, uint32_t codepage) {
    if (wstr == nullptr) {
        return _T("");
    }
    tstring tstr;
    wstring_to_tstring(wstr, tstr, codepage);
    return tstr;
}

tstring wstring_to_tstring(const std::wstring& wstr, uint32_t codepage) {
    tstring tstr;
    wstring_to_tstring(wstr.c_str(), tstr, codepage);
    return tstr;
}

unsigned int char_to_tstring(tstring& tstr, const char *str, DWORD codepage) {
#if UNICODE
    return char_to_wstring(tstr, str, codepage);
#else
    tstr = std::string(str);
    return (unsigned int)tstr.length();
#endif
}

tstring char_to_tstring(const char *str, DWORD codepage) {
    tstring tstr;
    char_to_tstring(tstr, str, codepage);
    return tstr;
}
tstring char_to_tstring(const std::string& str, DWORD codepage) {
    tstring tstr;
    char_to_tstring(tstr, str.c_str(), codepage);
    return tstr;
}
std::string strsprintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    const size_t len = _vscprintf(format, args) + 1;

    std::vector<char> buffer(len, 0);
    vsprintf_s(buffer.data(), len, format, args);
    va_end(args);
    std::string retStr = std::string(buffer.data());
    return retStr;
}
std::wstring strsprintf(const WCHAR* format, ...) {
    va_list args;
    va_start(args, format);
    const size_t len = _vscwprintf(format, args) + 1;

    std::vector<WCHAR> buffer(len, 0);
    vswprintf_s(buffer.data(), len, format, args);
    va_end(args);
    std::wstring retStr = std::wstring(buffer.data());
    return retStr;
}

std::string str_replace(std::string str, const std::string& from, const std::string& to) {
    std::string::size_type pos = 0;
    while (pos = str.find(from, pos), pos != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
    return std::move(str);
}

std::wstring str_replace(std::wstring str, const std::wstring& from, const std::wstring& to) {
    std::wstring::size_type pos = 0;
    while (pos = str.find(from, pos), pos != std::wstring::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
    return std::move(str);
}

#pragma warning (pop)

#if defined(_WIN32) || defined(_WIN64)
std::vector<std::wstring> split(const std::wstring &str, const std::wstring &delim, bool bTrim) {
    std::vector<std::wstring> res;
    size_t current = 0, found, delimlen = delim.size();
    while (std::wstring::npos != (found = str.find(delim, current))) {
        auto segment = std::wstring(str, current, found - current);
        if (bTrim) {
            segment = trim(segment);
        }
        if (!bTrim || segment.length()) {
            res.push_back(segment);
        }
        current = found + delimlen;
    }
    auto segment = std::wstring(str, current, str.size() - current);
    if (bTrim) {
        segment = trim(segment);
    }
    if (!bTrim || segment.length()) {
        res.push_back(std::wstring(segment.c_str()));
    }
    return res;
}
#endif //#if defined(_WIN32) || defined(_WIN64)

std::vector<std::string> split(const std::string &str, const std::string &delim, bool bTrim) {
    std::vector<std::string> res;
    size_t current = 0, found, delimlen = delim.size();
    while (std::string::npos != (found = str.find(delim, current))) {
        auto segment = std::string(str, current, found - current);
        if (bTrim) {
            segment = trim(segment);
        }
        if (!bTrim || segment.length()) {
            res.push_back(segment);
        }
        current = found + delimlen;
    }
    auto segment = std::string(str, current, str.size() - current);
    if (bTrim) {
        segment = trim(segment);
    }
    if (!bTrim || segment.length()) {
        res.push_back(std::string(segment.c_str()));
    }
    return res;
}

std::string lstrip(const std::string& string, const char* trim) {
    auto result = string;
    auto left = string.find_first_not_of(trim);
    if (left != std::string::npos) {
        result = string.substr(left, 0);
    }
    return result;
}

std::string rstrip(const std::string& string, const char* trim) {
    auto result = string;
    auto right = string.find_last_not_of(trim);
    if (right != std::string::npos) {
        result = string.substr(0, right);
    }
    return result;
}

std::string trim(const std::string& string, const char* trim) {
    auto result = string;
    auto left = string.find_first_not_of(trim);
    if (left != std::string::npos) {
        auto right = string.find_last_not_of(trim);
        result = string.substr(left, right - left + 1);
    }
    return result;
}

std::wstring lstrip(const std::wstring& string, const WCHAR* trim) {
    auto result = string;
    auto left = string.find_first_not_of(trim);
    if (left != std::string::npos) {
        result = string.substr(left, 0);
    }
    return result;
}

std::wstring rstrip(const std::wstring& string, const WCHAR* trim) {
    auto result = string;
    auto right = string.find_last_not_of(trim);
    if (right != std::string::npos) {
        result = string.substr(0, right);
    }
    return result;
}

std::wstring trim(const std::wstring& string, const WCHAR* trim) {
    auto result = string;
    auto left = string.find_first_not_of(trim);
    if (left != std::string::npos) {
        auto right = string.find_last_not_of(trim);
        result = string.substr(left, right - left + 1);
    }
    return result;
}

std::string GetFullPath(const char *path) {
#if defined(_WIN32) || defined(_WIN64)
    if (PathIsRelativeA(path) == FALSE)
        return std::string(path);
#endif //#if defined(_WIN32) || defined(_WIN64)
    std::vector<char> buffer(strlen(path) + 1024, 0);
    _fullpath(buffer.data(), path, buffer.size());
    return std::string(buffer.data());
}
#if defined(_WIN32) || defined(_WIN64)
std::wstring GetFullPath(const WCHAR *path) {
    if (PathIsRelativeW(path) == FALSE)
        return std::wstring(path);

    std::vector<WCHAR> buffer(wcslen(path) + 1024, 0);
    _wfullpath(buffer.data(), path, buffer.size());
    return std::wstring(buffer.data());
}
//ルートディレクトリを取得
std::string PathGetRoot(const char *path) {
    auto fullpath = GetFullPath(path);
    std::vector<char> buffer(fullpath.length() + 1, 0);
    memcpy(buffer.data(), fullpath.c_str(), fullpath.length() * sizeof(fullpath[0]));
    PathStripToRootA(buffer.data());
    return buffer.data();
}
std::wstring PathGetRoot(const WCHAR *path) {
    auto fullpath = GetFullPath(path);
    std::vector<WCHAR> buffer(fullpath.length() + 1, 0);
    memcpy(buffer.data(), fullpath.c_str(), fullpath.length() * sizeof(fullpath[0]));
    PathStripToRootW(buffer.data());
    return buffer.data();
}

//パスのルートが存在するかどうか
static bool PathRootExists(const char *path) {
    if (path == nullptr)
        return false;
    return PathIsDirectoryA(PathGetRoot(path).c_str()) != 0;
}
static bool PathRootExists(const WCHAR *path) {
    if (path == nullptr)
        return false;
    return PathIsDirectoryW(PathGetRoot(path).c_str()) != 0;
}
#endif //#if defined(_WIN32) || defined(_WIN64)
std::pair<int, std::string> PathRemoveFileSpecFixed(const std::string& path) {
    const char *ptr = path.c_str();
    const char *qtr = PathFindFileNameA(ptr);
    if (qtr == ptr) {
        return std::make_pair(0, path);
    }
    std::string newPath = path.substr(0, qtr - ptr - 1);
    return std::make_pair((int)(path.length() - newPath.length()), newPath);
}
#if defined(_WIN32) || defined(_WIN64)
std::pair<int, std::wstring> PathRemoveFileSpecFixed(const std::wstring& path) {
    const WCHAR *ptr = path.c_str();
    WCHAR *qtr = PathFindFileNameW(ptr);
    if (qtr == ptr) {
        return std::make_pair(0, path);
    }
    std::wstring newPath = path.substr(0, qtr - ptr - 1);
    return std::make_pair((int)(path.length() - newPath.length()), newPath);
}
std::string PathCombineS(const std::string& dir, const std::string& filename) {
    std::vector<char> buffer(dir.length() + filename.length() + 128, '\0');
    PathCombineA(buffer.data(), dir.c_str(), filename.c_str());
    return std::string(buffer.data());
}
std::wstring PathCombineS(const std::wstring& dir, const std::wstring& filename) {
    std::vector<WCHAR> buffer(dir.length() + filename.length() + 128, '\0');
    PathCombineW(buffer.data(), dir.c_str(), filename.c_str());
    return std::wstring(buffer.data());
}
#endif //#if defined(_WIN32) || defined(_WIN64)
//フォルダがあればOK、なければ作成する
bool CreateDirectoryRecursive(const char *dir) {
    if (PathIsDirectoryA(dir)) {
        return true;
    }
#if defined(_WIN32) || defined(_WIN64)
    if (!PathRootExists(dir)) {
        return false;
    }
#endif //#if defined(_WIN32) || defined(_WIN64)
    auto ret = PathRemoveFileSpecFixed(dir);
    if (ret.first == 0) {
        return false;
    }
    if (!CreateDirectoryRecursive(ret.second.c_str())) {
        return false;
    }
    return CreateDirectoryA(dir, NULL) != 0;
}
#if defined(_WIN32) || defined(_WIN64)
bool CreateDirectoryRecursive(const WCHAR *dir) {
    if (PathIsDirectoryW(dir)) {
        return true;
    }
    if (!PathRootExists(dir)) {
        return false;
    }
    auto ret = PathRemoveFileSpecFixed(dir);
    if (ret.first == 0) {
        return false;
    }
    if (!CreateDirectoryRecursive(ret.second.c_str())) {
        return false;
    }
    return CreateDirectoryW(dir, NULL) != 0;
}
#endif //#if defined(_WIN32) || defined(_WIN64)

bool check_ext(const TCHAR *filename, const std::vector<const char*>& ext_list) {
    const TCHAR *target = PathFindExtension(filename);
    if (target) {
        for (auto ext : ext_list) {
            if (0 == _tcsicmp(target, char_to_tstring(ext).c_str())) {
                return true;
            }
        }
    }
    return false;
}

bool vce_get_filesize(const char *filepath, uint64_t *filesize) {
#if defined(_WIN32) || defined(_WIN64)
    WIN32_FILE_ATTRIBUTE_DATA fd = { 0 };
    bool ret = (GetFileAttributesExA(filepath, GetFileExInfoStandard, &fd)) ? true : false;
    *filesize = (ret) ? (((UINT64)fd.nFileSizeHigh) << 32) + (UINT64)fd.nFileSizeLow : NULL;
    return ret;
#else //#if defined(_WIN32) || defined(_WIN64)
    struct stat stat;
    FILE *fp = fopen(filepath, "rb");
    if (fp == NULL || fstat(fileno(fp), &stat)) {
        *filesize = 0;
        return 1;
    }
    if (fp) {
        fclose(fp);
    }
    *filesize = stat.st_size;
    return 0;
#endif //#if defined(_WIN32) || defined(_WIN64)
}

#if defined(_WIN32) || defined(_WIN64)
bool vce_get_filesize(const WCHAR *filepath, UINT64 *filesize) {
    WIN32_FILE_ATTRIBUTE_DATA fd = { 0 };
    bool ret = (GetFileAttributesExW(filepath, GetFileExInfoStandard, &fd)) ? true : false;
    *filesize = (ret) ? (((UINT64)fd.nFileSizeHigh) << 32) + (UINT64)fd.nFileSizeLow : NULL;
    return ret;
}
#endif //#if defined(_WIN32) || defined(_WIN64)

tstring print_time(double time) {
    int sec = (int)time;
    time -= sec;
    int miniute = (int)(sec / 60);
    sec -= miniute * 60;
    int hour = miniute / 60;
    miniute -= hour * 60;
    tstring frac = strsprintf(_T("%.3f"), time);
    return strsprintf(_T("%d:%02d:%02d%s"), hour, miniute, sec, frac.substr(frac.find_first_of(_T("."))).c_str());
}

#include "cl_func.h"

#if ENABLE_OPENCL
static bool get_gpu_frequency(cl_device_id device_id, uint32_t *gpu_freq) {
    size_t size;
    return 0 != clGetDeviceInfo(device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(gpu_freq[0]), gpu_freq, &size);
}
#endif //ENABLE_OPENCL

int vce_print_stderr(int log_level, const TCHAR *mes, HANDLE handle) {
    CONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };
    static const WORD LOG_COLOR[] = {
        FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE, //水色
        FOREGROUND_INTENSITY | FOREGROUND_GREEN, //緑
        FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED, //黄色
        FOREGROUND_INTENSITY | FOREGROUND_RED //赤
    };
    if (handle == NULL) {
        handle = GetStdHandle(STD_ERROR_HANDLE);
    }
    if (handle && log_level != VCE_LOG_INFO) {
        GetConsoleScreenBufferInfo(handle, &csbi);
        SetConsoleTextAttribute(handle, LOG_COLOR[clamp(log_level, VCE_LOG_TRACE, VCE_LOG_ERROR) - VCE_LOG_TRACE] | (csbi.wAttributes & 0x00f0));
    }
    //このfprintfで"%"が消えてしまわないよう置換する
    int ret = _ftprintf(stderr, (nullptr == _tcschr(mes, _T('%'))) ? mes : str_replace(tstring(mes), _T("%"), _T("%%")).c_str());
    fflush(stderr);
    if (handle && log_level != VCE_LOG_INFO) {
        SetConsoleTextAttribute(handle, csbi.wAttributes); //元に戻す
    }
    return ret;
}

size_t malloc_degeneracy(void **ptr, size_t nSize, size_t nMinSize) {
    *ptr = nullptr;
    nMinSize = (std::max<size_t>)(nMinSize, 1);
    //確保できなかったら、サイズを小さくして再度確保を試みる (最終的に1MBも確保できなかったら諦める)
    while (nSize >= nMinSize) {
        void *qtr = malloc(nSize);
        if (qtr != nullptr) {
            *ptr = qtr;
            return nSize;
        }
        size_t nNextSize = 0;
        for (size_t i = nMinSize; i < nSize; i<<=1) {
            nNextSize = i;
        }
        nSize = nNextSize;
    }
    return 0;
}

BOOL is_64bit_os() {
    SYSTEM_INFO sinfo = { 0 };
    GetNativeSystemInfo(&sinfo);
    return sinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;
}

typedef void (WINAPI *RtlGetVersion_FUNC)(OSVERSIONINFOEXW*);

static int getRealWindowsVersion(OSVERSIONINFOEXW *osinfo) {
    OSVERSIONINFOEXW osver = { 0 };
    osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    HMODULE hModule = NULL;
    RtlGetVersion_FUNC func = NULL;
    int ret = 1;
    if (   NULL != (hModule = LoadLibrary(_T("ntdll.dll")))
        && NULL != (func = (RtlGetVersion_FUNC)GetProcAddress(hModule, "RtlGetVersion"))) {
        func(&osver);
        memcpy(osinfo, &osver, sizeof(osver));
        ret = 0;
    }
    if (hModule) {
        FreeLibrary(hModule);
    }
    return ret;
}

tstring getOSVersion(OSVERSIONINFOEXW *osinfo) {
    const TCHAR *ptr = _T("Unknown");
    OSVERSIONINFOW info ={ 0 };
    OSVERSIONINFOEXW infoex ={ 0 };
    info.dwOSVersionInfoSize = sizeof(info);
    infoex.dwOSVersionInfoSize = sizeof(infoex);
    GetVersionExW(&info);
    switch (info.dwPlatformId) {
    case VER_PLATFORM_WIN32_WINDOWS:
        if (4 <= info.dwMajorVersion) {
            switch (info.dwMinorVersion) {
            case 0:  ptr = _T("Windows 95"); break;
            case 10: ptr = _T("Windows 98"); break;
            case 90: ptr = _T("Windows Me"); break;
            default: break;
            }
        }
        break;
    case VER_PLATFORM_WIN32_NT:
        if (info.dwMajorVersion >= 6 || (info.dwMajorVersion == 5 && info.dwMinorVersion >= 2)) {
            GetVersionExW((OSVERSIONINFOW *)&infoex);
        } else {
            memcpy(&infoex, &info, sizeof(info));
        }
        if (info.dwMajorVersion == 6) {
            getRealWindowsVersion(&infoex);
        }
        if (osinfo) {
            memcpy(osinfo, &infoex, sizeof(infoex));
        }
        switch (infoex.dwMajorVersion) {
        case 3:
            switch (infoex.dwMinorVersion) {
            case 0:  ptr = _T("Windows NT 3"); break;
            case 1:  ptr = _T("Windows NT 3.1"); break;
            case 5:  ptr = _T("Windows NT 3.5"); break;
            case 51: ptr = _T("Windows NT 3.51"); break;
            default: break;
            }
            break;
        case 4:
            if (0 == infoex.dwMinorVersion)
                ptr = _T("Windows NT 4.0");
            break;
        case 5:
            switch (infoex.dwMinorVersion) {
            case 0:  ptr = _T("Windows 2000"); break;
            case 1:  ptr = _T("Windows XP"); break;
            case 2:  ptr = _T("Windows Server 2003"); break;
            default: break;
            }
            break;
        case 6:
            switch (infoex.dwMinorVersion) {
            case 0:  ptr = (infoex.wProductType == VER_NT_WORKSTATION) ? _T("Windows Vista") : _T("Windows Server 2008");    break;
            case 1:  ptr = (infoex.wProductType == VER_NT_WORKSTATION) ? _T("Windows 7")     : _T("Windows Server 2008 R2"); break;
            case 2:  ptr = (infoex.wProductType == VER_NT_WORKSTATION) ? _T("Windows 8")     : _T("Windows Server 2012");    break;
            case 3:  ptr = (infoex.wProductType == VER_NT_WORKSTATION) ? _T("Windows 8.1")   : _T("Windows Server 2012 R2"); break;
            case 4:  ptr = (infoex.wProductType == VER_NT_WORKSTATION) ? _T("Windows 10")    : _T("Windows Server 2016");    break;
            default:
                if (5 <= infoex.dwMinorVersion) {
                    ptr = _T("Later than Windows 10");
                }
                break;
            }
            break;
        case 10:
            ptr = (infoex.wProductType == VER_NT_WORKSTATION) ? _T("Windows 10") : _T("Windows Server 2016"); break;
        default:
            if (10 <= infoex.dwMajorVersion) {
                ptr = _T("Later than Windows 10");
            }
            break;
        }
        break;
    default:
        break;
    }
    return tstring(ptr);
}

int bitstreamInit(sBitstream *pBitstream, uint32_t nSize) {
    bitstreamClear(pBitstream);

    if (nullptr == (pBitstream->Data = (uint8_t *)_aligned_malloc(nSize, 32))) {
        return 1;
    }

    pBitstream->MaxLength = nSize;
    return 0;
}

int bitstreamCopy(sBitstream *pBitstreamCopy, const sBitstream *pBitstream) {
    memcpy(pBitstreamCopy, pBitstream, sizeof(pBitstreamCopy[0]));
    pBitstreamCopy->Data = nullptr;
    pBitstreamCopy->DataLength = 0;
    pBitstreamCopy->DataOffset = 0;
    pBitstreamCopy->MaxLength = 0;
    auto sts = bitstreamInit(pBitstreamCopy, pBitstream->MaxLength);
    if (sts == 0) {
        memcpy(pBitstreamCopy->Data, pBitstream->Data, pBitstreamCopy->DataLength);
    }
    return sts;
}

int bitstreamExtend(sBitstream *pBitstream, uint32_t nSize) {
    uint8_t *pData = (uint8_t *)_aligned_malloc(nSize, 32);
    if (nullptr == pData) {
        return 1;
    }

    auto nDataLen = pBitstream->DataLength;
    if (nDataLen) {
        memmove(pData, pBitstream->Data + pBitstream->DataOffset, nDataLen);
    }

    bitstreamClear(pBitstream);

    pBitstream->Data       = pData;
    pBitstream->DataOffset = 0;
    pBitstream->DataLength = nDataLen;
    pBitstream->MaxLength  = nSize;

    return 0;
}

void bitstreamClear(sBitstream *pBitstream) {
    if (pBitstream->Data) {
        _aligned_free(pBitstream->Data);
    }
    memset(pBitstream, 0, sizeof(pBitstream[0]));
}

int bitstreamAppend(sBitstream *pBitstream, const uint8_t *data, uint32_t size) {
    int sts = 0;
    if (data) {
        const uint32_t new_data_length = pBitstream->DataLength + size;
        if (pBitstream->MaxLength < new_data_length) {
            if (0 != (sts = bitstreamExtend(pBitstream, new_data_length))) {
                return sts;
            }
        }

        if (pBitstream->MaxLength < new_data_length + pBitstream->DataOffset) {
            memmove(pBitstream->Data, pBitstream->Data + pBitstream->DataOffset, pBitstream->DataLength);
            pBitstream->DataOffset = 0;
        }
        memcpy(pBitstream->Data + pBitstream->DataLength + pBitstream->DataOffset, data, size);
        pBitstream->DataLength = new_data_length;
    }
    return sts;
}

const TCHAR *CodecIdToStr(uint32_t codecId) {
    switch (codecId) {
    case VCE_CODEC_H264:  return _T("H.264/AVC");
    case VCE_CODEC_MPEG2: return _T("MPEG2");
    case VCE_CODEC_VC1:   return _T("VC-1");
    case VCE_CODEC_WMV3:  return _T("WMV3");
    default:              return _T("Unknown");
    }
}
