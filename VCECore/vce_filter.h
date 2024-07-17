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
#include "rgy_version.h"
#include "rgy_err.h"
#include "rgy_util.h"
#pragma warning(push)
#pragma warning(disable:4100)
RGY_DISABLE_WARNING_PUSH
RGY_DISABLE_WARNING_STR("-Wclass-memaccess")
#include "Context.h"
RGY_DISABLE_WARNING_POP
#pragma warning(pop)
#include "rgy_filter.h"
#include "vce_util.h"

namespace amf {
    class AMFFactory;
    class AMFTrace;
}

class AMFFilter {
public:
    AMFFilter(amf::AMFContextPtr context, std::shared_ptr<RGYLog>& log);
    virtual ~AMFFilter();

    virtual RGY_ERR init(amf::AMFFactory *factory, amf::AMFTrace *trace, std::shared_ptr<RGYFilterParam> param) = 0;
    virtual void close();

    amf::AMFComponentPtr filter() { return m_filter; }

    const RGYFilterParam *GetFilterParam() const { return m_param.get(); }
    tstring name() const { return m_name; }
    const tstring& GetInputMessage() const { return m_infoStr; }

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

        tstring mes = m_name + tstring(_T(": ")) + buffer.data();

        if (m_log.get() != nullptr) {
            m_log->write(log_level, RGY_LOGT_VPP, mes.c_str());
        } else {
            _ftprintf(stderr, _T("%s"), mes.c_str());
        }
    }
    virtual void setFrameParam(amf::AMFSurfacePtr pSurface) {
        return;
    }
protected:
    void setFilterInfo(const tstring &info) {
        m_infoStr = info;
        PrintMes(RGY_LOG_DEBUG, _T("%s"), info.c_str());
    }

    amf::AMFContextPtr m_context;
    amf::AMFComponentPtr m_filter;
    tstring m_name;
    tstring m_infoStr;
    std::shared_ptr<RGYFilterParam> m_param;
    std::shared_ptr<RGYLog> m_log;
};

class AMFFilterParamConverter : public RGYFilterParam {
public:
    int dummy_pad;
    AMFFilterParamConverter() : dummy_pad(0) {};
    virtual ~AMFFilterParamConverter() {};
    virtual tstring print() const override;
};

class AMFFilterConverter : public AMFFilter {
public:
    AMFFilterConverter(amf::AMFContextPtr context, std::shared_ptr<RGYLog>& log);
    virtual ~AMFFilterConverter();

    virtual RGY_ERR init(amf::AMFFactory *factory, amf::AMFTrace *trace, std::shared_ptr<RGYFilterParam> param) override;
    virtual void setFrameParam(amf::AMFSurfacePtr pSurface) override;
protected:
};

class AMFFilterParamHQScaler : public RGYFilterParam {
public:
    VppAMFHQScaler scaler;

    AMFFilterParamHQScaler() : scaler() {};
    virtual ~AMFFilterParamHQScaler() {};
    virtual tstring print() const override;
};

class AMFFilterHQScaler : public AMFFilter {
public:
    AMFFilterHQScaler(amf::AMFContextPtr context, std::shared_ptr<RGYLog>& log);
    virtual ~AMFFilterHQScaler();

    virtual RGY_ERR init(amf::AMFFactory *factory, amf::AMFTrace *trace, std::shared_ptr<RGYFilterParam> param) override;
protected:
};

class AMFFilterParamPreProcessing : public RGYFilterParam {
public:
    VppAMFPreProcessing pp;
    int bitrate;

    AMFFilterParamPreProcessing() : pp() {};
    virtual ~AMFFilterParamPreProcessing() {};
    virtual tstring print() const override;
};

class AMFFilterPreProcessing : public AMFFilter {
public:
    AMFFilterPreProcessing(amf::AMFContextPtr context, std::shared_ptr<RGYLog>& log);
    virtual ~AMFFilterPreProcessing();

    virtual RGY_ERR init(amf::AMFFactory *factory, amf::AMFTrace *trace, std::shared_ptr<RGYFilterParam> param) override;
protected:
};

class AMFFilterParamVQEnhancer : public RGYFilterParam {
public:
    VppAMFVQEnhancer enhancer;

    AMFFilterParamVQEnhancer() : enhancer() {};
    virtual ~AMFFilterParamVQEnhancer() {};
    virtual tstring print() const override;
};

class AMFFilterVQEnhancer : public AMFFilter {
public:
    AMFFilterVQEnhancer(amf::AMFContextPtr context, std::shared_ptr<RGYLog>& log);
    virtual ~AMFFilterVQEnhancer();

    virtual RGY_ERR init(amf::AMFFactory *factory, amf::AMFTrace *trace, std::shared_ptr<RGYFilterParam> param) override;
protected:
};

class AMFFilterParamFRC : public RGYFilterParam {
public:
    VppAMFFRC frc;

    AMFFilterParamFRC() : frc() {};
    virtual ~AMFFilterParamFRC() {};
    virtual tstring print() const override;
};

class AMFFilterFRC : public AMFFilter {
public:
    static const TCHAR *FRC_FILTER_NAME;
    AMFFilterFRC(amf::AMFContextPtr context, std::shared_ptr<RGYLog>& log);
    virtual ~AMFFilterFRC();

    virtual RGY_ERR init(amf::AMFFactory *factory, amf::AMFTrace *trace, std::shared_ptr<RGYFilterParam> param) override;
protected:
};