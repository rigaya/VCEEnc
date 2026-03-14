project('vceencc', ['c', 'cpp'],
  version: run_command('git', 'describe', '--tags', '--abbrev=0', check: true).stdout().strip(),
  license: 'MIT',
  meson_version: '>=0.61.0',
  default_options: [
    'cpp_std=c++17',
    'buildtype=release',
    'warning_level=1',
  ]
)

cpp = meson.get_compiler('cpp')
fs = import('fs')

if host_machine.system() != 'linux'
  error('This meson.build is for Linux only')
endif

source_root = meson.project_source_root()
build_root = meson.project_build_root()

enable_vapoursynth = get_option('enable_vapoursynth')
enable_avisynth = get_option('enable_avisynth')
enable_libass = get_option('enable_libass')
enable_libplacebo = get_option('enable_libplacebo')
enable_vulkan = get_option('enable_vulkan')
enable_dtl_opt = get_option('enable_dtl')
opencl_headers = get_option('opencl_headers')

threads_dep = dependency('threads')
dl_dep = cpp.find_library('dl', required: true)
m_dep = cpp.find_library('m', required: true)
stdcxxfs_dep = cpp.find_library('stdc++fs', required: false)

libva_dep = dependency('libva', required: true)
libva_drm_dep = dependency('libva-drm', required: false)
libva_x11_dep = dependency('libva-x11', required: false)
libdrm_dep = dependency('libdrm', required: false)

opencl_inc_args = []
if opencl_headers != ''
  opencl_inc_args += ['-I' + opencl_headers]
endif
if not cpp.has_header('CL/opencl.h', args: opencl_inc_args, required: false)
  error('OpenCL headers not found (CL/opencl.h)')
endif
if not cpp.has_header('clRNG/clRNG.h', args: ['-I' + source_root / 'clRNG/src/include'], required: false)
  error('clRNG headers not found (clRNG/clRNG.h)')
endif

libavutil_dep = dependency('libavutil', required: true)
libavcodec_dep = dependency('libavcodec', required: true)
libavformat_dep = dependency('libavformat', required: true)
libavfilter_dep = dependency('libavfilter', required: true)
libavdevice_dep = dependency('libavdevice', required: true)
libswresample_dep = dependency('libswresample', required: true)

ffmpeg_deps = [
  libavutil_dep,
  libavcodec_dep,
  libavformat_dep,
  libavfilter_dep,
  libavdevice_dep,
  libswresample_dep,
]

vapoursynth_pkg_dep = dependency('vapoursynth', required: false)
avisynth_pkg_dep = dependency('avisynth', required: false)

vapoursynth_compile_dep = declare_dependency()
if vapoursynth_pkg_dep.found()
  vapoursynth_compile_dep = vapoursynth_pkg_dep.partial_dependency(includes: true, compile_args: true)
endif

avisynth_compile_dep = declare_dependency()
if avisynth_pkg_dep.found()
  avisynth_compile_dep = avisynth_pkg_dep.partial_dependency(includes: true, compile_args: true)
endif

vapoursynth_v3_headers_code = '''
#include "VapourSynth.h"
#include "VSScript.h"
int main(void) { return 0; }
'''
have_vapoursynth_v3_headers = cpp.compiles(
  vapoursynth_v3_headers_code,
  dependencies: vapoursynth_compile_dep,
  args: get_option('cpp_args'),
)

vapoursynth_v4_headers_code = '''
#include "VSScript4.h"
int main(void) { return 0; }
'''
have_vapoursynth_v4_headers = cpp.compiles(
  vapoursynth_v4_headers_code,
  dependencies: vapoursynth_compile_dep,
  args: get_option('cpp_args'),
)

have_vapoursynth_headers = have_vapoursynth_v3_headers and have_vapoursynth_v4_headers
if enable_vapoursynth and not have_vapoursynth_headers
  error('VapourSynth headers not found (require V3: VapourSynth.h/VSScript.h and V4: VSScript4.h)')
endif

avisynth_headers_code = '''
#include "avisynth_c.h"
int main(void) { return 0; }
'''
have_avisynth_headers = cpp.compiles(
  avisynth_headers_code,
  dependencies: avisynth_compile_dep,
  args: get_option('cpp_args'),
)
if enable_avisynth and not have_avisynth_headers
  error('AviSynth headers not found (avisynth_c.h)')
endif
libass_dep = dependency('libass', required: enable_libass)
if enable_vulkan
  vulkan_dep = dependency('vulkan', required: true)
else
  vulkan_dep = disabler()
endif
if enable_libplacebo
  libplacebo_dep = dependency('libplacebo', required: true)
else
  libplacebo_dep = disabler()
endif

if libplacebo_dep.found()
  placebo_api_check = '''
#include <libplacebo/config.h>
#if PL_API_VER < 338
#error "libplacebo API too old"
#endif
int main(void) { return 0; }
'''
  if not cpp.compiles(placebo_api_check, dependencies: libplacebo_dep)
    error('libplacebo API version >= 338 required')
  endif
endif


# libdovi / libhdr10plus: system優先、なければ同梱build (configure互換)
cargo_prog = find_program('cargo', required: false)
if cargo_prog.found()
  cargo_home_dir = source_root / '.cargo_home'
  cargo_c_check = run_command(
    'env',
    'CARGO_HOME=' + cargo_home_dir,
    cargo_prog.full_path(),
    'cinstall',
    '--version',
    check: false,
  )
  if cargo_c_check.returncode() != 0
    cargo_c_install = run_command(
      'env',
      'CARGO_HOME=' + cargo_home_dir,
      cargo_prog.full_path(),
      'install',
      'cargo-c',
      '--locked',
      check: false,
    )
    if cargo_c_install.returncode() != 0
      error('Failed to install cargo-c (exit=' + cargo_c_install.returncode().to_string() + ')\nstdout:\n' + cargo_c_install.stdout().strip() + '\nstderr:\n' + cargo_c_install.stderr().strip())
    endif
  endif
endif

libdovi_dep = dependency('dovi', required: false)
if not libdovi_dep.found()
  if not cargo_prog.found()
    error('cargo is required to build bundled libdovi when system dovi is not available')
  endif
  libdovi_install_dir = build_root / 'rust_libs' / 'libdovi'
  libdovi_build_result = run_command(
    'env',
    'CARGO_HOME=' + cargo_home_dir,
    'CARGO=' + cargo_prog.full_path(),
    'bash',
    source_root / 'build_libdovi.sh',
    libdovi_install_dir,
    check: false,
  )
  if libdovi_build_result.returncode() != 0
    error('Failed to build bundled libdovi (exit=' + libdovi_build_result.returncode().to_string() + ')\nstdout:\n' + libdovi_build_result.stdout().strip() + '\nstderr:\n' + libdovi_build_result.stderr().strip())
  endif
  libdovi_lib_dirs = []
  if fs.is_file(libdovi_install_dir / 'lib/x86_64-linux-gnu/libdovi.a')
    libdovi_lib_dirs += libdovi_install_dir / 'lib/x86_64-linux-gnu'
  elif fs.is_file(libdovi_install_dir / 'lib64/libdovi.a')
    libdovi_lib_dirs += libdovi_install_dir / 'lib64'
  elif fs.is_file(libdovi_install_dir / 'lib/libdovi.a')
    libdovi_lib_dirs += libdovi_install_dir / 'lib'
  endif
  if libdovi_lib_dirs.length() == 0
    error('Bundled libdovi build succeeded but libdovi.a was not found')
  endif
  libdovi_lib = cpp.find_library('dovi', dirs: libdovi_lib_dirs, static: true, required: true)
  libdovi_dep = declare_dependency(
    dependencies: libdovi_lib,
    compile_args: ['-I' + (libdovi_install_dir / 'include')],
  )
  if not cpp.compiles(
    '#include <libdovi/rpu_parser.h>\nint main(void){ dovi_parse_rpu(nullptr, 0); return 0; }',
    dependencies: libdovi_dep,
  )
    error('Bundled libdovi check failed (libdovi/rpu_parser.h)')
  endif
endif

libhdr10plus_dep = dependency('hdr10plus-rs', required: false)
if not libhdr10plus_dep.found()
  if not cargo_prog.found()
    error('cargo is required to build bundled libhdr10plus-rs when system hdr10plus-rs is not available')
  endif
  libhdr10plus_install_dir = build_root / 'rust_libs' / 'libhdr10plus'
  libhdr10plus_build_result = run_command(
    'env',
    'CARGO_HOME=' + cargo_home_dir,
    'CARGO=' + cargo_prog.full_path(),
    'bash',
    source_root / 'build_libhdr10plus.sh',
    libhdr10plus_install_dir,
    check: false,
  )
  if libhdr10plus_build_result.returncode() != 0
    error('Failed to build bundled libhdr10plus-rs (exit=' + libhdr10plus_build_result.returncode().to_string() + ')\nstdout:\n' + libhdr10plus_build_result.stdout().strip() + '\nstderr:\n' + libhdr10plus_build_result.stderr().strip())
  endif
  libhdr10plus_lib_dirs = []
  if fs.is_file(libhdr10plus_install_dir / 'lib/x86_64-linux-gnu/libhdr10plus-rs.a')
    libhdr10plus_lib_dirs += libhdr10plus_install_dir / 'lib/x86_64-linux-gnu'
  elif fs.is_file(libhdr10plus_install_dir / 'lib64/libhdr10plus-rs.a')
    libhdr10plus_lib_dirs += libhdr10plus_install_dir / 'lib64'
  elif fs.is_file(libhdr10plus_install_dir / 'lib/libhdr10plus-rs.a')
    libhdr10plus_lib_dirs += libhdr10plus_install_dir / 'lib'
  endif
  if libhdr10plus_lib_dirs.length() == 0
    error('Bundled libhdr10plus-rs build succeeded but libhdr10plus-rs.a was not found')
  endif
  libhdr10plus_lib = cpp.find_library('hdr10plus-rs', dirs: libhdr10plus_lib_dirs, static: true, required: true)
  libhdr10plus_dep = declare_dependency(
    dependencies: libhdr10plus_lib,
    compile_args: ['-I' + (libhdr10plus_install_dir / 'include')],
  )
  if not cpp.compiles(
    '#include <libhdr10plus-rs/hdr10plus.h>\nint main(void){ hdr10plus_rs_parse_json(nullptr); return 0; }',
    dependencies: libhdr10plus_dep,
  )
    error('Bundled libhdr10plus-rs check failed (libhdr10plus-rs/hdr10plus.h)')
  endif
endif

have_dtl_header = cpp.has_header('dtl/dtl.hpp', args: ['-I' + source_root / 'dtl'], required: false)
enable_dtl = enable_dtl_opt and have_dtl_header

conf_data = configuration_data()

av_channel_layout_code = '''
#include <libavutil/channel_layout.h>
int main(void) { AVChannelLayout ch_layout; return 0; }
'''
conf_data.set10('AV_CHANNEL_LAYOUT_STRUCT_AVAIL',
  cpp.compiles(av_channel_layout_code, dependencies: libavutil_dep))

av_frame_duration_code = '''
#include <libavutil/frame.h>
int main(void) { AVFrame *frame; frame->duration = 0; return 0; }
'''
conf_data.set10('AV_FRAME_DURATION_AVAIL',
  cpp.compiles(av_frame_duration_code, dependencies: libavutil_dep))

avcodec_par_side_data_code = '''
#include <libavcodec/codec_par.h>
int main(void) { AVCodecParameters *codecpar; codecpar->coded_side_data = 0; return 0; }
'''
conf_data.set10('AVCODEC_PAR_CODED_SIDE_DATA_AVAIL',
  cpp.compiles(avcodec_par_side_data_code, dependencies: libavcodec_dep))

conf_data.set10('ENABLE_RAW_READER', true)
conf_data.set10('ENABLE_AVI_READER', false)
conf_data.set10('ENABLE_AVISYNTH_READER', enable_avisynth and have_avisynth_headers)
conf_data.set10('ENABLE_VAPOURSYNTH_READER', enable_vapoursynth and have_vapoursynth_headers)
conf_data.set10('ENABLE_AVSW_READER', true)
conf_data.set10('ENABLE_SM_READER', false)
conf_data.set10('ENABLE_CUSTOM_VPP', true)
conf_data.set10('ENABLE_LIBASS_SUBBURN', libass_dep.found())
conf_data.set10('ENABLE_OPENCL', true)
conf_data.set10('ENABLE_AVCODEC_OUT_THREAD', true)
conf_data.set10('ENABLE_CPP_REGEX', true)
conf_data.set10('ENABLE_DTL', enable_dtl)
conf_data.set10('ENABLE_LIBDOVI', libdovi_dep.found())
conf_data.set10('ENABLE_LIBHDR10PLUS', libhdr10plus_dep.found())
conf_data.set10('ENABLE_VULKAN', vulkan_dep.found() and libplacebo_dep.found())
conf_data.set10('ENABLE_LIBPLACEBO', libplacebo_dep.found())

if enable_avisynth and have_avisynth_headers
  conf_data.set('AVS_INTERF_VER', 12)
else
  conf_data.set('AVS_INTERF_VER', 0)
endif

configure_file(
  output: 'rgy_config.h',
  configuration: conf_data,
)

rev_count = run_command('sh', '-c', 'cd "' + source_root + '" && git rev-list HEAD | wc -l', check: false).stdout().strip()
if rev_count == ''
  rev_count = '0'
endif

rev_data = configuration_data()
rev_data.set_quoted('ENCODER_REV', rev_count)
configure_file(
  output: 'rgy_rev.h',
  configuration: rev_data,
)

build_dir_include = '-I' + meson.current_build_dir()

common_cpp_args = [
  build_dir_include,
  '-DLINUX',
  '-DUNIX',
  '-D_FILE_OFFSET_BITS=64',
  '-D__USE_LARGEFILE64',
  '-D__STDC_FORMAT_MACROS',
  '-D__STDC_CONSTANT_MACROS',
  '-Wno-unknown-pragmas',
  '-Wno-unused',
  '-Wno-missing-braces',
  '-msse4.1',
] + opencl_inc_args

if libva_dep.found()
  common_cpp_args += ['-DLIBVA_SUPPORT=1']
else
  common_cpp_args += ['-DLIBVA_SUPPORT=0']
endif
if libva_drm_dep.found()
  common_cpp_args += ['-DLIBVA_DRM_SUPPORT=1']
else
  common_cpp_args += ['-DLIBVA_DRM_SUPPORT=0']
endif
if libva_x11_dep.found()
  common_cpp_args += ['-DLIBVA_X11_SUPPORT=1']
else
  common_cpp_args += ['-DLIBVA_X11_SUPPORT=0']
endif

common_c_args = [
  '-DLINUX',
  '-DUNIX',
  '-D_FILE_OFFSET_BITS=64',
  '-D__USE_LARGEFILE64',
  '-D__STDC_FORMAT_MACROS',
  '-D__STDC_CONSTANT_MACROS',
  '-Wno-unknown-pragmas',
  '-Wno-unused',
  '-Wno-missing-braces',
  '-msse4.1',
]

include_dirs_list = [
  '.',
  'AMF/amf/public/include',
  'AMF/amf/public/include/core',
  'AMF/amf/public/include/components',
  'VCECore',
  'tinyxml2',
  'cppcodec',
  'clRNG/src/include',
]
if enable_dtl
  include_dirs_list += ['dtl']
endif

common_include_dirs = include_directories(include_dirs_list)

vcecore_sources = files(
  'VCECore/convert_csp.cpp',
  'VCECore/cpu_info.cpp',
  'VCECore/gpu_info.cpp',
  'VCECore/gpuz_info.cpp',
  'VCECore/logo.cpp',
  'VCECore/rgy_aspect_ratio.cpp',
  'VCECore/rgy_avlog.cpp',
  'VCECore/rgy_avutil.cpp',
  'VCECore/rgy_bitstream.cpp',
  'VCECore/rgy_bitstream_aac.cpp',
  'VCECore/rgy_chapter.cpp',
  'VCECore/rgy_cmd.cpp',
  'VCECore/rgy_codepage.cpp',
  'VCECore/rgy_def.cpp',
  'VCECore/rgy_device.cpp',
  'VCECore/rgy_device_info_cache.cpp',
  'VCECore/rgy_device_usage.cpp',
  'VCECore/rgy_device_vulkan.cpp',
  'VCECore/rgy_env.cpp',
  'VCECore/rgy_err.cpp',
  'VCECore/rgy_event.cpp',
  'VCECore/rgy_faw.cpp',
  'VCECore/rgy_filesystem.cpp',
  'VCECore/rgy_filter.cpp',
  'VCECore/rgy_filter_afs.cpp',
  'VCECore/rgy_filter_afs_analyze.cpp',
  'VCECore/rgy_filter_afs_filter.cpp',
  'VCECore/rgy_filter_afs_merge.cpp',
  'VCECore/rgy_filter_afs_synthesize.cpp',
  'VCECore/rgy_filter_cl.cpp',
  'VCECore/rgy_filter_colorspace.cpp',
  'VCECore/rgy_filter_crop.cpp',
  'VCECore/rgy_filter_convolution3d.cpp',
  'VCECore/rgy_filter_curves.cpp',
  'VCECore/rgy_filter_deband.cpp',
  'VCECore/rgy_filter_decimate.cpp',
  'VCECore/rgy_filter_decomb.cpp',
  'VCECore/rgy_filter_delogo.cpp',
  'VCECore/rgy_filter_denoise_dct.cpp',
  'VCECore/rgy_filter_denoise_fft3d.cpp',
  'VCECore/rgy_filter_denoise_knn.cpp',
  'VCECore/rgy_filter_denoise_nlmeans.cpp',
  'VCECore/rgy_filter_denoise_pmd.cpp',
  'VCECore/rgy_filter_edgelevel.cpp',
  'VCECore/rgy_filter_libplacebo.cpp',
  'VCECore/rgy_filter_mpdecimate.cpp',
  'VCECore/rgy_filter_nnedi.cpp',
  'VCECore/rgy_filter_overlay.cpp',
  'VCECore/rgy_filter_pad.cpp',
  'VCECore/rgy_filter_resize.cpp',
  'VCECore/rgy_filter_rff.cpp',
  'VCECore/rgy_filter_smooth.cpp',
  'VCECore/rgy_filter_ssim.cpp',
  'VCECore/rgy_filter_subburn.cpp',
  'VCECore/rgy_filter_transform.cpp',
  'VCECore/rgy_filter_tweak.cpp',
  'VCECore/rgy_filter_unsharp.cpp',
  'VCECore/rgy_filter_warpsharp.cpp',
  'VCECore/rgy_filter_yadif.cpp',
  'VCECore/rgy_frame.cpp',
  'VCECore/rgy_frame_info.cpp',
  'VCECore/rgy_hdr10plus.cpp',
  'VCECore/rgy_ini.cpp',
  'VCECore/rgy_input.cpp',
  'VCECore/rgy_input_avcodec.cpp',
  'VCECore/rgy_input_avi.cpp',
  'VCECore/rgy_input_avs.cpp',
  'VCECore/rgy_input_raw.cpp',
  'VCECore/rgy_input_sm.cpp',
  'VCECore/rgy_input_vpy.cpp',
  'VCECore/rgy_language.cpp',
  'VCECore/rgy_libdovi.cpp',
  'VCECore/rgy_libplacebo.cpp',
  'VCECore/rgy_level.cpp',
  'VCECore/rgy_level_av1.cpp',
  'VCECore/rgy_level_h264.cpp',
  'VCECore/rgy_level_hevc.cpp',
  'VCECore/rgy_memmem.cpp',
  'VCECore/rgy_log.cpp',
  'VCECore/rgy_opencl.cpp',
  'VCECore/rgy_output.cpp',
  'VCECore/rgy_output_avcodec.cpp',
  'VCECore/rgy_parallel_enc.cpp',
  'VCECore/rgy_perf_counter.cpp',
  'VCECore/rgy_perf_monitor.cpp',
  'VCECore/rgy_pipe.cpp',
  'VCECore/rgy_pipe_linux.cpp',
  'VCECore/rgy_prm.cpp',
  'VCECore/rgy_resource.cpp',
  'VCECore/rgy_simd.cpp',
  'VCECore/rgy_status.cpp',
  'VCECore/rgy_thread_affinity.cpp',
  'VCECore/rgy_timecode.cpp',
  'VCECore/rgy_util.cpp',
  'VCECore/rgy_vapoursynth_wrapper.cpp',
  'VCECore/rgy_vapoursynth_wrapper_v3.cpp',
  'VCECore/rgy_vapoursynth_wrapper_v4.cpp',
  'VCECore/rgy_version.cpp',
  'VCECore/rgy_vulkan.cpp',
  'VCECore/rgy_wav_parser.cpp',
  'VCECore/vce_amf.cpp',
  'VCECore/vce_filter.cpp',
  'VCECore/vce_cmd.cpp',
  'VCECore/vce_core.cpp',
  'VCECore/vce_device.cpp',
  'VCECore/vce_param.cpp',
  'VCECore/vce_util.cpp',
)

clrng_sources = files(
  'clRNG/src/library/clRNG.c',
  'clRNG/src/library/private.c',
)

tinyxml2_sources = files('tinyxml2/tinyxml2.cpp')
vceencc_sources = files('VCEEncC/VCEEncC.cpp')

simd_sse2_lib = static_library('simd_sse2',
  'VCECore/convert_csp_sse2.cpp',
  cpp_args: common_cpp_args + ['-msse2'],
  include_directories: common_include_dirs,
)

simd_ssse3_lib = static_library('simd_ssse3',
  'VCECore/convert_csp_ssse3.cpp',
  cpp_args: common_cpp_args + ['-mssse3'],
  include_directories: common_include_dirs,
)

simd_sse41_lib = static_library('simd_sse41',
  'VCECore/convert_csp_sse41.cpp',
  cpp_args: common_cpp_args + ['-msse4.1'],
  include_directories: common_include_dirs,
)

simd_avx_lib = static_library('simd_avx',
  'VCECore/convert_csp_avx.cpp',
  cpp_args: common_cpp_args + ['-mavx', '-mpopcnt'],
  include_directories: common_include_dirs,
)

simd_avx2_lib = static_library('simd_avx2',
  [
    'VCECore/convert_csp_avx2.cpp',
    'VCECore/rgy_bitstream_avx2.cpp',
    'VCECore/rgy_faw_avx2.cpp',
    'VCECore/rgy_memmem_avx2.cpp',
  ],
  cpp_args: common_cpp_args + ['-mavx2', '-mfma', '-mpopcnt', '-mbmi', '-mbmi2'],
  include_directories: common_include_dirs,
)

simd_avx512_lib = static_library('simd_avx512',
  [
    'VCECore/rgy_bitstream_avx512bw.cpp',
    'VCECore/rgy_faw_avx512bw.cpp',
    'VCECore/rgy_memmem_avx512bw.cpp',
  ],
  cpp_args: common_cpp_args + ['-mavx512f', '-mavx512bw', '-mpopcnt', '-mbmi', '-mbmi2'],
  include_directories: common_include_dirs,
)

simd_libs = [
  simd_sse2_lib,
  simd_ssse3_lib,
  simd_sse41_lib,
  simd_avx_lib,
  simd_avx2_lib,
  simd_avx512_lib,
]

all_deps = [
  threads_dep,
  dl_dep,
  m_dep,
] + ffmpeg_deps

if stdcxxfs_dep.found()
  all_deps += stdcxxfs_dep
endif
if libva_dep.found()
  all_deps += libva_dep
endif
if libva_drm_dep.found()
  all_deps += libva_drm_dep
endif
if libva_x11_dep.found()
  all_deps += libva_x11_dep
endif
if libdrm_dep.found()
  all_deps += libdrm_dep
endif
if vapoursynth_compile_dep.found()
  all_deps += vapoursynth_compile_dep
endif
if avisynth_compile_dep.found()
  all_deps += avisynth_compile_dep
endif
if libass_dep.found()
  all_deps += libass_dep
endif
if vulkan_dep.found()
  all_deps += vulkan_dep
endif
if libplacebo_dep.found()
  all_deps += libplacebo_dep
endif
if libdovi_dep.found()
  all_deps += libdovi_dep
endif
if libhdr10plus_dep.found()
  all_deps += libhdr10plus_dep
endif

subdir('meson_embed')

executable('vceencc',
  vcecore_sources + clrng_sources + tinyxml2_sources + vceencc_sources + resource_objects,
  c_args: common_c_args,
  cpp_args: common_cpp_args,
  include_directories: common_include_dirs,
  dependencies: all_deps,
  link_with: simd_libs,
  install: true,
)

summary({
  'libva': libva_dep.found(),
  'libva-drm': libva_drm_dep.found(),
  'libva-x11': libva_x11_dep.found(),
  'libdrm': libdrm_dep.found(),
  'VapourSynth': enable_vapoursynth and have_vapoursynth_headers,
  'AviSynth': enable_avisynth and have_avisynth_headers,
  'libass': libass_dep.found(),
  'libplacebo': libplacebo_dep.found(),
  'Vulkan': vulkan_dep.found(),
  'dtl': enable_dtl,
  'libdovi': libdovi_dep.found(),
  'libhdr10plus': libhdr10plus_dep.found(),
}, section: 'VCEEnc Features')
