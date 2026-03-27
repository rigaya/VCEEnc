
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
  > Ubuntu 24.04 + RADV環境では、AMFの最新userspaceを入れると `Pal::IPlatform::EnumerateDevices()` や `luid not found in devices returned by Pal::IPlatform::EnumerateDevices()` といったエラーでエンコーダ初期化に失敗することがあります。  
  > 関連情報: [AMF issue #575](https://github.com/GPUOpen-LibrariesAndSDKs/AMF/issues/575), [workaround comment](https://github.com/GPUOpen-LibrariesAndSDKs/AMF/issues/575#issuecomment-4042920061)  
  > All-Openスタックはそのまま使い、AMF userspace (`amf-amdgpu-pro`, `libamdenc-amdgpu-pro`) だけを 6.4.4 / 25.10 系に差し替えることで、問題を回避できることを確認しています。

### 1. AMD ドライバのインストール  
[AMDのWebページ](https://www.amd.com/ja/support)からUbuntu 24.04向けのドライバをダウンロードします。

その後、パッケージを展開し、All-Openスタックをインストールします。

<!--
本来の最新AMF userspaceの導入方法 (AMF Wikiの現行案内):

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

### 2. [回避策] AMF userspaceを6.4.4 / 25.10系に差し替える

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

### 3. OpenCLの使用のため、ユーザーを下記グループに追加
```Shell
# OpenCL
sudo gpasswd -a ${USER} render
sudo gpasswd -a ${USER} video
sudo reboot
```

### 4. GPUの認識状況を確認

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

### 5. VCEEncCのインストール
VCEEncCのdebファイルを[こちら](https://github.com/rigaya/VCEEnc/releases)からダウンロードします。

その後、下記のようにインストールします。"x.xx"はインストールするバージョンに置き換えてください。

```Shell
sudo apt install ./VCEEncC_x.xx_Ubuntu24.04_amd64.deb
```

### 6. VCEEncCでの認識状況を確認

VCEEncCで実際にエンコーダが使えるか確認します。`Supported Codecs` に H.264/HEVC が表示されれば、AMF初期化は成功しています。

```Shell
vceencc --check-hw
```

### 7. 追加オプション
下記機能を使用するには、追加でインストールが必要です。

- avs読み込み  
  [AvisynthPlus](https://github.com/AviSynth/AviSynthPlus)のインストールが必要です。
  
- vpy読み込み  
  [VapourSynth](https://www.vapoursynth.com/)のインストールが必要です。

### 8. その他

- VCEEncc実行時に、"Failed to load OpenCL." というエラーが出る場合  
  /lib/x86_64-linux-gnu/libOpenCL.so が存在することを確認してください。 libOpenCL.so.1 しかない場合は、下記のようにシンボリックリンクを作成してください。
  
  ```Shell
  sudo ln -s /lib/x86_64-linux-gnu/libOpenCL.so.1 /lib/x86_64-linux-gnu/libOpenCL.so
  ```
