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
#ifndef _AVCODEC_VCE_H_
#define _AVCODEC_VCE_H_

#include "VCEVersion.h"

#if ENABLE_AVCODEC_VCE_READER
#include <algorithm>
#include <map>

#pragma warning (push)
#pragma warning (disable: 4244)
extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}
#pragma comment (lib, "avcodec.lib")
#pragma comment (lib, "avformat.lib")
#pragma comment (lib, "avutil.lib")
#pragma comment (lib, "swresample.lib")
#pragma comment (lib, "avfilter.lib")
#pragma warning (pop)

#include "VCELog.h"
#include "VCEUtil.h"
#include "VCEParam.h"
#include "VideoDecoderUVD.h"

#if _DEBUG
#define VCE_AV_LOG_LEVEL AV_LOG_WARNING
#else
#define VCE_AV_LOG_LEVEL AV_LOG_ERROR
#endif

typedef struct VCECodec {
    uint32_t codec_id;   //avcodecのコーデックID
    uint32_t vce_codec; //VCEのcodec
} VCECodec;

static const std::map<uint32_t, const WCHAR *> VCE_CODEC_UVD_NAME = {
    { VCE_CODEC_H264,  AMFVideoDecoderUVD_H264_AVC },
    { VCE_CODEC_HEVC,  AMFVideoDecoderHW_H265_HEVC },
    { VCE_CODEC_MPEG2, AMFVideoDecoderUVD_MPEG2    },
    { VCE_CODEC_VC1,   AMFVideoDecoderUVD_VC1      },
    //{ VCE_CODEC_WMV3,  AMFVideoDecoderUVD_WMV3     }
};

//QSVでデコード可能なコーデックのリスト
static const VCECodec VCE_DECODE_LIST[] = { 
    { AV_CODEC_ID_H264,       VCE_CODEC_H264       },
    { AV_CODEC_ID_HEVC,       VCE_CODEC_HEVC       },
    { AV_CODEC_ID_MPEG2VIDEO, VCE_CODEC_MPEG2      },
    { AV_CODEC_ID_VC1,        VCE_CODEC_VC1        },
    //{ AV_CODEC_ID_WMV3,       VCE_CODEC_WMV3       },
    //{ AV_CODEC_ID_VP8,           },
    //{ AV_CODEC_ID_VP9,           },
};
static const VCECodec VCE_ENCODE_LIST[] = {
    { AV_CODEC_ID_H264,       VCE_CODEC_H264 },
    { AV_CODEC_ID_HEVC,       VCE_CODEC_HEVC },
};

static const TCHAR *AVVCE_CODEC_AUTO = _T("auto");
static const TCHAR *AVVCE_CODEC_COPY = _T("copy");

static const int AVQSV_DEFAULT_AUDIO_BITRATE = 192;

static inline bool av_isvalid_q(AVRational q) {
    return q.den * q.num != 0;
}

static inline bool avcodecIsCopy(const TCHAR *codec) {
    return codec == nullptr || 0 == _tcsicmp(codec, AVVCE_CODEC_COPY);
}
static inline bool avcodecIsAuto(const TCHAR *codec) {
    return codec != nullptr && 0 == _tcsicmp(codec, AVVCE_CODEC_AUTO);
}

//AV_LOG_TRACE    56 - VCE_LOG_TRACE -3
//AV_LOG_DEBUG    48 - VCE_LOG_DEBUG -2
//AV_LOG_VERBOSE  40 - VCE_LOG_MORE  -1
//AV_LOG_INFO     32 - VCE_LOG_INFO   0
//AV_LOG_WARNING  24 - VCE_LOG_WARN   1
//AV_LOG_ERROR    16 - VCE_LOG_ERROR  2
static inline int log_level_av2vce(int level) {
    return clamp((AV_LOG_INFO / 8) - (level / 8), VCE_LOG_TRACE, VCE_LOG_ERROR);
}

static inline int log_level_vce2av(int level) {
    return clamp(AV_LOG_INFO - level * 8, AV_LOG_QUIET, AV_LOG_TRACE);
}

static tstring errorMesForCodec(const TCHAR *mes, AVCodecID targetCodec) {
    return mes + tstring(_T(" for ")) + char_to_tstring(avcodec_get_name(targetCodec)) + tstring(_T(".\n"));
};

static const AVRational VCE_NATIVE_TIMEBASE = { 1, VCE_TIMEBASE };
static const TCHAR *AVCODEC_DLL_NAME[] = {
    _T("avcodec-57.dll"), _T("avformat-57.dll"), _T("avutil-55.dll"), _T("avfilter-6.dll"), _T("swresample-2.dll")
};

enum AVQSVCodecType : uint32_t {
    AVQSV_CODEC_DEC = 0x01,
    AVQSV_CODEC_ENC = 0x02,
};

enum AVQSVFormatType : uint32_t {
    AVQSV_FORMAT_DEMUX = 0x01,
    AVQSV_FORMAT_MUX   = 0x02,
};

//AVFieldOrderを返す
AVFieldOrder vce_field_order(VCE_PICSTRUCT nInterlaced);

//avcodecのエラーを表示
tstring qsv_av_err2str(int ret);

//コーデックの種類を表示
tstring get_media_type_string(AVCodecID codecId);

//必要なavcodecのdllがそろっているかを確認
bool check_avcodec_dll();

//avcodecのdllが存在しない場合のエラーメッセージ
tstring error_mes_avcodec_dll_not_found();

//avcodecのライセンスがLGPLであるかどうかを確認
bool checkAvcodecLicense();

//avqsvでサポートされている動画コーデックを表示
tstring getAVVCESupportedCodecList();

//利用可能な音声エンコーダ/デコーダを表示
tstring getAVCodecs(AVQSVCodecType flag);

//利用可能なフォーマットを表示
tstring getAVFormats(AVQSVFormatType flag);

//利用可能なフィルターを表示
tstring getAVFilters();

//チャンネルレイアウトを表示
std::string getChannelLayoutChar(int channels, uint64_t channel_layout);
tstring getChannelLayoutString(int channels, uint64_t channel_layout);

//利用可能なプロトコル情報のリストを取得
vector<std::string> getAVProtocolList(int bOutput);

//利用可能なプロトコル情報を取得
tstring getAVProtocols();

//protocolを使用
bool usingAVProtocols(std::string filename, int bOutput);

//avformatのネットワークを初期化する
bool avformatNetworkInit();

//avformatのネットワークを閉じる
void avformatNetworkDeinit();

//バージョン情報の取得
tstring getAVVersions();

#endif //ENABLE_AVCODEC_VCE_READER

#endif //_AVCODEC_VCE_H_
