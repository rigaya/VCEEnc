
# VCEEncのビルド方法

- [Windows](./Build.ja.md#windows)
- Linux
  - [Linux (Ubuntu 24.04)](./Build.ja.md#linux-ubuntu-2404)

## Windows 

### 0. 準備
ビルドには、下記のものが必要です。

- Visual Studio 2022
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

`--vpp-onnx` を使用するには、DirectML対応版の ONNX Runtime が必要です。
[Microsoft.ML.OnnxRuntime.DirectML](https://www.nuget.org/packages/Microsoft.ML.OnnxRuntime.DirectML) の NuGet パッケージを展開し、展開先を環境変数 `ONNXRUNTIME_DIR` に設定してください。
Visual Studio プロジェクトは `$(ONNXRUNTIME_DIR)\include` を参照します。
下記は `F:\VisualStudio2022\Projects\onnxruntime` に配置する例です。

```Batchfile
set ONNXRUNTIME_DIR=F:\VisualStudio2022\Projects\onnxruntime
if not exist "%ONNXRUNTIME_DIR%" mkdir "%ONNXRUNTIME_DIR%"
if not exist "%ONNXRUNTIME_DIR%\include" mkdir "%ONNXRUNTIME_DIR%\include"
if not exist "%ONNXRUNTIME_DIR%\lib" mkdir "%ONNXRUNTIME_DIR%\lib"

curl -L -o onnxruntime-directml.nupkg https://api.nuget.org/v3-flatcontainer/microsoft.ml.onnxruntime.directml/1.23.0/microsoft.ml.onnxruntime.directml.1.23.0.nupkg
7z x -oonnxruntime-directml -y onnxruntime-directml.nupkg

xcopy /e /i /y onnxruntime-directml\build\native\include "%ONNXRUNTIME_DIR%\include"
copy /y onnxruntime-directml\runtimes\win-x64\native\onnxruntime.dll "%ONNXRUNTIME_DIR%\lib\"
copy /y onnxruntime-directml\runtimes\win-x64\native\onnxruntime.lib "%ONNXRUNTIME_DIR%\lib\"
copy /y onnxruntime-directml\runtimes\win-x64\native\onnxruntime_providers_shared.dll "%ONNXRUNTIME_DIR%\lib\"

setx ONNXRUNTIME_DIR "%ONNXRUNTIME_DIR%"
```

実行時には `%ONNXRUNTIME_DIR%\lib` を `PATH` に追加してください。
Windows のシステムディレクトリに別の `onnxruntime.dll` が存在することがあるため、DirectML対応版の `onnxruntime.dll` が先に見つかるように設定してください。

### 1. ソースのダウンロード

```Batchfile
git clone https://github.com/rigaya/VCEEnc --recursive
cd VCEEnc
curl -s -o ffmpeg_lgpl.7z -L https://github.com/rigaya/ffmpeg_dlls_for_hwenc/releases/download/20250830/ffmpeg_dlls_for_hwenc_20250830.7z
7z x -offmpeg_lgpl -y ffmpeg_lgpl.7z
```

### 2. VCEEnc.auo / VCEEncC のビルド

VCEEnc.slnを開きます。

ビルドしたいものに合わせて、構成を選択してください。

|              |Debug用構成|Release用構成|
|:---------------------|:------|:--------|
|VCEEnc.auo (win32のみ) | Debug | Release |
|VCEEncC(64).exe | DebugStatic | RelStatic |


## Linux (Ubuntu 24.04)

### 0. ビルドに必要なもの

- C++17 Compiler
- meson + ninja-build
- AMD Radeonグラフィックスドライバ
- git
- libraries
  - OpenCL / Vulkan / X11 headers
  - ffmpeg libs (libavcodec*, libavformat*, libavfilter*, libavutil*, libswresample*, libavdevice*)
  - libass9
  - [Optional] VapourSynth

### 1. コンパイラ等のインストール

```Shell
sudo apt install build-essential libtool git meson ninja-build
```

- rust + cargo-cのインストール (libdovi, libhdr10plusビルド用)

  ```Shell
  sudo apt install libssl-dev curl pkgconf
  curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y --profile minimal \
    && . ~/.cargo/env \
    && cargo install cargo-c
  ```

### 2. AMD ドライバのインストール

[AMDのWebページ](https://www.amd.com/ja/support)からUbuntu 24.04向けのドライバをダウンロードします。

その後、パッケージを展開し、All-Openスタックをインストールします。

  > [!WARNING]
  > Ubuntu 24.04 + RADV環境では、AMFの最新userspaceを入れると `Pal::IPlatform::EnumerateDevices()` や `luid not found in devices returned by Pal::IPlatform::EnumerateDevices()` といったエラーでエンコーダ初期化に失敗することがあります。
  > All-Openスタックはそのまま使い、AMF userspaceだけを 6.4.4 / 25.10 系に差し替えると回避できる場合があります。

```Shell
cd ~/Downloads
sudo apt-get install ./amdgpu-install-VERSION.deb
sudo apt-get update
sudo amdgpu-install -y --opencl=rocr
```

### 3. [回避策] AMF userspaceを6.4.4 / 25.10系に差し替える

最新AMF userspaceの代わりに、6.4.4 / 25.10系の `amdgpu-pro-core`, `libamdenc-amdgpu-pro`, `amf-amdgpu-pro` をインストールします。

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

### 4. ビルドに必要なライブラリのインストール

```Shell
sudo apt install \
  opencl-headers \
  libvulkan-dev \
  libx11-dev

sudo apt install ffmpeg \
  libavcodec-extra libavcodec-dev libavutil-dev libavformat-dev libswresample-dev libavfilter-dev libavdevice-dev \
  libass9 libass-dev
```

### 5. OpenCLの使用のため、ユーザーを下記グループに追加して再起動
```Shell
# OpenCL
sudo gpasswd -a ${USER} render
sudo gpasswd -a ${USER} video
sudo reboot
```

### 6. GPUの認識状況を確認

VCEEncCをビルドする前に、GPUが正しく認識されているか確認します。

```Shell
sudo apt install vulkan-utils clinfo

# OpenCLでのGPUの認識状況
clinfo

# VulkanでのGPUの認識状況
vulkaninfo --summary
```

特に `vulkaninfo` では、対象のAMD GPUが `GPU0` として認識されていることを確認してください。

### 7. [オプション] VapourSynthのビルド
VapourSynthのインストールは必須ではありませんが、インストールしておくとvpyを読み込めるようになります。

必要のない場合は 8. VCEEncCのビルド に進んでください。

<details><summary>VapourSynthのビルドの詳細はこちら</summary>

#### 7.1 ビルドに必要なツールのインストール
```Shell
sudo apt install python3-pip autoconf automake libtool meson
```

#### 7.2 zimgのインストール
```Shell
git clone https://github.com/sekrit-twc/zimg.git
cd zimg
./autogen.sh
./configure
sudo make install -j16
cd ..
```

#### 7.3 cythonのインストール
```Shell
sudo pip3 install Cython
```

#### 7.4 VapourSynthのビルド
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

#### 7.5 VapourSynthの動作確認
エラーが出ずにバージョンが表示されればOK。
```Shell
vspipe --version
```

#### 7.6 [おまけ] vslsmashsourceのビルド
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

### 8. VCEEncCのビルド
```Shell
git clone https://github.com/rigaya/VCEEnc --recursive
cd VCEEnc
meson setup ./build --buildtype=release
meson compile -C ./build
```
動作するか確認します。
```Shell
./build/vceencc --check-hw
```
