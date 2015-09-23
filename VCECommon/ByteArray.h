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
 * @file <ByteArray.h>
 *
 * @brief Header file for Byte Array Processing
 *
 *******************************************************************************
 */

#pragma once
#include "AMFPlatform.h"
#define    INIT_ARRAY_SIZE 1024
//------------------------------------------------------------------------
class AMFByteArray
{
protected:
    amf_uint8 *m_pData;
    amf_size m_iSize;
    amf_size m_iMaxSize;
public:
    AMFByteArray() :
        m_pData(0), m_iSize(0), m_iMaxSize(0)
    {
    }
    AMFByteArray(amf_size num) :
        m_pData(0), m_iSize(0), m_iMaxSize(0)
    {
        SetSize(num);
    }
    virtual ~AMFByteArray()
    {
        if (m_pData != 0)
        {
            delete[] m_pData;
        }
    }
    void SetSize(amf_size num)
    {
        if (num == m_iSize)
        {
            return;
        }
        if (num < m_iSize)
        {
            memset(m_pData + num, 0, m_iMaxSize - num);
        }
        else if (num > m_iMaxSize)
        {
            m_iMaxSize = (num / INIT_ARRAY_SIZE) * INIT_ARRAY_SIZE
                            + INIT_ARRAY_SIZE;
            amf_uint8 *pNewData = new amf_uint8[m_iMaxSize];
            memset(pNewData, 0, m_iMaxSize);
            if (m_pData != NULL)
            {
                memcpy(pNewData, m_pData, m_iSize);
                delete[] m_pData;
            }
            m_pData = pNewData;
        }
        m_iSize = num;
    }
    void Copy(const AMFByteArray &old)
    {
        if (m_iMaxSize < old.m_iSize)
        {
            m_iMaxSize = old.m_iMaxSize;
            if (m_pData != NULL)
            {
                delete[] m_pData;
            }
            m_pData = new amf_uint8[m_iMaxSize];
            memset(m_pData, 0, m_iMaxSize);
        }
        memcpy(m_pData, old.m_pData, old.m_iSize);
        m_iSize = old.m_iSize;
    }
    amf_uint8 operator[](amf_size iPos) const
    {
        return m_pData[iPos];
    }
    amf_uint8& operator[](amf_size iPos)
    {
        return m_pData[iPos];
    }
    amf_uint8 *GetData() const
    {
        return m_pData;
    }
    amf_size GetSize() const
    {
        return m_iSize;
    }
};
