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
#ifndef __VCE_UTIL_H__
#define __VCE_UTIL_H__

#include "rgy_util.h"
#include "rgy_err.h"
#pragma warning(push)
#pragma warning(disable:4201)
#pragma warning(disable:4100)
RGY_DISABLE_WARNING_PUSH
RGY_DISABLE_WARNING_STR("-Wclass-memaccess")
#include "VideoEncoderVCE.h"
#include "vce_param.h"
RGY_DISABLE_WARNING_POP
#pragma warning(pop)
#include "convert_csp.h"
#include "rgy_opencl.h"

class RGYFrameData;

bool check_if_vce_available(int deviceId, const RGYParamLogLevel& loglevel);
tstring check_vce_enc_features(const std::vector<RGY_CODEC>& codecs, int deviceId, const RGYParamLogLevel& loglevel);
tstring check_vce_dec_features(int deviceId, const RGYParamLogLevel& loglevel);
tstring check_vce_filter_features(int deviceId, const RGYParamLogLevel& loglevel);

MAP_PAIR_0_1_PROTO(codec, rgy, RGY_CODEC, enc, const wchar_t *);
MAP_PAIR_0_1_PROTO(codec, rgy, RGY_CODEC, dec, const wchar_t *);
MAP_PAIR_0_1_PROTO(csp, rgy, RGY_CSP, enc, amf::AMF_SURFACE_FORMAT);
MAP_PAIR_0_1_PROTO(loglevel, rgy, int, enc, int);
MAP_PAIR_0_1_PROTO(frametype, rgy, RGY_PICSTRUCT, enc, amf::AMF_FRAME_TYPE);
MAP_PAIR_0_1_PROTO(resize_mode, rgy, RGY_VPP_RESIZE_ALGO, enc, AMF_HQ_SCALER_ALGORITHM_ENUM);

const wchar_t * codec_rgy_to_dec_10bit(const RGY_CODEC codec);

AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_ENUM picstruct_rgy_to_enc(RGY_PICSTRUCT picstruct);
RGY_PICSTRUCT picstruct_enc_to_rgy(AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_ENUM picstruct);

const TCHAR *AMFRetString(AMF_RESULT ret);

struct RGYBitstream {
private:
    uint8_t *dataptr;
    size_t  dataLength;
    size_t  dataOffset;
    size_t  maxLength;
    int64_t  dataDts;
    int64_t  dataPts;
    uint32_t dataFlag;
    uint32_t dataAvgQP;
    RGY_FRAMETYPE dataFrametype;
    RGY_PICSTRUCT dataPicstruct;
    int dataFrameIdx;
    int64_t dataDuration;
    RGYFrameData **frameDataList;
    int frameDataNum;

public:
    uint8_t *bufptr() const {
        return dataptr;
    }

    uint8_t *data() const {
        return dataptr + dataOffset;
    }

    uint8_t *release() {
        uint8_t *ptr = dataptr;
        dataptr = nullptr;
        dataOffset = 0;
        dataLength = 0;
        maxLength = 0;
        return ptr;
    }

    uint32_t dataflag() const {
        return dataFlag;
    }

    void setDataflag(uint32_t flag) {
        dataFlag = flag;
    }

    RGY_FRAMETYPE frametype() const {
        return dataFrametype;
    }

    void setFrametype(RGY_FRAMETYPE type) {
        dataFrametype = type;
    }

    RGY_PICSTRUCT picstruct() const {
        return dataPicstruct;
    }

    void setPicstruct(RGY_PICSTRUCT picstruct) {
        dataPicstruct = picstruct;
    }

    int64_t duration() const {
        return dataDuration;
    }

    void setDuration(int64_t duration) {
        dataDuration = duration;
    }

    int frameIdx() const {
        return dataFrameIdx;
    }

    void setFrameIdx(int frameIdx) {
        dataFrameIdx = frameIdx;
    }

    size_t size() const {
        return dataLength;
    }

    void setSize(size_t size) {
        dataLength = size;
    }

    size_t offset() const {
        return dataOffset;
    }

    void addOffset(size_t add) {
        dataOffset += add;
    }

    void setOffset(size_t offset) {
        dataOffset = offset;
    }

    size_t bufsize() const {
        return maxLength;
    }

    int64_t pts() const {
        return dataPts;
    }

    void setPts(int64_t pts) {
        dataPts = pts;
    }

    int64_t dts() const {
        return dataDts;
    }

    void setDts(int64_t dts) {
        dataDts = dts;
    }

    uint32_t avgQP() {
        return dataAvgQP;
    }

    void setAvgQP(uint32_t avgQP) {
        dataAvgQP = avgQP;
    }

    void clear() {
        if (dataptr && maxLength) {
            _aligned_free(dataptr);
        }
        dataptr = nullptr;
        dataLength = 0;
        dataOffset = 0;
        maxLength = 0;
    }

    RGY_ERR init(size_t nSize) {
        clear();

        if (nSize > 0) {
            if (nullptr == (dataptr = (uint8_t *)_aligned_malloc(nSize, 32))) {
                return RGY_ERR_NULL_PTR;
            }

            maxLength = nSize;
        }
        return RGY_ERR_NONE;
    }

    void trim() {
        if (dataOffset > 0 && dataLength > 0) {
            memmove(dataptr, dataptr + dataOffset, dataLength);
            dataOffset = 0;
        }
    }

    RGY_ERR copy(const uint8_t *setData, size_t setSize) {
        if (setData == nullptr || setSize == 0) {
            return RGY_ERR_MORE_BITSTREAM;
        }
        if (maxLength < setSize) {
            clear();
            auto sts = init(setSize);
            if (sts != RGY_ERR_NONE) {
                return sts;
            }
        }
        dataLength = setSize;
        dataOffset = 0;
        memcpy(dataptr, setData, setSize);
        return RGY_ERR_NONE;
    }

    RGY_ERR copy(const uint8_t *setData, size_t setSize, int64_t pts, int64_t dts, int64_t duration) {
        auto sts = copy(setData, setSize);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        dataDts = dts;
        dataPts = pts;
        dataDuration = duration;
        return RGY_ERR_NONE;
    }

    RGY_ERR ref(uint8_t *refData, size_t refSize) {
        clear();
        dataptr = refData;
        dataLength = refSize;
        dataOffset = 0;
        maxLength = 0;
        return RGY_ERR_NONE;
    }

    RGY_ERR copy(const uint8_t *setData, size_t setSize, int64_t dts, int64_t pts) {
        auto sts = copy(setData, setSize);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        dataDts = dts;
        dataPts = pts;
        return RGY_ERR_NONE;
    }

    RGY_ERR ref(uint8_t *refData, size_t refSize, int64_t pts, int64_t dts, int64_t duration) {
        auto sts = ref(refData, refSize);
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        dataDts = dts;
        dataPts = pts;
        dataDuration = duration;
        return RGY_ERR_NONE;
    }

    RGY_ERR copy(const RGYBitstream *pBitstream) {
        auto sts = copy(pBitstream->data(), pBitstream->size());
        if (sts != RGY_ERR_NONE) {
            return sts;
        }
        return RGY_ERR_NONE;
    }

    RGY_ERR changeSize(size_t nNewSize) {
        uint8_t *pData = (uint8_t *)_aligned_malloc(nNewSize, 32);
        if (pData == nullptr) {
            return RGY_ERR_NULL_PTR;
        }

        auto nDataLen = dataLength;
        if (dataLength) {
            memcpy(pData, dataptr + dataOffset, (std::min)(nDataLen, nNewSize));
        }
        clear();

        dataptr       = pData;
        dataOffset = 0;
        dataLength = nDataLen;
        maxLength  = nNewSize;

        return RGY_ERR_NONE;
    }

    RGY_ERR append(const uint8_t *appendData, size_t appendSize) {
        if (appendData && appendSize > 0) {
            const auto new_data_length = appendSize + dataLength;
            if (maxLength < new_data_length) {
                auto sts = changeSize(new_data_length);
                if (sts != RGY_ERR_NONE) {
                    return sts;
                }
            }

            if (maxLength < new_data_length + dataOffset) {
                memmove(dataptr, dataptr + dataOffset, dataLength);
                dataOffset = 0;
            }
            memcpy(dataptr + dataLength + dataOffset, appendData, appendSize);
            dataLength = new_data_length;
        }
        return RGY_ERR_NONE;
    }

    RGY_ERR append(RGYBitstream *pBitstream) {
        return append(pBitstream->data(), pBitstream->size());
    }

    RGY_ERR resize(size_t nNewSize) {
        if (nNewSize > maxLength) {
            auto err = changeSize(nNewSize);
            dataLength = (err == RGY_ERR_NONE) ? nNewSize : 0;
            return err;
        }
        if (nNewSize + dataOffset > maxLength) {
            memmove(dataptr, dataptr + dataOffset, dataLength);
            dataOffset = 0;
        }
        dataLength = nNewSize;
        return RGY_ERR_NONE;
    }
    
    void addFrameData(RGYFrameData *frameData);
    void clearFrameDataList();
    std::vector<RGYFrameData *> getFrameDataList();
};

static inline RGYBitstream RGYBitstreamInit() {
    RGYBitstream bitstream;
    memset(&bitstream, 0, sizeof(bitstream));
    return bitstream;
}

#ifndef __CUDACC__
static_assert(std::is_pod<RGYBitstream>::value == true, "RGYBitstream should be POD type.");
#endif

#if ENABLE_OPENCL

struct RGYFrameAMF : public RGYFrame {
private:
    const wchar_t *PROP_INPUT_FRAMEID = L"RGYFrameInputFrameID";
    const wchar_t *PROP_FLAGS = L"RGYFrameFlags";
    amf::AMFSurfacePtr m_amfptr;
    std::vector<std::shared_ptr<RGYFrameData>> m_dataList;
public:
    RGYFrameAMF() : m_amfptr(), m_dataList() {};
    RGYFrameAMF(const amf::AMFSurfacePtr &pSurface) : m_amfptr(std::move(pSurface)), m_dataList() { }
    virtual ~RGYFrameAMF() { }
    virtual bool isempty() const override { return m_amfptr == nullptr; }
    const amf::AMFSurfacePtr &amf() const { return m_amfptr; }
    amf::AMFSurfacePtr &amf() { return m_amfptr; }
    const amf::AMFSurface *surf() const { return m_amfptr.GetPtr(); }
    amf::AMFSurface *surf() { return m_amfptr.GetPtr(); }
    amf::AMFSurfacePtr detachSurface() {
        amf::AMFSurfacePtr ptr = m_amfptr;
        m_amfptr = nullptr;
        return ptr;
    }
    std::unique_ptr<RGYFrame> createCopyAMF() {
        amf::AMFDataPtr data;
        m_amfptr->Duplicate(m_amfptr->GetMemoryType(), &data);
        return std::make_unique<RGYFrameAMF>(amf::AMFSurfacePtr(data));
    }
public:
    virtual void setTimestamp(uint64_t timestamp) override { m_amfptr->SetPts(timestamp); }
    virtual void setDuration(uint64_t duration) override { m_amfptr->SetDuration(duration); }
    virtual void setPicstruct(RGY_PICSTRUCT picstruct) override { m_amfptr->SetFrameType(frametype_rgy_to_enc(picstruct)); }
    virtual void setInputFrameId(int id) override { int64_t value = id; m_amfptr->SetProperty(PROP_INPUT_FRAMEID, value); }
    virtual void setFlags(RGY_FRAME_FLAGS flags) override { int64_t value = flags; m_amfptr->SetProperty(PROP_FLAGS, value); }
    virtual void clearDataList() override { m_dataList.clear(); }
    virtual const std::vector<std::shared_ptr<RGYFrameData>>& dataList() const override { return m_dataList; }
    virtual std::vector<std::shared_ptr<RGYFrameData>>& dataList() override { return m_dataList; }
    virtual void setDataList(const std::vector<std::shared_ptr<RGYFrameData>>& dataList) override { m_dataList = dataList; }
    RGYFrameInfo getInfoCopy() const { return getInfo(); }
protected:
    virtual RGYFrameInfo getInfo() const override {
        RGYFrameInfo info;
        for (int i = 0; i < 4; i++) {
            auto plane = m_amfptr->GetPlaneAt(i);
            if (plane) {
                info.ptr[i] = (uint8_t *)plane->GetNative();
                info.pitch[i] = plane->GetHPitch();
            }
        }
        info.width = m_amfptr->GetPlaneAt(0)->GetWidth();
        info.height = m_amfptr->GetPlaneAt(0)->GetHeight();
        info.csp = csp_enc_to_rgy(m_amfptr->GetFormat());
        info.timestamp = m_amfptr->GetPts();
        info.duration = m_amfptr->GetDuration();
        info.picstruct = frametype_enc_to_rgy(m_amfptr->GetFrameType());
        info.mem_type = m_amfptr->GetMemoryType() == amf::AMF_MEMORY_HOST ? RGY_MEM_TYPE_CPU : RGY_MEM_TYPE_GPU_IMAGE;
        int64_t value = 0;
        if (m_amfptr->GetProperty(PROP_INPUT_FRAMEID, &value) == AMF_OK) {
            info.inputFrameId = (int)value;
        }
        info.flags = RGY_FRAME_FLAG_NONE;
        if (m_amfptr->GetProperty(PROP_FLAGS, &value) == AMF_OK) {
            info.flags = (RGY_FRAME_FLAGS)value;
        }
        info.dataList = m_dataList;
        return info;
    }
};
#endif //#if ENABLE_OPENCL

VideoInfo videooutputinfo(
    RGY_CODEC codec,
    amf::AMF_SURFACE_FORMAT encFormat,
    const AMFParams &prm,
    const rgy_rational<int>& sar,
    RGY_PICSTRUCT picstruct,
    const VideoVUIInfo& vui);

int64_t rational_rescale(int64_t v, rgy_rational<int> from, rgy_rational<int> to);

tstring AccelTypeToString(amf::AMF_ACCELERATION_TYPE accelType);

#endif //#ifndef __VCE_UTIL_H__
