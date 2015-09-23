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
 * @file <BitStreamParser.cpp>
 *
 * @brief Source file for parsing bit-stream
 *
 *******************************************************************************
 */

#include <assert.h>
#include <string>
#include <cctype>
#include <algorithm>
#include <functional>
#include <iterator>
#include "BitStreamParser.h"
#include "BitStreamParserH264.h"

BitStreamParser::~BitStreamParser()
{
}

typedef BitStreamParser* (*BitstreamParserCreateFunction)(
                const wchar_t* fileName);

/*
 const std::array<StreamTypeInfo, 5> StreamTypeAssociations = {
 StreamTypeInfo(L".h264", StreamType::H264AnnexB, H264::CreateAnnexBParser),
 StreamTypeInfo(L".avcc", StreamType::H264AvcC, H264::CreateAvcCParser),
 StreamTypeInfo(L".m2v", StreamType::Mpeg2, Mpeg2::CreateParser),
 StreamTypeInfo(L".m4v", StreamType::Mpeg4part2, Mpeg4Visual::CreateParser),
 StreamTypeInfo(L".vc1", StreamType::VC1, VC1::CreateParser)
 };
 */
BitStreamType GetStreamType(const wchar_t* path)
{
    const wchar_t ExtDelimiter = L'.';

    std::wstring name(toUpper(path));
    std::wstring::size_type delimiterPos = name.find_last_of(ExtDelimiter);

    if (std::wstring::npos == delimiterPos)
    {
        return BitStreamUnknown;
    }

    const std::wstring ext = name.substr(delimiterPos);

    if (ext == L".H264" || ext == L".264" || ext == L".SVC")
    {
        return BitStreamH264AnnexB;
    }
    if (ext == L".AVCC")
    {
        //CreateAvcCParser(fileName);
    }

    return BitStreamUnknown;
}

BitStreamParserPtr BitStreamParser::Create(AMFDataStreamPtr pStream,
                BitStreamType type, amf::AMFContext* pContext)
{
    BitStreamParserPtr pParser;
    switch (type)
    {
    case BitStreamH264AnnexB:
        pParser = BitStreamParserPtr(CreateAnnexBParser(pStream, pContext));
    case BitStreamH264AvcC:
    case BitStreamMpeg2:
    case BitStreamMpeg4part2:
    case BitStreamVC1:
    default:
        break;
    }
    return pParser;
}
