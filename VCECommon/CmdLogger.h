/*******************************************************************************
 Copyright 息2014 Advanced Micro Devices, Inc. All rights reserved.

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
 * @file <CmdLogger.h>
 *
 * @brief Header file for Command Logging
 *
 *******************************************************************************
 */

#pragma once
#include <string>
#include <memory>
#include <sstream>
#include <ios>
#include "Result.h"
#include "Debug.h"

#include "Thread.h"

#pragma warning(push)
#pragma warning(disable: 4100)

enum LogType
{
    LogTypeInfo, LogTypeSuccess, LogTypeError
};

void WriteLog(const wchar_t* message, LogType type);

#define LOG_WRITE(a, type)\
    do{ \
        std::wstringstream messageStream12345;\
        messageStream12345 << a;\
        WriteLog(messageStream12345.str().c_str(), type);\
    }while(0)

#define LOG_INFO(a) LOG_WRITE(a << std::endl, LogTypeInfo)
#define LOG_SUCCESS(a) LOG_WRITE(a << std::endl, LogTypeSuccess)
#define LOG_ERROR(a) LOG_WRITE(a << std::endl, LogTypeError)

#ifdef _DEBUG
#define LOG_DEBUG(a)     LOG_INFO(a)
#else
#define LOG_DEBUG(a)
#endif

#define LOG_AMF_ERROR(err, text) \
    do{ \
        if( (err) != AMF_OK) \
        { \
            LOG_WRITE(text << L" Error:" << amf::AMFGetResultText((err)) << std::endl, LogTypeError);\
        } \
    }while(0)

#define CHECK_RETURN(exp, err, text) \
    do{ \
        if((exp) == false) \
        {  \
            LOG_AMF_ERROR(err, text);\
            return (err); \
        } \
    }while(0)

#define CHECK_AMF_ERROR_RETURN(err, text) \
    do{ \
        if((err) != AMF_OK) \
        {  \
            LOG_AMF_ERROR(err, text);\
            return (err); \
        } \
    }while(0)

#define CHECK_HRESULT_ERROR_RETURN(err, text) \
    do{ \
        if(FAILED(err)) \
        {  \
            LOG_WRITE(text << L" HRESULT Error: " << std::hex << err << std::endl, LogTypeError); \
            return AMF_FAIL; \
        } \
    }while(0)

#define CHECK_OPENCL_ERROR_RETURN(err, text) \
    do{ \
        if(err) \
        {  \
            LOG_WRITE(text << L" OpenCL Error: " << err<< std::endl, LogTypeError); \
            return AMF_FAIL; \
        } \
    }while(0)

class AMFCustomTraceWriter: public amf::AMFTraceWriter
{
public:
    AMFCustomTraceWriter(amf_int32 level)
    {
        amf::AMFTraceRegisterWriter(L"AMFCustomTraceWriter", this);
        amf::AMFTraceSetWriterLevel(L"AMFCustomTraceWriter", level);
        amf::AMFTraceEnableWriter(AMF_TRACE_WRITER_CONSOLE, false);
    }
    ~AMFCustomTraceWriter()
    {
        amf::AMFTraceUnregisterWriter(L"AMFCustomTraceWriter");
    }
    virtual void Write(const wchar_t* scope, const wchar_t* message)
    {
        WriteLog(message, LogTypeInfo);
    }
    virtual void Flush()
    {
    }
};
#pragma warning(pop)
