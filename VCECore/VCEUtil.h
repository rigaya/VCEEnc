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

#pragma once

#include <string>
#include <algorithm>
#include <vector>
#include <array>
#include <type_traits>
#include <tchar.h>
#include <cstdint>
#include <memory>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

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
#define MIN3(a,b,c) ((std::min)((a), (std::min)((b), (c))))
#endif
#ifndef MAX3
#define MAX3(a,b,c) ((std::max)((a), (std::max)((b), (c))))
#endif

#ifndef clamp
#define clamp(x, low, high) (((x) <= (high)) ? (((x) >= (low)) ? (x) : (low)) : (high))
#endif

#define ALIGN(x,align) (((x)+((align)-1))&(~((align)-1)))
#define ALIGN16(x) (((x)+15)&(~15))
#define ALIGN32(x) (((x)+31)&(~31))

template<typename T, size_t size>
std::vector<T> make_vector(T(&ptr)[size]) {
    return std::vector<T>(ptr, ptr + size);
}
template<typename T, size_t size>
std::vector<T> make_vector(const T(&ptr)[size]) {
    return std::vector<T>(ptr, ptr + size);
}
template<typename T0, typename T1>
std::vector<T0> make_vector(const T0 *ptr, T1 size) {
    static_assert(std::is_integral<T1>::value == true, "T1 should be integral");
    return (ptr && size) ? std::vector<T0>(ptr, ptr + size) : std::vector<T0>();
}
template<typename T0, typename T1>
std::vector<T0> make_vector(T0 *ptr, T1 size) {
    static_assert(std::is_integral<T1>::value == true, "T1 should be integral");
    return (ptr && size) ? std::vector<T0>(ptr, ptr + size) : std::vector<T0>();
}
template<typename T, typename ...Args>
constexpr std::array<T, sizeof...(Args)> make_array(Args&&... args) {
    return std::array<T, sizeof...(Args)>{ static_cast<Args&&>(args)... };
}
template<typename T, std::size_t N>
constexpr std::size_t array_size(const std::array<T, N>&) {
    return N;
}
template<typename T, std::size_t N>
constexpr std::size_t array_size(T(&)[N]) {
    return N;
}
template<typename T>
void vector_cat(vector<T>& v1, const vector<T>& v2) {
    if (v2.size()) {
        v1.insert(v1.end(), v2.begin(), v2.end());
    }
}
template<typename T>
static void vce_free(T& ptr) {
    static_assert(std::is_pointer<T>::value == true, "T should be pointer");
    if (ptr) {
        free(ptr);
        ptr = nullptr;
    }
}

struct aligned_malloc_deleter {
    void operator()(void* ptr) const {
        _aligned_free(ptr);
    }
};

struct malloc_deleter {
    void operator()(void* ptr) const {
        free(ptr);
    }
};

struct fp_deleter {
    void operator()(FILE* fp) const {
        if (fp) {
            fflush(fp);
            fclose(fp);
        }
    }
};

struct handle_deleter {
    void operator()(HANDLE handle) const {
        if (handle) {
#if defined(_WIN32) || defined(_WIN64)
            CloseHandle(handle);
#endif //#if defined(_WIN32) || defined(_WIN64)
        }
    }
};

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
unsigned int tchar_to_string(const tstring& tstr, std::string& str, DWORD codepage = CP_THREAD_ACP);
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
unsigned int wstring_to_tstring(const WCHAR *wstr, tstring& tstr, uint32_t codepage = CP_THREAD_ACP);
tstring wstring_to_tstring(const WCHAR *wstr, uint32_t codepage = CP_THREAD_ACP);
tstring wstring_to_tstring(const std::wstring& wstr, uint32_t codepage = CP_THREAD_ACP);
std::string strsprintf(const char* format, ...);
std::wstring strsprintf(const WCHAR* format, ...);
std::vector<std::wstring> split(const std::wstring &str, const std::wstring &delim, bool bTrim = false);
std::vector<std::string> split(const std::string &str, const std::string &delim, bool bTrim = false);
std::string lstrip(const std::string& string, const char* trim = " \t\v\r\n");
std::string rstrip(const std::string& string, const char* trim = " \t\v\r\n");
std::string trim(const std::string& string, const char* trim = " \t\v\r\n");
std::wstring lstrip(const std::wstring& string, const WCHAR* trim = L" \t\v\r\n");
std::wstring rstrip(const std::wstring& string, const WCHAR* trim = L" \t\v\r\n");
std::wstring trim(const std::wstring& string, const WCHAR* trim = L" \t\v\r\n");
std::string str_replace(std::string str, const std::string& from, const std::string& to);
std::wstring str_replace(std::wstring str, const std::wstring& from, const std::wstring& to);
std::string GetFullPath(const char *path);
std::wstring GetFullPath(const WCHAR *path);
bool vce_get_filesize(const char *filepath, uint64_t *filesize);
bool vce_get_filesize(const WCHAR *filepath, UINT64 *filesize);
std::pair<int, std::string> PathRemoveFileSpecFixed(const std::string& path);
std::pair<int, std::wstring> PathRemoveFileSpecFixed(const std::wstring& path);
bool CreateDirectoryRecursive(const char *dir);
bool CreateDirectoryRecursive(const WCHAR *dir);

static const int VCE_TIMEBASE = 10000000;

tstring print_time(double time);

static inline uint16_t readUB16(const void *ptr) {
    uint16_t i = *(uint16_t *)ptr;
    return (i >> 8) | (i << 8);
}

static inline uint32_t readUB32(const void *ptr) {
    uint32_t i = *(uint32_t *)ptr;
    return (i >> 24) | ((i & 0xff0000) >> 8) | ((i & 0xff00) << 8) | ((i & 0xff) << 24);
}

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

size_t malloc_degeneracy(void **ptr, size_t nSize, size_t nMinSize);

tstring getOSVersion(OSVERSIONINFOEXW *osinfo = nullptr);
BOOL is_64bit_os();

int getCPUInfo(TCHAR *buffer, size_t nSize);
int getGPUInfo(const char *VendorName, TCHAR *buffer, unsigned int buffer_size, bool driver_version_only = false);

int vce_print_stderr(int log_level, const TCHAR *mes, HANDLE handle);

struct sBitstream {
    uint8_t *Data;
    uint32_t DataOffset;
    uint32_t DataLength;
    uint32_t MaxLength;
    uint64_t TimeStamp;
    uint64_t DecodeTimeStamp;
    int      FrameType;
    int      DataFlag;
    int      PictStruct;
    int      RepeatPict;
};

int bitstreamInit(sBitstream *pBitstream, uint32_t nSize);
int bitstreamCopy(sBitstream *pBitstreamCopy, const sBitstream *pBitstream);
int bitstreamExtend(sBitstream *pBitstream, uint32_t nSize);
int bitstreamAppend(sBitstream *pBitstream, const uint8_t *data, uint32_t size);
void bitstreamClear(sBitstream *pBitstream);

const TCHAR *CodecIdToStr(uint32_t codecId);

bool check_if_vce_available();
bool check_if_vce_hevc_available();
tstring check_vce_features(int nCodecId);

struct nal_info {
    const uint8_t *ptr;
    uint8_t type;
    uint32_t size;
};

enum : uint8_t {
    NALU_H264_UNDEF    = 0,
    NALU_H264_NONIDR   = 1,
    NALU_H264_SLICEA   = 2,
    NALU_H264_SLICEB   = 3,
    NALU_H264_SLICEC   = 4,
    NALU_H264_IDR      = 5,
    NALU_H264_SEI      = 6,
    NALU_H264_SPS      = 7,
    NALU_H264_PPS      = 8,
    NALU_H264_AUD      = 9,
    NALU_H264_EOSEQ    = 10,
    NALU_H264_EOSTREAM = 11,
    NALU_H264_FILLER   = 12,
    NALU_H264_SPSEXT   = 13,
    NALU_H264_PREFIX   = 14,
    NALU_H264_SUBSPS   = 15,

    NALU_HEVC_UNDEF    = 0,
    NALU_HEVC_VPS      = 32,
    NALU_HEVC_SPS      = 33,
    NALU_HEVC_PPS      = 34,
    NALU_HEVC_AUD      = 35,
    NALU_HEVC_EOS      = 36,
    NALU_HEVC_EOB      = 37,
    NALU_HEVC_FILLER     = 38,
    NALU_HEVC_PREFIX_SEI = 39,
    NALU_HEVC_SUFFIX_SEI = 40,
};

static std::vector<nal_info> parse_nal_unit_h264(const uint8_t *data, uint32_t size) {
    std::vector<nal_info> nal_list;
    nal_info nal_start = { nullptr, 0, 0 };
    const int i_fin = size - 3;
    for (int i = 0; i < i_fin; i++) {
        if (data[i+0] == 0 && data[i+1] == 0 && data[i+2] == 1) {
            if (nal_start.ptr) {
                nal_list.push_back(nal_start);
            }
            nal_start.ptr = data + i - (i > 0 && data[i-1] == 0);
            nal_start.type = data[i+3] & 0x1f;
            nal_start.size = (int)(data + size - nal_start.ptr);
            if (nal_list.size()) {
                auto prev = nal_list.end()-1;
                prev->size = (int)(nal_start.ptr - prev->ptr);
            }
            i += 3;
        }
    }
    if (nal_start.ptr) {
        nal_list.push_back(nal_start);
    }
    return nal_list;
}

static std::vector<nal_info> parse_nal_unit_hevc(const uint8_t *data, uint32_t size) {
    std::vector<nal_info> nal_list;
    nal_info nal_start = { nullptr, 0, 0 };
    const int i_fin = size - 3;

    for (int i = 0; i < i_fin; i++) {
        if (data[i+0] == 0 && data[i+1] == 0 && data[i+2] == 1) {
            if (nal_start.ptr) {
                nal_list.push_back(nal_start);
            }
            nal_start.ptr = data + i - (i > 0 && data[i-1] == 0);
            nal_start.type = (data[i+3] & 0x7f) >> 1;
            nal_start.size = (int)(data + size - nal_start.ptr);
            if (nal_list.size()) {
                auto prev = nal_list.end()-1;
                prev->size = (int)(nal_start.ptr - prev->ptr);
            }
            i += 3;
        }
    }
    if (nal_start.ptr) {
        nal_list.push_back(nal_start);
    }
    return nal_list;
}
