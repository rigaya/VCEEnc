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

#include <thread>
#include <future>
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
#include "rgy_device.h"
#include "vce_device.h"
#include "vce_param.h"
#include "vce_amf.h"
#include "vce_pipeline.h"
#include "rgy_filter.h"
#include "rgy_filter_ssim.h"
#include "rgy_device_usage.h"

#pragma warning(pop)

#if defined(_WIN32) || defined(_WIN64)
#define THREAD_DEC_USE_FUTURE 0
#else
// linuxではスレッド周りの使用の違いにより、従来の実装ではVCECore解放時に異常終了するので、
// std::futureを使った実装に切り替える
// std::threadだとtry joinのようなことができないのが問題
#define THREAD_DEC_USE_FUTURE 1
#endif

#if ENABLE_AVSW_READER
struct AVChapter;
#endif //#if ENABLE_AVSW_READER
class RGYTimecode;

class RGYPipelineFrame {
    RGYPipelineFrame(amf::AMFSurfacePtr surf) : surface(surf), frame() {};
    RGYPipelineFrame(shared_ptr<RGYCLFrame> clframe) : surface(), frame(clframe) {};
    ~RGYPipelineFrame() {
        surface->Release();
        frame.reset();
    };
protected:
    amf::AMFSurfacePtr surface;
    shared_ptr<RGYCLFrame> frame;
};

class VCECore : public VCEAMF {
public:
    VCECore();
    virtual ~VCECore();

    virtual RGY_ERR init(VCEParam *prm);
    virtual RGY_ERR initLog(VCEParam *prm);
    virtual RGY_ERR initDevice(std::vector<std::unique_ptr<VCEDevice>> &gpuList, int deviceId, const RGYDeviceUsageLockManager *devUsageLock);
    virtual RGY_ERR initInput(VCEParam *pParams, DeviceCodecCsp& HWDecCodecCsp);
    virtual RGY_ERR initOutput(VCEParam *prm);
    virtual RGY_ERR run2();
    virtual void Terminate() override;

    DeviceCodecCsp getHWDecCodecCsp(bool skipHWDecodeCheck, std::vector<std::unique_ptr<VCEDevice>>& devList);
    tstring GetEncoderParam();
    void PrintEncoderParam();
    void PrintResult();

    VCEDevice *dev() { return m_dev.get(); };

    void SetAbortFlagPointer(bool *abortFlag);
protected:
    virtual RGY_ERR readChapterFile(tstring chapfile);
    std::pair<RGY_ERR, VideoInfo> GetOutputVideoInfo();
    RGY_ERR checkGPUListByEncoder(std::vector<std::unique_ptr<VCEDevice>> &gpuList, VCEParam *prm, int deviceId);
    RGY_ERR gpuAutoSelect(std::vector<std::unique_ptr<VCEDevice>> &gpuList, const VCEParam *prm, const RGYDeviceUsageLockManager *devUsageLock);
    virtual RGY_CSP GetEncoderCSP(const VCEParam *inputParam) const;
    virtual int GetEncoderBitdepth(const VCEParam *inputParam) const;
    virtual RGY_ERR checkParam(VCEParam *prm);
    virtual RGY_ERR initPerfMonitor(VCEParam *prm);
    virtual RGY_ERR InitParallelEncode(VCEParam *inputParam, const int maxEncoders);
    virtual RGY_ERR createDecoder(VCEParam *prm, amf::AMFComponentPtr& decoder);
    virtual RGY_ERR tryDecode(amf::AMFComponentPtr& decoder);
    virtual RGY_ERR initDecoder(VCEParam *prm);
    virtual RGY_ERR initFilters(VCEParam *prm);
    virtual std::vector<VppType> InitFiltersCreateVppList(const VCEParam *inputParam,
        const bool cspConvRequired, const bool cropRequired, const RGY_VPP_RESIZE_TYPE resizeRequired);
    virtual RGY_ERR AddFilterOpenCL(std::vector<std::unique_ptr<RGYFilter>>&clfilters,
        RGYFrameInfo & inputFrame, const VppType vppType, const VCEParam *prm, const sInputCrop * crop, const std::pair<int, int> resize, VideoVUIInfo& vuiInfo);
    virtual std::tuple<RGY_ERR, std::unique_ptr<AMFFilter>> AddFilterAMF(
        RGYFrameInfo & inputFrame, const VppType vppType, const VCEParam *prm, const sInputCrop * crop, const std::pair<int, int> resize, VideoVUIInfo& vuiInfo);
    virtual RGY_ERR createOpenCLCopyFilterForPreVideoMetric(const VCEParam *inputParam);
    virtual RGY_ERR initChapters(VCEParam *prm);
    virtual RGY_ERR initEncoder(VCEParam *prm);
    virtual RGY_ERR initPowerThrottoling(VCEParam *prm);
    virtual RGY_ERR initSSIMCalc(VCEParam *prm);
    virtual RGY_ERR initPipeline(VCEParam *prm);

    bool VppAfsRffAware() const;
    virtual RGY_ERR allocatePiplelineFrames();

    virtual RGY_ERR run_decode();
    virtual RGY_ERR run_output();

    RGY_CODEC          m_encCodec;
    RGY_CSP            m_encCSP;
    bool m_bTimerPeriodTuning;
#if ENABLE_AVSW_READER
    bool                          m_keyOnChapter;        //チャプター上にキーフレームを配置する
    vector<int>                   m_keyFile;             //キーフレームの指定
    vector<unique_ptr<AVChapter>> m_Chapters;            //ファイルから読み込んだチャプター
#endif //#if ENABLE_AVSW_READER
    std::unique_ptr<RGYTimecode>     m_timecode;
    std::unique_ptr<RGYHDR10Plus>    m_hdr10plus;
    bool                             m_hdr10plusMetadataCopy;
    std::unique_ptr<RGYHDRMetadata>  m_hdrseiIn;
    std::unique_ptr<RGYHDRMetadata>  m_hdrseiOut;
    std::unique_ptr<DOVIRpu>         m_dovirpu;
    bool                             m_dovirpuMetadataCopy;
    RGYDOVIProfile                   m_doviProfile;
    std::unique_ptr<RGYTimestamp>    m_encTimestamp;

    sTrimParam m_trimParam;
    std::unique_ptr<RGYPoolAVPacket> m_poolPkt;
    std::unique_ptr<RGYPoolAVFrame> m_poolFrame;
    shared_ptr<RGYInput> m_pFileReader;
    vector<shared_ptr<RGYInput>> m_AudioReaders;
    shared_ptr<RGYOutput> m_pFileWriter;
    vector<shared_ptr<RGYOutput>> m_pFileWriterListAudio;
    shared_ptr<EncodeStatus> m_pStatus;
    shared_ptr<CPerfMonitor> m_pPerfMonitor;

    int                m_pipelineDepth;
    int                m_nProcSpeedLimit;       //処理速度制限 (0で制限なし)
    bool               m_taskPerfMonitor;       //タスクのパフォーマンス監視を有効にする
    RGYAVSync          m_nAVSyncMode;           //映像音声同期設定
    bool               m_timestampPassThrough;  //タイムスタンプをそのまま通す
    rgy_rational<int>  m_inputFps;              //入力フレームレート
    rgy_rational<int>  m_encFps;             //出力フレームレート
    rgy_rational<int>  m_outputTimebase;        //出力のtimebase
    int                m_encWidth;
    int                m_encHeight;
    rgy_rational<int>  m_sar;
    RGY_PICSTRUCT      m_picStruct;
    VideoVUIInfo       m_encVUI;
    bool               m_repeatHeaders;

    std::vector<tstring> m_devNames;
    std::unique_ptr<VCEDevice> m_dev;
    std::unique_ptr<RGYDeviceUsage> m_deviceUsage;
    std::unique_ptr<RGYParallelEnc> m_parallelEnc;

    vector<VppVilterBlock>        m_vpFilters;
    shared_ptr<RGYFilterParam>    m_pLastFilterParam;
    unique_ptr<RGYFilterSsim>     m_videoQualityMetric;

    RGYRunState m_state;

    sTrimParam *m_pTrimParam;

    amf::AMFComponentPtr m_pDecoder;
    amf::AMFComponentPtr m_pEncoder;
#if THREAD_DEC_USE_FUTURE
    std::future<RGY_ERR> m_thDecoder;
#else
    std::thread m_thDecoder;
#endif
    std::future<RGY_ERR> m_thOutput;

    AMFParams m_params;

    std::vector<std::unique_ptr<PipelineTask>> m_pipelineTasks;

    bool *m_pAbortByUser;
};

class VCEFeatures {
public:
    VCEFeatures() : m_core() {};

    virtual ~VCEFeatures() {};

    RGY_ERR init(int deviceId, const RGYParamLogLevel& loglevel);
    tstring devName() const { return m_core->dev()->getGPUInfo(); }
    tstring checkEncFeatures(RGY_CODEC codec);
    tstring checkDecFeatures(RGY_CODEC codec);
    tstring checkFilterFeatures(const std::wstring& filter);
protected:
    std::unique_ptr<VCECore> m_core;
};