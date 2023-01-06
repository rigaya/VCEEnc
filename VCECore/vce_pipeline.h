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
#pragma warning(push)
#pragma warning(disable:4100)
#include "VideoEncoderVCE.h"
#include "Factory.h"
#include "Trace.h"

#include "rgy_version.h"
#include "rgy_err.h"
#include "rgy_util.h"
#include "rgy_log.h"
#include "rgy_input.h"
#include "rgy_output.h"
#include "rgy_opencl.h"
#include "rgy_filter.h"
#include "rgy_filter_ssim.h"
#include "rgy_thread.h"
#include "rgy_timecode.h"
#include "rgy_device.h"
#include "vce_device.h"
#include "vce_param.h"

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

    AMF_FILTER_A,

    AMF_MAX,

    CL_MIN = AMF_MAX,

    CL_CROP,
    CL_AFS,
    CL_NNEDI,
    CL_DECIMATE,
    CL_MPDECIMATE,
    CL_YADIF,
    CL_COLORSPACE,
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

    CL_TWEAK,

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
    amf::AMFComponentPtr vppamf;
    std::vector<std::unique_ptr<RGYFilter>> vppcl;

    VppVilterBlock(amf::AMFComponentPtr& filter) : type(VppFilterType::FILTER_AMF), vppamf(filter), vppcl() {};
    VppVilterBlock(std::vector<std::unique_ptr<RGYFilter>>& filter) : type(VppFilterType::FILTER_OPENCL), vppamf(), vppcl(std::move(filter)) {};
};

enum class PipelineTaskOutputType {
    UNKNOWN,
    SURFACE,
    BITSTREAM
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
        return amfsurf() == nullptr && clframe() == nullptr;
    }
    bool operator !=(const PipelineTaskSurface& obj) const { return amfsurf() != obj.amfsurf() || clframe() != obj.clframe(); }
    bool operator ==(const PipelineTaskSurface& obj) const { return amfsurf() == obj.amfsurf() && clframe() == obj.clframe(); }
    bool operator !=(std::nullptr_t) const { return amfsurf() != nullptr || clframe() != nullptr; }
    bool operator ==(std::nullptr_t) const { return amfsurf() == nullptr && clframe() == nullptr; }
    const amf::AMFSurface *amfsurf() const { return (surf && surf->amf()) ? surf->amf().GetPtr() : nullptr; }
    amf::AMFSurface *amfsurf() { return (surf && surf->amf()) ? surf->amf().GetPtr() : nullptr; }
    const RGYCLFrame *clframe() const { return (surf && surf->cl()) ? surf->cl().get() : nullptr; }
    RGYCLFrame *clframe() { return (surf && surf->cl()) ? surf->cl().get() : nullptr; }
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
            m_surfaces[i] = std::make_unique<PipelineTaskSurfacesPair>(std::make_unique<RGYFrame>(surfs[i]));
        }
    }
    void setSurfaces(std::vector<std::unique_ptr<RGYCLFrame>>& frames) {
        clear();
        m_surfaces.resize(frames.size());
        for (size_t i = 0; i < m_surfaces.size(); i++) {
            m_surfaces[i] = std::make_unique<PipelineTaskSurfacesPair>(std::make_unique<RGYFrame>(std::move(frames[i])));
        }
    }
    PipelineTaskSurface addSurface(std::unique_ptr<RGYFrame>& surf) {
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
        auto s = findSurf(surf);
        if (s != nullptr) {
            return s->getRef();
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

    PipelineTaskSurfacesPair *findSurf(amf::AMFSurface *surf) {
        for (auto& s : m_surfaces) {
            if (s->surf()->surf() == surf) {
                return s.get();
            }
        }
        return nullptr;
    }
    PipelineTaskSurfacesPair *findSurf(RGYCLFrame *frame) {
        for (auto& s : m_surfaces) {
            if (s->surf()->clframe() == frame) {
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
        auto clframe = m_surf.clframe();
        auto err = clframe->queueMapBuffer(*clqueue, CL_MAP_READ); // CPUが読み込むためにmapする
        if (err != RGY_ERR_NONE) {
            return err;
        }
        clframe->mapWait();
        auto mappedframe = std::make_unique<RGYFrameRef>(clframe->mappedHost());
        err = writer->WriteNextFrame(mappedframe.get());
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
        auto err = (m_surf.amfsurf() != nullptr) ? writeAMF(writer) : writeCL(writer, clqueue);
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
    AMFVPP,
    AMFDEC,
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

class PipelineTaskInputCL : public PipelineTask {
    RGYInput *m_input;
    std::shared_ptr<RGYOpenCLContext> m_cl;
public:
    PipelineTaskInputCL(amf::AMFContextPtr context, int outMaxQueueSize, RGYInput *input, std::shared_ptr<RGYOpenCLContext> cl, std::shared_ptr<RGYLog> log)
        : PipelineTask(PipelineTaskType::INPUT, context, outMaxQueueSize, log), m_input(input), m_cl(cl) {

    };
    virtual ~PipelineTaskInputCL() {};
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override { return std::nullopt; };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override { return std::nullopt; };
    RGY_ERR loadNextFrameAMF() {
    }
    RGY_ERR loadNextFrameCL() {
    }
    virtual RGY_ERR sendFrame([[maybe_unused]] std::unique_ptr<PipelineTaskOutput>& frame) override {
        auto surfWork = getWorkSurf();
        if (surfWork == nullptr) {
            PrintMes(RGY_LOG_ERROR, _T("failed to get work surface for input.\n"));
            return RGY_ERR_NOT_ENOUGH_BUFFER;
        }
        auto clframe = surfWork.clframe();
        auto err = clframe->queueMapBuffer(m_cl->queue(), CL_MAP_WRITE); // CPUが書き込むためにMapする
        if (err != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to map buffer: %s.\n"), get_err_mes(err));
            return err;
        }
        clframe->mapWait(); //すぐ終わるはず
        auto mappedframe = std::make_unique<RGYFrameRef>(clframe->mappedHost());
        err = m_input->LoadNextFrame(mappedframe.get());
        if (err != RGY_ERR_NONE) {
            //Unlockする必要があるので、ここに入ってもすぐにreturnしてはいけない
            if (err == RGY_ERR_MORE_DATA) { // EOF
                err = RGY_ERR_MORE_BITSTREAM; // EOF を PipelineTaskMFXDecode のreturnコードに合わせる
            } else {
                PrintMes(RGY_LOG_ERROR, _T("Error in reader: %s.\n"), get_err_mes(err));
            }
        }
        auto clerr = clframe->unmapBuffer();
        if (clerr != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to unmap buffer: %s.\n"), get_err_mes(err));
            if (err == RGY_ERR_NONE) {
                err = clerr;
            }
        }
        m_outQeueue.push_back(std::make_unique<PipelineTaskOutputSurf>(surfWork));
        return err;
    }
};

class PipelineTaskInputAMF : public PipelineTask {
    RGYInput *m_input;
    std::shared_ptr<RGYOpenCLContext> m_cl;
public:
    PipelineTaskInputAMF(amf::AMFContextPtr context, int outMaxQueueSize, RGYInput *input, std::shared_ptr<RGYOpenCLContext> cl, std::shared_ptr<RGYLog> log)
        : PipelineTask(PipelineTaskType::INPUT, context, outMaxQueueSize, log), m_input(input), m_cl(cl) {

    };
    virtual ~PipelineTaskInputAMF() {};
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override { return std::nullopt; };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override { return std::nullopt; };
    virtual RGY_ERR sendFrame([[maybe_unused]] std::unique_ptr<PipelineTaskOutput>& frame) override {
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
        auto inputFrame = std::make_unique<RGYFrame>(pSurface);
        auto err = m_input->LoadNextFrame(inputFrame.get());
        if (err == RGY_ERR_MORE_DATA) {// EOF
            err = RGY_ERR_MORE_BITSTREAM; // EOF を PipelineTaskMFXDecode のreturnコードに合わせる
        } else if (err != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("Error in reader: %s.\n"), get_err_mes(err));
        }
        auto surfInput = std::make_unique<RGYFrame>(pSurface);
        m_outQeueue.push_back(std::make_unique<PipelineTaskOutputSurf>(m_workSurfs.addSurface(surfInput)));
        return err;
    }
};

class PipelineTaskAMFDecode : public PipelineTask {
protected:
    amf::AMFComponentPtr m_dec;
    RGYInput *m_input;
    int m_decFrameOutCount;
    RGYQueueMPMP<RGYFrameDataMetadata*> m_queueHDR10plusMetadata;
    RGYRunState m_state;
#if THREAD_DEC_USE_FUTURE
    std::future m_thDecoder;
#else
    std::thread m_thDecoder;
#endif //#if THREAD_DEC_USE_FUTURE
public:
    PipelineTaskAMFDecode(amf::AMFComponentPtr dec, amf::AMFContextPtr context, int outMaxQueueSize, RGYInput *input, std::shared_ptr<RGYLog> log)
        : PipelineTask(PipelineTaskType::AMFDEC, context, outMaxQueueSize, log), m_dec(dec), m_input(input),
        m_decFrameOutCount(0), m_queueHDR10plusMetadata(),
        m_state(RGY_STATE_STOPPED), m_thDecoder() {
        m_queueHDR10plusMetadata.init(256);
    };
    virtual ~PipelineTaskAMFDecode() {
        m_state = RGY_STATE_ABORT;
        closeThread();
        m_queueHDR10plusMetadata.close([](RGYFrameDataMetadata **ptr) { if (*ptr) { delete *ptr; *ptr = nullptr; }; });
    };
    void setDec(amf::AMFComponentPtr dec) { m_dec = dec; };

    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override { return std::nullopt; };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override { return std::nullopt; };

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
                }
                bitstream.setSize(0);
                bitstream.setOffset(0);
                if (pictureBuffer || sts == RGY_ERR_MORE_BITSTREAM /*EOFの場合はDrainを送る*/) {
                    auto ar = AMF_OK;
                    do {
                        if (sts == RGY_ERR_MORE_BITSTREAM) {
                            ar = m_dec->Drain();
                        } else {
                            ar = m_dec->SubmitInput(pictureBuffer);
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
        amf::AMFSurfacePtr surf;
        auto ar = AMF_REPEAT;
        auto timeS = std::chrono::system_clock::now();
        if (m_state == RGY_STATE_STOPPED) {
            m_state = RGY_STATE_RUNNING;
            startThread();
        }
        while (m_state == RGY_STATE_RUNNING) {
            amf::AMFDataPtr data;
            ar = m_dec->QueryOutput(&data);
            if (ar == AMF_EOF) {
                break;
            }
            if (ar == AMF_REPEAT) {
                ar = AMF_OK; //これ重要...ここが欠けると最後の数フレームが欠落する
            }
            if (ar == AMF_OK && data != nullptr) {
                surf = amf::AMFSurfacePtr(data);
                break;
            }
            if (ar != AMF_OK) break;
            if ((std::chrono::system_clock::now() - timeS) > std::chrono::seconds(10)) {
                PrintMes(RGY_LOG_ERROR, _T("10 sec has passed after getting last frame from decoder.\n"));
                PrintMes(RGY_LOG_ERROR, _T("Decoder seems to have crushed.\n"));
                ar = AMF_FAIL;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (ar == AMF_EOF) {
            ret = RGY_ERR_MORE_DATA;
        } else if (ar != AMF_OK) {
            ret = err_to_rgy(ar); m_state = RGY_STATE_ERROR;
            PrintMes(RGY_LOG_ERROR, _T("Failed to load input frame: %s.\n"), get_err_mes(ret));
            return ret;
        }
        auto surfDecOut = std::make_unique<RGYFrame>(surf);
        if (surfDecOut != nullptr) {
            surfDecOut->clearDataList();
            surfDecOut->setInputFrameId(m_decFrameOutCount++);
            if (auto data = getMetadata(RGY_FRAME_DATA_HDR10PLUS, surfDecOut->timestamp()); data) {
                surfDecOut->dataList().push_back(data);
            }
            if (auto data = getMetadata(RGY_FRAME_DATA_DOVIRPU, surfDecOut->timestamp()); data) {
                surfDecOut->dataList().push_back(data);
            }
            m_outQeueue.push_back(std::make_unique<PipelineTaskOutputSurf>(m_workSurfs.addSurface(surfDecOut)));
        }
        return ret;
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

    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override { return std::nullopt; };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override { return std::nullopt; };

    std::pair<RGY_ERR, std::shared_ptr<RGYBitstream>> getOutputBitstream() {
        const auto VCE_TIMEBASE = rgy_rational<int>(1, AMF_SECOND);
        amf::AMFDataPtr data;
        auto ar = m_encoder->QueryOutput(&data);
        if (ar == AMF_REPEAT || (ar == AMF_OK && data == nullptr)) {
            return { RGY_ERR_MORE_DATA, nullptr };
        }
        if (ar == AMF_EOF) {
            return { RGY_ERR_MORE_BITSTREAM, nullptr };
        }
        if (ar != AMF_OK) {
            return { err_to_rgy(ar), nullptr };
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
        RGYFrame *surfEncodeIn = (frame) ? dynamic_cast<PipelineTaskOutputSurf *>(frame.get())->surf().frame() : nullptr;
        const bool drain = surfEncodeIn == nullptr;
        if (surfEncodeIn) {
            int64_t pts = surfEncodeIn->timestamp();
            int64_t duration = surfEncodeIn->duration();
            auto frameDataList = surfEncodeIn->dataList();
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
            if (out_ret == RGY_ERR_MORE_DATA) {
                ; // もっとエンコーダへの投入が必要
            } else if (out_ret == RGY_ERR_NONE) {
                if (outBs.second) {
                    m_outQeueue.push_back(std::make_unique<PipelineTaskOutputBitstream>(outBs.second));
                }
            } else if (out_ret == RGY_ERR_MORE_BITSTREAM) { //EOF
                break;
            } else {
                enc_sts = out_ret;
                break;
            }

            if (drain && ar == AMF_INPUT_FULL) {
                //エンコーダのflush
                try {
                    ar = m_encoder->Drain();
                } catch (...) {
                    PrintMes(RGY_LOG_ERROR, _T("Fatal error when submitting frame to encoder.\n"));
                    return RGY_ERR_UNKNOWN;
                }
                if (ar == AMF_INPUT_FULL) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                } else {
                    enc_sts = err_to_rgy(ar);
                }
            } else {
                //エンコードへの投入
                try {
                    ar = m_encoder->SubmitInput(pSurface);
                } catch (...) {
                    PrintMes(RGY_LOG_ERROR, _T("Fatal error when submitting frame to encoder.\n"));
                    return RGY_ERR_UNKNOWN;
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














#endif //__VCE_PIPELINE_H__

