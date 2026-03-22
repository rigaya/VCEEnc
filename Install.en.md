
# How to install VCEEncC

- [Windows](./Install.en.md#windows)
- Linux
  - [Linux (Ubuntu 24.04)](./Install.en.md#linux-ubuntu-2004)
  - Other Linux OS  
    For other Linux OS, building from source will be needed. Please check the [build instrcutions](./Build.en.md).


## Windows

### 1. Install AMD Radeon Graphics driver
### 2. Download Windows binary  
Windows binary can be found from [this link](https://github.com/rigaya/VCEEnc/releases). VCEEncC_x.xx_Win32.7z contains 32bit exe file, VCEEncC_x.xx_x64.7z contains 64bit exe file.

VCEEncC could be run directly from the extracted directory.
  
## Linux (Ubuntu 24.04)

  > [!WARNING]
  > On some Ubuntu 24.04 + RADV environments, installing the latest AMF userspace causes encoder initialization failures such as `Pal::IPlatform::EnumerateDevices()` or `luid not found in devices returned by Pal::IPlatform::EnumerateDevices()`.  
  > Related information: [AMF issue #575](https://github.com/GPUOpen-LibrariesAndSDKs/AMF/issues/575), [workaround comment](https://github.com/GPUOpen-LibrariesAndSDKs/AMF/issues/575#issuecomment-4042920061)  
  > By replacing only the AMF userspace packages (`amf-amdgpu-pro`, `libamdenc-amdgpu-pro`) with the 6.4.4 / 25.10 generation seems to workaround this issue.

### 1. Install AMD Graphics driver  

Download AMD Graphics driver packages for Ubuntu 24.04 from [the webpage of AMD](https://support.amd.com/en-us/download).

Extract and run `amdgpu-install` to install the All-Open stack.

<!--
Original/latest AMF userspace installation flow from the AMF Wiki:

```Shell
cd ~/Downloads
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
cd ~/Downloads
sudo apt-get install ./amdgpu-install-VERSION.deb
sudo apt-get update
sudo amdgpu-install -y --opencl=rocr
```

### 2. [Workaround] Replace AMF userspace with the 6.4.4 / 25.10 generation

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

### 3. Add user to proper group to use OpenCL
```Shell
# OpenCL
sudo gpasswd -a ${USER} render
sudo gpasswd -a ${USER} video
sudo reboot
```

### 4. Check GPU Recognition Status

Check if your GPU is properly recognized.

```Shell
sudo apt install vulkan-utils clinfo
```

Check GPU recognition status using ```clinfo``` and ```vulkaninfo```. Especially with ```vulkaninfo```, confirm that your target AMD GPU is recognized as "GPU0".

```Shell
# Check GPU recognition status with OpenCL
clinfo

# Check GPU recognition status with Vulkan
vulkaninfo --summary
```

### 5. Install VCEEncC
Download deb package from [this link](https://github.com/rigaya/VCEEnc/releases), and install running the following command line. Please note "x.xx" should be replaced to the target version name.

```Shell
sudo apt install ./VCEEncC_x.xx_Ubuntu24.04_amd64.deb
```

### 6. Check VCEEncC hardware support

Verify that the encoder is actually usable from VCEEncC. If `Supported Codecs` lists H.264/HEVC, AMF initialization succeeded.

```Shell
vceencc --check-hw
```

### 7. Addtional Tools

There are some features which require additional installations.  

| Feature | Requirements |
|:--      |:--           |
| avs reader       | [AvisynthPlus](https://github.com/AviSynth/AviSynthPlus) |
| vpy reader       | [VapourSynth](https://www.vapoursynth.com/)              |

### 8. Others

- Error: "Failed to load OpenCL." when running VCEEncC  
  Please check if /lib/x86_64-linux-gnu/libOpenCL.so exists. There are some cases that only libOpenCL.so.1 exists. In that case, please create a link using following command line.
  
  ```Shell
  sudo ln -s /lib/x86_64-linux-gnu/libOpenCL.so.1 /lib/x86_64-linux-gnu/libOpenCL.so
  ```
