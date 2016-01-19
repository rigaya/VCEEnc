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

#pragma once

#include <string>
#include <algorithm>
#include <vector>
#include <type_traits>
#include <tchar.h>
#include <cstdint>
#include <memory>

enum VCELogLevel {
    VCE_LOG_TRACE = -3,
    VCE_LOG_DEBUG = -2,
    VCE_LOG_MORE = -1,
    VCE_LOG_INFO = 0,
    VCE_LOG_WARN,
    VCE_LOG_ERROR
};

using std::shared_ptr;
using std::unique_ptr;
using std::vector;

#ifndef MIN3
#define MIN3(a,b,c) (min((a), min((b), (c))))
#endif
#ifndef MAX3
#define MAX3(a,b,c) (max((a), max((b), (c))))
#endif

#ifndef clamp
#define clamp(x, low, high) (((x) <= (high)) ? (((x) >= (low)) ? (x) : (low)) : (high))
#endif

#if UNICODE
#define to_tstring to_wstring
#else
#define to_tstring to_string
#endif

typedef std::basic_string<TCHAR> tstring;
typedef std::basic_stringstream<TCHAR> TStringStream;

unsigned int wstring_to_string(const WCHAR *wstr, std::string& str, DWORD codepage = CP_THREAD_ACP);
std::string wstring_to_string(const WCHAR *wstr, DWORD codepage = CP_THREAD_ACP);
std::string wstring_to_string(const std::wstring& wstr, DWORD codepage = CP_THREAD_ACP);
unsigned int tchar_to_string(const TCHAR *tstr, std::string& str, DWORD codepage = CP_THREAD_ACP);
std::string tchar_to_string(const TCHAR *tstr, DWORD codepage = CP_THREAD_ACP);
std::string tchar_to_string(const tstring& tstr, DWORD codepage = CP_THREAD_ACP);
unsigned int char_to_tstring(tstring& tstr, const char *str, DWORD codepage = CP_THREAD_ACP);
tstring char_to_tstring(const char *str, DWORD codepage = CP_THREAD_ACP);
tstring char_to_tstring(const std::string& str, DWORD codepage = CP_THREAD_ACP);
unsigned int char_to_wstring(std::wstring& wstr, const char *str, DWORD codepage = CP_THREAD_ACP);
std::wstring char_to_wstring(const char *str, DWORD codepage = CP_THREAD_ACP);
std::wstring char_to_wstring(const std::string& str, DWORD codepage = CP_THREAD_ACP);
std::wstring tchar_to_wstring(const TCHAR *str, DWORD codepage = CP_THREAD_ACP);
std::wstring tchar_to_wstring(const tstring& str, DWORD codepage = CP_THREAD_ACP);
tstring wchar_to_tstring(const wchar_t *str, DWORD codepage = CP_THREAD_ACP);
tstring wchar_to_tstring(const std::wstring& str, DWORD codepage = CP_THREAD_ACP);
std::string strsprintf(const char* format, ...);
std::wstring strsprintf(const WCHAR* format, ...);
std::vector<tstring> split(const tstring &str, const tstring &delim);
std::vector<std::string> split(const std::string &str, const std::string &delim);
std::string str_replace(std::string str, const std::string& from, const std::string& to);
std::wstring str_replace(std::wstring str, const std::wstring& from, const std::wstring& to);
std::string GetFullPath(const char *path);
std::wstring GetFullPath(const WCHAR *path);

template<typename T>
static inline T vce_gcd(T a, T b) {
    static_assert(std::is_integral<T>::value, "vce_gcd is defined only for integer.");
    if (a == 0) return b;
    if (b == 0) return a;
    T c;
    while ((c = a % b) != 0)
        a = b, b = c;
    return b;
}

struct aligned_malloc_deleter {
    void operator()(void* ptr) const {
        _aligned_free(ptr);
    }
};
struct file_deleter {
    void operator()(FILE* fp) const {
        if (fp) {
            fclose(fp);
        }
    }
};

static inline uint32_t check_range_unsigned(uint32_t value, uint32_t min, uint32_t max) {
    return (value - min) <= (max - min);
}

static int popcnt32(uint32_t bits) {
    bits = (bits & 0x55555555) + (bits >> 1 & 0x55555555);
    bits = (bits & 0x33333333) + (bits >> 2 & 0x33333333);
    bits = (bits & 0x0f0f0f0f) + (bits >> 4 & 0x0f0f0f0f);
    bits = (bits & 0x00ff00ff) + (bits >> 8 & 0x00ff00ff);
    return (bits & 0x0000ffff) + (bits >>16 & 0x0000ffff);
}

static TCHAR *alloc_str(const TCHAR *str, size_t length = 0) {
    const size_t count = (length) ? length : _tcslen(str);
    TCHAR *ptr = (TCHAR *)calloc(count + 1, sizeof(str[0]));
    memcpy(ptr, str, sizeof(str[0]) * count);
    return ptr;
}

template<typename type>
static std::basic_string<type> repeatStr(std::basic_string<type> str, int count) {
    std::basic_string<type> ret;
    for (int i = 0; i < count; i++) {
        ret += str;
    }
    return ret;
}

bool check_ext(const TCHAR *filename, const std::vector<const char*>& ext_list);

const TCHAR *getOSVersion();
BOOL is_64bit_os();

int getCPUInfo(TCHAR *buffer, size_t nSize);
int getGPUInfo(const char *VendorName, TCHAR *buffer, unsigned int buffer_size, bool driver_version_only = false);

int vce_print_stderr(int log_level, const TCHAR *mes, HANDLE handle);
