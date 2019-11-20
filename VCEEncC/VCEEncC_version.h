//  -----------------------------------------------------------------------------------------
//    NVEnc by rigaya
//  -----------------------------------------------------------------------------------------
//   ソースコードについて
//   ・無保証です。
//   ・本ソースコードを使用したことによるいかなる損害・トラブルについてrigayaは責任を負いません。
//   以上に了解して頂ける場合、本ソースコードの使用、複製、改変、再頒布を行って頂いて構いません。
//  -----------------------------------------------------------------------------------------

#ifndef _VCEENCC_VERSION_H_
#define _VCEENCC_VERSION_H_

#include "rgy_version.h"

#ifdef DEBUG
#define VER_DEBUG   VS_FF_DEBUG
#define VER_PRIVATE VS_FF_PRIVATEBUILD
#else
#define VER_DEBUG   0
#define VER_PRIVATE 0
#endif

#ifdef _M_IX86
#define VCEENC_FILENAME "VCEEncC (x86) - VCEEnc Encoder (CUI)"
#else
#define VCEENC_FILENAME "VCEEncC (x64) - VCEEnc Encoder (CUI)"
#endif

#define VER_STR_COMMENTS         "based on AMF SDK"
#define VER_STR_COMPANYNAME      ""
#define VER_STR_FILEDESCRIPTION  VCEENC_FILENAME
#define VER_STR_INTERNALNAME     VCEENC_FILENAME
#define VER_STR_ORIGINALFILENAME "VCEEncC.exe"
#define VER_STR_LEGALCOPYRIGHT   "VCEEncC by rigaya"
#define VER_STR_PRODUCTNAME      VCEENC_FILENAME
#define VER_PRODUCTVERSION       VER_FILEVERSION
#define VER_STR_PRODUCTVERSION   VER_STR_FILEVERSION

#endif //_VCEENCC_VERSION_H_
