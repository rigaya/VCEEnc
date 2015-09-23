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
 * @file <BitStreamParser.h>
 *
 * @brief Header file for Bit-stream Parser
 *
 *******************************************************************************
 */

#pragma once

#include "AMFPlatform.h"
#include "Context.h"
#include "PipelineElement.h"

enum BitStreamType
{
    BitStreamH264AnnexB,
    BitStreamH264AvcC,
    BitStreamMpeg2,
    BitStreamMpeg4part2,
    BitStreamVC1,
    BitStreamUnknown
};

BitStreamType GetStreamType(const wchar_t* path);

class BitStreamParser;
typedef std::shared_ptr<BitStreamParser> BitStreamParserPtr;

class BitStreamParser: public PipelineElement
{
public:
    virtual ~BitStreamParser();

    virtual int GetOffsetX() const = 0;
    virtual int GetOffsetY() const = 0;
    virtual int GetPictureWidth() const = 0;
    virtual int GetPictureHeight() const = 0;
    virtual int GetAlignedWidth() const = 0;
    virtual int GetAlignedHeight() const = 0;

    virtual const unsigned char* GetExtraData() const = 0;
    virtual size_t GetExtraDataSize() const = 0;
    virtual void SetUseStartCodes(bool bUse) = 0;
    virtual void SetFrameRate(double fps) = 0;
	virtual double GetFrameRate() const = 0;
    virtual const wchar_t* GetCodecComponent() = 0;
    virtual AMF_RESULT QueryOutput(amf::AMFData** ppData) = 0;
public:
    static BitStreamParserPtr       Create(AMFDataStreamPtr pStream, BitStreamType type, amf::AMFContext* pContext);
};

// helpers
namespace Parser
{
inline char getLowByte(amf_uint16 data)
{
    return (data >> 8);
}

inline char getHiByte(amf_uint16 data)
{
    return (data & 0xFF);
}

inline bool getBit(const amf_uint8 *data, size_t &bitIdx)
{
    bool ret = (data[bitIdx / 8] >> (7 - bitIdx % 8) & 1);
    bitIdx++;
    return ret;
}
inline amf_uint32 getBitToUint32(const amf_uint8 *data, size_t &bitIdx)
{
    amf_uint32 ret = (data[bitIdx / 8] >> (7 - bitIdx % 8) & 1);
    bitIdx++;
    return ret;
}

    inline amf_uint32 readBits(const amf_uint8 *data, size_t &startBitIdx, size_t bitsToRead)
{
    if (bitsToRead > 32)
    {
        return 0; // assert(0);
    }
    amf_uint32 result = 0;
    for (size_t i = 0; i < bitsToRead; i++)
    {
        result = result << 1;
        result |= getBitToUint32(data, startBitIdx); // startBitIdx incremented inside
    }
    return result;
}

inline size_t countContiniusZeroBits(const amf_uint8 *data, size_t &startBitIdx)
{
    size_t startBitIdxOrg = startBitIdx;
    while (getBit(data, startBitIdx) == false) // startBitIdx incremented inside
    {
    }
    startBitIdx--; // remove non zero
    return startBitIdx - startBitIdxOrg;
}

    namespace ExpGolomb
    {
inline amf_uint32 readUe(const amf_uint8 *data, size_t &startBitIdx)
{
    size_t zeroBitsCount = countContiniusZeroBits(data, startBitIdx); // startBitIdx incremented inside
    if (zeroBitsCount > 30)
    {
        return 0; // assert(0)
    }

    amf_uint32 leftPart = (0x1 << zeroBitsCount) - 1;
    startBitIdx++;
    amf_uint32 rightPart = readBits(data, startBitIdx, zeroBitsCount);
    return leftPart + rightPart;
}

inline amf_int32 readSe(const amf_uint8 *data, size_t &startBitIdx)
{
    amf_uint32 ue = readUe(data, startBitIdx);
    // se From Ue
    amf_uint32 mod2 = ue % 2;
    amf_uint32 half = ue / 2;
    amf_uint32 r = ue / 2 + mod2;

    if (mod2 == 0)
    {
        return r * -1;
    }
    return r;
}
}
}

