
# VCEEnc  
by rigaya

[![Build Windows Releases](https://github.com/rigaya/VCEEnc/actions/workflows/build_releases.yml/badge.svg)](https://github.com/rigaya/VCEEnc/actions/workflows/build_releases.yml)  [![Build Linux Packages](https://github.com/rigaya/VCEEnc/actions/workflows/build_packages.yml/badge.svg)](https://github.com/rigaya/VCEEnc/actions/workflows/build_packages.yml)  

**[日本語版はこちら＞＞](./Readme.ja.md)**  

This software is meant to investigate performance and image quality of HW encoder (VCE) of AMD.
There are 2 types of software developed, one is command line version that runs independently, and the nother is a output plug-in of [Aviutl](http://spring-fragrance.mints.ne.jp/aviutl/).

VCEEncC.exe … Command line version supporting transcoding.  
VCEEnc.auo … Output plugin for [Aviutl](http://spring-fragrance.mints.ne.jp/aviutl/).  

## Downloads & update history
[rigayaの日記兼メモ帳＞＞](http://rigaya34589.blog135.fc2.com/blog-category-12.html)  
[github releases](https://github.com/rigaya/VCEEnc/releases)  

## Install
[Install instructions for Windows and Linux](./Install.en.md).

## Build
[Build instructions for Windows and Linux](./Build.en.md)

## System Requirements

### Windows
Windows 10/11 (x86 / x64)  
Aviutl 1.00 or later (VCEEnc.auo)  
Hardware which supports VCE  
  AMD GPU Radeon HD 7xxx or later  
  AMD APU Trinity (2nd Gen) or later  

### Linux
Debian/Ubuntu (VCEEncC)  
  It may be possible to run on other distributions (not tested).

| VCEEnc | required graphics driver version |
|:---|:---|
| VCEEnc 3.00 or later | AMD driver 17.1.1 (16.50.2611) or later |
| VCEEnc 5.00 or later | AMD driver 19.7.1 or later |
| VCEEnc 5.01 or later | AMD driver 19.12.1 or later |
| VCEEnc 5.02 or later | AMD driver 20.2.1 or later |
| VCEEnc 6.09 or later | AMD driver 20.11.2 or later |
| VCEEnc 6.13 or later | AMD driver 21.6.1 or later |
| VCEEnc 6.17 or later | AMD driver 21.12.1 or later |

## Usage and options of VCEEncC
[Option list and details of VCEEncC](./VCEEncC_Options.en.md)

## Precautions for using VCEEnc
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.


## Main usable functions
### Common to VCEEnc / VCEEncC
- Encoding using VCE
   - H.264/AVC
   - HEVC (Polaris or later)
- Each encode mode of VCE
   - CQP       (fixed quantization)
   - CBR       (Constant bitrate)
   - VBR       (Variable bitrate)
- supports setting of codec profile & level, SAR, colormatrix, maxbitrate, GOP len, etc...


### VCEEncC
- Supports hw decoding
  - H.264 / AVC
  - HEVC
  - MPEG2
  - VP9
  - VC-1
- Supports various formats such as avs, vpy, y4m, and raw
- Supports demux/muxing using libavformat
- Supports decode using libavcodec
- Calculation of ssim/psnr of the encode
- High performance filtering (VPP, Video Pre-Processing)
  - GPU filtering by OpenCL
    - deinterlacer
      - afs (Automatic field shift)
      - nnedi
    - colorspace conversion
      - hdr2sdr
    - subburn
    - resize
      - spline16, spline36, spline64
      - lanczos2, lanczos3, lanczos4
    - transpose / rotate / flip
    - padding
    - deband
    - noise reduction
      - knn (K-nearest neighbor)
      - pmd (modified pmd method)
    - edge / detail enhancement
      - unsharp
      - edgelevel (edge ​​level adjustment)
      - warpsharp

### VCEEnc.auo (Aviutl plugin)
- Audio encoding
- Mux audio and chapter
- afs (Automatic field shift) support
  
## Auto GPU selection in multi GPU envinronment
VCEEncC will automatically select a GPU depending on the options used,
when there are multiple GPUs available which support VCE/VCN.
--device option can be used to specify on which GPU to run manually. 

1. Select GPU which supports...  
  Items below will be checked whether the GPU supports it or not  
  - Codec, Profile, Level
  - Additionally, below items will be checked if specified
    - 10bit depth encoding
    - HW decode
  
2. Prefer GPU which supports...  
  - B frame support
  
3. If there are multiple GPUs which suports all the items checked in 1. and 2., GPU below will be prefered.  
  - GPU which has low Video Engine(VE) utilization
  - GPU which has low GPU core utilization
  
  The purpose of selecting GPU with lower VE/GPU ulitization is to assign tasks to mulitple GPUs
  and improve the throughput of the tasks.  
  
  Please note that VE and GPU ulitization are check at the initialization pahse of the app,
  and there are delays in values taken. Therefore, it is likely that the multiple tasks started at the same time
  to run on the same GPU, and divided into multiple GPUs, even if the options are supported in every GPUs.

## VCEEnc source code
- MIT license.
- This program is based on samples of [AMD Media Framework](https://github.com/GPUOpen-LibrariesAndSDKs/AMF), and contains source code provided by
  AMD Media Framework.
  For these licenses, please see the header part of the corresponding source and VCEEnc_license.txt.
- This software depends on
  [ffmpeg](https://ffmpeg.org/),
  [tinyxml2](http://www.grinninglizard.com/tinyxml2/),
  [dtl](https://github.com/cubicdaiya/dtl),
  [clRNG](https://github.com/clMathLibraries/clRNG),
  [ttmath](http://www.ttmath.org/) &
  [Caption2Ass](https://github.com/maki-rxrz/Caption2Ass_PCR).
  For these licenses, please see the header part of the corresponding source and VCEEnc_license.txt.

### About source code
Windows ... VC build

Character code: UTF-8-BOM  
Line feed: CRLF  
Indent: blank x4  
