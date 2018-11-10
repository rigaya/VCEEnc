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
#include "VideoEncoderVCE.h"
#include "PipelineElement.h"
#include "ParametersStorage.h"

bool check_if_vce_available(int nCodecId, int nDeviceId = 0);
tstring check_vce_features(int nCodecId, int nDeviceId);

MAP_PAIR_0_1_PROTO(codec, rgy, RGY_CODEC, enc, const wchar_t *);
MAP_PAIR_0_1_PROTO(codec, rgy, RGY_CODEC, dec, const wchar_t *);
MAP_PAIR_0_1_PROTO(csp, rgy, RGY_CSP, enc, amf::AMF_SURFACE_FORMAT);

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

public:
    uint8_t *bufptr() const {
        return dataptr;
    }

    uint8_t *data() const {
        return dataptr + dataOffset;
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

    int64_t duration() {
        return dataDuration;
    }

    void setDuration(int64_t duration) {
        dataDuration = duration;
    }

    int frameIdx() {
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
};

static inline RGYBitstream RGYBitstreamInit() {
    RGYBitstream bitstream;
    memset(&bitstream, 0, sizeof(bitstream));
    return bitstream;
}

#ifndef __CUDACC__
static_assert(std::is_pod<RGYBitstream>::value == true, "RGYBitstream should be POD type.");
#endif

struct RGYFrame {
private:
    FrameInfo info;
public:
    FrameInfo getInfo() const {
        return info;
    }
    void set(const FrameInfo& frameinfo) {
        info = frameinfo;
    }
    void set(uint8_t *ptr, int width, int height, int pitch, RGY_CSP csp, int64_t timestamp = 0, int64_t duration = 0) {
        info.ptr = ptr;
        info.width = width;
        info.height = height;
        info.pitch = pitch;
        info.csp = csp;
        info.timestamp = timestamp;
        info.duration = duration;
    }
    void ptrArray(void *array[3], bool bRGB) {
        UNREFERENCED_PARAMETER(bRGB);
        array[0] = info.ptr;
        array[1] = info.ptr + info.pitch * info.height;
        array[2] = info.ptr + info.pitch * info.height * 2;
    }
    uint8_t *ptrY() {
        return info.ptr;
    }
    uint8_t *ptrUV() {
        return info.ptr + info.pitch * info.height;
    }
    uint8_t *ptrU() {
        return info.ptr + info.pitch * info.height;
    }
    uint8_t *ptrV() {
        return info.ptr + info.pitch * info.height * 2;
    }
    uint8_t *ptrRGB() {
        return info.ptr;
    }
    uint32_t pitch() {
        return info.pitch;
    }
    uint64_t timestamp() {
        return info.timestamp;
    }
    void setTimestamp(uint64_t frame_timestamp) {
        info.timestamp = frame_timestamp;
    }
    int64_t duration() {
        return info.duration;
    }
    void setDuration(int64_t frame_duration) {
        info.duration = frame_duration;
    }
};

static inline RGYFrame RGYFrameInit() {
    RGYFrame frame;
    memset(&frame, 0, sizeof(frame));
    return frame;
}

static inline RGYFrame RGYFrameInit(const FrameInfo& frameinfo) {
    RGYFrame frame;
    frame.set(frameinfo);
    return frame;
}

static inline RGYFrame RGYFrameFromSurface(const amf::AMFSurfacePtr& pSurface) {
    auto plane = pSurface->GetPlaneAt(0);
    const int dst_pitch = plane->GetHPitch();
    const int dst_height = plane->GetVPitch();
    RGYFrame frame;
    frame.set((uint8_t *)plane->GetNative(),
        plane->GetWidth(),
        dst_height,
        dst_pitch,
        csp_enc_to_rgy(pSurface->GetFormat()),
        pSurface->GetPts(),
        pSurface->GetDuration());
    return frame;
}

#ifndef __CUDACC__
static_assert(std::is_pod<RGYFrame>::value == true, "RGYFrame should be POD type.");
#endif

VideoInfo videooutputinfo(
    RGY_CODEC codec,
    amf::AMF_SURFACE_FORMAT encFormat,
    ParametersStorage& prm,
    RGY_PICSTRUCT picstruct,
    const VideoVUIInfo& vui);

int64_t rational_rescale(int64_t v, rgy_rational<int> from, rgy_rational<int> to);

template<typename T>
T getprm(ParametersStorage& prm, const wchar_t *key) {
    T value;
    prm.GetParam(key, value);
    return value;
}

template<>
int getprm(ParametersStorage& prm, const wchar_t *key);

CodecCsp getHWDecCodecCsp();

#endif //#ifndef __VCE_UTIL_H__
