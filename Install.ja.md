
# VCEEncCのインストール方法

- [Windows](./Install.ja.md#windows)
- Linux
  - [Linux (Ubuntu 24.04)](./Install.ja.md#linux-ubuntu-2404)
  - その他のLinux OS  
    その他のLinux OS向けには、ソースコードからビルドする必要があります。ビルド方法については、[こちら](./Build.ja.md)を参照してください。


## Windows 

### 1. AMD Radeonグラフィックスドライバをインストールします。
### 2. Windows用実行ファイルをダウンロードして展開します。  
実行ファイルは[こちら](https://github.com/rigaya/VCEEnc/releases)からダウンロードできます。VCEEncC_x.xx_Win32.7z が 32bit版、VCEEncC_x.xx_x64.7z が 64bit版です。通常は、64bit版を使用します。

実行時は展開したフォルダからそのまま実行できます。
  
## Linux (Ubuntu 24.04)

  > [!WARNING]
  > 対象のAMD GPUが ```vulkaninfo --summary``` において、"GPU0" として認識されている必要があります。Linux環境では[使用するGPUを選択することはできません](https://github.com/GPUOpen-LibrariesAndSDKs/AMF/issues/194)。

### 1. AMD ドライバのインストール  
[AMDのWebページ](https://www.amd.com/ja/support)からUbuntu 24.04向けのドライバをダウンロードします。

その後、パッケージを展開し、amdgpu-installを下記のように実行してドライバをインストールし、再起動します。

```Shell
cd ~/Downloads
sudo apt-get install ./amdgpu-install-VERSION.deb
sudo apt-get update
sudo amdgpu-install -y --accept-eula --usecase=graphics,amf,opencl --opencl=rocr --vulkan=amdvlk --no-32
sudo reboot
```

### 2. OpenCLの使用のため、ユーザーを下記グループに追加
```Shell
# OpenCL
sudo gpasswd -a ${USER} render
```

### 3. GPUの認識状況を確認

GPUの認識状況を確認します。

```Shell
sudo apt install vulkan-utils clinfo
```

```clinfo``` と ```vulkaninfo``` でGPUの認識状況を確認します。特に ```vulkaninfo``` では、対象のAMD GPUが "GPU0" として認識されていることを確認してください。

```Shell
# OpenCLでのGPUの認識状況
clinfo

# VulkanでのGPUの認識状況
vulkaninfo --summary
```

### 4. VCEEncCのインストール
VCEEncCのdebファイルを[こちら](https://github.com/rigaya/VCEEnc/releases)からダウンロードします。

その後、下記のようにインストールします。"x.xx"はインストールするバージョンに置き換えてください。

```Shell
sudo apt install ./VCEEncC_x.xx_Ubuntu24.04_amd64.deb
```

### 5. 追加オプション
下記機能を使用するには、追加でインストールが必要です。

- avs読み込み  
  [AvisynthPlus](https://github.com/AviSynth/AviSynthPlus)のインストールが必要です。
  
- vpy読み込み  
  [VapourSynth](https://www.vapoursynth.com/)のインストールが必要です。

### 6. その他

- VCEEncc実行時に、"Failed to load OpenCL." というエラーが出る場合  
  /lib/x86_64-linux-gnu/libOpenCL.so が存在することを確認してください。 libOpenCL.so.1 しかない場合は、下記のようにシンボリックリンクを作成してください。
  
  ```Shell
  sudo ln -s /lib/x86_64-linux-gnu/libOpenCL.so.1 /lib/x86_64-linux-gnu/libOpenCL.so
  ```
