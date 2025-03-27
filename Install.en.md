
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

  > [!NOTE]
  > Device selection on Linux [is not available](https://github.com/GPUOpen-LibrariesAndSDKs/AMF/issues/194).

### 1. Install AMD Graphics driver  

Download AMD Graphics driver packages for Ubuntu 24.04 from [the webpage of AMD](https://support.amd.com/en-us/download).

Extract and run amdgpu-install to install the driver.

```Shell
cd ~/Downloads
sudo apt-get install ./amdgpu-install-VERSION.deb
sudo apt-get update
sudo amdgpu-install -y --accept-eula --usecase=graphics,amf,opencl --opencl=rocr --vulkan=amdvlk --no-32
sudo reboot
```

### 2. Add user to proper group to use OpenCL
```Shell
# OpenCL
sudo gpasswd -a ${USER} render
```

### 3. Check GPU Recognition Status

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

### 4. Install VCEEncC
Download deb package from [this link](https://github.com/rigaya/VCEEnc/releases), and install running the following command line. Please note "x.xx" should be replaced to the target version name.

```Shell
sudo apt install ./VCEEncC_x.xx_Ubuntu24.04_amd64.deb
```

### 5. Addtional Tools

There are some features which require additional installations.  

| Feature | Requirements |
|:--      |:--           |
| avs reader       | [AvisynthPlus](https://github.com/AviSynth/AviSynthPlus) |
| vpy reader       | [VapourSynth](https://www.vapoursynth.com/)              |

### 6. Others

- Error: "Failed to load OpenCL." when running VCEEncC  
  Please check if /lib/x86_64-linux-gnu/libOpenCL.so exists. There are some cases that only libOpenCL.so.1 exists. In that case, please create a link using following command line.
  
  ```Shell
  sudo ln -s /lib/x86_64-linux-gnu/libOpenCL.so.1 /lib/x86_64-linux-gnu/libOpenCL.so
  ```

