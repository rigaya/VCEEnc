
# VCEEncのビルド方法

- [Windows](./Build.ja.md#windows)
- Linux
  - [Linux (Ubuntu 20.04)](./Build.ja.md#linux-ubuntu-2004)
  - [Linux (CentOS 8)](./Build.ja.md#linux-centos-8)

## Windows 

### 0. 準備
ビルドには、下記のものが必要です。

- Visual Studio 2019
- [Avisynth](https://github.com/AviSynth/AviSynthPlus) SDK
- [VapourSynth](http://www.vapoursynth.com/) SDK
- Intel OpenCL SDK
- Intel Metric Framework SDK (Intel Platform Analysis Libraryに同梱)

Avisynth+とVapourSynthは、SDKがインストールされるよう設定してインストールします。

Avisynth+ SDKの"avisynth_c.h"とVapourSynth SDKの"VapourSynth.h", "VSScript.h"がVisual Studioのincludeパスに含まれるよう設定します。

includeパスは環境変数 "AVISYNTH_SDK" / "VAPOURSYNTH_SDK" で渡すことができます。

Avisynth+ / VapourSynthインストーラのデフォルトの場所にインストールした場合、下記のように設定することになります。
```Batchfile
setx AVISYNTH_SDK "C:\Program Files (x86)\AviSynth+\FilterSDK"
setx VAPOURSYNTH_SDK "C:\Program Files (x86)\VapourSynth\sdk"
```

ビルドに必要な[OpenCLのヘッダ](https://github.com/KhronosGroup/OpenCL-Headers.git)をcloneし、環境変数 "OPENCL_HEADERS" を設定します。

```Batchfile
git clone https://github.com/KhronosGroup/OpenCL-Headers.git <path-to-clone>
setx OPENCL_HEADERS <path-to-clone>
```

さらにビルドに必要な[Caption2Ass_PCR](https://github.com/maki-rxrz/Caption2Ass_PCR)をcloneし、環境変数 "CAPTION2ASS_SRC" を設定します。

```Batchfile
git clone https://github.com/maki-rxrz/Caption2Ass_PCR <path-to-clone>
setx CAPTION2ASS_SRC <path-to-clone>/src
```

### 1. ソースのダウンロード

```Batchfile
git clone https://github.com/rigaya/VCEEnc --recursive
```

### 2. VCEEnc.auo / VCEEncC のビルド

VCEEnc.slnを開きます。

ビルドしたいものに合わせて、構成を選択してください。

|              |Debug用構成|Release用構成|
|:---------------------|:------|:--------|
|VCEEnc.auo (win32のみ) | Debug | Release |
|VCEEncC(64).exe | DebugStatic | RelStatic |


## Linux (Ubuntu 20.04)

### 0. ビルドに必要なもの

- C++17 Compiler
- Intel Driver
- git
- libraries
  - libva, libdrm, libmfx 
  - ffmpeg 4.x libs (libavcodec58, libavformat58, libavfilter7, libavutil56, libswresample3)
  - libass9
  - [Optional] VapourSynth

### 1. コンパイラ等のインストール

```Shell
sudo apt install build-essential libtool git
```

### 2. Intel ドライバのインストール
[AMDのWebページ](https://www.amd.com/ja/support)からUbuntu 20.04向けのドライバをダウンロードします。

その後、パッケージを展開し、amdgpu-pro-installを下記のように実行してドライバをインストールします。

```Shell
cd ~/Downloads
tar -xf amdgpu-pro-*.tar.xz
cd amdgpu-pro-*
sudo ./amdgpu-pro-install --pro --opencl=rocr,legacy --no-32
```

### 3. ビルドに必要なライブラリのインストール

```Shell
sudo apt install \
  amf-amdgpu-pro \
  opencl-headers \
  libvulkan-dev \
  libx11-dev

sudo apt install ffmpeg \
  libavcodec-extra libavcodec-dev libavutil-dev libavformat-dev libswresample-dev libavfilter-dev \
  libass9 libass-dev
```

### 4. [オプション] VapourSynthのビルド
VapourSynthのインストールは必須ではありませんが、インストールしておくとvpyを読み込めるようになります。

必要のない場合は 5. VCEEncCのビルド に進んでください。

<details><summary>VapourSynthのビルドの詳細はこちら</summary>

#### 4.1 ビルドに必要なツールのインストール
```Shell
sudo apt install python3-pip autoconf automake libtool meson
```

#### 4.2 zimgのインストール
```Shell
git clone https://github.com/sekrit-twc/zimg.git
cd zimg
./autogen.sh
./configure
sudo make install -j16
cd ..
```

#### 4.3 cythonのインストール
```Shell
sudo pip3 install Cython
```

#### 4.4 VapourSynthのビルド
```Shell
git clone https://github.com/vapoursynth/vapoursynth.git
cd vapoursynth
./autogen.sh
./configure
make -j16
sudo make install

# vapoursynthが自動的にロードされるようにする
# "python3.x" は環境に応じて変えてください。これを書いた時点ではpython3.7でした
sudo ln -s /usr/local/lib/python3.x/site-packages/vapoursynth.so /usr/lib/python3.x/lib-dynload/vapoursynth.so
sudo ldconfig
```

#### 4.5 VapourSynthの動作確認
エラーが出ずにバージョンが表示されればOK。
```Shell
vspipe --version
```

#### 4.6 [おまけ] vslsmashsourceのビルド
```Shell
# lsmashのビルド
git clone https://github.com/l-smash/l-smash.git
cd l-smash
./configure --enable-shared
sudo make install -j16
cd ..
 
# vslsmashsourceのビルド
git clone https://github.com/HolyWu/L-SMASH-Works.git
# ffmpegのバージョンが合わないので、下記バージョンを取得する
cd L-SMASH-Works
git checkout -b 20200531 refs/tags/20200531
cd VapourSynth
meson build
cd build
sudo ninja install
cd ../../../
```

</details>

### 5. OpenCLの使用のため、ユーザーを下記グループに追加
```Shell
# OpenCL
sudo gpasswd -a ${USER} render
```

### 5. VCEEncCのビルド
```Shell
git clone https://github.com/rigaya/VCEEnc --recursive
cd VCEEnc
./configure
make -j8
```
動作するか確認します。
```Shell
./vceencc --check-hw
```

## Linux (CentOS 8)

### 0. ビルドに必要なもの

- C++17 Compiler
- Intel Driver
- git
- libraries
  - libva, libdrm, libmfx 
  - ffmpeg 4.x libs (libavcodec58, libavformat58, libavfilter7, libavutil56, libswresample3)
  - libass9
  - [Optional] VapourSynth

### 1. コンパイラ等のインストール

```Shell
sudo dnf install @development-tools
```

### 2. AMD ドライバのインストール

[AMDのWebページ](https://www.amd.com/ja/support)からCentOS 8向けのドライバをダウンロードします。

その後、パッケージを展開し、amdgpu-pro-installを下記のように実行してドライバをインストールします。

```Shell
cd ~/Downloads
tar -xf amdgpu-pro-*.tar.xz
cd amdgpu-pro-*
sudo ./amdgpu-pro-install --pro --opencl=rocr,legacy --no-32
```

### 3. ビルドに必要なライブラリのインストール

```Shell
sudo dnf install 'dnf-command(config-manager)'
sudo dnf config-manager --set-enabled powertools
sudo dnf install https://download.fedoraproject.org/pub/epel/epel-release-latest-8.noarch.rpm
sudo dnf localinstall --nogpgcheck https://download1.rpmfusion.org/free/el/rpmfusion-free-release-8.noarch.rpm

sudo dnf install opencl-headers libX11-devel vulkan-loader-devel
sudo dnf install ffmpeg ffmpeg-devel
```


### 4. [オプション] VapourSynthのビルド
VapourSynthのインストールは必須ではありませんが、インストールしておくとvpyを読み込めるようになります。

必要のない場合は 5. VCEEncCのビルド に進んでください。

<details><summary>VapourSynthのビルドの詳細はこちら</summary>

#### 4.1 ビルドに必要なツールのインストール
```Shell
sudo apt install python3-pip autoconf automake libtool meson
```

#### 4.2 zimgのインストール
```Shell
git clone https://github.com/sekrit-twc/zimg.git
cd zimg
./autogen.sh
./configure
sudo make install -j16
cd ..
```

#### 4.3 cythonのインストール
```Shell
sudo pip3 install Cython
```

#### 4.4 VapourSynthのビルド
```Shell
git clone https://github.com/vapoursynth/vapoursynth.git
cd vapoursynth
./autogen.sh
./configure
make -j16
sudo make install

# vapoursynthが自動的にロードされるようにする
# "python3.x" は環境に応じて変えてください。これを書いた時点ではpython3.7でした
sudo ln -s /usr/local/lib/python3.x/site-packages/vapoursynth.so /usr/lib/python3.x/lib-dynload/vapoursynth.so
sudo ldconfig
```

#### 4.5 VapourSynthの動作確認
エラーが出ずにバージョンが表示されればOK。
```Shell
vspipe --version
```

#### 4.6 [おまけ] vslsmashsourceのビルド
```Shell
# lsmashのビルド
git clone https://github.com/l-smash/l-smash.git
cd l-smash
./configure --enable-shared
sudo make install -j16
cd ..
 
# vslsmashsourceのビルド
git clone https://github.com/HolyWu/L-SMASH-Works.git
# ffmpegのバージョンが合わないので、下記バージョンを取得する
cd L-SMASH-Works
git checkout -b 20200531 refs/tags/20200531
cd VapourSynth
meson build
cd build
sudo ninja install
cd ../../../
```

</details>

### 5. OpenCLの使用のため、ユーザーを下記グループに追加
```Shell
# OpenCL
sudo gpasswd -a ${USER} render
```

### 6. VCEEncCのビルド
```Shell
git clone https://github.com/rigaya/VCEEnc --recursive
cd VCEEnc
./configure
make -j8
```
動作するか確認します。
```Shell
./vceencc --check-hw
```
