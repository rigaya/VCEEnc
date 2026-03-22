
# How to build VCEEnc

- [Windows](./Build.en.md#windows)
- Linux
  - [Linux (Ubuntu 24.04)](./Build.en.md#linux-ubuntu-2404)

## Windows

### 0. Requirements
To build VCEEnc, components below are required.

- Visual Studio 2022
- [Avisynth](https://github.com/AviSynth/AviSynthPlus) SDK
- [VapourSynth](http://www.vapoursynth.com/) SDK

Install Avisynth+ and VapourSynth, with the SDKs.

Then, "avisynth_c.h" of the Avisynth+ SDK and "VapourSynth.h" of the VapourSynth SDK should be added to the include path of Visual Studio.

These include path can be passed by environment variables "AVISYNTH_SDK" and "VAPOURSYNTH_SDK".

With default installation, environment variables could be set as below.
```Batchfile
setx AVISYNTH_SDK "C:\Program Files (x86)\AviSynth+\FilterSDK"
setx VAPOURSYNTH_SDK "C:\Program Files (x86)\VapourSynth\sdk"
```

[OpenCL header files](https://github.com/KhronosGroup/OpenCL-Headers.git) will also be needed,
and the include path shall be passed by environment variables "OPENCL_HEADERS".

```Batchfile
git clone https://github.com/KhronosGroup/OpenCL-Headers.git <path-to-clone>
setx OPENCL_HEADERS <path-to-clone>
```


### 1. Download source code

```Batchfile
git clone https://github.com/rigaya/VCEEnc --recursive
cd VCEEnc
curl -s -o ffmpeg_lgpl.7z -L https://github.com/rigaya/ffmpeg_dlls_for_hwenc/releases/download/20250830/ffmpeg_dlls_for_hwenc_20250830.7z
7z x -offmpeg_lgpl -y ffmpeg_lgpl.7z
```

### 2. Build VCEEncC.exe / VCEEnc.auo

Finally, open VCEEnc.sln, and start build of VCEEnc by Visual Studio.

|  |For Debug build|For Release build|
|:--------------|:--------------|:--------|
|VCEEnc.auo (win32 only) | Debug | Release |
|VCEEncC(64).exe | DebugStatic | RelStatic |


## Linux (Ubuntu 24.04)

### 0. Requirements

- C++17 Compiler
- meson + ninja-build
- AMD Graphics driver
- git
- libraries
  - OpenCL headers / Vulkan headers / X11 headers
  - ffmpeg libs  (libavcodec*, libavformat*, libavfilter*, libavutil*, libswresample*, libavdevice*)
  - libass9
  - [Optional] VapourSynth

### 1. Install build tools

```Shell
sudo apt install build-essential libtool pkg-config git meson ninja-build
```

- Install rust + cargo-c (for libdovi, libhdr10plus build)

  ```Shell
  sudo apt install libssl-dev curl pkgconf
  curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --profile minimal \
    && . ~/.cargo/env \
    && cargo install cargo-c
  ```

### 2. Install AMD Graphics driver

Download AMD Graphics driver packages for Ubuntu 24.04 from [the webpage of AMD](https://support.amd.com/en-us/download).

Extract and run `amdgpu-install` to install the All-Open stack.

  > [!WARNING]
  > On some Ubuntu 24.04 + RADV environments, installing the latest AMF userspace causes encoder initialization failures such as `Pal::IPlatform::EnumerateDevices()` or `luid not found in devices returned by Pal::IPlatform::EnumerateDevices()`.
  > Keeping the All-Open stack and replacing only the AMF userspace packages with the 6.4.4 / 25.10 generation appears to work around this issue.

```Shell
cd ~/Downloads
sudo apt-get install ./amdgpu-install-VERSION.deb
sudo apt-get update
sudo amdgpu-install -y --opencl=rocr
```

### 3. [Workaround] Replace AMF userspace with the 6.4.4 / 25.10 generation

Instead of the latest AMF userspace, install `amdgpu-pro-core`, `libamdenc-amdgpu-pro`, and `amf-amdgpu-pro` from the 6.4.4 / 25.10 generation.

```Shell
mkdir -p ~/amf-6.4.4
cd ~/amf-6.4.4

wget https://repo.radeon.com/amdgpu/6.4.4/ubuntu/pool/proprietary/a/amdgpu-pro-core/amdgpu-pro-core_25.10-2203192.24.04_all.deb
wget https://repo.radeon.com/amdgpu/6.4.4/ubuntu/pool/proprietary/liba/libamdenc-amdgpu-pro/libamdenc-amdgpu-pro_25.10-2203192.24.04_amd64.deb
wget https://repo.radeon.com/amdgpu/6.4.4/ubuntu/pool/proprietary/a/amf-amdgpu-pro/amf-amdgpu-pro_1.4.37-2203192.24.04_amd64.deb

sudo apt remove --purge -y amf-amdgpu-pro libamdenc-amdgpu-pro

sudo apt install -y --allow-downgrades \
  ./amdgpu-pro-core_25.10-2203192.24.04_all.deb \
  ./libamdenc-amdgpu-pro_25.10-2203192.24.04_amd64.deb \
  ./amf-amdgpu-pro_1.4.37-2203192.24.04_amd64.deb

sudo apt-mark hold amdgpu-pro-core libamdenc-amdgpu-pro amf-amdgpu-pro
```

### 4. Install required libraries

```Shell
sudo apt install \
  opencl-headers \
  libvulkan-dev \
  libx11-dev

sudo apt install ffmpeg \
  libavcodec-extra libavcodec-dev libavutil-dev libavformat-dev libswresample-dev libavfilter-dev libavdevice-dev \
  libass9 libass-dev
```

### 5. Add user to proper groups and reboot

```Shell
# OpenCL
sudo gpasswd -a ${USER} render
sudo gpasswd -a ${USER} video
sudo reboot
```

### 6. Check GPU Recognition Status

Check if your GPU is properly recognized before building VCEEncC.

```Shell
sudo apt install vulkan-utils clinfo

# Check GPU recognition status with OpenCL
clinfo

# Check GPU recognition status with Vulkan
vulkaninfo --summary
```

Especially with `vulkaninfo`, confirm that your target AMD GPU is recognized as `GPU0`.

### 7. [Optional] Install VapourSynth
VapourSynth is required only if you need VapourSynth(vpy) reader support.  

Please go on to [8. Build VCEEncC] if you don't need vpy reader.

<details><summary>How to build VapourSynth</summary>

#### 7.1 Install build tools for VapourSynth
```Shell
sudo apt install python3-pip autoconf automake libtool meson
```

#### 7.2 Install zimg
```Shell
git clone https://github.com/sekrit-twc/zimg.git
cd zimg
./autogen.sh
./configure
sudo make install -j16
cd ..
```

#### 7.3 Install cython
```Shell
sudo pip3 install Cython
```

#### 7.4 Install VapourSynth
```Shell
git clone https://github.com/vapoursynth/vapoursynth.git
cd vapoursynth
./autogen.sh
./configure
make -j16
sudo make install

# Make sure vapoursynth could be imported from python
# Change "python3.x" depending on your encironment
sudo ln -s /usr/local/lib/python3.x/site-packages/vapoursynth.so /usr/lib/python3.x/lib-dynload/vapoursynth.so
sudo ldconfig
```

#### 7.5 Check if VapourSynth has been installed properly
Make sure you get version number without errors.
```Shell
vspipe --version
```

#### 7.6 [Option] Build vslsmashsource
```Shell
# Install lsmash
git clone https://github.com/l-smash/l-smash.git
cd l-smash
./configure --enable-shared
sudo make install -j16
cd ..
 
# Install vslsmashsource
git clone https://github.com/HolyWu/L-SMASH-Works.git
# As the latest version requires more recent ffmpeg libs, checkout the older version
cd L-SMASH-Works
git checkout -b 20200531 refs/tags/20200531
cd VapourSynth
meson build
cd build
sudo ninja install
cd ../../../
```

</details>

### 8. Build VCEEncC
```Shell
git clone https://github.com/rigaya/VCEEnc --recursive
cd VCEEnc
meson setup ./build --buildtype=release
meson compile -C ./build
```
Check if it works properly.
```Shell
./build/vceencc --check-hw
```
