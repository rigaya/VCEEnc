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
 * @file <FileHelper.h>
 *
 * @brief Header file for the File Helper functions
 *
 *******************************************************************************
 */

#pragma once

#include "PipelineElement.h"
#include "Pipeline.h"
#include "ParametersStorage.h"
#include <vector>

amf::AMF_SURFACE_FORMAT GetFormatFromString(const wchar_t* str);
void ParseRawFileFormat(const std::wstring path, amf_int32 &width,
                amf_int32 &height, amf::AMF_SURFACE_FORMAT& format);
std::wstring PathGetExtension(const std::wstring path);

static const wchar_t* PARAM_NAME_INPUT = L"INPUT";
static const wchar_t* PARAM_NAME_INPUT_DIR = L"INPUT_DIR";
static const wchar_t* PARAM_NAME_INPUT_WIDTH = L"WIDTH";
static const wchar_t* PARAM_NAME_INPUT_HEIGHT = L"HEIGHT";
static const wchar_t* PARAM_NAME_INPUT_FORMAT = L"FORMAT";
static const wchar_t* PARAM_NAME_INPUT_FRAMES        = L"FRAMES";
static const wchar_t* PARAM_NAME_OUTPUT = L"OUTPUT";
static const wchar_t* PARAM_NAME_OUTPUT_DIR = L"OUTPUT_DIR";
static const wchar_t* PARAM_NAME_OUTPUT_WIDTH = L"OUTPUT_WIDTH";
static const wchar_t* PARAM_NAME_OUTPUT_HEIGHT = L"OUTPUT_HEIGHT";
static const wchar_t* PARAM_NAME_OUTPUT_FORMAT = L"OUTPUT_FORMAT";
