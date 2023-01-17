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

#include "vce_param.h"
#include "vce_filter.h"
#pragma warning(push)
#pragma warning(disable:4100)
RGY_DISABLE_WARNING_PUSH
RGY_DISABLE_WARNING_STR("-Wclass-memaccess")
#include "Factory.h"
#include "Trace.h"
RGY_DISABLE_WARNING_POP
#pragma warning(pop)

AMFFilter::AMFFilter(amf::AMFContextPtr context, std::shared_ptr<RGYLog>& log) :
    m_context(context),
    m_filter(),
    m_name(),
    m_infoStr(),
    m_param(),
    m_log(log) {

}

AMFFilter::~AMFFilter() {
    close();
}

void AMFFilter::close() {
    if (m_filter) {
        PrintMes(RGY_LOG_DEBUG, _T("Closing %s...\n"), m_name.c_str());
        m_filter->Terminate();
        m_filter = nullptr;
        PrintMes(RGY_LOG_DEBUG, _T("Closed %s.\n"), m_name.c_str());
    }
    m_param.reset();
    m_log.reset();
}

tstring AMFFilterParamConverter::print() const {
    return _T("");
}

AMFFilterConverter::AMFFilterConverter(amf::AMFContextPtr context, shared_ptr<RGYLog>& log) :
    AMFFilter(context, log) {
    m_name = _T("converter");
}

AMFFilterConverter::~AMFFilterConverter() {};

RGY_ERR AMFFilterConverter::init(amf::AMFFactory *factory, amf::AMFTrace *trace, std::shared_ptr<RGYFilterParam> param) {
    auto prm = std::dynamic_pointer_cast<AMFFilterParamConverter>(param);
    if (!prm) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid parameter type.\n"));
        return RGY_ERR_INVALID_PARAM;
    }
    auto res = factory->CreateComponent(m_context, AMFVideoConverter, &m_filter);
    if (res != AMF_OK) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to create %s: %s\n"), m_name.c_str(), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("created %s.\n"), m_name.c_str());
    const auto formatOut = csp_rgy_to_enc(prm->frameOut.csp);
    res = m_filter->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_FORMAT, formatOut);
    //res = m_filter->SetProperty(AMF_VIDEO_CONVERTER_COMPUTE_DEVICE, engine_type);
    //res = m_filter->SetProperty(AMF_VIDEO_CONVERTER_MEMORY_TYPE, engine_type);
    res = m_filter->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_SIZE, AMFConstructSize(prm->frameOut.width, prm->frameOut.height));
    if (prm->dummy_pad < 0) { // dummyで付加したpaddingを取り除く
        m_name += _T(" d-");
    } else if (prm->dummy_pad > 0) { // dummyでpaddingを付加
        m_name += _T(" d+");
        res = m_filter->SetProperty(AMF_VIDEO_CONVERTER_OUTPUT_RECT, AMFConstructRect(0, 0, prm->frameIn.width, prm->frameIn.height));
        //res = m_filter->SetProperty(AMF_VIDEO_CONVERTER_KEEP_ASPECT_RATIO, true);
        //res = m_filter->SetProperty(AMF_VIDEO_CONVERTER_FILL, true);
        //res = m_filter->SetProperty(AMF_VIDEO_CONVERTER_FILL_COLOR, AMFConstructColor(255, 255, 255, 255));
    }
    //res = m_filter->SetProperty(AMF_VIDEO_CONVERTER_SCALE, AMF_VIDEO_CONVERTER_SCALE_BICUBIC);
    PrintMes(RGY_LOG_DEBUG, _T("initialize %s by format out %s, size %dx%d.\n"), m_name.c_str(),
        wstring_to_tstring(trace->SurfaceGetFormatName(formatOut)).c_str(),
        std::min(prm->frameIn.width, prm->frameOut.width), std::min(prm->frameIn.height, prm->frameOut.height));
    if (AMF_OK != (res = m_filter->Init(csp_rgy_to_enc(prm->frameIn.csp), prm->frameIn.width, prm->frameIn.height))) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to init %s with %s %dx%d: %s\n"),
            m_name.c_str(),
            wstring_to_tstring(trace->SurfaceGetFormatName(formatOut)).c_str(),
            prm->frameIn.width, prm->frameIn.height,
            AMFRetString(res));
        return err_to_rgy(res);
    }
    m_param = prm;
    PrintMes(RGY_LOG_DEBUG, _T("initialized %s.\n"), m_name.c_str());
    setFilterInfo(strsprintf(_T("%s: %s %dx%d\n"), m_name.c_str(),
        wstring_to_tstring(trace->SurfaceGetFormatName(formatOut)).c_str(),
        std::min(prm->frameIn.width, prm->frameOut.width), std::min(prm->frameIn.height, prm->frameOut.height)));
    return RGY_ERR_NONE;
}

void AMFFilterConverter::setFrameParam(amf::AMFSurfacePtr pSurface) {
    auto prm = std::dynamic_pointer_cast<AMFFilterParamConverter>(m_param);
    if (prm) {
        if (prm->dummy_pad < 0) {
            // dummyのpaddingはフレームの下に入れているので、フレームの上側を有効範囲として指定する
            const int crop = -prm->dummy_pad;
            pSurface->SetCrop(0, 0, prm->frameIn.width, prm->frameIn.height - crop);
        }
    }
}

tstring AMFFilterParamHQScaler::print() const {
    return scaler.print();
}

AMFFilterHQScaler::AMFFilterHQScaler(amf::AMFContextPtr context, shared_ptr<RGYLog>& log) :
    AMFFilter(context, log) {
    m_name = _T("HQscaler");
}

AMFFilterHQScaler::~AMFFilterHQScaler() {};

RGY_ERR AMFFilterHQScaler::init(amf::AMFFactory *factory, amf::AMFTrace *trace, std::shared_ptr<RGYFilterParam> param) {
    auto prm = std::dynamic_pointer_cast<AMFFilterParamHQScaler>(param);
    if (!prm) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid parameter type.\n"));
        return RGY_ERR_INVALID_PARAM;
    }
    auto res = factory->CreateComponent(m_context, AMFHQScaler, &m_filter);
    if (res != AMF_OK) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to create %s: %s\n"), m_name.c_str(), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("created %s.\n"), m_name.c_str());
#if defined(_WIN32) || defined(_WIN64)
    const auto engine_type = amf::AMF_MEMORY_DX11;
#else
    const auto engine_type = amf::AMF_MEMORY_OPENCL;
#endif
    const auto formatOut = csp_rgy_to_enc(prm->frameIn.csp);
    res = m_filter->SetProperty(AMF_HQ_SCALER_ENGINE_TYPE, engine_type);
    res = m_filter->SetProperty(AMF_HQ_SCALER_OUTPUT_SIZE, AMFConstructSize(prm->frameOut.width, prm->frameOut.height));
    res = m_filter->SetProperty(AMF_HQ_SCALER_ALGORITHM, prm->scaler.algorithm);
    res = m_filter->SetProperty(AMF_HQ_SCALER_SHARPNESS, prm->scaler.sharpness);
    PrintMes(RGY_LOG_DEBUG, _T("initialize %s by engine type %s, format out %s, size %dx%d -> %dx%d.\n"), m_name.c_str(),
        wstring_to_tstring(trace->GetMemoryTypeName(engine_type)).c_str(),
        wstring_to_tstring(trace->SurfaceGetFormatName(formatOut)).c_str(),
        prm->frameIn.width, prm->frameIn.height, prm->frameOut.width, prm->frameOut.height);
    if (AMF_OK != (res = m_filter->Init(formatOut, prm->frameIn.width, prm->frameIn.height))) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to init %s: %s\n"), m_name.c_str(), AMFRetString(res));
        return err_to_rgy(res);
    }
    m_param = prm;
    PrintMes(RGY_LOG_DEBUG, _T("initialized %s.\n"), m_name.c_str());
    setFilterInfo(strsprintf(_T("scaler: %dx%d -> %dx%d, %s, sharpness %.1f\n"),
        prm->frameIn.width, prm->frameIn.height, prm->frameOut.width, prm->frameOut.height,
        get_cx_desc(list_vce_hq_scaler, prm->scaler.algorithm), prm->scaler.sharpness));
    m_param = prm;
    return RGY_ERR_NONE;
}

tstring AMFFilterParamPreProcessing::print() const {
    return pp.print();
}

AMFFilterPreProcessing::AMFFilterPreProcessing(amf::AMFContextPtr context, shared_ptr<RGYLog>& log) :
    AMFFilter(context, log) {
    m_name = _T("pp");
}
AMFFilterPreProcessing::~AMFFilterPreProcessing() {};

RGY_ERR AMFFilterPreProcessing::init(amf::AMFFactory *factory, amf::AMFTrace *trace, std::shared_ptr<RGYFilterParam> param) {
    auto prm = std::dynamic_pointer_cast<AMFFilterParamPreProcessing>(param);
    if (!prm) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid parameter type.\n"));
        return RGY_ERR_INVALID_PARAM;
    }
    auto res = factory->CreateComponent(m_context, AMFPreProcessing, &m_filter);
    if (res != AMF_OK) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to create %s: %s\n"), m_name.c_str(), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("created %s.\n"), m_name.c_str());

    res = m_filter->SetProperty(AMF_PP_ENGINE_TYPE, amf::AMF_MEMORY_OPENCL);
    res = m_filter->SetProperty(AMF_PP_OUTPUT_MEMORY_TYPE, amf::AMF_MEMORY_OPENCL);
    res = m_filter->SetProperty(AMF_PP_ADAPTIVE_FILTER_STRENGTH, prm->pp.strength);
    res = m_filter->SetProperty(AMF_PP_ADAPTIVE_FILTER_SENSITIVITY, prm->pp.sensitivity);
    if (prm->pp.adaptiveFilter) {
        res = m_filter->SetProperty(AMF_PP_TARGET_BITRATE, prm->bitrate * 1000);
        res = m_filter->SetProperty(AMF_PP_FRAME_RATE, AMFConstructRate(prm->baseFps.n(), prm->baseFps.d()));
        res = m_filter->SetProperty(AMF_PP_ADAPTIVE_FILTER_ENABLE, prm->pp.adaptiveFilter);
    }
    const auto formatOut = csp_rgy_to_enc(prm->frameIn.csp);
    PrintMes(RGY_LOG_DEBUG, _T("initialize %s by mem type %s, format out %s, output size %dx%x.\n"), m_name.c_str(),
        wstring_to_tstring(trace->GetMemoryTypeName(amf::AMF_MEMORY_OPENCL)).c_str(),
        wstring_to_tstring(trace->SurfaceGetFormatName(formatOut)).c_str(),
        prm->frameIn.width, prm->frameIn.height);
    if (AMF_OK != (res = m_filter->Init(formatOut, prm->frameIn.width, prm->frameIn.height))) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to init %s: %s\n"), m_name.c_str(), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("initialized %s.\n"), m_name.c_str());
    setFilterInfo(prm->print());
    m_param = prm;
    return RGY_ERR_NONE;
}

tstring AMFFilterParamVQEnhancer::print() const {
    return enhancer.print();
}

AMFFilterVQEnhancer::AMFFilterVQEnhancer(amf::AMFContextPtr context, shared_ptr<RGYLog>& log) :
    AMFFilter(context, log) {
    m_name = _T("enhance");
}
AMFFilterVQEnhancer::~AMFFilterVQEnhancer() {};

RGY_ERR AMFFilterVQEnhancer::init(amf::AMFFactory *factory, amf::AMFTrace *trace, std::shared_ptr<RGYFilterParam> param) {
    auto prm = std::dynamic_pointer_cast<AMFFilterParamVQEnhancer>(param);
    if (!prm) {
        PrintMes(RGY_LOG_ERROR, _T("Invalid parameter type.\n"));
        return RGY_ERR_INVALID_PARAM;
    }
    auto res = factory->CreateComponent(m_context, AMFVQEnhancer, &m_filter);
    if (res != AMF_OK) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to create %s: %s\n"), m_name.c_str(), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("created %s.\n"), m_name.c_str());

#if defined(_WIN32) || defined(_WIN64)
    const auto engine_type = amf::AMF_MEMORY_DX11;
#else
    const auto engine_type = amf::AMF_MEMORY_OPENCL;
#endif
    const auto formatOut = csp_rgy_to_enc(prm->frameIn.csp);
    res = m_filter->SetProperty(AMF_VIDEO_ENHANCER_ENGINE_TYPE, engine_type);
    res = m_filter->SetProperty(AMF_VIDEO_ENHANCER_OUTPUT_SIZE, AMFConstructSize(prm->frameIn.width, prm->frameIn.height));
    res = m_filter->SetProperty(AMF_VE_FCR_ATTENUATION, prm->enhancer.attenuation);
    res = m_filter->SetProperty(AMF_VE_FCR_RADIUS, prm->enhancer.fcrRadius);
    PrintMes(RGY_LOG_DEBUG, _T("initialize %s by engine type %s, format out %s, output size %dx%x.\n"), m_name.c_str(),
        wstring_to_tstring(trace->GetMemoryTypeName(engine_type)).c_str(),
        wstring_to_tstring(trace->SurfaceGetFormatName(formatOut)).c_str(),
        prm->frameIn.width, prm->frameIn.height);
    if (AMF_OK != (res = m_filter->Init(formatOut, prm->frameIn.width, prm->frameIn.height))) {
        PrintMes(RGY_LOG_ERROR, _T("Failed to init %s: %s\n"), m_name.c_str(), AMFRetString(res));
        return err_to_rgy(res);
    }
    PrintMes(RGY_LOG_DEBUG, _T("initialized %s.\n"), m_name.c_str());
    setFilterInfo(prm->print());
    m_param = param;
    return RGY_ERR_NONE;
}
