# VCEEnc

AMDのGPU/APUに搭載されているHWエンコーダ(VCE/VCN)をAMF SDKを介して呼び出す。単体で動作するコマンドライン版とAviUtl/AviUtl2出力プラグイン版がある。

QSVEnc / NVEnc / VCEEnc / rkmppenc と共通化できる部分は共通ファイルで同じ実装を用い、そのうえに固有の実装を重ねている。

## ディレクトリ/ファイル構成
  
- `VCECore`
  VCEEncのコア実装。Windows/Linux両対応。

  - 共通ファイル
    - 以下は QSVEnc / NVEnc / VCEEnc / rkmppenc で共通使用する。
      - afs*[.h/.cpp]
      - cl_func[.h/.cpp]
      - convert_*[.h/.cpp]
      - cpu_info[.h/.cpp]
      - gpu_info[.h/.cpp]
      - logo[.h/.cpp]
      - rgy_*[.h/.cpp]
    - 注意点
      - QSVEnc / VCEEnc / VCEEnc / rkmppenc での共通性を維持する
      - 共通化が難しい場合は、部分的なら`rgy_version.h`のマクロ(`ENCODER_QSV`, `ENCODER_NVENC`, `ENCODER_VCEENC`, `ENCODER_MPP`)で切り替える。部分的ですまない場合は、固有実装のほうで実装する。

  - 二層構造により部分的な共通化を行う。以下に2つの例を挙げる。
    - パラメータ類
      - vce_prm[.h/.cpp]
        VCEEnc固有のパラメータ (固有実装)
      - rgy_prm[.h/.cpp]
        エンコーダ共有パラメータ
    - コマンドライン
      - vce_cmd[.h/.cpp]
        VCEEnc固有のパラメータ (固有実装)
      - rgy_cmd[.h/.cpp]
        エンコーダ共有パラメータ

- `VCEEncC`
  `VCECore`の実装を使用したCLI。Windows/Linux両対応。

- `VCEEnc`
  AviUtl/AviUtl2用プラグイン。`VCEEncC`を呼び出してエンコードする。
  Win32ビルドはAviUtl向け(.auo)、x64ビルドはAviUtl2向け(.auo2)。

- `build_pkg`
  Linuxパッケージ作成用。

- `data`
  ドキュメント用のデータ。

- `docker`
  ビルド用のベースdockerfile。

- `GPUFeatures`
  `VCEEncC --check-features`の結果集。適宜追加。Readme.mdから参照
  
- `resource`
  ビルド用のデータ。

- 以下は依存ライブラリ。基本触らない。
  - `AMF`
  - `ffmpeg_lgpl` (Windowsでのみ使用)
  - `PerfMonitor`
  - `cppcodec`
  - `dtl`
  - `jitify`
  - `json`
  - `tinyxml2`
  - `ttmath`

## ビルド構成

ビルド方法は `Build.ja.md` を参照。

- Windows

  MSBuildで`VCEEnc.sln`をビルドする。

  |              |Debug用構成|Release用構成|
  |:---------------------|:------|:--------|
  |VCEEncC(64).exe | DebugStatic | RelStatic |
  |VCEEnc.[auo,auo2]  | Debug | Release |

- Linux

  AviUtlプラグイン、VCEEncCoreの拡張モジュール類は非対象。

  `./configure` → `make` する。

## ドキュメント

- VCEEncC_Options[.md/.ja.md/.cn.md]

  コマンドラインオプションについての記載。`rgy_cmd.cpp`のヘルプともに、オプションを追加したら更新すること。

- Readme[.md/.ja.md/.cn.md]
- Build[.md/.ja.md/.cn.md]
- Install[.md/.ja.md/.cn.md]
