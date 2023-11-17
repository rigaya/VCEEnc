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
#ifndef __VCE_PIPELINE_H__
#define __VCE_PIPELINE_H__

#include <thread>
#include <future>
#include <atomic>
#include <deque>
#include <set>
#include <unordered_map>
#pragma warning(push)
#pragma warning(disable:4100)
#include "VideoEncoderVCE.h"
#include "Component.h"
#include "Factory.h"
#include "Trace.h"

#include "rgy_version.h"
#include "rgy_err.h"
#include "rgy_util.h"
#include "rgy_log.h"
#include "rgy_input.h"
#include "rgy_input_avcodec.h"
#include "rgy_input_sm.h"
#include "rgy_output.h"
#include "rgy_output_avcodec.h"
#include "rgy_opencl.h"
#include "rgy_filter.h"
#include "rgy_filter_ssim.h"
#include "rgy_thread.h"
#include "rgy_timecode.h"
#include "rgy_device.h"
#include "vce_device.h"
#include "vce_param.h"
#include "vce_filter.h"

static const int RGY_WAIT_INTERVAL = 60000;

enum RGYRunState {
    RGY_STATE_STOPPED,
    RGY_STATE_RUNNING,
    RGY_STATE_ERROR,
    RGY_STATE_ABORT,
    RGY_STATE_EOF
};

enum class VppType : int {
    VPP_NONE,

    AMF_CONVERTER,
    AMF_PREPROCESS,
    AMF_RESIZE,
    AMF_VQENHANCE,

    AMF_MAX,

    CL_MIN = AMF_MAX,

    CL_CROP,
    CL_AFS,
    CL_NNEDI,
    CL_DECIMATE,
    CL_MPDECIMATE,
    CL_YADIF,
    CL_COLORSPACE,
    CL_RFF,
    CL_DELOGO,
    CL_TRANSFORM,

    CL_CONVOLUTION3D,
    CL_DENOISE_KNN,
    CL_DENOISE_PMD,
    CL_DENOISE_SMOOTH,

    CL_RESIZE,

    CL_SUBBURN,

    CL_UNSHARP,
    CL_EDGELEVEL,
    CL_WARPSHARP,

    CL_CURVES,
    CL_TWEAK,
    CL_OVERLAY,
    CL_DEBAND,

    CL_PAD,

    CL_MAX,
};

enum class VppFilterType { FILTER_NONE, FILTER_AMF, FILTER_OPENCL };

static VppFilterType getVppFilterType(VppType vpptype) {
    if (vpptype == VppType::VPP_NONE) return VppFilterType::FILTER_NONE;
    if (vpptype < VppType::AMF_MAX) return VppFilterType::FILTER_AMF;
    if (vpptype < VppType::CL_MAX) return VppFilterType::FILTER_OPENCL;
    return VppFilterType::FILTER_NONE;
}

struct VppVilterBlock {
    VppFilterType type;
    std::unique_ptr<AMFFilter> vppamf;
    std::vector<std::unique_ptr<RGYFilter>> vppcl;

    VppVilterBlock(std::unique_ptr<AMFFilter>& filter) : type(VppFilterType::FILTER_AMF), vppamf(std::move(filter)), vppcl() {};
    VppVilterBlock(std::vector<std::unique_ptr<RGYFilter>>& filter) : type(VppFilterType::FILTER_OPENCL), vppamf(), vppcl(std::move(filter)) {};
};

enum class PipelineTaskOutputType {
    UNKNOWN,
    SURFACE,
    BITSTREAM
};

enum class PipelineTaskSurfaceType {
    UNKNOWN,
    CL,
    AMF
};

class PipelineTaskSurface {
private:
    RGYFrame *surf;
    std::atomic<int> *ref;
public:
    PipelineTaskSurface() : surf(nullptr), ref(nullptr) {};
    PipelineTaskSurface(std::pair<RGYFrame *, std::atomic<int> *> surf_) : PipelineTaskSurface(surf_.first, surf_.second) {};
    PipelineTaskSurface(RGYFrame *surf_, std::atomic<int> *ref_) : surf(surf_), ref(ref_) { if (surf) (*ref)++; };
    PipelineTaskSurface(const PipelineTaskSurface& obj) : surf(obj.surf), ref(obj.ref) { if (surf) (*ref)++; }
    PipelineTaskSurface &operator=(const PipelineTaskSurface &obj) {
        if (this != &obj) { // 自身の代入チェック
            surf = obj.surf;
            ref = obj.ref;
            if (surf) (*ref)++;
        }
        return *this;
    }
    ~PipelineTaskSurface() { reset(); }
    void reset() { if (surf) (*ref)--; surf = nullptr; ref = nullptr; }
    bool operator !() const {
        return frame() == nullptr;
    }
    bool operator !=(const PipelineTaskSurface& obj) const { return frame() != obj.frame(); }
    bool operator ==(const PipelineTaskSurface& obj) const { return frame() == obj.frame(); }
    bool operator !=(std::nullptr_t) const { return frame() != nullptr; }
    bool operator ==(std::nullptr_t) const { return frame() == nullptr; }
    const RGYFrameAMF *amf() const { return dynamic_cast<const RGYFrameAMF*>(surf); }
    RGYFrameAMF *amf() { return dynamic_cast<RGYFrameAMF*>(surf); }
    const RGYCLFrame *cl() const { return dynamic_cast<const RGYCLFrame*>(surf); }
    RGYCLFrame *cl() { return dynamic_cast<RGYCLFrame*>(surf); }
    const RGYFrame *frame() const { return surf; }
    RGYFrame *frame() { return surf; }
};

// アプリ用の独自参照カウンタと組み合わせたクラス
class PipelineTaskSurfaces {
private:
    class PipelineTaskSurfacesPair {
    private:
        std::unique_ptr<RGYFrame> surf_;
        std::atomic<int> ref;
    public:
        PipelineTaskSurfacesPair(std::unique_ptr<RGYFrame> s) : surf_(std::move(s)), ref(0) {};
        
        bool isFree() const { return ref == 0; } // 使用されていないフレームかを返す
        PipelineTaskSurface getRef() { return PipelineTaskSurface(surf_.get(), &ref); };
        const RGYFrame *surf() const { return surf_.get(); }
        RGYFrame *surf() { return surf_.get(); }
        PipelineTaskSurfaceType type() const {
            if (!surf_) return PipelineTaskSurfaceType::UNKNOWN;
            if (dynamic_cast<const RGYCLFrame*>(surf_.get())) return PipelineTaskSurfaceType::CL;
            if (dynamic_cast<const RGYFrameAMF*>(surf_.get())) return PipelineTaskSurfaceType::AMF;
            return PipelineTaskSurfaceType::UNKNOWN;
        }
    };
    std::vector<std::unique_ptr<PipelineTaskSurfacesPair>> m_surfaces; // フレームと参照カウンタ
public:
    PipelineTaskSurfaces() : m_surfaces() {};
    ~PipelineTaskSurfaces() {}

    void clear() {
        m_surfaces.clear();
    }
    void setSurfaces(std::vector<amf::AMFSurfacePtr>& surfs) {
        clear();
        m_surfaces.resize(surfs.size());
        for (size_t i = 0; i < m_surfaces.size(); i++) {
            m_surfaces[i] = std::make_unique<PipelineTaskSurfacesPair>(std::make_unique<RGYFrameAMF>(surfs[i]));
        }
    }
    void setSurfaces(std::vector<std::unique_ptr<RGYCLFrame>>& frames) {
        clear();
        m_surfaces.resize(frames.size());
        for (size_t i = 0; i < m_surfaces.size(); i++) {
            m_surfaces[i] = std::make_unique<PipelineTaskSurfacesPair>(std::move(frames[i]));
        }
    }
    PipelineTaskSurface addSurface(std::unique_ptr<RGYFrameAMF>& surf) {
        deleteFreedSurface();
        m_surfaces.push_back(std::move(std::unique_ptr<PipelineTaskSurfacesPair>(new PipelineTaskSurfacesPair(std::move(surf)))));
        return m_surfaces.back()->getRef();
    }

    PipelineTaskSurface getFreeSurf() {
        for (auto& s : m_surfaces) {
            if (s->isFree()) {
                return s->getRef();
            }
        }
        return PipelineTaskSurface();
    }
    PipelineTaskSurface get(amf::AMFSurface *surf) {
        for (auto& s : m_surfaces) {
            if (auto amf = dynamic_cast<RGYFrameAMF*>(s->surf()); amf != nullptr) {
                if (amf->surf() == surf) {
                    return s->getRef();
                }
            }
        }
        return PipelineTaskSurface();
    }
    PipelineTaskSurface get(RGYCLFrame *frame) {
        auto s = findSurf(frame);
        if (s != nullptr) {
            return s->getRef();
        }
        return PipelineTaskSurface();
    }
    size_t bufCount() const { return m_surfaces.size(); }

    bool isAllFree() const {
        for (const auto& s : m_surfaces) {
            if (!s->isFree()) {
                return false;
            }
        }
        return true;
    }
    PipelineTaskSurfaceType type() const {
        if (m_surfaces.size() == 0) return PipelineTaskSurfaceType::UNKNOWN;
        return m_surfaces.front()->type();
    }
protected:
    void deleteFreedSurface() {
        for (auto it = m_surfaces.begin(); it != m_surfaces.end();) {
            if ((*it)->isFree()) {
                it = m_surfaces.erase(it);
            } else {
                it++;
            }
        }
    }
    PipelineTaskSurfacesPair *findSurf(RGYFrame *surf) {
        for (auto& s : m_surfaces) {
            if (s->surf() == surf) {
                return s.get();
            }
        }
        return nullptr;
    }
};

class PipelineTaskOutputDataCustom {
    int type;
public:
    PipelineTaskOutputDataCustom() {};
    virtual ~PipelineTaskOutputDataCustom() {};
};

class PipelineTaskOutputDataCheckPts : public PipelineTaskOutputDataCustom {
private:
    int64_t timestamp;
public:
    PipelineTaskOutputDataCheckPts() : PipelineTaskOutputDataCustom() {};
    PipelineTaskOutputDataCheckPts(int64_t timestampOverride) : PipelineTaskOutputDataCustom(), timestamp(timestampOverride) {};
    virtual ~PipelineTaskOutputDataCheckPts() {};
    int64_t timestampOverride() const { return timestamp; }
};

class PipelineTaskOutput {
protected:
    PipelineTaskOutputType m_type;
    std::unique_ptr<PipelineTaskOutputDataCustom> m_customData;
public:
    PipelineTaskOutput() : m_type(PipelineTaskOutputType::UNKNOWN), m_customData() {};
    PipelineTaskOutput(PipelineTaskOutputType type) : m_type(type), m_customData() {};
    PipelineTaskOutput(PipelineTaskOutputType type, std::unique_ptr<PipelineTaskOutputDataCustom>& customData) : m_type(type), m_customData(std::move(customData)) {};
    RGY_ERR waitsync(uint32_t wait = RGY_WAIT_INTERVAL) {
        return RGY_ERR_NONE;
    }
    virtual void depend_clear() {};
    PipelineTaskOutputType type() const { return m_type; }
    const PipelineTaskOutputDataCustom *customdata() const { return m_customData.get(); }
    virtual RGY_ERR write([[maybe_unused]] RGYOutput *writer, [[maybe_unused]] RGYOpenCLQueue *clqueue, [[maybe_unused]] RGYFilterSsim *videoQualityMetric) {
        return RGY_ERR_UNSUPPORTED;
    }
    virtual ~PipelineTaskOutput() {};
};

class PipelineTaskOutputSurf : public PipelineTaskOutput {
protected:
    PipelineTaskSurface m_surf;
    std::unique_ptr<PipelineTaskOutput> m_dependencyFrame;
    std::vector<RGYOpenCLEvent> m_clevents;
public:
    PipelineTaskOutputSurf(PipelineTaskSurface surf) :
        PipelineTaskOutput(PipelineTaskOutputType::SURFACE), m_surf(surf), m_dependencyFrame(), m_clevents() {
    };
    PipelineTaskOutputSurf(PipelineTaskSurface surf, std::unique_ptr<PipelineTaskOutputDataCustom>& customData) :
        PipelineTaskOutput(PipelineTaskOutputType::SURFACE, customData), m_surf(surf), m_dependencyFrame(), m_clevents() {
    };
    PipelineTaskOutputSurf(PipelineTaskSurface surf, std::unique_ptr<PipelineTaskOutput>& dependencyFrame, RGYOpenCLEvent& clevent) :
        PipelineTaskOutput(PipelineTaskOutputType::SURFACE),
        m_surf(surf), m_dependencyFrame(std::move(dependencyFrame)), m_clevents() {
        m_clevents.push_back(clevent);
    };
    virtual ~PipelineTaskOutputSurf() {
        depend_clear();
        m_surf.reset();
    };

    PipelineTaskSurface& surf() { return m_surf; }

    void addClEvent(RGYOpenCLEvent& clevent) {
        m_clevents.push_back(clevent);
    }

    virtual void depend_clear() override {
        RGYOpenCLEvent::wait(m_clevents);
        m_clevents.clear();
        m_dependencyFrame.reset();
    }

    RGY_ERR writeAMF(RGYOutput *writer) {
        auto err = writer->WriteNextFrame(m_surf.frame());
        return err;
    }

    RGY_ERR writeCL(RGYOutput *writer, RGYOpenCLQueue *clqueue) {
        if (clqueue == nullptr) {
            return RGY_ERR_NULL_PTR;
        }
        auto clframe = m_surf.cl();
        auto err = clframe->queueMapBuffer(*clqueue, CL_MAP_READ); // CPUが読み込むためにmapする
        if (err != RGY_ERR_NONE) {
            return err;
        }
        clframe->mapWait();
        auto mappedframe = clframe->mappedHost();
        err = writer->WriteNextFrame(mappedframe);
        clframe->unmapBuffer();
        return err;
    }

    virtual RGY_ERR write([[maybe_unused]] RGYOutput *writer, [[maybe_unused]] RGYOpenCLQueue *clqueue, [[maybe_unused]] RGYFilterSsim *videoQualityMetric) override {
        if (!writer || writer->getOutType() == OUT_TYPE_NONE) {
            return RGY_ERR_NOT_INITIALIZED;
        }
        if (writer->getOutType() != OUT_TYPE_SURFACE) {
            return RGY_ERR_INVALID_OPERATION;
        }
        auto err = (m_surf.amf() != nullptr) ? writeAMF(writer) : writeCL(writer, clqueue);
        return err;
    }
};

class PipelineTaskOutputBitstream : public PipelineTaskOutput {
protected:
    std::shared_ptr<RGYBitstream> m_bs;
public:
    PipelineTaskOutputBitstream(std::shared_ptr<RGYBitstream> bs) : PipelineTaskOutput(PipelineTaskOutputType::BITSTREAM), m_bs(bs) {};
    virtual ~PipelineTaskOutputBitstream() {};

    std::shared_ptr<RGYBitstream>& bitstream() { return m_bs; }

    virtual RGY_ERR write([[maybe_unused]] RGYOutput *writer, [[maybe_unused]] RGYOpenCLQueue *clqueue, [[maybe_unused]] RGYFilterSsim *videoQualityMetric) override {
        if (!writer || writer->getOutType() == OUT_TYPE_NONE) {
            return RGY_ERR_NOT_INITIALIZED;
        }
        if (writer->getOutType() != OUT_TYPE_BITSTREAM) {
            return RGY_ERR_INVALID_OPERATION;
        }
        if (videoQualityMetric) {
            if (!videoQualityMetric->decodeStarted()) {
                videoQualityMetric->initDecode(m_bs.get());
            }
            videoQualityMetric->addBitstream(m_bs.get());
        }
        return writer->WriteNextFrame(m_bs.get());
    }
};

enum class PipelineTaskType {
    UNKNOWN,
    AMFDEC,
    AMFVPP,
    AMFENC,
    INPUT,
    INPUTCL,
    CHECKPTS,
    TRIM,
    AUDIO,
    OUTPUTRAW,
    OPENCL,
    VIDEOMETRIC,
};

static const TCHAR *getPipelineTaskTypeName(PipelineTaskType type) {
    switch (type) {
    case PipelineTaskType::AMFVPP:      return _T("AMFVPP");
    case PipelineTaskType::AMFDEC:      return _T("AMFDEC");
    case PipelineTaskType::AMFENC:      return _T("AMFENC");
    case PipelineTaskType::INPUT:       return _T("INPUT");
    case PipelineTaskType::INPUTCL:     return _T("INPUTCL");
    case PipelineTaskType::CHECKPTS:    return _T("CHECKPTS");
    case PipelineTaskType::TRIM:        return _T("TRIM");
    case PipelineTaskType::OPENCL:      return _T("OPENCL");
    case PipelineTaskType::AUDIO:       return _T("AUDIO");
    case PipelineTaskType::VIDEOMETRIC: return _T("VIDEOMETRIC");
    case PipelineTaskType::OUTPUTRAW:   return _T("OUTRAW");
    default: return _T("UNKNOWN");
    }
}

// Alllocするときの優先度 値が高い方が優先
static const int getPipelineTaskAllocPriority(PipelineTaskType type) {
    switch (type) {
    case PipelineTaskType::AMFENC:    return 3;
    case PipelineTaskType::AMFDEC:    return 2;
    case PipelineTaskType::AMFVPP:    return 1;
    case PipelineTaskType::INPUT:
    case PipelineTaskType::INPUTCL:
    case PipelineTaskType::CHECKPTS:
    case PipelineTaskType::TRIM:
    case PipelineTaskType::OPENCL:
    case PipelineTaskType::AUDIO:
    case PipelineTaskType::OUTPUTRAW:
    case PipelineTaskType::VIDEOMETRIC:
    default: return 0;
    }
}

class PipelineTask {
protected:
    PipelineTaskType m_type;
    amf::AMFContextPtr m_context;
    std::deque<std::unique_ptr<PipelineTaskOutput>> m_outQeueue;
    PipelineTaskSurfaces m_workSurfs;
    int m_inFrames;
    int m_outFrames;
    int m_outMaxQueueSize;
    std::shared_ptr<RGYLog> m_log;
public:
    PipelineTask() : m_type(PipelineTaskType::UNKNOWN), m_context(), m_outQeueue(), m_workSurfs(), m_inFrames(0), m_outFrames(0), m_outMaxQueueSize(0), m_log() {};
    PipelineTask(PipelineTaskType type, amf::AMFContextPtr conetxt, int outMaxQueueSize, std::shared_ptr<RGYLog> log) :
        m_type(type), m_context(conetxt), m_outQeueue(), m_workSurfs(), m_inFrames(0), m_outFrames(0), m_outMaxQueueSize(outMaxQueueSize), m_log(log) {
    };
    virtual ~PipelineTask() {
        m_workSurfs.clear();
    }
    virtual bool isPassThrough() const { return false; }
    virtual tstring print() const { return getPipelineTaskTypeName(m_type); }
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() = 0;
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() = 0;
    virtual RGY_ERR sendFrame(std::unique_ptr<PipelineTaskOutput>& frame) = 0;
    virtual RGY_ERR getOutputFrameInfo(RGYFrameInfo& info) { info = RGYFrameInfo(); return RGY_ERR_NONE; }
    virtual std::vector<std::unique_ptr<PipelineTaskOutput>> getOutput(const bool sync) {
        std::vector<std::unique_ptr<PipelineTaskOutput>> output;
        while ((int)m_outQeueue.size() > m_outMaxQueueSize) {
            auto out = std::move(m_outQeueue.front());
            m_outQeueue.pop_front();
            if (sync) {
                out->waitsync();
            }
            out->depend_clear();
            m_outFrames++;
            output.push_back(std::move(out));
        }
        return output;
    }
    bool isAMFTask() const { return isAMFTask(m_type); }
    bool isAMFTask(const PipelineTaskType task) const {
        return task == PipelineTaskType::AMFDEC
            || task == PipelineTaskType::AMFVPP
            || task == PipelineTaskType::AMFENC;
    }
    // mfx関連とそうでないtaskのやり取りでロックが必要
    bool requireSync(const PipelineTaskType nextTaskType) const {
        return isAMFTask(m_type) != isAMFTask(nextTaskType);
    }
    int workSurfacesAllocPriority() const {
        return getPipelineTaskAllocPriority(m_type);
    }
    size_t workSurfacesCount() const {
        return m_workSurfs.bufCount();
    }

    void PrintMes(RGYLogLevel log_level, const TCHAR *format, ...) {
        if (m_log.get() == nullptr) {
            if (log_level <= RGY_LOG_INFO) {
                return;
            }
        } else if (log_level < m_log->getLogLevel(RGY_LOGT_CORE)) {
            return;
        }

        va_list args;
        va_start(args, format);

        int len = _vsctprintf(format, args) + 1; // _vscprintf doesn't count terminating '\0'
        vector<TCHAR> buffer(len, 0);
        _vstprintf_s(buffer.data(), len, format, args);
        va_end(args);

        tstring mes = getPipelineTaskTypeName(m_type) + tstring(_T(": ")) + buffer.data();

        if (m_log.get() != nullptr) {
            m_log->write(log_level, RGY_LOGT_CORE, mes.c_str());
        } else {
            _ftprintf(stderr, _T("%s"), mes.c_str());
        }
    }
protected:
    RGY_ERR workSurfacesClear() {
        if (m_outQeueue.size() != 0) {
            return RGY_ERR_UNSUPPORTED;
        }
        if (!m_workSurfs.isAllFree()) {
            return RGY_ERR_UNSUPPORTED;
        }
        return RGY_ERR_NONE;
    }
public:
    RGY_ERR workSurfacesAllocCL(const int numFrames, const RGYFrameInfo &frame, RGYOpenCLContext *cl) {
        auto sts = workSurfacesClear();
        if (sts != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("allocWorkSurfaces:   Failed to clear old surfaces: %s.\n"), get_err_mes(sts));
            return sts;
        }
        PrintMes(RGY_LOG_DEBUG, _T("allocWorkSurfaces:   cleared old surfaces: %s.\n"), get_err_mes(sts));

        // OpenCLフレームの確保
        std::vector<std::unique_ptr<RGYCLFrame>> frames(numFrames);
        for (size_t i = 0; i < frames.size(); i++) {
            //CPUとのやり取りが効率化できるよう、CL_MEM_ALLOC_HOST_PTR を指定する
            //これでmap/unmapで可能な場合コピーが発生しない
            frames[i] = cl->createFrameBuffer(frame, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR);
        }
        m_workSurfs.setSurfaces(frames);
        return RGY_ERR_NONE;
    }
    PipelineTaskSurfaceType workSurfaceType() const {
        if (m_workSurfs.bufCount() == 0) {
            return PipelineTaskSurfaceType::UNKNOWN;
        }
        return m_workSurfs.type();
    }
    // 使用中でないフレームを探してきて、参照カウンタを加算したものを返す
    // 破棄時にアプリ側の参照カウンタを減算するようにshared_ptrで設定してある
    PipelineTaskSurface getWorkSurf() {
        if (m_workSurfs.bufCount() == 0) {
            PrintMes(RGY_LOG_ERROR, _T("getWorkSurf:   No buffer allocated!\n"));
            return PipelineTaskSurface();
        }
        for (int i = 0; i < RGY_WAIT_INTERVAL; i++) {
            PipelineTaskSurface s = m_workSurfs.getFreeSurf();
            if (s != nullptr) {
                return s;
            }
            sleep_hybrid(i);
        }
        PrintMes(RGY_LOG_ERROR, _T("getWorkSurf:   Failed to get work surface, all %d frames used.\n"), m_workSurfs.bufCount());
        return PipelineTaskSurface();
    }

    void setOutputMaxQueueSize(int size) { m_outMaxQueueSize = size; }

    PipelineTaskType taskType() const { return m_type; }
    int inputFrames() const { return m_inFrames; }
    int outputFrames() const { return m_outFrames; }
    int outputMaxQueueSize() const { return m_outMaxQueueSize; }
};

class PipelineTaskInput : public PipelineTask {
    RGYInput *m_input;
    std::shared_ptr<RGYOpenCLContext> m_cl;
public:
    PipelineTaskInput(amf::AMFContextPtr context, int outMaxQueueSize, RGYInput *input, std::shared_ptr<RGYOpenCLContext> cl, std::shared_ptr<RGYLog> log)
        : PipelineTask(PipelineTaskType::INPUT, context, outMaxQueueSize, log), m_input(input), m_cl(cl) {

    };
    virtual ~PipelineTaskInput() {};
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override { return std::nullopt; };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override {
        const auto inputFrameInfo = m_input->GetInputFrameInfo();
        RGYFrameInfo info(inputFrameInfo.srcWidth, inputFrameInfo.srcHeight, inputFrameInfo.csp, inputFrameInfo.bitdepth, inputFrameInfo.picstruct, RGY_MEM_TYPE_GPU);
        return std::make_pair(info, m_outMaxQueueSize);
    };
    RGY_ERR LoadNextFrameCL() {
        auto surfWork = getWorkSurf();
        if (surfWork == nullptr) {
            PrintMes(RGY_LOG_ERROR, _T("failed to get work surface for input.\n"));
            return RGY_ERR_NOT_ENOUGH_BUFFER;
        }
        auto clframe = surfWork.cl();
        auto err = clframe->queueMapBuffer(m_cl->queue(), CL_MAP_WRITE); // CPUが書き込むためにMapする
        if (err != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to map buffer: %s.\n"), get_err_mes(err));
            return err;
        }
        clframe->mapWait(); //すぐ終わるはず
        auto mappedframe = clframe->mappedHost();
        err = m_input->LoadNextFrame(mappedframe);
        if (err != RGY_ERR_NONE) {
            //Unlockする必要があるので、ここに入ってもすぐにreturnしてはいけない
            if (err == RGY_ERR_MORE_DATA) { // EOF
                err = RGY_ERR_MORE_BITSTREAM; // EOF を PipelineTaskMFXDecode のreturnコードに合わせる
            } else {
                PrintMes(RGY_LOG_ERROR, _T("Error in reader: %s.\n"), get_err_mes(err));
            }
        }
        clframe->setPropertyFrom(mappedframe);
        auto clerr = clframe->unmapBuffer();
        if (clerr != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to unmap buffer: %s.\n"), get_err_mes(err));
            if (err == RGY_ERR_NONE) {
                err = clerr;
            }
        }
        if (err == RGY_ERR_NONE) {
            surfWork.frame()->setDuration(mappedframe->duration());
            surfWork.frame()->setTimestamp(mappedframe->timestamp());
            surfWork.frame()->setInputFrameId(mappedframe->inputFrameId());
            surfWork.frame()->setPicstruct(mappedframe->picstruct());
            surfWork.frame()->setFlags(mappedframe->flags());
            surfWork.frame()->setDataList(mappedframe->dataList());
            surfWork.frame()->setInputFrameId(m_inFrames++);
            m_outQeueue.push_back(std::make_unique<PipelineTaskOutputSurf>(surfWork));
        }
        return err;
    }
    RGY_ERR LoadNextFrameAMF() {
        const auto inputFrameInfo = m_input->GetInputFrameInfo();
        amf::AMFSurfacePtr pSurface;
        auto ar = m_context->AllocSurface(amf::AMF_MEMORY_HOST, csp_rgy_to_enc(inputFrameInfo.csp),
            inputFrameInfo.srcWidth - inputFrameInfo.crop.e.left - inputFrameInfo.crop.e.right,
            inputFrameInfo.srcHeight - inputFrameInfo.crop.e.bottom - inputFrameInfo.crop.e.up,
            &pSurface);
        if (ar != AMF_OK) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to allocate surface: %s.\n"), get_err_mes(err_to_rgy(ar)));
            return err_to_rgy(ar);
        }
        pSurface->SetFrameType(frametype_rgy_to_enc(inputFrameInfo.picstruct));
        auto inputFrame = std::make_unique<RGYFrameAMF>(pSurface);
        auto err = m_input->LoadNextFrame(inputFrame.get());
        if (err == RGY_ERR_MORE_DATA) {// EOF
            err = RGY_ERR_MORE_BITSTREAM; // EOF を PipelineTaskMFXDecode のreturnコードに合わせる
        } else if (err != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("Error in reader: %s.\n"), get_err_mes(err));
        } else {
            inputFrame->setInputFrameId(m_inFrames++);
            m_outQeueue.push_back(std::make_unique<PipelineTaskOutputSurf>(m_workSurfs.addSurface(inputFrame)));
        }
        return err;
    }
    virtual RGY_ERR sendFrame([[maybe_unused]] std::unique_ptr<PipelineTaskOutput>& frame) override {
        if (workSurfaceType() == PipelineTaskSurfaceType::CL) {
            return LoadNextFrameCL();
        }
        return LoadNextFrameAMF();
    }
};

class PipelineTaskAMFDecode : public PipelineTask {
protected:
    struct FrameFlags {
        int64_t timestamp;
        RGY_FRAME_FLAGS flags;

        FrameFlags() : timestamp(AV_NOPTS_VALUE), flags(RGY_FRAME_FLAG_NONE) {};
        FrameFlags(int64_t pts, RGY_FRAME_FLAGS f) : timestamp(pts), flags(f) {};
    };
    amf::AMFComponentPtr m_dec;
    RGYInput *m_input;
    RGYQueueMPMP<RGYFrameDataMetadata*> m_queueHDR10plusMetadata;
    RGYQueueMPMP<FrameFlags> m_dataFlag;
    RGYRunState m_state;
    int m_decOutFrames;
#if THREAD_DEC_USE_FUTURE
    std::future m_thDecoder;
#else
    std::thread m_thDecoder;
#endif //#if THREAD_DEC_USE_FUTURE
public:
    PipelineTaskAMFDecode(amf::AMFComponentPtr dec, amf::AMFContextPtr context, int outMaxQueueSize, RGYInput *input, std::shared_ptr<RGYLog> log)
        : PipelineTask(PipelineTaskType::AMFDEC, context, outMaxQueueSize, log), m_dec(dec), m_input(input),
        m_queueHDR10plusMetadata(), m_dataFlag(),
        m_state(RGY_STATE_STOPPED), m_decOutFrames(0), m_thDecoder() {
        m_queueHDR10plusMetadata.init(256);
        m_dataFlag.init();
    };
    virtual ~PipelineTaskAMFDecode() {
        m_state = RGY_STATE_ABORT;
        closeThread();
        m_queueHDR10plusMetadata.close([](RGYFrameDataMetadata **ptr) { if (*ptr) { delete *ptr; *ptr = nullptr; }; });
    };
    void setDec(amf::AMFComponentPtr dec) { m_dec = dec; };

    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override { return std::nullopt; };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override {
        const auto inputFrameInfo = m_input->GetInputFrameInfo();
        RGYFrameInfo info(inputFrameInfo.srcWidth, inputFrameInfo.srcHeight, inputFrameInfo.csp, inputFrameInfo.bitdepth, inputFrameInfo.picstruct, RGY_MEM_TYPE_GPU);
        return std::make_pair(info, 0);
    };

    void closeThread() {
#if THREAD_DEC_USE_FUTURE
        if (m_thDecoder.valid()) {
            while (m_thDecoder.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
#else
        if (m_thDecoder.joinable()) {
            while (RGYThreadStillActive(m_thDecoder.native_handle())) {
#endif
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
#if !THREAD_DEC_USE_FUTURE
            // linuxでは、これがRGYThreadStillActiveのwhile文を抜けるときに行われるため、
            // これを呼ぶとエラーになってしまう
            m_thDecoder.join();
#endif
        }
    }
    RGY_ERR startThread() {
        m_state = RGY_STATE_RUNNING;
#if THREAD_DEC_USE_FUTURE
        m_thDecoder = std::async(std::launch::async, [this]() {
#else
        m_thDecoder = std::thread([this]() {
#endif //#if THREAD_DEC_USE_FUTURE
            const auto VCE_TIMEBASE = rgy_rational<int>(1, AMF_SECOND); // In 100 NanoSeconds
            RGYBitstream bitstream = RGYBitstreamInit();
            RGY_ERR sts = RGY_ERR_NONE;
            for (int i = 0; sts == RGY_ERR_NONE && m_state == RGY_STATE_RUNNING; i++) {
                if (((  sts = m_input->LoadNextFrame(nullptr)) != RGY_ERR_NONE //進捗表示のため
                    || (sts = m_input->GetNextBitstream(&bitstream)) != RGY_ERR_NONE)
                    && sts != RGY_ERR_MORE_DATA) {
                    m_state = RGY_STATE_ERROR;
                    break;
                }

                amf::AMFBufferPtr pictureBuffer;
                if (sts == RGY_ERR_NONE) {
                    m_inFrames++;

                    auto ar = m_context->AllocBuffer(amf::AMF_MEMORY_HOST, bitstream.size(), &pictureBuffer);
                    if (ar != AMF_OK) {
                        return err_to_rgy(ar);
                    }
                    memcpy(pictureBuffer->GetNative(), bitstream.data(), bitstream.size());

                    //const auto duration = rgy_change_scale(bitstream.duration(), to_rgy(inTimebase), VCE_TIMEBASE);
                    //const auto pts = rgy_change_scale(bitstream.pts(), to_rgy(inTimebase), VCE_TIMEBASE);
                    pictureBuffer->SetDuration(bitstream.duration());
                    pictureBuffer->SetPts(bitstream.pts());

                    for (auto& frameData : bitstream.getFrameDataList()) {
                        if (frameData->dataType() == RGY_FRAME_DATA_HDR10PLUS) {
                            auto ptr = dynamic_cast<RGYFrameDataHDR10plus*>(frameData);
                            if (ptr) {
                                m_queueHDR10plusMetadata.push(new RGYFrameDataHDR10plus(*ptr));
                            }
                        } else if (frameData->dataType() == RGY_FRAME_DATA_DOVIRPU) {
                            auto ptr = dynamic_cast<RGYFrameDataDOVIRpu*>(frameData);
                            if (ptr) {
                                m_queueHDR10plusMetadata.push(new RGYFrameDataDOVIRpu(*ptr));
                            }
                        }
                    }
                    const auto flags = FrameFlags(bitstream.pts(), (RGY_FRAME_FLAGS)bitstream.dataflag());
                    m_dataFlag.push(flags);
                }
                bitstream.setSize(0);
                bitstream.setOffset(0);
                if (pictureBuffer || sts == RGY_ERR_MORE_BITSTREAM /*EOFの場合はDrainを送る*/) {
                    auto ar = AMF_OK;
                    do {
                        if (sts == RGY_ERR_MORE_BITSTREAM) {
                            ar = m_dec->Drain();
                        } else {
                            try {
                                ar = m_dec->SubmitInput(pictureBuffer);
                            } catch (...) {
                                PrintMes(RGY_LOG_ERROR, _T("ERROR: Unexpected error while submitting bitstream to decoder.\n"));
                                ar = AMF_UNEXPECTED;
                            }
                        }
                        if (ar == AMF_NEED_MORE_INPUT) {
                            break;
                        } else if (ar == AMF_RESOLUTION_CHANGED || ar == AMF_RESOLUTION_UPDATED) {
                            PrintMes(RGY_LOG_ERROR, _T("ERROR: Resolution changed during decoding.\n"));
                            break;
                        } else if (ar == AMF_INPUT_FULL || ar == AMF_DECODER_NO_FREE_SURFACES) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        } else if (ar == AMF_REPEAT) {
                            pictureBuffer = nullptr;
                        } else {
                            break;
                        }
                    } while (m_state == RGY_STATE_RUNNING);
                    if (ar != AMF_OK) {
                        m_state = RGY_STATE_ERROR;
                        return err_to_rgy(ar);
                    }
                }
            }
            m_dec->Drain();
            bitstream.clear();
            return sts;
        });
        PrintMes(RGY_LOG_DEBUG, _T("Started Decode thread.\n"));
        return RGY_ERR_NONE;
    }

    //データを使用すると、bitstreamのsizeを0にする
    //これを確認すること
    RGY_ERR sendFrame(RGYBitstream *bitstream) {
        return getOutput();
    }
    virtual RGY_ERR sendFrame([[maybe_unused]] std::unique_ptr<PipelineTaskOutput>& frame) override {
        return getOutput();
    }
protected:
    RGY_ERR getOutput() {
        auto ret = RGY_ERR_NONE;
        amf::AMFSurfacePtr surfDecOut;
        auto ar = AMF_REPEAT;
        auto timeS = std::chrono::system_clock::now();
        if (m_state == RGY_STATE_STOPPED) {
            m_state = RGY_STATE_RUNNING;
            startThread();
        }
        while (m_state == RGY_STATE_RUNNING) {
            amf::AMFDataPtr data;
            try {
                ar = m_dec->QueryOutput(&data);
            } catch (...) {
                PrintMes(RGY_LOG_ERROR, _T("ERROR: Unexpected error while getting frame from decoder.\n"));
                ar = AMF_UNEXPECTED;
            }
            if (ar == AMF_EOF) {
                break;
            }
            if (ar == AMF_REPEAT) {
                ar = AMF_OK; //これ重要...ここが欠けると最後の数フレームが欠落する
            }
            if (ar == AMF_OK && data != nullptr) {
                surfDecOut = amf::AMFSurfacePtr(data);
                break;
            }
            if (ar != AMF_OK) break;
            if (false && (std::chrono::system_clock::now() - timeS) > std::chrono::seconds(10)) {
                PrintMes(RGY_LOG_ERROR, _T("10 sec has passed after getting last frame from decoder.\n"));
                PrintMes(RGY_LOG_ERROR, _T("Decoder seems to have crushed.\n"));
                ar = AMF_FAIL;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (ar == AMF_EOF) {
            ret = RGY_ERR_MORE_BITSTREAM;
        } else if (ar != AMF_OK) {
            ret = err_to_rgy(ar); m_state = RGY_STATE_ERROR;
            PrintMes(RGY_LOG_ERROR, _T("Failed to load input frame: %s.\n"), get_err_mes(ret));
            return ret;
        }
        if (surfDecOut != nullptr) {
            // pre-analysis使用時などに発生するSubmitInput時のAMF_DECODER_NO_FREE_SURFACESを回避するため、
            // 明示的にsurface->Duplicateを行ってコピーを下流に渡していく
            // デコーダのオプションのAMF_VIDEO_DECODER_SURFACE_COPYでも同様になるはずだが、
            // メモリ確保エラーが発生することがある(AMF_DIRECTX_FAIL)ので、明示的なコピーのほうがよい
            amf::AMFDataPtr dataCopy;
            ar = surfDecOut->Duplicate(surfDecOut->GetMemoryType(), &dataCopy);
            if (ar != AMF_OK) {
                ret = err_to_rgy(ar); m_state = RGY_STATE_ERROR;
                PrintMes(RGY_LOG_ERROR, _T("Faield to copy decoded frame: %s.\n"), get_err_mes(ret));
                return ret;
            }
            if (auto surfCopy = amf::AMFSurfacePtr(dataCopy); surfCopy != nullptr) {
                auto surfDecOutCopy = std::make_unique<RGYFrameAMF>(surfCopy);
                surfDecOutCopy->setInputFrameId(m_decOutFrames++);

                auto flags = RGY_FRAME_FLAG_NONE;
                if (getDataFlag(surfDecOutCopy->timestamp()) & RGY_FRAME_FLAG_RFF) {
                    flags |= RGY_FRAME_FLAG_RFF;
                }
                surfDecOutCopy->setFlags(flags);

                surfDecOutCopy->clearDataList();
                if (auto data = getMetadata(RGY_FRAME_DATA_HDR10PLUS, surfDecOutCopy->timestamp()); data) {
                    surfDecOutCopy->dataList().push_back(data);
                }
                if (auto data = getMetadata(RGY_FRAME_DATA_DOVIRPU, surfDecOutCopy->timestamp()); data) {
                    surfDecOutCopy->dataList().push_back(data);
                }
                m_outQeueue.push_back(std::make_unique<PipelineTaskOutputSurf>(m_workSurfs.addSurface(surfDecOutCopy)));
            }
        }
        return ret;
    }
    RGY_FRAME_FLAGS getDataFlag(const int64_t timestamp) {
        FrameFlags pts_flag;
        while (m_dataFlag.front_copy_no_lock(&pts_flag)) {
            if (pts_flag.timestamp < timestamp || pts_flag.timestamp == AV_NOPTS_VALUE) {
                m_dataFlag.pop();
            } else {
                break;
            }
        }
        size_t queueSize = m_dataFlag.size();
        for (uint32_t i = 0; i < queueSize; i++) {
            if (m_dataFlag.copy(&pts_flag, i, &queueSize)) {
                if (pts_flag.timestamp == timestamp) {
                    return pts_flag.flags;
                }
            }
        }
        return RGY_FRAME_FLAG_NONE;
    }
    std::shared_ptr<RGYFrameData> getMetadata(const RGYFrameDataType datatype, const int64_t timestamp) {
        std::shared_ptr<RGYFrameData> frameData;
        RGYFrameDataMetadata *frameDataPtr = nullptr;
        while (m_queueHDR10plusMetadata.front_copy_no_lock(&frameDataPtr)) {
            if (frameDataPtr->timestamp() < timestamp) {
                m_queueHDR10plusMetadata.pop();
                delete frameDataPtr;
            } else {
                break;
            }
        }
        size_t queueSize = m_queueHDR10plusMetadata.size();
        for (uint32_t i = 0; i < queueSize; i++) {
            if (m_queueHDR10plusMetadata.copy(&frameDataPtr, i, &queueSize)) {
                if (frameDataPtr->timestamp() == timestamp && frameDataPtr->dataType() == datatype) {
                    if (frameDataPtr->dataType() == RGY_FRAME_DATA_HDR10PLUS) {
                        auto ptr = dynamic_cast<RGYFrameDataHDR10plus*>(frameDataPtr);
                        if (ptr) {
                            frameData = std::make_shared<RGYFrameDataHDR10plus>(*ptr);
                        }
                    } else if (frameData->dataType() == RGY_FRAME_DATA_DOVIRPU) {
                        auto ptr = dynamic_cast<RGYFrameDataDOVIRpu*>(frameDataPtr);
                        if (ptr) {
                            frameData = std::make_shared<RGYFrameDataDOVIRpu>(*ptr);
                        }
                    }
                    break;
                }
            }
        }
        return frameData;
    };
};

class PipelineTaskCheckPTS : public PipelineTask {
protected:
    rgy_rational<int> m_srcTimebase;
    rgy_rational<int> m_streamTimebase;
    rgy_rational<int> m_outputTimebase;
    RGYAVSync m_avsync;
    bool m_vpp_rff;
    bool m_vpp_afs_rff_aware;
    int64_t m_outFrameDuration; //(m_outputTimebase基準)
    int64_t m_tsOutFirst;     //(m_outputTimebase基準)
    int64_t m_tsOutEstimated; //(m_outputTimebase基準)
    int64_t m_tsPrev;         //(m_outputTimebase基準)
    uint32_t m_inputFramePosIdx;
    FramePosList *m_framePosList;
public:
    PipelineTaskCheckPTS(amf::AMFContextPtr context, rgy_rational<int> srcTimebase, rgy_rational<int> streamTimebase, rgy_rational<int> outputTimebase, int64_t outFrameDuration, RGYAVSync avsync, bool vpp_afs_rff_aware, FramePosList *framePosList, std::shared_ptr<RGYLog> log) :
        PipelineTask(PipelineTaskType::CHECKPTS, context, /*outMaxQueueSize = */ 0 /*常に0である必要がある*/, log),
        m_srcTimebase(srcTimebase), m_streamTimebase(streamTimebase), m_outputTimebase(outputTimebase), m_avsync(avsync), m_vpp_rff(false), m_vpp_afs_rff_aware(vpp_afs_rff_aware), m_outFrameDuration(outFrameDuration),
        m_tsOutFirst(-1), m_tsOutEstimated(0), m_tsPrev(-1), m_inputFramePosIdx(std::numeric_limits<decltype(m_inputFramePosIdx)>::max()), m_framePosList(framePosList) {
    };
    virtual ~PipelineTaskCheckPTS() {};

    virtual bool isPassThrough() const override {
        // そのまま渡すのでpaththrough
        return true;
    }
    static const int MAX_FORCECFR_INSERT_FRAMES = 1024; //事実上の無制限
public:
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override { return std::nullopt; };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override { return std::nullopt; };

    virtual RGY_ERR sendFrame(std::unique_ptr<PipelineTaskOutput>& frame) override {
        if (!frame) {
            //PipelineTaskCheckPTSは、getOutputで1フレームずつしか取り出さない
            //そのためm_outQeueueにまだフレームが残っている可能性がある
            return (m_outQeueue.size() > 0) ? RGY_ERR_MORE_SURFACE : RGY_ERR_MORE_DATA;
        }
        int64_t outPtsSource = m_tsOutEstimated; //(m_outputTimebase基準)
        int64_t outDuration = m_outFrameDuration; //入力fpsに従ったduration

        PipelineTaskOutputSurf *taskSurf = dynamic_cast<PipelineTaskOutputSurf *>(frame.get());
        if (taskSurf == nullptr) {
            PrintMes(RGY_LOG_ERROR, _T("Invalid frame type: failed to cast to PipelineTaskOutputSurf.\n"));
            return RGY_ERR_UNSUPPORTED;
        }

        if ((m_srcTimebase.n() > 0 && m_srcTimebase.is_valid())
            && ((m_avsync & (RGY_AVSYNC_VFR | RGY_AVSYNC_FORCE_CFR)) || m_vpp_rff || m_vpp_afs_rff_aware)) {
            //CFR仮定ではなく、オリジナルの時間を見る
            const auto srcTimestamp = taskSurf->surf().frame()->timestamp();
            outPtsSource = rational_rescale(srcTimestamp, m_srcTimebase, m_outputTimebase);
            if (taskSurf->surf().frame()->duration() > 0) {
                outDuration = rational_rescale(taskSurf->surf().frame()->duration(), m_srcTimebase, m_outputTimebase);
                taskSurf->surf().frame()->setDuration(outDuration);
            }
        }
        PrintMes(RGY_LOG_TRACE, _T("check_pts(%d/%d): nOutEstimatedPts %lld, outPtsSource %lld, outDuration %d\n"), taskSurf->surf().frame()->inputFrameId(), m_inFrames, m_tsOutEstimated, outPtsSource, outDuration);
        if (m_tsOutFirst < 0) {
            m_tsOutFirst = outPtsSource; //最初のpts
            PrintMes(RGY_LOG_TRACE, _T("check_pts: m_tsOutFirst %lld\n"), outPtsSource);
        }
        //最初のptsを0に修正
        outPtsSource -= m_tsOutFirst;

        if ((m_avsync & RGY_AVSYNC_VFR) || m_vpp_rff || m_vpp_afs_rff_aware) {
            if (m_vpp_rff || m_vpp_afs_rff_aware) {
                if (std::abs(outPtsSource - m_tsOutEstimated) >= 32 * m_outFrameDuration) {
                    PrintMes(RGY_LOG_TRACE, _T("check_pts: detected gap %lld, changing offset.\n"), outPtsSource, std::abs(outPtsSource - m_tsOutEstimated));
                    //timestampに一定以上の差があればそれを無視する
                    m_tsOutFirst += (outPtsSource - m_tsOutEstimated); //今後の位置合わせのための補正
                    outPtsSource = m_tsOutEstimated;
                    PrintMes(RGY_LOG_TRACE, _T("check_pts:   changed to m_tsOutFirst %lld, outPtsSource %lld.\n"), m_tsOutFirst, outPtsSource);
                }
                auto ptsDiff = outPtsSource - m_tsOutEstimated;
                if (ptsDiff <= std::min<int64_t>(-1, -1 * m_outFrameDuration * 7 / 8)) {
                    //間引きが必要
                    PrintMes(RGY_LOG_TRACE, _T("check_pts(%d):   skipping frame (vfr)\n"), taskSurf->surf().frame()->inputFrameId());
                    return RGY_ERR_MORE_SURFACE;
                }
                // 少しのずれはrffによるものとみなし、基準値を修正する
                m_tsOutEstimated = outPtsSource;
            }
            if (ENCODER_VCEENC && m_framePosList) {
                //cuvidデコード時は、timebaseの分子はかならず1なので、streamIn->time_baseとズレているかもしれないのでオリジナルを計算
                const auto orig_pts = rational_rescale(taskSurf->surf().frame()->timestamp(), m_srcTimebase, m_streamTimebase);
                //ptsからフレーム情報を取得する
                const auto framePos = m_framePosList->findpts(orig_pts, &m_inputFramePosIdx);
                PrintMes(RGY_LOG_TRACE, _T("check_pts(%d):   estimetaed orig_pts %lld, framePos %d\n"), taskSurf->surf().frame()->inputFrameId(), orig_pts, framePos.poc);
                if (framePos.poc != FRAMEPOS_POC_INVALID && framePos.duration > 0) {
                    //有効な値ならオリジナルのdurationを使用する
                    outDuration = rational_rescale(framePos.duration, m_streamTimebase, m_outputTimebase);
                    PrintMes(RGY_LOG_TRACE, _T("check_pts(%d):   changing duration to original: %d\n"), taskSurf->surf().frame()->inputFrameId(), outDuration);
                }
            }
        }
        if (m_avsync & RGY_AVSYNC_FORCE_CFR) {
            if (std::abs(outPtsSource - m_tsOutEstimated) >= CHECK_PTS_MAX_INSERT_FRAMES * m_outFrameDuration) {
                //timestampに一定以上の差があればそれを無視する
                m_tsOutFirst += (outPtsSource - m_tsOutEstimated); //今後の位置合わせのための補正
                outPtsSource = m_tsOutEstimated;
                PrintMes(RGY_LOG_WARN, _T("Big Gap was found between 2 frames, avsync might be corrupted.\n"));
                PrintMes(RGY_LOG_TRACE, _T("check_pts:   changed to m_tsOutFirst %lld, outPtsSource %lld.\n"), m_tsOutFirst, outPtsSource);
            }
            auto ptsDiff = outPtsSource - m_tsOutEstimated;
            if (ptsDiff <= std::min<int64_t>(-1, -1 * m_outFrameDuration * 7 / 8)) {
                //間引きが必要
                PrintMes(RGY_LOG_DEBUG, _T("Drop frame: framepts %lld, estimated next %lld, diff %lld [%.1f]\n"), outPtsSource, m_tsOutEstimated, ptsDiff, ptsDiff / (double)m_outFrameDuration);
                return RGY_ERR_MORE_SURFACE;
            }
            while (ptsDiff >= std::max<int64_t>(1, m_outFrameDuration * 7 / 8)) {
                PrintMes(RGY_LOG_DEBUG, _T("Insert frame: framepts %lld, estimated next %lld, diff %lld [%.1f]\n"), outPtsSource, m_tsOutEstimated, ptsDiff, ptsDiff / (double)m_outFrameDuration);
                //水増しが必要
                if (auto surf = taskSurf->surf().amf(); surf != nullptr) {
                    //AMFのsurfaceの場合はコピーを作成する
                    amf::AMFDataPtr dataCopy;
                    auto ar = surf->amf()->Duplicate(surf->amf()->GetMemoryType(), &dataCopy);
                    if (ar != AMF_OK) {
                        PrintMes(RGY_LOG_ERROR, _T("Failed to copy frame: %s.\n"), get_err_mes(err_to_rgy(ar)));
                        return err_to_rgy(ar);
                    }
                    auto surfCopy = amf::AMFSurfacePtr(dataCopy);
                    auto surfOutCopy = std::make_unique<RGYFrameAMF>(surfCopy);
                    surfOutCopy->setDataList(taskSurf->surf().frame()->dataList());
                    surfOutCopy->setInputFrameId(taskSurf->surf().frame()->inputFrameId());
                    surfOutCopy->setTimestamp(m_tsOutEstimated);
                    if (ENCODER_VCEENC) {
                        surfOutCopy->setDuration(outDuration);
                    }
                    m_outQeueue.push_back(std::make_unique<PipelineTaskOutputSurf>(m_workSurfs.addSurface(surfOutCopy)));
                } else {
                    PipelineTaskSurface surfVppOut = taskSurf->surf();
                    surfVppOut.frame()->setInputFrameId(taskSurf->surf().frame()->inputFrameId());
                    surfVppOut.frame()->setTimestamp(m_tsOutEstimated);
                    if (ENCODER_VCEENC) {
                        surfVppOut.frame()->setDuration(outDuration);
                    }
                    //timestampの上書き情報
                    //surfVppOut内部のmfxSurface1自体は同じデータを指すため、複数のタイムスタンプを持つことができない
                    //この問題をm_outQeueueのPipelineTaskOutput(これは個別)に与えるPipelineTaskOutputDataCheckPtsの値で、
                    //PipelineTaskCheckPTS::getOutput時にtimestampを変更するようにする
                    //そのため、checkptsからgetOutputしたフレームは
                    //(次にPipelineTaskCheckPTS::getOutputを呼ぶより前に)直ちに後続タスクに投入するよう制御する必要がある
                    std::unique_ptr<PipelineTaskOutputDataCustom> timestampOverride(new PipelineTaskOutputDataCheckPts(m_tsOutEstimated));
                    m_outQeueue.push_back(std::make_unique<PipelineTaskOutputSurf>(surfVppOut, timestampOverride));
                }
                m_tsOutEstimated += m_outFrameDuration;
                ptsDiff = outPtsSource - m_tsOutEstimated;
            }
            outPtsSource = m_tsOutEstimated;
        }
        if (m_tsPrev >= outPtsSource) {
            if (m_tsPrev - outPtsSource >= MAX_FORCECFR_INSERT_FRAMES * m_outFrameDuration) {
                PrintMes(RGY_LOG_DEBUG, _T("check_pts: previous pts %lld, current pts %lld, estimated pts %lld, m_tsOutFirst %lld, changing offset.\n"), m_tsPrev, outPtsSource, m_tsOutEstimated, m_tsOutFirst);
                m_tsOutFirst += (outPtsSource - m_tsOutEstimated); //今後の位置合わせのための補正
                outPtsSource = m_tsOutEstimated;
                PrintMes(RGY_LOG_DEBUG, _T("check_pts:   changed to m_tsOutFirst %lld, outPtsSource %lld.\n"), m_tsOutFirst, outPtsSource);
            } else {
                if (m_avsync & RGY_AVSYNC_FORCE_CFR) {
                    //間引きが必要
                    PrintMes(RGY_LOG_WARN, _T("check_pts(%d/%d): timestamp of video frame is smaller than previous frame, skipping frame: previous pts %lld, current pts %lld.\n"),
                        taskSurf->surf().frame()->inputFrameId(), m_inFrames, m_tsPrev, outPtsSource);
                    return RGY_ERR_MORE_SURFACE;
                } else {
                    const auto origPts = outPtsSource;
                    outPtsSource = m_tsPrev + std::max<int64_t>(1, m_outFrameDuration / 4);
                    PrintMes(RGY_LOG_WARN, _T("check_pts(%d/%d): timestamp of video frame is smaller than previous frame, changing pts: %lld -> %lld (previous pts %lld).\n"),
                        taskSurf->surf().frame()->inputFrameId(), m_inFrames, origPts, outPtsSource, m_tsPrev);
                }
            }
        }

        //次のフレームのptsの予想
        m_inFrames++;
        m_tsOutEstimated += outDuration;
        m_tsPrev = outPtsSource;
        PipelineTaskSurface outSurf = taskSurf->surf();
        outSurf.frame()->setInputFrameId(taskSurf->surf().frame()->inputFrameId());
        outSurf.frame()->setTimestamp(outPtsSource);
        if (ENCODER_VCEENC) {
            outSurf.frame()->setDuration(outDuration);
        }
        std::unique_ptr<PipelineTaskOutputDataCustom> timestampOverride(new PipelineTaskOutputDataCheckPts(outPtsSource));
        m_outQeueue.push_back(std::make_unique<PipelineTaskOutputSurf>(outSurf, timestampOverride));
        return RGY_ERR_NONE;
    }
    //checkptsではtimestampを上書きするため特別に常に1フレームしか取り出さない
    //これは--avsync frocecfrでフレームを参照コピーする際、
    //mfxSurface1自体は同じデータを指すため、複数のタイムスタンプを持つことができないため、
    //1フレームずつgetOutputし、都度タイムスタンプを上書きしてすぐに後続のタスクに投入してタイムスタンプを反映させる必要があるため
    virtual std::vector<std::unique_ptr<PipelineTaskOutput>> getOutput(const bool sync) override {
        std::vector<std::unique_ptr<PipelineTaskOutput>> output;
        if ((int)m_outQeueue.size() > m_outMaxQueueSize) {
            auto out = std::move(m_outQeueue.front());
            m_outQeueue.pop_front();
            if (sync) {
                out->waitsync();
            }
            out->depend_clear();
            if (out->customdata() != nullptr) {
                const auto dataCheckPts = dynamic_cast<const PipelineTaskOutputDataCheckPts *>(out->customdata());
                if (dataCheckPts == nullptr) {
                    PrintMes(RGY_LOG_ERROR, _T("Failed to get timestamp data, timestamp might be inaccurate!\n"));
                } else {
                    PipelineTaskOutputSurf *outSurf = dynamic_cast<PipelineTaskOutputSurf *>(out.get());
                    outSurf->surf().frame()->setTimestamp(dataCheckPts->timestampOverride());
                }
            }
            m_outFrames++;
            output.push_back(std::move(out));
        }
        if (output.size() > 1) {
            PrintMes(RGY_LOG_ERROR, _T("output queue more than 1, invalid!\n"));
        }
        return output;
    }
};

class PipelineTaskTrim : public PipelineTask {
protected:
    const sTrimParam &m_trimParam;
    RGYInput *m_input;
    rgy_rational<int> m_srcTimebase;
public:
    PipelineTaskTrim(amf::AMFContextPtr context, const sTrimParam &trimParam, RGYInput *input, const rgy_rational<int>& srcTimebase, int outMaxQueueSize, std::shared_ptr<RGYLog> log) :
        PipelineTask(PipelineTaskType::TRIM, context, outMaxQueueSize, log),
        m_trimParam(trimParam), m_input(input), m_srcTimebase(srcTimebase) {
    };
    virtual ~PipelineTaskTrim() {};

    virtual bool isPassThrough() const override { return true; }
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override { return std::nullopt; };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override { return std::nullopt; };

    virtual RGY_ERR sendFrame(std::unique_ptr<PipelineTaskOutput>& frame) override {
        if (!frame) {
            return RGY_ERR_MORE_DATA;
        }
        m_inFrames++;
        PipelineTaskOutputSurf *taskSurf = dynamic_cast<PipelineTaskOutputSurf *>(frame.get());
        if (!frame_inside_range(taskSurf->surf().frame()->inputFrameId(), m_trimParam.list).first) {
            return RGY_ERR_NONE;
        }
        if (!m_input->checkTimeSeekTo(taskSurf->surf().frame()->timestamp(), m_srcTimebase)) {
            return RGY_ERR_NONE; //seektoにより脱落させるフレーム
        }
        m_outQeueue.push_back(std::make_unique<PipelineTaskOutputSurf>(taskSurf->surf()));
        return RGY_ERR_NONE;
    }
};

class PipelineTaskAudio : public PipelineTask {
protected:
    RGYInput *m_input;
    std::map<int, std::shared_ptr<RGYOutputAvcodec>> m_pWriterForAudioStreams;
    std::map<int, RGYFilter *> m_filterForStreams;
    std::vector<std::shared_ptr<RGYInput>> m_audioReaders;
public:
    PipelineTaskAudio(amf::AMFContextPtr context, RGYInput *input, std::vector<std::shared_ptr<RGYInput>>& audioReaders, std::vector<std::shared_ptr<RGYOutput>>& fileWriterListAudio, std::vector<VppVilterBlock>& vpFilters, int outMaxQueueSize, std::shared_ptr<RGYLog> log) :
        PipelineTask(PipelineTaskType::AUDIO, context, outMaxQueueSize, log),
        m_input(input), m_audioReaders(audioReaders) {
        //streamのindexから必要なwriteへのポインタを返すテーブルを作成
        for (auto writer : fileWriterListAudio) {
            auto pAVCodecWriter = std::dynamic_pointer_cast<RGYOutputAvcodec>(writer);
            if (pAVCodecWriter) {
                auto trackIdList = pAVCodecWriter->GetStreamTrackIdList();
                for (auto trackID : trackIdList) {
                    m_pWriterForAudioStreams[trackID] = pAVCodecWriter;
                }
            }
        }
        //streamのtrackIdからパケットを送信するvppフィルタへのポインタを返すテーブルを作成
        for (auto& filterBlock : vpFilters) {
            if (filterBlock.type == VppFilterType::FILTER_OPENCL) {
                for (auto& filter : filterBlock.vppcl) {
                    const auto targetTrackId = filter->targetTrackIdx();
                    if (targetTrackId != 0) {
                        m_filterForStreams[targetTrackId] = filter.get();
                    }
                }
            }
        }
    };
    virtual ~PipelineTaskAudio() {};

    virtual bool isPassThrough() const override { return true; }

    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override { return std::nullopt; };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override { return std::nullopt; };


    void flushAudio() {
        PrintMes(RGY_LOG_DEBUG, _T("Clear packets in writer...\n"));
        std::set<RGYOutputAvcodec*> writers;
        for (const auto& [ streamid, writer ] : m_pWriterForAudioStreams) {
            auto pWriter = std::dynamic_pointer_cast<RGYOutputAvcodec>(writer);
            if (pWriter != nullptr) {
                writers.insert(pWriter.get());
            }
        }
        for (const auto& writer : writers) {
            //エンコーダなどにキャッシュされたパケットを書き出す
            writer->WriteNextPacket(nullptr);
        }
    }

    RGY_ERR extractAudio(int inputFrames) {
        RGY_ERR ret = RGY_ERR_NONE;
#if ENABLE_AVSW_READER
        if (m_pWriterForAudioStreams.size() > 0) {
#if ENABLE_SM_READER
            RGYInputSM *pReaderSM = dynamic_cast<RGYInputSM *>(m_input);
            const int droppedInAviutl = (pReaderSM != nullptr) ? pReaderSM->droppedFrames() : 0;
#else
            const int droppedInAviutl = 0;
#endif

            auto packetList = m_input->GetStreamDataPackets(inputFrames + droppedInAviutl);

            //音声ファイルリーダーからのトラックを結合する
            for (const auto& reader : m_audioReaders) {
                vector_cat(packetList, reader->GetStreamDataPackets(inputFrames + droppedInAviutl));
            }
            //パケットを各Writerに分配する
            for (uint32_t i = 0; i < packetList.size(); i++) {
                AVPacket *pkt = packetList[i];
                const int nTrackId = pktFlagGetTrackID(pkt);
                const bool sendToFilter = m_filterForStreams.count(nTrackId) > 0;
                const bool sendToWriter = m_pWriterForAudioStreams.count(nTrackId) > 0;
                if (sendToFilter) {
                    AVPacket *pktToFilter = nullptr;
                    if (sendToWriter) {
                        pktToFilter = av_packet_clone(pkt);
                    } else {
                        std::swap(pktToFilter, pkt);
                    }
                    auto err = m_filterForStreams[nTrackId]->addStreamPacket(pktToFilter);
                    if (err != RGY_ERR_NONE) {
                        return err;
                    }
                }
                if (sendToWriter) {
                    auto pWriter = m_pWriterForAudioStreams[nTrackId];
                    if (pWriter == nullptr) {
                        PrintMes(RGY_LOG_ERROR, _T("Invalid writer found for %s track #%d\n"), char_to_tstring(trackMediaTypeStr(nTrackId)).c_str(), trackID(nTrackId));
                        return RGY_ERR_NOT_FOUND;
                    }
                    auto err = pWriter->WriteNextPacket(pkt);
                    if (err != RGY_ERR_NONE) {
                        return err;
                    }
                    pkt = nullptr;
                }
                if (pkt != nullptr) {
                    PrintMes(RGY_LOG_ERROR, _T("Failed to find writer for %s track #%d\n"), char_to_tstring(trackMediaTypeStr(nTrackId)).c_str(), trackID(nTrackId));
                    return RGY_ERR_NOT_FOUND;
                }
            }
        }
#endif //ENABLE_AVSW_READER
        return ret;
    };

    virtual RGY_ERR sendFrame(std::unique_ptr<PipelineTaskOutput>& frame) override {
        m_inFrames++;
        auto err = extractAudio(m_inFrames);
        if (err != RGY_ERR_NONE) {
            return err;
        }
        if (!frame) {
            flushAudio();
            return RGY_ERR_MORE_DATA;
        }
        PipelineTaskOutputSurf *taskSurf = dynamic_cast<PipelineTaskOutputSurf *>(frame.get());
        m_outQeueue.push_back(std::make_unique<PipelineTaskOutputSurf>(taskSurf->surf()));
        return RGY_ERR_NONE;
    }
};

class PipelineTaskVideoQualityMetric : public PipelineTask {
private:
    std::shared_ptr<RGYOpenCLContext> m_cl;
    RGYFilterSsim *m_videoMetric;
public:
    PipelineTaskVideoQualityMetric(amf::AMFContextPtr context, RGYFilterSsim *videoMetric, std::shared_ptr<RGYOpenCLContext> cl, int outMaxQueueSize, std::shared_ptr<RGYLog> log)
        : PipelineTask(PipelineTaskType::VIDEOMETRIC, context, outMaxQueueSize, log), m_cl(cl), m_videoMetric(videoMetric) {
    };

    virtual bool isPassThrough() const override { return true; }
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override { return std::nullopt; };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override { return std::nullopt; };
    virtual RGY_ERR sendFrame(std::unique_ptr<PipelineTaskOutput>& frame) override {
        if (!frame) {
            return RGY_ERR_MORE_DATA;
        }
        //明示的に待機が必要
        frame->depend_clear();

        PipelineTaskOutputSurf *taskSurf = dynamic_cast<PipelineTaskOutputSurf *>(frame.get());
        if (taskSurf == nullptr) {
            PrintMes(RGY_LOG_ERROR, _T("Invalid task surface.\n"));
            return RGY_ERR_NULL_PTR;
        }
        RGYFrameInfo inputFrame;
        if (auto surfVppIn = taskSurf->surf().amf(); surfVppIn != nullptr) {
            if (taskSurf->surf().frame()->mem_type() != RGY_MEM_TYPE_CPU
                && surfVppIn->amf()->GetMemoryType() != amf::AMF_MEMORY_OPENCL) {
                amf::AMFContext::AMFOpenCLLocker locker(m_context);
#if 0
                auto ar = inAmf->Interop(amf::AMF_MEMORY_OPENCL);
#else
#if 0
                //dummyのCPUへのメモリコピーを行う
                //こうしないとデコーダからの出力をOpenCLに渡したときに、フレームが壊れる(フレーム順序が入れ替わってガクガクする)
                amf::AMFDataPtr data;
                surfVppIn->Duplicate(amf::AMF_MEMORY_HOST, &data);
#endif
                auto ar = surfVppIn->amf()->Convert(amf::AMF_MEMORY_OPENCL);
#endif
                if (ar != AMF_OK) {
                    PrintMes(RGY_LOG_ERROR, _T("Failed to convert plane: %s.\n"), get_err_mes(err_to_rgy(ar)));
                    return err_to_rgy(ar);
                }
            }
            inputFrame = taskSurf->surf().amf()->getInfoCopy();
        } else if (taskSurf->surf().cl() != nullptr) {
            //OpenCLフレームが出てきた時の場合
            auto clframe = taskSurf->surf().cl();
            if (clframe == nullptr) {
                PrintMes(RGY_LOG_ERROR, _T("Invalid cl frame.\n"));
                return RGY_ERR_NULL_PTR;
            }
            inputFrame = clframe->frameInfo();
        } else {
            PrintMes(RGY_LOG_ERROR, _T("Invalid input frame.\n"));
            return RGY_ERR_NULL_PTR;
        }
        //フレームを転送
        RGYOpenCLEvent inputReleaseEvent;
        int dummy = 0;
        auto err = m_videoMetric->filter(&inputFrame, nullptr, &dummy, m_cl->queue(), &inputReleaseEvent);
        if (err != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to send frame for video metric calcualtion: %s.\n"), get_err_mes(err));
            return err;
        }
        //eventを入力フレームを使用し終わったことの合図として登録する
        taskSurf->addClEvent(inputReleaseEvent);
        m_outQeueue.push_back(std::move(frame));
        return RGY_ERR_NONE;
    }
};

class PipelineTaskAMFEncode : public PipelineTask {
protected:
    amf::AMFComponentPtr m_encoder;
    RGY_CODEC m_encCodec;
    AMFParams& m_encParams;
    RGYTimecode *m_timecode;
    RGYTimestamp *m_encTimestamp;
    rgy_rational<int> m_outputTimebase;
    RGYListRef<RGYBitstream> m_bitStreamOut;
    RGYHDR10Plus *m_hdr10plus;
    bool m_hdr10plusMetadataCopy;
public:
    PipelineTaskAMFEncode(
        amf::AMFComponentPtr enc, RGY_CODEC encCodec, AMFParams& encParams, amf::AMFContextPtr context, int outMaxQueueSize,
        RGYTimecode *timecode, RGYTimestamp *encTimestamp, rgy_rational<int> outputTimebase, RGYHDR10Plus *hdr10plus, bool hdr10plusMetadataCopy, std::shared_ptr<RGYLog> log)
        : PipelineTask(PipelineTaskType::AMFENC, context, outMaxQueueSize, log),
        m_encoder(enc), m_encCodec(encCodec), m_encParams(encParams), m_timecode(timecode), m_encTimestamp(encTimestamp), m_outputTimebase(outputTimebase), m_bitStreamOut(), m_hdr10plus(hdr10plus), m_hdr10plusMetadataCopy(hdr10plusMetadataCopy) {
    };
    virtual ~PipelineTaskAMFEncode() {
        m_outQeueue.clear(); // m_bitStreamOutが解放されるよう前にこちらを解放する
    };
    void setEnc(amf::AMFComponentPtr encode) { m_encoder = encode; };

    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override {
        int outWidth = 0, outHeight = 0;
        m_encParams.GetParam(VCE_PARAM_KEY_OUTPUT_WIDTH, outWidth);
        m_encParams.GetParam(VCE_PARAM_KEY_OUTPUT_HEIGHT, outHeight);
        int bitdepth = 8;
        if (m_encCodec == RGY_CODEC_HEVC || m_encCodec == RGY_CODEC_AV1) {
            m_encParams.GetParam(AMF_PARAM_COLOR_BIT_DEPTH(m_encCodec), bitdepth);
        }
        auto csp = RGY_CSP_NV12;
        const bool yuv444 = false;
        if (bitdepth > 8) {
            csp = (yuv444) ? RGY_CSP_YUV444_16 : RGY_CSP_P010;
        } else {
            csp = (yuv444) ? RGY_CSP_YUV444 : RGY_CSP_NV12;
        }
        RGYFrameInfo info(outWidth, outHeight, csp, bitdepth, RGY_PICSTRUCT_FRAME, RGY_MEM_TYPE_GPU_IMAGE);
        return std::make_pair(info, 0);
    }
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override { return std::nullopt; };

    std::pair<RGY_ERR, std::shared_ptr<RGYBitstream>> getOutputBitstream() {
        const auto VCE_TIMEBASE = rgy_rational<int>(1, AMF_SECOND);
        amf::AMFDataPtr data;
        try {
            auto ar = m_encoder->QueryOutput(&data);
            if (ar == AMF_REPEAT || (ar == AMF_OK && data == nullptr)) {
                return { RGY_ERR_MORE_SURFACE, nullptr };
            }
            if (ar == AMF_EOF) {
                return { RGY_ERR_MORE_DATA, nullptr };
            }
            if (ar != AMF_OK) {
                return { err_to_rgy(ar), nullptr };
            }
        } catch (...) {
            PrintMes(RGY_LOG_ERROR, _T("Fatal error when getting output bitstream from encoder.\n"));
            return { RGY_ERR_DEVICE_FAILED, nullptr };
        }

        auto output = m_bitStreamOut.get([](RGYBitstream *bs) {
            *bs = RGYBitstreamInit();
            return 0;
        });
        if (!output) {
            return { RGY_ERR_NULL_PTR, nullptr };
        }
        amf::AMFBufferPtr buffer(data);
        int64_t value = 0;
        int64_t pts = rgy_change_scale(buffer->GetPts(), VCE_TIMEBASE, m_outputTimebase);
        int64_t duration = rgy_change_scale(buffer->GetDuration(), VCE_TIMEBASE, m_outputTimebase);
        if (buffer->GetProperty(RGY_PROP_TIMESTAMP, &value) == AMF_OK) {
            pts = value;
        }
        if (buffer->GetProperty(RGY_PROP_DURATION, &value) == AMF_OK) {
            duration = value;
        }
        output->copy((uint8_t *)buffer->GetNative(), buffer->GetSize(), pts, 0, duration);
        if (buffer->GetProperty(AMF_PARAM_OUTPUT_DATA_TYPE(m_encCodec), &value) == AMF_OK) {
            switch ((AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_ENUM)value) {
            case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_IDR: output->setFrametype(RGY_FRAMETYPE_IDR); break;
            case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_I:   output->setFrametype(RGY_FRAMETYPE_I); break;
            case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_P:   output->setFrametype(RGY_FRAMETYPE_P); break;
            case AMF_VIDEO_ENCODER_OUTPUT_DATA_TYPE_B:
            default:
                output->setFrametype((m_encCodec == RGY_CODEC_AV1) ? RGY_FRAMETYPE_P : RGY_FRAMETYPE_B); break;
            }
        }
        if (m_encCodec == RGY_CODEC_H264 || m_encCodec == RGY_CODEC_HEVC) {
            uint32_t value32 = 0;
            if (buffer->GetProperty(AMF_PARAM_STATISTIC_AVERAGE_QP(m_encCodec), &value32) == AMF_OK) {
                output->setAvgQP(value32);
            }
        }
        return { RGY_ERR_NONE, output };
    }

    virtual RGY_ERR sendFrame(std::unique_ptr<PipelineTaskOutput>& frame) override {
        if (frame && frame->type() != PipelineTaskOutputType::SURFACE) {
            PrintMes(RGY_LOG_ERROR, _T("Invalid frame type.\n"));
            return RGY_ERR_UNSUPPORTED;
        }

        std::vector<std::shared_ptr<RGYFrameData>> metadatalist;
        if (m_encCodec == RGY_CODEC_HEVC || m_encCodec == RGY_CODEC_AV1) {
            if (m_hdr10plus) {
                if (const auto data = m_hdr10plus->getData(m_inFrames); data) {
                    metadatalist.push_back(std::make_shared<RGYFrameDataHDR10plus>(data->data(), data->size(), dynamic_cast<PipelineTaskOutputSurf *>(frame.get())->surf().frame()->timestamp()));
                }
            } else if (m_hdr10plusMetadataCopy && frame) {
                metadatalist = dynamic_cast<PipelineTaskOutputSurf *>(frame.get())->surf().frame()->dataList();
            }
        }

        //以下の処理は
        amf::AMFSurfacePtr pSurface = nullptr;
        RGYFrameAMF *surfEncodeIn = (frame) ? dynamic_cast<PipelineTaskOutputSurf *>(frame.get())->surf().amf() : nullptr;
        const bool drain = surfEncodeIn == nullptr;
        if (surfEncodeIn) {
            const int64_t pts = surfEncodeIn->timestamp();
            const int64_t duration = surfEncodeIn->duration();
            const auto frameDataList = surfEncodeIn->dataList();
            const auto inputFrameId = surfEncodeIn->inputFrameId();
            if (inputFrameId < 0) {
                PrintMes(RGY_LOG_ERROR, _T("Invalid inputFrameId: %d.\n"), inputFrameId);
                return RGY_ERR_UNKNOWN;
            }
            pSurface = surfEncodeIn->detachSurface();
            //現状VCEはインタレをサポートしないので、強制的にプログレとして処理する
            pSurface->SetFrameType(amf::AMF_FRAME_PROGRESSIVE);
            //現状VCEはインタレをサポートしないので、強制的にプログレとして処理する
            //フレーム情報のほうもプログレに書き換えなければ、SubmitInputでエラーが返る
            if (m_encCodec == RGY_CODEC_H264) {
                m_encParams.SetParam(AMF_VIDEO_ENCODER_PICTURE_STRUCTURE, AMF_VIDEO_ENCODER_PICTURE_STRUCTURE_TOP_FIELD);
            }
            // apply frame-specific properties to the current frame
            m_encParams.Apply(pSurface, AMF_PARAM_FRAME, m_log.get());
            // apply dynamic properties to the encoder
            //m_encParams.Apply(m_pEncoder, AMF_PARAM_DYNAMIC, m_pLog.get());

            pSurface->SetProperty(RGY_PROP_TIMESTAMP, pts);
            pSurface->SetProperty(RGY_PROP_DURATION, duration);

            pSurface->SetProperty(AMF_PARAM_STATISTICS_FEEDBACK(m_encCodec), true);

            if (m_timecode) {
                m_timecode->write(pts, m_outputTimebase);
            }
            m_encTimestamp->add(pts, inputFrameId, m_inFrames, duration, metadatalist);
            m_inFrames++;

            //エンコーダまでたどり着いたフレームについてはdataListを解放
            surfEncodeIn->clearDataList();
        }

        auto enc_sts = RGY_ERR_NONE;
        auto ar = (drain) ? AMF_INPUT_FULL : AMF_OK;
        for (;;) {
            //エンコーダからの取り出し
            auto outBs = getOutputBitstream();
            const auto out_ret = outBs.first;
            if (out_ret == RGY_ERR_MORE_SURFACE) {
                ; // もっとエンコーダへの投入が必要
            } else if (out_ret == RGY_ERR_NONE) {
                if (outBs.second) {
                    m_outQeueue.push_back(std::make_unique<PipelineTaskOutputBitstream>(outBs.second));
                }
            } else if (out_ret == RGY_ERR_MORE_DATA) { //EOF
                enc_sts = RGY_ERR_MORE_DATA;
                break;
            } else {
                enc_sts = out_ret;
                break;
            }

            if (drain) {
                if (ar == AMF_INPUT_FULL) {
                    //エンコーダのflush
                    try {
                        ar = m_encoder->Drain();
                    } catch (...) {
                        PrintMes(RGY_LOG_ERROR, _T("Fatal error when submitting frame to encoder.\n"));
                        return RGY_ERR_DEVICE_FAILED;
                    }
                    if (ar == AMF_INPUT_FULL) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    } else {
                        enc_sts = err_to_rgy(ar);
                    }
                }
            } else {
                //エンコードへの投入
                try {
                    ar = m_encoder->SubmitInput(pSurface);
                } catch (...) {
                    PrintMes(RGY_LOG_ERROR, _T("Fatal error when submitting frame to encoder.\n"));
                    return RGY_ERR_DEVICE_FAILED;
                }
                if (ar == AMF_NEED_MORE_INPUT) {
                    break;
                } else if (ar == AMF_INPUT_FULL) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                } else if (ar == AMF_REPEAT) {
                    pSurface = nullptr;
                } else {
                    enc_sts = err_to_rgy(ar);
                    break;
                }
            }
        };
        return enc_sts;
    }
};

class PipelineTaskAMFPreProcess : public PipelineTask {
protected:
    std::shared_ptr<RGYOpenCLContext> m_cl;
    std::unique_ptr<AMFFilter>& m_vppFilter;
    int m_vppOutFrames;
    std::unordered_map<int64_t, std::vector<std::shared_ptr<RGYFrameData>>> m_metadatalist;
    std::deque<std::unique_ptr<PipelineTaskOutput>> m_prevInputFrame; //前回投入されたフレーム、完了通知を待ってから解放するため、参照を保持する
    RGYFilterSsim *m_videoMetric;
public:
    PipelineTaskAMFPreProcess(amf::AMFContextPtr context, std::unique_ptr<AMFFilter>& vppfilter, std::shared_ptr<RGYOpenCLContext> cl, int outMaxQueueSize, std::shared_ptr<RGYLog> log) :
        PipelineTask(PipelineTaskType::AMFVPP, context, outMaxQueueSize, log), m_cl(cl), m_vppFilter(vppfilter), m_vppOutFrames(), m_metadatalist(), m_prevInputFrame() {

    };
    virtual ~PipelineTaskAMFPreProcess() {
        m_prevInputFrame.clear();
        m_cl.reset();
    };

    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override {
        return std::make_pair(m_vppFilter->GetFilterParam()->frameIn, 0);
    };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override {
        return std::make_pair(m_vppFilter->GetFilterParam()->frameOut, 0);
    };
    virtual RGY_ERR sendFrame(std::unique_ptr<PipelineTaskOutput>& frame) override {
        if (m_prevInputFrame.size() > 0) {
            //前回投入したフレームの処理が完了していることを確認したうえで参照を破棄することでロックを解放する
            auto prevframe = std::move(m_prevInputFrame.front());
            m_prevInputFrame.pop_front();
            prevframe->depend_clear();
        }

        if (frame && frame->type() != PipelineTaskOutputType::SURFACE) {
            PrintMes(RGY_LOG_ERROR, _T("Invalid frame type.\n"));
            return RGY_ERR_UNSUPPORTED;
        }

        amf::AMFSurfacePtr pSurface = nullptr;
        RGYFrameAMF *surfVppIn = (frame) ? dynamic_cast<PipelineTaskOutputSurf *>(frame.get())->surf().amf() : nullptr;
        const bool drain = surfVppIn == nullptr;
        if (surfVppIn) {
            if (surfVppIn->dataList().size() > 0) {
                m_metadatalist[surfVppIn->timestamp()] = surfVppIn->dataList();
            }
            int64_t pts = surfVppIn->timestamp();
            int64_t duration = surfVppIn->duration();
            const auto inputFrameId = surfVppIn->inputFrameId();
            auto frameDataList = surfVppIn->dataList();
            if (inputFrameId < 0) {
                PrintMes(RGY_LOG_ERROR, _T("Invalid inputFrameId: %d.\n"), inputFrameId);
                return RGY_ERR_UNKNOWN;
            }
            pSurface = surfVppIn->detachSurface();
            //現状VCEはインタレをサポートしないので、強制的にプログレとして処理する
            pSurface->SetFrameType(amf::AMF_FRAME_PROGRESSIVE);
            pSurface->SetProperty(RGY_PROP_TIMESTAMP, pts);
            pSurface->SetProperty(RGY_PROP_DURATION, duration);
            pSurface->SetProperty(RGY_PROP_INPUT_FRAMEID, inputFrameId);
            m_vppFilter->setFrameParam(pSurface);
            m_inFrames++;

            surfVppIn->clearDataList();
        }

        auto enc_sts = RGY_ERR_NONE;
        auto ar = (drain) ? AMF_INPUT_FULL : AMF_OK;
        for (;;) {
            {
                //VPPからの取り出し
                amf::AMFDataPtr data;
                const auto ar_out = m_vppFilter->filter()->QueryOutput(&data);
                if (ar_out == AMF_EOF) {
                    enc_sts = RGY_ERR_MORE_DATA;
                    break;
                } else if (ar_out == AMF_REPEAT) {
                    ; //これ重要...ここが欠けると最後の数フレームが欠落する
                } else if (ar_out != AMF_OK) {
                    enc_sts = err_to_rgy(ar_out);
                    break;
                } else if (data != nullptr) {
                    auto surfVppOut = amf::AMFSurfacePtr(data);
                    int64_t pts = 0, duration = 0, inputFrameId = 0;
                    surfVppOut->GetProperty(RGY_PROP_TIMESTAMP, &pts);
                    surfVppOut->GetProperty(RGY_PROP_DURATION, &duration);
                    surfVppOut->GetProperty(RGY_PROP_INPUT_FRAMEID, &inputFrameId);

                    auto surfDecOut = std::make_unique<RGYFrameAMF>(surfVppOut);
                    surfDecOut->clearDataList();
                    surfDecOut->setTimestamp(pts);
                    surfDecOut->setDuration(duration);
                    surfDecOut->setInputFrameId(m_vppOutFrames++);
                    if (auto it = m_metadatalist.find(pts); it != m_metadatalist.end()) {
                        surfDecOut->setDataList(m_metadatalist[pts]);
                    }
                    m_outQeueue.push_back(std::make_unique<PipelineTaskOutputSurf>(m_workSurfs.addSurface(surfDecOut)));
                }
            }

            if (drain) {
                if (ar == AMF_INPUT_FULL) {
                    //VPPのflush
                    try {
                        ar = m_vppFilter->filter()->Drain();
                    } catch (...) {
                        PrintMes(RGY_LOG_ERROR, _T("Fatal error when submitting frame to encoder.\n"));
                        return RGY_ERR_DEVICE_FAILED;
                    }
                    if (ar == AMF_INPUT_FULL) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    } else {
                        enc_sts = err_to_rgy(ar);
                    }
                }
            } else {
                //VPPへの投入
                try {
                    ar = m_vppFilter->filter()->SubmitInput(pSurface);
                } catch (...) {
                    PrintMes(RGY_LOG_ERROR, _T("Fatal error when submitting frame to encoder.\n"));
                    return RGY_ERR_DEVICE_FAILED;
                }
                if (ar == AMF_NEED_MORE_INPUT) {
                    break;
                } else if (ar == AMF_INPUT_FULL) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                } else if (ar == AMF_REPEAT) {
                    pSurface = nullptr;
                } else {
                    enc_sts = err_to_rgy(ar);
                    break;
                }
            }
        };
        return enc_sts;
    }
};

class PipelineTaskOpenCL : public PipelineTask {
protected:
    std::shared_ptr<RGYOpenCLContext> m_cl;
    bool m_dx11interlop;
    std::vector<std::unique_ptr<RGYFilter>>& m_vpFilters;
    std::deque<std::unique_ptr<PipelineTaskOutput>> m_prevInputFrame; //前回投入されたフレーム、完了通知を待ってから解放するため、参照を保持する
    RGYFilterSsim *m_videoMetric;
public:
    PipelineTaskOpenCL(amf::AMFContextPtr context, std::vector<std::unique_ptr<RGYFilter>>& vppfilters, RGYFilterSsim *videoMetric, std::shared_ptr<RGYOpenCLContext> cl, int outMaxQueueSize, bool dx11interlop, std::shared_ptr<RGYLog> log) :
        PipelineTask(PipelineTaskType::OPENCL, context, outMaxQueueSize, log), m_cl(cl), m_dx11interlop(dx11interlop), m_vpFilters(vppfilters), m_prevInputFrame(), m_videoMetric(videoMetric) {

    };
    virtual ~PipelineTaskOpenCL() {
        m_prevInputFrame.clear();
        m_cl.reset();
    };

    void setVideoQualityMetricFilter(RGYFilterSsim *videoMetric) {
        m_videoMetric = videoMetric;
    }

    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override { return std::nullopt; };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override { return std::nullopt; };
    virtual RGY_ERR sendFrame(std::unique_ptr<PipelineTaskOutput>& frame) override {
        if (m_prevInputFrame.size() > 0) {
            //前回投入したフレームの処理が完了していることを確認したうえで参照を破棄することでロックを解放する
            auto prevframe = std::move(m_prevInputFrame.front());
            m_prevInputFrame.pop_front();
            prevframe->depend_clear();
        }

        deque<std::pair<RGYFrameInfo, uint32_t>> filterframes;
        bool drain = !frame;
        if (!frame) {
            filterframes.push_back(std::make_pair(RGYFrameInfo(), 0u));
        } else {
            auto taskSurf = dynamic_cast<PipelineTaskOutputSurf *>(frame.get());
            if (taskSurf == nullptr) {
                PrintMes(RGY_LOG_ERROR, _T("Invalid task surface.\n"));
                return RGY_ERR_NULL_PTR;
            }
            if (auto surfVppInAMF = taskSurf->surf().amf(); surfVppInAMF != nullptr) {
                if (taskSurf->surf().frame()->mem_type() != RGY_MEM_TYPE_CPU
                    && surfVppInAMF->amf()->GetMemoryType() != amf::AMF_MEMORY_OPENCL) {
                    amf::AMFContext::AMFOpenCLLocker locker(m_context);
#if 0
                    auto ar = inAmf->Interop(amf::AMF_MEMORY_OPENCL);
#else
#if 0
                    //dummyのCPUへのメモリコピーを行う
                    //こうしないとデコーダからの出力をOpenCLに渡したときに、フレームが壊れる(フレーム順序が入れ替わってガクガクする)
                    amf::AMFDataPtr data;
                    surfVppInAMF->Duplicate(amf::AMF_MEMORY_HOST, &data);
#endif
                    auto ar = surfVppInAMF->amf()->Convert(amf::AMF_MEMORY_OPENCL);
#endif
                    if (ar != AMF_OK) {
                        PrintMes(RGY_LOG_ERROR, _T("Failed to convert plane: %s.\n"), get_err_mes(err_to_rgy(ar)));
                        return err_to_rgy(ar);
                    }
                }
                filterframes.push_back(std::make_pair(surfVppInAMF->getInfoCopy(), 0u));
            } else if (auto surfVppInCL = taskSurf->surf().cl(); surfVppInCL != nullptr) {
                filterframes.push_back(std::make_pair(surfVppInCL->frameInfo(), 0u));
            } else {
                PrintMes(RGY_LOG_ERROR, _T("Invalid task surface (not opencl or amf).\n"));
                return RGY_ERR_NULL_PTR;
            }
            //ここでinput frameの参照を m_prevInputFrame で保持するようにして、OpenCLによるフレームの処理が完了しているかを確認できるようにする
            //これを行わないとこのフレームが再度使われてしまうことになる
            m_prevInputFrame.push_back(std::move(frame));
        }
#define FRAME_COPY_ONLY 0
#if !FRAME_COPY_ONLY
        std::vector<std::unique_ptr<PipelineTaskOutputSurf>> outputSurfs;
        while (filterframes.size() > 0 || drain) {
            //フィルタリングするならここ
            for (uint32_t ifilter = filterframes.front().second; ifilter < m_vpFilters.size() - 1; ifilter++) {
                // コピーを作ってそれをfilter関数に渡す
                // vpp-rffなどoverwirteするフィルタのときに、filterframes.pop_front -> push がうまく動作しない
                RGYFrameInfo input = filterframes.front().first;

                int nOutFrames = 0;
                RGYFrameInfo *outInfo[16] = { 0 };
                auto sts_filter = m_vpFilters[ifilter]->filter(&input, (RGYFrameInfo **)&outInfo, &nOutFrames);
                if (sts_filter != RGY_ERR_NONE) {
                    PrintMes(RGY_LOG_ERROR, _T("Error while running filter \"%s\".\n"), m_vpFilters[ifilter]->name().c_str());
                    return sts_filter;
                }
                if (nOutFrames == 0) {
                    if (drain) {
                        filterframes.front().second++;
                        continue;
                    }
                    return RGY_ERR_NONE;
                }
                drain = false; //途中でフレームが出てきたら、drain完了していない

                filterframes.pop_front();
                //最初に出てきたフレームは先頭に追加する
                for (int jframe = nOutFrames - 1; jframe >= 0; jframe--) {
                    filterframes.push_front(std::make_pair(*outInfo[jframe], ifilter + 1));
                }
            }
            if (drain) {
                return RGY_ERR_MORE_DATA; //最後までdrain = trueなら、drain完了
            }
            PipelineTaskSurface surfVppOut;
            RGYFrameInfo surfVppOutInfo;
            if (workSurfaceType() == PipelineTaskSurfaceType::CL) {
                surfVppOut = getWorkSurf();
                if (surfVppOut == nullptr) {
                    PrintMes(RGY_LOG_ERROR, _T("failed to get work surface for input.\n"));
                    return RGY_ERR_NOT_ENOUGH_BUFFER;
                }
                surfVppOutInfo = surfVppOut.cl()->frameInfo();
            } else {
                //エンコードバッファにコピー
                amf::AMFContext::AMFOpenCLLocker locker(m_context);
                auto &lastFilter = m_vpFilters[m_vpFilters.size() - 1];
                const auto lastFilterOut = lastFilter->GetFilterParam()->frameOut;
                amf::AMFSurfacePtr pSurface;
                if (m_dx11interlop) {
                    auto ar = m_context->AllocSurface(amf::AMF_MEMORY_DX11, csp_rgy_to_enc(lastFilterOut.csp),
                        lastFilterOut.width, lastFilterOut.height, &pSurface);
                    if (ar != AMF_OK) {
                        PrintMes(RGY_LOG_ERROR, _T("Failed to allocate surface: %s.\n"), get_err_mes(err_to_rgy(ar)));
                        return err_to_rgy(ar);
                    }
                    ar = pSurface->Interop(amf::AMF_MEMORY_OPENCL);
                    if (ar != AMF_OK) {
                        PrintMes(RGY_LOG_ERROR, _T("Failed to get interop of surface: %s.\n"), get_err_mes(err_to_rgy(ar)));
                        return err_to_rgy(ar);
                    }
                } else {
                    auto ar = m_context->AllocSurface(amf::AMF_MEMORY_OPENCL, csp_rgy_to_enc(lastFilterOut.csp),
                        lastFilterOut.width, lastFilterOut.height, &pSurface);
                    if (ar != AMF_OK) {
                        PrintMes(RGY_LOG_ERROR, _T("Failed to allocate surface: %s.\n"), get_err_mes(err_to_rgy(ar)));
                        return err_to_rgy(ar);
                    }
                }
                auto surfVppOutAMF = std::make_unique<RGYFrameAMF>(pSurface);
                surfVppOutInfo = surfVppOutAMF->getInfoCopy();
                surfVppOut = m_workSurfs.addSurface(surfVppOutAMF);
            }
            //エンコードバッファにコピー
            auto &lastFilter = m_vpFilters[m_vpFilters.size() - 1];
            //最後のフィルタはRGYFilterCspCropでなければならない
            if (typeid(*lastFilter.get()) != typeid(RGYFilterCspCrop)) {
                PrintMes(RGY_LOG_ERROR, _T("Last filter setting invalid.\n"));
                return RGY_ERR_INVALID_PARAM;
            }
            //エンコードバッファのポインタを渡す
            int nOutFrames = 0;
            RGYFrameInfo *outInfo[1];
            outInfo[0] = &surfVppOutInfo;
            RGYOpenCLEvent clevent; // 最終フィルタの処理完了を伝えるevent
            auto sts_filter = lastFilter->filter(&filterframes.front().first, (RGYFrameInfo **)&outInfo, &nOutFrames, m_cl->queue(), &clevent);
            if (sts_filter != RGY_ERR_NONE) {
                PrintMes(RGY_LOG_ERROR, _T("Error while running filter \"%s\".\n"), lastFilter->name().c_str());
                return sts_filter;
            }
            if (m_videoMetric) {
                //フレームを転送
                int dummy = 0;
                auto err = m_videoMetric->filter(&filterframes.front().first, nullptr, &dummy, m_cl->queue(), &clevent);
                if (err != RGY_ERR_NONE) {
                    PrintMes(RGY_LOG_ERROR, _T("Failed to send frame for video metric calcualtion: %s.\n"), get_err_mes(err));
                    return err;
                }
            }
            filterframes.pop_front();

            surfVppOut.frame()->setDuration(surfVppOutInfo.duration);
            surfVppOut.frame()->setTimestamp(surfVppOutInfo.timestamp);
            surfVppOut.frame()->setInputFrameId(surfVppOutInfo.inputFrameId);
            surfVppOut.frame()->setPicstruct(surfVppOutInfo.picstruct);
            surfVppOut.frame()->setFlags(surfVppOutInfo.flags);
            surfVppOut.frame()->setDataList(surfVppOutInfo.dataList);

            outputSurfs.push_back(std::make_unique<PipelineTaskOutputSurf>(surfVppOut, frame, clevent));

            #undef clFrameOutInteropRelease
        }
        m_outQeueue.insert(m_outQeueue.end(),
            std::make_move_iterator(outputSurfs.begin()),
            std::make_move_iterator(outputSurfs.end())
        );
#else
        auto surfVppOut = getWorkSurf();
        if (m_surfVppOutInterop.count(surfVppOut.get()) == 0) {
            m_surfVppOutInterop[surfVppOut.get()] = getOpenCLFrameInterop(surfVppOut.get(), m_memType, CL_MEM_WRITE_ONLY, m_allocator, m_cl.get(), m_cl->queue(), m_vpFilters.front()->GetFilterParam()->frameIn);
        }
        auto clFrameOutInterop = m_surfVppOutInterop[surfVppOut.get()].get();
        if (!clFrameOutInterop) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to get OpenCL interop [out].\n"));
            return RGY_ERR_NULL_PTR;
        }
        auto err = clFrameOutInterop->acquire(m_cl->queue());
        if (err != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to acquire OpenCL interop [out]: %s.\n"), get_err_mes(err));
            return RGY_ERR_NULL_PTR;
        }
        auto inputSurface = clFrameInInterop->frameInfo();
        surfVppOut->Data.TimeStamp = inputSurface.timestamp;
        surfVppOut->Data.FrameOrder = inputSurface.inputFrameId;
        surfVppOut->Info.PicStruct = picstruct_rgy_to_enc(inputSurface.picstruct);
        surfVppOut->Data.DataFlag = (mfxU16)inputSurface.flags;

        auto encSurfaceInfo = clFrameOutInterop->frameInfo();
        RGYOpenCLEvent clevent;
        m_cl->copyFrame(&encSurfaceInfo, &inputSurface, nullptr, m_cl->queue(), &clevent);
        if (clFrameInInterop) {
            clFrameInInterop->release(&clevent);
            if (!m_prevInputFrame.empty() && m_prevInputFrame.back()) {
                dynamic_cast<PipelineTaskOutputSurf *>(m_prevInputFrame.back().get())->addClEvent(clevent);
            }
        }
        clFrameOutInterop->release(&clevent);
        m_outQeueue.push_back(std::make_unique<PipelineTaskOutputSurf>(m_mfxSession, surfVppOut, frame, clevent));
#endif
        return RGY_ERR_NONE;
    }
};












#endif //__VCE_PIPELINE_H__

