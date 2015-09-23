/*******************************************************************************
 Copyright ｩ2014 Advanced Micro Devices, Inc. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1   Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 2   Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

/**
 *******************************************************************************
 * @file <FileHelper.cpp>
 *
 * @brief Source file for the File Helper functions
 *
 *******************************************************************************
 */

#include "FileHelper.h"
#include "Thread.h"
#include "CmdLogger.h"
#include <sstream>
#include <set>
#include <fstream>

#include <atlbase.h>
#include "d3d9.h"
#pragma comment(lib, "d3d9.lib")

#pragma warning(disable:4355)

inline bool AMFIsDecimal(wchar_t sum)
{
    return sum >= L'0' && sum <= L'9';
}

std::wstring PathGetExtension(const std::wstring path)
{
    std::wstring::size_type dot_pos = path.find_last_of(L'.');
    std::wstring ext = path.substr(dot_pos + 1);
    return ext;
}

void ParseRawFileFormat(const std::wstring path, amf_int32 &width,
                amf_int32 &height, amf::AMF_SURFACE_FORMAT& format)
{
    if (width == 0 || height == 0)
    {
        std::wstring::size_type dot_pos = path.find_last_of(L'.');
        std::wstring::size_type slash_pos = path.find_last_of(L"\\/");
        std::wstring fileName = path.substr(slash_pos + 1, dot_pos - (slash_pos
                        + 1));

        amf_size pos = 0;
        amf_size leftPos = std::wstring::npos;
        amf_size rightPos = std::wstring::npos;
        amf_size len = fileName.length();

        std::wstring tmp;
        while (std::wstring::npos != (pos = fileName.find_first_of('x', pos)))
        {
            if (pos == 0 || pos == len - 1)
            {
                ++pos;
                continue;
            }
            // find left
            leftPos = pos - 1;
            rightPos = pos + 1;
            if (AMFIsDecimal(fileName[leftPos]) && AMFIsDecimal(
                            fileName[rightPos]))
            {
                while (leftPos != 0)
                {
                    if (!AMFIsDecimal(fileName[leftPos]))
                    {
                        ++leftPos;
                        break;
                    }
                    --leftPos;
                }
                while (rightPos != len)
                {
                    if (!AMFIsDecimal(fileName[rightPos]))
                    {
                        --rightPos;
                        break;
                    }
                    ++rightPos;
                }
                tmp = fileName.substr(leftPos, pos - leftPos);
                swscanf_s(tmp.c_str(), L"%d", &width);
                tmp = fileName.substr(pos + 1, rightPos - pos);
                swscanf_s(tmp.c_str(), L"%d", &height);

                break;
            }
            ++pos;
        }
    }

    if (format == amf::AMF_SURFACE_UNKNOWN)
    {
        std::wstring::size_type dot_pos = path.find_last_of(L'.');
        std::wstring ext = path.substr(dot_pos + 1);
        format = GetFormatFromString(ext.c_str());
    }
}

amf::AMF_SURFACE_FORMAT GetFormatFromString(const wchar_t* str)
{
    amf::AMF_SURFACE_FORMAT ret = amf::AMF_SURFACE_UNKNOWN;
    std::wstring std_string = str;
    if (std_string == L"420p" || std_string == L"yuv" || std_string == L"I420")
    {
        ret = amf::AMF_SURFACE_YUV420P;
    }
    else if (std_string == L"bgra")
    {
        ret = amf::AMF_SURFACE_BGRA;
    }
    else if (std_string == L"rgba")
    {
        ret = amf::AMF_SURFACE_RGBA;
    }
    else if (std_string == L"argb")
    {
        ret = amf::AMF_SURFACE_ARGB;
    }
    else if (std_string == L"nv12")
    {
        ret = amf::AMF_SURFACE_NV12;
    }
    else if (std_string == L"yv12")
    {
        ret = amf::AMF_SURFACE_YV12;
    }
    return ret;
}

