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
#include "rgy_parallel_enc.h"

static const int RGY_WAIT_INTERVAL = 60000;

enum RGYRunState {
    RGY_STATE_STOPPED,
    RGY_STATE_RUNNING,
    RGY_STATE_ERROR,
    RGY_STATE_ABORT,
    RGY_STATE_EOF
};

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

class PipelineTaskStopWatch {
    std::array<std::vector<std::pair<tstring, int64_t>>, 2> m_ticks;
    std::array<std::chrono::high_resolution_clock::time_point, 2> m_prevTimepoints;
public:
    PipelineTaskStopWatch(const std::vector<tstring>& tickSend, const std::vector<tstring>& tickGet) : m_ticks(), m_prevTimepoints() {
        for (size_t i = 0; i < tickSend.size(); i++) {
            m_ticks[0].push_back({ tickSend[i], 0 });
        }
        for (size_t i = 0; i < tickGet.size(); i++) {
            m_ticks[1].push_back({ tickGet[i], 0 });
        }
    };
    void set(const int type) {
        m_prevTimepoints[type] = std::chrono::high_resolution_clock::now();
    }
    void add(const int type, const int idx) {
        auto now = std::chrono::high_resolution_clock::now();
        m_ticks[type][idx].second += std::chrono::duration_cast<std::chrono::nanoseconds>(now - m_prevTimepoints[type]).count();
        m_prevTimepoints[type] = now;
    }
    int64_t totalTicks() const {
        int64_t total = 0;
        for (int itype = 0; itype < 2; itype++) {
            for (int i = 0; i < (int)m_ticks[itype].size(); i++) {
                total += m_ticks[itype][i].second;
            }
        }
        return total;
    }
    size_t maxWorkStrLen() const {
        size_t maxLen = 0;
        for (size_t itype = 0; itype < m_ticks.size(); itype++) {
            for (int i = 0; i < (int)m_ticks[itype].size(); i++) {
                maxLen = (std::max)(maxLen, m_ticks[itype][i].first.length());
            }
        }
        return maxLen;
    }
    tstring print(const int64_t totalTicks, const size_t maxLen) {
        const TCHAR *type[] = {_T("send"), _T("get ")};
        tstring str;
        for (size_t itype = 0; itype < m_ticks.size(); itype++) {
            int64_t total = 0;
            for (int i = 0; i < (int)m_ticks[itype].size(); i++) {
                str += type[itype] + tstring(_T(":"));
                str += m_ticks[itype][i].first;
                str += tstring(maxLen - m_ticks[itype][i].first.length(), _T(' '));
                str += strsprintf(_T(" : %8d ms [%5.1f]\n"), ((m_ticks[itype][i].second + 500000) / 1000000), m_ticks[itype][i].second * 100.0 / totalTicks);
                total += m_ticks[itype][i].second;
            }
            if (m_ticks[itype].size() > 1) {
                str += type[itype] + tstring(_T(":"));
                str += _T("total");
                str += tstring(maxLen - _tcslen(_T("total")), _T(' '));
                str += strsprintf(_T(" : %8d ms [%5.1f]\n"), ((total + 500000) / 1000000), total * 100.0 / totalTicks);
            }
        }
        return str;
    }
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
        auto amfsurf = m_surf.amf();
        amfsurf->amf()->Convert(amf::AMF_MEMORY_HOST);
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
    PECOLLECT,
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
    case PipelineTaskType::PECOLLECT:   return _T("PECOLLECT");
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
    case PipelineTaskType::PECOLLECT:
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
    std::unique_ptr<PipelineTaskStopWatch> m_stopwatch;
public:
    PipelineTask() : m_type(PipelineTaskType::UNKNOWN), m_context(), m_outQeueue(), m_workSurfs(), m_inFrames(0), m_outFrames(0), m_outMaxQueueSize(0), m_log() {};
    PipelineTask(PipelineTaskType type, amf::AMFContextPtr conetxt, int outMaxQueueSize, std::shared_ptr<RGYLog> log) :
        m_type(type), m_context(conetxt), m_outQeueue(), m_workSurfs(), m_inFrames(0), m_outFrames(0), m_outMaxQueueSize(outMaxQueueSize), m_log(log), m_stopwatch() {
    };
    virtual ~PipelineTask() {
        m_workSurfs.clear();
    }
    virtual void setStopWatch() {};
    virtual void printStopWatch(const int64_t totalTicks, const size_t maxLen) {
        if (m_stopwatch) {
            const auto strlines = split(m_stopwatch->print(totalTicks, maxLen), _T("\n"));
            for (auto& str : strlines) {
                if (str.length() > 0) {
                    PrintMes(RGY_LOG_INFO, _T("%s\n"), str.c_str());
                }
            }
        }
    }
    virtual int64_t getStopWatchTotal() const {
        return (m_stopwatch) ? m_stopwatch->totalTicks() : 0ll;
    }
    virtual size_t getStopWatchMaxWorkStrLen() const {
        return (m_stopwatch) ? m_stopwatch->maxWorkStrLen() : 0u;
    }
    virtual bool isPassThrough() const { return false; }
    virtual tstring print() const { return getPipelineTaskTypeName(m_type); }
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() = 0;
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() = 0;
    virtual RGY_ERR sendFrame(std::unique_ptr<PipelineTaskOutput>& frame) = 0;
    virtual RGY_ERR getOutputFrameInfo(RGYFrameInfo& info) { info = RGYFrameInfo(); return RGY_ERR_NONE; }
    virtual std::vector<std::unique_ptr<PipelineTaskOutput>> getOutput(const bool sync) {
        if (m_stopwatch) m_stopwatch->set(1);
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
        if (m_stopwatch) m_stopwatch->add(1, 0);
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
    int64_t m_endPts; // 並列処理時用の終了時刻 (この時刻は含まないようにする) -1の場合は制限なし(最後まで)
public:
    PipelineTaskInput(amf::AMFContextPtr context, int64_t endPts, int outMaxQueueSize, RGYInput *input, std::shared_ptr<RGYOpenCLContext> cl, std::shared_ptr<RGYLog> log)
        : PipelineTask(PipelineTaskType::INPUT, context, outMaxQueueSize, log), m_input(input), m_cl(cl), m_endPts(endPts) {

    };
    virtual ~PipelineTaskInput() {};
    virtual void setStopWatch() override {
        m_stopwatch = std::make_unique<PipelineTaskStopWatch>(
            std::vector<tstring>{ _T("getWorkSurf"), _T("queueMapBuffer"), _T("LoadNextFrame"), _T("unmapBuffer") },
            std::vector<tstring>{_T("")}
        );
    }
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
        if (m_stopwatch) m_stopwatch->add(0, 0);
        auto clframe = surfWork.cl();
        auto err = clframe->queueMapBuffer(m_cl->queue(), CL_MAP_WRITE); // CPUが書き込むためにMapする
        if (err != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to map buffer: %s.\n"), get_err_mes(err));
            return err;
        }
        clframe->mapWait(); //すぐ終わるはず
        if (m_stopwatch) m_stopwatch->add(0, 1);
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
        if (m_stopwatch) m_stopwatch->add(0, 2);
        clframe->setPropertyFrom(mappedframe);
        auto clerr = clframe->unmapBuffer();
        if (clerr != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("Failed to unmap buffer: %s.\n"), get_err_mes(err));
            if (err == RGY_ERR_NONE) {
                err = clerr;
            }
        }
        if (m_endPts >= 0
            && (int64_t)mappedframe->timestamp() != AV_NOPTS_VALUE // timestampが設定されていない場合は無視
            && (int64_t)mappedframe->timestamp() >= m_endPts) { // m_endPtsは含まないようにする(重要)
            return RGY_ERR_MORE_BITSTREAM; //入力ビットストリームは終了
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
        if (m_stopwatch) m_stopwatch->add(0, 3);
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
        if (m_stopwatch) m_stopwatch->add(0, 0);
        pSurface->SetFrameType(frametype_rgy_to_enc(inputFrameInfo.picstruct));
        auto inputFrame = std::make_unique<RGYFrameAMF>(pSurface);
        auto err = m_input->LoadNextFrame(inputFrame.get());
        if (err == RGY_ERR_MORE_DATA) {// EOF
            err = RGY_ERR_MORE_BITSTREAM; // EOF を PipelineTaskMFXDecode のreturnコードに合わせる
        } else if (err != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("Error in reader: %s.\n"), get_err_mes(err));
        } else {
            if (m_endPts >= 0
                && (int64_t)inputFrame->timestamp() != AV_NOPTS_VALUE // timestampが設定されていない場合は無視
                && (int64_t)inputFrame->timestamp() >= m_endPts) { // m_endPtsは含まないようにする(重要)
                return RGY_ERR_MORE_BITSTREAM; //入力ビットストリームは終了
            }
            inputFrame->setInputFrameId(m_inFrames++);
            m_outQeueue.push_back(std::make_unique<PipelineTaskOutputSurf>(m_workSurfs.addSurface(inputFrame)));
        }
        if (m_stopwatch) m_stopwatch->add(0, 2);
        return err;
    }
    virtual RGY_ERR sendFrame([[maybe_unused]] std::unique_ptr<PipelineTaskOutput>& frame) override {
        if (m_stopwatch) m_stopwatch->set(0);
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
    bool m_gotFrameFirstKeyPts;
    bool m_reachedEndPts;
    int64_t m_firstKeyPts;
    int64_t m_endPts; // 並列処理時用の終了時刻 (この時刻は含まないようにする) -1の場合は制限なし(最後まで)
    int m_decOutFrames;
#if THREAD_DEC_USE_FUTURE
    std::future m_thDecoder;
#else
    std::thread m_thDecoder;
#endif //#if THREAD_DEC_USE_FUTURE
public:
    PipelineTaskAMFDecode(amf::AMFComponentPtr dec, amf::AMFContextPtr context, int64_t endPts, int outMaxQueueSize, RGYInput *input, std::shared_ptr<RGYLog> log)
        : PipelineTask(PipelineTaskType::AMFDEC, context, outMaxQueueSize, log), m_dec(dec), m_input(input),
        m_queueHDR10plusMetadata(), m_dataFlag(),
        m_state(RGY_STATE_STOPPED), m_gotFrameFirstKeyPts(false), m_reachedEndPts(false), m_firstKeyPts(AV_NOPTS_VALUE), m_endPts(endPts), m_decOutFrames(0), m_thDecoder() {
        m_queueHDR10plusMetadata.init(256);
        m_dataFlag.init();
    };
    virtual ~PipelineTaskAMFDecode() {
        m_state = RGY_STATE_ABORT;
        closeThread();
        m_queueHDR10plusMetadata.close([](RGYFrameDataMetadata **ptr) { if (*ptr) { delete *ptr; *ptr = nullptr; }; });
    };
    virtual void setStopWatch() override {
        m_stopwatch = std::make_unique<PipelineTaskStopWatch>(
            std::vector<tstring>{ _T("QueryOutput"), _T("Duplicate") },
            std::vector<tstring>{_T("")}
        );
    }
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
                if (m_reachedEndPts) { // endPtsに到達したらファイル読み込み終了と同じ動作をさせる
                    sts = RGY_ERR_MORE_BITSTREAM;
                } else if (sts == RGY_ERR_NONE) {
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
                    if (m_firstKeyPts == AV_NOPTS_VALUE) {
                        m_firstKeyPts = bitstream.pts();
                    }

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
                            continue; // データはまだ使用されていないので、再度呼び出し
                        } else {
                            break;
                        }
                    } while (m_state == RGY_STATE_RUNNING);
                    if (ar != AMF_OK && ar != AMF_NEED_MORE_INPUT) {
                        m_state = RGY_STATE_ERROR;
                        PrintMes(RGY_LOG_ERROR, _T("ERROR: Unexpected error while submitting bitstream to decoder: %s.\n"), get_err_mes(err_to_rgy(ar)));
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
        if (m_stopwatch) m_stopwatch->set(0);
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
        if (m_stopwatch) m_stopwatch->add(0, 0);
        if (ar == AMF_EOF || m_state == RGY_STATE_EOF) {
            ret = RGY_ERR_MORE_BITSTREAM;
        } else if (ar != AMF_OK) {
            ret = err_to_rgy(ar); m_state = RGY_STATE_ERROR;
            PrintMes(RGY_LOG_ERROR, _T("Failed to load input frame: %s.\n"), get_err_mes(ret));
            return ret;
        }
        if (!m_gotFrameFirstKeyPts && surfDecOut != nullptr && surfDecOut->GetPts() < m_firstKeyPts) {
            return RGY_ERR_NONE;
        }
        m_gotFrameFirstKeyPts = true;
        if (m_endPts >= 0
            && surfDecOut != nullptr
            && surfDecOut->GetPts() >= m_endPts) { // m_endPtsは含まないようにする(重要)
            m_reachedEndPts = true; // 読み込みスレッドに終了を通知
            return RGY_ERR_MORE_BITSTREAM; //入力ビットストリームは終了
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
        if (m_stopwatch) m_stopwatch->add(0, 1);
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
                    } else if (frameDataPtr->dataType() == RGY_FRAME_DATA_DOVIRPU) {
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
    bool m_timestampPassThrough;
    bool m_vpp_rff;
    bool m_vpp_afs_rff_aware;
    int64_t m_outFrameDuration; //(m_outputTimebase基準)
    int64_t m_tsOutFirst;     //(m_outputTimebase基準)
    int64_t m_tsOutEstimated; //(m_outputTimebase基準)
    int64_t m_tsPrev;         //(m_outputTimebase基準)
    uint32_t m_inputFramePosIdx;
    FramePosList *m_framePosList;
    static const int64_t INVALID_PTS = AV_NOPTS_VALUE;
public:
    PipelineTaskCheckPTS(amf::AMFContextPtr context, rgy_rational<int> srcTimebase, rgy_rational<int> streamTimebase, rgy_rational<int> outputTimebase, int64_t outFrameDuration, RGYAVSync avsync, bool timestampPassThrough, bool vpp_afs_rff_aware, FramePosList *framePosList, std::shared_ptr<RGYLog> log) :
        PipelineTask(PipelineTaskType::CHECKPTS, context, /*outMaxQueueSize = */ 0 /*常に0である必要がある*/, log),
        m_srcTimebase(srcTimebase), m_streamTimebase(streamTimebase), m_outputTimebase(outputTimebase), m_avsync(avsync), m_timestampPassThrough(timestampPassThrough), m_vpp_rff(false), m_vpp_afs_rff_aware(vpp_afs_rff_aware), m_outFrameDuration(outFrameDuration),
        m_tsOutFirst(AV_NOPTS_VALUE), m_tsOutEstimated(0), m_tsPrev(-1), m_inputFramePosIdx(std::numeric_limits<decltype(m_inputFramePosIdx)>::max()), m_framePosList(framePosList) {
    };
    virtual ~PipelineTaskCheckPTS() {};

    virtual void setStopWatch() override {
        m_stopwatch = std::make_unique<PipelineTaskStopWatch>(
            std::vector<tstring>{ _T("") },
            std::vector<tstring>{_T("")}
        );
    }

    virtual bool isPassThrough() const override {
        // そのまま渡すのでpaththrough
        return true;
    }
    static const int MAX_FORCECFR_INSERT_FRAMES = 1024; //事実上の無制限
public:
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override { return std::nullopt; };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override { return std::nullopt; };

    virtual RGY_ERR sendFrame(std::unique_ptr<PipelineTaskOutput>& frame) override {
        if (m_stopwatch) m_stopwatch->set(0);
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
            && ((m_avsync & (RGY_AVSYNC_VFR | RGY_AVSYNC_FORCE_CFR)) || m_vpp_rff || m_vpp_afs_rff_aware || m_timestampPassThrough)) {
            //CFR仮定ではなく、オリジナルの時間を見る
            const auto srcTimestamp = taskSurf->surf().frame()->timestamp();
            if (srcTimestamp == AV_NOPTS_VALUE) {
                outPtsSource = m_tsPrev + m_outFrameDuration + m_tsOutFirst/*あとでm_tsOutFirstが引かれるので*/;
            } else {
                outPtsSource = rational_rescale(srcTimestamp, m_srcTimebase, m_outputTimebase);
            }
            if (taskSurf->surf().frame()->duration() > 0) {
                outDuration = rational_rescale(taskSurf->surf().frame()->duration(), m_srcTimebase, m_outputTimebase);
                taskSurf->surf().frame()->setDuration(outDuration);
            }
        }
        PrintMes(RGY_LOG_TRACE, _T("check_pts(%d/%d): nOutEstimatedPts %lld, outPtsSource %lld, outDuration %d\n"), taskSurf->surf().frame()->inputFrameId(), m_inFrames, m_tsOutEstimated, outPtsSource, outDuration);
        if (m_tsOutFirst == INVALID_PTS) {
            m_tsOutFirst = outPtsSource; //最初のpts
            PrintMes(RGY_LOG_TRACE, _T("check_pts: m_tsOutFirst %lld\n"), outPtsSource);
        }
        //最初のptsを0に修正
        if (!m_timestampPassThrough) {
            //最初のptsを0に修正
            outPtsSource -= m_tsOutFirst;
        }

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
        if (m_stopwatch) m_stopwatch->add(0, 0);
        return RGY_ERR_NONE;
    }
    //checkptsではtimestampを上書きするため特別に常に1フレームしか取り出さない
    //これは--avsync frocecfrでフレームを参照コピーする際、
    //mfxSurface1自体は同じデータを指すため、複数のタイムスタンプを持つことができないため、
    //1フレームずつgetOutputし、都度タイムスタンプを上書きしてすぐに後続のタスクに投入してタイムスタンプを反映させる必要があるため
    virtual std::vector<std::unique_ptr<PipelineTaskOutput>> getOutput(const bool sync) override {
        if (m_stopwatch) m_stopwatch->set(1);
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
        if (m_stopwatch) m_stopwatch->add(1, 0);
        return output;
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

    virtual void setStopWatch() override {
        m_stopwatch = std::make_unique<PipelineTaskStopWatch>(
            std::vector<tstring>{ _T("") },
            std::vector<tstring>{_T("")}
        );
    }

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
        if (m_stopwatch) m_stopwatch->set(0);
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
        if (m_stopwatch) m_stopwatch->add(0, 0);
        return RGY_ERR_NONE;
    }
};

class PipelineTaskParallelEncBitstream : public PipelineTask {
protected:
    RGYInput *m_input;
    int m_currentChunk; // いま並列処理の何番目を処理中か
    RGYTimestamp *m_encTimestamp;
    RGYTimecode *m_timecode;
    RGYParallelEnc *m_parallelEnc;
    EncodeStatus *m_encStatus;
    rgy_rational<int> m_encFps;
    rgy_rational<int> m_outputTimebase;
    std::unique_ptr<PipelineTaskAudio> m_taskAudio;
    std::unique_ptr<FILE, fp_deleter> m_fReader;
    int64_t m_firstPts; //最初のpts
    int64_t m_maxPts; // 最後のpts
    int64_t m_ptsOffset; // 分割出力間の(2分割目以降の)ptsのオフセット
    int64_t m_encFrameOffset; // 分割出力間の(2分割目以降の)エンコードフレームのオフセット
    int64_t m_inputFrameOffset; // 分割出力間の(2分割目以降の)エンコードフレームのオフセット
    int64_t m_maxEncFrameIdx; // 最後にエンコードしたフレームのindex
    int64_t m_maxInputFrameIdx; // 最後にエンコードしたフレームのindex
    RGYBitstream m_decInputBitstream; // 映像読み込み (ダミー)
    bool m_inputBitstreamEOF; // 映像側の読み込み終了フラグ (音声処理の終了も確認する必要があるため)
    RGYListRef<RGYBitstream> m_bitStreamOut;
    RGYDurationCheck m_durationCheck;
    bool m_tsDebug;
public:
    PipelineTaskParallelEncBitstream(amf::AMFContextPtr context, RGYInput *input, RGYTimestamp *encTimestamp, RGYTimecode *timecode, RGYParallelEnc *parallelEnc, EncodeStatus *encStatus,
        rgy_rational<int> encFps, rgy_rational<int> outputTimebase,
        std::unique_ptr<PipelineTaskAudio>& taskAudio, int outMaxQueueSize, std::shared_ptr<RGYLog> log) :
        PipelineTask(PipelineTaskType::PECOLLECT, context, outMaxQueueSize, log),
        m_input(input), m_currentChunk(-1), m_encTimestamp(encTimestamp), m_timecode(timecode),
        m_parallelEnc(parallelEnc), m_encStatus(encStatus), m_encFps(encFps), m_outputTimebase(outputTimebase),
        m_taskAudio(std::move(taskAudio)), m_fReader(std::unique_ptr<FILE, fp_deleter>(nullptr, fp_deleter())),
        m_firstPts(-1), m_maxPts(-1), m_ptsOffset(0), m_encFrameOffset(0), m_inputFrameOffset(0), m_maxEncFrameIdx(-1), m_maxInputFrameIdx(-1),
        m_decInputBitstream(), m_inputBitstreamEOF(false), m_bitStreamOut(), m_durationCheck(), m_tsDebug(false) {
        m_decInputBitstream.init(AVCODEC_READER_INPUT_BUF_SIZE);
        auto reader = dynamic_cast<RGYInputAvcodec*>(input);
        if (reader) {
            // 親側で不要なデコーダを終了させる、こうしないとavsw使用時に映像が無駄にデコードされてしまう
            reader->CloseVideoDecoder();
        }
    };
    virtual ~PipelineTaskParallelEncBitstream() {
        m_decInputBitstream.clear();
    };

    virtual bool isPassThrough() const override { return true; }

    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override { return std::nullopt; };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override { return std::nullopt; };
protected:
    RGY_ERR checkEncodeResult() {
        // まずそのエンコーダの終了を待機
        while (m_parallelEnc->waitProcessFinished(m_currentChunk, UPDATE_INTERVAL) != WAIT_OBJECT_0) {
            // 進捗表示の更新
            auto currentData = m_encStatus->GetEncodeData();
            m_encStatus->UpdateDisplay(currentData.progressPercent);
        }
        // 戻り値を確認
        auto procsts = m_parallelEnc->processReturnCode(m_currentChunk);
        if (!procsts.has_value()) { // そんなはずはないのだが、一応
            PrintMes(RGY_LOG_ERROR, _T("Unknown error in parallel enc: %d.\n"), m_currentChunk);
            return RGY_ERR_UNKNOWN;
        }
        if (procsts.value() != RGY_ERR_NONE) {
            PrintMes(RGY_LOG_ERROR, _T("Error in parallel enc %d: %s\n"), m_currentChunk, get_err_mes(procsts.value()));
            return procsts.value();
        }
        return RGY_ERR_NONE;
    }

    RGY_ERR openNextFile() {
        if (m_currentChunk >= 0 && m_parallelEnc->cacheMode(m_currentChunk) == RGYParamParallelEncCache::Mem) {
            // メモリモードの場合は、まだそのエンコーダの戻り値をチェックしていないので、ここでチェック
            auto procsts = checkEncodeResult();
            if (procsts != RGY_ERR_NONE) {
                PrintMes(RGY_LOG_ERROR, _T("Error in parallel enc %d: %s\n"), m_currentChunk, get_err_mes(procsts));
                return procsts;
            }
        }

        m_currentChunk++;
        if (m_currentChunk >= (int)m_parallelEnc->parallelCount()) {
            return RGY_ERR_MORE_BITSTREAM;
        }
        
        if (m_parallelEnc->cacheMode(m_currentChunk) == RGYParamParallelEncCache::File) {
            // 戻り値を確認
            auto procsts = checkEncodeResult();
            if (procsts != RGY_ERR_NONE) {
                PrintMes(RGY_LOG_ERROR, _T("Error in parallel enc %d: %s\n"), m_currentChunk, get_err_mes(procsts));
                return procsts;
            }
            // ファイルを開く
            auto tmpPath = m_parallelEnc->tmpPath(m_currentChunk);
            if (tmpPath.empty()) {
                PrintMes(RGY_LOG_ERROR, _T("Failed to get tmp path for parallel enc %d.\n"), m_currentChunk);
                return RGY_ERR_UNKNOWN;
            }
            m_fReader = std::unique_ptr<FILE, fp_deleter>(_tfopen(tmpPath.c_str(), _T("rb")), fp_deleter());
            if (m_fReader == nullptr) {
                PrintMes(RGY_LOG_ERROR, _T("Failed to open file: %s\n"), tmpPath.c_str());
                return RGY_ERR_FILE_OPEN;
            }
        }
        //最初のファイルに対するptsの差を取り、それをtimebaseを変換して適用する
        const auto inputFrameInfo = m_input->GetInputFrameInfo();
        const auto inputFpsTimebase = rgy_rational<int>((int)inputFrameInfo.fpsD, (int)inputFrameInfo.fpsN);
        const auto srcTimebase = (m_input->getInputTimebase().n() > 0 && m_input->getInputTimebase().is_valid()) ? m_input->getInputTimebase() : inputFpsTimebase;
        // seek結果による入力ptsを用いて計算した本来のpts offset
        const auto ptsOffsetOrig = (m_firstPts < 0) ? 0 : rational_rescale(m_parallelEnc->getVideofirstKeyPts(m_currentChunk), srcTimebase, m_outputTimebase) - m_firstPts;
        // 直前のフレームから計算したpts offset(-1フレーム分) 最低でもこれ以上のoffsetがないといけない
        const auto ptsOffsetMax = (m_firstPts < 0) ? 0 : m_maxPts - m_firstPts;
        // フレームの長さを決める
        int64_t lastDuration = 0;
        const auto frameDuration = m_durationCheck.getDuration(lastDuration);
        // frameDuration のうち、登場回数が最も多いものを探す
        int mostFrequentDuration = 0;
        int64_t mostFrequentDurationCount = 0;
        int64_t totalFrameCount = 0;
        for (const auto& [duration, count] : frameDuration) {
            if (count > mostFrequentDurationCount) {
                mostFrequentDuration = duration;
                mostFrequentDurationCount = count;
            }
            totalFrameCount += count;
        }
        // フレーム長が1つしかない場合、あるいは登場頻度の高いフレーム長がある場合、そのフレーム長を採用する
        if (frameDuration.size() == 1 || ((totalFrameCount * 9 / 10) < mostFrequentDurationCount)) {
            m_ptsOffset = ptsOffsetMax + mostFrequentDuration;
        } else if (frameDuration.size() == 2) {
            if ((totalFrameCount * 7 / 10) < mostFrequentDurationCount || lastDuration != mostFrequentDuration) {
                m_ptsOffset = ptsOffsetMax + mostFrequentDuration;
            } else {
                int otherDuration = mostFrequentDuration;
                for (auto itr = frameDuration.begin(); itr != frameDuration.end(); itr++) {
                    if (itr->first != mostFrequentDuration) {
                        otherDuration = itr->first;
                        break;
                    }
                }
                m_ptsOffset = ptsOffsetMax + otherDuration;
            }
        } else {
            // ptsOffsetOrigが必要offsetの最小値(ptsOffsetMax)より大きく、そのずれが2フレーム以内ならそれを採用する
            // そうでなければ、ptsOffsetMaxに1フレーム分の時間を足した時刻にする
            m_ptsOffset = (m_firstPts < 0) ? 0 :
                ((ptsOffsetOrig - ptsOffsetMax > 0 && ptsOffsetOrig - ptsOffsetMax <= rational_rescale(2, m_encFps.inv(), m_outputTimebase))
                    ? ptsOffsetOrig : (ptsOffsetMax + rational_rescale(1, m_encFps.inv(), m_outputTimebase)));
        }
        m_encFrameOffset = (m_currentChunk > 0) ? m_maxEncFrameIdx + 1 : 0;
        m_inputFrameOffset = (m_currentChunk > 0) ? m_maxInputFrameIdx + 1 : 0;
        PrintMes(m_tsDebug ? RGY_LOG_ERROR : RGY_LOG_TRACE, _T("Switch to next file: pts offset %lld, frame offset %d.\n")
            _T("  firstKeyPts 0: % lld, %d : % lld.\n")
            _T("  ptsOffsetOrig: %lld, ptsOffsetMax: %lld, m_maxPts: %lld\n"),
            m_ptsOffset, m_encFrameOffset,
            m_firstPts, m_currentChunk, rational_rescale(m_parallelEnc->getVideofirstKeyPts(m_currentChunk), srcTimebase, m_outputTimebase),
            ptsOffsetOrig, ptsOffsetMax, m_maxPts);
        return RGY_ERR_NONE;
    }

    void updateAndSetHeaderProperties(RGYBitstream *bsOut, RGYOutputRawPEExtHeader *header) {
        header->pts += m_ptsOffset;
        header->dts += m_ptsOffset;
        header->encodeFrameIdx += m_encFrameOffset;
        header->inputFrameIdx += m_inputFrameOffset;
        bsOut->setPts(header->pts);
        bsOut->setDts(header->dts);
        bsOut->setDuration(header->duration);
        bsOut->setFrametype(header->frameType);
        bsOut->setPicstruct(header->picstruct);
        bsOut->setFrameIdx((int)header->encodeFrameIdx);
        bsOut->setDataflag((RGY_FRAME_FLAGS)header->flags);
    }

    RGY_ERR getBitstreamOneFrameFromQueue(RGYBitstream *bsOut, RGYOutputRawPEExtHeader& header) {
        RGYOutputRawPEExtHeader *packet = nullptr;
        auto err = m_parallelEnc->getNextPacket(m_currentChunk, &packet);
        if (err != RGY_ERR_NONE) {
            return err;
        }
        if (packet == nullptr) {
            return RGY_ERR_UNDEFINED_BEHAVIOR;
        }
        updateAndSetHeaderProperties(bsOut, packet);
        if (packet->size <= 0) {
            return RGY_ERR_UNDEFINED_BEHAVIOR;
        } else {
            bsOut->resize(packet->size);
            memcpy(&header, packet, sizeof(header));
            memcpy(bsOut->data(), (void *)(packet + 1), packet->size);
        }
        // メモリを使いまわすため、使い終わったパケットを回収する
        m_parallelEnc->putFreePacket(m_currentChunk, packet);
        PrintMes(RGY_LOG_TRACE, _T("Q: pts %08lld, dts %08lld, size %d.\n"), bsOut->pts(), bsOut->dts(), bsOut->size());
        return RGY_ERR_NONE;
    }

    RGY_ERR getBitstreamOneFrameFromFile(FILE *fp, RGYBitstream *bsOut, RGYOutputRawPEExtHeader& header) {
        if (fread(&header, 1, sizeof(header), fp) != sizeof(header)) {
            return RGY_ERR_MORE_BITSTREAM;
        }
        if (header.size <= 0) {
            return RGY_ERR_UNDEFINED_BEHAVIOR;
        }
        updateAndSetHeaderProperties(bsOut, &header);
        bsOut->resize(header.size);
        PrintMes(RGY_LOG_TRACE, _T("F: pts %08lld, dts %08lld, size %d.\n"), bsOut->pts(), bsOut->dts(), bsOut->size());

        if (fread(bsOut->data(), 1, bsOut->size(), fp) != bsOut->size()) {
            return RGY_ERR_UNDEFINED_BEHAVIOR;
        }
        return RGY_ERR_NONE;
    }

    RGY_ERR getBitstreamOneFrame(RGYBitstream *bsOut, RGYOutputRawPEExtHeader& header) {
        return (m_parallelEnc->cacheMode(m_currentChunk) == RGYParamParallelEncCache::File)
            ? getBitstreamOneFrameFromFile(m_fReader.get(), bsOut, header)
            : getBitstreamOneFrameFromQueue(bsOut, header);
    }

    virtual RGY_ERR getBitstream(RGYBitstream *bsOut, RGYOutputRawPEExtHeader& header) {
        if (m_currentChunk < 0) {
            if (auto err = openNextFile(); err != RGY_ERR_NONE) {
                return err;
            }
        } else if (m_currentChunk >= (int)m_parallelEnc->parallelCount()) {
            return RGY_ERR_MORE_BITSTREAM;
        }
        auto err = getBitstreamOneFrame(bsOut, header);
        if (err == RGY_ERR_MORE_BITSTREAM) {
            if ((err = openNextFile()) != RGY_ERR_NONE) {
                return err;
            }
            err = getBitstreamOneFrame(bsOut, header);
        }
        return err;
    }
public:
    virtual RGY_ERR sendFrame([[maybe_unused]] std::unique_ptr<PipelineTaskOutput>& frame) override {
        m_inFrames++;
        auto ret = m_input->LoadNextFrame(nullptr); // 進捗表示用のダミー
        if (ret != RGY_ERR_NONE && ret != RGY_ERR_MORE_DATA && ret != RGY_ERR_MORE_BITSTREAM) {
            PrintMes(RGY_LOG_ERROR, _T("Error in reader: %s.\n"), get_err_mes(ret));
            return ret;
        }
        m_inputBitstreamEOF |= (ret == RGY_ERR_MORE_DATA || ret == RGY_ERR_MORE_BITSTREAM);

        // 音声等抽出のため、入力ファイルの読み込みを進める
        //この関数がMFX_ERR_NONE以外を返せば、入力ビットストリームは終了
        ret = m_input->GetNextBitstream(&m_decInputBitstream);
        m_inputBitstreamEOF |= (ret == RGY_ERR_MORE_DATA || ret == RGY_ERR_MORE_BITSTREAM);
        if (ret != RGY_ERR_NONE && ret != RGY_ERR_MORE_DATA && ret != RGY_ERR_MORE_BITSTREAM) {
            PrintMes(RGY_LOG_ERROR, _T("Error in reader: %s.\n"), get_err_mes(ret));
            return ret; //エラー
        }
        m_decInputBitstream.clear();

        if (m_taskAudio) {
            ret = m_taskAudio->extractAudio(m_inFrames);
            if (ret != RGY_ERR_NONE) {
                return ret;
            }
        }

        // 定期的に全スレッドでエラー終了したものがないかチェックする
        if ((m_inFrames & 15) == 0) {
            if ((ret = m_parallelEnc->checkAllProcessErrors()) != RGY_ERR_NONE) {
                return ret; //エラー
            }
        }

        auto bsOut = m_bitStreamOut.get([](RGYBitstream *bs) {
            *bs = RGYBitstreamInit();
            return 0;
        });
        RGYOutputRawPEExtHeader header;
        ret = getBitstream(bsOut.get(), header);
        if (ret != RGY_ERR_NONE && ret != RGY_ERR_MORE_BITSTREAM) {
            return ret;
        }
        if (ret == RGY_ERR_NONE && bsOut->size() > 0) {
            std::vector<std::shared_ptr<RGYFrameData>> metadatalist;
            const auto duration = (ENCODER_QSV) ? header.duration : bsOut->duration(); // QSVの場合、Bitstreamにdurationの値がないため、durationはheaderから取得する
            m_encTimestamp->add(bsOut->pts(), header.inputFrameIdx, header.encodeFrameIdx, duration, metadatalist);
            if (m_firstPts < 0) m_firstPts = bsOut->pts();
            m_maxPts = std::max(m_maxPts, bsOut->pts());
            m_maxEncFrameIdx = std::max(m_maxEncFrameIdx, header.encodeFrameIdx);
            m_maxInputFrameIdx = std::max(m_maxInputFrameIdx, header.inputFrameIdx);
            PrintMes(m_tsDebug ? RGY_LOG_WARN : RGY_LOG_TRACE, _T("Packet: pts %lld, dts: %lld, duration: %d, input idx: %lld, encode idx: %lld, size %lld.\n"), bsOut->pts(), bsOut->dts(), duration, header.inputFrameIdx, header.encodeFrameIdx, bsOut->size());
            if (m_timecode) {
                m_timecode->write(bsOut->pts(), m_outputTimebase);
            }
            m_durationCheck.add(bsOut->pts());
            m_outQeueue.push_back(std::make_unique<PipelineTaskOutputBitstream>(bsOut));
        }
        if (m_inputBitstreamEOF && ret == RGY_ERR_MORE_BITSTREAM && m_taskAudio) {
            m_taskAudio->flushAudio();
        }
        return (m_inputBitstreamEOF && ret == RGY_ERR_MORE_BITSTREAM) ? RGY_ERR_MORE_BITSTREAM : RGY_ERR_NONE;
    }
};

class PipelineTaskTrim : public PipelineTask {
protected:
    const sTrimParam &m_trimParam;
    RGYInput *m_input;
    RGYParallelEnc *m_parallelEnc;
    rgy_rational<int> m_srcTimebase;
public:
    PipelineTaskTrim(amf::AMFContextPtr context, const sTrimParam &trimParam, RGYInput *input, RGYParallelEnc *parallelEnc, const rgy_rational<int>& srcTimebase, int outMaxQueueSize, std::shared_ptr<RGYLog> log) :
        PipelineTask(PipelineTaskType::TRIM, context, outMaxQueueSize, log),
        m_trimParam(trimParam), m_input(input), m_parallelEnc(parallelEnc), m_srcTimebase(srcTimebase) {
    };
    virtual ~PipelineTaskTrim() {};

    virtual void setStopWatch() override {
        m_stopwatch = std::make_unique<PipelineTaskStopWatch>(
            std::vector<tstring>{ _T("") },
            std::vector<tstring>{_T("")}
        );
    }

    virtual bool isPassThrough() const override { return true; }
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override { return std::nullopt; };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override { return std::nullopt; };

    virtual RGY_ERR sendFrame(std::unique_ptr<PipelineTaskOutput>& frame) override {
        if (m_stopwatch) m_stopwatch->set(0);
        if (!frame) {
            return RGY_ERR_MORE_DATA;
        }
        m_inFrames++;
        PipelineTaskOutputSurf *taskSurf = dynamic_cast<PipelineTaskOutputSurf *>(frame.get());
        if (!frame_inside_range(taskSurf->surf().frame()->inputFrameId(), m_trimParam.list).first) {
            return RGY_ERR_NONE;
        }
        const auto surfPts = (int64_t)taskSurf->surf().frame()->timestamp();
        if (m_parallelEnc) {
            auto finKeyPts = m_parallelEnc->getVideoEndKeyPts();
            if (finKeyPts >= 0 && surfPts >= finKeyPts) {
                m_parallelEnc->setVideoFinished();
                return RGY_ERR_NONE;
            }
        }
        if (!m_input->checkTimeSeekTo(taskSurf->surf().frame()->timestamp(), m_srcTimebase)) {
            return RGY_ERR_NONE; //seektoにより脱落させるフレーム
        }
        m_outQeueue.push_back(std::make_unique<PipelineTaskOutputSurf>(taskSurf->surf()));
        if (m_stopwatch) m_stopwatch->add(0, 0);
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
    const RGYHDR10Plus *m_hdr10plus;
    const DOVIRpu *m_doviRpu;
public:
    PipelineTaskAMFEncode(
        amf::AMFComponentPtr enc, RGY_CODEC encCodec, AMFParams& encParams, amf::AMFContextPtr context, int outMaxQueueSize,
        RGYTimecode *timecode, RGYTimestamp *encTimestamp, rgy_rational<int> outputTimebase, const RGYHDR10Plus *hdr10plus, const DOVIRpu *doviRpu, std::shared_ptr<RGYLog> log)
        : PipelineTask(PipelineTaskType::AMFENC, context, outMaxQueueSize, log),
        m_encoder(enc), m_encCodec(encCodec), m_encParams(encParams), m_timecode(timecode), m_encTimestamp(encTimestamp), m_outputTimebase(outputTimebase), m_bitStreamOut(), m_hdr10plus(hdr10plus), m_doviRpu(doviRpu) {
    };
    virtual ~PipelineTaskAMFEncode() {
        m_outQeueue.clear(); // m_bitStreamOutが解放されるよう前にこちらを解放する
    };
    void setEnc(amf::AMFComponentPtr encode) { m_encoder = encode; };
    virtual void setStopWatch() override {
        m_stopwatch = std::make_unique<PipelineTaskStopWatch>(
            std::vector<tstring>{ _T("GetFramePorperties"), _T("getOutputBitstream"), _T("SubmitInput"), _T("Drain") },
            std::vector<tstring>{_T("")}
        );
    }

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
        if (m_stopwatch) m_stopwatch->set(0);
        if (frame && frame->type() != PipelineTaskOutputType::SURFACE) {
            PrintMes(RGY_LOG_ERROR, _T("Invalid frame type.\n"));
            return RGY_ERR_UNSUPPORTED;
        }

        std::vector<std::shared_ptr<RGYFrameData>> metadatalist;
        if (m_encCodec == RGY_CODEC_HEVC || m_encCodec == RGY_CODEC_AV1) {
            if (frame) {
                metadatalist = dynamic_cast<PipelineTaskOutputSurf *>(frame.get())->surf().frame()->dataList();
            }
            if (m_hdr10plus) {
                // 外部からHDR10+を読み込む場合、metadatalist 内のHDR10+の削除
                for (auto it = metadatalist.begin(); it != metadatalist.end(); ) {
                    if ((*it)->dataType() == RGY_FRAME_DATA_HDR10PLUS) {
                        it = metadatalist.erase(it);
                    } else {
                        it++;
                    }
                }
            }
            if (m_doviRpu) {
                // 外部からdoviを読み込む場合、metadatalist 内のdovi rpuの削除
                for (auto it = metadatalist.begin(); it != metadatalist.end(); ) {
                    if ((*it)->dataType() == RGY_FRAME_DATA_DOVIRPU) {
                        it = metadatalist.erase(it);
                    } else {
                        it++;
                    }
                }
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
            
            // ヘッダ関連に関してはm_encParams.Apply後にここで上書きする
            // 特にHEVCの場合、ここでフラグを設定すると常にヘッダーが入ってしまうので指定しない
            //if (m_encCodec == RGY_CODEC_H264) {
            //    //m_encParams.SetParam(AMF_VIDEO_ENCODER_INSERT_PPS, m_inFrames==0);
            //    //m_encParams.SetParam(AMF_VIDEO_ENCODER_INSERT_SPS, m_inFrames==0);
            //} else if (m_encCodec == RGY_CODEC_HEVC) {  
            //    //m_encParams.SetParam(AMF_VIDEO_ENCODER_HEVC_INSERT_HEADER, m_inFrames==0);
            //}

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
        if (m_stopwatch) m_stopwatch->add(0, 0);

        auto enc_sts = RGY_ERR_NONE;
        auto ar = (drain) ? AMF_INPUT_FULL : AMF_OK;
        for (;;) {
            if (m_stopwatch) m_stopwatch->set(0);
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
            if (m_stopwatch) m_stopwatch->add(0, 1);

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
                if (m_stopwatch) m_stopwatch->add(0, 3);
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
                if (m_stopwatch) m_stopwatch->add(0, 2);
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

    struct AMFFRCTmp {
        bool isFRC;
        int64_t output;
        int64_t prevPts;
        int64_t prevDuration;

        AMFFRCTmp() : isFRC(false), output(0), prevPts(0), prevDuration(0) {};

        void addFrame(int64_t& pts, int64_t& duration) {
            if (!isFRC) {
                return;
            }
            if ((output++) % 2 == 0) {
                // オリジナルのフレーム
                prevPts = pts;
                prevDuration = duration;
                duration = duration / 2;
                return;
            }
            if (prevDuration == 0) prevDuration = pts - prevPts;
            auto interpPts = std::min(prevPts + prevDuration / 2, pts - 1);
            auto interpDuration = pts - interpPts;
            pts = interpPts;
            duration = interpDuration;
        }
    } frc;

public:
    PipelineTaskAMFPreProcess(amf::AMFContextPtr context, std::unique_ptr<AMFFilter>& vppfilter, std::shared_ptr<RGYOpenCLContext> cl, int outMaxQueueSize, std::shared_ptr<RGYLog> log) :
        PipelineTask(PipelineTaskType::AMFVPP, context, outMaxQueueSize, log), m_cl(cl), m_vppFilter(vppfilter), m_vppOutFrames(), m_metadatalist(), m_prevInputFrame() {
        frc.isFRC = m_vppFilter->name() == AMFFilterFRC::FRC_FILTER_NAME;
    };
    virtual ~PipelineTaskAMFPreProcess() {
        m_prevInputFrame.clear();
        m_cl.reset();
    };
    virtual void setStopWatch() override {
        m_stopwatch = std::make_unique<PipelineTaskStopWatch>(
            std::vector<tstring>{ _T("depend_clear"), _T("Convert"), _T("QueryOutput"), _T("SubmitInput"), _T("Drain") },
            std::vector<tstring>{_T("")}
        );
    }
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override {
        return std::make_pair(m_vppFilter->GetFilterParam()->frameIn, 0);
    };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override {
        return std::make_pair(m_vppFilter->GetFilterParam()->frameOut, 0);
    };
    virtual RGY_ERR sendFrame(std::unique_ptr<PipelineTaskOutput>& frame) override {
        if (m_stopwatch) m_stopwatch->set(0);
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

        if (m_stopwatch) m_stopwatch->add(0, 0);
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

            // これをやらないと(pSurfaceがhostメモリの場合に)、
            // m_vppFilter->filter()->QueryOutput(&data)が返答を返さなくなる(関数内でフリーズしてしまう)
            if (pSurface->GetMemoryType() == amf::AMF_MEMORY_HOST) {
                pSurface->Convert(amf::AMF_MEMORY_OPENCL);
            }

            m_inFrames++;

            surfVppIn->clearDataList();
        }
        if (m_stopwatch) m_stopwatch->add(0, 1);

        auto add_surf_to_out_queue = [&](amf::AMFSurfacePtr surfVppOut) {
            int64_t pts = 0, duration = 0, inputFrameId = 0;
            surfVppOut->GetProperty(RGY_PROP_TIMESTAMP, &pts);
            surfVppOut->GetProperty(RGY_PROP_DURATION, &duration);
            surfVppOut->GetProperty(RGY_PROP_INPUT_FRAMEID, &inputFrameId);

            frc.addFrame(pts, duration);

            auto surfDecOut = std::make_unique<RGYFrameAMF>(surfVppOut);
            surfDecOut->clearDataList();
            surfDecOut->setTimestamp(pts);
            surfDecOut->setDuration(duration);
            surfDecOut->setInputFrameId((int)inputFrameId);
            if (auto it = m_metadatalist.find(pts); it != m_metadatalist.end()) {
                surfDecOut->setDataList(m_metadatalist[pts]);
            }
            m_outQeueue.push_back(std::make_unique<PipelineTaskOutputSurf>(m_workSurfs.addSurface(surfDecOut)));
        };

        auto enc_sts = RGY_ERR_NONE;
        auto ar = (drain) ? AMF_INPUT_FULL : AMF_OK;
        for (;;) {
            if (m_stopwatch) m_stopwatch->set(0);
            do {
                //VPPからの取り出し
                amf::AMFDataPtr data;
                const auto ar_out = m_vppFilter->filter()->QueryOutput(&data);
                if (ar_out == AMF_EOF) {
                    enc_sts = RGY_ERR_MORE_DATA;
                    break;
                } else if (ar_out == AMF_REPEAT) { //これ重要...ここが欠けると最後の数フレームが欠落する
                    if (data != nullptr) {
                        add_surf_to_out_queue(amf::AMFSurfacePtr(data));
                    }
                    break;
                } else if (ar_out != AMF_OK) {
                    enc_sts = err_to_rgy(ar_out);
                    break;
                } else if (data != nullptr) {
                    add_surf_to_out_queue(amf::AMFSurfacePtr(data));
                }
            } while (enc_sts == RGY_ERR_NONE);
            if (enc_sts != RGY_ERR_NONE) {
                break;
            }
            if (m_stopwatch) m_stopwatch->add(0, 2);

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
                if (m_stopwatch) m_stopwatch->add(0, 4);
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
                if (m_stopwatch) m_stopwatch->add(0, 3);
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
    virtual void setStopWatch() override {
        m_stopwatch = std::make_unique<PipelineTaskStopWatch>(
            std::vector<tstring>{ _T("depend_clear"), _T("Convert"), _T("filter"), _T("getOutFrame"), _T("setOutFrame") },
            std::vector<tstring>{_T("")}
        );
    }

    void setVideoQualityMetricFilter(RGYFilterSsim *videoMetric) {
        m_videoMetric = videoMetric;
    }

    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override { return std::nullopt; };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override {
        return std::make_pair(m_vpFilters.back()->GetFilterParam()->frameOut, m_outMaxQueueSize);
    };
    virtual RGY_ERR sendFrame(std::unique_ptr<PipelineTaskOutput>& frame) override {
        if (m_stopwatch) m_stopwatch->set(0);
        if (m_prevInputFrame.size() > 0) {
            //前回投入したフレームの処理が完了していることを確認したうえで参照を破棄することでロックを解放する
            auto prevframe = std::move(m_prevInputFrame.front());
            m_prevInputFrame.pop_front();
            prevframe->depend_clear();
        }
        if (m_stopwatch) m_stopwatch->add(0, 0);

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
        if (m_stopwatch) m_stopwatch->add(0, 1);
#define FRAME_COPY_ONLY 0
#if !FRAME_COPY_ONLY
        std::vector<std::unique_ptr<PipelineTaskOutputSurf>> outputSurfs;
        while (filterframes.size() > 0 || drain) {
            if (m_stopwatch) m_stopwatch->set(0);
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
            if (m_stopwatch) m_stopwatch->add(0, 2);
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
            if (m_stopwatch) m_stopwatch->add(0, 3);
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
            if (m_stopwatch) m_stopwatch->add(0, 4);
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

class PipelineTaskOutputRaw : public PipelineTask {
    RGYOutput *m_writer;
public:
    PipelineTaskOutputRaw(amf::AMFContextPtr context, RGYOutput *writer, int outMaxQueueSize, std::shared_ptr<RGYLog> log) :
        PipelineTask(PipelineTaskType::OUTPUTRAW, context, outMaxQueueSize, log), m_writer(writer) {};
    virtual ~PipelineTaskOutputRaw() {
        if (m_writer) {
            m_writer->WriteNextFrame((RGYFrame *)nullptr);
        }
    };

    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfIn() override { return std::nullopt; };
    virtual std::optional<std::pair<RGYFrameInfo, int>> requiredSurfOut() override { return std::nullopt; };

    virtual RGY_ERR sendFrame(std::unique_ptr<PipelineTaskOutput>& frame) override {
        if (!frame) {
            return RGY_ERR_MORE_DATA;
        }
        m_inFrames++;
        m_outQeueue.push_back(std::move(frame));
        return RGY_ERR_NONE;
    }
};

#endif //__VCE_PIPELINE_H__

