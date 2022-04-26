
# How to install VCEEncC

- [Windows 10](./Install.en.md#windows)
- Linux
  - [Linux (Ubuntu 20.04)](./Install.en.md#linux-ubuntu-2004)
  - [Linux (CentOS 8)](./Install.en.md#linux-centos-8)
  - Other Linux OS  
    For other Linux OS, building from source will be needed. Please check the [build instrcutions](./Build.en.md).


## Windows 10

### 1. Install AMD Radeon Graphics driver
### 2. Download Windows binary  
Windows binary can be found from [this link](https://github.com/rigaya/VCEEnc/releases). VCEEncC_x.xx_Win32.7z contains 32bit exe file, VCEEncC_x.xx_x64.7z contains 64bit exe file.

VCEEncC could be run directly from the extracted directory.
  
## Linux (Ubuntu 20.04)

### 1. Install AMD Graphics driver  
Download AMD Graphics driver packages for Ubuntu 20.04 from [the webpage of AMD](https://support.amd.com/en-us/download).

Extract and run amdgpu-pro-install to install the driver.

```Shell
cd ~/Downloads
tar -xf amdgpu-pro-*.tar.xz
cd amdgpu-pro-*
sudo ./amdgpu-pro-install --pro --opencl=rocr,legacy --no-32
```

### 2. Add user to proper group to use OpenCL
```Shell
# OpenCL
sudo gpasswd -a ${USER} render
```

### 3. Install VCEEncc
Download deb package from [this link](https://github.com/rigaya/VCEEnc/releases), and install running the following command line. Please note "x.xx" should be replaced to the target version name.

```Shell
sudo apt install ./VCEEncc_x.xx_Ubuntu20.04_amd64.deb
```

### 4. Addtional Tools

There are some features which require additional installations.  

| Feature | Requirements |
|:--      |:--           |
| avs reader       | [AvisynthPlus](https://github.com/AviSynth/AviSynthPlus) |
| vpy reader       | [VapourSynth](https://www.vapoursynth.com/)              |

### 5. Others

- Error: "Failed to load OpenCL." when running VCEEncc  
  Please check if /lib/x86_64-linux-gnu/libOpenCL.so exists. There are some cases that only libOpenCL.so.1 exists. In that case, please create a link using following command line.
  
  ```Shell
  sudo ln -s /lib/x86_64-linux-gnu/libOpenCL.so.1 /lib/x86_64-linux-gnu/libOpenCL.so
  ```


## Linux (CentOS 8)

### 1. Install AMD Graphics driver  
Download AMD Graphics driver packages for CentOS 8 from [the webpage of AMD](https://support.amd.com/en-us/download).

Extract and run amdgpu-pro-install to install the driver.

```Shell
cd ~/Downloads
tar -xf amdgpu-pro-*.tar.xz
cd amdgpu-pro-*
sudo ./amdgpu-pro-install --pro --opencl=rocr,legacy --no-32
```

### 2. Add user to proper group to use OpenCL
```Shell
# OpenCL
sudo gpasswd -a ${USER} render
```

### 3. Install VCEEncc
Download rpm package from [this link](https://github.com/rigaya/VCEEnc/releases), and install running the following command line. Please note "x.xx" should be replaced to the target version name.

```Shell
sudo dnf install ./VCEEncc_x.xx_1.x86_64.rpm
```

### 4. Addtional Tools

There are some features which require additional installations.  

| Feature | Requirements |
|:--      |:--           |
| avs reader       | [AvisynthPlus](https://github.com/AviSynth/AviSynthPlus) |
| vpy reader       | [VapourSynth](https://www.vapoursynth.com/)              |

### 5. Others

- Error: "Failed to load OpenCL." when running VCEEncc  
  Please check if /lib/x86_64-linux-gnu/libOpenCL.so exists. There are some cases that only libOpenCL.so.1 exists. In that case, please create a link using following command line.
  
  ```Shell
  sudo ln -s /lib/x86_64-linux-gnu/libOpenCL.so.1 /lib/x86_64-linux-gnu/libOpenCL.so
  ```