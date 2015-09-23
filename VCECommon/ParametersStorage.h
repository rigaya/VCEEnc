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
 * @file <ParametersStorage.h>
 *
 * @brief Header file for Parameters Storage
 *
 *******************************************************************************
 */

#pragma once
#include <map>
#include "PropertyStorage.h"
#include "Thread.h"
#include "CmdLogger.h"

enum ParamType
{
    ParamUnknown = -1, ParamCommon = 0, ParamEncoderUsage, // sets to encoder first
    ParamEncoderStatic, // sets to encoder before initialization
    ParamEncoderDynamic, // sets to encoder at any time
    ParamEncoderFrame
// sets to frame before frame submission
};

AMF_RESULT ParamConverterBoolean(const std::wstring& value,
                amf::AMFVariant& valueOut);
AMF_RESULT ParamConverterVideoPresenter(const std::wstring& value,
                amf::AMFVariant& valueOut);
AMF_RESULT ParamConverterMemoryType(const std::wstring& value,
                amf::AMFVariant& valueOut);
AMF_RESULT ParamConverterFormat(const std::wstring& value,
                amf::AMFVariant& valueOut);
std::wstring toUpper(const std::wstring& str);
std::wstring AddIndexToPath(const std::wstring& path, amf_int32 index);

class ParametersStorage
{
public:
    ParametersStorage();

    AMF_RESULT SetParam(const wchar_t* name, amf::AMFVariantStruct value);
    AMF_RESULT GetParam(const wchar_t* name, amf::AMFVariantStruct* value);
    AMF_RESULT SetParamAsString(const std::wstring& name,
                    const std::wstring& value);

    template<typename _T>
    AMF_RESULT SetParam(const wchar_t* name, const _T& value);
    template<typename _T>
    AMF_RESULT GetParam(const wchar_t* name, _T& value);
    template<typename _T>
    AMF_RESULT GetParamWString(const wchar_t* name, _T& value);

    amf_size GetParamCount();
    AMF_RESULT GetParamAt(amf_size index, std::wstring& name,
                    amf::AMFVariantStruct* value);

    typedef AMF_RESULT (*ParamConverter)(const std::wstring& value,
                    amf::AMFVariant& valueOut);

    struct ParamDescription
    {
        std::wstring m_Name;
        ParamType m_Type;
        std::wstring m_Description;
        ParamConverter m_Converter;

        ParamDescription() :
            m_Type(ParamUnknown), m_Converter(NULL)
        {
        }
        ParamDescription(const std::wstring &name, ParamType type,
                        const std::wstring &description,
                        ParamConverter converter = NULL) :
            m_Name(name), m_Type(type), m_Description(description),
                            m_Converter(converter)
        {
        }
    };
    AMF_RESULT GetParamDescription(const wchar_t* name,
                    ParamDescription& description);
    AMF_RESULT
                    SetParamDescription(const wchar_t* name, ParamType type,
                                    const wchar_t* description,
                                    ParamConverter converter = NULL);
    std::wstring GetParamUsage();
protected:
    std::wstring GetParamUsage(ParamType type);

    typedef std::map<std::wstring, amf::AMFVariant> ParametersMap; // name / value

    ParametersMap m_parameters;
    typedef std::map<std::wstring, ParamDescription> ParamDescriptionMap; // name / description
    AMFCriticalSection m_csSect;

    ParamDescriptionMap m_descriptionMap;
};

typedef std::shared_ptr<ParametersStorage> ParametersStoragePtr;

//----------------------------------------------------------------------------------------------
// template methods implementations
//----------------------------------------------------------------------------------------------
template<typename _T> inline AMF_RESULT ParametersStorage::SetParam(
                const wchar_t* name, const _T& value)
{
    AMF_RESULT res = SetParam(name,
                    static_cast<const amf::AMFVariantStruct&> (amf::AMFVariant(
                                    value)));
    return res;
}

template<typename _T> inline AMF_RESULT ParametersStorage::GetParam(
                const wchar_t* name, _T& value)
{
    amf::AMFVariant var;
    AMF_RESULT err = GetParam(name, static_cast<amf::AMFVariantStruct*> (&var));
    if (err == AMF_OK)
    {
        value = static_cast<_T> (var);
    }
    return err;
}
template<typename _T> inline AMF_RESULT ParametersStorage::GetParamWString(
                const wchar_t* name, _T& value)
{
    amf::AMFVariant var;
    AMF_RESULT err = GetParam(name, static_cast<amf::AMFVariantStruct*> (&var));
    if (err == AMF_OK)
    {
        value = var.ToWString().c_str();
    }
    return err;
}

AMF_RESULT PushParamsToPropertyStorage(ParametersStorage* pParams,
                ParamType ptype, amf::AMFPropertyStorage *storage);
