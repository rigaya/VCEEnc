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
#include "VideoEncoderVCE.h"
#include "vce_param.h"
#pragma warning(pop)
#include "convert_csp.h"
#include "rgy_opencl.h"

class RGYFrameData;

bool check_if_vce_available(int deviceId, int logLevel);
tstring check_vce_features(const std::vector<RGY_CODEC>& codecs, int deviceId, int logLevel);

MAP_PAIR_0_1_PROTO(codec, rgy, RGY_CODEC, enc, const wchar_t *);
MAP_PAIR_0_1_PROTO(codec, rgy, RGY_CODEC, dec, const wchar_t *);
MAP_PAIR_0_1_PROTO(csp, rgy, RGY_CSP, enc, amf::AMF_SURFACE_FORMAT);
MAP_PAIR_0_1_PROTO(loglevel, rgy, int, enc, int);
MAP_PAIR_0_1_PROTO(frametype, rgy, RGY_PICSTRUCT, enc, amf::AMF_FRAME_TYPE);

AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_ENUM picstruct_rgy_to_enc(RGY_PICSTRUCT picstruct);
RGY_PICSTRUCT picstruct_enc_to_rgy(AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_ENUM picstruct);

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

struct RGYFrame {
private:
    const wchar_t *PROP_INPUT_FRAMEID = L"RGYFrameInputFrameID";
    const wchar_t *PROP_FLAGS = L"RGYFrameFlags";
    amf::AMFSurfacePtr amfptr;
    unique_ptr<RGYCLFrame> clbuf;
    std::vector<std::shared_ptr<RGYFrameData>> dummy;
public:
    RGYFrame() : amfptr(), clbuf() {};
    RGYFrame(const amf::AMFSurfacePtr &pSurface) : amfptr(std::move(pSurface)), clbuf(), dummy() {
    }
    RGYFrame(unique_ptr<RGYCLFrame> clframe) : amfptr(), clbuf(std::move(clframe)) {
    }
    ~RGYFrame() {
        clbuf.reset();
    }
    bool isempty() const {
        return amfptr == nullptr && clbuf == nullptr;
    }
    const amf::AMFSurfacePtr &amf() const {
        return amfptr;
    }
    amf::AMFSurfacePtr &amf() {
        return amfptr;
    }
    const unique_ptr<RGYCLFrame> &cl() const {
        return clbuf;
    }
    unique_ptr<RGYCLFrame> &cl() {
        return clbuf;
    }
    amf::AMFSurfacePtr detachSurface() {
        amf::AMFSurfacePtr ptr = amfptr;
        amfptr = nullptr;
        return ptr;
    }
    unique_ptr<RGYCLFrame> detachCLFrame() {
        return std::move(clbuf);
    }
    unique_ptr<RGYFrame> createCopy() {
        if (amfptr) {
            return createCopyAMF();
        } else if (clbuf) {
            //未実装
            return std::make_unique<RGYFrame>();
        } else {
            return std::make_unique<RGYFrame>();
        }
    }
private:
    unique_ptr<RGYFrame> createCopyAMF() {
        amf::AMFDataPtr data;
        amfptr->Duplicate(amfptr->GetMemoryType(), &data);
        return std::make_unique<RGYFrame>(amf::AMFSurfacePtr(data));
    }

    FrameInfo infoAMF() const {
        FrameInfo info;
        for (int i = 0; i < 4; i++) {
            auto plane = amfptr->GetPlaneAt(i);
            if (plane) {
                info.ptr[i] = (uint8_t *)plane->GetNative();
                info.pitch[i] = plane->GetHPitch();
            }
        }
        info.width = amfptr->GetPlaneAt(0)->GetWidth();
        info.height = amfptr->GetPlaneAt(0)->GetHeight();
        info.csp = csp_enc_to_rgy(amfptr->GetFormat());
        info.timestamp = amfptr->GetPts();
        info.duration = amfptr->GetDuration();
        info.picstruct = frametype_enc_to_rgy(amfptr->GetFrameType());
        info.flags = RGY_FRAME_FLAG_NONE;
        info.mem_type = amfptr->GetMemoryType() == amf::AMF_MEMORY_HOST ? RGY_MEM_TYPE_CPU : RGY_MEM_TYPE_GPU_IMAGE;
        int64_t value = 0;
        if (amfptr->GetProperty(PROP_INPUT_FRAMEID, &value) == AMF_OK) {
            info.inputFrameId = (int)value;
        }
        if (amfptr->GetProperty(PROP_FLAGS, &value) == AMF_OK) {
            info.flags = (RGY_FRAME_FLAGS)value;
        }
        return info;
    }
    FrameInfo infoCL() const {
        return clbuf->frame;
    }
public:
    FrameInfo getInfo() const {
        if (amfptr) {
            return infoAMF();
        } else if (clbuf) {
            return infoCL();
        } else {
            return FrameInfo();
        }
    }
    void ptrArray(void *array[3], bool bRGB) {
        auto frame = getInfo();
        UNREFERENCED_PARAMETER(bRGB);
        array[0] = (void *)frame.ptr[0];
        array[1] = (void *)frame.ptr[1];
        array[2] = (void *)frame.ptr[2];
    }
    uint8_t *ptrY() const {
        return getInfo().ptr[0];
    }
    uint8_t *ptrUV() const {
        return getInfo().ptr[1];
    }
    uint8_t *ptrU() const {
        return getInfo().ptr[1];
    }
    uint8_t *ptrV() const {
        return getInfo().ptr[2];
    }
    uint8_t *ptrRGB() const {
        return getInfo().ptr[0];
    }
    uint32_t pitch(int index = 0) const {
        return getInfo().pitch[index];
    }
    uint64_t timestamp() const {
        return getInfo().timestamp;
    }
    void setTimestamp(uint64_t timestamp) {
        if (amfptr) {
            amfptr->SetPts(timestamp);
        } else if (clbuf) {
            clbuf->frame.timestamp = timestamp;
        }
    }
    int64_t duration() const {
        return getInfo().duration;
    }
    void setDuration(uint64_t duration) {
        if (amfptr) {
            amfptr->SetDuration(duration);
        } else if (clbuf) {
            clbuf->frame.duration = duration;
        }
    }
    RGY_PICSTRUCT picstruct() const {
        return getInfo().picstruct;
    }
    void setPicstruct(RGY_PICSTRUCT picstruct) {
        if (amfptr) {
            amfptr->SetFrameType(frametype_rgy_to_enc(picstruct));
        } else if (clbuf) {
            clbuf->frame.picstruct = picstruct;
        }
    }
    int inputFrameId() const {
        return getInfo().inputFrameId;
    }
    void setInputFrameId(int id) {
        if (amfptr) {
            int64_t value = id;
            amfptr->SetProperty(PROP_INPUT_FRAMEID, value);
        } else if (clbuf) {
            clbuf->frame.inputFrameId = id;
        }
    }
    RGY_FRAME_FLAGS flags() const {
        return getInfo().flags;
    }
    void setFlags(RGY_FRAME_FLAGS flags) {
        if (amfptr) {
            int64_t value = flags;
            amfptr->SetProperty(PROP_FLAGS, value);
        } else if (clbuf) {
            clbuf->frame.flags = flags;
        }
    }
    const std::vector<std::shared_ptr<RGYFrameData>>& dataList() const {
        if (clbuf) {
            return clbuf->frame.dataList;
        } else {
            return dummy;
        }
    };
    std::vector<std::shared_ptr<RGYFrameData>>& dataList() {
        if (clbuf) {
            return clbuf->frame.dataList;
        } else {
            return dummy;
        }
    };
};
#else
typedef void RGYFrame;

#endif //#if ENABLE_OPENCL

VideoInfo videooutputinfo(
    RGY_CODEC codec,
    amf::AMF_SURFACE_FORMAT encFormat,
    const AMFParams &prm,
    RGY_PICSTRUCT picstruct,
    const VideoVUIInfo& vui);

int64_t rational_rescale(int64_t v, rgy_rational<int> from, rgy_rational<int> to);

CodecCsp getHWDecCodecCsp(const bool skipHWDecodeCheck);

tstring AccelTypeToString(amf::AMF_ACCELERATION_TYPE accelType);

#endif //#ifndef __VCE_UTIL_H__
