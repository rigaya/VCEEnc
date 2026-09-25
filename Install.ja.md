
# VCEEncCのインストール方法

- [Windows](./Install.ja.md#windows)
- Linux
  - [Linux (Ubuntu 24.04以降)](./Install.ja.md#linux)
  - その他のLinux OS  
    その他のLinux OS向けには、ソースコードからビルドする必要があります。ビルド方法については、[こちら](./Build.ja.md)を参照してください。


## Windows 

### 1. AMD Radeonグラフィックスドライバをインストールします。
### 2. Windows用実行ファイルをダウンロードして展開します。  
実行ファイルは[こちら](https://github.com/rigaya/VCEEnc/releases)からダウンロードできます。VCEEncC_x.xx_Win32.7z が 32bit版、VCEEncC_x.xx_x64.7z が 64bit版です。通常は、64bit版を使用します。

実行時は展開したフォルダからそのまま実行できます。

64bit版の配布archiveには`libvmaf.dll`とNVIDIA backend版の`libvship.dll`が含まれます。VMAF評価はCPUで実行できますが、同梱のlibvshipによる評価にはNVIDIA GPUと対応ドライバが必要です。評価を使用しない通常のエンコードにはこれらのDLLは不要です。

## Linux

Linuxでは、HWエンコーダを使う方法を、AMFとVA-APIの2種類から選べます。

| 方法 | AMF | VA-API |
|:--|:--:|:--:|
| 対応GPU | RDNA (Radeon RX 5000シリーズ) 以降、Ryzen 5000シリーズ以降のAPU ※1 | GCN世代以降のRadeon ※2 |
| インストール | やや面倒 | 容易 |
| HWエンコーダの詳細設定 | 可 | 基本的な設定のみ ※3 |
| HWデコード (`--avhw`) | 可 | 可 (遅め) ※4 |
| 処理速度 | 高 | 中 |

※1 AMFのLinux向けのサポート表 ([AMF Wiki](https://github.com/GPUOpen-LibrariesAndSDKs/AMF/wiki/Driver%20Linux)) に載っているGPUです。Polaris・Vega世代のdGPUや、Ryzen 4000シリーズ以前のAPUは対象外です。  
※2 Mesa (radeonsi) のVA-APIを使います。使えるコーデックはGPUの世代によって異なります。H.264はGCN世代以降、HEVC 8bitはPolaris (RX 400/500) 以降、HEVC 10bitはRDNA (RX 5000) ・Ryzen 4000シリーズのAPU以降、AV1はRDNA3 (RX 7000) 以降が目安です。動作確認済みのGPUは、RX 550 (Polaris)、Radeon Vega 8 (Ryzen 3 3200G、Picasso)、RX 9060 XT (RDNA4) です。  
※3 レート制御 (CQP/CBR/VBR)、GOP長、プリセットなどの基本的な設定に対応します。AMF固有の詳細設定は、警告を出して無視します。Bフレーム数や参照フレーム数は、GPUの能力に合わせて制限されます。  
※4 VA-APIでは、HWデコードは `--avhw` を明示したときだけ使います。デコードした映像をいったんCPU側に転送するため、多くの場合、ソフトウェアデコード (`--avsw`、既定) の方が高速です。

AMF非対応のGPUでは、VA-APIを選んでください。VCEEncCは、AMFがインストールされていればAMFを、なければ自動でVA-APIを使います (`--backend auto`、既定)。AMFがインストールされている環境でVA-APIを使いたい場合は、`--backend vaapi` を指定します。

### 1. 事前準備

#### 1-1. AMFを使用する場合

> [!WARNING]
> Ubuntu 24.04 + RADV環境では、AMFの最新userspaceを入れると `Pal::IPlatform::EnumerateDevices()` や `luid not found in devices returned by Pal::IPlatform::EnumerateDevices()` といったエラーでエンコーダ初期化に失敗することがあります。  
> 関連情報: [AMF issue #575](https://github.com/GPUOpen-LibrariesAndSDKs/AMF/issues/575), [workaround comment](https://github.com/GPUOpen-LibrariesAndSDKs/AMF/issues/575#issuecomment-4042920061)  
> All-Openスタックはそのまま使い、AMF userspace (`amf-amdgpu-pro`, `libamdenc-amdgpu-pro`) だけを 6.4.4 / 25.10 系に差し替えることで、問題を回避できることを確認しています。

##### 1-1-1. AMD ドライバのインストール

[AMDのWebページ](https://www.amd.com/ja/support)からUbuntu 24.04向けのドライバをダウンロードします。

その後、パッケージを展開し、All-Openスタックをインストールします。`--opencl=rocr` により、OpenCLフィルタで使うROCmのOpenCLも同時にインストールされます。

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

##### 1-1-2. [回避策] AMF userspaceを6.4.4 / 25.10系に差し替える

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

#### 1-2. VA-APIを使用する場合

AMDのドライバの追加インストールは不要です。Ubuntu標準のMesaのVA-APIドライバを使います。

```Shell
# VA-APIドライバと、確認用のvainfo
sudo apt install mesa-va-drivers vainfo
```

Ubuntu 26.04以降では、VA-APIドライバは `mesa-libgallium` に統合されているため、`mesa-va-drivers` の代わりに `mesa-libgallium` をインストールします (通常はインストール済みです)。

OpenCLフィルタ (`--vpp-*`) や、入力の途中での解像度変更への対応には、OpenCLが必要です。VA-APIでは、MesaのOpenCL (rusticl) を使います。OpenCLを使わない場合は、この手順は不要です。

```Shell
# MesaのOpenCL (rusticl) と、確認用のclinfo
sudo apt install mesa-opencl-icd clinfo
```

rusticlは、既定ではRadeonが無効になっていますが、VCEEncCは実行時に自動で有効にします (環境変数 `RUSTICL_ENABLE` が未設定の場合、`RUSTICL_ENABLE=radeonsi` を設定します)。

### 2. ユーザーを下記グループに追加

GPUを使うには、ユーザーが `render` と `video` のグループに入っている必要があります。AMF・VA-APIのどちらでも必要です。

```Shell
sudo gpasswd -a ${USER} render
sudo gpasswd -a ${USER} video
sudo reboot
```

### 3. GPUの認識状況を確認

#### 3-1. AMFを使用する場合

```Shell
sudo apt install vulkan-tools clinfo
```

`clinfo` で `Number of platforms 0` と表示される場合は、ROCm OpenCL のICDとライブラリ検索パスを登録します。`--opencl=rocr` のインストールが完了していることを確認してから、下記を実行してください。

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

```clinfo``` と ```vulkaninfo``` でGPUの認識状況を確認します。特に ```vulkaninfo``` では、対象のAMD GPUが "GPU0" として認識されていることを確認してください。

```Shell
# OpenCLでのGPUの認識状況
clinfo

# VulkanでのGPUの認識状況
vulkaninfo --summary
```

#### 3-2. VA-APIを使用する場合

```vainfo``` で、VA-APIでGPUが認識されていることを確認します。`VAEntrypointEncSlice` の行があるコーデックが、HWエンコードに使えるコーデックです。GPUが複数ある場合は、`/dev/dri/renderD128` の番号を変えて、対象のRadeonを確認してください。

```Shell
vainfo --display drm --device /dev/dri/renderD128
```

OpenCL (rusticl) をインストールした場合は、```clinfo``` で `rusticl` のplatformに対象のRadeonが表示されることを確認します。`clinfo` 単体で確認するときは、`RUSTICL_ENABLE=radeonsi` を指定してください。

```Shell
RUSTICL_ENABLE=radeonsi clinfo -l
```

### 4. VCEEncCのインストール

VCEEncCのdebファイル (`vceencc_x.xx_amd64.deb`) を[こちら](https://github.com/rigaya/VCEEnc/releases)からダウンロードします。

その後、下記のようにインストールします。"x.xx"はインストールするバージョンに置き換えてください。

```Shell
sudo apt install ./vceencc_x.xx_amd64.deb
```

VCEEncCのパッケージは、AMFには依存しません。AMFを使う場合は、「1-1. AMFを使用する場合」の手順でAMFをインストールしてください。

### 5. VCEEncCでの認識状況を確認

VCEEncCで、実際にエンコーダが使えるか確認します。

```Shell
vceencc --check-hw
```

- AMFがインストールされている場合は、AMFでの確認結果を表示します。`Supported Codecs` に H.264/HEVC が表示されれば、AMFの初期化は成功しています。
- AMFがない場合は、VA-APIに切り替えて、VA-APIのデバイスの情報を表示します。

VA-APIで使えるエンコード・デコードの機能の詳細は、下記で確認できます。

```Shell
vceencc --backend vaapi --check-features
```

GPUを開けない場合は、`Permission denied` と、`render` グループに追加するよう促すメッセージを表示します。「2. ユーザーを下記グループに追加」を確認してください。

### 6. 追加オプション
下記機能を使用するには、追加でインストールが必要です。

- avs読み込み  
  [AvisynthPlus](https://github.com/AviSynth/AviSynthPlus)のインストールが必要です。
  
- vpy読み込み  
  [VapourSynth](https://www.vapoursynth.com/)のインストールが必要です。
