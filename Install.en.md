
# How to install VCEEncC

- [Windows](./Install.en.md#windows)
- Linux
  - [Linux (Ubuntu 24.04 or later)](./Install.en.md#linux)
  - Other Linux OS  
    For other Linux OS, building from source will be needed. Please check the [build instrcutions](./Build.en.md).


## Windows

### 1. Install AMD Radeon Graphics driver
### 2. Download Windows binary  
Windows binary can be found from [this link](https://github.com/rigaya/VCEEnc/releases). VCEEncC_x.xx_Win32.7z contains 32bit exe file, VCEEncC_x.xx_x64.7z contains 64bit exe file.

VCEEncC could be run directly from the extracted directory.
  
## Linux

On Linux, you can choose from two ways to use the HW encoder: AMF or VA-API.

| Method | AMF | VA-API |
|:--|:--:|:--:|
| Supported GPUs | RDNA (Radeon RX 5000 series) or later, Ryzen 5000 series or later APUs | Radeon GCN generation or later |
| Installation | Somewhat complicated | Easy |
| Detailed HW encoder settings | Yes | Basic settings only *1 |
| HW decode (`--avhw`) | Yes | Yes *4 |

*1 Basic settings such as rate control (CQP/CBR/VBR), GOP length and presets are supported. AMF specific detailed settings are ignored with a warning.  
*2 With VA-API, HW decode is used only when `--avhw` is specified explicitly. 

On GPUs not supported by AMF, please choose VA-API. VCEEncC uses AMF when AMF is installed, and automatically uses VA-API when it is not (`--backend auto`, default). To use VA-API on a system with AMF installed, specify `--backend vaapi`.

### 1. Preparation

#### 1-1. When using AMF

> [!WARNING]
> On some Ubuntu 24.04 + RADV environments, installing the latest AMF userspace causes encoder initialization failures such as `Pal::IPlatform::EnumerateDevices()` or `luid not found in devices returned by Pal::IPlatform::EnumerateDevices()`.  
> Related information: [AMF issue #575](https://github.com/GPUOpen-LibrariesAndSDKs/AMF/issues/575), [workaround comment](https://github.com/GPUOpen-LibrariesAndSDKs/AMF/issues/575#issuecomment-4042920061)  
> Keeping the All-Open stack and replacing only the AMF userspace packages (`amf-amdgpu-pro`, `libamdenc-amdgpu-pro`) with the 6.4.4 / 25.10 generation has been confirmed to work around this issue.

##### 1-1-1. Install AMD Graphics driver

Download AMD Graphics driver packages for Ubuntu 24.04 from [the webpage of AMD](https://www.amd.com/en/support).

Extract and run `amdgpu-install` to install the All-Open stack. `--opencl=rocr` also installs ROCm OpenCL, which is used by the OpenCL filters.

<!--
Original/latest AMF userspace installation flow from the AMF Wiki:

```Shell
sudo apt-get install ./amdgpu-install-VERSION.deb
sudo apt-get update
sudo amdgpu-install -y --opencl=rocr

curl -s https://api.github.com/repos/GPUOpen-LibrariesAndSDKs/AMF/releases/latest \
| jq -r '.assets[].browser_download_url | select(test("amf_installer_.*\\.zip$"))' \
| head -n1 \
| xargs -I{} sh -c 'wget -q {}; f=$(basename {}); unzip -o "$f"; sudo "./${f%.zip}.sh" --accept-eula'
```
-->

```Shell
wget https://repo.radeon.com/amdgpu-install/25.35.1/ubuntu/noble/amdgpu-install_7.2.1.70201-1_all.deb
sudo apt-get install ./amdgpu-install_7.2.1.70201-1_all.deb
sudo apt-get update
sudo amdgpu-install -y --opencl=rocr
```

##### 1-1-2. [Workaround] Replace AMF userspace with the 6.4.4 / 25.10 generation

Instead of the latest AMF userspace, install `amdgpu-pro-core`, `libamdenc-amdgpu-pro` and `amf-amdgpu-pro` of the 6.4.4 / 25.10 generation.

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

#### 1-2. When using VA-API

No additional AMD driver installation is required. The VA-API driver of Mesa provided by Ubuntu is used.

```Shell
# VA-API driver, and vainfo for checking
sudo apt install mesa-va-drivers vainfo
```

On Ubuntu 26.04 or later, the VA-API driver is merged into `mesa-libgallium`, so install `mesa-libgallium` instead of `mesa-va-drivers` (it is usually already installed).

OpenCL is required for OpenCL filters (`--vpp-*`) and for handling resolution changes in the middle of the input. With VA-API, OpenCL of Mesa (rusticl) is used. This step is not required if you do not use OpenCL.

```Shell
# OpenCL of Mesa (rusticl), and clinfo for checking
sudo apt install mesa-opencl-icd clinfo
```

Radeon is disabled in rusticl by default, but VCEEncC enables it automatically at runtime (when the environment variable `RUSTICL_ENABLE` is not set, `RUSTICL_ENABLE=radeonsi` is set).

### 2. Add user to the groups below

To use the GPU, the user needs to be in the `render` and `video` groups. This is required for both AMF and VA-API.

```Shell
sudo gpasswd -a ${USER} render
sudo gpasswd -a ${USER} video
sudo reboot
```

### 3. Check GPU recognition status

#### 3-1. When using AMF

```Shell
sudo apt install vulkan-tools clinfo
```

If `clinfo` shows `Number of platforms 0`, register the ICD and the library search path of ROCm OpenCL. Make sure the installation with `--opencl=rocr` has completed, then run the commands below.

```Shell
rocm_icd=$(find /opt/rocm -path '*/etc/OpenCL/vendors/amdocl64.icd' -print -quit)
rocm_root=$(dirname "$(dirname "$(dirname "$(dirname "$rocm_icd")")")")
rocm_lib=$(find "$rocm_root" -path '*/lib/opencl/libamdocl64.so' -print -quit)

sudo install -d /etc/OpenCL/vendors
printf '%s\n' 'libamdocl64.so' |
    sudo tee /etc/OpenCL/vendors/amdocl64.icd >/dev/null
printf '%s\n' "$(dirname "$rocm_lib")" |
    sudo tee /etc/ld.so.conf.d/rocm-opencl.conf >/dev/null
sudo ldconfig
```

Check the GPU recognition status with ```clinfo``` and ```vulkaninfo```. In particular, make sure that the target AMD GPU is recognized as "GPU0" in ```vulkaninfo```.

```Shell
# Check GPU recognition status with OpenCL
clinfo

# Check GPU recognition status with Vulkan
vulkaninfo --summary
```

#### 3-2. When using VA-API

Check that the GPU is recognized by VA-API with ```vainfo```. Codecs with a `VAEntrypointEncSlice` line can be used for HW encoding. If you have multiple GPUs, change the number of `/dev/dri/renderD128` to check the target Radeon.

```Shell
vainfo --display drm --device /dev/dri/renderD128
```

If you installed OpenCL (rusticl), check that the target Radeon is shown in the `rusticl` platform of ```clinfo```. When checking with `clinfo` alone, specify `RUSTICL_ENABLE=radeonsi`.

```Shell
RUSTICL_ENABLE=radeonsi clinfo -l
```

### 4. Install VCEEncC

Download the deb package of VCEEncC (`vceencc_x.xx_amd64.deb`) from [this link](https://github.com/rigaya/VCEEnc/releases).

Then install it as below. Replace "x.xx" with the version to install.

```Shell
sudo apt install ./vceencc_x.xx_amd64.deb
```

The VCEEncC package does not depend on AMF. To use AMF, install AMF following "1-1. When using AMF".

### 5. Check VCEEncC hardware support

Check whether the encoder can actually be used from VCEEncC.

```Shell
vceencc --check-hw
```

- When AMF is installed, the result with AMF is shown. If `Supported Codecs` lists H.264/HEVC, AMF initialization succeeded.
- When AMF is not available, it switches to VA-API and shows the information of the VA-API devices.

Details of the encode / decode features available with VA-API can be checked with the command below.

```Shell
vceencc --backend vaapi --check-features
```

If the GPU cannot be opened, `Permission denied` and a message suggesting adding the user to the `render` group are shown. Please check "2. Add user to the groups below".

### 6. Additional Tools

The following tools are required for some features.

- Reading avs files  
  [AvisynthPlus](https://github.com/AviSynth/AviSynthPlus) is required.
  
- Reading vpy files  
  [VapourSynth](https://www.vapoursynth.com/) is required.
