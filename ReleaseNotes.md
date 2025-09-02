# VCEEnc Release Notes

## 9.00

- Fix -u slower not working and set to -u slow for H.264/HEVC encoding. ( #134 )
- Add [--aud](./VCEEncC_Options.en.md#--aud), [--repeat-headers](./VCEEncC_Options.en.md#--repeat-headers). ( #133 )
- Add feature to use filters with avcodec encoders.
  - Available with ```-c av_xxx```
    Example: [-c](./VCEEncC_Options.en.md#-c---codec-string) av_libsvtav1 [--avcodec-prms](./VCEEncC_Options.en.md#--avcodec-prms-string) "preset=6,crf=30,svtav1-params=enable-variance-boost=1:variance-boost-strength=2"
    Other usable options include av_libvvenc, av_libvpx-vp9, etc.
- Add QSVEnc.auo2 with official support for AviUtl2.
- Improve --vpp-subburn quality for moving subtitles. 
- Update ffmpeg libraries. (Windows)
  - ffmpeg 7.1+ (20240822) -> 8.0
  - libpng 1.6.44 -> 1.6.50
  - expat 2.6.2 -> 2.7.1
  - fribidi 1.0.11 -> 1.0.16
  - libogg 1.3.5 -> 1.3.6
  - libxml2 2.12.6 -> 2.14.5
  - libvpl 2.13.0 -> 2.15.0
  - libvpx 1.14.1 -> 1.15.2
  - dav1d 1.4.3 -> 1.5.1
  - libxxhash 0.8.2 -> 0.8.3
  - glslang 15.0.0 -> 15.4.0
  - dovi_tool 2.1.2 -> 2.3.1
  - libjpeg-turbo 2.1.0 -> 3.1.1
  - lcms2 2.16 -> 2.17
  - zimg 3.0.5 -> 3.0.6
  - libplacebo 7.349.0 -> 7.351.0
  - libsvtav1 3.1.0 (new!) x64 only
  - libvvenc 1.13.1 (new!) x64 only
  - libass 0.9.0 -> 0.17.4 (x64), 0.14.0 (x86)
  - harfbuzz 11.4.4 (new)
  - libunibreak 6.1 (new)
  - Remove mmt/tlv patch

## 8.38

- Fix 10-bit processing in [--vpp-afs](./VCEEncC_Options.en.md#--vpp-afs-param1value1param2value2).
- Improve precision of [--vpp-afs](./VCEEncC_Options.en.md#--vpp-afs-param1value1param2value2).
- Updates for VCEEnc.auo (AviUtl/AviUtl2 plugin).

## 8.37

- Add raw output. (-c raw)
- Switch to sw decode when hw decode fails to start. ( #101 )

## 8.36

- Fix black/white processing in 10-bit depth for [--vpp-edgelevel](./VCEEncC_Options.en.md#--vpp-edgelevel-param1value1param2value2).
- Improve interlace detection when using [--avsw](./VCEEncC_Options.en.md#--avsw-string).
- Fixed an issue with [--vpp-decimate](./VCEEncC_Options.en.md#--vpp-decimate-param1value1param2value2) where timestamp and duration of frames became incorrect due to improper handling of the final frame's timing.
- Use thread pool to prevent unlimited OpenCL build threads.
- Improve stability of [--parallel](./VCEEncC_Options.en.md#--parallel-int-or-param1value1param2value2). ( #248 )

## 8.35

- Improve audio and video synchronization to achieve more uniform mixing when muxing with subtitles or data tracks.
- Improve invalid input data hadling to avoid freeze when "failed to run h264_mp4toannexb bitstream filter" error occurs.
  Now properly exits with error.
- Add support for uyvy as input color format.
- Improve B-frame settings in AV1 encoding. ( #120 )
- Automatically disable --parallel when number of encoders is 1 when using ```--parallel auto```.

## 8.34

- Add support for SAR (Sample Aspect Ratio) in AV1 encoding (on container side).
- Fix B-frame QP value in AV1 encoding.

## 8.33

- Update AMF to 1.4.36.
  Requires AMD Radeon Software Adrenalin Edition 25.1.1 or later.
- Add ```slower``` option to ([--preset](./VCEEncC_Options.en.md#-u---preset)) for H.264/HEVC.
- Add parallel encoding feature with file splitting. ([--parallel](./VCEEncC_Options.en.md#--parallel-int-or-param1value1param2value2), Windows only)
- Add support for ISO 639-2 T-codes in language code specification.
- Continue processing even when DirectX11/Vulkan initialization fails.
- Fix timestamps occasionally becoming incorrect when using [--seek](./VCEEncC_Options.en.md#--seek-intintintint) with certain input files.
- Fix --qvbr not reflected properly. ( #115 )
- Fix error when AMD GPU is not recognized as first GPU in Vulkan.

## 8.32

- Fix [--dolby-vision-rpu](./VCEEncC_Options.en.md#--dolby-vision-rpu-string) in AV1 encoding.

## 8.31

- Fix some codecs not being able to decode with [--avsw](./VCEEncC_Options.en.md#--avsw) since version 8.29.
- Add options 10.0, 10.1, 10.2, 10.4 to [--dolby-vision-profile](./VCEEncC_Options.en.md#--dolby-vision-profile-string-hevc-av1).
- Fix [--dolby-vision-profile](./VCEEncC_Options.en.md#--dolby-vision-profile-string-hevc-av1) not working with inputs other than avhw/avsw.

## 8.30

- Fix --avsw not working in VCEEnc 8.29.

## 8.29

- Fix colormatrix/colorprim/transfer/SAR not written properly when writing into container format.

## 8.28

- Fix some case that audio not being able to play when writing to mkv using --audio-copy.
- Fix colormatrix/colorprim/transfer not set properly in some cases. ( #114 )
- Add check for --level and --max-bitrate.

## 8.27

- Avoid width field in mp4 Track Header Box getting 0 when SAR is undefined.

## 8.26

- Fix some of the parameters of [--vpp-libplacebo-tonemapping](./VCEEncC_Options.en.md#--vpp-libplacebo-tonemapping-param1value1param2value2) not working properly.
- Fix [--trim](./VCEEncC_Options.en.md#--trim-intintintintintint) being offset for a few frames when input file is a "cut" file (which does not start from key frame) and is coded using OpenGOP.

## 8.25

- Fix --dolby-vision-rpu from file (not copy) corrupted from VCEEnc 8.24.

## 8.24

- Update AMF to 1.4.35.
  Requires AMD Radeon Software Adrenalin Edition 24.9.1 or later.
- Add support for AV1 [--bframes](./VCEEncC_Options.en.md#-b---bframes-int) and [--adapt-minigop](./VCEEncC_Options.en.md#--adapt-minigop-h264av1) for upcoming VCN5 hardware.
- Update ffmpeg libraries. (Windows)
  - ffmpeg 7.0 -> 20240822
  - dav1d 1.4.1 -> 1.4.3
  - libvpl 2.11.0 -> 2.12.0
  - libvpx 2.14.0
- Improve auto GPU device selection on multi GPU environments.
  Now will be able to select unutilized GPU more correctly.
- Now [--temporal-layers](./VCEEncC_Options.en.md#--temporal-layers-int-hevcav1) can also be used for HEVC encoding. ( #112 )
- Add Linux support for auto GPU device selection on multi GPU environments.
- Add feature to copy Dolby Vision profile from input file. ([--dolby-vision-profile](./VCEEncC_Options.en.md#--dolby-vision-profile-string-hevc-av1) copy)
  Supported on Windows and Ubuntu 24.04.
- Add feature to copy Dolby Vision rpu metadata from input HEVC file. ([--dolby-vision-rpu](./VCEEncC_Options.en.md#--dolby-vision-rpu-string-hevc-av1) copy)
- Now [--dolby-vision-rpu](./VCEEncC_Options.en.md#--dolby-vision-rpu-string-hevc-av1) and [--dhdr10-info](./VCEEncC_Options.en.md#--dhdr10-info-string-hevc-av1) can be used together.
- Add option to set active area offsets to 0 for dolby vision rpu metadata.
- Fix invalid value not returning error when using [--dolby-vision-profile](./VCEEncC_Options.en.md#--dolby-vision-profile-string-hevc-av1) .
- Now [--dolby-vision-rpu](./VCEEncC_Options.en.md#--dolby-vision-rpu-string-hevc-av1) copy will automatically convert to dolby vision profile 8 when input files is dolby vision profile 7 using libdovi.
- Improve AV1 output when using [--dhdr10-info](./VCEEncC_Options.en.md#--dhdr10-info-string-hevc-av1).
- Changed implementation of [--dhdr10-info](./VCEEncC_Options.en.md#--dhdr10-info-string-hevc-av1) to work on Linux.
  Changed to use libhdr10plus instead of hdr10plus_gen.exe.
- Add libplacebo resize filters for Windows build (--vpp-resize).
- Add custom shader filter using libplacebo. ([--vpp-libplaceo-shader](./VCEEncC_Options.en.md#--vpp-libplacebo-shader-param1value1param2value2))
- Add deband filter by libplacebo. ([--vpp-libplacebo-deband](./VCEEncC_Options.en.md#--vpp-libplacebo-deband-param1value1param2value2))
- Add tone mapping filter by libplacebo. ([--vpp-libplacebo-tonemapping](./VCEEncC_Options.en.md#--vpp-libplacebo-tonemapping-param1value1param2value2))
- Avoid "failed to get header." error on some HEVC input files.
  When H.264/HEVC header cannot be extracted, it can be now retrieved from the actual data packets.
- Fix help of [--vpp-smooth](./VCEEncC_Options.en.md#--vpp-smooth-param1value1param2value2) showing wrong value range for qp option.
- Fix [--vpp-transform](./VCEEncC_Options.en.md#--vpp-transform-param1value1param2value2) causing illegal memory access error when width or height cannot be divided by 64.
- Fix timestamp error caused when using [--vpp-afs](./VCEEncC_Options.en.md#--vpp-afs-param1value1param2value2) with VFR inputs. 
- Fix seek issue caused by audio getting muxed to a different fragment than the video at the same time, due to insufficient buffer for audio mux.
- Fix muxer error copying PGS subtitles (using --sub-copy) when the input has unsorted subtitle packets.
  Now is able to avoid "Application provided invalid, non monotonically increasing dts to muxer" error.
- Fix skipframe log being shown always on. ( #112 )
- Fix crush when aborting with Ctrl+C. ( #111 )
- Change log behavior.

## 8.23

- Update AMF to 1.4.34.
  Requires AMD Radeon Software Adrenalin Edition 24.6.1 or later.
- Update to ffmpeg libs to 7.0 on Windows.
  - ffmpeg 6.1 -> 7.0
  - libpng 1.4.0 -> 1.4.3
  - expat 2.5.0 -> 2.6.2
  - opus 1.4 -> 1.5.2
  - libxml2 2.12.0 -> 2.12.6
  - dav1d 1.3.0 -> 1.4.1
  - libvpl 2.11.0 (new!)
  - nv-codec-headers 12.2.72.0 (new!)
- Add new denoise filter ([--vpp-fft3d](./VCEEncC_Options.en.md#--vpp-fft3d-param1value1param2value2))
- Add new denoise filter. ([--vpp-nlmeans](./VCEEncC_Options.en.md#--vpp-nlmeans-param1value1param2value2))
- Add new deinterlace filter. ([--vpp-decomb](./VCEEncC_Options.en.md#--vpp-decomb-param1value1param2value2)).
- Add per channel options to [--vpp-tweak](./VCEEncC_Options.en.md#--vpp-tweak-param1value1param2value2).
- Add frame rate conversion filter. ([--vpp-frc](./VCEEncC_Options.en.md#--vpp-frc-param1value1param2value2))
- Add ```ignore_sar``` options to [--output-res](output-res).
- Add option to specify portable VapourSynth dir when using vpy reader. ([--vsdir](./VCEEncC_Options.en.md#--vsdir-string))
- Extend [--audio-resampler](./VCEEncC_Options.en.md#--audio-resampler-string) to take extra options.
- Now [--avsw](./VCEEncC_Options.en.md#--avsw-string) can specify the decoder name to be used.
- When [--audio-bitrate](./VCEEncC_Options.en.md#--audio-bitrate-intstringint) is not specified, let codec decide it's bitrate instead of setting a default bitrate of 192kbps.
- Improve audio channel selection when output codec does not support the same audio channels as the input audio. 
- Improve Windows language locale settings.
- Don't process audio/subtitle/data tracks specified by [--audio-bitrate](./VCEEncC_Options.en.md#--audio-bitrate-intstringint) or [--audio-copy](./VCEEncC_Options.en.md#--audio-copy-intstringintstring).
- Improve handling of missing partial frame timestamps.
- Slightly improve performance of [--vpp-smooth](./VCEEncC_Options.en.md#--vpp-smooth-param1value1param2value2) and [--vpp-denoise-dct](./VCEEncC_Options.en.md#--vpp-denoise-dct-param1value1param2value2).
- Fix crush of y4m reader when width cannot be divided by 16.
- Fix encode freezing when [--avsw](./VCEEncC_Options.en.md#--avsw-string), [--pa](./VCEEncC_Options.en.md#--pa--param1value1param2value2), [--vpp-resize](./VCEEncC_Options.en.md#--vpp-resize-string) amf_* are used at the same time. ( #103 )
- Fix seek issue caused by audio getting muxed to a different fragment than the video at the same time, due to insufficient buffer for audio mux. ( #109 )

## 8.22

- Add new noise reduction filter. (--vpp-denoise-dct)
- Add option to specify audio by quality. ( --audio-quality )
- Fix spline16/36/64 resize might have crushed on some environments.
- Fix --vpp-smooth creating artifacts when run on RDNA3 GPUs.
- Fix --vpp-smooth strength did not match that of 8-bit output when 10-bit output.
- Fix problem when bitrate and max-bitrate is both set to 0. ( #102 )
- Fix debug log output of --vpp-smooth.
- Fix debug log output of --vpp-colorspace.
- Changed default value of --avsync from cfr to auto, which does not fit the actual situation.

## 8.21

- Update windows libav* dlls.
  - ffmpeg 5.1 -> 6.1
  - libpng 1.3.9 -> 1.4.0
  - opus 1.3.1 -> 1.4
  - libsndfile 1.2.0 -> 1.2.2
  - libxml2 2.10.3 -> 2.12.0
  - dav1d 1.0.0 -> 1.3.0
  - libaribcaption 1.1.1 (new!)

- Avoid crush from VCEEnc 8.17 on systems which does not support AVX2.
- Improved progress display when --seek is used.
- Fix --option-file error when target file is empty.
- Changed --audio-delay to allow passing in decimal points.
- Improve AAC copy handling when muxing into mpegts.
- Remove ```--caption2ass```.
  Equivalent processing can be done by ```--sub-codec ass#sub_type=ass,ass_single_rect=true```.