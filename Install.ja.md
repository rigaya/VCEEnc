
# VCEEncCのインストール方法

- [Windows](./Install.ja.md#windows)
- Linux
  - [Linux (Ubuntu 20.04)](./Install.ja.md#linux-ubuntu-2004)
  - [Linux (CentOS 8)](./Install.ja.md#linux-centos-8)
  - その他のLinux OS  
    その他のLinux OS向けには、ソースコードからビルドする必要があります。ビルド方法については、[こちら](./Build.ja.md)を参照してください。


## Windows 

### 1. AMD Radeonグラフィックスドライバをインストールします。
### 2. Windows用実行ファイルをダウンロードして展開します。  
実行ファイルは[こちら](https://github.com/rigaya/VCEEnc/releases)からダウンロードできます。VCEEncC_x.xx_Win32.7z が 32bit版、VCEEncC_x.xx_x64.7z が 64bit版です。通常は、64bit版を使用します。

実行時は展開したフォルダからそのまま実行できます。
  
## Linux (Ubuntu 20.04)

### 1. AMD ドライバのインストール  
[AMDのWebページ](https://www.amd.com/ja/support)からUbuntu 20.04向けのドライバをダウンロードします。

その後、パッケージを展開し、amdgpu-installを下記のように実行してドライバをインストールし、再起動します。

```Shell
cd ~/Downloads
sudo apt-get install ./amdgpu-install-VERSION.deb
sudo apt-get update
sudo amdgpu-install -y --accept-eula --usecase=graphics,amf,opencl --opencl=rocr,legacy --no-32
sudo reboot
```

### 2. OpenCLの使用のため、ユーザーを下記グループに追加
```Shell
# OpenCL
sudo gpasswd -a ${USER} render
```

### 3. VCEEncCのインストール
VCEEncCのdebファイルを[こちら](https://github.com/rigaya/VCEEnc/releases)からダウンロードします。

その後、下記のようにインストールします。"x.xx"はインストールするバージョンに置き換えてください。

```Shell
sudo apt install ./VCEEncC_x.xx_Ubuntu20.04_amd64.deb
```

### 4. 追加オプション
下記機能を使用するには、追加でインストールが必要です。

- avs読み込み  
  [AvisynthPlus](https://github.com/AviSynth/AviSynthPlus)のインストールが必要です。
  
- vpy読み込み  
  [VapourSynth](https://www.vapoursynth.com/)のインストールが必要です。

### 5. その他

- VCEEncc実行時に、"Failed to load OpenCL." というエラーが出る場合  
  /lib/x86_64-linux-gnu/libOpenCL.so が存在することを確認してください。 libOpenCL.so.1 しかない場合は、下記のようにシンボリックリンクを作成してください。
  
  ```Shell
  sudo ln -s /lib/x86_64-linux-gnu/libOpenCL.so.1 /lib/x86_64-linux-gnu/libOpenCL.so
  ```
  

## Linux (CentOS 8)

### 1. AMD ドライバのインストール  
[AMDのWebページ](https://www.amd.com/ja/support)からCentOS 8向けのドライバをダウンロードします。

その後、パッケージを展開し、amdgpu-installを下記のように実行してドライバをインストールし、再起動します。

```Shell
cd ~/Downloads
sudo yum install ./amdgpu-install-VERSION.rpm
sudo amdgpu-install -y --accept-eula --usecase=graphics,amf,opencl --opencl=rocr,legacy --no-32
sudo reboot
```

### 2. OpenCLの使用のため、ユーザーを下記グループに追加
```Shell
# OpenCL
sudo gpasswd -a ${USER} render
```

### 3. VCEEnccのインストール
VCEEnccのrpmファイルを[こちら](https://github.com/rigaya/VCEEnc/releases)からダウンロードします。

その後、下記のようにインストールします。"x.xx"はインストールするバージョンに置き換えてください。

```Shell
sudo dnf install ./VCEEncc_x.xx_1.x86_64.rpm
```

### 4. 追加オプション
下記機能を使用するには、追加でインストールが必要です。

- avs読み込み  
  [AvisynthPlus](https://github.com/AviSynth/AviSynthPlus)のインストールが必要です。
  
- vpy読み込み  
  [VapourSynth](https://www.vapoursynth.com/)のインストールが必要です。

### 5. その他

- VCEEncc実行時に、"Failed to load OpenCL." というエラーが出る場合  
  /lib/x86_64-linux-gnu/libOpenCL.so が存在することを確認してください。 libOpenCL.so.1 しかない場合は、下記のようにシンボリックリンクを作成してください。
  
  ```Shell
  sudo ln -s /lib/x86_64-linux-gnu/libOpenCL.so.1 /lib/x86_64-linux-gnu/libOpenCL.so
  ```
