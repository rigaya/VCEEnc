//  -----------------------------------------------------------------------------------------
//    VCEEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include <intrin.h>

#include "VCEUtil.h"

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

tstring wchar_to_tstring(const wchar_t *str, DWORD codepage) {
#if UNICODE
    return std::wstring(str);
#else
    return wstring_to_string(str);
#endif
}
tstring wchar_to_tstring(const std::wstring& str, DWORD codepage) {
#if UNICODE
    return str;
#else
    return wstring_to_string(str);
#endif
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

std::vector<std::wstring> split(const std::wstring &str, const std::wstring &delim) {
    std::vector<std::wstring> res;
    size_t current = 0, found, delimlen = delim.size();
    while (std::wstring::npos != (found = str.find(delim, current))) {
        res.push_back(std::wstring(str, current, found - current));
        current = found + delimlen;
    }
    res.push_back(std::wstring(str, current, str.size() - current));
    return res;
}

std::vector<std::string> split(const std::string &str, const std::string &delim) {
    std::vector<std::string> res;
    size_t current = 0, found, delimlen = delim.size();
    while (std::string::npos != (found = str.find(delim, current))) {
        res.push_back(std::string(str, current, found - current));
        current = found + delimlen;
    }
    res.push_back(std::string(str, current, str.size() - current));
    return res;
}

std::string GetFullPath(const char *path) {
    if (PathIsRelativeA(path) == FALSE)
        return std::string(path);

    std::vector<char> buffer(strlen(path) + 1024, 0);
    _fullpath(buffer.data(), path, buffer.size());
    return std::string(buffer.data());
}
std::wstring GetFullPath(const WCHAR *path) {
    if (PathIsRelativeW(path) == FALSE)
        return std::wstring(path);

    std::vector<WCHAR> buffer(wcslen(path) + 1024, 0);
    _wfullpath(buffer.data(), path, buffer.size());
    return std::wstring(buffer.data());
}

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

#include "cl_func.h"

static bool get_gpu_frequency(cl_device_id device_id, uint32_t *gpu_freq) {
    size_t size;
    return 0 != clGetDeviceInfo(device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(gpu_freq[0]), gpu_freq, &size);
}

int vce_print_stderr(int log_level, const TCHAR *mes, HANDLE handle) {
    CONSOLE_SCREEN_BUFFER_INFO csbi ={ 0 };
    static const WORD LOG_COLOR[] ={
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
    int ret = _ftprintf(stderr, mes);
    fflush(stderr);
    if (handle && log_level != VCE_LOG_INFO) {
        SetConsoleTextAttribute(handle, csbi.wAttributes); //元に戻す
    }
    return ret;
}

#include <Windows.h>

BOOL is_64bit_os() {
    SYSTEM_INFO sinfo = { 0 };
    GetNativeSystemInfo(&sinfo);
    return sinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;
}

typedef void (WINAPI *RtlGetVersion_FUNC)(OSVERSIONINFOEXW*);

static int getRealWindowsVersion(DWORD *major, DWORD *minor) {
    *major = 0;
    *minor = 0;
    OSVERSIONINFOEXW osver;
    HMODULE hModule = NULL;
    RtlGetVersion_FUNC func = NULL;
    int ret = 1;
    if (NULL != (hModule = LoadLibrary(_T("ntdll.dll")))
        && NULL != (func = (RtlGetVersion_FUNC)GetProcAddress(hModule, "RtlGetVersion"))) {
        func(&osver);
        *major = osver.dwMajorVersion;
        *minor = osver.dwMinorVersion;
        ret = 0;
    }
    if (hModule) {
        FreeLibrary(hModule);
    }
    return ret;
}

const TCHAR *getOSVersion() {
    const TCHAR *ptr = _T("Unknown");
    OSVERSIONINFO info ={ 0 };
    info.dwOSVersionInfoSize = sizeof(info);
    GetVersionEx(&info);
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
        if (info.dwMajorVersion == 6) {
            getRealWindowsVersion(&info.dwMajorVersion, &info.dwMinorVersion);
        }
        switch (info.dwMajorVersion) {
        case 3:
            switch (info.dwMinorVersion) {
            case 0:  ptr = _T("Windows NT 3"); break;
            case 1:  ptr = _T("Windows NT 3.1"); break;
            case 5:  ptr = _T("Windows NT 3.5"); break;
            case 51: ptr = _T("Windows NT 3.51"); break;
            default: break;
            }
            break;
        case 4:
            if (0 == info.dwMinorVersion)
                ptr = _T("Windows NT 4.0");
            break;
        case 5:
            switch (info.dwMinorVersion) {
            case 0:  ptr = _T("Windows 2000"); break;
            case 1:  ptr = _T("Windows XP"); break;
            case 2:  ptr = _T("Windows Server 2003"); break;
            default: break;
            }
            break;
        case 6:
            switch (info.dwMinorVersion) {
            case 0:  ptr = _T("Windows Vista"); break;
            case 1:  ptr = _T("Windows 7"); break;
            case 2:  ptr = _T("Windows 8"); break;
            case 3:  ptr = _T("Windows 8.1"); break;
            case 4:  ptr = _T("Windows 10"); break;
            default:
                if (5 <= info.dwMinorVersion) {
                    ptr = _T("Later than Windows 10");
                }
                break;
            }
            break;
        case 10:
            ptr = _T("Windows 10");
            break;
        default:
            if (10 <= info.dwMajorVersion) {
                ptr = _T("Later than Windows 10");
            }
            break;
        }
        break;
    default:
        break;
    }
    return ptr;
}
