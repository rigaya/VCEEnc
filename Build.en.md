
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
curl -s -o ffmpeg_lgpl.7z -L https://github.com/rigaya/ffmpeg_dlls_for_hwenc/releases/download/20241102/ffmpeg_dlls_for_hwenc_20241102.7z
7z x -offmpeg_lgpl -y ffmpeg_lgpl.7z
```

### 2. Build VCEEncC.exe / VCEEnc.auo

Finally, open VCEEnc.sln, and start build of VCEEnc by Visual Studio.

|  |For Debug build|For Release build|
|:--------------|:--------------|:--------|
|VCEEnc.auo (win32 only) | Debug | Release |
|VCEEncC(64).exe | DebugStatic | RelStatic |


## Linux (Ubuntu 24.04)

  > [!WARNING]
  > Target AMD Radeon GPU must be recognized as "GPU0" in ```vulkaninfo --summary```, otherwise it might fail to run.

### 0. Requirements

- C++17 Compiler
- Intel Driver
- git
- libraries
  - libva, libdrm, libmfx 
  - ffmpeg libs (libavcodec58, libavformat58, libavfilter7, libavutil56, libswresample3, libavdevice58)
  - libass9
  - [Optional] VapourSynth

### 1. Install build tools

```Shell
sudo apt install build-essential libtool pkg-config git
```

### 2. Install AMD Graphics driver

Download AMD Graphics driver packages for Ubuntu 24.04 from [the webpage of AMD](https://support.amd.com/en-us/download).

Extract and run amdgpu-install to install the driver.

```Shell
cd ~/Downloads
sudo apt-get install ./amdgpu-install-VERSION.deb
sudo apt-get update
sudo amdgpu-install -y --accept-eula --usecase=graphics,amf,opencl --opencl=rocr --vulkan=amdvlk --no-32
sudo reboot
```

### 3. Install required libraries

```Shell
sudo apt install \
  opencl-headers \
  libvulkan-dev \
  libx11-dev

sudo apt install ffmpeg \
  libavcodec-extra libavcodec-dev libavutil-dev libavformat-dev libswresample-dev libavfilter-dev libavdevice-dev \
  libass9 libass-dev
```

### 4. [Optional] Install VapourSynth
VapourSynth is required only if you need VapourSynth(vpy) reader support.  

Please go on to [5. Build VCEEncC] if you don't need vpy reader.

<details><summary>How to build VapourSynth</summary>

#### 4.1 Install build tools for VapourSynth
```Shell
sudo apt install python3-pip autoconf automake libtool meson
```

#### 4.2 Install zimg
```Shell
git clone https://github.com/sekrit-twc/zimg.git
cd zimg
./autogen.sh
./configure
sudo make install -j16
cd ..
```

#### 4.3 Install cython
```Shell
sudo pip3 install Cython
```

#### 4.4 Install VapourSynth
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

#### 4.5 Check if VapourSynth has been installed properly
Make sure you get version number without errors.
```Shell
vspipe --version
```

#### 4.6 [Option] Build vslsmashsource
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

### 5. Add user to proper group
```Shell
# OpenCL
sudo gpasswd -a ${USER} render
```

### 6. Build VCEEncC
```Shell
git clone https://github.com/rigaya/VCEEnc --recursive
cd VCEEnc
./configure
make -j8
```
Check if it works properly.
```Shell
./vceencc --check-hw
```
