---------------------------------------------------


	VCEEnc
	 by rigaya

---------------------------------------------------

VCEEnc は、
AMDのVCE(VideoCodecEngine)を使用してエンコードを行うAviutlの出力プラグインです。
VCEによるハードウェア高速エンコードを目指します。

【基本動作環境】
Windows 10/11 (x86/x64)
Aviutl 1.00 以降
VCEが載ったハードウェア
  AMD製 GPU Radeon HD 7xxx以降
  AMD製 APU Trinity世代(第2世代)以降
AMD Radeon Software Adrenalin Edition 23.1.2 以降


【VCEEnc 使用にあたっての注意事項】
無保証です。自己責任で使用してください。
VCEEncを使用したことによる、いかなる損害・トラブルについても責任を負いません。

【VCEEnc 再配布(二次配布)について】
このファイル(VCEEnc_readme.txt)と一緒に配布してください。念のため。
まあできればアーカイブまるごとで。


【導入方法】
※ 下記リンク先では図も使用して説明していますので、よりわかりやすいかもしれません。
   https://github.com/rigaya/VCEEnc/blob/master/VCEEnc_auo_readme.md#VCEEnc-の-aviutl-への導入更新

1.
ダウンロードしたAviutl_VCEEnc_7.xx.zipを開きます。

2.
zipファイル内のフォルダすべてをAviutlフォルダにコピーします。

3.
Aviutlを起動します。

4.
環境によっては、ウィンドウが表示され必要なモジュールのインストールが行われます。
その際、この不明な発行元のアプリがデバイスに変更を加えることを許可しますか? と出ることがありますが、
「はい」を選択してください。

5.
「その他」>「出力プラグイン情報」にVCEEnc 7.xxがあるか確かめます。
ここでVCEEncの表示がない場合、
- zipファイル内のフォルダすべてをコピーできていない
- 必要なモジュールのインストールに失敗した
  - この不明な発行元のアプリがデバイスに変更を加えることを許可しますか? で 「はい」を選択しなかった
  - (まれなケース) ウイルス対策ソフトにより、必要な実行ファイルが削除された
などの原因が考えられます。


【削除方法】
※ 下記リンク先では図も使用して説明していますので、よりわかりやすいかもしれません。
   https://github.com/rigaya/VCEEnc/blob/master/VCEEnc_auo_readme.md#VCEEnc-の-aviutl-からの削除

・Aviutlのpulginsフォルダ内から下記フォルダとファイルを削除してください。
  - [フォルダ] VCEEnc_stg
  - [ファイル] VCEEnc.auo
  - [ファイル] VCEEnc.conf (存在する場合)
  - [ファイル] VCEEnc(.ini)
  - [ファイル] auo_setup.auf


【iniファイルによる拡張】
VCEEnc.iniを書き換えることにより、
音声エンコーダやmuxerのコマンドラインを変更できます。
また音声エンコーダを追加することもできます。

デフォルトの設定では不十分だと思った場合は、
iniファイルの音声やmuxerのコマンドラインを調整してみてください。


【ビルドについて】
ビルドにはVC++2022が必要です。

【コンパイル環境】
VC++ 2022 Community

【dtlの使用表記】
本プログラムは、dtlライブラリを内部で使用しています。
https://github.com/cubicdaiya/dtl

【tinyxml2の使用表記】
本プログラムは、tinyxml2を内部で使用しています。
http://www.grinninglizard.com/tinyxml2/index.html


【検証環境 2012.11～】
Win7 x64
Xeon W3680 + ASUS P6T Deluxe V2 (X58)
Radeon HD 7770
18GB RAM
CatalystControlCenter 12.8
CatalystControlCenter 12.10
CatalystControlCenter 12.11 beta

【検証環境 2013.10】
Win7 x64
Core i7 4770K + Asrock Z87 Extreme4
Radeon HD 7770
16GB RAM
CatalystControlCenter 13.4

【検証環境 2015.09】
Win7 x64
Core i7 4770K + Asrock Z97E-ITX/ac
Radeon R7 360
8GB RAM
CatalystControlCenter 15.7

【検証環境 2015.10】
なし

【検証環境 2016.06】
Win7 x64
Core i7 4770K + Asrock Z97E-ITX/ac
Radeon R7 360
8GB RAM
CatalystControlCenter 15.7

【検証環境 2016.06】
Win7 x64
Core i7 4770K + Asrock Z97E-ITX/ac
Radeon RX 460
8GB RAM
CatalystControlCenter 17.1

【検証環境 2018.11】
Win10 x64
Core i7 7700K + Asrock Z270 Gaming-ITX/ac
Radeon RX 460
16GB RAM

【検証環境 2019.11】
Win10 x64
Ryzen3 3200G + Asrock AB350 Pro4
16GB RAM

【検証環境 2020.06】
Win10 x64
Ryzen3 3200G + Asrock AB350 Pro4
Radeon RX 460
16GB RAM

【検証環境 2021.01】
Win10 x64
Ryzen9 5950X + Gigabyte B550 AOURUS Master
Radeon RX5500XT
Radeon RX460
32GB RAM

【検証環境 2022.09】
Win10 x64
Core i7 7700K + Asrock Z270 Gaming-ITX/ac
Radeon RX5500XT
8GB RAM

【検証環境 2022.10】
Win11 x64
Ryzen 7950X + MSI Pro X670-P WIFI
Radeon RX5500XT
Radeon RX550
64GB RAM

【検証環境 2022.12】
Win11 x64
Ryzen 7950X + MSI Pro X670-P WIFI
Radeon RX7900XT
Radeon RX5500XT
Radeon RX550
64GB RAM

【お断り】
今後の更新で設定ファイルの互換性がなくなるかもしれません。

【どうでもいいメモ】
2023.11.xx (8.17)
[VCEEncC]
- rffを展開するフィルタを追加。(--vpp-rff)
- --log-levelにquietを追加。
- 新しいAVChannelLayout APIに対応(Windows版)。
- --video-tagの指定がない場合、HEVCでは再生互換性改善のため、 "hvc1"をデフォルトとする。
  (libavformatのデフォルトは"hev1")
- --audio-streamをavs読み込み時にも対応。
- 音声フィルターの切り替えがエンコード中に発生する時に--thread-audio > 1で異常終了する問題を修正。
- --vpp-afs, --vpp-nnedi, --vpp-yadif, --vpp-padのエラーメッセージを拡充。
- --vpp-padのエラーメッセージを拡充。
- --vpp-decimateで最終フレームのタイムスタンプが適切に計算されず、異常終了する問題を修正。
- --vpp-select-everyが最終フレームの処理でエラー終了する場合があったのを修正。

[VCEEnc.auo]
- ログ出力に映像と音声の長さを表示するように。
- 一時ファイルの削除・リネーム失敗に関するエラーメッセージを拡充。
- 外部音声エンコーダを使用すると、muxerの制限でAV1が出力できない問題を修正。
  外部muxerの使用を廃止し、内部muxerを使用するよう変更した。
- AV1時のCQPの上限を255に。

2023.06.24 (8.16)
- faw処理時に音声がブツブツ切れる場合があったのを修正。
- 入力のcolorprimの指定がない場合に、--vpp-colorspaceのhdr2sdrが正常に動作しない問題を修正。

2023.06.20 (8.15)
[VCEEnc.auo]
- VCEEnc 8.14のfaw処理に問題があり、異常終了してしまうケースがあったのを修正。

2023.06.07 (8.14)
[VCEEncC]
- SAR比使用時の出力を改善。
- 複数のデバイスからGPUを選択する際、OpenCLの初期化に失敗したデバイスの優先度を落とすように。
- tsファイルで--audio-copyを使用すると、"AAC bitstream not in ADTS format and extradata missing"というエラーが出るのを回避。

[VCEEnc.auo]
- fawの処理機能を内蔵し、faw2aac.auo/fawcl.exeがなくても内蔵機能でfawを処理できるように。

2023.06.03 (8.13)
- dshowのカメラ入力等に対応。
- --audio-source, --sub-sourceのコマンドラインの区切りを変更。
- --vpp-colorspaceの調整。
- 音声処理をトラックごとに並列化。

2023.05.04 (8.12)
[VCEEncC]
- --audio-source/--sub-sourceでファイルのフォーマット等を指定できるように。
- libavdeviceのサポートを追加。
- timestampが0で始まらない音声を--audio-sourceで読み込むと、映像と正しく同期が取れない問題を修正。

2023.03.31 (8.11)
[VCEEncC]
- HQScalerが使用できない環境でリサイズしようとすると異常終了するのを回避。
- 縮小時のbilinear補間の精度を向上させるとともに、bicubicを追加。
- 色空間情報の記載のないy4mファイルの色がおかしくなるのを回避。デフォルトをYV12に変更。
- Linux環境で、 pgs_frame_mergeが見つからないというエラーを回避。
- --no-deblockオプションを追加。

[VCEEnc.auo]
- 出力する動画の長さが短い場合の警告を追加。

2023.03.11 (8.10)
[VCEEncC]
- エンコーダの呼び出しを頑健化。
- 音声・字幕のtimestampに負の値が入ることがあったのを回避。
- 動画の焼き込み時に--vpp-overlayが正常に動作しない場合があったのを修正。
- 8.09で--vpp-colorspaceを使用しようとすると異常終了する問題を修正。
- ログ出力を調整。

2023.03.05 (8.09)
[VCEEncC]
- 初期化時にGPUの情報を取得する際の処理のエラー耐性を向上。
- 色調を指定したカーブに従って変更するオプションを追加。(--vpp-curves)
- --vpp-overlayを最後のフィルタに変更。
- Windowsでドライバのバージョンをログに出力できるように。
- 複数のGPUがある場合の集計方法を合計から最大に変更。

2023.03.02 (8.08)
[VCEEncC]
- --lowlatency時にエンコーダの低遅延モードを有効にするのを取りやめ。
  キーフレームの画質が著しく劣化するため。

2023.02.28 (8.07)
[VCEEncC]
- AMF 1.4.29に対応。
  AMD Radeon Software Adrenalin Edition 23.1.2 以降が必要。
- Smart Access Videoに対応。(--smart-access-memory)
- OpenCLの初期化に失敗した際に、OpenCLを無効にして処理を継続するように。
  本来ドライバをインストールすればOpenCLは利用可能なはずだが、OpenCLがなぜか使用できない場合がある。
  そこで、そうした環境でもOpenCLが不要な処理であれば、動作を継続できるようにした。

[VCEEnc.auo]
- オブジェクトエクスプローラからドラッグドロップしたファイルがある場合の出力に対応。

2023.02.13 (8.06)
- フレーム時刻をタイムコードファイルから設定するオプションを追加。(--tcfile-in)
- 時間精度を指定するオプションを追加。(--timebase)
- --audio-profileが変更できなかった時に警告を表示するように。

2023.02.09 (8.05)
- 8.02からPGS字幕のコピーがうまく動作しなくなっていた問題を修正。

2023.02.07 (8.04)
- 8.02から --vpp-resize spline16, spline36, spline64を使用すると、意図しない線やノイズが入る問題を修正。

2023.02.06 (8.03)
- --vpp-afsにrffオプション使用時の誤検出でRFF部が12fpsになることがあったのを修正。

2023.02.05 (8.02)
- --sub-sourceでPGS字幕を読み込むと正常にmuxできない問題を回避。
- ffmpegのライブラリを更新 (Windows版)
  ffmpeg     5.0    -> 5.1
  libpng     1.3.8  -> 1.3.9
  expat      2.4.4  -> 2.5.0
  libsndfile 1.0.31 -> 1.2.0
  libxml2    2.9.12 -> 2.10.3
  libbluray  1.3.0  -> 1.3.4
  dav1d      0.9.2  -> 1.0.0
- --vpp-afsにrffオプションを追加。
- --check-hwでエンコードをサポートするコーデック名も表示するように。
- --check-hwや--check-featuresのログ出力を--log-levelで制御できるように。

2023.01.30 (8.01)
- 画像を焼き込むフィルタを追加。 (--vpp-overlay)
- lowlatency向けにmuxの挙動を調整。
- 動画ファイルに添付ファイルをつけるオプションを追加。 (--attachement-source)
- --perf-monitorでビットレート情報等が出力されないのを修正。
- 音声エンコードスレッド (--thread-audio 1) が動作しなくなっていた問題を修正。


2023.01.25 (8.00)
- AMFの提供するフィルタ(リサイズ、前処理、強調処理)を利用可能に。
  - --vpp-resize amf_xxx
  - --vpp-preprocess (8bitのみ対応)
  - --vpp-enhance
- メインループの刷新によるhwデコード周りの安定性向上。
- HEVCでも--paや--qvbrを使用可能なように。
- --paとhwデコード併用時にフリーズしてしまうことがある問題を解消。
- 入力ファイルによってエンコードが終了しなくなってしまう問題を修正。
- ログ出力の改善。
- AV1のmaster-displayの取り扱いを改善。
- maxcllあるいはmastering displayの片方がないときに、AV1エンコードで適切でないデータが発行されていた問題の修正。
- 入力ファイルによってエンコードが終了しなくなってしまう問題を修正。
- 言語による--audio-copyの指定が適切に動作していなかった問題を修正。

2022.12.20 (7.16)
- --paのtaqにmode2を追加。
- --paオプションに誤ったパラメータを指定した場合のメッセージが不適切だったのを修正。

2022.12.18 (7.15)
- AMF 1.4.28に対応。
  AMD Radeon Software Adrenalin Edition 22.12.1 以降が必要。
- 7.14では、AV1エンコードが動作しなかった問題を修正。
  Radeon RX7900XTで動作を確認。
- AV1関連パラメータを追加。
  - screen-content-tools
  - cdf-update
  - cdf-frame-end-update
- HEVCエンコーダのQVBR/VBR-HQ/CBR-HQモードに対応。

2022.12.01 (7.14)
- AV1 hwエンコードに仮対応。
  未テストなので、動作しない可能性は高い。
- AV1関連パラメータを追加。
  - --tiles
  - --cdef-mode
  - --aq-mode
  - --temporal-layers
- --check-hw等に--log-levelが作用するよう変更。

2022.11.13 (7.13)
- --vpp-subburnと--dhdr10-info copyを併用すると、dynamic HDR10のmetadataが適切にコピーされない問題を修正。
- エンコードするフレーム数を指定するオプション(--frames)を追加。
- --fpsで入力フレームレートを強制できるように。
- --vpp-tweakにswapuvオプションを追加。
- --vpp-subburnにfoced flag付きの字幕のみ焼きこむオプションを追加。
- 出力ファイル名が.rawの場合もraw出力とするように。

2022.10.23 (7.12)
[VCEEncC]
- 必要な場合に自動的にPre-analysisを有効にするように。
- --dhdr10-infoに対応。

[VCEEnc.auo]
- GPUの選択欄を追加。

2022.09.19 (7.11)
[VCEEnc.auo]
- Aviutl中国語対応をされているNsyw様に提供いただいた中国語対応を追加。
  翻訳の対応、ありがとうございました！

2022.09.18 (7.10)
[VCEEnc.auo]
- 設定画面上にツールチップを追加。
- 英語表示に対応。

2022.09.04 (7.09)
[VCEEncC]
- HEVC 10bit hwデコードの検出ミスを修正。
- OpenCLフィルタの安定性を改善。
- デバッグ用にHEVC bsfを切り替えるオプションを追加。(--input-hevc-bsf)

2022.08.25 (7.08)
[VCEEncC]
- --audio-streamの処理で、途中で音声のチャンネルが変化した場合にも対応。

2022.08.24 (7.07)
[VCEEncC]
- --vpp-yadifが最終フレームでエラー終了してしまうことがある問題を修正。

2022.08.23 (7.06)
[VCEEncC]
- インタレ解除フィルタをさらに追加。(--vpp-yadif)

2022.08.21 (7.05)
[VCEEncC]
- Linux環境でlibOpenCL.soがない場合に、libOpenCL.so.1を探しに行くように。

[VCEEnc.auo]
- AVX2使用時にFAWの1/2モードが正常に処理できなかったのを修正。

2022.08.15 (7.04)
[VCEEncC]
- Linuxで標準入力から読み込ませたときに、意図せず処理が中断してしまう問題を修正。
  なお、これに伴いLinuxではコンソールからの'q'あるいは'Q'でのプログラム終了はサポートしないよう変更した。(Ctrl+Cで代用のこと)

[VCEEnc.auo]
- qaac/fdkaacのコマンドラインに --gapless-mode 2 を追加。

2022.08.10 (7.03)
[VCEEncC]
- AMF 1.4.26に対応。
  AMD Radeon Software Adrenalin Edition 22.7.1 以降が必要。
- --pa に新たなオプションを追加。
  taq, paq, ltr, lookahead, motion-quality
- H.264エンコードに --vbrhq, --cbrhq を追加。
- downmix時に音量が小さくなってしまうのを回避。

2022.08.04 (7.02)
[VCEEnc.auo]
追加コマンドの設定欄を追加。

2022.07.28 (7.01)
[VCEEncC]
- vpp-colorspaceのhdr2sdr使用時に、ldr_nits, source_peakに関するエラーチェックを追加。
- アスペクト比を維持しつつ、指定の解像度にリサイズするオプションを追加。(--output-res <w>x<h>,preserve_aspect_ratio=<string>)
- vpp-smoothでfp16やsubgroupがサポートされないときに対応。
- pre-analysisが使用できる環境でも使用できないと表示されていた問題を修正。
- yuv422読み込み時にcropを併用すると、黒い横線が生じてしまう問題を修正。
- hevc 10bitでavhw使用時に正常にデコードできない問題を修正。
- HEVCエンコードでraw出力すると、HDRのヘッダが付与されなかった問題を修正。
- YUV420でvpp-afs使用時に、二重化するフレームで縞模様が発生してしまう問題を修正。
- AVPacket関連の非推奨関数の使用を削減。
- Ubuntu 22.04向けパッケージを追加。

[VCEEnc.auo]
- 黒窓プラグイン使用時に設定画面の描画を調整。
- デフォルトの音声ビットレートを変更。
- プリセットの音声ビットレートを変更。
- exe_filesから実行ファイルを検出できない場合、plugins\exe_filesを検索するように。
- ffmpeg (AAC)で -aac_coder twoloop を使用するように。
- ディスク容量が足りない時にどのドライブが足りないか表示するように。
- 外部muxer使用時に、なるべくremuxerで処理するよう変更。
- ScrollToCaret()を使用しないように。
- 音声の一時出力先が反映されなくなっていたのを修正。

2022.04.30 (7.00)
[VCEEncC]
- AMF 1.4.24に対応。
  AMD Radeon Software Adrenalin Edition 22.3.1 以降が必要。
- Visual Studio 2022に移行。
- ffmpeg関連のdllを更新。(Windows版)
  ffmpeg     4.x    -> 5.0
  expat      2.2.5  -> 2.4.4
  fribidi    1.0.1  -> 1.0.11
  libogg     1.3.4  -> 1.3.5
  libvorbis  1.3.6  -> 1.3.7
  libsndfile 1.0.28 -> 1.0.31
  libxml2    2.9.10 -> 2.9.12
  libbluray  1.1.2  -> 1.3.0
  dav1d      0.6.0  -> 0.9.2
- 音声の開始時刻が0でなく、かつ映像と音声のtimebaseが異なる場合の音ズレを修正。
- 3次元ノイズ除去フィルタを追加。(--vpp-convolution3d)
- vpp-colorspaceで3D LUTを適用可能に。(--vpp-colorspace lut3d )
- 一部のGPUでHEVC main10のエンコードができなくなっていたのを修正。
- --vpp-mpdecimateの安定性を改善。
- SetThreadInformationが使用できない環境での問題を回避。

[VCEEnc.auo]
- .NET Framework 4.8に移行。
- パッケージのフォルダ構成を変更。
- 簡易インストーラによるインストールを廃止。
- パスが指定されていない場合、exe_files内の実行ファイルを検索して使用するように。
- ログに使用した実行ファイルのパスを出力するように。
- 相対パスでのパスの保存をデフォルトに。
- 簡易インストーラを直接実行した場合に、エラーメッセージを表示するように変更。
- 拡張編集使用時の映像と音声の長さが異なる場合の動作の改善。
  拡張編集で音声を読み込ませたあと、異なるサンプリングレートの音声をAviutl本体に読み込ませると、
  音声のサンプル数はそのままに、サンプリングレートだけが変わってしまい、音声の時間が変わってしまうことがある。
  拡張編集使用時に、映像と音声の長さにずれがある場合、これを疑ってサンプリングレートのずれの可能性がある場合は
  音声のサンプル数を修正する。
- エンコードするフレーム数が0の場合のエラーメッセージを追加。
- ログの保存に失敗すると、例外が発生していたのを修正。
- ログの保存に失敗した場合にその原因を表示するように。
- muxエラーの一部原因を詳しく表示するように。
  mp4出力で対応していない音声エンコーダを選択した場合のエラーメッセージを追加。
- エラーメッセージ
  「VCEEncCが予期せず途中終了しました。VCEEncCに不正なパラメータ（オプション）が渡された可能性があります。」
    の一部原因を詳しく表示するように。
  1. ディスク容量不足でエンコードに失敗した場合のエラーメッセージを追加。
  2. 環境依存文字を含むファイル名- フォルダ名で出力しようとした場合のエラーメッセージを追加。
  3. Windowsに保護されたフォルダ等、アクセス権のないフォルダに出力しようとした場合のエラーメッセージを追加。
- 出力の際、Aviutlが開いているファイルに上書きしないように。
- 設定が行われていない場合に、前回出力した設定を読み込むように。
- 複数のAviutlプロセスで出力していても正常にチャプターを扱えるように。
- ログ出力のモードを変更すると正常に出力できないことがあったのを修正。

2021.12.06 (6.17)
・AMFを1.4.23に更新。ドライバ21.12.1以降が必要。
・1.4.23で追加された--qvbr-qualityを追加。手元の環境では影響しているように見えない。
・--check-features使用時に環境によってクラッシュする場合があったのを修正。
・想定動作環境にWindows11を追加。
・Windows11の検出を追加。
・スレッドアフィニティを指定するオプションを追加。(--thread-affinity)
・スレッドの優先度とPower Throttolingモードを指定するオプションを追加。(--thread-priority, --thread-throttling)
・ログの各行に時刻を表示するオプションを追加(デバッグ用)。(--log-opt addtime)
・bitstreamのヘッダ探索をAVX2/AVX512を用いて高速化。
・12bit深度を10bit深度に変換するときなどに、画面の左上に緑色の線が入ることがあったのを修正。
・--caption2assが使用できなかったのを修正。
・OpenCLの情報を表示するオプションを追加。(--check-clinfo)
・--vpp-smoothがRDNA世代以降正常に動作していなかったのを修正。
・--vpp-smoothでquality=0のときにはprec=fp16を使用できないので、自動的にprec=fp32に切り替え。
・--vpp-resize lanczosxの最適化。
・--vpp-smoothの最適化。
・--vpp-knnの最適化。
・OpenCLフィルタのパフォーマンス測定用のオプションを追加。(--vpp-perf-monitor)
・OpenCLのkernelコンパイルをバックグラウンドで実行するように。
・音声にbitstream filterを適用する--audio-bsfを追加。

2021.09.08 (6.16)
・--qvbrが使用できないGPUでのエラーメッセージを改善。
・LinuxでのデフォルトをH.264 main profileでのエンコードに変更。
・Linuxでのエンコードで映像が乱れる場合があったのを修正。

2021.09.06 (6.15)
・--qvbrを追加。RDNA世代以降のH.264エンコードでのみ使用可能。
・OpenCLでGPUを探す処理の改善。
・--vpp-colorspace使用時に、解像度によっては最終行に緑の線が入る問題を修正。

2021.08.12 (6.14)
・VCEEncCのLinux版を追加。
・6.13からHEVC 10bitエンコードの時、エンコーダがおかしなcolorprim, transfer等を設定することがあるのでこれを常に上書きして回避。
・vpp-subburnで使用できるフォントのタイプを更新。
・audio-delayが効いていなかったのを修正。

2021.07.29 (6.13)
・最近のAMDドライバが、GetFormatAt関数で適切な値を返さないため、
  ... does not support input/output format などのエラーが生じる問題を回避。
・AMFを1.4.21に更新。AMD Driver 21.6.1以降が必要。
・入力ファイルと出力ファイルが同じである場合にエラー終了するように。
・OpenCLでcropすると色成分がずれるのを修正。
・y4m読み込みの際、指定したインタレ設定が上書きされてしまうのを修正。
・OpenCLのyuv444→nv12の修正。
・--vpp-decimateの計算用領域が不足していた不具合を修正。
・yuv444→p010のavx2版の色ずれを修正。
・AvisynthNeo環境などで生じるエラー終了を修正。

2021.05.23 (6.12)
・raw出力、log出力の際にカレントディレクトリに出力しようとすると異常終了が発生する問題を修正。

2021.05.09 (6.11)
・avsw/avhwでのファイル読み込み時にファイル解析サイズの上限を設定するオプションを追加。(--input-probesize)
・--input-analyzeを小数点で指定可能なよう拡張。
・読み込んだパケットの情報を出力するオプションを追加。( --log-packets )
・data streamに限り、タイムスタンプの得られていないパケットをそのまま転送するようにする。
・オプションを記載したファイルを読み込む機能を追加。( --option-file )
・動画情報を取得できない場合のエラーメッセージを追加。
・コピーするtrackをコーデック名で選択可能に。
・字幕の変換が必要な場合の処理が有効化されていなかったのを修正。
・下記OpenCLによるvppフィルタを追加。
  - --vpp-delogo
  - --vpp-decimate
  - --vpp-mpdecimate

2021.04.17 (6.10)
・RX5500XTなどでパラメータの設定が反映されない場合があったのを修正。
・--sub-metadata, --audio-metadataを指定した場合にも入力ファイルからのmetadataをコピーするように。
・字幕のmetadataが二重に出力されてしまっていた問題を修正。
・--audio-source/--sub-sourceを複数指定した場合の挙動を改善。
・--avsync forcecfrの連続フレーム挿入の制限を1024まで緩和。
・正常に動作しない--videoformat autoの削除。

2021.02.18 (6.09)
・AMF SDK 1.4.18に対応。これに伴い、AMD driver 20.11.2以降が必要。
・VP9 HWデコードのサポートを追加。
・AvisynthのUnicode対応に伴い、プロセスの文字コードがUTF-8になっているのを
  OSのデフォルトの文字コード(基本的にShiftJIS)に戻すオプションを追加。(--process-codepage os)
・AvisynthのUnicode対応を追加。
・Windows 10のlong path supportの追加。
・色空間変換を行うフィルタを追加。(--vpp-colorspace)
・細線化フィルタを追加。(--vpp-warpsharp)
・--vpp-subburnで埋め込みフォントを使用可能なように。
・--vpp-subburnでフォントの存在するフォルダを指定可能なように。
・--audio-source / --sub-source でmetadataを指定可能なよう拡張。
・--ssim, --psnrの10bit深度対応を追加。
・timecodeの出力を追加。(--timecode)
・--check-featureで10bit深度エンコードのサポートの有無を表示するように。
・--check-featureでHWデコーダの情報を表示するように。
・マルチGPU環境でGPUの自動選択をする際に、10bit深度エンコードのサポートの有無をチェックするように。
・マルチGPU環境でGPUの自動選択をする際に、--avhw使用時にはHWデコーダの有無をチェックするように。

2021.01.27 (6.08)
・RX5xxx以降での10bit深度のエンコードに対応。(--output-depth 10)
・--vpp-smoothがRX5xxxで正常に動作しなかったのを修正。
・timecodeの出力を追加。(--timecode)
・言語による音声や字幕の選択に対応。
・ビット深度を下げるときの丸め方法を改善。
・パフォーマンスカウンター集計を改善。
・--vpp-tweakのsaturation,hueを指定した場合に異常終了してしまうのを修正。
・--ssim計算時にエラーで異常終了する場合があったのを修正。
・NVEncと同様に--qualityを--presetに変更。

2020.11.23 (6.07)
[VCEEncC]
・VUIを設定すると出力が異常になる場合があったのを修正。
・--chapterが効かなかったのを修正。

2020.11.03 (6.06)
[VCEEncC]
・ノイズを除去するフィルタの追加。(--vpp-smooth)
・字幕トラックを焼きこむフィルタを追加。( --vpp-subburn )
・YUV422読み込みのサポートを追加。
・HEVCのlowlatencyが正しく設定されないのを修正。
・H.264のlowlatencyモードが、指定していないのに動作してしまうのを修正。
・異常終了が発生した場合のフリーズを回避。

2020.09.23 (6.05)
[VCEEncC]
・ログメッセージでlowlatencyが無効の場合もいつもlowlatencyと表示されていたのを修正。
・hrdをデフォルトではオフに。--enforce-hrdで有効化できる。

2020.09.17 (6.04)
[VCEEncC]
・2つめのインタレ解除フィルタ (--vpp-nnedi)を追加。
・映像にパディングを付加するフィルタ(--vpp-pad)を追加。
・映像の回転・反転を行うフィルタを追加。(--vpp-rotate/vpp-transform)
・preanalysisの情報が不必要に表示されていたのを修正。
・proresがデコードできないのを修正。
・raw読み込み時に色空間を指定するオプションを追加。(--input-csp)
・HEVC 10bitのhwデコードのサポートを追加。
・HEVCエンコード時のプリセットが正しく設定されないのを修正。

2020.07.06 (6.03)
[VCEEncC]
・ノイズ除去フィルタ(--vpp-knn)を追加。
・ノイズ除去フィルタ(--vpp-pmd)を追加。
・エッジ調整フィルタ(--vpp-edgelevel)を追加。
・エッジ強調フィルタ(--vpp-unsharp)を追加。
・バンディング低減フィルタ(--vpp-deband)を追加。
・色調整フィルタ(--vpp-tweak)を追加。
・複数のAMD GPU環境でも選択したGPUを適切に使用できるように改良。
・出力ファイルのmetadata制御を行うオプション群を追加。
  --metadata
  --video-metadata
  --audio-metadata
  --sub-metadata
・attachmentをコピーするオプションを追加。 ( --attachment-copy )
・streamのdispositionを指定するオプションを追加。 ( --audio-disposition, --sub-disposition )
・--audio-sourceでもdelayを指定できるように。
・6.01からvpy読み込みがシングルスレッド動作になっていたのをマルチスレッド動作に戻した。
・オプションリストを表示するオプションを追加。 ( --option-list )
・avs読み込みで、可能であれば詳細なバージョン情報を取得するように。
・一部のHEVCファイルで、正常にデコードできないことがあるのに対し、可能であればswデコーダでデコードできるようにした。
・遅延を伴う一部の--audio-filterで音声の最後がエンコードされなくなってしまう問題を修正。
・--audio-source/--sub-sourceでうまくファイル名を取得できないことがあるのを修正。
・avsw/avhw読み込み時にlowlatencyが使用できないのを修正。
・--video-tagを指定すると異常終了してしまうのを修正。
・raw出力でSAR比を指定したときに発生するメモリリークを修正。

[VCEEnc.auo]
・VCEEnc.auoの設定画面でも、--output-resに負の値を指定できるように。
・簡易インストーラ更新。
  VC runtimeのダウンロード先のリンク切れを修正。

2020.05.14 (6.02)
[VCEEncC]
・vpp-afsのNaviでのOpenCLのコンパイルエラーを修正、6.01での修正が不十分だった。

[VCEEnc.auo]
・HEVC時の設定画面の挙動を改善。

2020.05.12 (6.01)
[VCEEncC]
・pre-encodeによるレート制御を使用するオプションを追加。( --pe )
・vpp-afsのNaviでのOpenCLのコンパイルエラーを修正する。
・遅延を最小化するモードを追加。 ( --lowlatency )

[VCEEnc.auo]
・VCEEnc.auoからsarが指定できないのを修正。
・外部エンコーダ使用時に、音声エンコードを「同時」に行うと異常終了するのを修正。

2020.04.19 (6.00)
[VCEEncC]
・音声デコーダやエンコーダへのオプション指定が誤っていた場合に、
  エラーで異常終了するのではなく、警告を出して継続するよう変更。
・--chapterがavsw/avhw利用時にしか効かなかったのを修正。

[VCEEnc.auo]
・VCEEnc.auoで内部エンコーダを使用するモードを追加。
  こちらの動作をデフォルトにし、外部エンコーダを使うほうはオプションに。

2020.03.07 (5.04)
[VCEEncC]
・avsw/avhw読み込み時の入力オプションを指定するオプションを追加。(--input-option)
・trueHDなどの一部音声がうまくmuxできないのを改善。
・VCEEnc.auoの修正に対応する変更を実施。

[VCEEnc.auo]
・VCEEnc.auoから出力するときに、Aviutlのウィンドウを最小化したり元に戻すなどするとフレームが化ける問題を修正。

2020.02.29 (5.03)
[VCEEncC]
・caption2assが正常に動作しないケースがあったのを修正。
・5.02で--cqpが正常に動作しない問題を修正。

[VCEEnc.auo]
・簡易インストーラの安定動作を目指した改修。
  必要な実行ファイルをダウンロードしてインストールする形式から、
  あらかじめ同梱した実行ファイルを展開してインストールする方式に変更する。
・デフォルトの音声エンコーダをffmpegによるAACに変更。
・VCEEnc.auoの設定画面のタブによる遷移順を調整。

2020.02.11 (5.02)
[VCEEncC]
・動作環境の変更。Win10のみ対応。
・AMF 1.4.14 -> 1.4.16に更新。
  AMD Radeon Software Adrenalin Edition 20.2.1 以降が必要に。
・AMF 1.4.16で追加されたpre-anaysisに関するオプションを追加。(VCEEncCのみ)
  (--pa, --pa-sc, --pa-ss, --pa-activity-type, --pa-caq-strength, --pa-initqpsc, --pa-fskip-maxqp )
・ssim/psnrを計算するオプションを追加。(--ssim/--psnr)
・HDR関連のmeta情報を入力ファイルからコピーできるように。
  (--master-display copy, --max-cll copy)
・colormatrix等の情報を入力ファイルからコピーする機能を追加。
  --colormtarix copy
  --colorprim copy
  --transfer copy
  --chromaloc copy
  --colorrange copy
・HEVCエンコ時に、high tierの存在しないlevelが設定された場合、main tierに修正するように。
・mux時の動作の安定性を向上し、シーク時に不安定になる症状を改善。
・Windowsのパフォーマンスカウンタを使用したプロセスのGPU使用率情報を使用するように。
・Readme等で、NVEncのままの表記だった個所を修正。
・ログに常に出力ファイル名を表示するように。
・VUI情報、mastering dsiplay, maxcllの情報をログに表示するように。

2019.12.24 (5.01)
[VCEEncC]
・OpenCLで実装された自動フィールドシフトを追加。(--vpp-afs)
・音声処理でのメモリリークを解消。
・音声エンコード時のエラーメッセージ強化。
・字幕のコピー等が動かなくなっていたのを修正。
・trueHD in mkvなどで、音声デコードに失敗する場合があるのを修正。
・音声に遅延を加えるオプションを追加。 ( --audio-delay )
・mkv入りのVC-1をカットした動画のエンコードに失敗する問題を修正。
・HWデコード時に映像が乱れる場合があったのを回避。

[VCEEnc.auo]
・簡易インストーラを更新。
・AVX2版のyuy2→nv12i変換の誤りを修正。

2019.11.19 (5.00)
[共通]
・AMF 1.4.8 -> 1.4.14に更新。
・VC++2019に移行。

[VCEEnc.auo]
・VCEEnc.auo - VCEEncC間のフレーム転送を効率化して高速化。
・簡易インストーラを更新。
・VCEEnc.auoの出力をmp4/mkv出力に変更し、特に自動フィールドシフト使用時のmux工程数を削減する。
  また、VCEEncCのmuxerを使用することで、コンテナを作成したライブラリとしQSVEncCを記載するようにする。

[VCEEncC]
・可能なら進捗表示に沿うフレーム数と予想ファイルサイズを表示。
・映像のcodec tagを指定するオプションを追加。(--video-tag)
・字幕ファイルを読み込むオプションを追加。 (--sub-source )
・--audio-sourceを改修し、より柔軟に音声ファイルを取り扱えるように。
・データストリームをコピーするオプションを追加する。(--data-copy)
・--sub-copyで字幕をデータとしてコピーするモードを追加。
  --sub-copy asdata
・--audio-codecにデコーダオプションを指定できるように。
  --audio-codec aac#dual_mono_mode=main
・avsからの音声処理に対応。
・高負荷時にデッドロックが発生しうる問題を修正。
・音声エンコードの安定性を向上。
・CPUの動作周波数が適切に取得できないことがあったのを修正。
・--chapterでmatroska形式に対応する。
・--audio-copyでTrueHDなどが正しくコピーされないのを修正。
・--trimを使用すると音声とずれてしまう場合があったのを修正。
・音声エンコード時のtimestampを取り扱いを改良、VFR時の音ズレを抑制。
・mux時にmaster-displayやmax-cllの情報が化けるのを回避。
・ffmpegと関連dllを追加/更新。
  - [追加] libxml2 2.9.9
  - [追加] libbluray 1.1.2
  - [追加] aribb24 rev85
  - [更新] libpng 1.6.34 -> 1.6.37
  - [更新] libvorbis 1.3.5 -> 1.3.6
  - [更新] opus 1.2.1 -> 1.3.1
  - [更新] soxr 0.1.2 -> 0.1.3
・そのほかさまざまなNVEnc/QSVEnc側の更新を反映。

2018.12.11 (4.02)  
[VCEEnc.auo]
・自動フィールドシフト使用時、widthが32で割り切れない場合に範囲外アクセスの例外で落ちる可能性があったのを修正。
・Aviutlからのフレーム取得時間(平均)の表示をログに追加。

2018.11.24 (4.01)
[VCEEncC]
・読み込みにudp等のプロトコルを使用する場合に、正常に処理できなくなっていたのを修正。
・--audio-fileが正常に動作しないことがあったのを修正。

2018.11.18 (4.00)
[共通]
・AMF 1.4.9に対応。
・colormatrix/colorprim/transfer/videoformatの指定に対応。
・HEVCのSAR比指定に対応。

[VCEEnc.auo]
・エンコーダをプラグインに内蔵せず、VCEEncCにパイプ渡しするように。
  Aviutl本体プロセスのメモリ使用量を削減する。
・設定ファイルのフォーマットを変更したため、以前までの設定ファイルは読めなくなってしまったのでご注意ください。
・簡易インストーラを更新。
  - Apple dllがダウンロードできなくなっていたので対応。
  - システムのプロキシ設定を自動的に使用するように。

2017.02.27 (3.06)
[VCEEncC]
・HEVCデコードができなくなっていたのを修正。
・ログ出力を強化。

2017.02.16 (3.05v2)
[VCEEncC]
※同梱のdll類も更新してください!
・dllの更新忘れにより、HEVCデコードができなくなっていたのを修正。

2017.02.14 (3.05)
[VCEEncC]
・enforce-hdrをデフォルトでオフに。
・enforce-hdrの有効無効の切り替えオプションを追加。(--enforce-hdr)
・fillerデータの有効無効の切り替えオプションを追加。(--filler)

2017.02.03 (3.04)
[共通]
・3.00から高くなっていたCPU使用率を低減。
・リモートデスクトップ中のエンコードをサポート。
  DX9での初期化に失敗した場合、DX11での初期化を行う。
・特に指定がない場合、levelや最大ビットレートを解像度やフレームレート、refなどから自動的に決定するように。
  Levelの不足により、HEVC 4Kエンコードができないのを修正する。

[VCEEnc.auo]
・cbr/vbrモードで、最大ビットレートの指定ができない問題を修正。
  
[VCEEncC]
・高ビット深度の入力ファイルに対しては、自動的にswデコードに回すように。
・使用するデバイスIDを指定できるように。
・VCEが使用可能か、HEVCエンコードが可能かを、デバイスIDを指定して行えるように。
・エラーでないメッセージが赤で表示されていたのを修正。
・特に指定がなく、解像度の変更もなければ、読み込んだSAR比をそのまま反映するように。

2017.02.01 (3.03)
[共通]
・HEVCでのvbr/cbrモードでのVBAQを許可。

[VCEEnc.auo]
・HEVCエンコ時にLevelが正しく保存されないのを修正。
・VCEEnc.auoからもHWリサイザを利用できるように。
・VCEEncCのHEVCエンコで--vbrが正常に動作しない問題を修正。

[VCEEncC]
・avswリーダーでYUV420 10bitの読み込みに対応(エンコードは8bit)。
・avswリーダー使用時に解像度によっては、色がずれてしまうのを修正。

2017.01.30 (3.02)
※同梱のdll類も更新してください!

[共通]
・H.264のfullrangeを指定するオプションを追加。(--fullrange)
・HEVCエンコで--qualityを指定すると常にログでnormalと表示される問題を修正。
・4Kがエンコードできない問題を修正。
・ログに出るdeblockの有効無効が反転していたのを修正。

[VCEEncC]
・HW HEVCデコードの安定性を向上。
・avcodecによるswデコードをサポート。(--avsw)
・HEVCのtierを指定するオプションを追加。(--tier <string>)
・--pre-analysisのquarterが正常に動作しなかったのを修正。
・HEVCの機能情報が取得できない問題を回避。
・音声処理のエラー耐性の向上。

2017.01.25 (3.01)
[共通]
・実行時にVCEの機能を確認し、パラメータのチェックを行うように。
・参照距離を指定するオプションを追加。(--ref <int>)
・LTRフレーム数を指定するオプションを追加。(--ltr <int>)
・H.264 Level 5.2を追加。
・バージョン情報にAMFのバージョンを追記。

[VCEEncC]
・ヘルプの誤字脱字等を修正。
・VCEの機能をチェックするオプションを追加。(--check-features)
  HEVCの機能については正常に表示できない。
・HEVC(8bit)のHWデコードを追加。
・wmv3のHWデコードが正常に動作しないため、削除。

2017.01.22 (3.00)
[共通]
・AMD Media Framework 1.4に対応。
  AMD Radeon Software Crimson 17.1.1 以降が必要。
・PolarisのHEVCエンコードを追加。(-c hevc)
・SAR比の指定を追加。
・先行探索を行うオプションを追加。(--pre-analysis)
・VBAQオプションを追加。(H.264のみ)
・その他、複数の不具合を修正。

2016.06.21 (2.00)
[VCEEncC]
・H.264/MPEG2のハードウェアデコードに対応。(avvceリーダー)
・muxしながら出力する機能を追加。
・音声の抽出/エンコードに対応。
・ハードウェアリサイズに対応。
・そのほかいろいろ。

注意点
・QSVEncC/NVEncCにあるような以下の機能には対応していません。
  - avsync
  - trim
  - crop
  - インタレ保持エンコード
  - インタレ解除
  - colormatrix, colorprim, transfer, videoformatなど

2016.01.14 (1.03v2)
・簡易インストーラでQuickTimeがダウンロードできなくなっていたのを修正。

2015.10.24 (1.03)
[VCEEnc]
・エンコ後バッチ処理の設定画面を修正。

2015.09.26 (1.02)
[VCEEnc]
・FAWCheckが働かないのを修正。

2015.09.24 (1.01)
[VCEEnc]
・音声エンコード時に0xc0000005例外で落ちることがあるのを修正。

[VCEEncC]
・更新なし。

2015.09.23 (1.00)
[共通]
・AMD APP SDK 3.0 + 新APIに移行し、Catalyst15.7以降で動作しなくなっていたのを修正。
・VC++ 2015に移行。
・.NET Framework 4.5.2に移行。

[VCEEnc]
・設定ファイルの互換性がなくなってしまいました。
・様々な機能をx264guiEx 2.34相当にアップデート。

[VCEEncC]
・新たに追加。(x86/x64)
・読み込みはraw/y4m/avs/vpy。

2013.12.07 (0.02v2)
・簡易インストーラを更新
  - 簡易インストーラをインストール先のAviutlフォルダに展開すると
    一部ファイルのコピーに失敗する問題を修正
  - L-SMASHがダウンロードできなくなっていたのを修正。
  - インストール先が管理者権限を必要とする際は、
    これを取得するダイアログを表示するようにした。
    
2013.10.20 (0.02)
・自動フィールドシフト対応。
・簡易インストーラを更新
  - Windows 8.1に対応した(と思う)
  - アップデートの際にプリセットを上書き更新するかを選択できるようにした。
・x264guiEx 2.03までの更新を反映。
  - ログウィンドウの位置を保存するようにした。
  - 高Dpi設定時の表示の崩れを修正。
  - エンコード開始時にクラッシュする可能性があるのを修正。
  - エンコ前後バッチ処理を最小化で実行する設定を追加。
  - 出力ファイルの種類のデフォルトを変更できるようにした。
  - FAW half size mix モード対応。
  - mux時にディスクの空き容量の取得に失敗した場合でも、警告を出して続行するようにした。
  - 設定画面で「デフォルト」をクリックした時の挙動を修正。
    「デフォルト」をクリックしたあと、「キャンセル」してもキャンセルされていなかった。
  - ログウィンドウで出力ファイル名を右クリックから
    「動画を再生」「動画のあるフォルダを開く」機能を追加。
  - 変更したフォントの(標準⇔斜体)が保存されない問題を修正。

2012.11.17 (0.01)
・シークができなくなる問題に対処。
・ログウィンドウの色の指定を可能に。ログウィンドウ右クリックから。

2012.11.06 (0.00)
  公開

2012.11.04
  エンコードスレッドを別に立てて高速化
  
2012.11.03
  パイプライン型エンコードにより高速化
  
2012.11.01
  なんか遅い
  
2012.10.28
  結構遅い。CPUが遊ぶ
  
2012.10.27
  とりあえず動く