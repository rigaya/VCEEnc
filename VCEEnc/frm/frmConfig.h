// -----------------------------------------------------------------------------------------
// x264guiEx/x265guiEx/svtAV1guiEx/ffmpegOut/QSVEnc/NVEnc/VCEEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2010-2022 rigaya
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// --------------------------------------------------------------------------------------------

#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "auo.h"
#include "auo_pipe.h"
#include "auo_conf.h"
#include "auo_settings.h"
#include "auo_system.h"
#include "auo_util.h"
#include "auo_clrutil.h"

#include "vce_param.h"
#include "frmConfig_helper.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::IO;
using namespace System::Threading::Tasks;


namespace AUO_NAME_R {

    /// <summary>
    /// frmConfig の概要
    ///
    /// 警告: このクラスの名前を変更する場合、このクラスが依存するすべての .resx ファイルに関連付けられた
    ///          マネージ リソース コンパイラ ツールに対して 'Resource File Name' プロパティを
    ///          変更する必要があります。この変更を行わないと、
    ///          デザイナと、このフォームに関連付けられたローカライズ済みリソースとが、
    ///          正しく相互に利用できなくなります。
    /// </summary>
    public ref class frmConfig : public System::Windows::Forms::Form
    {
    public:
        frmConfig(CONF_GUIEX *_conf, const SYSTEM_DATA *_sys_dat)
        {
            //ライブラリのチェック
            InitData(_conf, _sys_dat);
            list_lng = nullptr;
            dwStgReader = nullptr;
            themeMode = AuoTheme::DefaultLight;
            cnf_stgSelected = (CONF_GUIEX *)calloc(1, sizeof(CONF_GUIEX));
            InitializeComponent();
            //
            //TODO: ここにコンストラクタ コードを追加します
            //
        }

    protected:
        /// <summary>
        /// 使用中のリソースをすべてクリーンアップします。
        /// </summary>
        ~frmConfig()
        {
            if (components)
            {
                delete components;
            }
            CloseBitrateCalc();
            if (dwStgReader != nullptr)
                delete dwStgReader;
            if (cnf_stgSelected) free(cnf_stgSelected); cnf_stgSelected = NULL;
            if (list_lng != nullptr)
                delete list_lng;
        }




    private: System::Windows::Forms::ToolStrip^  fcgtoolStripSettings;

    private: System::Windows::Forms::TabControl^  fcgtabControlMux;
    private: System::Windows::Forms::TabPage^  fcgtabPageMP4;
    private: System::Windows::Forms::TabPage^  fcgtabPageMKV;
    private: System::Windows::Forms::TextBox^  fcgTXCmd;

    private: System::Windows::Forms::Button^  fcgBTCancel;

    private: System::Windows::Forms::Button^  fcgBTOK;
    private: System::Windows::Forms::Button^  fcgBTDefault;




    private: System::Windows::Forms::ToolStripButton^  fcgTSBSave;

    private: System::Windows::Forms::ToolStripButton^  fcgTSBSaveNew;

    private: System::Windows::Forms::ToolStripButton^  fcgTSBDelete;

    private: System::Windows::Forms::ToolStripSeparator^  fcgtoolStripSeparator1;
    private: System::Windows::Forms::ToolStripDropDownButton^  fcgTSSettings;





























































































































































































































































    private: System::Windows::Forms::TabPage^  fcgtabPageMux;












    private: System::Windows::Forms::ComboBox^  fcgCXMP4CmdEx;

    private: System::Windows::Forms::Label^  fcgLBMP4CmdEx;
    private: System::Windows::Forms::CheckBox^  fcgCBMP4MuxerExt;
    private: System::Windows::Forms::Button^  fcgBTMP4BoxTempDir;
    private: System::Windows::Forms::TextBox^  fcgTXMP4BoxTempDir;


    private: System::Windows::Forms::ComboBox^  fcgCXMP4BoxTempDir;
    private: System::Windows::Forms::Label^  fcgLBMP4BoxTempDir;
    private: System::Windows::Forms::Button^  fcgBTTC2MP4Path;
    private: System::Windows::Forms::TextBox^  fcgTXTC2MP4Path;
    private: System::Windows::Forms::Button^  fcgBTMP4MuxerPath;
    private: System::Windows::Forms::TextBox^  fcgTXMP4MuxerPath;

    private: System::Windows::Forms::Label^  fcgLBTC2MP4Path;
    private: System::Windows::Forms::Label^  fcgLBMP4MuxerPath;


    private: System::Windows::Forms::Button^  fcgBTMKVMuxerPath;

    private: System::Windows::Forms::TextBox^  fcgTXMKVMuxerPath;

    private: System::Windows::Forms::Label^  fcgLBMKVMuxerPath;
    private: System::Windows::Forms::ComboBox^  fcgCXMKVCmdEx;
    private: System::Windows::Forms::Label^  fcgLBMKVMuxerCmdEx;
    private: System::Windows::Forms::CheckBox^  fcgCBMKVMuxerExt;
    private: System::Windows::Forms::ComboBox^  fcgCXMuxPriority;
    private: System::Windows::Forms::Label^  fcgLBMuxPriority;
    private: System::Windows::Forms::CheckBox^  fcgCBMuxMinimize;
    private: System::Windows::Forms::Label^  fcgLBVersionDate;


    private: System::Windows::Forms::Label^  fcgLBVersion;
    private: System::Windows::Forms::FolderBrowserDialog^  fcgfolderBrowserTemp;
    private: System::Windows::Forms::OpenFileDialog^  fcgOpenFileDialog;










private: System::Windows::Forms::ToolTip^  fcgTTEx;






private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator2;
private: System::Windows::Forms::ToolStripButton^  fcgTSBOtherSettings;















































































































private: System::Windows::Forms::ToolStripButton^  fcgTSBBitrateCalc;
private: System::Windows::Forms::TabControl^  fcgtabControlVCE;
private: System::Windows::Forms::TabPage^  tabPageVideoEnc;












private: System::Windows::Forms::GroupBox^  fcgGroupBoxAspectRatio;
private: System::Windows::Forms::Label^  fcgLBAspectRatio;
private: System::Windows::Forms::NumericUpDown^  fcgNUAspectRatioY;
private: System::Windows::Forms::NumericUpDown^  fcgNUAspectRatioX;
private: System::Windows::Forms::ComboBox^  fcgCXAspectRatio;
private: System::Windows::Forms::ComboBox^  fcgCXInterlaced;
private: System::Windows::Forms::Label^  fcgLBInterlaced;
private: System::Windows::Forms::NumericUpDown^  fcgNUGopLength;
private: System::Windows::Forms::Label^  fcgLBGOPLength;
private: System::Windows::Forms::ComboBox^  fcgCXCodecLevel;
private: System::Windows::Forms::ComboBox^  fcgCXCodecProfile;
private: System::Windows::Forms::Label^  fcgLBCodecLevel;
private: System::Windows::Forms::Label^  fcgLBCodecProfile;




private: System::Windows::Forms::Label^  fcgLBMaxBitrate2;
private: System::Windows::Forms::Label^  fcgLBEncMode;
private: System::Windows::Forms::ComboBox^  fcgCXEncMode;
private: System::Windows::Forms::Label^  fcgLBMaxkbps;
private: System::Windows::Forms::NumericUpDown^  fcgNUMaxkbps;
private: System::Windows::Forms::Label^  fcgLBQPB;
private: System::Windows::Forms::Label^  fcgLBQPP;
private: System::Windows::Forms::Label^  fcgLBQPI;

private: System::Windows::Forms::NumericUpDown^  fcgNUQPB;
private: System::Windows::Forms::NumericUpDown^  fcgNUQPP;
private: System::Windows::Forms::NumericUpDown^  fcgNUQPI;



private: System::Windows::Forms::Label^  fcgLBBitrate2;
private: System::Windows::Forms::NumericUpDown^  fcgNUBitrate;
private: System::Windows::Forms::Label^  fcgLBBitrate;
private: System::Windows::Forms::TabPage^  tabPageVpp;


































private: System::Windows::Forms::Label^  fcgLBGOPLengthAuto;


































private: System::Windows::Forms::ToolStripLabel^  fcgTSLSettingsNotes;
private: System::Windows::Forms::ToolStripTextBox^  fcgTSTSettingsNotes;
private: System::Windows::Forms::TabPage^  fcgtabPageBat;
private: System::Windows::Forms::Button^  fcgBTBatAfterPath;

private: System::Windows::Forms::TextBox^  fcgTXBatAfterPath;

private: System::Windows::Forms::Label^  fcgLBBatAfterPath;

private: System::Windows::Forms::CheckBox^  fcgCBWaitForBatAfter;

private: System::Windows::Forms::CheckBox^  fcgCBRunBatAfter;

private: System::Windows::Forms::CheckBox^  fcgCBMP4MuxApple;











private: System::Windows::Forms::Panel^  fcgPNBitrate;
private: System::Windows::Forms::Panel^  fcgPNQP;













private: System::Windows::Forms::Button^  fcgBTMP4RawPath;

private: System::Windows::Forms::TextBox^  fcgTXMP4RawPath;
private: System::Windows::Forms::Label^  fcgLBMP4RawPath;





















private: System::Windows::Forms::ContextMenuStrip^  fcgCSExeFiles;
private: System::Windows::Forms::ToolStripMenuItem^  fcgTSExeFileshelp;




private: System::Windows::Forms::Label^  fcgLBBatAfterString;

private: System::Windows::Forms::Label^  fcgLBBatBeforeString;
private: System::Windows::Forms::Panel^  fcgPNSeparator;
private: System::Windows::Forms::Button^  fcgBTBatBeforePath;
private: System::Windows::Forms::TextBox^  fcgTXBatBeforePath;
private: System::Windows::Forms::Label^  fcgLBBatBeforePath;
private: System::Windows::Forms::CheckBox^  fcgCBWaitForBatBefore;
private: System::Windows::Forms::CheckBox^  fcgCBRunBatBefore;
private: System::Windows::Forms::LinkLabel^  fcgLBguiExBlog;
private: System::Windows::Forms::CheckBox^  fcgCBDeblock;























private: System::Windows::Forms::PictureBox^  fcgpictureBoxVCEDisabled;




private: System::Windows::Forms::PictureBox^  fcgpictureBoxVCEEnabled;


private: System::Windows::Forms::Label^  fcgLBDeblock;



private: System::Windows::Forms::NumericUpDown^  fcgNUBframes;
private: System::Windows::Forms::Label^  fcgLBBframes;
private: System::Windows::Forms::Label^  fcgLBQualityPreset;
private: System::Windows::Forms::ComboBox^  fcgCXQualityPreset;


private: System::Windows::Forms::Label^  fcgLBBPyramid;
private: System::Windows::Forms::CheckBox^  fcgCBBPyramid;
private: System::Windows::Forms::CheckBox^  fcgCBAFS;
private: System::Windows::Forms::NumericUpDown^  fcgNUQPMax;
private: System::Windows::Forms::Label^  fcgLBQPMax;
private: System::Windows::Forms::Label^  fcgLBQPMinMAX;
private: System::Windows::Forms::NumericUpDown^  fcgNUQPMin;
private: System::Windows::Forms::NumericUpDown^  fcgNUBDeltaQP;

private: System::Windows::Forms::Label^  fcgLBBDeltaQP;
private: System::Windows::Forms::Label^  fcgLBSkipFrame;
private: System::Windows::Forms::CheckBox^  fcgCBSkipFrame;
private: System::Windows::Forms::NumericUpDown^  fcgNUBRefDeltaQP;
private: System::Windows::Forms::Label^  fcgLBBRefDeltaQP;
private: System::Windows::Forms::NumericUpDown^  fcgNUVBVBufSize;
private: System::Windows::Forms::Label^  fcgLBVBVBufSize;
private: System::Windows::Forms::Label^  fcgLBVBVBufSizeKbps;
private: System::Windows::Forms::TabControl^  fcgtabControlAudio;
private: System::Windows::Forms::TabPage^  fcgtabPageAudioMain;


















private: System::Windows::Forms::CheckBox^  fcgCBFAWCheck;


private: System::Windows::Forms::TabPage^  fcgtabPageAudioOther;

private: System::Windows::Forms::Panel^  panel2;
private: System::Windows::Forms::Label^  fcgLBBatAfterAudioString;
private: System::Windows::Forms::Label^  fcgLBBatBeforeAudioString;
private: System::Windows::Forms::Button^  fcgBTBatAfterAudioPath;
private: System::Windows::Forms::TextBox^  fcgTXBatAfterAudioPath;
private: System::Windows::Forms::Label^  fcgLBBatAfterAudioPath;
private: System::Windows::Forms::CheckBox^  fcgCBRunBatAfterAudio;
private: System::Windows::Forms::Panel^  panel1;
private: System::Windows::Forms::Button^  fcgBTBatBeforeAudioPath;
private: System::Windows::Forms::TextBox^  fcgTXBatBeforeAudioPath;
private: System::Windows::Forms::Label^  fcgLBBatBeforeAudioPath;
private: System::Windows::Forms::CheckBox^  fcgCBRunBatBeforeAudio;
private: System::Windows::Forms::ComboBox^  fcgCXAudioPriority;
private: System::Windows::Forms::Label^  fcgLBAudioPriority;


private: System::Windows::Forms::Label^  fcgLBVBAQ;

private: System::Windows::Forms::Label^  fcgLBEncCodec;
private: System::Windows::Forms::ComboBox^  fcgCXEncCodec;




private: System::Windows::Forms::Panel^  fcgPNHEVCLevelProfile;
private: System::Windows::Forms::ComboBox^  fcgCXHEVCLevel;
private: System::Windows::Forms::ComboBox^  fcgCXHEVCProfile;
private: System::Windows::Forms::Label^  fcgLBHEVCLevel;
private: System::Windows::Forms::Label^  fcgLBHEVCProfile;
private: System::Windows::Forms::Panel^  fcgPNBframes;



private: System::Windows::Forms::NumericUpDown^  fcgNURefFrames;
private: System::Windows::Forms::Label^  fcgLBRefFrames;




private: System::Windows::Forms::Panel^  fcgPNH264Features;
private: System::Windows::Forms::CheckBox ^fcgCBVppResize;
private: System::Windows::Forms::GroupBox ^fcggroupBoxResize;
private: System::Windows::Forms::Label^  fcgLBVppResize;



private: System::Windows::Forms::NumericUpDown^  fcgNUResizeH;

private: System::Windows::Forms::NumericUpDown^  fcgNUResizeW;

private: System::Windows::Forms::Button^  fcgBTVideoEncoderPath;
private: System::Windows::Forms::TextBox^  fcgTXVideoEncoderPath;
private: System::Windows::Forms::Label^  fcgLBVideoEncoderPath;

private: System::Windows::Forms::Label^  fcgLBQPMin;
private: System::Windows::Forms::GroupBox^  fcggroupBoxColorMatrix;

private: System::Windows::Forms::ComboBox^  fcgCXTransfer;
private: System::Windows::Forms::ComboBox^  fcgCXColorPrim;
private: System::Windows::Forms::ComboBox^  fcgCXColorMatrix;
private: System::Windows::Forms::Label^  fcgLBTransfer;
private: System::Windows::Forms::Label^  fcgLBColorPrim;
private: System::Windows::Forms::Label^  fcgLBColorMatrix;
private: System::Windows::Forms::CheckBox^  fcgCBFullrange;
private: System::Windows::Forms::Label^  fcgLBFullrange;
private: System::Windows::Forms::ComboBox^  fcgCXVideoFormat;
private: System::Windows::Forms::Label^  fcgLBVideoFormat;
private: System::Windows::Forms::GroupBox ^fcggroupBoxVppDeinterlace;
private: System::Windows::Forms::Label ^fcgLBVppDeinterlace;
private: System::Windows::Forms::ComboBox ^fcgCXVppDeinterlace;
private: System::Windows::Forms::Panel ^fcgPNVppAfs;
private: System::Windows::Forms::TrackBar ^fcgTBVppAfsThreCMotion;
private: System::Windows::Forms::Label ^fcgLBVppAfsThreCMotion;
private: System::Windows::Forms::TrackBar ^fcgTBVppAfsThreYMotion;
private: System::Windows::Forms::Label ^fcgLBVppAfsThreYmotion;
private: System::Windows::Forms::TrackBar ^fcgTBVppAfsThreDeint;
private: System::Windows::Forms::Label ^fcgLBVppAfsThreDeint;
private: System::Windows::Forms::TrackBar ^fcgTBVppAfsThreShift;
private: System::Windows::Forms::Label ^fcgLBVppAfsThreShift;
private: System::Windows::Forms::TrackBar ^fcgTBVppAfsCoeffShift;
private: System::Windows::Forms::Label ^fcgLBVppAfsCoeffShift;
private: System::Windows::Forms::Label^  fcgLBVppAfsRight;

private: System::Windows::Forms::Label^  fcgLBVppAfsLeft;
private: System::Windows::Forms::Label^  fcgLBVppAfsBottom;



private: System::Windows::Forms::Label ^fcgLBVppAfsUp;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppAfsRight;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppAfsLeft;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppAfsBottom;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppAfsUp;
private: System::Windows::Forms::TrackBar ^fcgTBVppAfsMethodSwitch;
private: System::Windows::Forms::CheckBox ^fcgCBVppAfs24fps;
private: System::Windows::Forms::CheckBox ^fcgCBVppAfsTune;
private: System::Windows::Forms::CheckBox ^fcgCBVppAfsSmooth;
private: System::Windows::Forms::CheckBox ^fcgCBVppAfsDrop;
private: System::Windows::Forms::CheckBox ^fcgCBVppAfsShift;
private: System::Windows::Forms::Label ^fcgLBVppAfsAnalyze;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppAfsThreCMotion;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppAfsThreShift;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppAfsThreDeint;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppAfsThreYMotion;
private: System::Windows::Forms::Label ^fcgLBVppAfsMethodSwitch;
private: System::Windows::Forms::ComboBox ^fcgCXVppAfsAnalyze;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppAfsCoeffShift;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppAfsMethodSwitch;
private: System::Windows::Forms::ComboBox ^fcgCXVppResizeAlg;
private: System::Windows::Forms::CheckBox ^fcgCBPSNR;
private: System::Windows::Forms::CheckBox ^fcgCBSSIM;
private: System::Windows::Forms::Panel ^fcgPNAudioExt;
private: System::Windows::Forms::ComboBox ^fcgCXAudioDelayCut;
private: System::Windows::Forms::Label ^fcgLBAudioDelayCut;
private: System::Windows::Forms::Label ^fcgCBAudioEncTiming;
private: System::Windows::Forms::ComboBox ^fcgCXAudioEncTiming;
private: System::Windows::Forms::ComboBox ^fcgCXAudioTempDir;
private: System::Windows::Forms::TextBox ^fcgTXCustomAudioTempDir;
private: System::Windows::Forms::Button ^fcgBTCustomAudioTempDir;
private: System::Windows::Forms::CheckBox ^fcgCBAudioUsePipe;
private: System::Windows::Forms::NumericUpDown ^fcgNUAudioBitrate;
private: System::Windows::Forms::CheckBox ^fcgCBAudio2pass;
private: System::Windows::Forms::ComboBox ^fcgCXAudioEncMode;
private: System::Windows::Forms::Label ^fcgLBAudioEncMode;
private: System::Windows::Forms::Button ^fcgBTAudioEncoderPath;
private: System::Windows::Forms::TextBox ^fcgTXAudioEncoderPath;
private: System::Windows::Forms::Label ^fcgLBAudioEncoderPath;
private: System::Windows::Forms::CheckBox ^fcgCBAudioOnly;
private: System::Windows::Forms::ComboBox ^fcgCXAudioEncoder;
private: System::Windows::Forms::Label ^fcgLBAudioTemp;
private: System::Windows::Forms::Label ^fcgLBAudioBitrate;
private: System::Windows::Forms::Panel ^fcgPNAudioInternal;
private: System::Windows::Forms::Label ^fcgLBAudioBitrateInternal;
private: System::Windows::Forms::NumericUpDown ^fcgNUAudioBitrateInternal;
private: System::Windows::Forms::ComboBox ^fcgCXAudioEncModeInternal;
private: System::Windows::Forms::Label^  fcgLBAudioEncModeInternal;

private: System::Windows::Forms::ComboBox ^fcgCXAudioEncoderInternal;
private: System::Windows::Forms::CheckBox ^fcgCBAudioUseExt;
private: System::Windows::Forms::TabPage ^fcgtabPageInternal;
private: System::Windows::Forms::ComboBox ^fcgCXInternalCmdEx;
private: System::Windows::Forms::Label ^fcgLBInternalCmdEx;
private: System::Windows::Forms::GroupBox ^fcggroupBoxVppDetailEnahance;
private: System::Windows::Forms::ComboBox ^fcgCXVppDetailEnhance;
private: System::Windows::Forms::Panel ^fcgPNVppEdgelevel;
private: System::Windows::Forms::Label ^fcgLBVppEdgelevelWhite;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppEdgelevelWhite;
private: System::Windows::Forms::Label ^fcgLBVppEdgelevelThreshold;
private: System::Windows::Forms::Label ^fcgLBVppEdgelevelBlack;
private: System::Windows::Forms::Label ^fcgLBVppEdgelevelStrength;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppEdgelevelThreshold;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppEdgelevelBlack;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppEdgelevelStrength;


private: System::Windows::Forms::Panel ^fcgPNVppUnsharp;
private: System::Windows::Forms::Label ^fcgLBVppUnsharpThreshold;
private: System::Windows::Forms::Label ^fcgLBVppUnsharpWeight;
private: System::Windows::Forms::Label ^fcgLBVppUnsharpRadius;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppUnsharpThreshold;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppUnsharpWeight;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppUnsharpRadius;
private: System::Windows::Forms::GroupBox ^fcggroupBoxVppDenoise;
private: System::Windows::Forms::Panel ^fcgPNVppDenoiseSmooth;
private: System::Windows::Forms::Label ^fcgLBVppDenoiseSmoothQP;
private: System::Windows::Forms::Label ^fcgLBVppDenoiseSmoothQuality;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppDenoiseSmoothQP;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppDenoiseSmoothQuality;
private: System::Windows::Forms::Panel ^fcgPNVppDenoiseKnn;
private: System::Windows::Forms::Label ^fcgLBVppDenoiseKnnThreshold;
private: System::Windows::Forms::Label ^fcgLBVppDenoiseKnnStrength;
private: System::Windows::Forms::Label ^fcgLBVppDenoiseKnnRadius;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppDenoiseKnnThreshold;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppDenoiseKnnStrength;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppDenoiseKnnRadius;
private: System::Windows::Forms::ComboBox ^fcgCXVppDenoiseMethod;
private: System::Windows::Forms::Panel ^fcgPNVppDenoisePmd;
private: System::Windows::Forms::Label ^fcgLBVppDenoisePmdThreshold;
private: System::Windows::Forms::Label ^fcgLBVppDenoisePmdStrength;
private: System::Windows::Forms::Label ^fcgLBVppDenoisePmdApplyCount;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppDenoisePmdThreshold;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppDenoisePmdStrength;
private: System::Windows::Forms::NumericUpDown ^fcgNUVppDenoisePmdApplyCount;




















private: System::Windows::Forms::Panel ^fcgPNVppNnedi;
private: System::Windows::Forms::Label ^fcgLBVppNnediErrorType;
private: System::Windows::Forms::ComboBox ^fcgCXVppNnediErrorType;
private: System::Windows::Forms::Label ^fcgLBVppNnediPrescreen;
private: System::Windows::Forms::ComboBox ^fcgCXVppNnediPrescreen;
private: System::Windows::Forms::Label ^fcgLBVppNnediPrec;
private: System::Windows::Forms::ComboBox ^fcgCXVppNnediPrec;
private: System::Windows::Forms::Label ^fcgLBVppNnediQual;
private: System::Windows::Forms::ComboBox ^fcgCXVppNnediQual;
private: System::Windows::Forms::Label ^fcgLBVppNnediNsize;
private: System::Windows::Forms::ComboBox ^fcgCXVppNnediNsize;
private: System::Windows::Forms::Label ^fcgLBVppNnediNns;
private: System::Windows::Forms::ComboBox ^fcgCXVppNnediNns;
private: System::Windows::Forms::Panel ^fcgPNVppYadif;
private: System::Windows::Forms::Label ^fcgLBVppYadifMode;
private: System::Windows::Forms::ComboBox ^fcgCXVppYadifMode;
private: System::Windows::Forms::ComboBox^ fcgCXBitdepth;
private: System::Windows::Forms::Label^ fcgLBBitdepth;


private: System::Windows::Forms::Panel^  fcgPNVppWarpsharp;
private: System::Windows::Forms::Label^  fcgLBVppWarpsharpDepth;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppWarpsharpDepth;
private: System::Windows::Forms::Label^  fcgLBVppWarpsharpThreshold;
private: System::Windows::Forms::Label^  fcgLBVppWarpsharpType;
private: System::Windows::Forms::Label^  fcgLBVppWarpsharpBlur;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppWarpsharpThreshold;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppWarpsharpType;

private: System::Windows::Forms::NumericUpDown^  fcgNUVppWarpsharpBlur;


private: System::Windows::Forms::Panel^  fcgPNVppDenoiseConv3D;
private: System::Windows::Forms::ComboBox^  fcgCXVppDenoiseConv3DMatrix;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseConv3DMatrix;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseConv3DThreshTemporal;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseConv3DThreshSpatial;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseConv3DThreshCTemporal;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseConv3DThreshCSpatial;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDenoiseConv3DThreshCTemporal;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDenoiseConv3DThreshCSpatial;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseConv3DThreshYTemporal;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseConv3DThreshYSpatial;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDenoiseConv3DThreshYTemporal;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDenoiseConv3DThreshYSpatial;
private: System::Windows::Forms::Panel^  fcgPNHideToolStripBorder;
private: System::Windows::Forms::TabPage^  tabPageExOpt;
private: System::Windows::Forms::CheckBox^  fcgCBTimerPeriodTuning;
private: System::Windows::Forms::CheckBox^  fcgCBAuoTcfileout;
private: System::Windows::Forms::Label^  fcgLBTempDir;
private: System::Windows::Forms::Button^  fcgBTCustomTempDir;
private: System::Windows::Forms::TextBox^  fcgTXCustomTempDir;
private: System::Windows::Forms::ComboBox^  fcgCXTempDir;
private: System::Windows::Forms::GroupBox^  fcggroupBoxCmdEx;
private: System::Windows::Forms::TextBox^  fcgTXCmdEx;
private: System::Windows::Forms::TabPage^  tabPageVideoEnc2;
private: System::Windows::Forms::CheckBox^  fcgCBPreAnalysis;
private: System::Windows::Forms::GroupBox^  fcggroupBoxPreAnalysis;
private: System::Windows::Forms::Label^  fcgLBPASS;
private: System::Windows::Forms::ComboBox^  fcgCXPASS;


private: System::Windows::Forms::Label^  fcgLBPASC;
private: System::Windows::Forms::NumericUpDown^  fcgNUPALookahead;



private: System::Windows::Forms::ComboBox^  fcgCXPASC;
private: System::Windows::Forms::Label^  fcgLBPAMotionQuality;

private: System::Windows::Forms::ComboBox^  fcgCXPAMotionQuality;
private: System::Windows::Forms::Label^  fcgLBPALookahead;




private: System::Windows::Forms::ComboBox^  fcgCXPATAQ;
private: System::Windows::Forms::Label^  fcgLBPATAQ;
private: System::Windows::Forms::ComboBox^  fcgCXPAPAQ;
private: System::Windows::Forms::Label^  fcgLBPAPAQ;
private: System::Windows::Forms::ComboBox^  fcgCXPACAQ;
private: System::Windows::Forms::Label^  fcgLBPACAQ;
private: System::Windows::Forms::ComboBox^  fcgCXPAActivityType;
private: System::Windows::Forms::Label^  fcgLBPAActivityType;
private: System::Windows::Forms::CheckBox^  fcgCBPreEncode;
private: System::Windows::Forms::ToolStripDropDownButton^  fcgTSLanguage;
private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator1;
private: System::Windows::Forms::Label^ fcgLBDevice;
private: System::Windows::Forms::ComboBox^ fcgCXDevice;
private: System::Windows::Forms::Panel^  fcgPNAV1LevelProfile;
private: System::Windows::Forms::ComboBox^  fcgCXAV1Level;
private: System::Windows::Forms::ComboBox^  fcgCXAV1Profile;
private: System::Windows::Forms::Label^  fcgLBAV1Level;
private: System::Windows::Forms::Label^  fcgLBAV1Profile;
private: System::Windows::Forms::Panel^  fcgPNVppPreProcess;

private: System::Windows::Forms::Label^  fcgLBVppPreProcessAdaptiveFilter;
private: System::Windows::Forms::CheckBox^  fcgCBVppPreProcessAdaptiveFilter;
private: System::Windows::Forms::Label^  fcgLBVppPreProcessSensitivity;
private: System::Windows::Forms::Label^  fcgLBVppPreProcessStrength;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppPreProcessSensitivity;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppPreProcessStrength;
private: System::Windows::Forms::Panel^  fcgPNVppDetailEnhance;
private: System::Windows::Forms::Label^  fcgLBVppDetailEnhanceRadius;
private: System::Windows::Forms::Label^  fcgLBVppDetailEnhanceAttenuation;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDetailEnhanceRadius;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDetailEnhanceAttenuation;
private: System::Windows::Forms::Panel^  fcgPNVppDenoiseDct;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDenoiseDctSigma;
private: System::Windows::Forms::ComboBox^  fcgCXVppDenoiseDctBlockSize;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseDctBlockSize;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseDctSigma;
private: System::Windows::Forms::ComboBox^  fcgCXVppDenoiseDctStep;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseDctStep;
private: System::Windows::Forms::Panel^  fcgPNVppDenoiseNLMeans;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDenoiseNLMeansH;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseNLMeansH;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDenoiseNLMeansSigma;
private: System::Windows::Forms::ComboBox^  fcgCXVppDenoiseNLMeansSearch;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseNLMeansSearch;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseNLMeansSigma;
private: System::Windows::Forms::ComboBox^  fcgCXVppDenoiseNLMeansPatch;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseNLMeansPatch;
private: System::Windows::Forms::Panel^  fcgPNVppDecomb;
private: System::Windows::Forms::CheckBox^  fcgCBVppDecombBlend;
private: System::Windows::Forms::CheckBox^  fcgCBVppDecombFull;
private: System::Windows::Forms::Label^  fcgLBVppDecombDthreshold;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDecombDthreshold;
private: System::Windows::Forms::Label^  fcgLBVppDecombThreshold;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDecombThreshold;
private: System::Windows::Forms::Panel^  fcgPNVppDenoiseFFT3D;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseFFT3DTemporal;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseFFT3DPrecision;
private: System::Windows::Forms::ComboBox^  fcgCXVppDenoiseFFT3DPrecision;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDenoiseFFT3DOverlap;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDenoiseFFT3DAmount;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseFFT3DOverlap;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDenoiseFFT3DSigma;
private: System::Windows::Forms::ComboBox^  fcgCXVppDenoiseFFT3DTemporal;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseFFT3DAmount;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseFFT3DBlockSize;
private: System::Windows::Forms::ComboBox^  fcgCXVppDenoiseFFT3DBlockSize;
private: System::Windows::Forms::Label^  fcgLBVppDenoiseFFT3DSigma;
private: System::Windows::Forms::GroupBox^  fcggroupBoxVppDeband;
private: System::Windows::Forms::Panel^  fcgPNVppLibplaceboDeband;
private: System::Windows::Forms::Label^  fcgLBVppLibplaceboDebandLUTSize;
private: System::Windows::Forms::ComboBox^  fcgCXVppLibplaceboDebandLUTSize;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppLibplaceboDebandRadius;
private: System::Windows::Forms::Label^  fcgLBVppLibplaceboDebandRadius;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppLibplaceboDebandThreshold;
private: System::Windows::Forms::Label^  fcgLBVppLibplaceboDebandDither;
private: System::Windows::Forms::Label^  fcgLBVppLibplaceboDebandGrainC;
private: System::Windows::Forms::Label^  fcgLBVppLibplaceboDebandGrainY;
private: System::Windows::Forms::Label^  fcgLBVppLibplaceboDebandGrain;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppLibplaceboDebandGrainC;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppLibplaceboDebandGrainY;
private: System::Windows::Forms::Label^  fcgLBVppLibplaceboDebandThreshold;
private: System::Windows::Forms::Label^  fcgLBVppLibplaceboDebandIteration;
private: System::Windows::Forms::ComboBox^  fcgCXVppLibplaceboDebandDither;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppLibplaceboDebandIteration;
private: System::Windows::Forms::ComboBox^  fcgCXVppDeband;
private: System::Windows::Forms::Panel^  fcgPNVppDeband;
private: System::Windows::Forms::CheckBox^  fcgCBVppDebandRandEachFrame;
private: System::Windows::Forms::CheckBox^  fcgCBVppDebandBlurFirst;
private: System::Windows::Forms::Label^  fcgLBVppDebandSample;
private: System::Windows::Forms::Label^  fcgLBVppDebandDitherC;
private: System::Windows::Forms::Label^  fcgLBVppDebandDitherY;
private: System::Windows::Forms::Label^  fcgLBVppDebandDither;
private: System::Windows::Forms::Label^  fcgLBVppDebandThreCr;
private: System::Windows::Forms::Label^  fcgLBVppDebandThreCb;
private: System::Windows::Forms::Label^  fcgLBVppDebandThreY;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDebandDitherC;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDebandDitherY;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDebandThreCr;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDebandThreCb;
private: System::Windows::Forms::Label^  fcgLBVppDebandThreshold;
private: System::Windows::Forms::Label^  fcgLBVppDebandRange;
private: System::Windows::Forms::ComboBox^  fcgCXVppDebandSample;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDebandThreY;
private: System::Windows::Forms::NumericUpDown^  fcgNUVppDebandRange;
private: System::Windows::Forms::Panel^  fcgPNQVBR;

private: System::Windows::Forms::Label^  fcgLBQVBRQuality;
private: System::Windows::Forms::NumericUpDown^  fcgNUQVBRQuality;
private: System::Windows::Forms::Label^  label1;
private: System::Windows::Forms::ComboBox^  fcgCXMotionEst;
private: System::Windows::Forms::Label^  fcgLBMotionEst;
private: System::Windows::Forms::NumericUpDown^  fcgNUSlices;
private: System::Windows::Forms::Label^  fcgLBSlices;
private: System::Windows::Forms::ComboBox^  fcgCXVBAQ;
private: System::Windows::Forms::Panel^  fcgPNH264LevelProfile;






































































































    private: System::ComponentModel::IContainer^  components;




    private:
        /// <summary>
        /// 必要なデザイナ変数です。
        /// </summary>


#pragma region Windows Form Designer generated code
        /// <summary>
        /// デザイナ サポートに必要なメソッドです。このメソッドの内容を
        /// コード エディタで変更しないでください。
        /// </summary>
        void InitializeComponent(void)
        {
            this->components = (gcnew System::ComponentModel::Container());
            System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(frmConfig::typeid));
            this->fcgtoolStripSettings = (gcnew System::Windows::Forms::ToolStrip());
            this->fcgTSBSave = (gcnew System::Windows::Forms::ToolStripButton());
            this->fcgTSBSaveNew = (gcnew System::Windows::Forms::ToolStripButton());
            this->fcgTSBDelete = (gcnew System::Windows::Forms::ToolStripButton());
            this->fcgtoolStripSeparator1 = (gcnew System::Windows::Forms::ToolStripSeparator());
            this->fcgTSSettings = (gcnew System::Windows::Forms::ToolStripDropDownButton());
            this->fcgTSBBitrateCalc = (gcnew System::Windows::Forms::ToolStripButton());
            this->toolStripSeparator2 = (gcnew System::Windows::Forms::ToolStripSeparator());
            this->fcgTSLanguage = (gcnew System::Windows::Forms::ToolStripDropDownButton());
            this->toolStripSeparator1 = (gcnew System::Windows::Forms::ToolStripSeparator());
            this->fcgTSBOtherSettings = (gcnew System::Windows::Forms::ToolStripButton());
            this->fcgTSLSettingsNotes = (gcnew System::Windows::Forms::ToolStripLabel());
            this->fcgTSTSettingsNotes = (gcnew System::Windows::Forms::ToolStripTextBox());
            this->fcgtabControlMux = (gcnew System::Windows::Forms::TabControl());
            this->fcgtabPageMP4 = (gcnew System::Windows::Forms::TabPage());
            this->fcgBTMP4RawPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXMP4RawPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBMP4RawPath = (gcnew System::Windows::Forms::Label());
            this->fcgCBMP4MuxApple = (gcnew System::Windows::Forms::CheckBox());
            this->fcgBTMP4BoxTempDir = (gcnew System::Windows::Forms::Button());
            this->fcgTXMP4BoxTempDir = (gcnew System::Windows::Forms::TextBox());
            this->fcgCXMP4BoxTempDir = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBMP4BoxTempDir = (gcnew System::Windows::Forms::Label());
            this->fcgBTTC2MP4Path = (gcnew System::Windows::Forms::Button());
            this->fcgTXTC2MP4Path = (gcnew System::Windows::Forms::TextBox());
            this->fcgBTMP4MuxerPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXMP4MuxerPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBTC2MP4Path = (gcnew System::Windows::Forms::Label());
            this->fcgLBMP4MuxerPath = (gcnew System::Windows::Forms::Label());
            this->fcgCXMP4CmdEx = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBMP4CmdEx = (gcnew System::Windows::Forms::Label());
            this->fcgCBMP4MuxerExt = (gcnew System::Windows::Forms::CheckBox());
            this->fcgtabPageMKV = (gcnew System::Windows::Forms::TabPage());
            this->fcgBTMKVMuxerPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXMKVMuxerPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBMKVMuxerPath = (gcnew System::Windows::Forms::Label());
            this->fcgCXMKVCmdEx = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBMKVMuxerCmdEx = (gcnew System::Windows::Forms::Label());
            this->fcgCBMKVMuxerExt = (gcnew System::Windows::Forms::CheckBox());
            this->fcgtabPageMux = (gcnew System::Windows::Forms::TabPage());
            this->fcgCXMuxPriority = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBMuxPriority = (gcnew System::Windows::Forms::Label());
            this->fcgCBMuxMinimize = (gcnew System::Windows::Forms::CheckBox());
            this->fcgtabPageBat = (gcnew System::Windows::Forms::TabPage());
            this->fcgLBBatAfterString = (gcnew System::Windows::Forms::Label());
            this->fcgLBBatBeforeString = (gcnew System::Windows::Forms::Label());
            this->fcgPNSeparator = (gcnew System::Windows::Forms::Panel());
            this->fcgBTBatBeforePath = (gcnew System::Windows::Forms::Button());
            this->fcgTXBatBeforePath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBBatBeforePath = (gcnew System::Windows::Forms::Label());
            this->fcgCBWaitForBatBefore = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBRunBatBefore = (gcnew System::Windows::Forms::CheckBox());
            this->fcgBTBatAfterPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXBatAfterPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBBatAfterPath = (gcnew System::Windows::Forms::Label());
            this->fcgCBWaitForBatAfter = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBRunBatAfter = (gcnew System::Windows::Forms::CheckBox());
            this->fcgtabPageInternal = (gcnew System::Windows::Forms::TabPage());
            this->fcgCXInternalCmdEx = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBInternalCmdEx = (gcnew System::Windows::Forms::Label());
            this->fcgTXCmd = (gcnew System::Windows::Forms::TextBox());
            this->fcgBTCancel = (gcnew System::Windows::Forms::Button());
            this->fcgBTOK = (gcnew System::Windows::Forms::Button());
            this->fcgBTDefault = (gcnew System::Windows::Forms::Button());
            this->fcgLBVersionDate = (gcnew System::Windows::Forms::Label());
            this->fcgLBVersion = (gcnew System::Windows::Forms::Label());
            this->fcgfolderBrowserTemp = (gcnew System::Windows::Forms::FolderBrowserDialog());
            this->fcgOpenFileDialog = (gcnew System::Windows::Forms::OpenFileDialog());
            this->fcgTTEx = (gcnew System::Windows::Forms::ToolTip(this->components));
            this->fcgtabControlVCE = (gcnew System::Windows::Forms::TabControl());
            this->tabPageVideoEnc = (gcnew System::Windows::Forms::TabPage());
            this->label1 = (gcnew System::Windows::Forms::Label());
            this->fcgPNQVBR = (gcnew System::Windows::Forms::Panel());
            this->fcgLBQVBRQuality = (gcnew System::Windows::Forms::Label());
            this->fcgNUQVBRQuality = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBDevice = (gcnew System::Windows::Forms::Label());
            this->fcgCXDevice = (gcnew System::Windows::Forms::ComboBox());
            this->fcgPNHEVCLevelProfile = (gcnew System::Windows::Forms::Panel());
            this->fcgPNAV1LevelProfile = (gcnew System::Windows::Forms::Panel());
            this->fcgCXAV1Level = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXAV1Profile = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBAV1Level = (gcnew System::Windows::Forms::Label());
            this->fcgLBAV1Profile = (gcnew System::Windows::Forms::Label());
            this->fcgCXHEVCLevel = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXHEVCProfile = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBHEVCLevel = (gcnew System::Windows::Forms::Label());
            this->fcgLBHEVCProfile = (gcnew System::Windows::Forms::Label());
            this->fcgCXBitdepth = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBBitdepth = (gcnew System::Windows::Forms::Label());
            this->fcgCXVideoFormat = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVideoFormat = (gcnew System::Windows::Forms::Label());
            this->fcggroupBoxColorMatrix = (gcnew System::Windows::Forms::GroupBox());
            this->fcgCBFullrange = (gcnew System::Windows::Forms::CheckBox());
            this->fcgLBFullrange = (gcnew System::Windows::Forms::Label());
            this->fcgCXTransfer = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXColorPrim = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXColorMatrix = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBTransfer = (gcnew System::Windows::Forms::Label());
            this->fcgLBColorPrim = (gcnew System::Windows::Forms::Label());
            this->fcgLBColorMatrix = (gcnew System::Windows::Forms::Label());
            this->fcgLBQPMin = (gcnew System::Windows::Forms::Label());
            this->fcgBTVideoEncoderPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXVideoEncoderPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBVideoEncoderPath = (gcnew System::Windows::Forms::Label());
            this->fcgNURefFrames = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBRefFrames = (gcnew System::Windows::Forms::Label());
            this->fcgLBEncCodec = (gcnew System::Windows::Forms::Label());
            this->fcgCXEncCodec = (gcnew System::Windows::Forms::ComboBox());
            this->fcgNUVBVBufSize = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBSkipFrame = (gcnew System::Windows::Forms::Label());
            this->fcgLBVBVBufSize = (gcnew System::Windows::Forms::Label());
            this->fcgLBVBVBufSizeKbps = (gcnew System::Windows::Forms::Label());
            this->fcgCBSkipFrame = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBAFS = (gcnew System::Windows::Forms::CheckBox());
            this->fcgNUQPMax = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBQPMax = (gcnew System::Windows::Forms::Label());
            this->fcgLBQPMinMAX = (gcnew System::Windows::Forms::Label());
            this->fcgNUQPMin = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBQualityPreset = (gcnew System::Windows::Forms::Label());
            this->fcgCXQualityPreset = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBDeblock = (gcnew System::Windows::Forms::Label());
            this->fcgpictureBoxVCEEnabled = (gcnew System::Windows::Forms::PictureBox());
            this->fcgpictureBoxVCEDisabled = (gcnew System::Windows::Forms::PictureBox());
            this->fcgCBDeblock = (gcnew System::Windows::Forms::CheckBox());
            this->fcgLBGOPLengthAuto = (gcnew System::Windows::Forms::Label());
            this->fcgGroupBoxAspectRatio = (gcnew System::Windows::Forms::GroupBox());
            this->fcgLBAspectRatio = (gcnew System::Windows::Forms::Label());
            this->fcgNUAspectRatioY = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUAspectRatioX = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCXAspectRatio = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXInterlaced = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBInterlaced = (gcnew System::Windows::Forms::Label());
            this->fcgNUGopLength = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBGOPLength = (gcnew System::Windows::Forms::Label());
            this->fcgCXCodecLevel = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXCodecProfile = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBCodecLevel = (gcnew System::Windows::Forms::Label());
            this->fcgLBCodecProfile = (gcnew System::Windows::Forms::Label());
            this->fcgLBEncMode = (gcnew System::Windows::Forms::Label());
            this->fcgCXEncMode = (gcnew System::Windows::Forms::ComboBox());
            this->fcgPNBframes = (gcnew System::Windows::Forms::Panel());
            this->fcgLBBframes = (gcnew System::Windows::Forms::Label());
            this->fcgNUBframes = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCBBPyramid = (gcnew System::Windows::Forms::CheckBox());
            this->fcgLBBPyramid = (gcnew System::Windows::Forms::Label());
            this->fcgLBBDeltaQP = (gcnew System::Windows::Forms::Label());
            this->fcgNUBDeltaQP = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBBRefDeltaQP = (gcnew System::Windows::Forms::Label());
            this->fcgNUBRefDeltaQP = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgPNH264Features = (gcnew System::Windows::Forms::Panel());
            this->fcgCXVBAQ = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVBAQ = (gcnew System::Windows::Forms::Label());
            this->fcgPNBitrate = (gcnew System::Windows::Forms::Panel());
            this->fcgLBBitrate = (gcnew System::Windows::Forms::Label());
            this->fcgNUBitrate = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBBitrate2 = (gcnew System::Windows::Forms::Label());
            this->fcgNUMaxkbps = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBMaxkbps = (gcnew System::Windows::Forms::Label());
            this->fcgLBMaxBitrate2 = (gcnew System::Windows::Forms::Label());
            this->fcgPNQP = (gcnew System::Windows::Forms::Panel());
            this->fcgLBQPI = (gcnew System::Windows::Forms::Label());
            this->fcgNUQPI = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUQPP = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUQPB = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBQPP = (gcnew System::Windows::Forms::Label());
            this->fcgLBQPB = (gcnew System::Windows::Forms::Label());
            this->tabPageVideoEnc2 = (gcnew System::Windows::Forms::TabPage());
            this->fcgCXMotionEst = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBMotionEst = (gcnew System::Windows::Forms::Label());
            this->fcgNUSlices = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBSlices = (gcnew System::Windows::Forms::Label());
            this->fcgCBPreEncode = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBPreAnalysis = (gcnew System::Windows::Forms::CheckBox());
            this->fcggroupBoxPreAnalysis = (gcnew System::Windows::Forms::GroupBox());
            this->fcgLBPAMotionQuality = (gcnew System::Windows::Forms::Label());
            this->fcgCXPAMotionQuality = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBPALookahead = (gcnew System::Windows::Forms::Label());
            this->fcgCXPATAQ = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBPATAQ = (gcnew System::Windows::Forms::Label());
            this->fcgCXPAPAQ = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBPAPAQ = (gcnew System::Windows::Forms::Label());
            this->fcgCXPACAQ = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBPACAQ = (gcnew System::Windows::Forms::Label());
            this->fcgCXPAActivityType = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBPAActivityType = (gcnew System::Windows::Forms::Label());
            this->fcgLBPASS = (gcnew System::Windows::Forms::Label());
            this->fcgCXPASS = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBPASC = (gcnew System::Windows::Forms::Label());
            this->fcgNUPALookahead = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCXPASC = (gcnew System::Windows::Forms::ComboBox());
            this->tabPageVpp = (gcnew System::Windows::Forms::TabPage());
            this->fcggroupBoxVppDeband = (gcnew System::Windows::Forms::GroupBox());
            this->fcgPNVppLibplaceboDeband = (gcnew System::Windows::Forms::Panel());
            this->fcgLBVppLibplaceboDebandLUTSize = (gcnew System::Windows::Forms::Label());
            this->fcgCXVppLibplaceboDebandLUTSize = (gcnew System::Windows::Forms::ComboBox());
            this->fcgNUVppLibplaceboDebandRadius = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBVppLibplaceboDebandRadius = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppLibplaceboDebandThreshold = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBVppLibplaceboDebandDither = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppLibplaceboDebandGrainC = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppLibplaceboDebandGrainY = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppLibplaceboDebandGrain = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppLibplaceboDebandGrainC = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppLibplaceboDebandGrainY = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBVppLibplaceboDebandThreshold = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppLibplaceboDebandIteration = (gcnew System::Windows::Forms::Label());
            this->fcgCXVppLibplaceboDebandDither = (gcnew System::Windows::Forms::ComboBox());
            this->fcgNUVppLibplaceboDebandIteration = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCXVppDeband = (gcnew System::Windows::Forms::ComboBox());
            this->fcgPNVppDeband = (gcnew System::Windows::Forms::Panel());
            this->fcgCBVppDebandRandEachFrame = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBVppDebandBlurFirst = (gcnew System::Windows::Forms::CheckBox());
            this->fcgLBVppDebandSample = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDebandDitherC = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDebandDitherY = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDebandDither = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDebandThreCr = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDebandThreCb = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDebandThreY = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppDebandDitherC = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppDebandDitherY = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppDebandThreCr = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppDebandThreCb = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBVppDebandThreshold = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDebandRange = (gcnew System::Windows::Forms::Label());
            this->fcgCXVppDebandSample = (gcnew System::Windows::Forms::ComboBox());
            this->fcgNUVppDebandThreY = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppDebandRange = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcggroupBoxVppDetailEnahance = (gcnew System::Windows::Forms::GroupBox());
            this->fcgCXVppDetailEnhance = (gcnew System::Windows::Forms::ComboBox());
            this->fcgPNVppWarpsharp = (gcnew System::Windows::Forms::Panel());
            this->fcgLBVppWarpsharpDepth = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppWarpsharpDepth = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBVppWarpsharpThreshold = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppWarpsharpType = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppWarpsharpBlur = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppWarpsharpThreshold = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppWarpsharpType = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppWarpsharpBlur = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgPNVppDetailEnhance = (gcnew System::Windows::Forms::Panel());
            this->fcgLBVppDetailEnhanceRadius = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDetailEnhanceAttenuation = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppDetailEnhanceRadius = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppDetailEnhanceAttenuation = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgPNVppEdgelevel = (gcnew System::Windows::Forms::Panel());
            this->fcgLBVppEdgelevelWhite = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppEdgelevelWhite = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBVppEdgelevelThreshold = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppEdgelevelBlack = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppEdgelevelStrength = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppEdgelevelThreshold = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppEdgelevelBlack = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppEdgelevelStrength = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgPNVppUnsharp = (gcnew System::Windows::Forms::Panel());
            this->fcgLBVppUnsharpThreshold = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppUnsharpWeight = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppUnsharpRadius = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppUnsharpThreshold = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppUnsharpWeight = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppUnsharpRadius = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcggroupBoxVppDenoise = (gcnew System::Windows::Forms::GroupBox());
            this->fcgPNVppDenoiseFFT3D = (gcnew System::Windows::Forms::Panel());
            this->fcgLBVppDenoiseFFT3DTemporal = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDenoiseFFT3DPrecision = (gcnew System::Windows::Forms::Label());
            this->fcgCXVppDenoiseFFT3DPrecision = (gcnew System::Windows::Forms::ComboBox());
            this->fcgNUVppDenoiseFFT3DOverlap = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppDenoiseFFT3DAmount = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBVppDenoiseFFT3DOverlap = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppDenoiseFFT3DSigma = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCXVppDenoiseFFT3DTemporal = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVppDenoiseFFT3DAmount = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDenoiseFFT3DBlockSize = (gcnew System::Windows::Forms::Label());
            this->fcgCXVppDenoiseFFT3DBlockSize = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVppDenoiseFFT3DSigma = (gcnew System::Windows::Forms::Label());
            this->fcgPNVppDenoiseNLMeans = (gcnew System::Windows::Forms::Panel());
            this->fcgNUVppDenoiseNLMeansH = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBVppDenoiseNLMeansH = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppDenoiseNLMeansSigma = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCXVppDenoiseNLMeansSearch = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVppDenoiseNLMeansSearch = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDenoiseNLMeansSigma = (gcnew System::Windows::Forms::Label());
            this->fcgCXVppDenoiseNLMeansPatch = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVppDenoiseNLMeansPatch = (gcnew System::Windows::Forms::Label());
            this->fcgPNVppDenoiseDct = (gcnew System::Windows::Forms::Panel());
            this->fcgNUVppDenoiseDctSigma = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCXVppDenoiseDctBlockSize = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVppDenoiseDctBlockSize = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDenoiseDctSigma = (gcnew System::Windows::Forms::Label());
            this->fcgCXVppDenoiseDctStep = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVppDenoiseDctStep = (gcnew System::Windows::Forms::Label());
            this->fcgPNVppPreProcess = (gcnew System::Windows::Forms::Panel());
            this->fcgLBVppPreProcessAdaptiveFilter = (gcnew System::Windows::Forms::Label());
            this->fcgCBVppPreProcessAdaptiveFilter = (gcnew System::Windows::Forms::CheckBox());
            this->fcgLBVppPreProcessSensitivity = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppPreProcessStrength = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppPreProcessSensitivity = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppPreProcessStrength = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCXVppDenoiseMethod = (gcnew System::Windows::Forms::ComboBox());
            this->fcgPNVppDenoisePmd = (gcnew System::Windows::Forms::Panel());
            this->fcgLBVppDenoisePmdThreshold = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDenoisePmdStrength = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDenoisePmdApplyCount = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppDenoisePmdThreshold = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppDenoisePmdStrength = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppDenoisePmdApplyCount = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgPNVppDenoiseConv3D = (gcnew System::Windows::Forms::Panel());
            this->fcgCXVppDenoiseConv3DMatrix = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVppDenoiseConv3DMatrix = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDenoiseConv3DThreshTemporal = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDenoiseConv3DThreshSpatial = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDenoiseConv3DThreshCTemporal = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDenoiseConv3DThreshCSpatial = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppDenoiseConv3DThreshCTemporal = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppDenoiseConv3DThreshCSpatial = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBVppDenoiseConv3DThreshYTemporal = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDenoiseConv3DThreshYSpatial = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppDenoiseConv3DThreshYTemporal = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppDenoiseConv3DThreshYSpatial = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgPNVppDenoiseSmooth = (gcnew System::Windows::Forms::Panel());
            this->fcgLBVppDenoiseSmoothQP = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDenoiseSmoothQuality = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppDenoiseSmoothQP = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppDenoiseSmoothQuality = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgPNVppDenoiseKnn = (gcnew System::Windows::Forms::Panel());
            this->fcgLBVppDenoiseKnnThreshold = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDenoiseKnnStrength = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppDenoiseKnnRadius = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppDenoiseKnnThreshold = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppDenoiseKnnStrength = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppDenoiseKnnRadius = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCBPSNR = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBSSIM = (gcnew System::Windows::Forms::CheckBox());
            this->fcggroupBoxVppDeinterlace = (gcnew System::Windows::Forms::GroupBox());
            this->fcgPNVppDecomb = (gcnew System::Windows::Forms::Panel());
            this->fcgCBVppDecombBlend = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBVppDecombFull = (gcnew System::Windows::Forms::CheckBox());
            this->fcgLBVppDecombDthreshold = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppDecombDthreshold = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBVppDecombThreshold = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppDecombThreshold = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBVppDeinterlace = (gcnew System::Windows::Forms::Label());
            this->fcgCXVppDeinterlace = (gcnew System::Windows::Forms::ComboBox());
            this->fcgPNVppAfs = (gcnew System::Windows::Forms::Panel());
            this->fcgTBVppAfsThreCMotion = (gcnew System::Windows::Forms::TrackBar());
            this->fcgLBVppAfsThreCMotion = (gcnew System::Windows::Forms::Label());
            this->fcgTBVppAfsThreYMotion = (gcnew System::Windows::Forms::TrackBar());
            this->fcgLBVppAfsThreYmotion = (gcnew System::Windows::Forms::Label());
            this->fcgTBVppAfsThreDeint = (gcnew System::Windows::Forms::TrackBar());
            this->fcgLBVppAfsThreDeint = (gcnew System::Windows::Forms::Label());
            this->fcgTBVppAfsThreShift = (gcnew System::Windows::Forms::TrackBar());
            this->fcgLBVppAfsThreShift = (gcnew System::Windows::Forms::Label());
            this->fcgTBVppAfsCoeffShift = (gcnew System::Windows::Forms::TrackBar());
            this->fcgLBVppAfsCoeffShift = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppAfsRight = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppAfsLeft = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppAfsBottom = (gcnew System::Windows::Forms::Label());
            this->fcgLBVppAfsUp = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppAfsRight = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppAfsLeft = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppAfsBottom = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppAfsUp = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgTBVppAfsMethodSwitch = (gcnew System::Windows::Forms::TrackBar());
            this->fcgCBVppAfs24fps = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBVppAfsTune = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBVppAfsSmooth = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBVppAfsDrop = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBVppAfsShift = (gcnew System::Windows::Forms::CheckBox());
            this->fcgLBVppAfsAnalyze = (gcnew System::Windows::Forms::Label());
            this->fcgNUVppAfsThreCMotion = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppAfsThreShift = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppAfsThreDeint = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppAfsThreYMotion = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBVppAfsMethodSwitch = (gcnew System::Windows::Forms::Label());
            this->fcgCXVppAfsAnalyze = (gcnew System::Windows::Forms::ComboBox());
            this->fcgNUVppAfsCoeffShift = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUVppAfsMethodSwitch = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgPNVppYadif = (gcnew System::Windows::Forms::Panel());
            this->fcgLBVppYadifMode = (gcnew System::Windows::Forms::Label());
            this->fcgCXVppYadifMode = (gcnew System::Windows::Forms::ComboBox());
            this->fcgPNVppNnedi = (gcnew System::Windows::Forms::Panel());
            this->fcgLBVppNnediErrorType = (gcnew System::Windows::Forms::Label());
            this->fcgCXVppNnediErrorType = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVppNnediPrescreen = (gcnew System::Windows::Forms::Label());
            this->fcgCXVppNnediPrescreen = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVppNnediPrec = (gcnew System::Windows::Forms::Label());
            this->fcgCXVppNnediPrec = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVppNnediQual = (gcnew System::Windows::Forms::Label());
            this->fcgCXVppNnediQual = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVppNnediNsize = (gcnew System::Windows::Forms::Label());
            this->fcgCXVppNnediNsize = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVppNnediNns = (gcnew System::Windows::Forms::Label());
            this->fcgCXVppNnediNns = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCBVppResize = (gcnew System::Windows::Forms::CheckBox());
            this->fcggroupBoxResize = (gcnew System::Windows::Forms::GroupBox());
            this->fcgCXVppResizeAlg = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVppResize = (gcnew System::Windows::Forms::Label());
            this->fcgNUResizeH = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUResizeW = (gcnew System::Windows::Forms::NumericUpDown());
            this->tabPageExOpt = (gcnew System::Windows::Forms::TabPage());
            this->fcggroupBoxCmdEx = (gcnew System::Windows::Forms::GroupBox());
            this->fcgTXCmdEx = (gcnew System::Windows::Forms::TextBox());
            this->fcgCBTimerPeriodTuning = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBAuoTcfileout = (gcnew System::Windows::Forms::CheckBox());
            this->fcgLBTempDir = (gcnew System::Windows::Forms::Label());
            this->fcgBTCustomTempDir = (gcnew System::Windows::Forms::Button());
            this->fcgTXCustomTempDir = (gcnew System::Windows::Forms::TextBox());
            this->fcgCXTempDir = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCSExeFiles = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
            this->fcgTSExeFileshelp = (gcnew System::Windows::Forms::ToolStripMenuItem());
            this->fcgLBguiExBlog = (gcnew System::Windows::Forms::LinkLabel());
            this->fcgtabControlAudio = (gcnew System::Windows::Forms::TabControl());
            this->fcgtabPageAudioMain = (gcnew System::Windows::Forms::TabPage());
            this->fcgCBAudioUseExt = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBFAWCheck = (gcnew System::Windows::Forms::CheckBox());
            this->fcgPNAudioExt = (gcnew System::Windows::Forms::Panel());
            this->fcgCXAudioDelayCut = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBAudioDelayCut = (gcnew System::Windows::Forms::Label());
            this->fcgCBAudioEncTiming = (gcnew System::Windows::Forms::Label());
            this->fcgCXAudioEncTiming = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXAudioTempDir = (gcnew System::Windows::Forms::ComboBox());
            this->fcgTXCustomAudioTempDir = (gcnew System::Windows::Forms::TextBox());
            this->fcgBTCustomAudioTempDir = (gcnew System::Windows::Forms::Button());
            this->fcgCBAudioUsePipe = (gcnew System::Windows::Forms::CheckBox());
            this->fcgNUAudioBitrate = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCBAudio2pass = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCXAudioEncMode = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBAudioEncMode = (gcnew System::Windows::Forms::Label());
            this->fcgBTAudioEncoderPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXAudioEncoderPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBAudioEncoderPath = (gcnew System::Windows::Forms::Label());
            this->fcgCBAudioOnly = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCXAudioEncoder = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBAudioTemp = (gcnew System::Windows::Forms::Label());
            this->fcgLBAudioBitrate = (gcnew System::Windows::Forms::Label());
            this->fcgPNAudioInternal = (gcnew System::Windows::Forms::Panel());
            this->fcgLBAudioBitrateInternal = (gcnew System::Windows::Forms::Label());
            this->fcgNUAudioBitrateInternal = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCXAudioEncModeInternal = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBAudioEncModeInternal = (gcnew System::Windows::Forms::Label());
            this->fcgCXAudioEncoderInternal = (gcnew System::Windows::Forms::ComboBox());
            this->fcgtabPageAudioOther = (gcnew System::Windows::Forms::TabPage());
            this->panel2 = (gcnew System::Windows::Forms::Panel());
            this->fcgLBBatAfterAudioString = (gcnew System::Windows::Forms::Label());
            this->fcgLBBatBeforeAudioString = (gcnew System::Windows::Forms::Label());
            this->fcgBTBatAfterAudioPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXBatAfterAudioPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBBatAfterAudioPath = (gcnew System::Windows::Forms::Label());
            this->fcgCBRunBatAfterAudio = (gcnew System::Windows::Forms::CheckBox());
            this->panel1 = (gcnew System::Windows::Forms::Panel());
            this->fcgBTBatBeforeAudioPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXBatBeforeAudioPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBBatBeforeAudioPath = (gcnew System::Windows::Forms::Label());
            this->fcgCBRunBatBeforeAudio = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCXAudioPriority = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBAudioPriority = (gcnew System::Windows::Forms::Label());
            this->fcgPNHideToolStripBorder = (gcnew System::Windows::Forms::Panel());
            this->fcgPNH264LevelProfile = (gcnew System::Windows::Forms::Panel());
            this->fcgtoolStripSettings->SuspendLayout();
            this->fcgtabControlMux->SuspendLayout();
            this->fcgtabPageMP4->SuspendLayout();
            this->fcgtabPageMKV->SuspendLayout();
            this->fcgtabPageMux->SuspendLayout();
            this->fcgtabPageBat->SuspendLayout();
            this->fcgtabPageInternal->SuspendLayout();
            this->fcgtabControlVCE->SuspendLayout();
            this->tabPageVideoEnc->SuspendLayout();
            this->fcgPNQVBR->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQVBRQuality))->BeginInit();
            this->fcgPNHEVCLevelProfile->SuspendLayout();
            this->fcgPNAV1LevelProfile->SuspendLayout();
            this->fcggroupBoxColorMatrix->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNURefFrames))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVBVBufSize))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQPMax))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQPMin))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgpictureBoxVCEEnabled))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgpictureBoxVCEDisabled))->BeginInit();
            this->fcgGroupBoxAspectRatio->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAspectRatioY))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAspectRatioX))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUGopLength))->BeginInit();
            this->fcgPNBframes->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUBframes))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUBDeltaQP))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUBRefDeltaQP))->BeginInit();
            this->fcgPNH264Features->SuspendLayout();
            this->fcgPNBitrate->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUBitrate))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUMaxkbps))->BeginInit();
            this->fcgPNQP->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQPI))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQPP))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQPB))->BeginInit();
            this->tabPageVideoEnc2->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUSlices))->BeginInit();
            this->fcggroupBoxPreAnalysis->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUPALookahead))->BeginInit();
            this->tabPageVpp->SuspendLayout();
            this->fcggroupBoxVppDeband->SuspendLayout();
            this->fcgPNVppLibplaceboDeband->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppLibplaceboDebandRadius))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppLibplaceboDebandThreshold))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppLibplaceboDebandGrainC))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppLibplaceboDebandGrainY))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppLibplaceboDebandIteration))->BeginInit();
            this->fcgPNVppDeband->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDebandDitherC))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDebandDitherY))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDebandThreCr))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDebandThreCb))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDebandThreY))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDebandRange))->BeginInit();
            this->fcggroupBoxVppDetailEnahance->SuspendLayout();
            this->fcgPNVppWarpsharp->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppWarpsharpDepth))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppWarpsharpThreshold))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppWarpsharpType))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppWarpsharpBlur))->BeginInit();
            this->fcgPNVppDetailEnhance->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDetailEnhanceRadius))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDetailEnhanceAttenuation))->BeginInit();
            this->fcgPNVppEdgelevel->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppEdgelevelWhite))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppEdgelevelThreshold))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppEdgelevelBlack))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppEdgelevelStrength))->BeginInit();
            this->fcgPNVppUnsharp->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppUnsharpThreshold))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppUnsharpWeight))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppUnsharpRadius))->BeginInit();
            this->fcggroupBoxVppDenoise->SuspendLayout();
            this->fcgPNVppDenoiseFFT3D->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseFFT3DOverlap))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseFFT3DAmount))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseFFT3DSigma))->BeginInit();
            this->fcgPNVppDenoiseNLMeans->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseNLMeansH))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseNLMeansSigma))->BeginInit();
            this->fcgPNVppDenoiseDct->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseDctSigma))->BeginInit();
            this->fcgPNVppPreProcess->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppPreProcessSensitivity))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppPreProcessStrength))->BeginInit();
            this->fcgPNVppDenoisePmd->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoisePmdThreshold))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoisePmdStrength))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoisePmdApplyCount))->BeginInit();
            this->fcgPNVppDenoiseConv3D->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseConv3DThreshCTemporal))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseConv3DThreshCSpatial))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseConv3DThreshYTemporal))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseConv3DThreshYSpatial))->BeginInit();
            this->fcgPNVppDenoiseSmooth->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseSmoothQP))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseSmoothQuality))->BeginInit();
            this->fcgPNVppDenoiseKnn->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseKnnThreshold))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseKnnStrength))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseKnnRadius))->BeginInit();
            this->fcggroupBoxVppDeinterlace->SuspendLayout();
            this->fcgPNVppDecomb->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDecombDthreshold))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDecombThreshold))->BeginInit();
            this->fcgPNVppAfs->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgTBVppAfsThreCMotion))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgTBVppAfsThreYMotion))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgTBVppAfsThreDeint))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgTBVppAfsThreShift))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgTBVppAfsCoeffShift))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsRight))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsLeft))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsBottom))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsUp))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgTBVppAfsMethodSwitch))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsThreCMotion))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsThreShift))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsThreDeint))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsThreYMotion))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsCoeffShift))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsMethodSwitch))->BeginInit();
            this->fcgPNVppYadif->SuspendLayout();
            this->fcgPNVppNnedi->SuspendLayout();
            this->fcggroupBoxResize->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUResizeH))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUResizeW))->BeginInit();
            this->tabPageExOpt->SuspendLayout();
            this->fcggroupBoxCmdEx->SuspendLayout();
            this->fcgCSExeFiles->SuspendLayout();
            this->fcgtabControlAudio->SuspendLayout();
            this->fcgtabPageAudioMain->SuspendLayout();
            this->fcgPNAudioExt->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAudioBitrate))->BeginInit();
            this->fcgPNAudioInternal->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAudioBitrateInternal))->BeginInit();
            this->fcgtabPageAudioOther->SuspendLayout();
            this->fcgPNH264LevelProfile->SuspendLayout();
            this->SuspendLayout();
            // 
            // fcgtoolStripSettings
            // 
            this->fcgtoolStripSettings->ImageScalingSize = System::Drawing::Size(20, 20);
            this->fcgtoolStripSettings->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(12) {
                this->fcgTSBSave,
                    this->fcgTSBSaveNew, this->fcgTSBDelete, this->fcgtoolStripSeparator1, this->fcgTSSettings, this->fcgTSBBitrateCalc, this->toolStripSeparator2,
                    this->fcgTSLanguage, this->toolStripSeparator1, this->fcgTSBOtherSettings, this->fcgTSLSettingsNotes, this->fcgTSTSettingsNotes
            });
            this->fcgtoolStripSettings->Location = System::Drawing::Point(0, 0);
            this->fcgtoolStripSettings->Name = L"fcgtoolStripSettings";
            this->fcgtoolStripSettings->Size = System::Drawing::Size(1512, 34);
            this->fcgtoolStripSettings->TabIndex = 1;
            this->fcgtoolStripSettings->Text = L"toolStrip1";
            // 
            // fcgTSBSave
            // 
            this->fcgTSBSave->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSBSave.Image")));
            this->fcgTSBSave->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSBSave->Name = L"fcgTSBSave";
            this->fcgTSBSave->Size = System::Drawing::Size(122, 29);
            this->fcgTSBSave->Text = L"上書き保存";
            this->fcgTSBSave->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSBSave_Click);
            // 
            // fcgTSBSaveNew
            // 
            this->fcgTSBSaveNew->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSBSaveNew.Image")));
            this->fcgTSBSaveNew->ImageTransparentColor = System::Drawing::Color::Black;
            this->fcgTSBSaveNew->Name = L"fcgTSBSaveNew";
            this->fcgTSBSaveNew->Size = System::Drawing::Size(108, 29);
            this->fcgTSBSaveNew->Text = L"新規保存";
            this->fcgTSBSaveNew->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSBSaveNew_Click);
            // 
            // fcgTSBDelete
            // 
            this->fcgTSBDelete->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSBDelete.Image")));
            this->fcgTSBDelete->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSBDelete->Name = L"fcgTSBDelete";
            this->fcgTSBDelete->Size = System::Drawing::Size(72, 29);
            this->fcgTSBDelete->Text = L"削除";
            this->fcgTSBDelete->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSBDelete_Click);
            // 
            // fcgtoolStripSeparator1
            // 
            this->fcgtoolStripSeparator1->Name = L"fcgtoolStripSeparator1";
            this->fcgtoolStripSeparator1->Size = System::Drawing::Size(6, 34);
            // 
            // fcgTSSettings
            // 
            this->fcgTSSettings->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSSettings.Image")));
            this->fcgTSSettings->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSSettings->Name = L"fcgTSSettings";
            this->fcgTSSettings->Size = System::Drawing::Size(115, 29);
            this->fcgTSSettings->Text = L"プリセット";
            this->fcgTSSettings->DropDownItemClicked += gcnew System::Windows::Forms::ToolStripItemClickedEventHandler(this, &frmConfig::fcgTSSettings_DropDownItemClicked);
            this->fcgTSSettings->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSSettings_Click);
            // 
            // fcgTSBBitrateCalc
            // 
            this->fcgTSBBitrateCalc->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->fcgTSBBitrateCalc->CheckOnClick = true;
            this->fcgTSBBitrateCalc->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->fcgTSBBitrateCalc->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSBBitrateCalc.Image")));
            this->fcgTSBBitrateCalc->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSBBitrateCalc->Name = L"fcgTSBBitrateCalc";
            this->fcgTSBBitrateCalc->Size = System::Drawing::Size(143, 29);
            this->fcgTSBBitrateCalc->Text = L"ビットレート計算機";
            this->fcgTSBBitrateCalc->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgTSBBitrateCalc_CheckedChanged);
            // 
            // toolStripSeparator2
            // 
            this->toolStripSeparator2->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->toolStripSeparator2->Name = L"toolStripSeparator2";
            this->toolStripSeparator2->Size = System::Drawing::Size(6, 34);
            // 
            // fcgTSLanguage
            // 
            this->fcgTSLanguage->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->fcgTSLanguage->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->fcgTSLanguage->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSLanguage.Image")));
            this->fcgTSLanguage->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSLanguage->Name = L"fcgTSLanguage";
            this->fcgTSLanguage->Size = System::Drawing::Size(66, 29);
            this->fcgTSLanguage->Text = L"言語";
            this->fcgTSLanguage->DropDownItemClicked += gcnew System::Windows::Forms::ToolStripItemClickedEventHandler(this, &frmConfig::fcgTSLanguage_DropDownItemClicked);
            // 
            // toolStripSeparator1
            // 
            this->toolStripSeparator1->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->toolStripSeparator1->Name = L"toolStripSeparator1";
            this->toolStripSeparator1->Size = System::Drawing::Size(6, 34);
            // 
            // fcgTSBOtherSettings
            // 
            this->fcgTSBOtherSettings->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->fcgTSBOtherSettings->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->fcgTSBOtherSettings->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgTSBOtherSettings.Image")));
            this->fcgTSBOtherSettings->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSBOtherSettings->Name = L"fcgTSBOtherSettings";
            this->fcgTSBOtherSettings->Size = System::Drawing::Size(113, 29);
            this->fcgTSBOtherSettings->Text = L"その他の設定";
            this->fcgTSBOtherSettings->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSBOtherSettings_Click);
            // 
            // fcgTSLSettingsNotes
            // 
            this->fcgTSLSettingsNotes->DoubleClickEnabled = true;
            this->fcgTSLSettingsNotes->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgTSLSettingsNotes->Margin = System::Windows::Forms::Padding(3, 1, 0, 2);
            this->fcgTSLSettingsNotes->Name = L"fcgTSLSettingsNotes";
            this->fcgTSLSettingsNotes->Overflow = System::Windows::Forms::ToolStripItemOverflow::Never;
            this->fcgTSLSettingsNotes->Size = System::Drawing::Size(69, 31);
            this->fcgTSLSettingsNotes->Text = L"メモ表示";
            this->fcgTSLSettingsNotes->DoubleClick += gcnew System::EventHandler(this, &frmConfig::fcgTSLSettingsNotes_DoubleClick);
            // 
            // fcgTSTSettingsNotes
            // 
            this->fcgTSTSettingsNotes->BackColor = System::Drawing::SystemColors::Window;
            this->fcgTSTSettingsNotes->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgTSTSettingsNotes->Margin = System::Windows::Forms::Padding(3, 0, 1, 0);
            this->fcgTSTSettingsNotes->Name = L"fcgTSTSettingsNotes";
            this->fcgTSTSettingsNotes->Size = System::Drawing::Size(298, 34);
            this->fcgTSTSettingsNotes->Text = L"メモ...";
            this->fcgTSTSettingsNotes->Visible = false;
            this->fcgTSTSettingsNotes->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTSTSettingsNotes_Leave);
            this->fcgTSTSettingsNotes->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &frmConfig::fcgTSTSettingsNotes_KeyDown);
            this->fcgTSTSettingsNotes->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTSTSettingsNotes_TextChanged);
            // 
            // fcgtabControlMux
            // 
            this->fcgtabControlMux->Controls->Add(this->fcgtabPageMP4);
            this->fcgtabControlMux->Controls->Add(this->fcgtabPageMKV);
            this->fcgtabControlMux->Controls->Add(this->fcgtabPageMux);
            this->fcgtabControlMux->Controls->Add(this->fcgtabPageBat);
            this->fcgtabControlMux->Controls->Add(this->fcgtabPageInternal);
            this->fcgtabControlMux->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgtabControlMux->Location = System::Drawing::Point(934, 509);
            this->fcgtabControlMux->Margin = System::Windows::Forms::Padding(5);
            this->fcgtabControlMux->Name = L"fcgtabControlMux";
            this->fcgtabControlMux->SelectedIndex = 0;
            this->fcgtabControlMux->Size = System::Drawing::Size(576, 301);
            this->fcgtabControlMux->TabIndex = 3;
            // 
            // fcgtabPageMP4
            // 
            this->fcgtabPageMP4->Controls->Add(this->fcgBTMP4RawPath);
            this->fcgtabPageMP4->Controls->Add(this->fcgTXMP4RawPath);
            this->fcgtabPageMP4->Controls->Add(this->fcgLBMP4RawPath);
            this->fcgtabPageMP4->Controls->Add(this->fcgCBMP4MuxApple);
            this->fcgtabPageMP4->Controls->Add(this->fcgBTMP4BoxTempDir);
            this->fcgtabPageMP4->Controls->Add(this->fcgTXMP4BoxTempDir);
            this->fcgtabPageMP4->Controls->Add(this->fcgCXMP4BoxTempDir);
            this->fcgtabPageMP4->Controls->Add(this->fcgLBMP4BoxTempDir);
            this->fcgtabPageMP4->Controls->Add(this->fcgBTTC2MP4Path);
            this->fcgtabPageMP4->Controls->Add(this->fcgTXTC2MP4Path);
            this->fcgtabPageMP4->Controls->Add(this->fcgBTMP4MuxerPath);
            this->fcgtabPageMP4->Controls->Add(this->fcgTXMP4MuxerPath);
            this->fcgtabPageMP4->Controls->Add(this->fcgLBTC2MP4Path);
            this->fcgtabPageMP4->Controls->Add(this->fcgLBMP4MuxerPath);
            this->fcgtabPageMP4->Controls->Add(this->fcgCXMP4CmdEx);
            this->fcgtabPageMP4->Controls->Add(this->fcgLBMP4CmdEx);
            this->fcgtabPageMP4->Controls->Add(this->fcgCBMP4MuxerExt);
            this->fcgtabPageMP4->Location = System::Drawing::Point(4, 31);
            this->fcgtabPageMP4->Margin = System::Windows::Forms::Padding(5);
            this->fcgtabPageMP4->Name = L"fcgtabPageMP4";
            this->fcgtabPageMP4->Padding = System::Windows::Forms::Padding(5);
            this->fcgtabPageMP4->Size = System::Drawing::Size(568, 266);
            this->fcgtabPageMP4->TabIndex = 0;
            this->fcgtabPageMP4->Text = L"mp4";
            this->fcgtabPageMP4->UseVisualStyleBackColor = true;
            // 
            // fcgBTMP4RawPath
            // 
            this->fcgBTMP4RawPath->Location = System::Drawing::Point(510, 154);
            this->fcgBTMP4RawPath->Margin = System::Windows::Forms::Padding(5);
            this->fcgBTMP4RawPath->Name = L"fcgBTMP4RawPath";
            this->fcgBTMP4RawPath->Size = System::Drawing::Size(46, 35);
            this->fcgBTMP4RawPath->TabIndex = 23;
            this->fcgBTMP4RawPath->Text = L"...";
            this->fcgBTMP4RawPath->UseVisualStyleBackColor = true;
            this->fcgBTMP4RawPath->Visible = false;
            this->fcgBTMP4RawPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMP4RawMuxerPath_Click);
            // 
            // fcgTXMP4RawPath
            // 
            this->fcgTXMP4RawPath->AllowDrop = true;
            this->fcgTXMP4RawPath->Location = System::Drawing::Point(204, 155);
            this->fcgTXMP4RawPath->Margin = System::Windows::Forms::Padding(5);
            this->fcgTXMP4RawPath->Name = L"fcgTXMP4RawPath";
            this->fcgTXMP4RawPath->Size = System::Drawing::Size(302, 28);
            this->fcgTXMP4RawPath->TabIndex = 22;
            this->fcgTXMP4RawPath->Visible = false;
            this->fcgTXMP4RawPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4RawMuxerPath_TextChanged);
            this->fcgTXMP4RawPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXMP4RawPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            this->fcgTXMP4RawPath->Enter += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4RawPath_Enter);
            this->fcgTXMP4RawPath->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4RawPath_Leave);
            // 
            // fcgLBMP4RawPath
            // 
            this->fcgLBMP4RawPath->AutoSize = true;
            this->fcgLBMP4RawPath->Location = System::Drawing::Point(6, 158);
            this->fcgLBMP4RawPath->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBMP4RawPath->Name = L"fcgLBMP4RawPath";
            this->fcgLBMP4RawPath->Size = System::Drawing::Size(75, 22);
            this->fcgLBMP4RawPath->TabIndex = 21;
            this->fcgLBMP4RawPath->Text = L"～の指定";
            this->fcgLBMP4RawPath->Visible = false;
            // 
            // fcgCBMP4MuxApple
            // 
            this->fcgCBMP4MuxApple->AutoSize = true;
            this->fcgCBMP4MuxApple->Location = System::Drawing::Point(382, 50);
            this->fcgCBMP4MuxApple->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBMP4MuxApple->Name = L"fcgCBMP4MuxApple";
            this->fcgCBMP4MuxApple->Size = System::Drawing::Size(163, 26);
            this->fcgCBMP4MuxApple->TabIndex = 20;
            this->fcgCBMP4MuxApple->Tag = L"chValue";
            this->fcgCBMP4MuxApple->Text = L"Apple形式に対応";
            this->fcgCBMP4MuxApple->UseVisualStyleBackColor = true;
            // 
            // fcgBTMP4BoxTempDir
            // 
            this->fcgBTMP4BoxTempDir->Location = System::Drawing::Point(510, 218);
            this->fcgBTMP4BoxTempDir->Margin = System::Windows::Forms::Padding(5);
            this->fcgBTMP4BoxTempDir->Name = L"fcgBTMP4BoxTempDir";
            this->fcgBTMP4BoxTempDir->Size = System::Drawing::Size(46, 35);
            this->fcgBTMP4BoxTempDir->TabIndex = 8;
            this->fcgBTMP4BoxTempDir->Text = L"...";
            this->fcgBTMP4BoxTempDir->UseVisualStyleBackColor = true;
            this->fcgBTMP4BoxTempDir->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMP4BoxTempDir_Click);
            // 
            // fcgTXMP4BoxTempDir
            // 
            this->fcgTXMP4BoxTempDir->Location = System::Drawing::Point(161, 221);
            this->fcgTXMP4BoxTempDir->Margin = System::Windows::Forms::Padding(5);
            this->fcgTXMP4BoxTempDir->Name = L"fcgTXMP4BoxTempDir";
            this->fcgTXMP4BoxTempDir->Size = System::Drawing::Size(339, 28);
            this->fcgTXMP4BoxTempDir->TabIndex = 7;
            this->fcgTXMP4BoxTempDir->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4BoxTempDir_TextChanged);
            // 
            // fcgCXMP4BoxTempDir
            // 
            this->fcgCXMP4BoxTempDir->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXMP4BoxTempDir->FormattingEnabled = true;
            this->fcgCXMP4BoxTempDir->Location = System::Drawing::Point(217, 179);
            this->fcgCXMP4BoxTempDir->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXMP4BoxTempDir->Name = L"fcgCXMP4BoxTempDir";
            this->fcgCXMP4BoxTempDir->Size = System::Drawing::Size(306, 30);
            this->fcgCXMP4BoxTempDir->TabIndex = 6;
            this->fcgCXMP4BoxTempDir->Tag = L"chValue";
            // 
            // fcgLBMP4BoxTempDir
            // 
            this->fcgLBMP4BoxTempDir->AutoSize = true;
            this->fcgLBMP4BoxTempDir->Location = System::Drawing::Point(37, 182);
            this->fcgLBMP4BoxTempDir->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBMP4BoxTempDir->Name = L"fcgLBMP4BoxTempDir";
            this->fcgLBMP4BoxTempDir->Size = System::Drawing::Size(158, 22);
            this->fcgLBMP4BoxTempDir->TabIndex = 18;
            this->fcgLBMP4BoxTempDir->Text = L"mp4box一時フォルダ";
            // 
            // fcgBTTC2MP4Path
            // 
            this->fcgBTTC2MP4Path->Location = System::Drawing::Point(510, 120);
            this->fcgBTTC2MP4Path->Margin = System::Windows::Forms::Padding(5);
            this->fcgBTTC2MP4Path->Name = L"fcgBTTC2MP4Path";
            this->fcgBTTC2MP4Path->Size = System::Drawing::Size(46, 35);
            this->fcgBTTC2MP4Path->TabIndex = 5;
            this->fcgBTTC2MP4Path->Text = L"...";
            this->fcgBTTC2MP4Path->UseVisualStyleBackColor = true;
            this->fcgBTTC2MP4Path->Visible = false;
            this->fcgBTTC2MP4Path->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTTC2MP4Path_Click);
            // 
            // fcgTXTC2MP4Path
            // 
            this->fcgTXTC2MP4Path->AllowDrop = true;
            this->fcgTXTC2MP4Path->Location = System::Drawing::Point(204, 121);
            this->fcgTXTC2MP4Path->Margin = System::Windows::Forms::Padding(5);
            this->fcgTXTC2MP4Path->Name = L"fcgTXTC2MP4Path";
            this->fcgTXTC2MP4Path->Size = System::Drawing::Size(302, 28);
            this->fcgTXTC2MP4Path->TabIndex = 4;
            this->fcgTXTC2MP4Path->Visible = false;
            this->fcgTXTC2MP4Path->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXTC2MP4Path_TextChanged);
            this->fcgTXTC2MP4Path->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXTC2MP4Path->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            this->fcgTXTC2MP4Path->Enter += gcnew System::EventHandler(this, &frmConfig::fcgTXTC2MP4Path_Enter);
            this->fcgTXTC2MP4Path->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTXTC2MP4Path_Leave);
            // 
            // fcgBTMP4MuxerPath
            // 
            this->fcgBTMP4MuxerPath->Location = System::Drawing::Point(510, 86);
            this->fcgBTMP4MuxerPath->Margin = System::Windows::Forms::Padding(5);
            this->fcgBTMP4MuxerPath->Name = L"fcgBTMP4MuxerPath";
            this->fcgBTMP4MuxerPath->Size = System::Drawing::Size(46, 35);
            this->fcgBTMP4MuxerPath->TabIndex = 3;
            this->fcgBTMP4MuxerPath->Text = L"...";
            this->fcgBTMP4MuxerPath->UseVisualStyleBackColor = true;
            this->fcgBTMP4MuxerPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMP4MuxerPath_Click);
            // 
            // fcgTXMP4MuxerPath
            // 
            this->fcgTXMP4MuxerPath->AllowDrop = true;
            this->fcgTXMP4MuxerPath->Location = System::Drawing::Point(204, 89);
            this->fcgTXMP4MuxerPath->Margin = System::Windows::Forms::Padding(5);
            this->fcgTXMP4MuxerPath->Name = L"fcgTXMP4MuxerPath";
            this->fcgTXMP4MuxerPath->Size = System::Drawing::Size(302, 28);
            this->fcgTXMP4MuxerPath->TabIndex = 2;
            this->fcgTXMP4MuxerPath->Tag = L"";
            this->fcgTXMP4MuxerPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4MuxerPath_TextChanged);
            this->fcgTXMP4MuxerPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXMP4MuxerPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            this->fcgTXMP4MuxerPath->Enter += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4MuxerPath_Enter);
            this->fcgTXMP4MuxerPath->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4MuxerPath_Leave);
            // 
            // fcgLBTC2MP4Path
            // 
            this->fcgLBTC2MP4Path->AutoSize = true;
            this->fcgLBTC2MP4Path->Location = System::Drawing::Point(6, 126);
            this->fcgLBTC2MP4Path->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBTC2MP4Path->Name = L"fcgLBTC2MP4Path";
            this->fcgLBTC2MP4Path->Size = System::Drawing::Size(75, 22);
            this->fcgLBTC2MP4Path->TabIndex = 4;
            this->fcgLBTC2MP4Path->Text = L"～の指定";
            this->fcgLBTC2MP4Path->Visible = false;
            // 
            // fcgLBMP4MuxerPath
            // 
            this->fcgLBMP4MuxerPath->AutoSize = true;
            this->fcgLBMP4MuxerPath->Location = System::Drawing::Point(6, 94);
            this->fcgLBMP4MuxerPath->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBMP4MuxerPath->Name = L"fcgLBMP4MuxerPath";
            this->fcgLBMP4MuxerPath->Size = System::Drawing::Size(75, 22);
            this->fcgLBMP4MuxerPath->TabIndex = 3;
            this->fcgLBMP4MuxerPath->Text = L"～の指定";
            // 
            // fcgCXMP4CmdEx
            // 
            this->fcgCXMP4CmdEx->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXMP4CmdEx->FormattingEnabled = true;
            this->fcgCXMP4CmdEx->Location = System::Drawing::Point(319, 11);
            this->fcgCXMP4CmdEx->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXMP4CmdEx->Name = L"fcgCXMP4CmdEx";
            this->fcgCXMP4CmdEx->Size = System::Drawing::Size(233, 30);
            this->fcgCXMP4CmdEx->TabIndex = 1;
            this->fcgCXMP4CmdEx->Tag = L"chValue";
            // 
            // fcgLBMP4CmdEx
            // 
            this->fcgLBMP4CmdEx->AutoSize = true;
            this->fcgLBMP4CmdEx->Location = System::Drawing::Point(209, 14);
            this->fcgLBMP4CmdEx->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBMP4CmdEx->Name = L"fcgLBMP4CmdEx";
            this->fcgLBMP4CmdEx->Size = System::Drawing::Size(106, 22);
            this->fcgLBMP4CmdEx->TabIndex = 1;
            this->fcgLBMP4CmdEx->Text = L"拡張オプション";
            // 
            // fcgCBMP4MuxerExt
            // 
            this->fcgCBMP4MuxerExt->AutoSize = true;
            this->fcgCBMP4MuxerExt->Location = System::Drawing::Point(14, 13);
            this->fcgCBMP4MuxerExt->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBMP4MuxerExt->Name = L"fcgCBMP4MuxerExt";
            this->fcgCBMP4MuxerExt->Size = System::Drawing::Size(170, 26);
            this->fcgCBMP4MuxerExt->TabIndex = 0;
            this->fcgCBMP4MuxerExt->Tag = L"chValue";
            this->fcgCBMP4MuxerExt->Text = L"外部muxerを使用";
            this->fcgCBMP4MuxerExt->UseVisualStyleBackColor = true;
            // 
            // fcgtabPageMKV
            // 
            this->fcgtabPageMKV->Controls->Add(this->fcgBTMKVMuxerPath);
            this->fcgtabPageMKV->Controls->Add(this->fcgTXMKVMuxerPath);
            this->fcgtabPageMKV->Controls->Add(this->fcgLBMKVMuxerPath);
            this->fcgtabPageMKV->Controls->Add(this->fcgCXMKVCmdEx);
            this->fcgtabPageMKV->Controls->Add(this->fcgLBMKVMuxerCmdEx);
            this->fcgtabPageMKV->Controls->Add(this->fcgCBMKVMuxerExt);
            this->fcgtabPageMKV->Location = System::Drawing::Point(4, 31);
            this->fcgtabPageMKV->Margin = System::Windows::Forms::Padding(5);
            this->fcgtabPageMKV->Name = L"fcgtabPageMKV";
            this->fcgtabPageMKV->Padding = System::Windows::Forms::Padding(5);
            this->fcgtabPageMKV->Size = System::Drawing::Size(568, 266);
            this->fcgtabPageMKV->TabIndex = 1;
            this->fcgtabPageMKV->Text = L"mkv";
            this->fcgtabPageMKV->UseVisualStyleBackColor = true;
            // 
            // fcgBTMKVMuxerPath
            // 
            this->fcgBTMKVMuxerPath->Location = System::Drawing::Point(510, 114);
            this->fcgBTMKVMuxerPath->Margin = System::Windows::Forms::Padding(5);
            this->fcgBTMKVMuxerPath->Name = L"fcgBTMKVMuxerPath";
            this->fcgBTMKVMuxerPath->Size = System::Drawing::Size(46, 35);
            this->fcgBTMKVMuxerPath->TabIndex = 3;
            this->fcgBTMKVMuxerPath->Text = L"...";
            this->fcgBTMKVMuxerPath->UseVisualStyleBackColor = true;
            this->fcgBTMKVMuxerPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMKVMuxerPath_Click);
            // 
            // fcgTXMKVMuxerPath
            // 
            this->fcgTXMKVMuxerPath->Location = System::Drawing::Point(197, 115);
            this->fcgTXMKVMuxerPath->Margin = System::Windows::Forms::Padding(5);
            this->fcgTXMKVMuxerPath->Name = L"fcgTXMKVMuxerPath";
            this->fcgTXMKVMuxerPath->Size = System::Drawing::Size(309, 28);
            this->fcgTXMKVMuxerPath->TabIndex = 2;
            this->fcgTXMKVMuxerPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMKVMuxerPath_TextChanged);
            this->fcgTXMKVMuxerPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXMKVMuxerPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            this->fcgTXMKVMuxerPath->Enter += gcnew System::EventHandler(this, &frmConfig::fcgTXMKVMuxerPath_Enter);
            this->fcgTXMKVMuxerPath->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTXMKVMuxerPath_Leave);
            // 
            // fcgLBMKVMuxerPath
            // 
            this->fcgLBMKVMuxerPath->AutoSize = true;
            this->fcgLBMKVMuxerPath->Location = System::Drawing::Point(6, 120);
            this->fcgLBMKVMuxerPath->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBMKVMuxerPath->Name = L"fcgLBMKVMuxerPath";
            this->fcgLBMKVMuxerPath->Size = System::Drawing::Size(75, 22);
            this->fcgLBMKVMuxerPath->TabIndex = 19;
            this->fcgLBMKVMuxerPath->Text = L"～の指定";
            // 
            // fcgCXMKVCmdEx
            // 
            this->fcgCXMKVCmdEx->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXMKVCmdEx->FormattingEnabled = true;
            this->fcgCXMKVCmdEx->Location = System::Drawing::Point(319, 65);
            this->fcgCXMKVCmdEx->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXMKVCmdEx->Name = L"fcgCXMKVCmdEx";
            this->fcgCXMKVCmdEx->Size = System::Drawing::Size(233, 30);
            this->fcgCXMKVCmdEx->TabIndex = 1;
            this->fcgCXMKVCmdEx->Tag = L"chValue";
            // 
            // fcgLBMKVMuxerCmdEx
            // 
            this->fcgLBMKVMuxerCmdEx->AutoSize = true;
            this->fcgLBMKVMuxerCmdEx->Location = System::Drawing::Point(209, 70);
            this->fcgLBMKVMuxerCmdEx->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBMKVMuxerCmdEx->Name = L"fcgLBMKVMuxerCmdEx";
            this->fcgLBMKVMuxerCmdEx->Size = System::Drawing::Size(106, 22);
            this->fcgLBMKVMuxerCmdEx->TabIndex = 17;
            this->fcgLBMKVMuxerCmdEx->Text = L"拡張オプション";
            // 
            // fcgCBMKVMuxerExt
            // 
            this->fcgCBMKVMuxerExt->AutoSize = true;
            this->fcgCBMKVMuxerExt->Location = System::Drawing::Point(14, 67);
            this->fcgCBMKVMuxerExt->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBMKVMuxerExt->Name = L"fcgCBMKVMuxerExt";
            this->fcgCBMKVMuxerExt->Size = System::Drawing::Size(170, 26);
            this->fcgCBMKVMuxerExt->TabIndex = 0;
            this->fcgCBMKVMuxerExt->Tag = L"chValue";
            this->fcgCBMKVMuxerExt->Text = L"外部muxerを使用";
            this->fcgCBMKVMuxerExt->UseVisualStyleBackColor = true;
            // 
            // fcgtabPageMux
            // 
            this->fcgtabPageMux->Controls->Add(this->fcgCXMuxPriority);
            this->fcgtabPageMux->Controls->Add(this->fcgLBMuxPriority);
            this->fcgtabPageMux->Controls->Add(this->fcgCBMuxMinimize);
            this->fcgtabPageMux->Location = System::Drawing::Point(4, 31);
            this->fcgtabPageMux->Margin = System::Windows::Forms::Padding(5);
            this->fcgtabPageMux->Name = L"fcgtabPageMux";
            this->fcgtabPageMux->Size = System::Drawing::Size(568, 266);
            this->fcgtabPageMux->TabIndex = 2;
            this->fcgtabPageMux->Text = L"Mux共通設定";
            this->fcgtabPageMux->UseVisualStyleBackColor = true;
            // 
            // fcgCXMuxPriority
            // 
            this->fcgCXMuxPriority->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXMuxPriority->FormattingEnabled = true;
            this->fcgCXMuxPriority->Location = System::Drawing::Point(154, 96);
            this->fcgCXMuxPriority->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXMuxPriority->Name = L"fcgCXMuxPriority";
            this->fcgCXMuxPriority->Size = System::Drawing::Size(294, 30);
            this->fcgCXMuxPriority->TabIndex = 1;
            this->fcgCXMuxPriority->Tag = L"chValue";
            // 
            // fcgLBMuxPriority
            // 
            this->fcgLBMuxPriority->AutoSize = true;
            this->fcgLBMuxPriority->Location = System::Drawing::Point(23, 101);
            this->fcgLBMuxPriority->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBMuxPriority->Name = L"fcgLBMuxPriority";
            this->fcgLBMuxPriority->Size = System::Drawing::Size(95, 22);
            this->fcgLBMuxPriority->TabIndex = 1;
            this->fcgLBMuxPriority->Text = L"Mux優先度";
            // 
            // fcgCBMuxMinimize
            // 
            this->fcgCBMuxMinimize->AutoSize = true;
            this->fcgCBMuxMinimize->Location = System::Drawing::Point(26, 38);
            this->fcgCBMuxMinimize->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBMuxMinimize->Name = L"fcgCBMuxMinimize";
            this->fcgCBMuxMinimize->Size = System::Drawing::Size(87, 26);
            this->fcgCBMuxMinimize->TabIndex = 0;
            this->fcgCBMuxMinimize->Tag = L"chValue";
            this->fcgCBMuxMinimize->Text = L"最小化";
            this->fcgCBMuxMinimize->UseVisualStyleBackColor = true;
            // 
            // fcgtabPageBat
            // 
            this->fcgtabPageBat->Controls->Add(this->fcgLBBatAfterString);
            this->fcgtabPageBat->Controls->Add(this->fcgLBBatBeforeString);
            this->fcgtabPageBat->Controls->Add(this->fcgPNSeparator);
            this->fcgtabPageBat->Controls->Add(this->fcgBTBatBeforePath);
            this->fcgtabPageBat->Controls->Add(this->fcgTXBatBeforePath);
            this->fcgtabPageBat->Controls->Add(this->fcgLBBatBeforePath);
            this->fcgtabPageBat->Controls->Add(this->fcgCBWaitForBatBefore);
            this->fcgtabPageBat->Controls->Add(this->fcgCBRunBatBefore);
            this->fcgtabPageBat->Controls->Add(this->fcgBTBatAfterPath);
            this->fcgtabPageBat->Controls->Add(this->fcgTXBatAfterPath);
            this->fcgtabPageBat->Controls->Add(this->fcgLBBatAfterPath);
            this->fcgtabPageBat->Controls->Add(this->fcgCBWaitForBatAfter);
            this->fcgtabPageBat->Controls->Add(this->fcgCBRunBatAfter);
            this->fcgtabPageBat->Location = System::Drawing::Point(4, 31);
            this->fcgtabPageBat->Margin = System::Windows::Forms::Padding(5);
            this->fcgtabPageBat->Name = L"fcgtabPageBat";
            this->fcgtabPageBat->Size = System::Drawing::Size(568, 266);
            this->fcgtabPageBat->TabIndex = 3;
            this->fcgtabPageBat->Text = L"エンコ前後バッチ処理";
            this->fcgtabPageBat->UseVisualStyleBackColor = true;
            // 
            // fcgLBBatAfterString
            // 
            this->fcgLBBatAfterString->AutoSize = true;
            this->fcgLBBatAfterString->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Italic | System::Drawing::FontStyle::Underline)),
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(128)));
            this->fcgLBBatAfterString->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
            this->fcgLBBatAfterString->Location = System::Drawing::Point(456, 168);
            this->fcgLBBatAfterString->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBBatAfterString->Name = L"fcgLBBatAfterString";
            this->fcgLBBatAfterString->Size = System::Drawing::Size(40, 23);
            this->fcgLBBatAfterString->TabIndex = 20;
            this->fcgLBBatAfterString->Text = L" 後& ";
            this->fcgLBBatAfterString->TextAlign = System::Drawing::ContentAlignment::TopCenter;
            // 
            // fcgLBBatBeforeString
            // 
            this->fcgLBBatBeforeString->AutoSize = true;
            this->fcgLBBatBeforeString->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Italic | System::Drawing::FontStyle::Underline)),
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(128)));
            this->fcgLBBatBeforeString->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
            this->fcgLBBatBeforeString->Location = System::Drawing::Point(456, 22);
            this->fcgLBBatBeforeString->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBBatBeforeString->Name = L"fcgLBBatBeforeString";
            this->fcgLBBatBeforeString->Size = System::Drawing::Size(40, 23);
            this->fcgLBBatBeforeString->TabIndex = 19;
            this->fcgLBBatBeforeString->Text = L" 前& ";
            this->fcgLBBatBeforeString->TextAlign = System::Drawing::ContentAlignment::TopCenter;
            // 
            // fcgPNSeparator
            // 
            this->fcgPNSeparator->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->fcgPNSeparator->Location = System::Drawing::Point(26, 132);
            this->fcgPNSeparator->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNSeparator->Name = L"fcgPNSeparator";
            this->fcgPNSeparator->Size = System::Drawing::Size(512, 1);
            this->fcgPNSeparator->TabIndex = 18;
            // 
            // fcgBTBatBeforePath
            // 
            this->fcgBTBatBeforePath->Location = System::Drawing::Point(494, 83);
            this->fcgBTBatBeforePath->Margin = System::Windows::Forms::Padding(5);
            this->fcgBTBatBeforePath->Name = L"fcgBTBatBeforePath";
            this->fcgBTBatBeforePath->Size = System::Drawing::Size(46, 35);
            this->fcgBTBatBeforePath->TabIndex = 17;
            this->fcgBTBatBeforePath->Tag = L"chValue";
            this->fcgBTBatBeforePath->Text = L"...";
            this->fcgBTBatBeforePath->UseVisualStyleBackColor = true;
            this->fcgBTBatBeforePath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTBatBeforePath_Click);
            // 
            // fcgTXBatBeforePath
            // 
            this->fcgTXBatBeforePath->AllowDrop = true;
            this->fcgTXBatBeforePath->Location = System::Drawing::Point(190, 84);
            this->fcgTXBatBeforePath->Margin = System::Windows::Forms::Padding(5);
            this->fcgTXBatBeforePath->Name = L"fcgTXBatBeforePath";
            this->fcgTXBatBeforePath->Size = System::Drawing::Size(302, 28);
            this->fcgTXBatBeforePath->TabIndex = 16;
            this->fcgTXBatBeforePath->Tag = L"chValue";
            this->fcgTXBatBeforePath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXBatBeforePath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            // 
            // fcgLBBatBeforePath
            // 
            this->fcgLBBatBeforePath->AutoSize = true;
            this->fcgLBBatBeforePath->Location = System::Drawing::Point(60, 89);
            this->fcgLBBatBeforePath->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBBatBeforePath->Name = L"fcgLBBatBeforePath";
            this->fcgLBBatBeforePath->Size = System::Drawing::Size(94, 22);
            this->fcgLBBatBeforePath->TabIndex = 15;
            this->fcgLBBatBeforePath->Text = L"バッチファイル";
            // 
            // fcgCBWaitForBatBefore
            // 
            this->fcgCBWaitForBatBefore->AutoSize = true;
            this->fcgCBWaitForBatBefore->Location = System::Drawing::Point(60, 46);
            this->fcgCBWaitForBatBefore->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBWaitForBatBefore->Name = L"fcgCBWaitForBatBefore";
            this->fcgCBWaitForBatBefore->Size = System::Drawing::Size(229, 26);
            this->fcgCBWaitForBatBefore->TabIndex = 14;
            this->fcgCBWaitForBatBefore->Tag = L"chValue";
            this->fcgCBWaitForBatBefore->Text = L"バッチ処理の終了を待機する";
            this->fcgCBWaitForBatBefore->UseVisualStyleBackColor = true;
            // 
            // fcgCBRunBatBefore
            // 
            this->fcgCBRunBatBefore->AutoSize = true;
            this->fcgCBRunBatBefore->Location = System::Drawing::Point(26, 10);
            this->fcgCBRunBatBefore->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBRunBatBefore->Name = L"fcgCBRunBatBefore";
            this->fcgCBRunBatBefore->Size = System::Drawing::Size(274, 26);
            this->fcgCBRunBatBefore->TabIndex = 13;
            this->fcgCBRunBatBefore->Tag = L"chValue";
            this->fcgCBRunBatBefore->Text = L"エンコード開始前、バッチ処理を行う";
            this->fcgCBRunBatBefore->UseVisualStyleBackColor = true;
            // 
            // fcgBTBatAfterPath
            // 
            this->fcgBTBatAfterPath->Location = System::Drawing::Point(494, 218);
            this->fcgBTBatAfterPath->Margin = System::Windows::Forms::Padding(5);
            this->fcgBTBatAfterPath->Name = L"fcgBTBatAfterPath";
            this->fcgBTBatAfterPath->Size = System::Drawing::Size(46, 35);
            this->fcgBTBatAfterPath->TabIndex = 10;
            this->fcgBTBatAfterPath->Tag = L"chValue";
            this->fcgBTBatAfterPath->Text = L"...";
            this->fcgBTBatAfterPath->UseVisualStyleBackColor = true;
            this->fcgBTBatAfterPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTBatAfterPath_Click);
            // 
            // fcgTXBatAfterPath
            // 
            this->fcgTXBatAfterPath->AllowDrop = true;
            this->fcgTXBatAfterPath->Location = System::Drawing::Point(190, 221);
            this->fcgTXBatAfterPath->Margin = System::Windows::Forms::Padding(5);
            this->fcgTXBatAfterPath->Name = L"fcgTXBatAfterPath";
            this->fcgTXBatAfterPath->Size = System::Drawing::Size(302, 28);
            this->fcgTXBatAfterPath->TabIndex = 9;
            this->fcgTXBatAfterPath->Tag = L"chValue";
            this->fcgTXBatAfterPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXBatAfterPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            // 
            // fcgLBBatAfterPath
            // 
            this->fcgLBBatAfterPath->AutoSize = true;
            this->fcgLBBatAfterPath->Location = System::Drawing::Point(60, 226);
            this->fcgLBBatAfterPath->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBBatAfterPath->Name = L"fcgLBBatAfterPath";
            this->fcgLBBatAfterPath->Size = System::Drawing::Size(94, 22);
            this->fcgLBBatAfterPath->TabIndex = 8;
            this->fcgLBBatAfterPath->Text = L"バッチファイル";
            // 
            // fcgCBWaitForBatAfter
            // 
            this->fcgCBWaitForBatAfter->AutoSize = true;
            this->fcgCBWaitForBatAfter->Location = System::Drawing::Point(60, 182);
            this->fcgCBWaitForBatAfter->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBWaitForBatAfter->Name = L"fcgCBWaitForBatAfter";
            this->fcgCBWaitForBatAfter->Size = System::Drawing::Size(229, 26);
            this->fcgCBWaitForBatAfter->TabIndex = 7;
            this->fcgCBWaitForBatAfter->Tag = L"chValue";
            this->fcgCBWaitForBatAfter->Text = L"バッチ処理の終了を待機する";
            this->fcgCBWaitForBatAfter->UseVisualStyleBackColor = true;
            // 
            // fcgCBRunBatAfter
            // 
            this->fcgCBRunBatAfter->AutoSize = true;
            this->fcgCBRunBatAfter->Location = System::Drawing::Point(26, 146);
            this->fcgCBRunBatAfter->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBRunBatAfter->Name = L"fcgCBRunBatAfter";
            this->fcgCBRunBatAfter->Size = System::Drawing::Size(274, 26);
            this->fcgCBRunBatAfter->TabIndex = 6;
            this->fcgCBRunBatAfter->Tag = L"chValue";
            this->fcgCBRunBatAfter->Text = L"エンコード終了後、バッチ処理を行う";
            this->fcgCBRunBatAfter->UseVisualStyleBackColor = true;
            // 
            // fcgtabPageInternal
            // 
            this->fcgtabPageInternal->Controls->Add(this->fcgCXInternalCmdEx);
            this->fcgtabPageInternal->Controls->Add(this->fcgLBInternalCmdEx);
            this->fcgtabPageInternal->Location = System::Drawing::Point(4, 31);
            this->fcgtabPageInternal->Margin = System::Windows::Forms::Padding(5);
            this->fcgtabPageInternal->Name = L"fcgtabPageInternal";
            this->fcgtabPageInternal->Size = System::Drawing::Size(568, 266);
            this->fcgtabPageInternal->TabIndex = 5;
            this->fcgtabPageInternal->Text = L"mux";
            this->fcgtabPageInternal->UseVisualStyleBackColor = true;
            // 
            // fcgCXInternalCmdEx
            // 
            this->fcgCXInternalCmdEx->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXInternalCmdEx->FormattingEnabled = true;
            this->fcgCXInternalCmdEx->Location = System::Drawing::Point(125, 24);
            this->fcgCXInternalCmdEx->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXInternalCmdEx->Name = L"fcgCXInternalCmdEx";
            this->fcgCXInternalCmdEx->Size = System::Drawing::Size(233, 30);
            this->fcgCXInternalCmdEx->TabIndex = 4;
            this->fcgCXInternalCmdEx->Tag = L"chValue";
            // 
            // fcgLBInternalCmdEx
            // 
            this->fcgLBInternalCmdEx->AutoSize = true;
            this->fcgLBInternalCmdEx->Location = System::Drawing::Point(13, 29);
            this->fcgLBInternalCmdEx->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBInternalCmdEx->Name = L"fcgLBInternalCmdEx";
            this->fcgLBInternalCmdEx->Size = System::Drawing::Size(106, 22);
            this->fcgLBInternalCmdEx->TabIndex = 5;
            this->fcgLBInternalCmdEx->Text = L"拡張オプション";
            // 
            // fcgTXCmd
            // 
            this->fcgTXCmd->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->fcgTXCmd->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgTXCmd->Location = System::Drawing::Point(13, 817);
            this->fcgTXCmd->Margin = System::Windows::Forms::Padding(5);
            this->fcgTXCmd->Name = L"fcgTXCmd";
            this->fcgTXCmd->ReadOnly = true;
            this->fcgTXCmd->Size = System::Drawing::Size(1486, 28);
            this->fcgTXCmd->TabIndex = 4;
            this->fcgTXCmd->DoubleClick += gcnew System::EventHandler(this, &frmConfig::fcgTXCmd_DoubleClick);
            // 
            // fcgBTCancel
            // 
            this->fcgBTCancel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
            this->fcgBTCancel->Location = System::Drawing::Point(1157, 854);
            this->fcgBTCancel->Margin = System::Windows::Forms::Padding(5);
            this->fcgBTCancel->Name = L"fcgBTCancel";
            this->fcgBTCancel->Size = System::Drawing::Size(126, 42);
            this->fcgBTCancel->TabIndex = 5;
            this->fcgBTCancel->Text = L"キャンセル";
            this->fcgBTCancel->UseVisualStyleBackColor = true;
            this->fcgBTCancel->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTCancel_Click);
            // 
            // fcgBTOK
            // 
            this->fcgBTOK->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
            this->fcgBTOK->Location = System::Drawing::Point(1339, 854);
            this->fcgBTOK->Margin = System::Windows::Forms::Padding(5);
            this->fcgBTOK->Name = L"fcgBTOK";
            this->fcgBTOK->Size = System::Drawing::Size(126, 42);
            this->fcgBTOK->TabIndex = 6;
            this->fcgBTOK->Text = L"OK";
            this->fcgBTOK->UseVisualStyleBackColor = true;
            this->fcgBTOK->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTOK_Click);
            // 
            // fcgBTDefault
            // 
            this->fcgBTDefault->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
            this->fcgBTDefault->Location = System::Drawing::Point(13, 858);
            this->fcgBTDefault->Margin = System::Windows::Forms::Padding(5);
            this->fcgBTDefault->Name = L"fcgBTDefault";
            this->fcgBTDefault->Size = System::Drawing::Size(168, 42);
            this->fcgBTDefault->TabIndex = 7;
            this->fcgBTDefault->Text = L"デフォルト";
            this->fcgBTDefault->UseVisualStyleBackColor = true;
            this->fcgBTDefault->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTDefault_Click);
            // 
            // fcgLBVersionDate
            // 
            this->fcgLBVersionDate->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
            this->fcgLBVersionDate->AutoSize = true;
            this->fcgLBVersionDate->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Italic, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgLBVersionDate->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
            this->fcgLBVersionDate->Location = System::Drawing::Point(667, 869);
            this->fcgLBVersionDate->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVersionDate->Name = L"fcgLBVersionDate";
            this->fcgLBVersionDate->Size = System::Drawing::Size(71, 22);
            this->fcgLBVersionDate->TabIndex = 8;
            this->fcgLBVersionDate->Text = L"Version";
            // 
            // fcgLBVersion
            // 
            this->fcgLBVersion->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
            this->fcgLBVersion->AutoSize = true;
            this->fcgLBVersion->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Italic, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgLBVersion->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
            this->fcgLBVersion->Location = System::Drawing::Point(204, 869);
            this->fcgLBVersion->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVersion->Name = L"fcgLBVersion";
            this->fcgLBVersion->Size = System::Drawing::Size(71, 22);
            this->fcgLBVersion->TabIndex = 9;
            this->fcgLBVersion->Text = L"Version";
            // 
            // fcgOpenFileDialog
            // 
            this->fcgOpenFileDialog->FileName = L"openFileDialog1";
            // 
            // fcgTTEx
            // 
            this->fcgTTEx->AutomaticDelay = 200;
            this->fcgTTEx->AutoPopDelay = 9999;
            this->fcgTTEx->InitialDelay = 200;
            this->fcgTTEx->IsBalloon = true;
            this->fcgTTEx->ReshowDelay = 50;
            this->fcgTTEx->ShowAlways = true;
            this->fcgTTEx->UseAnimation = false;
            this->fcgTTEx->UseFading = false;
            // 
            // fcgtabControlVCE
            // 
            this->fcgtabControlVCE->Controls->Add(this->tabPageVideoEnc);
            this->fcgtabControlVCE->Controls->Add(this->tabPageVideoEnc2);
            this->fcgtabControlVCE->Controls->Add(this->tabPageVpp);
            this->fcgtabControlVCE->Controls->Add(this->tabPageExOpt);
            this->fcgtabControlVCE->Location = System::Drawing::Point(6, 47);
            this->fcgtabControlVCE->Margin = System::Windows::Forms::Padding(5);
            this->fcgtabControlVCE->Name = L"fcgtabControlVCE";
            this->fcgtabControlVCE->SelectedIndex = 0;
            this->fcgtabControlVCE->Size = System::Drawing::Size(924, 763);
            this->fcgtabControlVCE->TabIndex = 49;
            // 
            // tabPageVideoEnc
            // 
            this->tabPageVideoEnc->Controls->Add(this->label1);
            this->tabPageVideoEnc->Controls->Add(this->fcgPNQVBR);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBDevice);
            this->tabPageVideoEnc->Controls->Add(this->fcgCXDevice);
            this->tabPageVideoEnc->Controls->Add(this->fcgCXBitdepth);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBBitdepth);
            this->tabPageVideoEnc->Controls->Add(this->fcgCXVideoFormat);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBVideoFormat);
            this->tabPageVideoEnc->Controls->Add(this->fcggroupBoxColorMatrix);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBQPMin);
            this->tabPageVideoEnc->Controls->Add(this->fcgBTVideoEncoderPath);
            this->tabPageVideoEnc->Controls->Add(this->fcgTXVideoEncoderPath);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBVideoEncoderPath);
            this->tabPageVideoEnc->Controls->Add(this->fcgNURefFrames);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBRefFrames);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBEncCodec);
            this->tabPageVideoEnc->Controls->Add(this->fcgCXEncCodec);
            this->tabPageVideoEnc->Controls->Add(this->fcgNUVBVBufSize);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBSkipFrame);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBVBVBufSize);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBVBVBufSizeKbps);
            this->tabPageVideoEnc->Controls->Add(this->fcgCBSkipFrame);
            this->tabPageVideoEnc->Controls->Add(this->fcgCBAFS);
            this->tabPageVideoEnc->Controls->Add(this->fcgNUQPMax);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBQPMax);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBQPMinMAX);
            this->tabPageVideoEnc->Controls->Add(this->fcgNUQPMin);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBQualityPreset);
            this->tabPageVideoEnc->Controls->Add(this->fcgCXQualityPreset);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBDeblock);
            this->tabPageVideoEnc->Controls->Add(this->fcgpictureBoxVCEEnabled);
            this->tabPageVideoEnc->Controls->Add(this->fcgpictureBoxVCEDisabled);
            this->tabPageVideoEnc->Controls->Add(this->fcgCBDeblock);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBGOPLengthAuto);
            this->tabPageVideoEnc->Controls->Add(this->fcgGroupBoxAspectRatio);
            this->tabPageVideoEnc->Controls->Add(this->fcgCXInterlaced);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBInterlaced);
            this->tabPageVideoEnc->Controls->Add(this->fcgNUGopLength);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBGOPLength);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBEncMode);
            this->tabPageVideoEnc->Controls->Add(this->fcgCXEncMode);
            this->tabPageVideoEnc->Controls->Add(this->fcgPNBframes);
            this->tabPageVideoEnc->Controls->Add(this->fcgPNH264Features);
            this->tabPageVideoEnc->Controls->Add(this->fcgPNBitrate);
            this->tabPageVideoEnc->Controls->Add(this->fcgPNQP);
            this->tabPageVideoEnc->Controls->Add(this->fcgPNHEVCLevelProfile);
            this->tabPageVideoEnc->Controls->Add(this->fcgPNAV1LevelProfile);
            this->tabPageVideoEnc->Controls->Add(this->fcgPNH264LevelProfile);
            this->tabPageVideoEnc->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->tabPageVideoEnc->Location = System::Drawing::Point(4, 32);
            this->tabPageVideoEnc->Margin = System::Windows::Forms::Padding(5);
            this->tabPageVideoEnc->Name = L"tabPageVideoEnc";
            this->tabPageVideoEnc->Padding = System::Windows::Forms::Padding(5);
            this->tabPageVideoEnc->Size = System::Drawing::Size(916, 727);
            this->tabPageVideoEnc->TabIndex = 0;
            this->tabPageVideoEnc->Text = L"動画エンコード";
            this->tabPageVideoEnc->UseVisualStyleBackColor = true;
            // 
            // label1
            // 
            this->label1->AutoSize = true;
            this->label1->Location = System::Drawing::Point(324, 485);
            this->label1->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(102, 22);
            this->label1->TabIndex = 147;
            this->label1->Text = L"※\"0\"で自動";
            // 
            // fcgPNQVBR
            // 
            this->fcgPNQVBR->Controls->Add(this->fcgLBQVBRQuality);
            this->fcgPNQVBR->Controls->Add(this->fcgNUQVBRQuality);
            this->fcgPNQVBR->Location = System::Drawing::Point(12, 323);
            this->fcgPNQVBR->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNQVBR->Name = L"fcgPNQVBR";
            this->fcgPNQVBR->Size = System::Drawing::Size(433, 40);
            this->fcgPNQVBR->TabIndex = 17;
            // 
            // fcgLBQVBRQuality
            // 
            this->fcgLBQVBRQuality->AutoSize = true;
            this->fcgLBQVBRQuality->Location = System::Drawing::Point(7, 6);
            this->fcgLBQVBRQuality->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBQVBRQuality->Name = L"fcgLBQVBRQuality";
            this->fcgLBQVBRQuality->Size = System::Drawing::Size(44, 22);
            this->fcgLBQVBRQuality->TabIndex = 11;
            this->fcgLBQVBRQuality->Text = L"品質";
            // 
            // fcgNUQVBRQuality
            // 
            this->fcgNUQVBRQuality->Location = System::Drawing::Point(186, 2);
            this->fcgNUQVBRQuality->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUQVBRQuality->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65536000, 0, 0, 0 });
            this->fcgNUQVBRQuality->Name = L"fcgNUQVBRQuality";
            this->fcgNUQVBRQuality->Size = System::Drawing::Size(115, 28);
            this->fcgNUQVBRQuality->TabIndex = 12;
            this->fcgNUQVBRQuality->Tag = L"reCmd";
            this->fcgNUQVBRQuality->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBDevice
            // 
            this->fcgLBDevice->AutoSize = true;
            this->fcgLBDevice->Location = System::Drawing::Point(19, 166);
            this->fcgLBDevice->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBDevice->Name = L"fcgLBDevice";
            this->fcgLBDevice->Size = System::Drawing::Size(63, 22);
            this->fcgLBDevice->TabIndex = 145;
            this->fcgLBDevice->Text = L"デバイス";
            // 
            // fcgCXDevice
            // 
            this->fcgCXDevice->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXDevice->FormattingEnabled = true;
            this->fcgCXDevice->Items->AddRange(gcnew cli::array< System::Object^  >(3) { L"高品質", L"標準", L"高速" });
            this->fcgCXDevice->Location = System::Drawing::Point(121, 161);
            this->fcgCXDevice->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXDevice->Name = L"fcgCXDevice";
            this->fcgCXDevice->Size = System::Drawing::Size(280, 30);
            this->fcgCXDevice->TabIndex = 146;
            this->fcgCXDevice->Tag = L"reCmd";
            // 
            // fcgPNHEVCLevelProfile
            // 
            this->fcgPNHEVCLevelProfile->Controls->Add(this->fcgCXHEVCLevel);
            this->fcgPNHEVCLevelProfile->Controls->Add(this->fcgCXHEVCProfile);
            this->fcgPNHEVCLevelProfile->Controls->Add(this->fcgLBHEVCLevel);
            this->fcgPNHEVCLevelProfile->Controls->Add(this->fcgLBHEVCProfile);
            this->fcgPNHEVCLevelProfile->Location = System::Drawing::Point(520, 200);
            this->fcgPNHEVCLevelProfile->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNHEVCLevelProfile->Name = L"fcgPNHEVCLevelProfile";
            this->fcgPNHEVCLevelProfile->Size = System::Drawing::Size(383, 95);
            this->fcgPNHEVCLevelProfile->TabIndex = 110;
            // 
            // fcgPNAV1LevelProfile
            // 
            this->fcgPNAV1LevelProfile->Controls->Add(this->fcgCXAV1Level);
            this->fcgPNAV1LevelProfile->Controls->Add(this->fcgCXAV1Profile);
            this->fcgPNAV1LevelProfile->Controls->Add(this->fcgLBAV1Level);
            this->fcgPNAV1LevelProfile->Controls->Add(this->fcgLBAV1Profile);
            this->fcgPNAV1LevelProfile->Location = System::Drawing::Point(520, 200);
            this->fcgPNAV1LevelProfile->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNAV1LevelProfile->Name = L"fcgPNAV1LevelProfile";
            this->fcgPNAV1LevelProfile->Size = System::Drawing::Size(383, 95);
            this->fcgPNAV1LevelProfile->TabIndex = 111;
            // 
            // fcgCXAV1Level
            // 
            this->fcgCXAV1Level->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAV1Level->FormattingEnabled = true;
            this->fcgCXAV1Level->Location = System::Drawing::Point(167, 56);
            this->fcgCXAV1Level->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXAV1Level->Name = L"fcgCXAV1Level";
            this->fcgCXAV1Level->Size = System::Drawing::Size(179, 30);
            this->fcgCXAV1Level->TabIndex = 104;
            this->fcgCXAV1Level->Tag = L"reCmd";
            // 
            // fcgCXAV1Profile
            // 
            this->fcgCXAV1Profile->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAV1Profile->FormattingEnabled = true;
            this->fcgCXAV1Profile->Location = System::Drawing::Point(167, 8);
            this->fcgCXAV1Profile->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXAV1Profile->Name = L"fcgCXAV1Profile";
            this->fcgCXAV1Profile->Size = System::Drawing::Size(179, 30);
            this->fcgCXAV1Profile->TabIndex = 102;
            this->fcgCXAV1Profile->Tag = L"reCmd";
            // 
            // fcgLBAV1Level
            // 
            this->fcgLBAV1Level->AutoSize = true;
            this->fcgLBAV1Level->Location = System::Drawing::Point(18, 61);
            this->fcgLBAV1Level->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBAV1Level->Name = L"fcgLBAV1Level";
            this->fcgLBAV1Level->Size = System::Drawing::Size(51, 22);
            this->fcgLBAV1Level->TabIndex = 103;
            this->fcgLBAV1Level->Text = L"レベル";
            // 
            // fcgLBAV1Profile
            // 
            this->fcgLBAV1Profile->AutoSize = true;
            this->fcgLBAV1Profile->Location = System::Drawing::Point(18, 13);
            this->fcgLBAV1Profile->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBAV1Profile->Name = L"fcgLBAV1Profile";
            this->fcgLBAV1Profile->Size = System::Drawing::Size(82, 22);
            this->fcgLBAV1Profile->TabIndex = 101;
            this->fcgLBAV1Profile->Text = L"プロファイル";
            // 
            // fcgCXHEVCLevel
            // 
            this->fcgCXHEVCLevel->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXHEVCLevel->FormattingEnabled = true;
            this->fcgCXHEVCLevel->Location = System::Drawing::Point(167, 56);
            this->fcgCXHEVCLevel->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXHEVCLevel->Name = L"fcgCXHEVCLevel";
            this->fcgCXHEVCLevel->Size = System::Drawing::Size(179, 30);
            this->fcgCXHEVCLevel->TabIndex = 104;
            this->fcgCXHEVCLevel->Tag = L"reCmd";
            // 
            // fcgCXHEVCProfile
            // 
            this->fcgCXHEVCProfile->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXHEVCProfile->FormattingEnabled = true;
            this->fcgCXHEVCProfile->Location = System::Drawing::Point(167, 8);
            this->fcgCXHEVCProfile->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXHEVCProfile->Name = L"fcgCXHEVCProfile";
            this->fcgCXHEVCProfile->Size = System::Drawing::Size(179, 30);
            this->fcgCXHEVCProfile->TabIndex = 102;
            this->fcgCXHEVCProfile->Tag = L"reCmd";
            // 
            // fcgLBHEVCLevel
            // 
            this->fcgLBHEVCLevel->AutoSize = true;
            this->fcgLBHEVCLevel->Location = System::Drawing::Point(18, 61);
            this->fcgLBHEVCLevel->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBHEVCLevel->Name = L"fcgLBHEVCLevel";
            this->fcgLBHEVCLevel->Size = System::Drawing::Size(51, 22);
            this->fcgLBHEVCLevel->TabIndex = 103;
            this->fcgLBHEVCLevel->Text = L"レベル";
            // 
            // fcgLBHEVCProfile
            // 
            this->fcgLBHEVCProfile->AutoSize = true;
            this->fcgLBHEVCProfile->Location = System::Drawing::Point(18, 13);
            this->fcgLBHEVCProfile->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBHEVCProfile->Name = L"fcgLBHEVCProfile";
            this->fcgLBHEVCProfile->Size = System::Drawing::Size(82, 22);
            this->fcgLBHEVCProfile->TabIndex = 101;
            this->fcgLBHEVCProfile->Text = L"プロファイル";
            // 
            // fcgCXBitdepth
            // 
            this->fcgCXBitdepth->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXBitdepth->FormattingEnabled = true;
            this->fcgCXBitdepth->Location = System::Drawing::Point(686, 305);
            this->fcgCXBitdepth->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXBitdepth->Name = L"fcgCXBitdepth";
            this->fcgCXBitdepth->Size = System::Drawing::Size(179, 30);
            this->fcgCXBitdepth->TabIndex = 144;
            this->fcgCXBitdepth->Tag = L"reCmd";
            // 
            // fcgLBBitdepth
            // 
            this->fcgLBBitdepth->AutoSize = true;
            this->fcgLBBitdepth->Location = System::Drawing::Point(539, 312);
            this->fcgLBBitdepth->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBBitdepth->Name = L"fcgLBBitdepth";
            this->fcgLBBitdepth->Size = System::Drawing::Size(78, 22);
            this->fcgLBBitdepth->TabIndex = 143;
            this->fcgLBBitdepth->Text = L"ビット深度";
            // 
            // fcgCXVideoFormat
            // 
            this->fcgCXVideoFormat->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVideoFormat->FormattingEnabled = true;
            this->fcgCXVideoFormat->Location = System::Drawing::Point(686, 491);
            this->fcgCXVideoFormat->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVideoFormat->Name = L"fcgCXVideoFormat";
            this->fcgCXVideoFormat->Size = System::Drawing::Size(179, 30);
            this->fcgCXVideoFormat->TabIndex = 141;
            this->fcgCXVideoFormat->Tag = L"reCmd";
            // 
            // fcgLBVideoFormat
            // 
            this->fcgLBVideoFormat->AutoSize = true;
            this->fcgLBVideoFormat->Location = System::Drawing::Point(540, 494);
            this->fcgLBVideoFormat->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVideoFormat->Name = L"fcgLBVideoFormat";
            this->fcgLBVideoFormat->Size = System::Drawing::Size(109, 22);
            this->fcgLBVideoFormat->TabIndex = 140;
            this->fcgLBVideoFormat->Text = L"videoformat";
            // 
            // fcggroupBoxColorMatrix
            // 
            this->fcggroupBoxColorMatrix->Controls->Add(this->fcgCBFullrange);
            this->fcggroupBoxColorMatrix->Controls->Add(this->fcgLBFullrange);
            this->fcggroupBoxColorMatrix->Controls->Add(this->fcgCXTransfer);
            this->fcggroupBoxColorMatrix->Controls->Add(this->fcgCXColorPrim);
            this->fcggroupBoxColorMatrix->Controls->Add(this->fcgCXColorMatrix);
            this->fcggroupBoxColorMatrix->Controls->Add(this->fcgLBTransfer);
            this->fcggroupBoxColorMatrix->Controls->Add(this->fcgLBColorPrim);
            this->fcggroupBoxColorMatrix->Controls->Add(this->fcgLBColorMatrix);
            this->fcggroupBoxColorMatrix->Location = System::Drawing::Point(530, 533);
            this->fcggroupBoxColorMatrix->Margin = System::Windows::Forms::Padding(5);
            this->fcggroupBoxColorMatrix->Name = L"fcggroupBoxColorMatrix";
            this->fcggroupBoxColorMatrix->Padding = System::Windows::Forms::Padding(5);
            this->fcggroupBoxColorMatrix->Size = System::Drawing::Size(361, 181);
            this->fcggroupBoxColorMatrix->TabIndex = 142;
            this->fcggroupBoxColorMatrix->TabStop = false;
            this->fcggroupBoxColorMatrix->Text = L"色設定";
            // 
            // fcgCBFullrange
            // 
            this->fcgCBFullrange->AutoSize = true;
            this->fcgCBFullrange->Location = System::Drawing::Point(158, 151);
            this->fcgCBFullrange->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBFullrange->Name = L"fcgCBFullrange";
            this->fcgCBFullrange->Size = System::Drawing::Size(22, 21);
            this->fcgCBFullrange->TabIndex = 176;
            this->fcgCBFullrange->Tag = L"reCmd";
            this->fcgCBFullrange->UseVisualStyleBackColor = true;
            // 
            // fcgLBFullrange
            // 
            this->fcgLBFullrange->AutoSize = true;
            this->fcgLBFullrange->Location = System::Drawing::Point(26, 150);
            this->fcgLBFullrange->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBFullrange->Name = L"fcgLBFullrange";
            this->fcgLBFullrange->Size = System::Drawing::Size(83, 22);
            this->fcgLBFullrange->TabIndex = 175;
            this->fcgLBFullrange->Text = L"fullrange";
            // 
            // fcgCXTransfer
            // 
            this->fcgCXTransfer->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXTransfer->FormattingEnabled = true;
            this->fcgCXTransfer->Location = System::Drawing::Point(157, 108);
            this->fcgCXTransfer->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXTransfer->Name = L"fcgCXTransfer";
            this->fcgCXTransfer->Size = System::Drawing::Size(179, 30);
            this->fcgCXTransfer->TabIndex = 2;
            this->fcgCXTransfer->Tag = L"reCmd";
            // 
            // fcgCXColorPrim
            // 
            this->fcgCXColorPrim->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXColorPrim->FormattingEnabled = true;
            this->fcgCXColorPrim->Location = System::Drawing::Point(157, 66);
            this->fcgCXColorPrim->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXColorPrim->Name = L"fcgCXColorPrim";
            this->fcgCXColorPrim->Size = System::Drawing::Size(179, 30);
            this->fcgCXColorPrim->TabIndex = 1;
            this->fcgCXColorPrim->Tag = L"reCmd";
            // 
            // fcgCXColorMatrix
            // 
            this->fcgCXColorMatrix->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXColorMatrix->FormattingEnabled = true;
            this->fcgCXColorMatrix->Location = System::Drawing::Point(157, 24);
            this->fcgCXColorMatrix->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXColorMatrix->Name = L"fcgCXColorMatrix";
            this->fcgCXColorMatrix->Size = System::Drawing::Size(179, 30);
            this->fcgCXColorMatrix->TabIndex = 0;
            this->fcgCXColorMatrix->Tag = L"reCmd";
            // 
            // fcgLBTransfer
            // 
            this->fcgLBTransfer->AutoSize = true;
            this->fcgLBTransfer->Location = System::Drawing::Point(26, 113);
            this->fcgLBTransfer->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBTransfer->Name = L"fcgLBTransfer";
            this->fcgLBTransfer->Size = System::Drawing::Size(75, 22);
            this->fcgLBTransfer->TabIndex = 2;
            this->fcgLBTransfer->Text = L"transfer";
            // 
            // fcgLBColorPrim
            // 
            this->fcgLBColorPrim->AutoSize = true;
            this->fcgLBColorPrim->Location = System::Drawing::Point(26, 71);
            this->fcgLBColorPrim->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBColorPrim->Name = L"fcgLBColorPrim";
            this->fcgLBColorPrim->Size = System::Drawing::Size(87, 22);
            this->fcgLBColorPrim->TabIndex = 1;
            this->fcgLBColorPrim->Text = L"colorprim";
            // 
            // fcgLBColorMatrix
            // 
            this->fcgLBColorMatrix->AutoSize = true;
            this->fcgLBColorMatrix->Location = System::Drawing::Point(26, 29);
            this->fcgLBColorMatrix->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBColorMatrix->Name = L"fcgLBColorMatrix";
            this->fcgLBColorMatrix->Size = System::Drawing::Size(102, 22);
            this->fcgLBColorMatrix->TabIndex = 0;
            this->fcgLBColorMatrix->Text = L"colormatrix";
            // 
            // fcgLBQPMin
            // 
            this->fcgLBQPMin->AutoSize = true;
            this->fcgLBQPMin->Location = System::Drawing::Point(145, 563);
            this->fcgLBQPMin->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBQPMin->Name = L"fcgLBQPMin";
            this->fcgLBQPMin->Size = System::Drawing::Size(44, 22);
            this->fcgLBQPMin->TabIndex = 39;
            this->fcgLBQPMin->Text = L"下限";
            // 
            // fcgBTVideoEncoderPath
            // 
            this->fcgBTVideoEncoderPath->Location = System::Drawing::Point(410, 118);
            this->fcgBTVideoEncoderPath->Margin = System::Windows::Forms::Padding(5);
            this->fcgBTVideoEncoderPath->Name = L"fcgBTVideoEncoderPath";
            this->fcgBTVideoEncoderPath->Size = System::Drawing::Size(46, 35);
            this->fcgBTVideoEncoderPath->TabIndex = 2;
            this->fcgBTVideoEncoderPath->Text = L"...";
            this->fcgBTVideoEncoderPath->UseVisualStyleBackColor = true;
            this->fcgBTVideoEncoderPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTVideoEncoderPath_Click);
            // 
            // fcgTXVideoEncoderPath
            // 
            this->fcgTXVideoEncoderPath->AllowDrop = true;
            this->fcgTXVideoEncoderPath->Location = System::Drawing::Point(31, 120);
            this->fcgTXVideoEncoderPath->Margin = System::Windows::Forms::Padding(5);
            this->fcgTXVideoEncoderPath->Name = L"fcgTXVideoEncoderPath";
            this->fcgTXVideoEncoderPath->Size = System::Drawing::Size(370, 28);
            this->fcgTXVideoEncoderPath->TabIndex = 1;
            this->fcgTXVideoEncoderPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXVideoEncoderPath_TextChanged);
            this->fcgTXVideoEncoderPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXVideoEncoderPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            this->fcgTXVideoEncoderPath->Enter += gcnew System::EventHandler(this, &frmConfig::fcgTXVideoEncoderPath_Enter);
            this->fcgTXVideoEncoderPath->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTXVideoEncoderPath_Leave);
            // 
            // fcgLBVideoEncoderPath
            // 
            this->fcgLBVideoEncoderPath->AutoSize = true;
            this->fcgLBVideoEncoderPath->Location = System::Drawing::Point(13, 94);
            this->fcgLBVideoEncoderPath->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVideoEncoderPath->Name = L"fcgLBVideoEncoderPath";
            this->fcgLBVideoEncoderPath->Size = System::Drawing::Size(75, 22);
            this->fcgLBVideoEncoderPath->TabIndex = 0;
            this->fcgLBVideoEncoderPath->Text = L"～の指定";
            // 
            // fcgNURefFrames
            // 
            this->fcgNURefFrames->Location = System::Drawing::Point(199, 481);
            this->fcgNURefFrames->Margin = System::Windows::Forms::Padding(5);
            this->fcgNURefFrames->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16, 0, 0, 0 });
            this->fcgNURefFrames->Name = L"fcgNURefFrames";
            this->fcgNURefFrames->Size = System::Drawing::Size(114, 28);
            this->fcgNURefFrames->TabIndex = 34;
            this->fcgNURefFrames->Tag = L"reCmd";
            this->fcgNURefFrames->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNURefFrames->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            // 
            // fcgLBRefFrames
            // 
            this->fcgLBRefFrames->AutoSize = true;
            this->fcgLBRefFrames->Location = System::Drawing::Point(22, 485);
            this->fcgLBRefFrames->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBRefFrames->Name = L"fcgLBRefFrames";
            this->fcgLBRefFrames->Size = System::Drawing::Size(78, 22);
            this->fcgLBRefFrames->TabIndex = 33;
            this->fcgLBRefFrames->Text = L"参照距離";
            // 
            // fcgLBEncCodec
            // 
            this->fcgLBEncCodec->AutoSize = true;
            this->fcgLBEncCodec->Location = System::Drawing::Point(19, 205);
            this->fcgLBEncCodec->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBEncCodec->Name = L"fcgLBEncCodec";
            this->fcgLBEncCodec->Size = System::Drawing::Size(72, 22);
            this->fcgLBEncCodec->TabIndex = 3;
            this->fcgLBEncCodec->Text = L"コーデック";
            // 
            // fcgCXEncCodec
            // 
            this->fcgCXEncCodec->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXEncCodec->FormattingEnabled = true;
            this->fcgCXEncCodec->Items->AddRange(gcnew cli::array< System::Object^  >(3) { L"高品質", L"標準", L"高速" });
            this->fcgCXEncCodec->Location = System::Drawing::Point(121, 202);
            this->fcgCXEncCodec->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXEncCodec->Name = L"fcgCXEncCodec";
            this->fcgCXEncCodec->Size = System::Drawing::Size(280, 30);
            this->fcgCXEncCodec->TabIndex = 4;
            this->fcgCXEncCodec->Tag = L"reCmd";
            this->fcgCXEncCodec->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXCodec_SelectedIndexChanged);
            // 
            // fcgNUVBVBufSize
            // 
            this->fcgNUVBVBufSize->Location = System::Drawing::Point(198, 444);
            this->fcgNUVBVBufSize->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVBVBufSize->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 100000, 0, 0, 0 });
            this->fcgNUVBVBufSize->Name = L"fcgNUVBVBufSize";
            this->fcgNUVBVBufSize->Size = System::Drawing::Size(115, 28);
            this->fcgNUVBVBufSize->TabIndex = 31;
            this->fcgNUVBVBufSize->Tag = L"reCmd";
            this->fcgNUVBVBufSize->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBSkipFrame
            // 
            this->fcgLBSkipFrame->AutoSize = true;
            this->fcgLBSkipFrame->Location = System::Drawing::Point(539, 418);
            this->fcgLBSkipFrame->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBSkipFrame->Name = L"fcgLBSkipFrame";
            this->fcgLBSkipFrame->Size = System::Drawing::Size(112, 22);
            this->fcgLBSkipFrame->TabIndex = 126;
            this->fcgLBSkipFrame->Text = L"スキップフレーム";
            // 
            // fcgLBVBVBufSize
            // 
            this->fcgLBVBVBufSize->AutoSize = true;
            this->fcgLBVBVBufSize->Location = System::Drawing::Point(22, 450);
            this->fcgLBVBVBufSize->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVBVBufSize->Name = L"fcgLBVBVBufSize";
            this->fcgLBVBVBufSize->Size = System::Drawing::Size(130, 22);
            this->fcgLBVBVBufSize->TabIndex = 30;
            this->fcgLBVBVBufSize->Text = L"VBVバッファサイズ";
            // 
            // fcgLBVBVBufSizeKbps
            // 
            this->fcgLBVBVBufSizeKbps->AutoSize = true;
            this->fcgLBVBVBufSizeKbps->Location = System::Drawing::Point(324, 446);
            this->fcgLBVBVBufSizeKbps->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVBVBufSizeKbps->Name = L"fcgLBVBVBufSizeKbps";
            this->fcgLBVBVBufSizeKbps->Size = System::Drawing::Size(47, 22);
            this->fcgLBVBVBufSizeKbps->TabIndex = 32;
            this->fcgLBVBVBufSizeKbps->Text = L"kbps";
            // 
            // fcgCBSkipFrame
            // 
            this->fcgCBSkipFrame->AutoSize = true;
            this->fcgCBSkipFrame->Location = System::Drawing::Point(689, 419);
            this->fcgCBSkipFrame->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBSkipFrame->Name = L"fcgCBSkipFrame";
            this->fcgCBSkipFrame->Size = System::Drawing::Size(22, 21);
            this->fcgCBSkipFrame->TabIndex = 127;
            this->fcgCBSkipFrame->Tag = L"reCmd";
            this->fcgCBSkipFrame->UseVisualStyleBackColor = true;
            // 
            // fcgCBAFS
            // 
            this->fcgCBAFS->AutoSize = true;
            this->fcgCBAFS->Location = System::Drawing::Point(542, 7);
            this->fcgCBAFS->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBAFS->Name = L"fcgCBAFS";
            this->fcgCBAFS->Size = System::Drawing::Size(280, 26);
            this->fcgCBAFS->TabIndex = 70;
            this->fcgCBAFS->Tag = L"reCmd";
            this->fcgCBAFS->Text = L"自動フィールドシフト(afs)を使用する";
            this->fcgCBAFS->UseVisualStyleBackColor = true;
            // 
            // fcgNUQPMax
            // 
            this->fcgNUQPMax->Location = System::Drawing::Point(353, 558);
            this->fcgNUQPMax->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUQPMax->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 51, 0, 0, 0 });
            this->fcgNUQPMax->Name = L"fcgNUQPMax";
            this->fcgNUQPMax->Size = System::Drawing::Size(86, 28);
            this->fcgNUQPMax->TabIndex = 42;
            this->fcgNUQPMax->Tag = L"reCmd";
            this->fcgNUQPMax->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBQPMax
            // 
            this->fcgLBQPMax->AutoSize = true;
            this->fcgLBQPMax->Location = System::Drawing::Point(299, 563);
            this->fcgLBQPMax->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBQPMax->Name = L"fcgLBQPMax";
            this->fcgLBQPMax->Size = System::Drawing::Size(44, 22);
            this->fcgLBQPMax->TabIndex = 41;
            this->fcgLBQPMax->Text = L"上限";
            // 
            // fcgLBQPMinMAX
            // 
            this->fcgLBQPMinMAX->AutoSize = true;
            this->fcgLBQPMinMAX->Location = System::Drawing::Point(22, 563);
            this->fcgLBQPMinMAX->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBQPMinMAX->Name = L"fcgLBQPMinMAX";
            this->fcgLBQPMinMAX->Size = System::Drawing::Size(73, 22);
            this->fcgLBQPMinMAX->TabIndex = 38;
            this->fcgLBQPMinMAX->Text = L"QP 最小";
            // 
            // fcgNUQPMin
            // 
            this->fcgNUQPMin->Location = System::Drawing::Point(198, 558);
            this->fcgNUQPMin->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUQPMin->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 51, 0, 0, 0 });
            this->fcgNUQPMin->Name = L"fcgNUQPMin";
            this->fcgNUQPMin->Size = System::Drawing::Size(79, 28);
            this->fcgNUQPMin->TabIndex = 40;
            this->fcgNUQPMin->Tag = L"reCmd";
            this->fcgNUQPMin->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBQualityPreset
            // 
            this->fcgLBQualityPreset->AutoSize = true;
            this->fcgLBQualityPreset->Location = System::Drawing::Point(19, 287);
            this->fcgLBQualityPreset->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBQualityPreset->Name = L"fcgLBQualityPreset";
            this->fcgLBQualityPreset->Size = System::Drawing::Size(44, 22);
            this->fcgLBQualityPreset->TabIndex = 7;
            this->fcgLBQualityPreset->Text = L"品質";
            // 
            // fcgCXQualityPreset
            // 
            this->fcgCXQualityPreset->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXQualityPreset->FormattingEnabled = true;
            this->fcgCXQualityPreset->Items->AddRange(gcnew cli::array< System::Object^  >(3) { L"高品質", L"標準", L"高速" });
            this->fcgCXQualityPreset->Location = System::Drawing::Point(121, 282);
            this->fcgCXQualityPreset->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXQualityPreset->Name = L"fcgCXQualityPreset";
            this->fcgCXQualityPreset->Size = System::Drawing::Size(280, 30);
            this->fcgCXQualityPreset->TabIndex = 8;
            this->fcgCXQualityPreset->Tag = L"reCmd";
            // 
            // fcgLBDeblock
            // 
            this->fcgLBDeblock->AutoSize = true;
            this->fcgLBDeblock->Location = System::Drawing::Point(540, 382);
            this->fcgLBDeblock->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBDeblock->Name = L"fcgLBDeblock";
            this->fcgLBDeblock->Size = System::Drawing::Size(118, 22);
            this->fcgLBDeblock->TabIndex = 124;
            this->fcgLBDeblock->Text = L"デブロックフィルタ";
            // 
            // fcgpictureBoxVCEEnabled
            // 
            this->fcgpictureBoxVCEEnabled->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgpictureBoxVCEEnabled.Image")));
            this->fcgpictureBoxVCEEnabled->Location = System::Drawing::Point(24, 7);
            this->fcgpictureBoxVCEEnabled->Margin = System::Windows::Forms::Padding(5);
            this->fcgpictureBoxVCEEnabled->Name = L"fcgpictureBoxVCEEnabled";
            this->fcgpictureBoxVCEEnabled->Size = System::Drawing::Size(229, 79);
            this->fcgpictureBoxVCEEnabled->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
            this->fcgpictureBoxVCEEnabled->TabIndex = 133;
            this->fcgpictureBoxVCEEnabled->TabStop = false;
            // 
            // fcgpictureBoxVCEDisabled
            // 
            this->fcgpictureBoxVCEDisabled->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"fcgpictureBoxVCEDisabled.Image")));
            this->fcgpictureBoxVCEDisabled->Location = System::Drawing::Point(24, 7);
            this->fcgpictureBoxVCEDisabled->Margin = System::Windows::Forms::Padding(5);
            this->fcgpictureBoxVCEDisabled->Name = L"fcgpictureBoxVCEDisabled";
            this->fcgpictureBoxVCEDisabled->Size = System::Drawing::Size(229, 79);
            this->fcgpictureBoxVCEDisabled->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
            this->fcgpictureBoxVCEDisabled->TabIndex = 130;
            this->fcgpictureBoxVCEDisabled->TabStop = false;
            // 
            // fcgCBDeblock
            // 
            this->fcgCBDeblock->AutoSize = true;
            this->fcgCBDeblock->Location = System::Drawing::Point(689, 383);
            this->fcgCBDeblock->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBDeblock->Name = L"fcgCBDeblock";
            this->fcgCBDeblock->Size = System::Drawing::Size(22, 21);
            this->fcgCBDeblock->TabIndex = 125;
            this->fcgCBDeblock->Tag = L"reCmd";
            this->fcgCBDeblock->UseVisualStyleBackColor = true;
            // 
            // fcgLBGOPLengthAuto
            // 
            this->fcgLBGOPLengthAuto->AutoSize = true;
            this->fcgLBGOPLengthAuto->Location = System::Drawing::Point(322, 523);
            this->fcgLBGOPLengthAuto->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBGOPLengthAuto->Name = L"fcgLBGOPLengthAuto";
            this->fcgLBGOPLengthAuto->Size = System::Drawing::Size(102, 22);
            this->fcgLBGOPLengthAuto->TabIndex = 37;
            this->fcgLBGOPLengthAuto->Text = L"※\"0\"で自動";
            // 
            // fcgGroupBoxAspectRatio
            // 
            this->fcgGroupBoxAspectRatio->Controls->Add(this->fcgLBAspectRatio);
            this->fcgGroupBoxAspectRatio->Controls->Add(this->fcgNUAspectRatioY);
            this->fcgGroupBoxAspectRatio->Controls->Add(this->fcgNUAspectRatioX);
            this->fcgGroupBoxAspectRatio->Controls->Add(this->fcgCXAspectRatio);
            this->fcgGroupBoxAspectRatio->Location = System::Drawing::Point(542, 41);
            this->fcgGroupBoxAspectRatio->Margin = System::Windows::Forms::Padding(5);
            this->fcgGroupBoxAspectRatio->Name = L"fcgGroupBoxAspectRatio";
            this->fcgGroupBoxAspectRatio->Padding = System::Windows::Forms::Padding(5);
            this->fcgGroupBoxAspectRatio->Size = System::Drawing::Size(329, 114);
            this->fcgGroupBoxAspectRatio->TabIndex = 80;
            this->fcgGroupBoxAspectRatio->TabStop = false;
            this->fcgGroupBoxAspectRatio->Text = L"アスペクト比";
            // 
            // fcgLBAspectRatio
            // 
            this->fcgLBAspectRatio->AutoSize = true;
            this->fcgLBAspectRatio->Location = System::Drawing::Point(154, 72);
            this->fcgLBAspectRatio->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBAspectRatio->Name = L"fcgLBAspectRatio";
            this->fcgLBAspectRatio->Size = System::Drawing::Size(17, 22);
            this->fcgLBAspectRatio->TabIndex = 3;
            this->fcgLBAspectRatio->Text = L":";
            // 
            // fcgNUAspectRatioY
            // 
            this->fcgNUAspectRatioY->Location = System::Drawing::Point(180, 70);
            this->fcgNUAspectRatioY->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUAspectRatioY->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65535, 0, 0, 0 });
            this->fcgNUAspectRatioY->Name = L"fcgNUAspectRatioY";
            this->fcgNUAspectRatioY->Size = System::Drawing::Size(106, 28);
            this->fcgNUAspectRatioY->TabIndex = 2;
            this->fcgNUAspectRatioY->Tag = L"reCmd";
            this->fcgNUAspectRatioY->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUAspectRatioX
            // 
            this->fcgNUAspectRatioX->Location = System::Drawing::Point(38, 70);
            this->fcgNUAspectRatioX->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUAspectRatioX->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65535, 0, 0, 0 });
            this->fcgNUAspectRatioX->Name = L"fcgNUAspectRatioX";
            this->fcgNUAspectRatioX->Size = System::Drawing::Size(106, 28);
            this->fcgNUAspectRatioX->TabIndex = 1;
            this->fcgNUAspectRatioX->Tag = L"reCmd";
            this->fcgNUAspectRatioX->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCXAspectRatio
            // 
            this->fcgCXAspectRatio->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAspectRatio->FormattingEnabled = true;
            this->fcgCXAspectRatio->Location = System::Drawing::Point(38, 30);
            this->fcgCXAspectRatio->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXAspectRatio->Name = L"fcgCXAspectRatio";
            this->fcgCXAspectRatio->Size = System::Drawing::Size(258, 30);
            this->fcgCXAspectRatio->TabIndex = 0;
            this->fcgCXAspectRatio->Tag = L"reCmd";
            // 
            // fcgCXInterlaced
            // 
            this->fcgCXInterlaced->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXInterlaced->FormattingEnabled = true;
            this->fcgCXInterlaced->Location = System::Drawing::Point(686, 163);
            this->fcgCXInterlaced->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXInterlaced->Name = L"fcgCXInterlaced";
            this->fcgCXInterlaced->Size = System::Drawing::Size(179, 30);
            this->fcgCXInterlaced->TabIndex = 91;
            this->fcgCXInterlaced->Tag = L"reCmd";
            // 
            // fcgLBInterlaced
            // 
            this->fcgLBInterlaced->AutoSize = true;
            this->fcgLBInterlaced->Location = System::Drawing::Point(539, 168);
            this->fcgLBInterlaced->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBInterlaced->Name = L"fcgLBInterlaced";
            this->fcgLBInterlaced->Size = System::Drawing::Size(99, 22);
            this->fcgLBInterlaced->TabIndex = 90;
            this->fcgLBInterlaced->Text = L"フレームタイプ";
            // 
            // fcgNUGopLength
            // 
            this->fcgNUGopLength->Location = System::Drawing::Point(198, 518);
            this->fcgNUGopLength->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUGopLength->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1000, 0, 0, 0 });
            this->fcgNUGopLength->Name = L"fcgNUGopLength";
            this->fcgNUGopLength->Size = System::Drawing::Size(115, 28);
            this->fcgNUGopLength->TabIndex = 36;
            this->fcgNUGopLength->Tag = L"reCmd";
            this->fcgNUGopLength->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBGOPLength
            // 
            this->fcgLBGOPLength->AutoSize = true;
            this->fcgLBGOPLength->Location = System::Drawing::Point(22, 523);
            this->fcgLBGOPLength->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBGOPLength->Name = L"fcgLBGOPLength";
            this->fcgLBGOPLength->Size = System::Drawing::Size(62, 22);
            this->fcgLBGOPLength->TabIndex = 35;
            this->fcgLBGOPLength->Text = L"GOP長";
            // 
            // fcgCXCodecLevel
            // 
            this->fcgCXCodecLevel->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXCodecLevel->FormattingEnabled = true;
            this->fcgCXCodecLevel->Location = System::Drawing::Point(167, 56);
            this->fcgCXCodecLevel->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXCodecLevel->Name = L"fcgCXCodecLevel";
            this->fcgCXCodecLevel->Size = System::Drawing::Size(179, 30);
            this->fcgCXCodecLevel->TabIndex = 114;
            this->fcgCXCodecLevel->Tag = L"reCmd";
            // 
            // fcgCXCodecProfile
            // 
            this->fcgCXCodecProfile->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXCodecProfile->FormattingEnabled = true;
            this->fcgCXCodecProfile->Location = System::Drawing::Point(167, 8);
            this->fcgCXCodecProfile->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXCodecProfile->Name = L"fcgCXCodecProfile";
            this->fcgCXCodecProfile->Size = System::Drawing::Size(179, 30);
            this->fcgCXCodecProfile->TabIndex = 112;
            this->fcgCXCodecProfile->Tag = L"reCmd";
            // 
            // fcgLBCodecLevel
            // 
            this->fcgLBCodecLevel->AutoSize = true;
            this->fcgLBCodecLevel->Location = System::Drawing::Point(20, 61);
            this->fcgLBCodecLevel->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBCodecLevel->Name = L"fcgLBCodecLevel";
            this->fcgLBCodecLevel->Size = System::Drawing::Size(51, 22);
            this->fcgLBCodecLevel->TabIndex = 113;
            this->fcgLBCodecLevel->Text = L"レベル";
            // 
            // fcgLBCodecProfile
            // 
            this->fcgLBCodecProfile->AutoSize = true;
            this->fcgLBCodecProfile->Location = System::Drawing::Point(20, 13);
            this->fcgLBCodecProfile->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBCodecProfile->Name = L"fcgLBCodecProfile";
            this->fcgLBCodecProfile->Size = System::Drawing::Size(82, 22);
            this->fcgLBCodecProfile->TabIndex = 111;
            this->fcgLBCodecProfile->Text = L"プロファイル";
            // 
            // fcgLBEncMode
            // 
            this->fcgLBEncMode->AutoSize = true;
            this->fcgLBEncMode->Location = System::Drawing::Point(19, 246);
            this->fcgLBEncMode->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBEncMode->Name = L"fcgLBEncMode";
            this->fcgLBEncMode->Size = System::Drawing::Size(49, 22);
            this->fcgLBEncMode->TabIndex = 5;
            this->fcgLBEncMode->Text = L"モード";
            // 
            // fcgCXEncMode
            // 
            this->fcgCXEncMode->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXEncMode->FormattingEnabled = true;
            this->fcgCXEncMode->Items->AddRange(gcnew cli::array< System::Object^  >(3) { L"高品質", L"標準", L"高速" });
            this->fcgCXEncMode->Location = System::Drawing::Point(121, 241);
            this->fcgCXEncMode->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXEncMode->Name = L"fcgCXEncMode";
            this->fcgCXEncMode->Size = System::Drawing::Size(280, 30);
            this->fcgCXEncMode->TabIndex = 6;
            this->fcgCXEncMode->Tag = L"reCmd";
            this->fcgCXEncMode->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgChangeEnabled);
            // 
            // fcgPNBframes
            // 
            this->fcgPNBframes->Controls->Add(this->fcgLBBframes);
            this->fcgPNBframes->Controls->Add(this->fcgNUBframes);
            this->fcgPNBframes->Controls->Add(this->fcgCBBPyramid);
            this->fcgPNBframes->Controls->Add(this->fcgLBBPyramid);
            this->fcgPNBframes->Controls->Add(this->fcgLBBDeltaQP);
            this->fcgPNBframes->Controls->Add(this->fcgNUBDeltaQP);
            this->fcgPNBframes->Controls->Add(this->fcgLBBRefDeltaQP);
            this->fcgPNBframes->Controls->Add(this->fcgNUBRefDeltaQP);
            this->fcgPNBframes->Location = System::Drawing::Point(11, 593);
            this->fcgPNBframes->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNBframes->Name = L"fcgPNBframes";
            this->fcgPNBframes->Size = System::Drawing::Size(494, 85);
            this->fcgPNBframes->TabIndex = 50;
            // 
            // fcgLBBframes
            // 
            this->fcgLBBframes->AutoSize = true;
            this->fcgLBBframes->Location = System::Drawing::Point(11, 10);
            this->fcgLBBframes->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBBframes->Name = L"fcgLBBframes";
            this->fcgLBBframes->Size = System::Drawing::Size(90, 22);
            this->fcgLBBframes->TabIndex = 51;
            this->fcgLBBframes->Text = L"Bフレーム数";
            // 
            // fcgNUBframes
            // 
            this->fcgNUBframes->Location = System::Drawing::Point(187, 5);
            this->fcgNUBframes->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUBframes->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
            this->fcgNUBframes->Name = L"fcgNUBframes";
            this->fcgNUBframes->Size = System::Drawing::Size(79, 28);
            this->fcgNUBframes->TabIndex = 52;
            this->fcgNUBframes->Tag = L"reCmd";
            this->fcgNUBframes->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCBBPyramid
            // 
            this->fcgCBBPyramid->AutoSize = true;
            this->fcgCBBPyramid->Location = System::Drawing::Point(407, 12);
            this->fcgCBBPyramid->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBBPyramid->Name = L"fcgCBBPyramid";
            this->fcgCBBPyramid->Size = System::Drawing::Size(22, 21);
            this->fcgCBBPyramid->TabIndex = 54;
            this->fcgCBBPyramid->Tag = L"reCmd";
            this->fcgCBBPyramid->UseVisualStyleBackColor = true;
            // 
            // fcgLBBPyramid
            // 
            this->fcgLBBPyramid->AutoSize = true;
            this->fcgLBBPyramid->Location = System::Drawing::Point(288, 10);
            this->fcgLBBPyramid->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBBPyramid->Name = L"fcgLBBPyramid";
            this->fcgLBBPyramid->Size = System::Drawing::Size(101, 22);
            this->fcgLBBPyramid->TabIndex = 53;
            this->fcgLBBPyramid->Text = L"ピラミッド参照";
            // 
            // fcgLBBDeltaQP
            // 
            this->fcgLBBDeltaQP->AutoSize = true;
            this->fcgLBBDeltaQP->Location = System::Drawing::Point(11, 49);
            this->fcgLBBDeltaQP->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBBDeltaQP->Name = L"fcgLBBDeltaQP";
            this->fcgLBBDeltaQP->Size = System::Drawing::Size(180, 22);
            this->fcgLBBDeltaQP->TabIndex = 55;
            this->fcgLBBDeltaQP->Text = L"QPオフセット    Bフレーム";
            // 
            // fcgNUBDeltaQP
            // 
            this->fcgNUBDeltaQP->Location = System::Drawing::Point(190, 46);
            this->fcgNUBDeltaQP->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUBDeltaQP->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
            this->fcgNUBDeltaQP->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, System::Int32::MinValue });
            this->fcgNUBDeltaQP->Name = L"fcgNUBDeltaQP";
            this->fcgNUBDeltaQP->Size = System::Drawing::Size(78, 28);
            this->fcgNUBDeltaQP->TabIndex = 56;
            this->fcgNUBDeltaQP->Tag = L"reCmd";
            this->fcgNUBDeltaQP->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBBRefDeltaQP
            // 
            this->fcgLBBRefDeltaQP->AutoSize = true;
            this->fcgLBBRefDeltaQP->Location = System::Drawing::Point(283, 49);
            this->fcgLBBRefDeltaQP->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBBRefDeltaQP->Name = L"fcgLBBRefDeltaQP";
            this->fcgLBBRefDeltaQP->Size = System::Drawing::Size(107, 22);
            this->fcgLBBRefDeltaQP->TabIndex = 57;
            this->fcgLBBRefDeltaQP->Text = L"参照Bフレーム";
            // 
            // fcgNUBRefDeltaQP
            // 
            this->fcgNUBRefDeltaQP->Location = System::Drawing::Point(407, 47);
            this->fcgNUBRefDeltaQP->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUBRefDeltaQP->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
            this->fcgNUBRefDeltaQP->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, System::Int32::MinValue });
            this->fcgNUBRefDeltaQP->Name = L"fcgNUBRefDeltaQP";
            this->fcgNUBRefDeltaQP->Size = System::Drawing::Size(78, 28);
            this->fcgNUBRefDeltaQP->TabIndex = 58;
            this->fcgNUBRefDeltaQP->Tag = L"reCmd";
            this->fcgNUBRefDeltaQP->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgPNH264Features
            // 
            this->fcgPNH264Features->Controls->Add(this->fcgCXVBAQ);
            this->fcgPNH264Features->Controls->Add(this->fcgLBVBAQ);
            this->fcgPNH264Features->Location = System::Drawing::Point(516, 445);
            this->fcgPNH264Features->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNH264Features->Name = L"fcgPNH264Features";
            this->fcgPNH264Features->Size = System::Drawing::Size(386, 38);
            this->fcgPNH264Features->TabIndex = 130;
            // 
            // fcgCXVBAQ
            // 
            this->fcgCXVBAQ->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVBAQ->FormattingEnabled = true;
            this->fcgCXVBAQ->Location = System::Drawing::Point(170, 4);
            this->fcgCXVBAQ->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVBAQ->Name = L"fcgCXVBAQ";
            this->fcgCXVBAQ->Size = System::Drawing::Size(179, 30);
            this->fcgCXVBAQ->TabIndex = 142;
            this->fcgCXVBAQ->Tag = L"reCmd";
            // 
            // fcgLBVBAQ
            // 
            this->fcgLBVBAQ->AutoSize = true;
            this->fcgLBVBAQ->Location = System::Drawing::Point(24, 10);
            this->fcgLBVBAQ->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVBAQ->Name = L"fcgLBVBAQ";
            this->fcgLBVBAQ->Size = System::Drawing::Size(58, 22);
            this->fcgLBVBAQ->TabIndex = 131;
            this->fcgLBVBAQ->Text = L"VBAQ";
            // 
            // fcgPNBitrate
            // 
            this->fcgPNBitrate->Controls->Add(this->fcgLBBitrate);
            this->fcgPNBitrate->Controls->Add(this->fcgNUBitrate);
            this->fcgPNBitrate->Controls->Add(this->fcgLBBitrate2);
            this->fcgPNBitrate->Controls->Add(this->fcgNUMaxkbps);
            this->fcgPNBitrate->Controls->Add(this->fcgLBMaxkbps);
            this->fcgPNBitrate->Controls->Add(this->fcgLBMaxBitrate2);
            this->fcgPNBitrate->Location = System::Drawing::Point(12, 323);
            this->fcgPNBitrate->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNBitrate->Name = L"fcgPNBitrate";
            this->fcgPNBitrate->Size = System::Drawing::Size(433, 82);
            this->fcgPNBitrate->TabIndex = 10;
            // 
            // fcgLBBitrate
            // 
            this->fcgLBBitrate->AutoSize = true;
            this->fcgLBBitrate->Location = System::Drawing::Point(7, 6);
            this->fcgLBBitrate->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBBitrate->Name = L"fcgLBBitrate";
            this->fcgLBBitrate->Size = System::Drawing::Size(83, 22);
            this->fcgLBBitrate->TabIndex = 11;
            this->fcgLBBitrate->Text = L"ビットレート";
            // 
            // fcgNUBitrate
            // 
            this->fcgNUBitrate->Location = System::Drawing::Point(186, 2);
            this->fcgNUBitrate->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUBitrate->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65536000, 0, 0, 0 });
            this->fcgNUBitrate->Name = L"fcgNUBitrate";
            this->fcgNUBitrate->Size = System::Drawing::Size(115, 28);
            this->fcgNUBitrate->TabIndex = 12;
            this->fcgNUBitrate->Tag = L"reCmd";
            this->fcgNUBitrate->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBBitrate2
            // 
            this->fcgLBBitrate2->AutoSize = true;
            this->fcgLBBitrate2->Location = System::Drawing::Point(311, 6);
            this->fcgLBBitrate2->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBBitrate2->Name = L"fcgLBBitrate2";
            this->fcgLBBitrate2->Size = System::Drawing::Size(47, 22);
            this->fcgLBBitrate2->TabIndex = 13;
            this->fcgLBBitrate2->Text = L"kbps";
            // 
            // fcgNUMaxkbps
            // 
            this->fcgNUMaxkbps->Location = System::Drawing::Point(186, 43);
            this->fcgNUMaxkbps->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUMaxkbps->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65536000, 0, 0, 0 });
            this->fcgNUMaxkbps->Name = L"fcgNUMaxkbps";
            this->fcgNUMaxkbps->Size = System::Drawing::Size(115, 28);
            this->fcgNUMaxkbps->TabIndex = 15;
            this->fcgNUMaxkbps->Tag = L"reCmd";
            this->fcgNUMaxkbps->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBMaxkbps
            // 
            this->fcgLBMaxkbps->AutoSize = true;
            this->fcgLBMaxkbps->Location = System::Drawing::Point(7, 49);
            this->fcgLBMaxkbps->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBMaxkbps->Name = L"fcgLBMaxkbps";
            this->fcgLBMaxkbps->Size = System::Drawing::Size(117, 22);
            this->fcgLBMaxkbps->TabIndex = 14;
            this->fcgLBMaxkbps->Text = L"最大ビットレート";
            // 
            // fcgLBMaxBitrate2
            // 
            this->fcgLBMaxBitrate2->AutoSize = true;
            this->fcgLBMaxBitrate2->Location = System::Drawing::Point(311, 47);
            this->fcgLBMaxBitrate2->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBMaxBitrate2->Name = L"fcgLBMaxBitrate2";
            this->fcgLBMaxBitrate2->Size = System::Drawing::Size(47, 22);
            this->fcgLBMaxBitrate2->TabIndex = 16;
            this->fcgLBMaxBitrate2->Text = L"kbps";
            // 
            // fcgPNQP
            // 
            this->fcgPNQP->Controls->Add(this->fcgLBQPI);
            this->fcgPNQP->Controls->Add(this->fcgNUQPI);
            this->fcgPNQP->Controls->Add(this->fcgNUQPP);
            this->fcgPNQP->Controls->Add(this->fcgNUQPB);
            this->fcgPNQP->Controls->Add(this->fcgLBQPP);
            this->fcgPNQP->Controls->Add(this->fcgLBQPB);
            this->fcgPNQP->Location = System::Drawing::Point(12, 323);
            this->fcgPNQP->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNQP->Name = L"fcgPNQP";
            this->fcgPNQP->Size = System::Drawing::Size(433, 119);
            this->fcgPNQP->TabIndex = 20;
            // 
            // fcgLBQPI
            // 
            this->fcgLBQPI->AutoSize = true;
            this->fcgLBQPI->Location = System::Drawing::Point(14, 6);
            this->fcgLBQPI->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBQPI->Name = L"fcgLBQPI";
            this->fcgLBQPI->Size = System::Drawing::Size(101, 22);
            this->fcgLBQPI->TabIndex = 21;
            this->fcgLBQPI->Text = L"QP I frame";
            // 
            // fcgNUQPI
            // 
            this->fcgNUQPI->Location = System::Drawing::Point(186, 2);
            this->fcgNUQPI->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUQPI->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 51, 0, 0, 0 });
            this->fcgNUQPI->Name = L"fcgNUQPI";
            this->fcgNUQPI->Size = System::Drawing::Size(115, 28);
            this->fcgNUQPI->TabIndex = 22;
            this->fcgNUQPI->Tag = L"reCmd";
            this->fcgNUQPI->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUQPP
            // 
            this->fcgNUQPP->Location = System::Drawing::Point(186, 43);
            this->fcgNUQPP->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUQPP->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 51, 0, 0, 0 });
            this->fcgNUQPP->Name = L"fcgNUQPP";
            this->fcgNUQPP->Size = System::Drawing::Size(115, 28);
            this->fcgNUQPP->TabIndex = 24;
            this->fcgNUQPP->Tag = L"reCmd";
            this->fcgNUQPP->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUQPB
            // 
            this->fcgNUQPB->Location = System::Drawing::Point(186, 83);
            this->fcgNUQPB->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUQPB->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 51, 0, 0, 0 });
            this->fcgNUQPB->Name = L"fcgNUQPB";
            this->fcgNUQPB->Size = System::Drawing::Size(115, 28);
            this->fcgNUQPB->TabIndex = 26;
            this->fcgNUQPB->Tag = L"reCmd";
            this->fcgNUQPB->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBQPP
            // 
            this->fcgLBQPP->AutoSize = true;
            this->fcgLBQPP->Location = System::Drawing::Point(14, 47);
            this->fcgLBQPP->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBQPP->Name = L"fcgLBQPP";
            this->fcgLBQPP->Size = System::Drawing::Size(104, 22);
            this->fcgLBQPP->TabIndex = 23;
            this->fcgLBQPP->Text = L"QP P frame";
            // 
            // fcgLBQPB
            // 
            this->fcgLBQPB->AutoSize = true;
            this->fcgLBQPB->Location = System::Drawing::Point(14, 85);
            this->fcgLBQPB->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBQPB->Name = L"fcgLBQPB";
            this->fcgLBQPB->Size = System::Drawing::Size(113, 22);
            this->fcgLBQPB->TabIndex = 25;
            this->fcgLBQPB->Text = L"QP B frames";
            // 
            // tabPageVideoEnc2
            // 
            this->tabPageVideoEnc2->Controls->Add(this->fcgCXMotionEst);
            this->tabPageVideoEnc2->Controls->Add(this->fcgLBMotionEst);
            this->tabPageVideoEnc2->Controls->Add(this->fcgNUSlices);
            this->tabPageVideoEnc2->Controls->Add(this->fcgLBSlices);
            this->tabPageVideoEnc2->Controls->Add(this->fcgCBPreEncode);
            this->tabPageVideoEnc2->Controls->Add(this->fcgCBPreAnalysis);
            this->tabPageVideoEnc2->Controls->Add(this->fcggroupBoxPreAnalysis);
            this->tabPageVideoEnc2->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->tabPageVideoEnc2->Location = System::Drawing::Point(4, 32);
            this->tabPageVideoEnc2->Margin = System::Windows::Forms::Padding(5);
            this->tabPageVideoEnc2->Name = L"tabPageVideoEnc2";
            this->tabPageVideoEnc2->Size = System::Drawing::Size(916, 727);
            this->tabPageVideoEnc2->TabIndex = 3;
            this->tabPageVideoEnc2->Text = L"動画エンコード(2)";
            this->tabPageVideoEnc2->UseVisualStyleBackColor = true;
            // 
            // fcgCXMotionEst
            // 
            this->fcgCXMotionEst->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXMotionEst->FormattingEnabled = true;
            this->fcgCXMotionEst->Location = System::Drawing::Point(150, 467);
            this->fcgCXMotionEst->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXMotionEst->Name = L"fcgCXMotionEst";
            this->fcgCXMotionEst->Size = System::Drawing::Size(146, 30);
            this->fcgCXMotionEst->TabIndex = 128;
            this->fcgCXMotionEst->Tag = L"reCmd";
            // 
            // fcgLBMotionEst
            // 
            this->fcgLBMotionEst->AutoSize = true;
            this->fcgLBMotionEst->Location = System::Drawing::Point(38, 472);
            this->fcgLBMotionEst->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBMotionEst->Name = L"fcgLBMotionEst";
            this->fcgLBMotionEst->Size = System::Drawing::Size(74, 22);
            this->fcgLBMotionEst->TabIndex = 127;
            this->fcgLBMotionEst->Text = L"動き予測";
            // 
            // fcgNUSlices
            // 
            this->fcgNUSlices->Location = System::Drawing::Point(437, 470);
            this->fcgNUSlices->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUSlices->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUSlices->Name = L"fcgNUSlices";
            this->fcgNUSlices->Size = System::Drawing::Size(77, 28);
            this->fcgNUSlices->TabIndex = 130;
            this->fcgNUSlices->Tag = L"reCmd";
            this->fcgNUSlices->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUSlices->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            // 
            // fcgLBSlices
            // 
            this->fcgLBSlices->AutoSize = true;
            this->fcgLBSlices->Location = System::Drawing::Point(317, 472);
            this->fcgLBSlices->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBSlices->Name = L"fcgLBSlices";
            this->fcgLBSlices->Size = System::Drawing::Size(77, 22);
            this->fcgLBSlices->TabIndex = 129;
            this->fcgLBSlices->Text = L"スライス数";
            // 
            // fcgCBPreEncode
            // 
            this->fcgCBPreEncode->AutoSize = true;
            this->fcgCBPreEncode->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgCBPreEncode->Location = System::Drawing::Point(42, 12);
            this->fcgCBPreEncode->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBPreEncode->Name = L"fcgCBPreEncode";
            this->fcgCBPreEncode->Size = System::Drawing::Size(130, 26);
            this->fcgCBPreEncode->TabIndex = 126;
            this->fcgCBPreEncode->Tag = L"reCmd";
            this->fcgCBPreEncode->Text = L"Pre Encode";
            this->fcgCBPreEncode->UseVisualStyleBackColor = true;
            // 
            // fcgCBPreAnalysis
            // 
            this->fcgCBPreAnalysis->AutoSize = true;
            this->fcgCBPreAnalysis->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgCBPreAnalysis->Location = System::Drawing::Point(41, 50);
            this->fcgCBPreAnalysis->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBPreAnalysis->Name = L"fcgCBPreAnalysis";
            this->fcgCBPreAnalysis->Size = System::Drawing::Size(136, 26);
            this->fcgCBPreAnalysis->TabIndex = 125;
            this->fcgCBPreAnalysis->Tag = L"reCmd";
            this->fcgCBPreAnalysis->Text = L"Pre Analysis";
            this->fcgCBPreAnalysis->UseVisualStyleBackColor = true;
            this->fcgCBPreAnalysis->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgCBPreAnalysis_CheckedChanged);
            // 
            // fcggroupBoxPreAnalysis
            // 
            this->fcggroupBoxPreAnalysis->Controls->Add(this->fcgLBPAMotionQuality);
            this->fcggroupBoxPreAnalysis->Controls->Add(this->fcgCXPAMotionQuality);
            this->fcggroupBoxPreAnalysis->Controls->Add(this->fcgLBPALookahead);
            this->fcggroupBoxPreAnalysis->Controls->Add(this->fcgCXPATAQ);
            this->fcggroupBoxPreAnalysis->Controls->Add(this->fcgLBPATAQ);
            this->fcggroupBoxPreAnalysis->Controls->Add(this->fcgCXPAPAQ);
            this->fcggroupBoxPreAnalysis->Controls->Add(this->fcgLBPAPAQ);
            this->fcggroupBoxPreAnalysis->Controls->Add(this->fcgCXPACAQ);
            this->fcggroupBoxPreAnalysis->Controls->Add(this->fcgLBPACAQ);
            this->fcggroupBoxPreAnalysis->Controls->Add(this->fcgCXPAActivityType);
            this->fcggroupBoxPreAnalysis->Controls->Add(this->fcgLBPAActivityType);
            this->fcggroupBoxPreAnalysis->Controls->Add(this->fcgLBPASS);
            this->fcggroupBoxPreAnalysis->Controls->Add(this->fcgCXPASS);
            this->fcggroupBoxPreAnalysis->Controls->Add(this->fcgLBPASC);
            this->fcggroupBoxPreAnalysis->Controls->Add(this->fcgNUPALookahead);
            this->fcggroupBoxPreAnalysis->Controls->Add(this->fcgCXPASC);
            this->fcggroupBoxPreAnalysis->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcggroupBoxPreAnalysis->Location = System::Drawing::Point(23, 49);
            this->fcggroupBoxPreAnalysis->Margin = System::Windows::Forms::Padding(5);
            this->fcggroupBoxPreAnalysis->Name = L"fcggroupBoxPreAnalysis";
            this->fcggroupBoxPreAnalysis->Padding = System::Windows::Forms::Padding(5);
            this->fcggroupBoxPreAnalysis->Size = System::Drawing::Size(503, 402);
            this->fcggroupBoxPreAnalysis->TabIndex = 81;
            this->fcggroupBoxPreAnalysis->TabStop = false;
            // 
            // fcgLBPAMotionQuality
            // 
            this->fcgLBPAMotionQuality->AutoSize = true;
            this->fcgLBPAMotionQuality->Location = System::Drawing::Point(14, 342);
            this->fcgLBPAMotionQuality->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBPAMotionQuality->Name = L"fcgLBPAMotionQuality";
            this->fcgLBPAMotionQuality->Size = System::Drawing::Size(161, 22);
            this->fcgLBPAMotionQuality->TabIndex = 16;
            this->fcgLBPAMotionQuality->Text = L"動き品質の向上モード";
            // 
            // fcgCXPAMotionQuality
            // 
            this->fcgCXPAMotionQuality->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXPAMotionQuality->FormattingEnabled = true;
            this->fcgCXPAMotionQuality->Location = System::Drawing::Point(234, 337);
            this->fcgCXPAMotionQuality->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXPAMotionQuality->Name = L"fcgCXPAMotionQuality";
            this->fcgCXPAMotionQuality->Size = System::Drawing::Size(251, 30);
            this->fcgCXPAMotionQuality->TabIndex = 15;
            this->fcgCXPAMotionQuality->Tag = L"reCmd";
            // 
            // fcgLBPALookahead
            // 
            this->fcgLBPALookahead->AutoSize = true;
            this->fcgLBPALookahead->Location = System::Drawing::Point(14, 298);
            this->fcgLBPALookahead->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBPALookahead->Name = L"fcgLBPALookahead";
            this->fcgLBPALookahead->Size = System::Drawing::Size(163, 22);
            this->fcgLBPALookahead->TabIndex = 14;
            this->fcgLBPALookahead->Text = L"先行探索バッファサイズ";
            // 
            // fcgCXPATAQ
            // 
            this->fcgCXPATAQ->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXPATAQ->FormattingEnabled = true;
            this->fcgCXPATAQ->Location = System::Drawing::Point(234, 251);
            this->fcgCXPATAQ->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXPATAQ->Name = L"fcgCXPATAQ";
            this->fcgCXPATAQ->Size = System::Drawing::Size(251, 30);
            this->fcgCXPATAQ->TabIndex = 13;
            this->fcgCXPATAQ->Tag = L"reCmd";
            // 
            // fcgLBPATAQ
            // 
            this->fcgLBPATAQ->AutoSize = true;
            this->fcgLBPATAQ->Location = System::Drawing::Point(14, 254);
            this->fcgLBPATAQ->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBPATAQ->Name = L"fcgLBPATAQ";
            this->fcgLBPATAQ->Size = System::Drawing::Size(152, 22);
            this->fcgLBPATAQ->TabIndex = 12;
            this->fcgLBPATAQ->Text = L"Temporal AQ 強さ";
            // 
            // fcgCXPAPAQ
            // 
            this->fcgCXPAPAQ->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXPAPAQ->FormattingEnabled = true;
            this->fcgCXPAPAQ->Location = System::Drawing::Point(234, 206);
            this->fcgCXPAPAQ->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXPAPAQ->Name = L"fcgCXPAPAQ";
            this->fcgCXPAPAQ->Size = System::Drawing::Size(251, 30);
            this->fcgCXPAPAQ->TabIndex = 11;
            this->fcgCXPAPAQ->Tag = L"reCmd";
            // 
            // fcgLBPAPAQ
            // 
            this->fcgLBPAPAQ->AutoSize = true;
            this->fcgLBPAPAQ->Location = System::Drawing::Point(14, 211);
            this->fcgLBPAPAQ->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBPAPAQ->Name = L"fcgLBPAPAQ";
            this->fcgLBPAPAQ->Size = System::Drawing::Size(163, 22);
            this->fcgLBPAPAQ->TabIndex = 10;
            this->fcgLBPAPAQ->Text = L"Perceptual AQ 強さ";
            // 
            // fcgCXPACAQ
            // 
            this->fcgCXPACAQ->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXPACAQ->FormattingEnabled = true;
            this->fcgCXPACAQ->Location = System::Drawing::Point(234, 163);
            this->fcgCXPACAQ->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXPACAQ->Name = L"fcgCXPACAQ";
            this->fcgCXPACAQ->Size = System::Drawing::Size(251, 30);
            this->fcgCXPACAQ->TabIndex = 9;
            this->fcgCXPACAQ->Tag = L"reCmd";
            // 
            // fcgLBPACAQ
            // 
            this->fcgLBPACAQ->AutoSize = true;
            this->fcgLBPACAQ->Location = System::Drawing::Point(14, 168);
            this->fcgLBPACAQ->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBPACAQ->Name = L"fcgLBPACAQ";
            this->fcgLBPACAQ->Size = System::Drawing::Size(141, 22);
            this->fcgLBPACAQ->TabIndex = 8;
            this->fcgLBPACAQ->Text = L"Content AQ 強さ";
            // 
            // fcgCXPAActivityType
            // 
            this->fcgCXPAActivityType->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXPAActivityType->FormattingEnabled = true;
            this->fcgCXPAActivityType->Location = System::Drawing::Point(234, 120);
            this->fcgCXPAActivityType->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXPAActivityType->Name = L"fcgCXPAActivityType";
            this->fcgCXPAActivityType->Size = System::Drawing::Size(251, 30);
            this->fcgCXPAActivityType->TabIndex = 7;
            this->fcgCXPAActivityType->Tag = L"reCmd";
            // 
            // fcgLBPAActivityType
            // 
            this->fcgLBPAActivityType->AutoSize = true;
            this->fcgLBPAActivityType->Location = System::Drawing::Point(14, 125);
            this->fcgLBPAActivityType->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBPAActivityType->Name = L"fcgLBPAActivityType";
            this->fcgLBPAActivityType->Size = System::Drawing::Size(127, 22);
            this->fcgLBPAActivityType->TabIndex = 6;
            this->fcgLBPAActivityType->Text = L"動きの検出モード";
            // 
            // fcgLBPASS
            // 
            this->fcgLBPASS->AutoSize = true;
            this->fcgLBPASS->Location = System::Drawing::Point(14, 82);
            this->fcgLBPASS->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBPASS->Name = L"fcgLBPASS";
            this->fcgLBPASS->Size = System::Drawing::Size(153, 22);
            this->fcgLBPASS->TabIndex = 5;
            this->fcgLBPASS->Text = L"静止シーン検出感度";
            // 
            // fcgCXPASS
            // 
            this->fcgCXPASS->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXPASS->FormattingEnabled = true;
            this->fcgCXPASS->Location = System::Drawing::Point(234, 77);
            this->fcgCXPASS->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXPASS->Name = L"fcgCXPASS";
            this->fcgCXPASS->Size = System::Drawing::Size(251, 30);
            this->fcgCXPASS->TabIndex = 4;
            this->fcgCXPASS->Tag = L"reCmd";
            // 
            // fcgLBPASC
            // 
            this->fcgLBPASC->AutoSize = true;
            this->fcgLBPASC->Location = System::Drawing::Point(14, 37);
            this->fcgLBPASC->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBPASC->Name = L"fcgLBPASC";
            this->fcgLBPASC->Size = System::Drawing::Size(169, 22);
            this->fcgLBPASC->TabIndex = 3;
            this->fcgLBPASC->Text = L"シーンチェンジ検出感度";
            // 
            // fcgNUPALookahead
            // 
            this->fcgNUPALookahead->Location = System::Drawing::Point(234, 294);
            this->fcgNUPALookahead->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUPALookahead->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 512, 0, 0, 0 });
            this->fcgNUPALookahead->Name = L"fcgNUPALookahead";
            this->fcgNUPALookahead->Size = System::Drawing::Size(106, 28);
            this->fcgNUPALookahead->TabIndex = 1;
            this->fcgNUPALookahead->Tag = L"reCmd";
            this->fcgNUPALookahead->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCXPASC
            // 
            this->fcgCXPASC->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXPASC->FormattingEnabled = true;
            this->fcgCXPASC->Location = System::Drawing::Point(234, 34);
            this->fcgCXPASC->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXPASC->Name = L"fcgCXPASC";
            this->fcgCXPASC->Size = System::Drawing::Size(251, 30);
            this->fcgCXPASC->TabIndex = 0;
            this->fcgCXPASC->Tag = L"reCmd";
            // 
            // tabPageVpp
            // 
            this->tabPageVpp->Controls->Add(this->fcggroupBoxVppDeband);
            this->tabPageVpp->Controls->Add(this->fcggroupBoxVppDetailEnahance);
            this->tabPageVpp->Controls->Add(this->fcggroupBoxVppDenoise);
            this->tabPageVpp->Controls->Add(this->fcgCBPSNR);
            this->tabPageVpp->Controls->Add(this->fcgCBSSIM);
            this->tabPageVpp->Controls->Add(this->fcggroupBoxVppDeinterlace);
            this->tabPageVpp->Controls->Add(this->fcgCBVppResize);
            this->tabPageVpp->Controls->Add(this->fcggroupBoxResize);
            this->tabPageVpp->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->tabPageVpp->Location = System::Drawing::Point(4, 32);
            this->tabPageVpp->Margin = System::Windows::Forms::Padding(5);
            this->tabPageVpp->Name = L"tabPageVpp";
            this->tabPageVpp->Size = System::Drawing::Size(916, 727);
            this->tabPageVpp->TabIndex = 1;
            this->tabPageVpp->Text = L"フィルタ";
            this->tabPageVpp->UseVisualStyleBackColor = true;
            // 
            // fcggroupBoxVppDeband
            // 
            this->fcggroupBoxVppDeband->Controls->Add(this->fcgPNVppLibplaceboDeband);
            this->fcggroupBoxVppDeband->Controls->Add(this->fcgCXVppDeband);
            this->fcggroupBoxVppDeband->Controls->Add(this->fcgPNVppDeband);
            this->fcggroupBoxVppDeband->Location = System::Drawing::Point(6, 478);
            this->fcggroupBoxVppDeband->Margin = System::Windows::Forms::Padding(5);
            this->fcggroupBoxVppDeband->Name = L"fcggroupBoxVppDeband";
            this->fcggroupBoxVppDeband->Padding = System::Windows::Forms::Padding(5);
            this->fcggroupBoxVppDeband->Size = System::Drawing::Size(480, 240);
            this->fcggroupBoxVppDeband->TabIndex = 41;
            this->fcggroupBoxVppDeband->TabStop = false;
            this->fcggroupBoxVppDeband->Text = L"バンディング低減";
            // 
            // fcgPNVppLibplaceboDeband
            // 
            this->fcgPNVppLibplaceboDeband->Controls->Add(this->fcgLBVppLibplaceboDebandLUTSize);
            this->fcgPNVppLibplaceboDeband->Controls->Add(this->fcgCXVppLibplaceboDebandLUTSize);
            this->fcgPNVppLibplaceboDeband->Controls->Add(this->fcgNUVppLibplaceboDebandRadius);
            this->fcgPNVppLibplaceboDeband->Controls->Add(this->fcgLBVppLibplaceboDebandRadius);
            this->fcgPNVppLibplaceboDeband->Controls->Add(this->fcgNUVppLibplaceboDebandThreshold);
            this->fcgPNVppLibplaceboDeband->Controls->Add(this->fcgLBVppLibplaceboDebandDither);
            this->fcgPNVppLibplaceboDeband->Controls->Add(this->fcgLBVppLibplaceboDebandGrainC);
            this->fcgPNVppLibplaceboDeband->Controls->Add(this->fcgLBVppLibplaceboDebandGrainY);
            this->fcgPNVppLibplaceboDeband->Controls->Add(this->fcgLBVppLibplaceboDebandGrain);
            this->fcgPNVppLibplaceboDeband->Controls->Add(this->fcgNUVppLibplaceboDebandGrainC);
            this->fcgPNVppLibplaceboDeband->Controls->Add(this->fcgNUVppLibplaceboDebandGrainY);
            this->fcgPNVppLibplaceboDeband->Controls->Add(this->fcgLBVppLibplaceboDebandThreshold);
            this->fcgPNVppLibplaceboDeband->Controls->Add(this->fcgLBVppLibplaceboDebandIteration);
            this->fcgPNVppLibplaceboDeband->Controls->Add(this->fcgCXVppLibplaceboDebandDither);
            this->fcgPNVppLibplaceboDeband->Controls->Add(this->fcgNUVppLibplaceboDebandIteration);
            this->fcgPNVppLibplaceboDeband->Location = System::Drawing::Point(7, 59);
            this->fcgPNVppLibplaceboDeband->Margin = System::Windows::Forms::Padding(4);
            this->fcgPNVppLibplaceboDeband->Name = L"fcgPNVppLibplaceboDeband";
            this->fcgPNVppLibplaceboDeband->Size = System::Drawing::Size(466, 174);
            this->fcgPNVppLibplaceboDeband->TabIndex = 22;
            // 
            // fcgLBVppLibplaceboDebandLUTSize
            // 
            this->fcgLBVppLibplaceboDebandLUTSize->AutoSize = true;
            this->fcgLBVppLibplaceboDebandLUTSize->Location = System::Drawing::Point(293, 112);
            this->fcgLBVppLibplaceboDebandLUTSize->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppLibplaceboDebandLUTSize->Name = L"fcgLBVppLibplaceboDebandLUTSize";
            this->fcgLBVppLibplaceboDebandLUTSize->Size = System::Drawing::Size(82, 22);
            this->fcgLBVppLibplaceboDebandLUTSize->TabIndex = 40;
            this->fcgLBVppLibplaceboDebandLUTSize->Text = L"LUTサイズ";
            // 
            // fcgCXVppLibplaceboDebandLUTSize
            // 
            this->fcgCXVppLibplaceboDebandLUTSize->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppLibplaceboDebandLUTSize->FormattingEnabled = true;
            this->fcgCXVppLibplaceboDebandLUTSize->Location = System::Drawing::Point(382, 108);
            this->fcgCXVppLibplaceboDebandLUTSize->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppLibplaceboDebandLUTSize->Name = L"fcgCXVppLibplaceboDebandLUTSize";
            this->fcgCXVppLibplaceboDebandLUTSize->Size = System::Drawing::Size(69, 30);
            this->fcgCXVppLibplaceboDebandLUTSize->TabIndex = 39;
            this->fcgCXVppLibplaceboDebandLUTSize->Tag = L"reCmd";
            // 
            // fcgNUVppLibplaceboDebandRadius
            // 
            this->fcgNUVppLibplaceboDebandRadius->DecimalPlaces = 1;
            this->fcgNUVppLibplaceboDebandRadius->Location = System::Drawing::Point(115, 38);
            this->fcgNUVppLibplaceboDebandRadius->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppLibplaceboDebandRadius->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 127, 0, 0, 0 });
            this->fcgNUVppLibplaceboDebandRadius->Name = L"fcgNUVppLibplaceboDebandRadius";
            this->fcgNUVppLibplaceboDebandRadius->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppLibplaceboDebandRadius->TabIndex = 38;
            this->fcgNUVppLibplaceboDebandRadius->Tag = L"reCmd";
            this->fcgNUVppLibplaceboDebandRadius->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBVppLibplaceboDebandRadius
            // 
            this->fcgLBVppLibplaceboDebandRadius->AutoSize = true;
            this->fcgLBVppLibplaceboDebandRadius->Location = System::Drawing::Point(5, 41);
            this->fcgLBVppLibplaceboDebandRadius->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppLibplaceboDebandRadius->Name = L"fcgLBVppLibplaceboDebandRadius";
            this->fcgLBVppLibplaceboDebandRadius->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppLibplaceboDebandRadius->TabIndex = 37;
            this->fcgLBVppLibplaceboDebandRadius->Text = L"半径";
            // 
            // fcgNUVppLibplaceboDebandThreshold
            // 
            this->fcgNUVppLibplaceboDebandThreshold->DecimalPlaces = 1;
            this->fcgNUVppLibplaceboDebandThreshold->Location = System::Drawing::Point(359, 40);
            this->fcgNUVppLibplaceboDebandThreshold->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppLibplaceboDebandThreshold->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 127, 0, 0, 0 });
            this->fcgNUVppLibplaceboDebandThreshold->Name = L"fcgNUVppLibplaceboDebandThreshold";
            this->fcgNUVppLibplaceboDebandThreshold->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppLibplaceboDebandThreshold->TabIndex = 36;
            this->fcgNUVppLibplaceboDebandThreshold->Tag = L"reCmd";
            this->fcgNUVppLibplaceboDebandThreshold->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBVppLibplaceboDebandDither
            // 
            this->fcgLBVppLibplaceboDebandDither->AutoSize = true;
            this->fcgLBVppLibplaceboDebandDither->Location = System::Drawing::Point(2, 112);
            this->fcgLBVppLibplaceboDebandDither->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppLibplaceboDebandDither->Name = L"fcgLBVppLibplaceboDebandDither";
            this->fcgLBVppLibplaceboDebandDither->Size = System::Drawing::Size(58, 22);
            this->fcgLBVppLibplaceboDebandDither->TabIndex = 32;
            this->fcgLBVppLibplaceboDebandDither->Text = L"dither";
            // 
            // fcgLBVppLibplaceboDebandGrainC
            // 
            this->fcgLBVppLibplaceboDebandGrainC->AutoSize = true;
            this->fcgLBVppLibplaceboDebandGrainC->Location = System::Drawing::Point(206, 76);
            this->fcgLBVppLibplaceboDebandGrainC->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppLibplaceboDebandGrainC->Name = L"fcgLBVppLibplaceboDebandGrainC";
            this->fcgLBVppLibplaceboDebandGrainC->Size = System::Drawing::Size(21, 22);
            this->fcgLBVppLibplaceboDebandGrainC->TabIndex = 30;
            this->fcgLBVppLibplaceboDebandGrainC->Text = L"C";
            // 
            // fcgLBVppLibplaceboDebandGrainY
            // 
            this->fcgLBVppLibplaceboDebandGrainY->AutoSize = true;
            this->fcgLBVppLibplaceboDebandGrainY->Location = System::Drawing::Point(88, 76);
            this->fcgLBVppLibplaceboDebandGrainY->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppLibplaceboDebandGrainY->Name = L"fcgLBVppLibplaceboDebandGrainY";
            this->fcgLBVppLibplaceboDebandGrainY->Size = System::Drawing::Size(21, 22);
            this->fcgLBVppLibplaceboDebandGrainY->TabIndex = 28;
            this->fcgLBVppLibplaceboDebandGrainY->Text = L"Y";
            // 
            // fcgLBVppLibplaceboDebandGrain
            // 
            this->fcgLBVppLibplaceboDebandGrain->AutoSize = true;
            this->fcgLBVppLibplaceboDebandGrain->Location = System::Drawing::Point(2, 76);
            this->fcgLBVppLibplaceboDebandGrain->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppLibplaceboDebandGrain->Name = L"fcgLBVppLibplaceboDebandGrain";
            this->fcgLBVppLibplaceboDebandGrain->Size = System::Drawing::Size(52, 22);
            this->fcgLBVppLibplaceboDebandGrain->TabIndex = 27;
            this->fcgLBVppLibplaceboDebandGrain->Text = L"grain";
            // 
            // fcgNUVppLibplaceboDebandGrainC
            // 
            this->fcgNUVppLibplaceboDebandGrainC->Location = System::Drawing::Point(234, 73);
            this->fcgNUVppLibplaceboDebandGrainC->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppLibplaceboDebandGrainC->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 31, 0, 0, 0 });
            this->fcgNUVppLibplaceboDebandGrainC->Name = L"fcgNUVppLibplaceboDebandGrainC";
            this->fcgNUVppLibplaceboDebandGrainC->Size = System::Drawing::Size(74, 28);
            this->fcgNUVppLibplaceboDebandGrainC->TabIndex = 31;
            this->fcgNUVppLibplaceboDebandGrainC->Tag = L"reCmd";
            this->fcgNUVppLibplaceboDebandGrainC->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUVppLibplaceboDebandGrainY
            // 
            this->fcgNUVppLibplaceboDebandGrainY->Location = System::Drawing::Point(115, 73);
            this->fcgNUVppLibplaceboDebandGrainY->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppLibplaceboDebandGrainY->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 31, 0, 0, 0 });
            this->fcgNUVppLibplaceboDebandGrainY->Name = L"fcgNUVppLibplaceboDebandGrainY";
            this->fcgNUVppLibplaceboDebandGrainY->Size = System::Drawing::Size(74, 28);
            this->fcgNUVppLibplaceboDebandGrainY->TabIndex = 29;
            this->fcgNUVppLibplaceboDebandGrainY->Tag = L"reCmd";
            this->fcgNUVppLibplaceboDebandGrainY->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBVppLibplaceboDebandThreshold
            // 
            this->fcgLBVppLibplaceboDebandThreshold->AutoSize = true;
            this->fcgLBVppLibplaceboDebandThreshold->Location = System::Drawing::Point(290, 42);
            this->fcgLBVppLibplaceboDebandThreshold->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppLibplaceboDebandThreshold->Name = L"fcgLBVppLibplaceboDebandThreshold";
            this->fcgLBVppLibplaceboDebandThreshold->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppLibplaceboDebandThreshold->TabIndex = 20;
            this->fcgLBVppLibplaceboDebandThreshold->Text = L"閾値";
            // 
            // fcgLBVppLibplaceboDebandIteration
            // 
            this->fcgLBVppLibplaceboDebandIteration->AutoSize = true;
            this->fcgLBVppLibplaceboDebandIteration->Location = System::Drawing::Point(2, 7);
            this->fcgLBVppLibplaceboDebandIteration->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppLibplaceboDebandIteration->Name = L"fcgLBVppLibplaceboDebandIteration";
            this->fcgLBVppLibplaceboDebandIteration->Size = System::Drawing::Size(78, 22);
            this->fcgLBVppLibplaceboDebandIteration->TabIndex = 18;
            this->fcgLBVppLibplaceboDebandIteration->Text = L"iteration";
            // 
            // fcgCXVppLibplaceboDebandDither
            // 
            this->fcgCXVppLibplaceboDebandDither->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppLibplaceboDebandDither->FormattingEnabled = true;
            this->fcgCXVppLibplaceboDebandDither->Location = System::Drawing::Point(94, 108);
            this->fcgCXVppLibplaceboDebandDither->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppLibplaceboDebandDither->Name = L"fcgCXVppLibplaceboDebandDither";
            this->fcgCXVppLibplaceboDebandDither->Size = System::Drawing::Size(177, 30);
            this->fcgCXVppLibplaceboDebandDither->TabIndex = 33;
            this->fcgCXVppLibplaceboDebandDither->Tag = L"reCmd";
            // 
            // fcgNUVppLibplaceboDebandIteration
            // 
            this->fcgNUVppLibplaceboDebandIteration->Location = System::Drawing::Point(115, 5);
            this->fcgNUVppLibplaceboDebandIteration->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppLibplaceboDebandIteration->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 127, 0, 0, 0 });
            this->fcgNUVppLibplaceboDebandIteration->Name = L"fcgNUVppLibplaceboDebandIteration";
            this->fcgNUVppLibplaceboDebandIteration->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppLibplaceboDebandIteration->TabIndex = 19;
            this->fcgNUVppLibplaceboDebandIteration->Tag = L"reCmd";
            this->fcgNUVppLibplaceboDebandIteration->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCXVppDeband
            // 
            this->fcgCXVppDeband->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppDeband->FormattingEnabled = true;
            this->fcgCXVppDeband->Location = System::Drawing::Point(38, 24);
            this->fcgCXVppDeband->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppDeband->Name = L"fcgCXVppDeband";
            this->fcgCXVppDeband->Size = System::Drawing::Size(285, 30);
            this->fcgCXVppDeband->TabIndex = 21;
            this->fcgCXVppDeband->Tag = L"reCmd";
            this->fcgCXVppDeband->SelectionChangeCommitted += gcnew System::EventHandler(this, &frmConfig::fcgChangeEnabled);
            // 
            // fcgPNVppDeband
            // 
            this->fcgPNVppDeband->Controls->Add(this->fcgCBVppDebandRandEachFrame);
            this->fcgPNVppDeband->Controls->Add(this->fcgCBVppDebandBlurFirst);
            this->fcgPNVppDeband->Controls->Add(this->fcgLBVppDebandSample);
            this->fcgPNVppDeband->Controls->Add(this->fcgLBVppDebandDitherC);
            this->fcgPNVppDeband->Controls->Add(this->fcgLBVppDebandDitherY);
            this->fcgPNVppDeband->Controls->Add(this->fcgLBVppDebandDither);
            this->fcgPNVppDeband->Controls->Add(this->fcgLBVppDebandThreCr);
            this->fcgPNVppDeband->Controls->Add(this->fcgLBVppDebandThreCb);
            this->fcgPNVppDeband->Controls->Add(this->fcgLBVppDebandThreY);
            this->fcgPNVppDeband->Controls->Add(this->fcgNUVppDebandDitherC);
            this->fcgPNVppDeband->Controls->Add(this->fcgNUVppDebandDitherY);
            this->fcgPNVppDeband->Controls->Add(this->fcgNUVppDebandThreCr);
            this->fcgPNVppDeband->Controls->Add(this->fcgNUVppDebandThreCb);
            this->fcgPNVppDeband->Controls->Add(this->fcgLBVppDebandThreshold);
            this->fcgPNVppDeband->Controls->Add(this->fcgLBVppDebandRange);
            this->fcgPNVppDeband->Controls->Add(this->fcgCXVppDebandSample);
            this->fcgPNVppDeband->Controls->Add(this->fcgNUVppDebandThreY);
            this->fcgPNVppDeband->Controls->Add(this->fcgNUVppDebandRange);
            this->fcgPNVppDeband->Location = System::Drawing::Point(7, 59);
            this->fcgPNVppDeband->Margin = System::Windows::Forms::Padding(4);
            this->fcgPNVppDeband->Name = L"fcgPNVppDeband";
            this->fcgPNVppDeband->Size = System::Drawing::Size(466, 174);
            this->fcgPNVppDeband->TabIndex = 20;
            // 
            // fcgCBVppDebandRandEachFrame
            // 
            this->fcgCBVppDebandRandEachFrame->AutoSize = true;
            this->fcgCBVppDebandRandEachFrame->Location = System::Drawing::Point(230, 145);
            this->fcgCBVppDebandRandEachFrame->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBVppDebandRandEachFrame->Name = L"fcgCBVppDebandRandEachFrame";
            this->fcgCBVppDebandRandEachFrame->Size = System::Drawing::Size(186, 26);
            this->fcgCBVppDebandRandEachFrame->TabIndex = 35;
            this->fcgCBVppDebandRandEachFrame->Tag = L"reCmd";
            this->fcgCBVppDebandRandEachFrame->Text = L"毎フレーム乱数を生成";
            this->fcgCBVppDebandRandEachFrame->UseVisualStyleBackColor = true;
            // 
            // fcgCBVppDebandBlurFirst
            // 
            this->fcgCBVppDebandBlurFirst->AutoSize = true;
            this->fcgCBVppDebandBlurFirst->Location = System::Drawing::Point(7, 145);
            this->fcgCBVppDebandBlurFirst->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBVppDebandBlurFirst->Name = L"fcgCBVppDebandBlurFirst";
            this->fcgCBVppDebandBlurFirst->Size = System::Drawing::Size(152, 26);
            this->fcgCBVppDebandBlurFirst->TabIndex = 34;
            this->fcgCBVppDebandBlurFirst->Tag = L"reCmd";
            this->fcgCBVppDebandBlurFirst->Text = L"ブラー処理を先に";
            this->fcgCBVppDebandBlurFirst->UseVisualStyleBackColor = true;
            // 
            // fcgLBVppDebandSample
            // 
            this->fcgLBVppDebandSample->AutoSize = true;
            this->fcgLBVppDebandSample->Location = System::Drawing::Point(2, 112);
            this->fcgLBVppDebandSample->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDebandSample->Name = L"fcgLBVppDebandSample";
            this->fcgLBVppDebandSample->Size = System::Drawing::Size(68, 22);
            this->fcgLBVppDebandSample->TabIndex = 32;
            this->fcgLBVppDebandSample->Text = L"sample";
            // 
            // fcgLBVppDebandDitherC
            // 
            this->fcgLBVppDebandDitherC->AutoSize = true;
            this->fcgLBVppDebandDitherC->Location = System::Drawing::Point(236, 76);
            this->fcgLBVppDebandDitherC->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDebandDitherC->Name = L"fcgLBVppDebandDitherC";
            this->fcgLBVppDebandDitherC->Size = System::Drawing::Size(21, 22);
            this->fcgLBVppDebandDitherC->TabIndex = 30;
            this->fcgLBVppDebandDitherC->Text = L"C";
            // 
            // fcgLBVppDebandDitherY
            // 
            this->fcgLBVppDebandDitherY->AutoSize = true;
            this->fcgLBVppDebandDitherY->Location = System::Drawing::Point(118, 76);
            this->fcgLBVppDebandDitherY->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDebandDitherY->Name = L"fcgLBVppDebandDitherY";
            this->fcgLBVppDebandDitherY->Size = System::Drawing::Size(21, 22);
            this->fcgLBVppDebandDitherY->TabIndex = 28;
            this->fcgLBVppDebandDitherY->Text = L"Y";
            // 
            // fcgLBVppDebandDither
            // 
            this->fcgLBVppDebandDither->AutoSize = true;
            this->fcgLBVppDebandDither->Location = System::Drawing::Point(2, 76);
            this->fcgLBVppDebandDither->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDebandDither->Name = L"fcgLBVppDebandDither";
            this->fcgLBVppDebandDither->Size = System::Drawing::Size(58, 22);
            this->fcgLBVppDebandDither->TabIndex = 27;
            this->fcgLBVppDebandDither->Text = L"dither";
            // 
            // fcgLBVppDebandThreCr
            // 
            this->fcgLBVppDebandThreCr->AutoSize = true;
            this->fcgLBVppDebandThreCr->Location = System::Drawing::Point(350, 42);
            this->fcgLBVppDebandThreCr->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDebandThreCr->Name = L"fcgLBVppDebandThreCr";
            this->fcgLBVppDebandThreCr->Size = System::Drawing::Size(28, 22);
            this->fcgLBVppDebandThreCr->TabIndex = 25;
            this->fcgLBVppDebandThreCr->Text = L"Cr";
            // 
            // fcgLBVppDebandThreCb
            // 
            this->fcgLBVppDebandThreCb->AutoSize = true;
            this->fcgLBVppDebandThreCb->Location = System::Drawing::Point(229, 42);
            this->fcgLBVppDebandThreCb->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDebandThreCb->Name = L"fcgLBVppDebandThreCb";
            this->fcgLBVppDebandThreCb->Size = System::Drawing::Size(31, 22);
            this->fcgLBVppDebandThreCb->TabIndex = 23;
            this->fcgLBVppDebandThreCb->Text = L"Cb";
            // 
            // fcgLBVppDebandThreY
            // 
            this->fcgLBVppDebandThreY->AutoSize = true;
            this->fcgLBVppDebandThreY->Location = System::Drawing::Point(118, 41);
            this->fcgLBVppDebandThreY->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDebandThreY->Name = L"fcgLBVppDebandThreY";
            this->fcgLBVppDebandThreY->Size = System::Drawing::Size(21, 22);
            this->fcgLBVppDebandThreY->TabIndex = 21;
            this->fcgLBVppDebandThreY->Text = L"Y";
            // 
            // fcgNUVppDebandDitherC
            // 
            this->fcgNUVppDebandDitherC->Location = System::Drawing::Point(264, 73);
            this->fcgNUVppDebandDitherC->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDebandDitherC->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 31, 0, 0, 0 });
            this->fcgNUVppDebandDitherC->Name = L"fcgNUVppDebandDitherC";
            this->fcgNUVppDebandDitherC->Size = System::Drawing::Size(74, 28);
            this->fcgNUVppDebandDitherC->TabIndex = 31;
            this->fcgNUVppDebandDitherC->Tag = L"reCmd";
            this->fcgNUVppDebandDitherC->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUVppDebandDitherY
            // 
            this->fcgNUVppDebandDitherY->Location = System::Drawing::Point(145, 73);
            this->fcgNUVppDebandDitherY->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDebandDitherY->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 31, 0, 0, 0 });
            this->fcgNUVppDebandDitherY->Name = L"fcgNUVppDebandDitherY";
            this->fcgNUVppDebandDitherY->Size = System::Drawing::Size(74, 28);
            this->fcgNUVppDebandDitherY->TabIndex = 29;
            this->fcgNUVppDebandDitherY->Tag = L"reCmd";
            this->fcgNUVppDebandDitherY->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUVppDebandThreCr
            // 
            this->fcgNUVppDebandThreCr->Location = System::Drawing::Point(382, 38);
            this->fcgNUVppDebandThreCr->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDebandThreCr->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 31, 0, 0, 0 });
            this->fcgNUVppDebandThreCr->Name = L"fcgNUVppDebandThreCr";
            this->fcgNUVppDebandThreCr->Size = System::Drawing::Size(74, 28);
            this->fcgNUVppDebandThreCr->TabIndex = 26;
            this->fcgNUVppDebandThreCr->Tag = L"reCmd";
            this->fcgNUVppDebandThreCr->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUVppDebandThreCb
            // 
            this->fcgNUVppDebandThreCb->Location = System::Drawing::Point(264, 38);
            this->fcgNUVppDebandThreCb->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDebandThreCb->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 31, 0, 0, 0 });
            this->fcgNUVppDebandThreCb->Name = L"fcgNUVppDebandThreCb";
            this->fcgNUVppDebandThreCb->Size = System::Drawing::Size(74, 28);
            this->fcgNUVppDebandThreCb->TabIndex = 24;
            this->fcgNUVppDebandThreCb->Tag = L"reCmd";
            this->fcgNUVppDebandThreCb->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBVppDebandThreshold
            // 
            this->fcgLBVppDebandThreshold->AutoSize = true;
            this->fcgLBVppDebandThreshold->Location = System::Drawing::Point(2, 41);
            this->fcgLBVppDebandThreshold->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDebandThreshold->Name = L"fcgLBVppDebandThreshold";
            this->fcgLBVppDebandThreshold->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppDebandThreshold->TabIndex = 20;
            this->fcgLBVppDebandThreshold->Text = L"閾値";
            // 
            // fcgLBVppDebandRange
            // 
            this->fcgLBVppDebandRange->AutoSize = true;
            this->fcgLBVppDebandRange->Location = System::Drawing::Point(2, 7);
            this->fcgLBVppDebandRange->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDebandRange->Name = L"fcgLBVppDebandRange";
            this->fcgLBVppDebandRange->Size = System::Drawing::Size(58, 22);
            this->fcgLBVppDebandRange->TabIndex = 18;
            this->fcgLBVppDebandRange->Text = L"range";
            // 
            // fcgCXVppDebandSample
            // 
            this->fcgCXVppDebandSample->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppDebandSample->FormattingEnabled = true;
            this->fcgCXVppDebandSample->Location = System::Drawing::Point(145, 108);
            this->fcgCXVppDebandSample->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppDebandSample->Name = L"fcgCXVppDebandSample";
            this->fcgCXVppDebandSample->Size = System::Drawing::Size(232, 30);
            this->fcgCXVppDebandSample->TabIndex = 33;
            this->fcgCXVppDebandSample->Tag = L"reCmd";
            // 
            // fcgNUVppDebandThreY
            // 
            this->fcgNUVppDebandThreY->Location = System::Drawing::Point(145, 38);
            this->fcgNUVppDebandThreY->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDebandThreY->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 31, 0, 0, 0 });
            this->fcgNUVppDebandThreY->Name = L"fcgNUVppDebandThreY";
            this->fcgNUVppDebandThreY->Size = System::Drawing::Size(74, 28);
            this->fcgNUVppDebandThreY->TabIndex = 22;
            this->fcgNUVppDebandThreY->Tag = L"reCmd";
            this->fcgNUVppDebandThreY->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUVppDebandRange
            // 
            this->fcgNUVppDebandRange->Location = System::Drawing::Point(145, 4);
            this->fcgNUVppDebandRange->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDebandRange->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 127, 0, 0, 0 });
            this->fcgNUVppDebandRange->Name = L"fcgNUVppDebandRange";
            this->fcgNUVppDebandRange->Size = System::Drawing::Size(74, 28);
            this->fcgNUVppDebandRange->TabIndex = 19;
            this->fcgNUVppDebandRange->Tag = L"reCmd";
            this->fcgNUVppDebandRange->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcggroupBoxVppDetailEnahance
            // 
            this->fcggroupBoxVppDetailEnahance->Controls->Add(this->fcgCXVppDetailEnhance);
            this->fcggroupBoxVppDetailEnahance->Controls->Add(this->fcgPNVppWarpsharp);
            this->fcggroupBoxVppDetailEnahance->Controls->Add(this->fcgPNVppDetailEnhance);
            this->fcggroupBoxVppDetailEnahance->Controls->Add(this->fcgPNVppEdgelevel);
            this->fcggroupBoxVppDetailEnahance->Controls->Add(this->fcgPNVppUnsharp);
            this->fcggroupBoxVppDetailEnahance->Location = System::Drawing::Point(6, 310);
            this->fcggroupBoxVppDetailEnahance->Margin = System::Windows::Forms::Padding(5);
            this->fcggroupBoxVppDetailEnahance->Name = L"fcggroupBoxVppDetailEnahance";
            this->fcggroupBoxVppDetailEnahance->Padding = System::Windows::Forms::Padding(5);
            this->fcggroupBoxVppDetailEnahance->Size = System::Drawing::Size(480, 166);
            this->fcggroupBoxVppDetailEnahance->TabIndex = 22;
            this->fcggroupBoxVppDetailEnahance->TabStop = false;
            this->fcggroupBoxVppDetailEnahance->Text = L"ディテール・輪郭強調";
            // 
            // fcgCXVppDetailEnhance
            // 
            this->fcgCXVppDetailEnhance->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppDetailEnhance->FormattingEnabled = true;
            this->fcgCXVppDetailEnhance->Location = System::Drawing::Point(38, 30);
            this->fcgCXVppDetailEnhance->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppDetailEnhance->Name = L"fcgCXVppDetailEnhance";
            this->fcgCXVppDetailEnhance->Size = System::Drawing::Size(285, 30);
            this->fcgCXVppDetailEnhance->TabIndex = 0;
            this->fcgCXVppDetailEnhance->Tag = L"reCmd";
            this->fcgCXVppDetailEnhance->SelectionChangeCommitted += gcnew System::EventHandler(this, &frmConfig::fcgChangeEnabled);
            // 
            // fcgPNVppWarpsharp
            // 
            this->fcgPNVppWarpsharp->Controls->Add(this->fcgLBVppWarpsharpDepth);
            this->fcgPNVppWarpsharp->Controls->Add(this->fcgNUVppWarpsharpDepth);
            this->fcgPNVppWarpsharp->Controls->Add(this->fcgLBVppWarpsharpThreshold);
            this->fcgPNVppWarpsharp->Controls->Add(this->fcgLBVppWarpsharpType);
            this->fcgPNVppWarpsharp->Controls->Add(this->fcgLBVppWarpsharpBlur);
            this->fcgPNVppWarpsharp->Controls->Add(this->fcgNUVppWarpsharpThreshold);
            this->fcgPNVppWarpsharp->Controls->Add(this->fcgNUVppWarpsharpType);
            this->fcgPNVppWarpsharp->Controls->Add(this->fcgNUVppWarpsharpBlur);
            this->fcgPNVppWarpsharp->Location = System::Drawing::Point(5, 65);
            this->fcgPNVppWarpsharp->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNVppWarpsharp->Name = L"fcgPNVppWarpsharp";
            this->fcgPNVppWarpsharp->Size = System::Drawing::Size(467, 91);
            this->fcgPNVppWarpsharp->TabIndex = 66;
            // 
            // fcgLBVppWarpsharpDepth
            // 
            this->fcgLBVppWarpsharpDepth->AutoSize = true;
            this->fcgLBVppWarpsharpDepth->Location = System::Drawing::Point(258, 55);
            this->fcgLBVppWarpsharpDepth->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppWarpsharpDepth->Name = L"fcgLBVppWarpsharpDepth";
            this->fcgLBVppWarpsharpDepth->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppWarpsharpDepth->TabIndex = 7;
            this->fcgLBVppWarpsharpDepth->Text = L"深度";
            // 
            // fcgNUVppWarpsharpDepth
            // 
            this->fcgNUVppWarpsharpDepth->Location = System::Drawing::Point(362, 50);
            this->fcgNUVppWarpsharpDepth->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppWarpsharpDepth->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 128, 0, 0, 0 });
            this->fcgNUVppWarpsharpDepth->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 128, 0, 0, System::Int32::MinValue });
            this->fcgNUVppWarpsharpDepth->Name = L"fcgNUVppWarpsharpDepth";
            this->fcgNUVppWarpsharpDepth->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppWarpsharpDepth->TabIndex = 8;
            this->fcgNUVppWarpsharpDepth->Tag = L"reCmd";
            this->fcgNUVppWarpsharpDepth->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppWarpsharpDepth->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16, 0, 0, 0 });
            // 
            // fcgLBVppWarpsharpThreshold
            // 
            this->fcgLBVppWarpsharpThreshold->AutoSize = true;
            this->fcgLBVppWarpsharpThreshold->Location = System::Drawing::Point(258, 13);
            this->fcgLBVppWarpsharpThreshold->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppWarpsharpThreshold->Name = L"fcgLBVppWarpsharpThreshold";
            this->fcgLBVppWarpsharpThreshold->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppWarpsharpThreshold->TabIndex = 3;
            this->fcgLBVppWarpsharpThreshold->Text = L"閾値";
            // 
            // fcgLBVppWarpsharpType
            // 
            this->fcgLBVppWarpsharpType->AutoSize = true;
            this->fcgLBVppWarpsharpType->Location = System::Drawing::Point(12, 55);
            this->fcgLBVppWarpsharpType->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppWarpsharpType->Name = L"fcgLBVppWarpsharpType";
            this->fcgLBVppWarpsharpType->Size = System::Drawing::Size(46, 22);
            this->fcgLBVppWarpsharpType->TabIndex = 5;
            this->fcgLBVppWarpsharpType->Text = L"type";
            // 
            // fcgLBVppWarpsharpBlur
            // 
            this->fcgLBVppWarpsharpBlur->AutoSize = true;
            this->fcgLBVppWarpsharpBlur->Location = System::Drawing::Point(12, 14);
            this->fcgLBVppWarpsharpBlur->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppWarpsharpBlur->Name = L"fcgLBVppWarpsharpBlur";
            this->fcgLBVppWarpsharpBlur->Size = System::Drawing::Size(42, 22);
            this->fcgLBVppWarpsharpBlur->TabIndex = 1;
            this->fcgLBVppWarpsharpBlur->Text = L"blur";
            // 
            // fcgNUVppWarpsharpThreshold
            // 
            this->fcgNUVppWarpsharpThreshold->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 0 });
            this->fcgNUVppWarpsharpThreshold->Location = System::Drawing::Point(362, 11);
            this->fcgNUVppWarpsharpThreshold->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppWarpsharpThreshold->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
            this->fcgNUVppWarpsharpThreshold->Name = L"fcgNUVppWarpsharpThreshold";
            this->fcgNUVppWarpsharpThreshold->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppWarpsharpThreshold->TabIndex = 4;
            this->fcgNUVppWarpsharpThreshold->Tag = L"reCmd";
            this->fcgNUVppWarpsharpThreshold->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppWarpsharpThreshold->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            // 
            // fcgNUVppWarpsharpType
            // 
            this->fcgNUVppWarpsharpType->Location = System::Drawing::Point(118, 50);
            this->fcgNUVppWarpsharpType->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppWarpsharpType->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUVppWarpsharpType->Name = L"fcgNUVppWarpsharpType";
            this->fcgNUVppWarpsharpType->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppWarpsharpType->TabIndex = 6;
            this->fcgNUVppWarpsharpType->Tag = L"reCmd";
            this->fcgNUVppWarpsharpType->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUVppWarpsharpBlur
            // 
            this->fcgNUVppWarpsharpBlur->Location = System::Drawing::Point(118, 11);
            this->fcgNUVppWarpsharpBlur->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppWarpsharpBlur->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16, 0, 0, 0 });
            this->fcgNUVppWarpsharpBlur->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUVppWarpsharpBlur->Name = L"fcgNUVppWarpsharpBlur";
            this->fcgNUVppWarpsharpBlur->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppWarpsharpBlur->TabIndex = 2;
            this->fcgNUVppWarpsharpBlur->Tag = L"reCmd";
            this->fcgNUVppWarpsharpBlur->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppWarpsharpBlur->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2, 0, 0, 0 });
            // 
            // fcgPNVppDetailEnhance
            // 
            this->fcgPNVppDetailEnhance->Controls->Add(this->fcgLBVppDetailEnhanceRadius);
            this->fcgPNVppDetailEnhance->Controls->Add(this->fcgLBVppDetailEnhanceAttenuation);
            this->fcgPNVppDetailEnhance->Controls->Add(this->fcgNUVppDetailEnhanceRadius);
            this->fcgPNVppDetailEnhance->Controls->Add(this->fcgNUVppDetailEnhanceAttenuation);
            this->fcgPNVppDetailEnhance->Location = System::Drawing::Point(5, 65);
            this->fcgPNVppDetailEnhance->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNVppDetailEnhance->Name = L"fcgPNVppDetailEnhance";
            this->fcgPNVppDetailEnhance->Size = System::Drawing::Size(467, 91);
            this->fcgPNVppDetailEnhance->TabIndex = 67;
            // 
            // fcgLBVppDetailEnhanceRadius
            // 
            this->fcgLBVppDetailEnhanceRadius->AutoSize = true;
            this->fcgLBVppDetailEnhanceRadius->Location = System::Drawing::Point(258, 13);
            this->fcgLBVppDetailEnhanceRadius->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDetailEnhanceRadius->Name = L"fcgLBVppDetailEnhanceRadius";
            this->fcgLBVppDetailEnhanceRadius->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppDetailEnhanceRadius->TabIndex = 3;
            this->fcgLBVppDetailEnhanceRadius->Text = L"半径";
            // 
            // fcgLBVppDetailEnhanceAttenuation
            // 
            this->fcgLBVppDetailEnhanceAttenuation->AutoSize = true;
            this->fcgLBVppDetailEnhanceAttenuation->Location = System::Drawing::Point(12, 14);
            this->fcgLBVppDetailEnhanceAttenuation->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDetailEnhanceAttenuation->Name = L"fcgLBVppDetailEnhanceAttenuation";
            this->fcgLBVppDetailEnhanceAttenuation->Size = System::Drawing::Size(61, 22);
            this->fcgLBVppDetailEnhanceAttenuation->TabIndex = 1;
            this->fcgLBVppDetailEnhanceAttenuation->Text = L"減衰度";
            // 
            // fcgNUVppDetailEnhanceRadius
            // 
            this->fcgNUVppDetailEnhanceRadius->Location = System::Drawing::Point(362, 11);
            this->fcgNUVppDetailEnhanceRadius->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDetailEnhanceRadius->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4, 0, 0, 0 });
            this->fcgNUVppDetailEnhanceRadius->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUVppDetailEnhanceRadius->Name = L"fcgNUVppDetailEnhanceRadius";
            this->fcgNUVppDetailEnhanceRadius->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppDetailEnhanceRadius->TabIndex = 4;
            this->fcgNUVppDetailEnhanceRadius->Tag = L"reCmd";
            this->fcgNUVppDetailEnhanceRadius->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDetailEnhanceRadius->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2, 0, 0, 0 });
            // 
            // fcgNUVppDetailEnhanceAttenuation
            // 
            this->fcgNUVppDetailEnhanceAttenuation->DecimalPlaces = 2;
            this->fcgNUVppDetailEnhanceAttenuation->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 131072 });
            this->fcgNUVppDetailEnhanceAttenuation->Location = System::Drawing::Point(118, 11);
            this->fcgNUVppDetailEnhanceAttenuation->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDetailEnhanceAttenuation->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4, 0, 0, 65536 });
            this->fcgNUVppDetailEnhanceAttenuation->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 131072 });
            this->fcgNUVppDetailEnhanceAttenuation->Name = L"fcgNUVppDetailEnhanceAttenuation";
            this->fcgNUVppDetailEnhanceAttenuation->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppDetailEnhanceAttenuation->TabIndex = 2;
            this->fcgNUVppDetailEnhanceAttenuation->Tag = L"reCmd";
            this->fcgNUVppDetailEnhanceAttenuation->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDetailEnhanceAttenuation->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 65536 });
            // 
            // fcgPNVppEdgelevel
            // 
            this->fcgPNVppEdgelevel->Controls->Add(this->fcgLBVppEdgelevelWhite);
            this->fcgPNVppEdgelevel->Controls->Add(this->fcgNUVppEdgelevelWhite);
            this->fcgPNVppEdgelevel->Controls->Add(this->fcgLBVppEdgelevelThreshold);
            this->fcgPNVppEdgelevel->Controls->Add(this->fcgLBVppEdgelevelBlack);
            this->fcgPNVppEdgelevel->Controls->Add(this->fcgLBVppEdgelevelStrength);
            this->fcgPNVppEdgelevel->Controls->Add(this->fcgNUVppEdgelevelThreshold);
            this->fcgPNVppEdgelevel->Controls->Add(this->fcgNUVppEdgelevelBlack);
            this->fcgPNVppEdgelevel->Controls->Add(this->fcgNUVppEdgelevelStrength);
            this->fcgPNVppEdgelevel->Location = System::Drawing::Point(5, 65);
            this->fcgPNVppEdgelevel->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNVppEdgelevel->Name = L"fcgPNVppEdgelevel";
            this->fcgPNVppEdgelevel->Size = System::Drawing::Size(467, 91);
            this->fcgPNVppEdgelevel->TabIndex = 1;
            // 
            // fcgLBVppEdgelevelWhite
            // 
            this->fcgLBVppEdgelevelWhite->AutoSize = true;
            this->fcgLBVppEdgelevelWhite->Location = System::Drawing::Point(258, 55);
            this->fcgLBVppEdgelevelWhite->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppEdgelevelWhite->Name = L"fcgLBVppEdgelevelWhite";
            this->fcgLBVppEdgelevelWhite->Size = System::Drawing::Size(27, 22);
            this->fcgLBVppEdgelevelWhite->TabIndex = 7;
            this->fcgLBVppEdgelevelWhite->Text = L"白";
            // 
            // fcgNUVppEdgelevelWhite
            // 
            this->fcgNUVppEdgelevelWhite->DecimalPlaces = 1;
            this->fcgNUVppEdgelevelWhite->Location = System::Drawing::Point(362, 50);
            this->fcgNUVppEdgelevelWhite->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppEdgelevelWhite->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 31, 0, 0, 0 });
            this->fcgNUVppEdgelevelWhite->Name = L"fcgNUVppEdgelevelWhite";
            this->fcgNUVppEdgelevelWhite->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppEdgelevelWhite->TabIndex = 8;
            this->fcgNUVppEdgelevelWhite->Tag = L"reCmd";
            this->fcgNUVppEdgelevelWhite->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppEdgelevelWhite->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            // 
            // fcgLBVppEdgelevelThreshold
            // 
            this->fcgLBVppEdgelevelThreshold->AutoSize = true;
            this->fcgLBVppEdgelevelThreshold->Location = System::Drawing::Point(258, 13);
            this->fcgLBVppEdgelevelThreshold->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppEdgelevelThreshold->Name = L"fcgLBVppEdgelevelThreshold";
            this->fcgLBVppEdgelevelThreshold->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppEdgelevelThreshold->TabIndex = 3;
            this->fcgLBVppEdgelevelThreshold->Text = L"閾値";
            // 
            // fcgLBVppEdgelevelBlack
            // 
            this->fcgLBVppEdgelevelBlack->AutoSize = true;
            this->fcgLBVppEdgelevelBlack->Location = System::Drawing::Point(12, 55);
            this->fcgLBVppEdgelevelBlack->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppEdgelevelBlack->Name = L"fcgLBVppEdgelevelBlack";
            this->fcgLBVppEdgelevelBlack->Size = System::Drawing::Size(27, 22);
            this->fcgLBVppEdgelevelBlack->TabIndex = 5;
            this->fcgLBVppEdgelevelBlack->Text = L"黒";
            // 
            // fcgLBVppEdgelevelStrength
            // 
            this->fcgLBVppEdgelevelStrength->AutoSize = true;
            this->fcgLBVppEdgelevelStrength->Location = System::Drawing::Point(12, 14);
            this->fcgLBVppEdgelevelStrength->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppEdgelevelStrength->Name = L"fcgLBVppEdgelevelStrength";
            this->fcgLBVppEdgelevelStrength->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppEdgelevelStrength->TabIndex = 1;
            this->fcgLBVppEdgelevelStrength->Text = L"特性";
            // 
            // fcgNUVppEdgelevelThreshold
            // 
            this->fcgNUVppEdgelevelThreshold->Location = System::Drawing::Point(362, 11);
            this->fcgNUVppEdgelevelThreshold->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppEdgelevelThreshold->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
            this->fcgNUVppEdgelevelThreshold->Name = L"fcgNUVppEdgelevelThreshold";
            this->fcgNUVppEdgelevelThreshold->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppEdgelevelThreshold->TabIndex = 4;
            this->fcgNUVppEdgelevelThreshold->Tag = L"reCmd";
            this->fcgNUVppEdgelevelThreshold->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppEdgelevelThreshold->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            // 
            // fcgNUVppEdgelevelBlack
            // 
            this->fcgNUVppEdgelevelBlack->DecimalPlaces = 1;
            this->fcgNUVppEdgelevelBlack->Location = System::Drawing::Point(118, 50);
            this->fcgNUVppEdgelevelBlack->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppEdgelevelBlack->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 31, 0, 0, 0 });
            this->fcgNUVppEdgelevelBlack->Name = L"fcgNUVppEdgelevelBlack";
            this->fcgNUVppEdgelevelBlack->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppEdgelevelBlack->TabIndex = 6;
            this->fcgNUVppEdgelevelBlack->Tag = L"reCmd";
            this->fcgNUVppEdgelevelBlack->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppEdgelevelBlack->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            // 
            // fcgNUVppEdgelevelStrength
            // 
            this->fcgNUVppEdgelevelStrength->Location = System::Drawing::Point(118, 11);
            this->fcgNUVppEdgelevelStrength->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppEdgelevelStrength->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 31, 0, 0, 0 });
            this->fcgNUVppEdgelevelStrength->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 31, 0, 0, System::Int32::MinValue });
            this->fcgNUVppEdgelevelStrength->Name = L"fcgNUVppEdgelevelStrength";
            this->fcgNUVppEdgelevelStrength->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppEdgelevelStrength->TabIndex = 2;
            this->fcgNUVppEdgelevelStrength->Tag = L"reCmd";
            this->fcgNUVppEdgelevelStrength->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppEdgelevelStrength->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
            // 
            // fcgPNVppUnsharp
            // 
            this->fcgPNVppUnsharp->Controls->Add(this->fcgLBVppUnsharpThreshold);
            this->fcgPNVppUnsharp->Controls->Add(this->fcgLBVppUnsharpWeight);
            this->fcgPNVppUnsharp->Controls->Add(this->fcgLBVppUnsharpRadius);
            this->fcgPNVppUnsharp->Controls->Add(this->fcgNUVppUnsharpThreshold);
            this->fcgPNVppUnsharp->Controls->Add(this->fcgNUVppUnsharpWeight);
            this->fcgPNVppUnsharp->Controls->Add(this->fcgNUVppUnsharpRadius);
            this->fcgPNVppUnsharp->Location = System::Drawing::Point(5, 65);
            this->fcgPNVppUnsharp->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNVppUnsharp->Name = L"fcgPNVppUnsharp";
            this->fcgPNVppUnsharp->Size = System::Drawing::Size(466, 91);
            this->fcgPNVppUnsharp->TabIndex = 65;
            // 
            // fcgLBVppUnsharpThreshold
            // 
            this->fcgLBVppUnsharpThreshold->AutoSize = true;
            this->fcgLBVppUnsharpThreshold->Location = System::Drawing::Point(258, 13);
            this->fcgLBVppUnsharpThreshold->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppUnsharpThreshold->Name = L"fcgLBVppUnsharpThreshold";
            this->fcgLBVppUnsharpThreshold->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppUnsharpThreshold->TabIndex = 11;
            this->fcgLBVppUnsharpThreshold->Text = L"閾値";
            // 
            // fcgLBVppUnsharpWeight
            // 
            this->fcgLBVppUnsharpWeight->AutoSize = true;
            this->fcgLBVppUnsharpWeight->Location = System::Drawing::Point(14, 55);
            this->fcgLBVppUnsharpWeight->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppUnsharpWeight->Name = L"fcgLBVppUnsharpWeight";
            this->fcgLBVppUnsharpWeight->Size = System::Drawing::Size(39, 22);
            this->fcgLBVppUnsharpWeight->TabIndex = 10;
            this->fcgLBVppUnsharpWeight->Text = L"強さ";
            // 
            // fcgLBVppUnsharpRadius
            // 
            this->fcgLBVppUnsharpRadius->AutoSize = true;
            this->fcgLBVppUnsharpRadius->Location = System::Drawing::Point(14, 14);
            this->fcgLBVppUnsharpRadius->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppUnsharpRadius->Name = L"fcgLBVppUnsharpRadius";
            this->fcgLBVppUnsharpRadius->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppUnsharpRadius->TabIndex = 9;
            this->fcgLBVppUnsharpRadius->Text = L"半径";
            // 
            // fcgNUVppUnsharpThreshold
            // 
            this->fcgNUVppUnsharpThreshold->Location = System::Drawing::Point(362, 11);
            this->fcgNUVppUnsharpThreshold->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppUnsharpThreshold->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
            this->fcgNUVppUnsharpThreshold->Name = L"fcgNUVppUnsharpThreshold";
            this->fcgNUVppUnsharpThreshold->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppUnsharpThreshold->TabIndex = 8;
            this->fcgNUVppUnsharpThreshold->Tag = L"reCmd";
            this->fcgNUVppUnsharpThreshold->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppUnsharpThreshold->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            // 
            // fcgNUVppUnsharpWeight
            // 
            this->fcgNUVppUnsharpWeight->DecimalPlaces = 1;
            this->fcgNUVppUnsharpWeight->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 65536 });
            this->fcgNUVppUnsharpWeight->Location = System::Drawing::Point(118, 50);
            this->fcgNUVppUnsharpWeight->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppUnsharpWeight->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
            this->fcgNUVppUnsharpWeight->Name = L"fcgNUVppUnsharpWeight";
            this->fcgNUVppUnsharpWeight->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppUnsharpWeight->TabIndex = 7;
            this->fcgNUVppUnsharpWeight->Tag = L"reCmd";
            this->fcgNUVppUnsharpWeight->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppUnsharpWeight->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            // 
            // fcgNUVppUnsharpRadius
            // 
            this->fcgNUVppUnsharpRadius->Location = System::Drawing::Point(118, 11);
            this->fcgNUVppUnsharpRadius->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppUnsharpRadius->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 9, 0, 0, 0 });
            this->fcgNUVppUnsharpRadius->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUVppUnsharpRadius->Name = L"fcgNUVppUnsharpRadius";
            this->fcgNUVppUnsharpRadius->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppUnsharpRadius->TabIndex = 6;
            this->fcgNUVppUnsharpRadius->Tag = L"reCmd";
            this->fcgNUVppUnsharpRadius->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppUnsharpRadius->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
            // 
            // fcggroupBoxVppDenoise
            // 
            this->fcggroupBoxVppDenoise->Controls->Add(this->fcgPNVppDenoiseFFT3D);
            this->fcggroupBoxVppDenoise->Controls->Add(this->fcgPNVppDenoiseNLMeans);
            this->fcggroupBoxVppDenoise->Controls->Add(this->fcgPNVppDenoiseDct);
            this->fcggroupBoxVppDenoise->Controls->Add(this->fcgPNVppPreProcess);
            this->fcggroupBoxVppDenoise->Controls->Add(this->fcgCXVppDenoiseMethod);
            this->fcggroupBoxVppDenoise->Controls->Add(this->fcgPNVppDenoisePmd);
            this->fcggroupBoxVppDenoise->Controls->Add(this->fcgPNVppDenoiseConv3D);
            this->fcggroupBoxVppDenoise->Controls->Add(this->fcgPNVppDenoiseSmooth);
            this->fcggroupBoxVppDenoise->Controls->Add(this->fcgPNVppDenoiseKnn);
            this->fcggroupBoxVppDenoise->Location = System::Drawing::Point(6, 96);
            this->fcggroupBoxVppDenoise->Margin = System::Windows::Forms::Padding(5);
            this->fcggroupBoxVppDenoise->Name = L"fcggroupBoxVppDenoise";
            this->fcggroupBoxVppDenoise->Padding = System::Windows::Forms::Padding(5);
            this->fcggroupBoxVppDenoise->Size = System::Drawing::Size(480, 209);
            this->fcggroupBoxVppDenoise->TabIndex = 21;
            this->fcggroupBoxVppDenoise->TabStop = false;
            this->fcggroupBoxVppDenoise->Text = L"ノイズ除去";
            // 
            // fcgPNVppDenoiseFFT3D
            // 
            this->fcgPNVppDenoiseFFT3D->Controls->Add(this->fcgLBVppDenoiseFFT3DTemporal);
            this->fcgPNVppDenoiseFFT3D->Controls->Add(this->fcgLBVppDenoiseFFT3DPrecision);
            this->fcgPNVppDenoiseFFT3D->Controls->Add(this->fcgCXVppDenoiseFFT3DPrecision);
            this->fcgPNVppDenoiseFFT3D->Controls->Add(this->fcgNUVppDenoiseFFT3DOverlap);
            this->fcgPNVppDenoiseFFT3D->Controls->Add(this->fcgNUVppDenoiseFFT3DAmount);
            this->fcgPNVppDenoiseFFT3D->Controls->Add(this->fcgLBVppDenoiseFFT3DOverlap);
            this->fcgPNVppDenoiseFFT3D->Controls->Add(this->fcgNUVppDenoiseFFT3DSigma);
            this->fcgPNVppDenoiseFFT3D->Controls->Add(this->fcgCXVppDenoiseFFT3DTemporal);
            this->fcgPNVppDenoiseFFT3D->Controls->Add(this->fcgLBVppDenoiseFFT3DAmount);
            this->fcgPNVppDenoiseFFT3D->Controls->Add(this->fcgLBVppDenoiseFFT3DBlockSize);
            this->fcgPNVppDenoiseFFT3D->Controls->Add(this->fcgCXVppDenoiseFFT3DBlockSize);
            this->fcgPNVppDenoiseFFT3D->Controls->Add(this->fcgLBVppDenoiseFFT3DSigma);
            this->fcgPNVppDenoiseFFT3D->Location = System::Drawing::Point(5, 62);
            this->fcgPNVppDenoiseFFT3D->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNVppDenoiseFFT3D->Name = L"fcgPNVppDenoiseFFT3D";
            this->fcgPNVppDenoiseFFT3D->Size = System::Drawing::Size(466, 138);
            this->fcgPNVppDenoiseFFT3D->TabIndex = 73;
            // 
            // fcgLBVppDenoiseFFT3DTemporal
            // 
            this->fcgLBVppDenoiseFFT3DTemporal->AutoSize = true;
            this->fcgLBVppDenoiseFFT3DTemporal->Location = System::Drawing::Point(11, 102);
            this->fcgLBVppDenoiseFFT3DTemporal->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseFFT3DTemporal->Name = L"fcgLBVppDenoiseFFT3DTemporal";
            this->fcgLBVppDenoiseFFT3DTemporal->Size = System::Drawing::Size(83, 22);
            this->fcgLBVppDenoiseFFT3DTemporal->TabIndex = 27;
            this->fcgLBVppDenoiseFFT3DTemporal->Text = L"temporal";
            // 
            // fcgLBVppDenoiseFFT3DPrecision
            // 
            this->fcgLBVppDenoiseFFT3DPrecision->AutoSize = true;
            this->fcgLBVppDenoiseFFT3DPrecision->Location = System::Drawing::Point(254, 102);
            this->fcgLBVppDenoiseFFT3DPrecision->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseFFT3DPrecision->Name = L"fcgLBVppDenoiseFFT3DPrecision";
            this->fcgLBVppDenoiseFFT3DPrecision->Size = System::Drawing::Size(46, 22);
            this->fcgLBVppDenoiseFFT3DPrecision->TabIndex = 26;
            this->fcgLBVppDenoiseFFT3DPrecision->Text = L"prec";
            // 
            // fcgCXVppDenoiseFFT3DPrecision
            // 
            this->fcgCXVppDenoiseFFT3DPrecision->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppDenoiseFFT3DPrecision->FormattingEnabled = true;
            this->fcgCXVppDenoiseFFT3DPrecision->Location = System::Drawing::Point(347, 96);
            this->fcgCXVppDenoiseFFT3DPrecision->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppDenoiseFFT3DPrecision->Name = L"fcgCXVppDenoiseFFT3DPrecision";
            this->fcgCXVppDenoiseFFT3DPrecision->Size = System::Drawing::Size(106, 30);
            this->fcgCXVppDenoiseFFT3DPrecision->TabIndex = 25;
            this->fcgCXVppDenoiseFFT3DPrecision->Tag = L"reCmd";
            // 
            // fcgNUVppDenoiseFFT3DOverlap
            // 
            this->fcgNUVppDenoiseFFT3DOverlap->DecimalPlaces = 2;
            this->fcgNUVppDenoiseFFT3DOverlap->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 65536 });
            this->fcgNUVppDenoiseFFT3DOverlap->Location = System::Drawing::Point(347, 55);
            this->fcgNUVppDenoiseFFT3DOverlap->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoiseFFT3DOverlap->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 8, 0, 0, 65536 });
            this->fcgNUVppDenoiseFFT3DOverlap->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2, 0, 0, 65536 });
            this->fcgNUVppDenoiseFFT3DOverlap->Name = L"fcgNUVppDenoiseFFT3DOverlap";
            this->fcgNUVppDenoiseFFT3DOverlap->Size = System::Drawing::Size(108, 28);
            this->fcgNUVppDenoiseFFT3DOverlap->TabIndex = 24;
            this->fcgNUVppDenoiseFFT3DOverlap->Tag = L"reCmd";
            this->fcgNUVppDenoiseFFT3DOverlap->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoiseFFT3DOverlap->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 65536 });
            // 
            // fcgNUVppDenoiseFFT3DAmount
            // 
            this->fcgNUVppDenoiseFFT3DAmount->DecimalPlaces = 3;
            this->fcgNUVppDenoiseFFT3DAmount->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 196608 });
            this->fcgNUVppDenoiseFFT3DAmount->Location = System::Drawing::Point(347, 14);
            this->fcgNUVppDenoiseFFT3DAmount->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoiseFFT3DAmount->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUVppDenoiseFFT3DAmount->Name = L"fcgNUVppDenoiseFFT3DAmount";
            this->fcgNUVppDenoiseFFT3DAmount->Size = System::Drawing::Size(108, 28);
            this->fcgNUVppDenoiseFFT3DAmount->TabIndex = 23;
            this->fcgNUVppDenoiseFFT3DAmount->Tag = L"reCmd";
            this->fcgNUVppDenoiseFFT3DAmount->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoiseFFT3DAmount->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 196608 });
            // 
            // fcgLBVppDenoiseFFT3DOverlap
            // 
            this->fcgLBVppDenoiseFFT3DOverlap->AutoSize = true;
            this->fcgLBVppDenoiseFFT3DOverlap->Location = System::Drawing::Point(254, 60);
            this->fcgLBVppDenoiseFFT3DOverlap->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseFFT3DOverlap->Name = L"fcgLBVppDenoiseFFT3DOverlap";
            this->fcgLBVppDenoiseFFT3DOverlap->Size = System::Drawing::Size(71, 22);
            this->fcgLBVppDenoiseFFT3DOverlap->TabIndex = 22;
            this->fcgLBVppDenoiseFFT3DOverlap->Text = L"overlap";
            // 
            // fcgNUVppDenoiseFFT3DSigma
            // 
            this->fcgNUVppDenoiseFFT3DSigma->DecimalPlaces = 2;
            this->fcgNUVppDenoiseFFT3DSigma->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 65536 });
            this->fcgNUVppDenoiseFFT3DSigma->Location = System::Drawing::Point(110, 14);
            this->fcgNUVppDenoiseFFT3DSigma->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoiseFFT3DSigma->Name = L"fcgNUVppDenoiseFFT3DSigma";
            this->fcgNUVppDenoiseFFT3DSigma->Size = System::Drawing::Size(108, 28);
            this->fcgNUVppDenoiseFFT3DSigma->TabIndex = 21;
            this->fcgNUVppDenoiseFFT3DSigma->Tag = L"reCmd";
            this->fcgNUVppDenoiseFFT3DSigma->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoiseFFT3DSigma->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            // 
            // fcgCXVppDenoiseFFT3DTemporal
            // 
            this->fcgCXVppDenoiseFFT3DTemporal->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppDenoiseFFT3DTemporal->FormattingEnabled = true;
            this->fcgCXVppDenoiseFFT3DTemporal->Location = System::Drawing::Point(110, 96);
            this->fcgCXVppDenoiseFFT3DTemporal->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppDenoiseFFT3DTemporal->Name = L"fcgCXVppDenoiseFFT3DTemporal";
            this->fcgCXVppDenoiseFFT3DTemporal->Size = System::Drawing::Size(106, 30);
            this->fcgCXVppDenoiseFFT3DTemporal->TabIndex = 20;
            this->fcgCXVppDenoiseFFT3DTemporal->Tag = L"reCmd";
            // 
            // fcgLBVppDenoiseFFT3DAmount
            // 
            this->fcgLBVppDenoiseFFT3DAmount->AutoSize = true;
            this->fcgLBVppDenoiseFFT3DAmount->Location = System::Drawing::Point(254, 19);
            this->fcgLBVppDenoiseFFT3DAmount->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseFFT3DAmount->Name = L"fcgLBVppDenoiseFFT3DAmount";
            this->fcgLBVppDenoiseFFT3DAmount->Size = System::Drawing::Size(74, 22);
            this->fcgLBVppDenoiseFFT3DAmount->TabIndex = 19;
            this->fcgLBVppDenoiseFFT3DAmount->Text = L"amount";
            // 
            // fcgLBVppDenoiseFFT3DBlockSize
            // 
            this->fcgLBVppDenoiseFFT3DBlockSize->AutoSize = true;
            this->fcgLBVppDenoiseFFT3DBlockSize->Location = System::Drawing::Point(11, 60);
            this->fcgLBVppDenoiseFFT3DBlockSize->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseFFT3DBlockSize->Name = L"fcgLBVppDenoiseFFT3DBlockSize";
            this->fcgLBVppDenoiseFFT3DBlockSize->Size = System::Drawing::Size(52, 22);
            this->fcgLBVppDenoiseFFT3DBlockSize->TabIndex = 17;
            this->fcgLBVppDenoiseFFT3DBlockSize->Text = L"block";
            // 
            // fcgCXVppDenoiseFFT3DBlockSize
            // 
            this->fcgCXVppDenoiseFFT3DBlockSize->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppDenoiseFFT3DBlockSize->FormattingEnabled = true;
            this->fcgCXVppDenoiseFFT3DBlockSize->Location = System::Drawing::Point(110, 55);
            this->fcgCXVppDenoiseFFT3DBlockSize->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppDenoiseFFT3DBlockSize->Name = L"fcgCXVppDenoiseFFT3DBlockSize";
            this->fcgCXVppDenoiseFFT3DBlockSize->Size = System::Drawing::Size(106, 30);
            this->fcgCXVppDenoiseFFT3DBlockSize->TabIndex = 16;
            this->fcgCXVppDenoiseFFT3DBlockSize->Tag = L"reCmd";
            // 
            // fcgLBVppDenoiseFFT3DSigma
            // 
            this->fcgLBVppDenoiseFFT3DSigma->AutoSize = true;
            this->fcgLBVppDenoiseFFT3DSigma->Location = System::Drawing::Point(11, 19);
            this->fcgLBVppDenoiseFFT3DSigma->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseFFT3DSigma->Name = L"fcgLBVppDenoiseFFT3DSigma";
            this->fcgLBVppDenoiseFFT3DSigma->Size = System::Drawing::Size(58, 22);
            this->fcgLBVppDenoiseFFT3DSigma->TabIndex = 2;
            this->fcgLBVppDenoiseFFT3DSigma->Text = L"sigma";
            // 
            // fcgPNVppDenoiseNLMeans
            // 
            this->fcgPNVppDenoiseNLMeans->Controls->Add(this->fcgNUVppDenoiseNLMeansH);
            this->fcgPNVppDenoiseNLMeans->Controls->Add(this->fcgLBVppDenoiseNLMeansH);
            this->fcgPNVppDenoiseNLMeans->Controls->Add(this->fcgNUVppDenoiseNLMeansSigma);
            this->fcgPNVppDenoiseNLMeans->Controls->Add(this->fcgCXVppDenoiseNLMeansSearch);
            this->fcgPNVppDenoiseNLMeans->Controls->Add(this->fcgLBVppDenoiseNLMeansSearch);
            this->fcgPNVppDenoiseNLMeans->Controls->Add(this->fcgLBVppDenoiseNLMeansSigma);
            this->fcgPNVppDenoiseNLMeans->Controls->Add(this->fcgCXVppDenoiseNLMeansPatch);
            this->fcgPNVppDenoiseNLMeans->Controls->Add(this->fcgLBVppDenoiseNLMeansPatch);
            this->fcgPNVppDenoiseNLMeans->Location = System::Drawing::Point(5, 62);
            this->fcgPNVppDenoiseNLMeans->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNVppDenoiseNLMeans->Name = L"fcgPNVppDenoiseNLMeans";
            this->fcgPNVppDenoiseNLMeans->Size = System::Drawing::Size(466, 138);
            this->fcgPNVppDenoiseNLMeans->TabIndex = 72;
            // 
            // fcgNUVppDenoiseNLMeansH
            // 
            this->fcgNUVppDenoiseNLMeansH->DecimalPlaces = 3;
            this->fcgNUVppDenoiseNLMeansH->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 196608 });
            this->fcgNUVppDenoiseNLMeansH->Location = System::Drawing::Point(348, 55);
            this->fcgNUVppDenoiseNLMeansH->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoiseNLMeansH->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUVppDenoiseNLMeansH->Name = L"fcgNUVppDenoiseNLMeansH";
            this->fcgNUVppDenoiseNLMeansH->Size = System::Drawing::Size(108, 28);
            this->fcgNUVppDenoiseNLMeansH->TabIndex = 23;
            this->fcgNUVppDenoiseNLMeansH->Tag = L"reCmd";
            this->fcgNUVppDenoiseNLMeansH->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoiseNLMeansH->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            // 
            // fcgLBVppDenoiseNLMeansH
            // 
            this->fcgLBVppDenoiseNLMeansH->AutoSize = true;
            this->fcgLBVppDenoiseNLMeansH->Location = System::Drawing::Point(253, 60);
            this->fcgLBVppDenoiseNLMeansH->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseNLMeansH->Name = L"fcgLBVppDenoiseNLMeansH";
            this->fcgLBVppDenoiseNLMeansH->Size = System::Drawing::Size(21, 22);
            this->fcgLBVppDenoiseNLMeansH->TabIndex = 22;
            this->fcgLBVppDenoiseNLMeansH->Text = L"h";
            // 
            // fcgNUVppDenoiseNLMeansSigma
            // 
            this->fcgNUVppDenoiseNLMeansSigma->DecimalPlaces = 3;
            this->fcgNUVppDenoiseNLMeansSigma->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 196608 });
            this->fcgNUVppDenoiseNLMeansSigma->Location = System::Drawing::Point(96, 54);
            this->fcgNUVppDenoiseNLMeansSigma->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoiseNLMeansSigma->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUVppDenoiseNLMeansSigma->Name = L"fcgNUVppDenoiseNLMeansSigma";
            this->fcgNUVppDenoiseNLMeansSigma->Size = System::Drawing::Size(108, 28);
            this->fcgNUVppDenoiseNLMeansSigma->TabIndex = 21;
            this->fcgNUVppDenoiseNLMeansSigma->Tag = L"reCmd";
            this->fcgNUVppDenoiseNLMeansSigma->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoiseNLMeansSigma->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            // 
            // fcgCXVppDenoiseNLMeansSearch
            // 
            this->fcgCXVppDenoiseNLMeansSearch->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppDenoiseNLMeansSearch->FormattingEnabled = true;
            this->fcgCXVppDenoiseNLMeansSearch->Location = System::Drawing::Point(348, 14);
            this->fcgCXVppDenoiseNLMeansSearch->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppDenoiseNLMeansSearch->Name = L"fcgCXVppDenoiseNLMeansSearch";
            this->fcgCXVppDenoiseNLMeansSearch->Size = System::Drawing::Size(106, 30);
            this->fcgCXVppDenoiseNLMeansSearch->TabIndex = 20;
            this->fcgCXVppDenoiseNLMeansSearch->Tag = L"reCmd";
            // 
            // fcgLBVppDenoiseNLMeansSearch
            // 
            this->fcgLBVppDenoiseNLMeansSearch->AutoSize = true;
            this->fcgLBVppDenoiseNLMeansSearch->Location = System::Drawing::Point(253, 19);
            this->fcgLBVppDenoiseNLMeansSearch->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseNLMeansSearch->Name = L"fcgLBVppDenoiseNLMeansSearch";
            this->fcgLBVppDenoiseNLMeansSearch->Size = System::Drawing::Size(65, 22);
            this->fcgLBVppDenoiseNLMeansSearch->TabIndex = 19;
            this->fcgLBVppDenoiseNLMeansSearch->Text = L"search";
            // 
            // fcgLBVppDenoiseNLMeansSigma
            // 
            this->fcgLBVppDenoiseNLMeansSigma->AutoSize = true;
            this->fcgLBVppDenoiseNLMeansSigma->Location = System::Drawing::Point(19, 60);
            this->fcgLBVppDenoiseNLMeansSigma->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseNLMeansSigma->Name = L"fcgLBVppDenoiseNLMeansSigma";
            this->fcgLBVppDenoiseNLMeansSigma->Size = System::Drawing::Size(58, 22);
            this->fcgLBVppDenoiseNLMeansSigma->TabIndex = 17;
            this->fcgLBVppDenoiseNLMeansSigma->Text = L"sigma";
            // 
            // fcgCXVppDenoiseNLMeansPatch
            // 
            this->fcgCXVppDenoiseNLMeansPatch->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppDenoiseNLMeansPatch->FormattingEnabled = true;
            this->fcgCXVppDenoiseNLMeansPatch->Location = System::Drawing::Point(96, 14);
            this->fcgCXVppDenoiseNLMeansPatch->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppDenoiseNLMeansPatch->Name = L"fcgCXVppDenoiseNLMeansPatch";
            this->fcgCXVppDenoiseNLMeansPatch->Size = System::Drawing::Size(106, 30);
            this->fcgCXVppDenoiseNLMeansPatch->TabIndex = 16;
            this->fcgCXVppDenoiseNLMeansPatch->Tag = L"reCmd";
            // 
            // fcgLBVppDenoiseNLMeansPatch
            // 
            this->fcgLBVppDenoiseNLMeansPatch->AutoSize = true;
            this->fcgLBVppDenoiseNLMeansPatch->Location = System::Drawing::Point(19, 19);
            this->fcgLBVppDenoiseNLMeansPatch->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseNLMeansPatch->Name = L"fcgLBVppDenoiseNLMeansPatch";
            this->fcgLBVppDenoiseNLMeansPatch->Size = System::Drawing::Size(56, 22);
            this->fcgLBVppDenoiseNLMeansPatch->TabIndex = 2;
            this->fcgLBVppDenoiseNLMeansPatch->Text = L"patch";
            // 
            // fcgPNVppDenoiseDct
            // 
            this->fcgPNVppDenoiseDct->Controls->Add(this->fcgNUVppDenoiseDctSigma);
            this->fcgPNVppDenoiseDct->Controls->Add(this->fcgCXVppDenoiseDctBlockSize);
            this->fcgPNVppDenoiseDct->Controls->Add(this->fcgLBVppDenoiseDctBlockSize);
            this->fcgPNVppDenoiseDct->Controls->Add(this->fcgLBVppDenoiseDctSigma);
            this->fcgPNVppDenoiseDct->Controls->Add(this->fcgCXVppDenoiseDctStep);
            this->fcgPNVppDenoiseDct->Controls->Add(this->fcgLBVppDenoiseDctStep);
            this->fcgPNVppDenoiseDct->Location = System::Drawing::Point(5, 62);
            this->fcgPNVppDenoiseDct->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNVppDenoiseDct->Name = L"fcgPNVppDenoiseDct";
            this->fcgPNVppDenoiseDct->Size = System::Drawing::Size(466, 138);
            this->fcgPNVppDenoiseDct->TabIndex = 71;
            // 
            // fcgNUVppDenoiseDctSigma
            // 
            this->fcgNUVppDenoiseDctSigma->DecimalPlaces = 1;
            this->fcgNUVppDenoiseDctSigma->Location = System::Drawing::Point(143, 54);
            this->fcgNUVppDenoiseDctSigma->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoiseDctSigma->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
            this->fcgNUVppDenoiseDctSigma->Name = L"fcgNUVppDenoiseDctSigma";
            this->fcgNUVppDenoiseDctSigma->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppDenoiseDctSigma->TabIndex = 21;
            this->fcgNUVppDenoiseDctSigma->Tag = L"reCmd";
            this->fcgNUVppDenoiseDctSigma->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoiseDctSigma->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            // 
            // fcgCXVppDenoiseDctBlockSize
            // 
            this->fcgCXVppDenoiseDctBlockSize->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppDenoiseDctBlockSize->FormattingEnabled = true;
            this->fcgCXVppDenoiseDctBlockSize->Location = System::Drawing::Point(143, 94);
            this->fcgCXVppDenoiseDctBlockSize->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppDenoiseDctBlockSize->Name = L"fcgCXVppDenoiseDctBlockSize";
            this->fcgCXVppDenoiseDctBlockSize->Size = System::Drawing::Size(232, 30);
            this->fcgCXVppDenoiseDctBlockSize->TabIndex = 20;
            this->fcgCXVppDenoiseDctBlockSize->Tag = L"reCmd";
            // 
            // fcgLBVppDenoiseDctBlockSize
            // 
            this->fcgLBVppDenoiseDctBlockSize->AutoSize = true;
            this->fcgLBVppDenoiseDctBlockSize->Location = System::Drawing::Point(19, 101);
            this->fcgLBVppDenoiseDctBlockSize->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseDctBlockSize->Name = L"fcgLBVppDenoiseDctBlockSize";
            this->fcgLBVppDenoiseDctBlockSize->Size = System::Drawing::Size(97, 22);
            this->fcgLBVppDenoiseDctBlockSize->TabIndex = 19;
            this->fcgLBVppDenoiseDctBlockSize->Text = L"ブロックサイズ";
            // 
            // fcgLBVppDenoiseDctSigma
            // 
            this->fcgLBVppDenoiseDctSigma->AutoSize = true;
            this->fcgLBVppDenoiseDctSigma->Location = System::Drawing::Point(19, 60);
            this->fcgLBVppDenoiseDctSigma->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseDctSigma->Name = L"fcgLBVppDenoiseDctSigma";
            this->fcgLBVppDenoiseDctSigma->Size = System::Drawing::Size(58, 22);
            this->fcgLBVppDenoiseDctSigma->TabIndex = 17;
            this->fcgLBVppDenoiseDctSigma->Text = L"sigma";
            // 
            // fcgCXVppDenoiseDctStep
            // 
            this->fcgCXVppDenoiseDctStep->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppDenoiseDctStep->FormattingEnabled = true;
            this->fcgCXVppDenoiseDctStep->Location = System::Drawing::Point(143, 13);
            this->fcgCXVppDenoiseDctStep->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppDenoiseDctStep->Name = L"fcgCXVppDenoiseDctStep";
            this->fcgCXVppDenoiseDctStep->Size = System::Drawing::Size(232, 30);
            this->fcgCXVppDenoiseDctStep->TabIndex = 16;
            this->fcgCXVppDenoiseDctStep->Tag = L"reCmd";
            // 
            // fcgLBVppDenoiseDctStep
            // 
            this->fcgLBVppDenoiseDctStep->AutoSize = true;
            this->fcgLBVppDenoiseDctStep->Location = System::Drawing::Point(19, 19);
            this->fcgLBVppDenoiseDctStep->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseDctStep->Name = L"fcgLBVppDenoiseDctStep";
            this->fcgLBVppDenoiseDctStep->Size = System::Drawing::Size(59, 22);
            this->fcgLBVppDenoiseDctStep->TabIndex = 2;
            this->fcgLBVppDenoiseDctStep->Text = L"ステップ";
            // 
            // fcgPNVppPreProcess
            // 
            this->fcgPNVppPreProcess->Controls->Add(this->fcgLBVppPreProcessAdaptiveFilter);
            this->fcgPNVppPreProcess->Controls->Add(this->fcgCBVppPreProcessAdaptiveFilter);
            this->fcgPNVppPreProcess->Controls->Add(this->fcgLBVppPreProcessSensitivity);
            this->fcgPNVppPreProcess->Controls->Add(this->fcgLBVppPreProcessStrength);
            this->fcgPNVppPreProcess->Controls->Add(this->fcgNUVppPreProcessSensitivity);
            this->fcgPNVppPreProcess->Controls->Add(this->fcgNUVppPreProcessStrength);
            this->fcgPNVppPreProcess->Location = System::Drawing::Point(5, 62);
            this->fcgPNVppPreProcess->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNVppPreProcess->Name = L"fcgPNVppPreProcess";
            this->fcgPNVppPreProcess->Size = System::Drawing::Size(467, 138);
            this->fcgPNVppPreProcess->TabIndex = 68;
            // 
            // fcgLBVppPreProcessAdaptiveFilter
            // 
            this->fcgLBVppPreProcessAdaptiveFilter->AutoSize = true;
            this->fcgLBVppPreProcessAdaptiveFilter->Location = System::Drawing::Point(74, 95);
            this->fcgLBVppPreProcessAdaptiveFilter->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppPreProcessAdaptiveFilter->Name = L"fcgLBVppPreProcessAdaptiveFilter";
            this->fcgLBVppPreProcessAdaptiveFilter->Size = System::Drawing::Size(91, 22);
            this->fcgLBVppPreProcessAdaptiveFilter->TabIndex = 19;
            this->fcgLBVppPreProcessAdaptiveFilter->Text = L"適応フィルタ";
            // 
            // fcgCBVppPreProcessAdaptiveFilter
            // 
            this->fcgCBVppPreProcessAdaptiveFilter->AutoSize = true;
            this->fcgCBVppPreProcessAdaptiveFilter->Location = System::Drawing::Point(198, 96);
            this->fcgCBVppPreProcessAdaptiveFilter->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBVppPreProcessAdaptiveFilter->Name = L"fcgCBVppPreProcessAdaptiveFilter";
            this->fcgCBVppPreProcessAdaptiveFilter->Size = System::Drawing::Size(22, 21);
            this->fcgCBVppPreProcessAdaptiveFilter->TabIndex = 18;
            this->fcgCBVppPreProcessAdaptiveFilter->Tag = L"reCmd";
            this->fcgCBVppPreProcessAdaptiveFilter->UseVisualStyleBackColor = true;
            // 
            // fcgLBVppPreProcessSensitivity
            // 
            this->fcgLBVppPreProcessSensitivity->AutoSize = true;
            this->fcgLBVppPreProcessSensitivity->Location = System::Drawing::Point(74, 50);
            this->fcgLBVppPreProcessSensitivity->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppPreProcessSensitivity->Name = L"fcgLBVppPreProcessSensitivity";
            this->fcgLBVppPreProcessSensitivity->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppPreProcessSensitivity->TabIndex = 11;
            this->fcgLBVppPreProcessSensitivity->Text = L"閾値";
            // 
            // fcgLBVppPreProcessStrength
            // 
            this->fcgLBVppPreProcessStrength->AutoSize = true;
            this->fcgLBVppPreProcessStrength->Location = System::Drawing::Point(74, 11);
            this->fcgLBVppPreProcessStrength->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppPreProcessStrength->Name = L"fcgLBVppPreProcessStrength";
            this->fcgLBVppPreProcessStrength->Size = System::Drawing::Size(39, 22);
            this->fcgLBVppPreProcessStrength->TabIndex = 10;
            this->fcgLBVppPreProcessStrength->Text = L"強さ";
            // 
            // fcgNUVppPreProcessSensitivity
            // 
            this->fcgNUVppPreProcessSensitivity->Location = System::Drawing::Point(198, 48);
            this->fcgNUVppPreProcessSensitivity->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppPreProcessSensitivity->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
            this->fcgNUVppPreProcessSensitivity->Name = L"fcgNUVppPreProcessSensitivity";
            this->fcgNUVppPreProcessSensitivity->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppPreProcessSensitivity->TabIndex = 8;
            this->fcgNUVppPreProcessSensitivity->Tag = L"reCmd";
            this->fcgNUVppPreProcessSensitivity->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppPreProcessSensitivity->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4, 0, 0, 0 });
            // 
            // fcgNUVppPreProcessStrength
            // 
            this->fcgNUVppPreProcessStrength->Location = System::Drawing::Point(198, 7);
            this->fcgNUVppPreProcessStrength->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppPreProcessStrength->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
            this->fcgNUVppPreProcessStrength->Name = L"fcgNUVppPreProcessStrength";
            this->fcgNUVppPreProcessStrength->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppPreProcessStrength->TabIndex = 7;
            this->fcgNUVppPreProcessStrength->Tag = L"reCmd";
            this->fcgNUVppPreProcessStrength->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppPreProcessStrength->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4, 0, 0, 0 });
            // 
            // fcgCXVppDenoiseMethod
            // 
            this->fcgCXVppDenoiseMethod->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppDenoiseMethod->FormattingEnabled = true;
            this->fcgCXVppDenoiseMethod->Location = System::Drawing::Point(38, 26);
            this->fcgCXVppDenoiseMethod->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppDenoiseMethod->Name = L"fcgCXVppDenoiseMethod";
            this->fcgCXVppDenoiseMethod->Size = System::Drawing::Size(285, 30);
            this->fcgCXVppDenoiseMethod->TabIndex = 0;
            this->fcgCXVppDenoiseMethod->Tag = L"reCmd";
            this->fcgCXVppDenoiseMethod->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgChangeEnabled);
            // 
            // fcgPNVppDenoisePmd
            // 
            this->fcgPNVppDenoisePmd->Controls->Add(this->fcgLBVppDenoisePmdThreshold);
            this->fcgPNVppDenoisePmd->Controls->Add(this->fcgLBVppDenoisePmdStrength);
            this->fcgPNVppDenoisePmd->Controls->Add(this->fcgLBVppDenoisePmdApplyCount);
            this->fcgPNVppDenoisePmd->Controls->Add(this->fcgNUVppDenoisePmdThreshold);
            this->fcgPNVppDenoisePmd->Controls->Add(this->fcgNUVppDenoisePmdStrength);
            this->fcgPNVppDenoisePmd->Controls->Add(this->fcgNUVppDenoisePmdApplyCount);
            this->fcgPNVppDenoisePmd->Location = System::Drawing::Point(5, 62);
            this->fcgPNVppDenoisePmd->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNVppDenoisePmd->Name = L"fcgPNVppDenoisePmd";
            this->fcgPNVppDenoisePmd->Size = System::Drawing::Size(467, 138);
            this->fcgPNVppDenoisePmd->TabIndex = 64;
            // 
            // fcgLBVppDenoisePmdThreshold
            // 
            this->fcgLBVppDenoisePmdThreshold->AutoSize = true;
            this->fcgLBVppDenoisePmdThreshold->Location = System::Drawing::Point(74, 98);
            this->fcgLBVppDenoisePmdThreshold->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoisePmdThreshold->Name = L"fcgLBVppDenoisePmdThreshold";
            this->fcgLBVppDenoisePmdThreshold->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppDenoisePmdThreshold->TabIndex = 11;
            this->fcgLBVppDenoisePmdThreshold->Text = L"閾値";
            // 
            // fcgLBVppDenoisePmdStrength
            // 
            this->fcgLBVppDenoisePmdStrength->AutoSize = true;
            this->fcgLBVppDenoisePmdStrength->Location = System::Drawing::Point(74, 59);
            this->fcgLBVppDenoisePmdStrength->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoisePmdStrength->Name = L"fcgLBVppDenoisePmdStrength";
            this->fcgLBVppDenoisePmdStrength->Size = System::Drawing::Size(39, 22);
            this->fcgLBVppDenoisePmdStrength->TabIndex = 10;
            this->fcgLBVppDenoisePmdStrength->Text = L"強さ";
            // 
            // fcgLBVppDenoisePmdApplyCount
            // 
            this->fcgLBVppDenoisePmdApplyCount->AutoSize = true;
            this->fcgLBVppDenoisePmdApplyCount->Location = System::Drawing::Point(74, 18);
            this->fcgLBVppDenoisePmdApplyCount->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoisePmdApplyCount->Name = L"fcgLBVppDenoisePmdApplyCount";
            this->fcgLBVppDenoisePmdApplyCount->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppDenoisePmdApplyCount->TabIndex = 9;
            this->fcgLBVppDenoisePmdApplyCount->Text = L"回数";
            // 
            // fcgNUVppDenoisePmdThreshold
            // 
            this->fcgNUVppDenoisePmdThreshold->Location = System::Drawing::Point(198, 96);
            this->fcgNUVppDenoisePmdThreshold->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoisePmdThreshold->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
            this->fcgNUVppDenoisePmdThreshold->Name = L"fcgNUVppDenoisePmdThreshold";
            this->fcgNUVppDenoisePmdThreshold->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppDenoisePmdThreshold->TabIndex = 8;
            this->fcgNUVppDenoisePmdThreshold->Tag = L"reCmd";
            this->fcgNUVppDenoisePmdThreshold->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoisePmdThreshold->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 100, 0, 0, 0 });
            // 
            // fcgNUVppDenoisePmdStrength
            // 
            this->fcgNUVppDenoisePmdStrength->Location = System::Drawing::Point(198, 55);
            this->fcgNUVppDenoisePmdStrength->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoisePmdStrength->Name = L"fcgNUVppDenoisePmdStrength";
            this->fcgNUVppDenoisePmdStrength->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppDenoisePmdStrength->TabIndex = 7;
            this->fcgNUVppDenoisePmdStrength->Tag = L"reCmd";
            this->fcgNUVppDenoisePmdStrength->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoisePmdStrength->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 100, 0, 0, 0 });
            // 
            // fcgNUVppDenoisePmdApplyCount
            // 
            this->fcgNUVppDenoisePmdApplyCount->Location = System::Drawing::Point(198, 14);
            this->fcgNUVppDenoisePmdApplyCount->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoisePmdApplyCount->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 0 });
            this->fcgNUVppDenoisePmdApplyCount->Name = L"fcgNUVppDenoisePmdApplyCount";
            this->fcgNUVppDenoisePmdApplyCount->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppDenoisePmdApplyCount->TabIndex = 6;
            this->fcgNUVppDenoisePmdApplyCount->Tag = L"reCmd";
            this->fcgNUVppDenoisePmdApplyCount->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoisePmdApplyCount->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 2, 0, 0, 0 });
            // 
            // fcgPNVppDenoiseConv3D
            // 
            this->fcgPNVppDenoiseConv3D->Controls->Add(this->fcgCXVppDenoiseConv3DMatrix);
            this->fcgPNVppDenoiseConv3D->Controls->Add(this->fcgLBVppDenoiseConv3DMatrix);
            this->fcgPNVppDenoiseConv3D->Controls->Add(this->fcgLBVppDenoiseConv3DThreshTemporal);
            this->fcgPNVppDenoiseConv3D->Controls->Add(this->fcgLBVppDenoiseConv3DThreshSpatial);
            this->fcgPNVppDenoiseConv3D->Controls->Add(this->fcgLBVppDenoiseConv3DThreshCTemporal);
            this->fcgPNVppDenoiseConv3D->Controls->Add(this->fcgLBVppDenoiseConv3DThreshCSpatial);
            this->fcgPNVppDenoiseConv3D->Controls->Add(this->fcgNUVppDenoiseConv3DThreshCTemporal);
            this->fcgPNVppDenoiseConv3D->Controls->Add(this->fcgNUVppDenoiseConv3DThreshCSpatial);
            this->fcgPNVppDenoiseConv3D->Controls->Add(this->fcgLBVppDenoiseConv3DThreshYTemporal);
            this->fcgPNVppDenoiseConv3D->Controls->Add(this->fcgLBVppDenoiseConv3DThreshYSpatial);
            this->fcgPNVppDenoiseConv3D->Controls->Add(this->fcgNUVppDenoiseConv3DThreshYTemporal);
            this->fcgPNVppDenoiseConv3D->Controls->Add(this->fcgNUVppDenoiseConv3DThreshYSpatial);
            this->fcgPNVppDenoiseConv3D->Location = System::Drawing::Point(5, 62);
            this->fcgPNVppDenoiseConv3D->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNVppDenoiseConv3D->Name = L"fcgPNVppDenoiseConv3D";
            this->fcgPNVppDenoiseConv3D->Size = System::Drawing::Size(467, 138);
            this->fcgPNVppDenoiseConv3D->TabIndex = 67;
            // 
            // fcgCXVppDenoiseConv3DMatrix
            // 
            this->fcgCXVppDenoiseConv3DMatrix->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppDenoiseConv3DMatrix->FormattingEnabled = true;
            this->fcgCXVppDenoiseConv3DMatrix->Location = System::Drawing::Point(143, 13);
            this->fcgCXVppDenoiseConv3DMatrix->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppDenoiseConv3DMatrix->Name = L"fcgCXVppDenoiseConv3DMatrix";
            this->fcgCXVppDenoiseConv3DMatrix->Size = System::Drawing::Size(177, 30);
            this->fcgCXVppDenoiseConv3DMatrix->TabIndex = 1;
            this->fcgCXVppDenoiseConv3DMatrix->Tag = L"reCmd";
            // 
            // fcgLBVppDenoiseConv3DMatrix
            // 
            this->fcgLBVppDenoiseConv3DMatrix->AutoSize = true;
            this->fcgLBVppDenoiseConv3DMatrix->Location = System::Drawing::Point(12, 18);
            this->fcgLBVppDenoiseConv3DMatrix->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseConv3DMatrix->Name = L"fcgLBVppDenoiseConv3DMatrix";
            this->fcgLBVppDenoiseConv3DMatrix->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppDenoiseConv3DMatrix->TabIndex = 12;
            this->fcgLBVppDenoiseConv3DMatrix->Text = L"行列";
            // 
            // fcgLBVppDenoiseConv3DThreshTemporal
            // 
            this->fcgLBVppDenoiseConv3DThreshTemporal->AutoSize = true;
            this->fcgLBVppDenoiseConv3DThreshTemporal->Location = System::Drawing::Point(6, 98);
            this->fcgLBVppDenoiseConv3DThreshTemporal->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseConv3DThreshTemporal->Name = L"fcgLBVppDenoiseConv3DThreshTemporal";
            this->fcgLBVppDenoiseConv3DThreshTemporal->Size = System::Drawing::Size(112, 22);
            this->fcgLBVppDenoiseConv3DThreshTemporal->TabIndex = 11;
            this->fcgLBVppDenoiseConv3DThreshTemporal->Text = L"時間方向閾値";
            // 
            // fcgLBVppDenoiseConv3DThreshSpatial
            // 
            this->fcgLBVppDenoiseConv3DThreshSpatial->AutoSize = true;
            this->fcgLBVppDenoiseConv3DThreshSpatial->Location = System::Drawing::Point(6, 60);
            this->fcgLBVppDenoiseConv3DThreshSpatial->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseConv3DThreshSpatial->Name = L"fcgLBVppDenoiseConv3DThreshSpatial";
            this->fcgLBVppDenoiseConv3DThreshSpatial->Size = System::Drawing::Size(112, 22);
            this->fcgLBVppDenoiseConv3DThreshSpatial->TabIndex = 10;
            this->fcgLBVppDenoiseConv3DThreshSpatial->Text = L"空間方向閾値";
            // 
            // fcgLBVppDenoiseConv3DThreshCTemporal
            // 
            this->fcgLBVppDenoiseConv3DThreshCTemporal->AutoSize = true;
            this->fcgLBVppDenoiseConv3DThreshCTemporal->Location = System::Drawing::Point(302, 101);
            this->fcgLBVppDenoiseConv3DThreshCTemporal->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseConv3DThreshCTemporal->Name = L"fcgLBVppDenoiseConv3DThreshCTemporal";
            this->fcgLBVppDenoiseConv3DThreshCTemporal->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppDenoiseConv3DThreshCTemporal->TabIndex = 8;
            this->fcgLBVppDenoiseConv3DThreshCTemporal->Text = L"色差";
            // 
            // fcgLBVppDenoiseConv3DThreshCSpatial
            // 
            this->fcgLBVppDenoiseConv3DThreshCSpatial->AutoSize = true;
            this->fcgLBVppDenoiseConv3DThreshCSpatial->Location = System::Drawing::Point(302, 60);
            this->fcgLBVppDenoiseConv3DThreshCSpatial->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseConv3DThreshCSpatial->Name = L"fcgLBVppDenoiseConv3DThreshCSpatial";
            this->fcgLBVppDenoiseConv3DThreshCSpatial->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppDenoiseConv3DThreshCSpatial->TabIndex = 6;
            this->fcgLBVppDenoiseConv3DThreshCSpatial->Text = L"色差";
            // 
            // fcgNUVppDenoiseConv3DThreshCTemporal
            // 
            this->fcgNUVppDenoiseConv3DThreshCTemporal->Location = System::Drawing::Point(378, 97);
            this->fcgNUVppDenoiseConv3DThreshCTemporal->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoiseConv3DThreshCTemporal->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
            this->fcgNUVppDenoiseConv3DThreshCTemporal->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUVppDenoiseConv3DThreshCTemporal->Name = L"fcgNUVppDenoiseConv3DThreshCTemporal";
            this->fcgNUVppDenoiseConv3DThreshCTemporal->Size = System::Drawing::Size(74, 28);
            this->fcgNUVppDenoiseConv3DThreshCTemporal->TabIndex = 5;
            this->fcgNUVppDenoiseConv3DThreshCTemporal->Tag = L"reCmd";
            this->fcgNUVppDenoiseConv3DThreshCTemporal->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoiseConv3DThreshCTemporal->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 12, 0, 0, 0 });
            // 
            // fcgNUVppDenoiseConv3DThreshCSpatial
            // 
            this->fcgNUVppDenoiseConv3DThreshCSpatial->Location = System::Drawing::Point(378, 58);
            this->fcgNUVppDenoiseConv3DThreshCSpatial->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoiseConv3DThreshCSpatial->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
            this->fcgNUVppDenoiseConv3DThreshCSpatial->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUVppDenoiseConv3DThreshCSpatial->Name = L"fcgNUVppDenoiseConv3DThreshCSpatial";
            this->fcgNUVppDenoiseConv3DThreshCSpatial->Size = System::Drawing::Size(74, 28);
            this->fcgNUVppDenoiseConv3DThreshCSpatial->TabIndex = 3;
            this->fcgNUVppDenoiseConv3DThreshCSpatial->Tag = L"reCmd";
            this->fcgNUVppDenoiseConv3DThreshCSpatial->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoiseConv3DThreshCSpatial->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
            // 
            // fcgLBVppDenoiseConv3DThreshYTemporal
            // 
            this->fcgLBVppDenoiseConv3DThreshYTemporal->AutoSize = true;
            this->fcgLBVppDenoiseConv3DThreshYTemporal->Location = System::Drawing::Point(138, 101);
            this->fcgLBVppDenoiseConv3DThreshYTemporal->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseConv3DThreshYTemporal->Name = L"fcgLBVppDenoiseConv3DThreshYTemporal";
            this->fcgLBVppDenoiseConv3DThreshYTemporal->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppDenoiseConv3DThreshYTemporal->TabIndex = 4;
            this->fcgLBVppDenoiseConv3DThreshYTemporal->Text = L"輝度";
            // 
            // fcgLBVppDenoiseConv3DThreshYSpatial
            // 
            this->fcgLBVppDenoiseConv3DThreshYSpatial->AutoSize = true;
            this->fcgLBVppDenoiseConv3DThreshYSpatial->Location = System::Drawing::Point(138, 60);
            this->fcgLBVppDenoiseConv3DThreshYSpatial->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseConv3DThreshYSpatial->Name = L"fcgLBVppDenoiseConv3DThreshYSpatial";
            this->fcgLBVppDenoiseConv3DThreshYSpatial->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppDenoiseConv3DThreshYSpatial->TabIndex = 2;
            this->fcgLBVppDenoiseConv3DThreshYSpatial->Text = L"輝度";
            // 
            // fcgNUVppDenoiseConv3DThreshYTemporal
            // 
            this->fcgNUVppDenoiseConv3DThreshYTemporal->Location = System::Drawing::Point(202, 97);
            this->fcgNUVppDenoiseConv3DThreshYTemporal->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoiseConv3DThreshYTemporal->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
            this->fcgNUVppDenoiseConv3DThreshYTemporal->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUVppDenoiseConv3DThreshYTemporal->Name = L"fcgNUVppDenoiseConv3DThreshYTemporal";
            this->fcgNUVppDenoiseConv3DThreshYTemporal->Size = System::Drawing::Size(74, 28);
            this->fcgNUVppDenoiseConv3DThreshYTemporal->TabIndex = 4;
            this->fcgNUVppDenoiseConv3DThreshYTemporal->Tag = L"reCmd";
            this->fcgNUVppDenoiseConv3DThreshYTemporal->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoiseConv3DThreshYTemporal->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 12, 0, 0, 0 });
            // 
            // fcgNUVppDenoiseConv3DThreshYSpatial
            // 
            this->fcgNUVppDenoiseConv3DThreshYSpatial->Location = System::Drawing::Point(202, 58);
            this->fcgNUVppDenoiseConv3DThreshYSpatial->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoiseConv3DThreshYSpatial->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
            this->fcgNUVppDenoiseConv3DThreshYSpatial->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUVppDenoiseConv3DThreshYSpatial->Name = L"fcgNUVppDenoiseConv3DThreshYSpatial";
            this->fcgNUVppDenoiseConv3DThreshYSpatial->Size = System::Drawing::Size(74, 28);
            this->fcgNUVppDenoiseConv3DThreshYSpatial->TabIndex = 2;
            this->fcgNUVppDenoiseConv3DThreshYSpatial->Tag = L"reCmd";
            this->fcgNUVppDenoiseConv3DThreshYSpatial->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoiseConv3DThreshYSpatial->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
            // 
            // fcgPNVppDenoiseSmooth
            // 
            this->fcgPNVppDenoiseSmooth->Controls->Add(this->fcgLBVppDenoiseSmoothQP);
            this->fcgPNVppDenoiseSmooth->Controls->Add(this->fcgLBVppDenoiseSmoothQuality);
            this->fcgPNVppDenoiseSmooth->Controls->Add(this->fcgNUVppDenoiseSmoothQP);
            this->fcgPNVppDenoiseSmooth->Controls->Add(this->fcgNUVppDenoiseSmoothQuality);
            this->fcgPNVppDenoiseSmooth->Location = System::Drawing::Point(5, 62);
            this->fcgPNVppDenoiseSmooth->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNVppDenoiseSmooth->Name = L"fcgPNVppDenoiseSmooth";
            this->fcgPNVppDenoiseSmooth->Size = System::Drawing::Size(467, 138);
            this->fcgPNVppDenoiseSmooth->TabIndex = 65;
            // 
            // fcgLBVppDenoiseSmoothQP
            // 
            this->fcgLBVppDenoiseSmoothQP->AutoSize = true;
            this->fcgLBVppDenoiseSmoothQP->Location = System::Drawing::Point(74, 59);
            this->fcgLBVppDenoiseSmoothQP->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseSmoothQP->Name = L"fcgLBVppDenoiseSmoothQP";
            this->fcgLBVppDenoiseSmoothQP->Size = System::Drawing::Size(30, 22);
            this->fcgLBVppDenoiseSmoothQP->TabIndex = 4;
            this->fcgLBVppDenoiseSmoothQP->Text = L"qp";
            // 
            // fcgLBVppDenoiseSmoothQuality
            // 
            this->fcgLBVppDenoiseSmoothQuality->AutoSize = true;
            this->fcgLBVppDenoiseSmoothQuality->Location = System::Drawing::Point(74, 18);
            this->fcgLBVppDenoiseSmoothQuality->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseSmoothQuality->Name = L"fcgLBVppDenoiseSmoothQuality";
            this->fcgLBVppDenoiseSmoothQuality->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppDenoiseSmoothQuality->TabIndex = 2;
            this->fcgLBVppDenoiseSmoothQuality->Text = L"品質";
            // 
            // fcgNUVppDenoiseSmoothQP
            // 
            this->fcgNUVppDenoiseSmoothQP->Location = System::Drawing::Point(198, 55);
            this->fcgNUVppDenoiseSmoothQP->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoiseSmoothQP->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 63, 0, 0, 0 });
            this->fcgNUVppDenoiseSmoothQP->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUVppDenoiseSmoothQP->Name = L"fcgNUVppDenoiseSmoothQP";
            this->fcgNUVppDenoiseSmoothQP->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppDenoiseSmoothQP->TabIndex = 5;
            this->fcgNUVppDenoiseSmoothQP->Tag = L"reCmd";
            this->fcgNUVppDenoiseSmoothQP->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoiseSmoothQP->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 12, 0, 0, 0 });
            // 
            // fcgNUVppDenoiseSmoothQuality
            // 
            this->fcgNUVppDenoiseSmoothQuality->Location = System::Drawing::Point(198, 14);
            this->fcgNUVppDenoiseSmoothQuality->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoiseSmoothQuality->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 6, 0, 0, 0 });
            this->fcgNUVppDenoiseSmoothQuality->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUVppDenoiseSmoothQuality->Name = L"fcgNUVppDenoiseSmoothQuality";
            this->fcgNUVppDenoiseSmoothQuality->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppDenoiseSmoothQuality->TabIndex = 3;
            this->fcgNUVppDenoiseSmoothQuality->Tag = L"reCmd";
            this->fcgNUVppDenoiseSmoothQuality->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoiseSmoothQuality->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
            // 
            // fcgPNVppDenoiseKnn
            // 
            this->fcgPNVppDenoiseKnn->Controls->Add(this->fcgLBVppDenoiseKnnThreshold);
            this->fcgPNVppDenoiseKnn->Controls->Add(this->fcgLBVppDenoiseKnnStrength);
            this->fcgPNVppDenoiseKnn->Controls->Add(this->fcgLBVppDenoiseKnnRadius);
            this->fcgPNVppDenoiseKnn->Controls->Add(this->fcgNUVppDenoiseKnnThreshold);
            this->fcgPNVppDenoiseKnn->Controls->Add(this->fcgNUVppDenoiseKnnStrength);
            this->fcgPNVppDenoiseKnn->Controls->Add(this->fcgNUVppDenoiseKnnRadius);
            this->fcgPNVppDenoiseKnn->Location = System::Drawing::Point(5, 62);
            this->fcgPNVppDenoiseKnn->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNVppDenoiseKnn->Name = L"fcgPNVppDenoiseKnn";
            this->fcgPNVppDenoiseKnn->Size = System::Drawing::Size(467, 138);
            this->fcgPNVppDenoiseKnn->TabIndex = 1;
            // 
            // fcgLBVppDenoiseKnnThreshold
            // 
            this->fcgLBVppDenoiseKnnThreshold->AutoSize = true;
            this->fcgLBVppDenoiseKnnThreshold->Location = System::Drawing::Point(74, 98);
            this->fcgLBVppDenoiseKnnThreshold->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseKnnThreshold->Name = L"fcgLBVppDenoiseKnnThreshold";
            this->fcgLBVppDenoiseKnnThreshold->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppDenoiseKnnThreshold->TabIndex = 6;
            this->fcgLBVppDenoiseKnnThreshold->Text = L"閾値";
            // 
            // fcgLBVppDenoiseKnnStrength
            // 
            this->fcgLBVppDenoiseKnnStrength->AutoSize = true;
            this->fcgLBVppDenoiseKnnStrength->Location = System::Drawing::Point(74, 59);
            this->fcgLBVppDenoiseKnnStrength->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseKnnStrength->Name = L"fcgLBVppDenoiseKnnStrength";
            this->fcgLBVppDenoiseKnnStrength->Size = System::Drawing::Size(39, 22);
            this->fcgLBVppDenoiseKnnStrength->TabIndex = 4;
            this->fcgLBVppDenoiseKnnStrength->Text = L"強さ";
            // 
            // fcgLBVppDenoiseKnnRadius
            // 
            this->fcgLBVppDenoiseKnnRadius->AutoSize = true;
            this->fcgLBVppDenoiseKnnRadius->Location = System::Drawing::Point(74, 18);
            this->fcgLBVppDenoiseKnnRadius->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDenoiseKnnRadius->Name = L"fcgLBVppDenoiseKnnRadius";
            this->fcgLBVppDenoiseKnnRadius->Size = System::Drawing::Size(44, 22);
            this->fcgLBVppDenoiseKnnRadius->TabIndex = 2;
            this->fcgLBVppDenoiseKnnRadius->Text = L"半径";
            // 
            // fcgNUVppDenoiseKnnThreshold
            // 
            this->fcgNUVppDenoiseKnnThreshold->DecimalPlaces = 2;
            this->fcgNUVppDenoiseKnnThreshold->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 131072 });
            this->fcgNUVppDenoiseKnnThreshold->Location = System::Drawing::Point(198, 96);
            this->fcgNUVppDenoiseKnnThreshold->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoiseKnnThreshold->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUVppDenoiseKnnThreshold->Name = L"fcgNUVppDenoiseKnnThreshold";
            this->fcgNUVppDenoiseKnnThreshold->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppDenoiseKnnThreshold->TabIndex = 7;
            this->fcgNUVppDenoiseKnnThreshold->Tag = L"reCmd";
            this->fcgNUVppDenoiseKnnThreshold->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoiseKnnThreshold->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            // 
            // fcgNUVppDenoiseKnnStrength
            // 
            this->fcgNUVppDenoiseKnnStrength->DecimalPlaces = 2;
            this->fcgNUVppDenoiseKnnStrength->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 131072 });
            this->fcgNUVppDenoiseKnnStrength->Location = System::Drawing::Point(198, 55);
            this->fcgNUVppDenoiseKnnStrength->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoiseKnnStrength->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUVppDenoiseKnnStrength->Name = L"fcgNUVppDenoiseKnnStrength";
            this->fcgNUVppDenoiseKnnStrength->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppDenoiseKnnStrength->TabIndex = 5;
            this->fcgNUVppDenoiseKnnStrength->Tag = L"reCmd";
            this->fcgNUVppDenoiseKnnStrength->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoiseKnnStrength->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            // 
            // fcgNUVppDenoiseKnnRadius
            // 
            this->fcgNUVppDenoiseKnnRadius->Location = System::Drawing::Point(198, 14);
            this->fcgNUVppDenoiseKnnRadius->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDenoiseKnnRadius->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 5, 0, 0, 0 });
            this->fcgNUVppDenoiseKnnRadius->Name = L"fcgNUVppDenoiseKnnRadius";
            this->fcgNUVppDenoiseKnnRadius->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppDenoiseKnnRadius->TabIndex = 3;
            this->fcgNUVppDenoiseKnnRadius->Tag = L"reCmd";
            this->fcgNUVppDenoiseKnnRadius->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppDenoiseKnnRadius->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
            // 
            // fcgCBPSNR
            // 
            this->fcgCBPSNR->AutoSize = true;
            this->fcgCBPSNR->Location = System::Drawing::Point(746, 562);
            this->fcgCBPSNR->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBPSNR->Name = L"fcgCBPSNR";
            this->fcgCBPSNR->Size = System::Drawing::Size(72, 26);
            this->fcgCBPSNR->TabIndex = 17;
            this->fcgCBPSNR->Tag = L"reCmd";
            this->fcgCBPSNR->Text = L"psnr";
            this->fcgCBPSNR->UseVisualStyleBackColor = true;
            // 
            // fcgCBSSIM
            // 
            this->fcgCBSSIM->AutoSize = true;
            this->fcgCBSSIM->Location = System::Drawing::Point(746, 526);
            this->fcgCBSSIM->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBSSIM->Name = L"fcgCBSSIM";
            this->fcgCBSSIM->Size = System::Drawing::Size(72, 26);
            this->fcgCBSSIM->TabIndex = 16;
            this->fcgCBSSIM->Tag = L"reCmd";
            this->fcgCBSSIM->Text = L"ssim";
            this->fcgCBSSIM->UseVisualStyleBackColor = true;
            // 
            // fcggroupBoxVppDeinterlace
            // 
            this->fcggroupBoxVppDeinterlace->Controls->Add(this->fcgPNVppDecomb);
            this->fcggroupBoxVppDeinterlace->Controls->Add(this->fcgLBVppDeinterlace);
            this->fcggroupBoxVppDeinterlace->Controls->Add(this->fcgCXVppDeinterlace);
            this->fcggroupBoxVppDeinterlace->Controls->Add(this->fcgPNVppAfs);
            this->fcggroupBoxVppDeinterlace->Controls->Add(this->fcgPNVppYadif);
            this->fcggroupBoxVppDeinterlace->Controls->Add(this->fcgPNVppNnedi);
            this->fcggroupBoxVppDeinterlace->Location = System::Drawing::Point(515, 10);
            this->fcggroupBoxVppDeinterlace->Margin = System::Windows::Forms::Padding(5);
            this->fcggroupBoxVppDeinterlace->Name = L"fcggroupBoxVppDeinterlace";
            this->fcggroupBoxVppDeinterlace->Padding = System::Windows::Forms::Padding(5);
            this->fcggroupBoxVppDeinterlace->Size = System::Drawing::Size(394, 506);
            this->fcggroupBoxVppDeinterlace->TabIndex = 20;
            this->fcggroupBoxVppDeinterlace->TabStop = false;
            this->fcggroupBoxVppDeinterlace->Text = L"インタレ解除";
            // 
            // fcgPNVppDecomb
            // 
            this->fcgPNVppDecomb->Controls->Add(this->fcgCBVppDecombBlend);
            this->fcgPNVppDecomb->Controls->Add(this->fcgCBVppDecombFull);
            this->fcgPNVppDecomb->Controls->Add(this->fcgLBVppDecombDthreshold);
            this->fcgPNVppDecomb->Controls->Add(this->fcgNUVppDecombDthreshold);
            this->fcgPNVppDecomb->Controls->Add(this->fcgLBVppDecombThreshold);
            this->fcgPNVppDecomb->Controls->Add(this->fcgNUVppDecombThreshold);
            this->fcgPNVppDecomb->Location = System::Drawing::Point(10, 61);
            this->fcgPNVppDecomb->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNVppDecomb->Name = L"fcgPNVppDecomb";
            this->fcgPNVppDecomb->Size = System::Drawing::Size(377, 442);
            this->fcgPNVppDecomb->TabIndex = 81;
            // 
            // fcgCBVppDecombBlend
            // 
            this->fcgCBVppDecombBlend->AutoSize = true;
            this->fcgCBVppDecombBlend->Location = System::Drawing::Point(22, 43);
            this->fcgCBVppDecombBlend->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBVppDecombBlend->Name = L"fcgCBVppDecombBlend";
            this->fcgCBVppDecombBlend->Size = System::Drawing::Size(87, 26);
            this->fcgCBVppDecombBlend->TabIndex = 19;
            this->fcgCBVppDecombBlend->Tag = L"reCmd";
            this->fcgCBVppDecombBlend->Text = L"ブレンド";
            this->fcgCBVppDecombBlend->UseVisualStyleBackColor = true;
            // 
            // fcgCBVppDecombFull
            // 
            this->fcgCBVppDecombFull->AutoSize = true;
            this->fcgCBVppDecombFull->Location = System::Drawing::Point(22, 10);
            this->fcgCBVppDecombFull->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBVppDecombFull->Name = L"fcgCBVppDecombFull";
            this->fcgCBVppDecombFull->Size = System::Drawing::Size(152, 26);
            this->fcgCBVppDecombFull->TabIndex = 18;
            this->fcgCBVppDecombFull->Tag = L"reCmd";
            this->fcgCBVppDecombFull->Text = L"全フレームを解除";
            this->fcgCBVppDecombFull->UseVisualStyleBackColor = true;
            // 
            // fcgLBVppDecombDthreshold
            // 
            this->fcgLBVppDecombDthreshold->AutoSize = true;
            this->fcgLBVppDecombDthreshold->Location = System::Drawing::Point(34, 118);
            this->fcgLBVppDecombDthreshold->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDecombDthreshold->Name = L"fcgLBVppDecombDthreshold";
            this->fcgLBVppDecombDthreshold->Size = System::Drawing::Size(78, 22);
            this->fcgLBVppDecombDthreshold->TabIndex = 7;
            this->fcgLBVppDecombDthreshold->Text = L"解除閾値";
            // 
            // fcgNUVppDecombDthreshold
            // 
            this->fcgNUVppDecombDthreshold->Location = System::Drawing::Point(138, 114);
            this->fcgNUVppDecombDthreshold->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDecombDthreshold->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
            this->fcgNUVppDecombDthreshold->Name = L"fcgNUVppDecombDthreshold";
            this->fcgNUVppDecombDthreshold->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppDecombDthreshold->TabIndex = 8;
            this->fcgNUVppDecombDthreshold->Tag = L"reCmd";
            this->fcgNUVppDecombDthreshold->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBVppDecombThreshold
            // 
            this->fcgLBVppDecombThreshold->AutoSize = true;
            this->fcgLBVppDecombThreshold->Location = System::Drawing::Point(34, 82);
            this->fcgLBVppDecombThreshold->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDecombThreshold->Name = L"fcgLBVppDecombThreshold";
            this->fcgLBVppDecombThreshold->Size = System::Drawing::Size(78, 22);
            this->fcgLBVppDecombThreshold->TabIndex = 5;
            this->fcgLBVppDecombThreshold->Text = L"判定閾値";
            // 
            // fcgNUVppDecombThreshold
            // 
            this->fcgNUVppDecombThreshold->Location = System::Drawing::Point(138, 78);
            this->fcgNUVppDecombThreshold->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppDecombThreshold->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 255, 0, 0, 0 });
            this->fcgNUVppDecombThreshold->Name = L"fcgNUVppDecombThreshold";
            this->fcgNUVppDecombThreshold->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppDecombThreshold->TabIndex = 6;
            this->fcgNUVppDecombThreshold->Tag = L"reCmd";
            this->fcgNUVppDecombThreshold->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBVppDeinterlace
            // 
            this->fcgLBVppDeinterlace->AutoSize = true;
            this->fcgLBVppDeinterlace->Location = System::Drawing::Point(23, 29);
            this->fcgLBVppDeinterlace->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppDeinterlace->Name = L"fcgLBVppDeinterlace";
            this->fcgLBVppDeinterlace->Size = System::Drawing::Size(83, 22);
            this->fcgLBVppDeinterlace->TabIndex = 0;
            this->fcgLBVppDeinterlace->Text = L"解除モード";
            // 
            // fcgCXVppDeinterlace
            // 
            this->fcgCXVppDeinterlace->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppDeinterlace->FormattingEnabled = true;
            this->fcgCXVppDeinterlace->Location = System::Drawing::Point(125, 23);
            this->fcgCXVppDeinterlace->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppDeinterlace->Name = L"fcgCXVppDeinterlace";
            this->fcgCXVppDeinterlace->Size = System::Drawing::Size(244, 30);
            this->fcgCXVppDeinterlace->TabIndex = 1;
            this->fcgCXVppDeinterlace->Tag = L"reCmd";
            this->fcgCXVppDeinterlace->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgChangeEnabled);
            // 
            // fcgPNVppAfs
            // 
            this->fcgPNVppAfs->Controls->Add(this->fcgTBVppAfsThreCMotion);
            this->fcgPNVppAfs->Controls->Add(this->fcgLBVppAfsThreCMotion);
            this->fcgPNVppAfs->Controls->Add(this->fcgTBVppAfsThreYMotion);
            this->fcgPNVppAfs->Controls->Add(this->fcgLBVppAfsThreYmotion);
            this->fcgPNVppAfs->Controls->Add(this->fcgTBVppAfsThreDeint);
            this->fcgPNVppAfs->Controls->Add(this->fcgLBVppAfsThreDeint);
            this->fcgPNVppAfs->Controls->Add(this->fcgTBVppAfsThreShift);
            this->fcgPNVppAfs->Controls->Add(this->fcgLBVppAfsThreShift);
            this->fcgPNVppAfs->Controls->Add(this->fcgTBVppAfsCoeffShift);
            this->fcgPNVppAfs->Controls->Add(this->fcgLBVppAfsCoeffShift);
            this->fcgPNVppAfs->Controls->Add(this->fcgLBVppAfsRight);
            this->fcgPNVppAfs->Controls->Add(this->fcgLBVppAfsLeft);
            this->fcgPNVppAfs->Controls->Add(this->fcgLBVppAfsBottom);
            this->fcgPNVppAfs->Controls->Add(this->fcgLBVppAfsUp);
            this->fcgPNVppAfs->Controls->Add(this->fcgNUVppAfsRight);
            this->fcgPNVppAfs->Controls->Add(this->fcgNUVppAfsLeft);
            this->fcgPNVppAfs->Controls->Add(this->fcgNUVppAfsBottom);
            this->fcgPNVppAfs->Controls->Add(this->fcgNUVppAfsUp);
            this->fcgPNVppAfs->Controls->Add(this->fcgTBVppAfsMethodSwitch);
            this->fcgPNVppAfs->Controls->Add(this->fcgCBVppAfs24fps);
            this->fcgPNVppAfs->Controls->Add(this->fcgCBVppAfsTune);
            this->fcgPNVppAfs->Controls->Add(this->fcgCBVppAfsSmooth);
            this->fcgPNVppAfs->Controls->Add(this->fcgCBVppAfsDrop);
            this->fcgPNVppAfs->Controls->Add(this->fcgCBVppAfsShift);
            this->fcgPNVppAfs->Controls->Add(this->fcgLBVppAfsAnalyze);
            this->fcgPNVppAfs->Controls->Add(this->fcgNUVppAfsThreCMotion);
            this->fcgPNVppAfs->Controls->Add(this->fcgNUVppAfsThreShift);
            this->fcgPNVppAfs->Controls->Add(this->fcgNUVppAfsThreDeint);
            this->fcgPNVppAfs->Controls->Add(this->fcgNUVppAfsThreYMotion);
            this->fcgPNVppAfs->Controls->Add(this->fcgLBVppAfsMethodSwitch);
            this->fcgPNVppAfs->Controls->Add(this->fcgCXVppAfsAnalyze);
            this->fcgPNVppAfs->Controls->Add(this->fcgNUVppAfsCoeffShift);
            this->fcgPNVppAfs->Controls->Add(this->fcgNUVppAfsMethodSwitch);
            this->fcgPNVppAfs->Location = System::Drawing::Point(10, 61);
            this->fcgPNVppAfs->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNVppAfs->Name = L"fcgPNVppAfs";
            this->fcgPNVppAfs->Size = System::Drawing::Size(377, 442);
            this->fcgPNVppAfs->TabIndex = 10;
            // 
            // fcgTBVppAfsThreCMotion
            // 
            this->fcgTBVppAfsThreCMotion->AutoSize = false;
            this->fcgTBVppAfsThreCMotion->Location = System::Drawing::Point(106, 269);
            this->fcgTBVppAfsThreCMotion->Margin = System::Windows::Forms::Padding(5);
            this->fcgTBVppAfsThreCMotion->Maximum = 1024;
            this->fcgTBVppAfsThreCMotion->Name = L"fcgTBVppAfsThreCMotion";
            this->fcgTBVppAfsThreCMotion->Size = System::Drawing::Size(168, 26);
            this->fcgTBVppAfsThreCMotion->TabIndex = 24;
            this->fcgTBVppAfsThreCMotion->TickStyle = System::Windows::Forms::TickStyle::None;
            this->fcgTBVppAfsThreCMotion->Scroll += gcnew System::EventHandler(this, &frmConfig::fcgTBVppAfsScroll);
            // 
            // fcgLBVppAfsThreCMotion
            // 
            this->fcgLBVppAfsThreCMotion->AutoSize = true;
            this->fcgLBVppAfsThreCMotion->Location = System::Drawing::Point(7, 269);
            this->fcgLBVppAfsThreCMotion->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppAfsThreCMotion->Name = L"fcgLBVppAfsThreCMotion";
            this->fcgLBVppAfsThreCMotion->Size = System::Drawing::Size(51, 22);
            this->fcgLBVppAfsThreCMotion->TabIndex = 23;
            this->fcgLBVppAfsThreCMotion->Text = L"C動き";
            // 
            // fcgTBVppAfsThreYMotion
            // 
            this->fcgTBVppAfsThreYMotion->AutoSize = false;
            this->fcgTBVppAfsThreYMotion->Location = System::Drawing::Point(106, 230);
            this->fcgTBVppAfsThreYMotion->Margin = System::Windows::Forms::Padding(5);
            this->fcgTBVppAfsThreYMotion->Maximum = 1024;
            this->fcgTBVppAfsThreYMotion->Name = L"fcgTBVppAfsThreYMotion";
            this->fcgTBVppAfsThreYMotion->Size = System::Drawing::Size(168, 26);
            this->fcgTBVppAfsThreYMotion->TabIndex = 21;
            this->fcgTBVppAfsThreYMotion->TickStyle = System::Windows::Forms::TickStyle::None;
            this->fcgTBVppAfsThreYMotion->Scroll += gcnew System::EventHandler(this, &frmConfig::fcgTBVppAfsScroll);
            // 
            // fcgLBVppAfsThreYmotion
            // 
            this->fcgLBVppAfsThreYmotion->AutoSize = true;
            this->fcgLBVppAfsThreYmotion->Location = System::Drawing::Point(7, 230);
            this->fcgLBVppAfsThreYmotion->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppAfsThreYmotion->Name = L"fcgLBVppAfsThreYmotion";
            this->fcgLBVppAfsThreYmotion->Size = System::Drawing::Size(51, 22);
            this->fcgLBVppAfsThreYmotion->TabIndex = 20;
            this->fcgLBVppAfsThreYmotion->Text = L"Y動き";
            // 
            // fcgTBVppAfsThreDeint
            // 
            this->fcgTBVppAfsThreDeint->AutoSize = false;
            this->fcgTBVppAfsThreDeint->Location = System::Drawing::Point(106, 193);
            this->fcgTBVppAfsThreDeint->Margin = System::Windows::Forms::Padding(5);
            this->fcgTBVppAfsThreDeint->Maximum = 1024;
            this->fcgTBVppAfsThreDeint->Name = L"fcgTBVppAfsThreDeint";
            this->fcgTBVppAfsThreDeint->Size = System::Drawing::Size(168, 26);
            this->fcgTBVppAfsThreDeint->TabIndex = 18;
            this->fcgTBVppAfsThreDeint->TickStyle = System::Windows::Forms::TickStyle::None;
            this->fcgTBVppAfsThreDeint->Scroll += gcnew System::EventHandler(this, &frmConfig::fcgTBVppAfsScroll);
            // 
            // fcgLBVppAfsThreDeint
            // 
            this->fcgLBVppAfsThreDeint->AutoSize = true;
            this->fcgLBVppAfsThreDeint->Location = System::Drawing::Point(7, 193);
            this->fcgLBVppAfsThreDeint->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppAfsThreDeint->Name = L"fcgLBVppAfsThreDeint";
            this->fcgLBVppAfsThreDeint->Size = System::Drawing::Size(75, 22);
            this->fcgLBVppAfsThreDeint->TabIndex = 17;
            this->fcgLBVppAfsThreDeint->Text = L"縞(解除)";
            // 
            // fcgTBVppAfsThreShift
            // 
            this->fcgTBVppAfsThreShift->AutoSize = false;
            this->fcgTBVppAfsThreShift->Location = System::Drawing::Point(106, 156);
            this->fcgTBVppAfsThreShift->Margin = System::Windows::Forms::Padding(5);
            this->fcgTBVppAfsThreShift->Maximum = 1024;
            this->fcgTBVppAfsThreShift->Name = L"fcgTBVppAfsThreShift";
            this->fcgTBVppAfsThreShift->Size = System::Drawing::Size(168, 26);
            this->fcgTBVppAfsThreShift->TabIndex = 15;
            this->fcgTBVppAfsThreShift->TickStyle = System::Windows::Forms::TickStyle::None;
            this->fcgTBVppAfsThreShift->Scroll += gcnew System::EventHandler(this, &frmConfig::fcgTBVppAfsScroll);
            // 
            // fcgLBVppAfsThreShift
            // 
            this->fcgLBVppAfsThreShift->AutoSize = true;
            this->fcgLBVppAfsThreShift->Location = System::Drawing::Point(7, 156);
            this->fcgLBVppAfsThreShift->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppAfsThreShift->Name = L"fcgLBVppAfsThreShift";
            this->fcgLBVppAfsThreShift->Size = System::Drawing::Size(68, 22);
            this->fcgLBVppAfsThreShift->TabIndex = 14;
            this->fcgLBVppAfsThreShift->Text = L"縞(ｼﾌﾄ)";
            // 
            // fcgTBVppAfsCoeffShift
            // 
            this->fcgTBVppAfsCoeffShift->AutoSize = false;
            this->fcgTBVppAfsCoeffShift->Location = System::Drawing::Point(106, 119);
            this->fcgTBVppAfsCoeffShift->Margin = System::Windows::Forms::Padding(5);
            this->fcgTBVppAfsCoeffShift->Maximum = 256;
            this->fcgTBVppAfsCoeffShift->Name = L"fcgTBVppAfsCoeffShift";
            this->fcgTBVppAfsCoeffShift->Size = System::Drawing::Size(168, 26);
            this->fcgTBVppAfsCoeffShift->TabIndex = 12;
            this->fcgTBVppAfsCoeffShift->TickStyle = System::Windows::Forms::TickStyle::None;
            this->fcgTBVppAfsCoeffShift->Scroll += gcnew System::EventHandler(this, &frmConfig::fcgTBVppAfsScroll);
            // 
            // fcgLBVppAfsCoeffShift
            // 
            this->fcgLBVppAfsCoeffShift->AutoSize = true;
            this->fcgLBVppAfsCoeffShift->Location = System::Drawing::Point(7, 119);
            this->fcgLBVppAfsCoeffShift->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppAfsCoeffShift->Name = L"fcgLBVppAfsCoeffShift";
            this->fcgLBVppAfsCoeffShift->Size = System::Drawing::Size(61, 22);
            this->fcgLBVppAfsCoeffShift->TabIndex = 11;
            this->fcgLBVppAfsCoeffShift->Text = L"判定比";
            // 
            // fcgLBVppAfsRight
            // 
            this->fcgLBVppAfsRight->AutoSize = true;
            this->fcgLBVppAfsRight->Location = System::Drawing::Point(252, 43);
            this->fcgLBVppAfsRight->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppAfsRight->Name = L"fcgLBVppAfsRight";
            this->fcgLBVppAfsRight->Size = System::Drawing::Size(27, 22);
            this->fcgLBVppAfsRight->TabIndex = 6;
            this->fcgLBVppAfsRight->Text = L"右";
            // 
            // fcgLBVppAfsLeft
            // 
            this->fcgLBVppAfsLeft->AutoSize = true;
            this->fcgLBVppAfsLeft->Location = System::Drawing::Point(94, 43);
            this->fcgLBVppAfsLeft->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppAfsLeft->Name = L"fcgLBVppAfsLeft";
            this->fcgLBVppAfsLeft->Size = System::Drawing::Size(27, 22);
            this->fcgLBVppAfsLeft->TabIndex = 4;
            this->fcgLBVppAfsLeft->Text = L"左";
            // 
            // fcgLBVppAfsBottom
            // 
            this->fcgLBVppAfsBottom->AutoSize = true;
            this->fcgLBVppAfsBottom->Location = System::Drawing::Point(252, 6);
            this->fcgLBVppAfsBottom->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppAfsBottom->Name = L"fcgLBVppAfsBottom";
            this->fcgLBVppAfsBottom->Size = System::Drawing::Size(27, 22);
            this->fcgLBVppAfsBottom->TabIndex = 2;
            this->fcgLBVppAfsBottom->Text = L"下";
            // 
            // fcgLBVppAfsUp
            // 
            this->fcgLBVppAfsUp->AutoSize = true;
            this->fcgLBVppAfsUp->Location = System::Drawing::Point(94, 6);
            this->fcgLBVppAfsUp->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppAfsUp->Name = L"fcgLBVppAfsUp";
            this->fcgLBVppAfsUp->Size = System::Drawing::Size(27, 22);
            this->fcgLBVppAfsUp->TabIndex = 0;
            this->fcgLBVppAfsUp->Text = L"上";
            // 
            // fcgNUVppAfsRight
            // 
            this->fcgNUVppAfsRight->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 8, 0, 0, 0 });
            this->fcgNUVppAfsRight->Location = System::Drawing::Point(278, 41);
            this->fcgNUVppAfsRight->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppAfsRight->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 8192, 0, 0, 0 });
            this->fcgNUVppAfsRight->Name = L"fcgNUVppAfsRight";
            this->fcgNUVppAfsRight->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppAfsRight->TabIndex = 7;
            this->fcgNUVppAfsRight->Tag = L"reCmd";
            this->fcgNUVppAfsRight->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUVppAfsLeft
            // 
            this->fcgNUVppAfsLeft->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 8, 0, 0, 0 });
            this->fcgNUVppAfsLeft->Location = System::Drawing::Point(122, 41);
            this->fcgNUVppAfsLeft->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppAfsLeft->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 8192, 0, 0, 0 });
            this->fcgNUVppAfsLeft->Name = L"fcgNUVppAfsLeft";
            this->fcgNUVppAfsLeft->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppAfsLeft->TabIndex = 5;
            this->fcgNUVppAfsLeft->Tag = L"reCmd";
            this->fcgNUVppAfsLeft->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUVppAfsBottom
            // 
            this->fcgNUVppAfsBottom->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 8, 0, 0, 0 });
            this->fcgNUVppAfsBottom->Location = System::Drawing::Point(278, 2);
            this->fcgNUVppAfsBottom->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppAfsBottom->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4096, 0, 0, 0 });
            this->fcgNUVppAfsBottom->Name = L"fcgNUVppAfsBottom";
            this->fcgNUVppAfsBottom->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppAfsBottom->TabIndex = 3;
            this->fcgNUVppAfsBottom->Tag = L"reCmd";
            this->fcgNUVppAfsBottom->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUVppAfsUp
            // 
            this->fcgNUVppAfsUp->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 8, 0, 0, 0 });
            this->fcgNUVppAfsUp->Location = System::Drawing::Point(122, 2);
            this->fcgNUVppAfsUp->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppAfsUp->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4096, 0, 0, 0 });
            this->fcgNUVppAfsUp->Name = L"fcgNUVppAfsUp";
            this->fcgNUVppAfsUp->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppAfsUp->TabIndex = 1;
            this->fcgNUVppAfsUp->Tag = L"reCmd";
            this->fcgNUVppAfsUp->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgTBVppAfsMethodSwitch
            // 
            this->fcgTBVppAfsMethodSwitch->AutoSize = false;
            this->fcgTBVppAfsMethodSwitch->Location = System::Drawing::Point(106, 82);
            this->fcgTBVppAfsMethodSwitch->Margin = System::Windows::Forms::Padding(5);
            this->fcgTBVppAfsMethodSwitch->Maximum = 256;
            this->fcgTBVppAfsMethodSwitch->Name = L"fcgTBVppAfsMethodSwitch";
            this->fcgTBVppAfsMethodSwitch->Size = System::Drawing::Size(168, 26);
            this->fcgTBVppAfsMethodSwitch->TabIndex = 9;
            this->fcgTBVppAfsMethodSwitch->TickStyle = System::Windows::Forms::TickStyle::None;
            this->fcgTBVppAfsMethodSwitch->Scroll += gcnew System::EventHandler(this, &frmConfig::fcgTBVppAfsScroll);
            // 
            // fcgCBVppAfs24fps
            // 
            this->fcgCBVppAfs24fps->AutoSize = true;
            this->fcgCBVppAfs24fps->Location = System::Drawing::Point(223, 347);
            this->fcgCBVppAfs24fps->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBVppAfs24fps->Name = L"fcgCBVppAfs24fps";
            this->fcgCBVppAfs24fps->Size = System::Drawing::Size(99, 26);
            this->fcgCBVppAfs24fps->TabIndex = 29;
            this->fcgCBVppAfs24fps->Tag = L"reCmd";
            this->fcgCBVppAfs24fps->Text = L"24fps化";
            this->fcgCBVppAfs24fps->UseVisualStyleBackColor = true;
            // 
            // fcgCBVppAfsTune
            // 
            this->fcgCBVppAfsTune->AutoSize = true;
            this->fcgCBVppAfsTune->Location = System::Drawing::Point(223, 409);
            this->fcgCBVppAfsTune->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBVppAfsTune->Name = L"fcgCBVppAfsTune";
            this->fcgCBVppAfsTune->Size = System::Drawing::Size(109, 26);
            this->fcgCBVppAfsTune->TabIndex = 32;
            this->fcgCBVppAfsTune->Tag = L"reCmd";
            this->fcgCBVppAfsTune->Text = L"調整モード";
            this->fcgCBVppAfsTune->UseVisualStyleBackColor = true;
            // 
            // fcgCBVppAfsSmooth
            // 
            this->fcgCBVppAfsSmooth->AutoSize = true;
            this->fcgCBVppAfsSmooth->Location = System::Drawing::Point(17, 409);
            this->fcgCBVppAfsSmooth->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBVppAfsSmooth->Name = L"fcgCBVppAfsSmooth";
            this->fcgCBVppAfsSmooth->Size = System::Drawing::Size(117, 26);
            this->fcgCBVppAfsSmooth->TabIndex = 31;
            this->fcgCBVppAfsSmooth->Tag = L"reCmd";
            this->fcgCBVppAfsSmooth->Text = L"スムージング";
            this->fcgCBVppAfsSmooth->UseVisualStyleBackColor = true;
            // 
            // fcgCBVppAfsDrop
            // 
            this->fcgCBVppAfsDrop->AutoSize = true;
            this->fcgCBVppAfsDrop->Location = System::Drawing::Point(17, 378);
            this->fcgCBVppAfsDrop->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBVppAfsDrop->Name = L"fcgCBVppAfsDrop";
            this->fcgCBVppAfsDrop->Size = System::Drawing::Size(83, 26);
            this->fcgCBVppAfsDrop->TabIndex = 30;
            this->fcgCBVppAfsDrop->Tag = L"reCmd";
            this->fcgCBVppAfsDrop->Text = L"間引き";
            this->fcgCBVppAfsDrop->UseVisualStyleBackColor = true;
            // 
            // fcgCBVppAfsShift
            // 
            this->fcgCBVppAfsShift->AutoSize = true;
            this->fcgCBVppAfsShift->Location = System::Drawing::Point(17, 347);
            this->fcgCBVppAfsShift->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBVppAfsShift->Name = L"fcgCBVppAfsShift";
            this->fcgCBVppAfsShift->Size = System::Drawing::Size(134, 26);
            this->fcgCBVppAfsShift->TabIndex = 28;
            this->fcgCBVppAfsShift->Tag = L"reCmd";
            this->fcgCBVppAfsShift->Text = L"フィールドシフト";
            this->fcgCBVppAfsShift->UseVisualStyleBackColor = true;
            // 
            // fcgLBVppAfsAnalyze
            // 
            this->fcgLBVppAfsAnalyze->AutoSize = true;
            this->fcgLBVppAfsAnalyze->Location = System::Drawing::Point(7, 311);
            this->fcgLBVppAfsAnalyze->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppAfsAnalyze->Name = L"fcgLBVppAfsAnalyze";
            this->fcgLBVppAfsAnalyze->Size = System::Drawing::Size(62, 22);
            this->fcgLBVppAfsAnalyze->TabIndex = 26;
            this->fcgLBVppAfsAnalyze->Text = L"解除Lv";
            // 
            // fcgNUVppAfsThreCMotion
            // 
            this->fcgNUVppAfsThreCMotion->Location = System::Drawing::Point(278, 269);
            this->fcgNUVppAfsThreCMotion->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppAfsThreCMotion->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1024, 0, 0, 0 });
            this->fcgNUVppAfsThreCMotion->Name = L"fcgNUVppAfsThreCMotion";
            this->fcgNUVppAfsThreCMotion->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppAfsThreCMotion->TabIndex = 25;
            this->fcgNUVppAfsThreCMotion->Tag = L"reCmd";
            this->fcgNUVppAfsThreCMotion->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppAfsThreCMotion->ValueChanged += gcnew System::EventHandler(this, &frmConfig::fcgNUVppAfsValueChanged);
            // 
            // fcgNUVppAfsThreShift
            // 
            this->fcgNUVppAfsThreShift->Location = System::Drawing::Point(278, 156);
            this->fcgNUVppAfsThreShift->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppAfsThreShift->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1024, 0, 0, 0 });
            this->fcgNUVppAfsThreShift->Name = L"fcgNUVppAfsThreShift";
            this->fcgNUVppAfsThreShift->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppAfsThreShift->TabIndex = 16;
            this->fcgNUVppAfsThreShift->Tag = L"reCmd";
            this->fcgNUVppAfsThreShift->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppAfsThreShift->ValueChanged += gcnew System::EventHandler(this, &frmConfig::fcgNUVppAfsValueChanged);
            // 
            // fcgNUVppAfsThreDeint
            // 
            this->fcgNUVppAfsThreDeint->Location = System::Drawing::Point(278, 193);
            this->fcgNUVppAfsThreDeint->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppAfsThreDeint->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1024, 0, 0, 0 });
            this->fcgNUVppAfsThreDeint->Name = L"fcgNUVppAfsThreDeint";
            this->fcgNUVppAfsThreDeint->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppAfsThreDeint->TabIndex = 19;
            this->fcgNUVppAfsThreDeint->Tag = L"reCmd";
            this->fcgNUVppAfsThreDeint->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppAfsThreDeint->ValueChanged += gcnew System::EventHandler(this, &frmConfig::fcgNUVppAfsValueChanged);
            // 
            // fcgNUVppAfsThreYMotion
            // 
            this->fcgNUVppAfsThreYMotion->Location = System::Drawing::Point(278, 230);
            this->fcgNUVppAfsThreYMotion->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppAfsThreYMotion->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1024, 0, 0, 0 });
            this->fcgNUVppAfsThreYMotion->Name = L"fcgNUVppAfsThreYMotion";
            this->fcgNUVppAfsThreYMotion->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppAfsThreYMotion->TabIndex = 22;
            this->fcgNUVppAfsThreYMotion->Tag = L"reCmd";
            this->fcgNUVppAfsThreYMotion->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppAfsThreYMotion->ValueChanged += gcnew System::EventHandler(this, &frmConfig::fcgNUVppAfsValueChanged);
            // 
            // fcgLBVppAfsMethodSwitch
            // 
            this->fcgLBVppAfsMethodSwitch->AutoSize = true;
            this->fcgLBVppAfsMethodSwitch->Location = System::Drawing::Point(7, 82);
            this->fcgLBVppAfsMethodSwitch->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppAfsMethodSwitch->Name = L"fcgLBVppAfsMethodSwitch";
            this->fcgLBVppAfsMethodSwitch->Size = System::Drawing::Size(61, 22);
            this->fcgLBVppAfsMethodSwitch->TabIndex = 8;
            this->fcgLBVppAfsMethodSwitch->Text = L"切替点";
            // 
            // fcgCXVppAfsAnalyze
            // 
            this->fcgCXVppAfsAnalyze->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppAfsAnalyze->FormattingEnabled = true;
            this->fcgCXVppAfsAnalyze->Location = System::Drawing::Point(106, 306);
            this->fcgCXVppAfsAnalyze->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppAfsAnalyze->Name = L"fcgCXVppAfsAnalyze";
            this->fcgCXVppAfsAnalyze->Size = System::Drawing::Size(262, 30);
            this->fcgCXVppAfsAnalyze->TabIndex = 27;
            this->fcgCXVppAfsAnalyze->Tag = L"reCmd";
            // 
            // fcgNUVppAfsCoeffShift
            // 
            this->fcgNUVppAfsCoeffShift->Location = System::Drawing::Point(278, 119);
            this->fcgNUVppAfsCoeffShift->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppAfsCoeffShift->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 256, 0, 0, 0 });
            this->fcgNUVppAfsCoeffShift->Name = L"fcgNUVppAfsCoeffShift";
            this->fcgNUVppAfsCoeffShift->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppAfsCoeffShift->TabIndex = 13;
            this->fcgNUVppAfsCoeffShift->Tag = L"reCmd";
            this->fcgNUVppAfsCoeffShift->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppAfsCoeffShift->ValueChanged += gcnew System::EventHandler(this, &frmConfig::fcgNUVppAfsValueChanged);
            // 
            // fcgNUVppAfsMethodSwitch
            // 
            this->fcgNUVppAfsMethodSwitch->Location = System::Drawing::Point(278, 82);
            this->fcgNUVppAfsMethodSwitch->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUVppAfsMethodSwitch->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 256, 0, 0, 0 });
            this->fcgNUVppAfsMethodSwitch->Name = L"fcgNUVppAfsMethodSwitch";
            this->fcgNUVppAfsMethodSwitch->Size = System::Drawing::Size(90, 28);
            this->fcgNUVppAfsMethodSwitch->TabIndex = 10;
            this->fcgNUVppAfsMethodSwitch->Tag = L"reCmd";
            this->fcgNUVppAfsMethodSwitch->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppAfsMethodSwitch->ValueChanged += gcnew System::EventHandler(this, &frmConfig::fcgNUVppAfsValueChanged);
            // 
            // fcgPNVppYadif
            // 
            this->fcgPNVppYadif->Controls->Add(this->fcgLBVppYadifMode);
            this->fcgPNVppYadif->Controls->Add(this->fcgCXVppYadifMode);
            this->fcgPNVppYadif->Location = System::Drawing::Point(10, 61);
            this->fcgPNVppYadif->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNVppYadif->Name = L"fcgPNVppYadif";
            this->fcgPNVppYadif->Size = System::Drawing::Size(377, 442);
            this->fcgPNVppYadif->TabIndex = 12;
            // 
            // fcgLBVppYadifMode
            // 
            this->fcgLBVppYadifMode->AutoSize = true;
            this->fcgLBVppYadifMode->Location = System::Drawing::Point(22, 18);
            this->fcgLBVppYadifMode->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppYadifMode->Name = L"fcgLBVppYadifMode";
            this->fcgLBVppYadifMode->Size = System::Drawing::Size(56, 22);
            this->fcgLBVppYadifMode->TabIndex = 78;
            this->fcgLBVppYadifMode->Text = L"mode";
            // 
            // fcgCXVppYadifMode
            // 
            this->fcgCXVppYadifMode->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppYadifMode->FormattingEnabled = true;
            this->fcgCXVppYadifMode->Location = System::Drawing::Point(121, 13);
            this->fcgCXVppYadifMode->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppYadifMode->Name = L"fcgCXVppYadifMode";
            this->fcgCXVppYadifMode->Size = System::Drawing::Size(238, 30);
            this->fcgCXVppYadifMode->TabIndex = 0;
            this->fcgCXVppYadifMode->Tag = L"reCmd";
            // 
            // fcgPNVppNnedi
            // 
            this->fcgPNVppNnedi->Controls->Add(this->fcgLBVppNnediErrorType);
            this->fcgPNVppNnedi->Controls->Add(this->fcgCXVppNnediErrorType);
            this->fcgPNVppNnedi->Controls->Add(this->fcgLBVppNnediPrescreen);
            this->fcgPNVppNnedi->Controls->Add(this->fcgCXVppNnediPrescreen);
            this->fcgPNVppNnedi->Controls->Add(this->fcgLBVppNnediPrec);
            this->fcgPNVppNnedi->Controls->Add(this->fcgCXVppNnediPrec);
            this->fcgPNVppNnedi->Controls->Add(this->fcgLBVppNnediQual);
            this->fcgPNVppNnedi->Controls->Add(this->fcgCXVppNnediQual);
            this->fcgPNVppNnedi->Controls->Add(this->fcgLBVppNnediNsize);
            this->fcgPNVppNnedi->Controls->Add(this->fcgCXVppNnediNsize);
            this->fcgPNVppNnedi->Controls->Add(this->fcgLBVppNnediNns);
            this->fcgPNVppNnedi->Controls->Add(this->fcgCXVppNnediNns);
            this->fcgPNVppNnedi->Location = System::Drawing::Point(10, 61);
            this->fcgPNVppNnedi->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNVppNnedi->Name = L"fcgPNVppNnedi";
            this->fcgPNVppNnedi->Size = System::Drawing::Size(377, 442);
            this->fcgPNVppNnedi->TabIndex = 11;
            // 
            // fcgLBVppNnediErrorType
            // 
            this->fcgLBVppNnediErrorType->AutoSize = true;
            this->fcgLBVppNnediErrorType->Location = System::Drawing::Point(22, 228);
            this->fcgLBVppNnediErrorType->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppNnediErrorType->Name = L"fcgLBVppNnediErrorType";
            this->fcgLBVppNnediErrorType->Size = System::Drawing::Size(87, 22);
            this->fcgLBVppNnediErrorType->TabIndex = 10;
            this->fcgLBVppNnediErrorType->Text = L"errortype";
            // 
            // fcgCXVppNnediErrorType
            // 
            this->fcgCXVppNnediErrorType->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppNnediErrorType->FormattingEnabled = true;
            this->fcgCXVppNnediErrorType->Location = System::Drawing::Point(121, 223);
            this->fcgCXVppNnediErrorType->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppNnediErrorType->Name = L"fcgCXVppNnediErrorType";
            this->fcgCXVppNnediErrorType->Size = System::Drawing::Size(238, 30);
            this->fcgCXVppNnediErrorType->TabIndex = 11;
            this->fcgCXVppNnediErrorType->Tag = L"reCmd";
            // 
            // fcgLBVppNnediPrescreen
            // 
            this->fcgLBVppNnediPrescreen->AutoSize = true;
            this->fcgLBVppNnediPrescreen->Location = System::Drawing::Point(22, 186);
            this->fcgLBVppNnediPrescreen->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppNnediPrescreen->Name = L"fcgLBVppNnediPrescreen";
            this->fcgLBVppNnediPrescreen->Size = System::Drawing::Size(92, 22);
            this->fcgLBVppNnediPrescreen->TabIndex = 8;
            this->fcgLBVppNnediPrescreen->Text = L"prescreen";
            // 
            // fcgCXVppNnediPrescreen
            // 
            this->fcgCXVppNnediPrescreen->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppNnediPrescreen->FormattingEnabled = true;
            this->fcgCXVppNnediPrescreen->Location = System::Drawing::Point(121, 181);
            this->fcgCXVppNnediPrescreen->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppNnediPrescreen->Name = L"fcgCXVppNnediPrescreen";
            this->fcgCXVppNnediPrescreen->Size = System::Drawing::Size(238, 30);
            this->fcgCXVppNnediPrescreen->TabIndex = 9;
            this->fcgCXVppNnediPrescreen->Tag = L"reCmd";
            // 
            // fcgLBVppNnediPrec
            // 
            this->fcgLBVppNnediPrec->AutoSize = true;
            this->fcgLBVppNnediPrec->Location = System::Drawing::Point(22, 144);
            this->fcgLBVppNnediPrec->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppNnediPrec->Name = L"fcgLBVppNnediPrec";
            this->fcgLBVppNnediPrec->Size = System::Drawing::Size(46, 22);
            this->fcgLBVppNnediPrec->TabIndex = 6;
            this->fcgLBVppNnediPrec->Text = L"prec";
            // 
            // fcgCXVppNnediPrec
            // 
            this->fcgCXVppNnediPrec->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppNnediPrec->FormattingEnabled = true;
            this->fcgCXVppNnediPrec->Location = System::Drawing::Point(121, 139);
            this->fcgCXVppNnediPrec->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppNnediPrec->Name = L"fcgCXVppNnediPrec";
            this->fcgCXVppNnediPrec->Size = System::Drawing::Size(238, 30);
            this->fcgCXVppNnediPrec->TabIndex = 7;
            this->fcgCXVppNnediPrec->Tag = L"reCmd";
            // 
            // fcgLBVppNnediQual
            // 
            this->fcgLBVppNnediQual->AutoSize = true;
            this->fcgLBVppNnediQual->Location = System::Drawing::Point(22, 102);
            this->fcgLBVppNnediQual->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppNnediQual->Name = L"fcgLBVppNnediQual";
            this->fcgLBVppNnediQual->Size = System::Drawing::Size(45, 22);
            this->fcgLBVppNnediQual->TabIndex = 4;
            this->fcgLBVppNnediQual->Text = L"qual";
            // 
            // fcgCXVppNnediQual
            // 
            this->fcgCXVppNnediQual->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppNnediQual->FormattingEnabled = true;
            this->fcgCXVppNnediQual->Location = System::Drawing::Point(121, 97);
            this->fcgCXVppNnediQual->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppNnediQual->Name = L"fcgCXVppNnediQual";
            this->fcgCXVppNnediQual->Size = System::Drawing::Size(238, 30);
            this->fcgCXVppNnediQual->TabIndex = 5;
            this->fcgCXVppNnediQual->Tag = L"reCmd";
            // 
            // fcgLBVppNnediNsize
            // 
            this->fcgLBVppNnediNsize->AutoSize = true;
            this->fcgLBVppNnediNsize->Location = System::Drawing::Point(22, 60);
            this->fcgLBVppNnediNsize->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppNnediNsize->Name = L"fcgLBVppNnediNsize";
            this->fcgLBVppNnediNsize->Size = System::Drawing::Size(52, 22);
            this->fcgLBVppNnediNsize->TabIndex = 2;
            this->fcgLBVppNnediNsize->Text = L"nsize";
            // 
            // fcgCXVppNnediNsize
            // 
            this->fcgCXVppNnediNsize->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppNnediNsize->FormattingEnabled = true;
            this->fcgCXVppNnediNsize->Location = System::Drawing::Point(121, 55);
            this->fcgCXVppNnediNsize->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppNnediNsize->Name = L"fcgCXVppNnediNsize";
            this->fcgCXVppNnediNsize->Size = System::Drawing::Size(238, 30);
            this->fcgCXVppNnediNsize->TabIndex = 3;
            this->fcgCXVppNnediNsize->Tag = L"reCmd";
            // 
            // fcgLBVppNnediNns
            // 
            this->fcgLBVppNnediNns->AutoSize = true;
            this->fcgLBVppNnediNns->Location = System::Drawing::Point(22, 18);
            this->fcgLBVppNnediNns->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppNnediNns->Name = L"fcgLBVppNnediNns";
            this->fcgLBVppNnediNns->Size = System::Drawing::Size(40, 22);
            this->fcgLBVppNnediNns->TabIndex = 0;
            this->fcgLBVppNnediNns->Text = L"nns";
            // 
            // fcgCXVppNnediNns
            // 
            this->fcgCXVppNnediNns->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppNnediNns->FormattingEnabled = true;
            this->fcgCXVppNnediNns->Location = System::Drawing::Point(121, 13);
            this->fcgCXVppNnediNns->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppNnediNns->Name = L"fcgCXVppNnediNns";
            this->fcgCXVppNnediNns->Size = System::Drawing::Size(238, 30);
            this->fcgCXVppNnediNns->TabIndex = 1;
            this->fcgCXVppNnediNns->Tag = L"reCmd";
            // 
            // fcgCBVppResize
            // 
            this->fcgCBVppResize->AutoSize = true;
            this->fcgCBVppResize->Location = System::Drawing::Point(24, 7);
            this->fcgCBVppResize->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBVppResize->Name = L"fcgCBVppResize";
            this->fcgCBVppResize->Size = System::Drawing::Size(86, 26);
            this->fcgCBVppResize->TabIndex = 0;
            this->fcgCBVppResize->Tag = L"reCmd";
            this->fcgCBVppResize->Text = L"リサイズ";
            this->fcgCBVppResize->UseVisualStyleBackColor = true;
            this->fcgCBVppResize->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgChangeEnabled);
            // 
            // fcggroupBoxResize
            // 
            this->fcggroupBoxResize->Controls->Add(this->fcgCXVppResizeAlg);
            this->fcggroupBoxResize->Controls->Add(this->fcgLBVppResize);
            this->fcggroupBoxResize->Controls->Add(this->fcgNUResizeH);
            this->fcggroupBoxResize->Controls->Add(this->fcgNUResizeW);
            this->fcggroupBoxResize->Location = System::Drawing::Point(6, 10);
            this->fcggroupBoxResize->Margin = System::Windows::Forms::Padding(5);
            this->fcggroupBoxResize->Name = L"fcggroupBoxResize";
            this->fcggroupBoxResize->Padding = System::Windows::Forms::Padding(5);
            this->fcggroupBoxResize->Size = System::Drawing::Size(499, 82);
            this->fcggroupBoxResize->TabIndex = 1;
            this->fcggroupBoxResize->TabStop = false;
            // 
            // fcgCXVppResizeAlg
            // 
            this->fcgCXVppResizeAlg->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppResizeAlg->FormattingEnabled = true;
            this->fcgCXVppResizeAlg->Location = System::Drawing::Point(275, 34);
            this->fcgCXVppResizeAlg->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXVppResizeAlg->Name = L"fcgCXVppResizeAlg";
            this->fcgCXVppResizeAlg->Size = System::Drawing::Size(214, 30);
            this->fcgCXVppResizeAlg->TabIndex = 3;
            this->fcgCXVppResizeAlg->Tag = L"reCmd";
            // 
            // fcgLBVppResize
            // 
            this->fcgLBVppResize->AutoSize = true;
            this->fcgLBVppResize->Location = System::Drawing::Point(127, 37);
            this->fcgLBVppResize->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBVppResize->Name = L"fcgLBVppResize";
            this->fcgLBVppResize->Size = System::Drawing::Size(19, 22);
            this->fcgLBVppResize->TabIndex = 1;
            this->fcgLBVppResize->Text = L"x";
            // 
            // fcgNUResizeH
            // 
            this->fcgNUResizeH->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16, 0, 0, 0 });
            this->fcgNUResizeH->Location = System::Drawing::Point(156, 35);
            this->fcgNUResizeH->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUResizeH->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65535, 0, 0, 0 });
            this->fcgNUResizeH->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65536, 0, 0, System::Int32::MinValue });
            this->fcgNUResizeH->Name = L"fcgNUResizeH";
            this->fcgNUResizeH->Size = System::Drawing::Size(106, 28);
            this->fcgNUResizeH->TabIndex = 2;
            this->fcgNUResizeH->Tag = L"reCmd";
            this->fcgNUResizeH->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUResizeW
            // 
            this->fcgNUResizeW->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16, 0, 0, 0 });
            this->fcgNUResizeW->Location = System::Drawing::Point(14, 35);
            this->fcgNUResizeW->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUResizeW->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65535, 0, 0, 0 });
            this->fcgNUResizeW->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65536, 0, 0, System::Int32::MinValue });
            this->fcgNUResizeW->Name = L"fcgNUResizeW";
            this->fcgNUResizeW->Size = System::Drawing::Size(106, 28);
            this->fcgNUResizeW->TabIndex = 0;
            this->fcgNUResizeW->Tag = L"reCmd";
            this->fcgNUResizeW->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // tabPageExOpt
            // 
            this->tabPageExOpt->Controls->Add(this->fcggroupBoxCmdEx);
            this->tabPageExOpt->Controls->Add(this->fcgCBTimerPeriodTuning);
            this->tabPageExOpt->Controls->Add(this->fcgCBAuoTcfileout);
            this->tabPageExOpt->Controls->Add(this->fcgLBTempDir);
            this->tabPageExOpt->Controls->Add(this->fcgBTCustomTempDir);
            this->tabPageExOpt->Controls->Add(this->fcgTXCustomTempDir);
            this->tabPageExOpt->Controls->Add(this->fcgCXTempDir);
            this->tabPageExOpt->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->tabPageExOpt->Location = System::Drawing::Point(4, 32);
            this->tabPageExOpt->Margin = System::Windows::Forms::Padding(5);
            this->tabPageExOpt->Name = L"tabPageExOpt";
            this->tabPageExOpt->Size = System::Drawing::Size(916, 727);
            this->tabPageExOpt->TabIndex = 2;
            this->tabPageExOpt->Text = L"その他";
            this->tabPageExOpt->UseVisualStyleBackColor = true;
            // 
            // fcggroupBoxCmdEx
            // 
            this->fcggroupBoxCmdEx->Controls->Add(this->fcgTXCmdEx);
            this->fcggroupBoxCmdEx->Location = System::Drawing::Point(14, 359);
            this->fcggroupBoxCmdEx->Margin = System::Windows::Forms::Padding(5);
            this->fcggroupBoxCmdEx->Name = L"fcggroupBoxCmdEx";
            this->fcggroupBoxCmdEx->Padding = System::Windows::Forms::Padding(5);
            this->fcggroupBoxCmdEx->Size = System::Drawing::Size(883, 349);
            this->fcggroupBoxCmdEx->TabIndex = 61;
            this->fcggroupBoxCmdEx->TabStop = false;
            this->fcggroupBoxCmdEx->Text = L"追加コマンド";
            // 
            // fcgTXCmdEx
            // 
            this->fcgTXCmdEx->AllowDrop = true;
            this->fcgTXCmdEx->Font = (gcnew System::Drawing::Font(L"ＭＳ ゴシック", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgTXCmdEx->Location = System::Drawing::Point(10, 30);
            this->fcgTXCmdEx->Margin = System::Windows::Forms::Padding(5);
            this->fcgTXCmdEx->Multiline = true;
            this->fcgTXCmdEx->Name = L"fcgTXCmdEx";
            this->fcgTXCmdEx->Size = System::Drawing::Size(863, 302);
            this->fcgTXCmdEx->TabIndex = 0;
            this->fcgTXCmdEx->Tag = L"chValue";
            // 
            // fcgCBTimerPeriodTuning
            // 
            this->fcgCBTimerPeriodTuning->AutoSize = true;
            this->fcgCBTimerPeriodTuning->Location = System::Drawing::Point(23, 13);
            this->fcgCBTimerPeriodTuning->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBTimerPeriodTuning->Name = L"fcgCBTimerPeriodTuning";
            this->fcgCBTimerPeriodTuning->Size = System::Drawing::Size(138, 26);
            this->fcgCBTimerPeriodTuning->TabIndex = 20;
            this->fcgCBTimerPeriodTuning->Tag = L"chValue";
            this->fcgCBTimerPeriodTuning->Text = L"高精度タイマー";
            this->fcgCBTimerPeriodTuning->UseVisualStyleBackColor = true;
            // 
            // fcgCBAuoTcfileout
            // 
            this->fcgCBAuoTcfileout->AutoSize = true;
            this->fcgCBAuoTcfileout->Location = System::Drawing::Point(22, 49);
            this->fcgCBAuoTcfileout->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBAuoTcfileout->Name = L"fcgCBAuoTcfileout";
            this->fcgCBAuoTcfileout->Size = System::Drawing::Size(146, 26);
            this->fcgCBAuoTcfileout->TabIndex = 21;
            this->fcgCBAuoTcfileout->Tag = L"chValue";
            this->fcgCBAuoTcfileout->Text = L"タイムコード出力";
            this->fcgCBAuoTcfileout->UseVisualStyleBackColor = true;
            // 
            // fcgLBTempDir
            // 
            this->fcgLBTempDir->AutoSize = true;
            this->fcgLBTempDir->Location = System::Drawing::Point(546, 13);
            this->fcgLBTempDir->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBTempDir->Name = L"fcgLBTempDir";
            this->fcgLBTempDir->Size = System::Drawing::Size(92, 22);
            this->fcgLBTempDir->TabIndex = 16;
            this->fcgLBTempDir->Text = L"一時フォルダ";
            // 
            // fcgBTCustomTempDir
            // 
            this->fcgBTCustomTempDir->Location = System::Drawing::Point(846, 86);
            this->fcgBTCustomTempDir->Margin = System::Windows::Forms::Padding(5);
            this->fcgBTCustomTempDir->Name = L"fcgBTCustomTempDir";
            this->fcgBTCustomTempDir->Size = System::Drawing::Size(43, 35);
            this->fcgBTCustomTempDir->TabIndex = 19;
            this->fcgBTCustomTempDir->Text = L"...";
            this->fcgBTCustomTempDir->UseVisualStyleBackColor = true;
            this->fcgBTCustomTempDir->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTCustomTempDir_Click);
            // 
            // fcgTXCustomTempDir
            // 
            this->fcgTXCustomTempDir->Location = System::Drawing::Point(569, 89);
            this->fcgTXCustomTempDir->Margin = System::Windows::Forms::Padding(5);
            this->fcgTXCustomTempDir->Name = L"fcgTXCustomTempDir";
            this->fcgTXCustomTempDir->Size = System::Drawing::Size(270, 28);
            this->fcgTXCustomTempDir->TabIndex = 18;
            this->fcgTXCustomTempDir->Tag = L"";
            // 
            // fcgCXTempDir
            // 
            this->fcgCXTempDir->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXTempDir->FormattingEnabled = true;
            this->fcgCXTempDir->Location = System::Drawing::Point(551, 47);
            this->fcgCXTempDir->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXTempDir->Name = L"fcgCXTempDir";
            this->fcgCXTempDir->Size = System::Drawing::Size(311, 30);
            this->fcgCXTempDir->TabIndex = 17;
            this->fcgCXTempDir->Tag = L"chValue";
            // 
            // fcgCSExeFiles
            // 
            this->fcgCSExeFiles->ImageScalingSize = System::Drawing::Size(20, 20);
            this->fcgCSExeFiles->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { this->fcgTSExeFileshelp });
            this->fcgCSExeFiles->Name = L"fcgCSx264";
            this->fcgCSExeFiles->Size = System::Drawing::Size(169, 36);
            // 
            // fcgTSExeFileshelp
            // 
            this->fcgTSExeFileshelp->Name = L"fcgTSExeFileshelp";
            this->fcgTSExeFileshelp->Size = System::Drawing::Size(168, 32);
            this->fcgTSExeFileshelp->Text = L"helpを表示";
            this->fcgTSExeFileshelp->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSExeFileshelp_Click);
            // 
            // fcgLBguiExBlog
            // 
            this->fcgLBguiExBlog->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
            this->fcgLBguiExBlog->AutoSize = true;
            this->fcgLBguiExBlog->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Italic, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgLBguiExBlog->LinkColor = System::Drawing::Color::Gray;
            this->fcgLBguiExBlog->Location = System::Drawing::Point(959, 869);
            this->fcgLBguiExBlog->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBguiExBlog->Name = L"fcgLBguiExBlog";
            this->fcgLBguiExBlog->Size = System::Drawing::Size(128, 22);
            this->fcgLBguiExBlog->TabIndex = 50;
            this->fcgLBguiExBlog->TabStop = true;
            this->fcgLBguiExBlog->Text = L"VCEEncについて";
            this->fcgLBguiExBlog->VisitedLinkColor = System::Drawing::Color::Gray;
            this->fcgLBguiExBlog->LinkClicked += gcnew System::Windows::Forms::LinkLabelLinkClickedEventHandler(this, &frmConfig::fcgLBguiExBlog_LinkClicked);
            // 
            // fcgtabControlAudio
            // 
            this->fcgtabControlAudio->Controls->Add(this->fcgtabPageAudioMain);
            this->fcgtabControlAudio->Controls->Add(this->fcgtabPageAudioOther);
            this->fcgtabControlAudio->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgtabControlAudio->Location = System::Drawing::Point(934, 47);
            this->fcgtabControlAudio->Margin = System::Windows::Forms::Padding(5);
            this->fcgtabControlAudio->Name = L"fcgtabControlAudio";
            this->fcgtabControlAudio->SelectedIndex = 0;
            this->fcgtabControlAudio->Size = System::Drawing::Size(570, 454);
            this->fcgtabControlAudio->TabIndex = 51;
            // 
            // fcgtabPageAudioMain
            // 
            this->fcgtabPageAudioMain->Controls->Add(this->fcgCBAudioUseExt);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgCBFAWCheck);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgPNAudioExt);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgPNAudioInternal);
            this->fcgtabPageAudioMain->Location = System::Drawing::Point(4, 31);
            this->fcgtabPageAudioMain->Margin = System::Windows::Forms::Padding(5);
            this->fcgtabPageAudioMain->Name = L"fcgtabPageAudioMain";
            this->fcgtabPageAudioMain->Padding = System::Windows::Forms::Padding(5);
            this->fcgtabPageAudioMain->Size = System::Drawing::Size(562, 419);
            this->fcgtabPageAudioMain->TabIndex = 0;
            this->fcgtabPageAudioMain->Text = L"音声";
            this->fcgtabPageAudioMain->UseVisualStyleBackColor = true;
            // 
            // fcgCBAudioUseExt
            // 
            this->fcgCBAudioUseExt->AutoSize = true;
            this->fcgCBAudioUseExt->Location = System::Drawing::Point(14, 7);
            this->fcgCBAudioUseExt->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBAudioUseExt->Name = L"fcgCBAudioUseExt";
            this->fcgCBAudioUseExt->Size = System::Drawing::Size(208, 26);
            this->fcgCBAudioUseExt->TabIndex = 79;
            this->fcgCBAudioUseExt->Tag = L"chValue";
            this->fcgCBAudioUseExt->Text = L"外部エンコーダを使用する";
            this->fcgCBAudioUseExt->UseVisualStyleBackColor = true;
            this->fcgCBAudioUseExt->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgCBAudioUseExt_CheckedChanged);
            // 
            // fcgCBFAWCheck
            // 
            this->fcgCBFAWCheck->AutoSize = true;
            this->fcgCBFAWCheck->Location = System::Drawing::Point(382, 7);
            this->fcgCBFAWCheck->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBFAWCheck->Name = L"fcgCBFAWCheck";
            this->fcgCBFAWCheck->Size = System::Drawing::Size(124, 26);
            this->fcgCBFAWCheck->TabIndex = 32;
            this->fcgCBFAWCheck->Tag = L"chValue";
            this->fcgCBFAWCheck->Text = L"FAWCheck";
            this->fcgCBFAWCheck->UseVisualStyleBackColor = true;
            // 
            // fcgPNAudioExt
            // 
            this->fcgPNAudioExt->Controls->Add(this->fcgCXAudioDelayCut);
            this->fcgPNAudioExt->Controls->Add(this->fcgLBAudioDelayCut);
            this->fcgPNAudioExt->Controls->Add(this->fcgCBAudioEncTiming);
            this->fcgPNAudioExt->Controls->Add(this->fcgCXAudioEncTiming);
            this->fcgPNAudioExt->Controls->Add(this->fcgCXAudioTempDir);
            this->fcgPNAudioExt->Controls->Add(this->fcgTXCustomAudioTempDir);
            this->fcgPNAudioExt->Controls->Add(this->fcgBTCustomAudioTempDir);
            this->fcgPNAudioExt->Controls->Add(this->fcgCBAudioUsePipe);
            this->fcgPNAudioExt->Controls->Add(this->fcgNUAudioBitrate);
            this->fcgPNAudioExt->Controls->Add(this->fcgCBAudio2pass);
            this->fcgPNAudioExt->Controls->Add(this->fcgCXAudioEncMode);
            this->fcgPNAudioExt->Controls->Add(this->fcgLBAudioEncMode);
            this->fcgPNAudioExt->Controls->Add(this->fcgBTAudioEncoderPath);
            this->fcgPNAudioExt->Controls->Add(this->fcgTXAudioEncoderPath);
            this->fcgPNAudioExt->Controls->Add(this->fcgLBAudioEncoderPath);
            this->fcgPNAudioExt->Controls->Add(this->fcgCBAudioOnly);
            this->fcgPNAudioExt->Controls->Add(this->fcgCXAudioEncoder);
            this->fcgPNAudioExt->Controls->Add(this->fcgLBAudioTemp);
            this->fcgPNAudioExt->Controls->Add(this->fcgLBAudioBitrate);
            this->fcgPNAudioExt->Location = System::Drawing::Point(0, 42);
            this->fcgPNAudioExt->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNAudioExt->Name = L"fcgPNAudioExt";
            this->fcgPNAudioExt->Size = System::Drawing::Size(562, 370);
            this->fcgPNAudioExt->TabIndex = 82;
            // 
            // fcgCXAudioDelayCut
            // 
            this->fcgCXAudioDelayCut->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioDelayCut->FormattingEnabled = true;
            this->fcgCXAudioDelayCut->Location = System::Drawing::Point(438, 161);
            this->fcgCXAudioDelayCut->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXAudioDelayCut->Name = L"fcgCXAudioDelayCut";
            this->fcgCXAudioDelayCut->Size = System::Drawing::Size(102, 30);
            this->fcgCXAudioDelayCut->TabIndex = 65;
            this->fcgCXAudioDelayCut->Tag = L"chValue";
            // 
            // fcgLBAudioDelayCut
            // 
            this->fcgLBAudioDelayCut->AutoSize = true;
            this->fcgLBAudioDelayCut->Location = System::Drawing::Point(337, 166);
            this->fcgLBAudioDelayCut->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBAudioDelayCut->Name = L"fcgLBAudioDelayCut";
            this->fcgLBAudioDelayCut->Size = System::Drawing::Size(93, 22);
            this->fcgLBAudioDelayCut->TabIndex = 75;
            this->fcgLBAudioDelayCut->Text = L"ディレイカット";
            // 
            // fcgCBAudioEncTiming
            // 
            this->fcgCBAudioEncTiming->AutoSize = true;
            this->fcgCBAudioEncTiming->Location = System::Drawing::Point(347, 42);
            this->fcgCBAudioEncTiming->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgCBAudioEncTiming->Name = L"fcgCBAudioEncTiming";
            this->fcgCBAudioEncTiming->Size = System::Drawing::Size(61, 22);
            this->fcgCBAudioEncTiming->TabIndex = 74;
            this->fcgCBAudioEncTiming->Text = L"処理順";
            // 
            // fcgCXAudioEncTiming
            // 
            this->fcgCXAudioEncTiming->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioEncTiming->FormattingEnabled = true;
            this->fcgCXAudioEncTiming->Location = System::Drawing::Point(431, 37);
            this->fcgCXAudioEncTiming->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXAudioEncTiming->Name = L"fcgCXAudioEncTiming";
            this->fcgCXAudioEncTiming->Size = System::Drawing::Size(100, 30);
            this->fcgCXAudioEncTiming->TabIndex = 73;
            this->fcgCXAudioEncTiming->Tag = L"chValue";
            // 
            // fcgCXAudioTempDir
            // 
            this->fcgCXAudioTempDir->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioTempDir->FormattingEnabled = true;
            this->fcgCXAudioTempDir->Location = System::Drawing::Point(204, 274);
            this->fcgCXAudioTempDir->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXAudioTempDir->Name = L"fcgCXAudioTempDir";
            this->fcgCXAudioTempDir->Size = System::Drawing::Size(222, 30);
            this->fcgCXAudioTempDir->TabIndex = 67;
            this->fcgCXAudioTempDir->Tag = L"chValue";
            // 
            // fcgTXCustomAudioTempDir
            // 
            this->fcgTXCustomAudioTempDir->Location = System::Drawing::Point(97, 314);
            this->fcgTXCustomAudioTempDir->Margin = System::Windows::Forms::Padding(5);
            this->fcgTXCustomAudioTempDir->Name = L"fcgTXCustomAudioTempDir";
            this->fcgTXCustomAudioTempDir->Size = System::Drawing::Size(365, 28);
            this->fcgTXCustomAudioTempDir->TabIndex = 68;
            this->fcgTXCustomAudioTempDir->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXCustomAudioTempDir_TextChanged);
            // 
            // fcgBTCustomAudioTempDir
            // 
            this->fcgBTCustomAudioTempDir->Location = System::Drawing::Point(475, 312);
            this->fcgBTCustomAudioTempDir->Margin = System::Windows::Forms::Padding(5);
            this->fcgBTCustomAudioTempDir->Name = L"fcgBTCustomAudioTempDir";
            this->fcgBTCustomAudioTempDir->Size = System::Drawing::Size(43, 35);
            this->fcgBTCustomAudioTempDir->TabIndex = 70;
            this->fcgBTCustomAudioTempDir->Text = L"...";
            this->fcgBTCustomAudioTempDir->UseVisualStyleBackColor = true;
            this->fcgBTCustomAudioTempDir->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTCustomAudioTempDir_Click);
            // 
            // fcgCBAudioUsePipe
            // 
            this->fcgCBAudioUsePipe->AutoSize = true;
            this->fcgCBAudioUsePipe->Location = System::Drawing::Point(197, 162);
            this->fcgCBAudioUsePipe->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBAudioUsePipe->Name = L"fcgCBAudioUsePipe";
            this->fcgCBAudioUsePipe->Size = System::Drawing::Size(109, 26);
            this->fcgCBAudioUsePipe->TabIndex = 64;
            this->fcgCBAudioUsePipe->Tag = L"chValue";
            this->fcgCBAudioUsePipe->Text = L"パイプ処理";
            this->fcgCBAudioUsePipe->UseVisualStyleBackColor = true;
            // 
            // fcgNUAudioBitrate
            // 
            this->fcgNUAudioBitrate->Location = System::Drawing::Point(319, 197);
            this->fcgNUAudioBitrate->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUAudioBitrate->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1536, 0, 0, 0 });
            this->fcgNUAudioBitrate->Name = L"fcgNUAudioBitrate";
            this->fcgNUAudioBitrate->Size = System::Drawing::Size(97, 28);
            this->fcgNUAudioBitrate->TabIndex = 62;
            this->fcgNUAudioBitrate->Tag = L"chValue";
            this->fcgNUAudioBitrate->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCBAudio2pass
            // 
            this->fcgCBAudio2pass->AutoSize = true;
            this->fcgCBAudio2pass->Location = System::Drawing::Point(90, 162);
            this->fcgCBAudio2pass->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBAudio2pass->Name = L"fcgCBAudio2pass";
            this->fcgCBAudio2pass->Size = System::Drawing::Size(83, 26);
            this->fcgCBAudio2pass->TabIndex = 63;
            this->fcgCBAudio2pass->Tag = L"chValue";
            this->fcgCBAudio2pass->Text = L"2pass";
            this->fcgCBAudio2pass->UseVisualStyleBackColor = true;
            this->fcgCBAudio2pass->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgCBAudio2pass_CheckedChanged);
            // 
            // fcgCXAudioEncMode
            // 
            this->fcgCXAudioEncMode->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioEncMode->FormattingEnabled = true;
            this->fcgCXAudioEncMode->Location = System::Drawing::Point(25, 194);
            this->fcgCXAudioEncMode->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXAudioEncMode->Name = L"fcgCXAudioEncMode";
            this->fcgCXAudioEncMode->Size = System::Drawing::Size(281, 30);
            this->fcgCXAudioEncMode->TabIndex = 61;
            this->fcgCXAudioEncMode->Tag = L"chValue";
            this->fcgCXAudioEncMode->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXAudioEncMode_SelectedIndexChanged);
            // 
            // fcgLBAudioEncMode
            // 
            this->fcgLBAudioEncMode->AutoSize = true;
            this->fcgLBAudioEncMode->Location = System::Drawing::Point(7, 166);
            this->fcgLBAudioEncMode->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBAudioEncMode->Name = L"fcgLBAudioEncMode";
            this->fcgLBAudioEncMode->Size = System::Drawing::Size(49, 22);
            this->fcgLBAudioEncMode->TabIndex = 69;
            this->fcgLBAudioEncMode->Text = L"モード";
            // 
            // fcgBTAudioEncoderPath
            // 
            this->fcgBTAudioEncoderPath->Location = System::Drawing::Point(487, 96);
            this->fcgBTAudioEncoderPath->Margin = System::Windows::Forms::Padding(5);
            this->fcgBTAudioEncoderPath->Name = L"fcgBTAudioEncoderPath";
            this->fcgBTAudioEncoderPath->Size = System::Drawing::Size(46, 35);
            this->fcgBTAudioEncoderPath->TabIndex = 60;
            this->fcgBTAudioEncoderPath->Text = L"...";
            this->fcgBTAudioEncoderPath->UseVisualStyleBackColor = true;
            this->fcgBTAudioEncoderPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTAudioEncoderPath_Click);
            // 
            // fcgTXAudioEncoderPath
            // 
            this->fcgTXAudioEncoderPath->AllowDrop = true;
            this->fcgTXAudioEncoderPath->Location = System::Drawing::Point(25, 98);
            this->fcgTXAudioEncoderPath->Margin = System::Windows::Forms::Padding(5);
            this->fcgTXAudioEncoderPath->Name = L"fcgTXAudioEncoderPath";
            this->fcgTXAudioEncoderPath->Size = System::Drawing::Size(453, 28);
            this->fcgTXAudioEncoderPath->TabIndex = 59;
            this->fcgTXAudioEncoderPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXAudioEncoderPath_TextChanged);
            this->fcgTXAudioEncoderPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXAudioEncoderPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            this->fcgTXAudioEncoderPath->Enter += gcnew System::EventHandler(this, &frmConfig::fcgTXAudioEncoderPath_Enter);
            this->fcgTXAudioEncoderPath->Leave += gcnew System::EventHandler(this, &frmConfig::fcgTXAudioEncoderPath_Leave);
            // 
            // fcgLBAudioEncoderPath
            // 
            this->fcgLBAudioEncoderPath->AutoSize = true;
            this->fcgLBAudioEncoderPath->Location = System::Drawing::Point(19, 73);
            this->fcgLBAudioEncoderPath->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBAudioEncoderPath->Name = L"fcgLBAudioEncoderPath";
            this->fcgLBAudioEncoderPath->Size = System::Drawing::Size(75, 22);
            this->fcgLBAudioEncoderPath->TabIndex = 66;
            this->fcgLBAudioEncoderPath->Text = L"～の指定";
            // 
            // fcgCBAudioOnly
            // 
            this->fcgCBAudioOnly->AutoSize = true;
            this->fcgCBAudioOnly->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
            this->fcgCBAudioOnly->Location = System::Drawing::Point(382, 5);
            this->fcgCBAudioOnly->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBAudioOnly->Name = L"fcgCBAudioOnly";
            this->fcgCBAudioOnly->Size = System::Drawing::Size(133, 26);
            this->fcgCBAudioOnly->TabIndex = 57;
            this->fcgCBAudioOnly->Tag = L"chValue";
            this->fcgCBAudioOnly->Text = L"音声のみ出力";
            this->fcgCBAudioOnly->UseVisualStyleBackColor = true;
            // 
            // fcgCXAudioEncoder
            // 
            this->fcgCXAudioEncoder->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioEncoder->FormattingEnabled = true;
            this->fcgCXAudioEncoder->Location = System::Drawing::Point(26, 12);
            this->fcgCXAudioEncoder->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXAudioEncoder->Name = L"fcgCXAudioEncoder";
            this->fcgCXAudioEncoder->Size = System::Drawing::Size(256, 30);
            this->fcgCXAudioEncoder->TabIndex = 55;
            this->fcgCXAudioEncoder->Tag = L"chValue";
            this->fcgCXAudioEncoder->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXAudioEncoder_SelectedIndexChanged);
            // 
            // fcgLBAudioTemp
            // 
            this->fcgLBAudioTemp->AutoSize = true;
            this->fcgLBAudioTemp->Location = System::Drawing::Point(12, 277);
            this->fcgLBAudioTemp->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBAudioTemp->Name = L"fcgLBAudioTemp";
            this->fcgLBAudioTemp->Size = System::Drawing::Size(176, 22);
            this->fcgLBAudioTemp->TabIndex = 72;
            this->fcgLBAudioTemp->Text = L"音声一時ファイル出力先";
            // 
            // fcgLBAudioBitrate
            // 
            this->fcgLBAudioBitrate->AutoSize = true;
            this->fcgLBAudioBitrate->Location = System::Drawing::Point(427, 203);
            this->fcgLBAudioBitrate->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBAudioBitrate->Name = L"fcgLBAudioBitrate";
            this->fcgLBAudioBitrate->Size = System::Drawing::Size(47, 22);
            this->fcgLBAudioBitrate->TabIndex = 71;
            this->fcgLBAudioBitrate->Text = L"kbps";
            // 
            // fcgPNAudioInternal
            // 
            this->fcgPNAudioInternal->Controls->Add(this->fcgLBAudioBitrateInternal);
            this->fcgPNAudioInternal->Controls->Add(this->fcgNUAudioBitrateInternal);
            this->fcgPNAudioInternal->Controls->Add(this->fcgCXAudioEncModeInternal);
            this->fcgPNAudioInternal->Controls->Add(this->fcgLBAudioEncModeInternal);
            this->fcgPNAudioInternal->Controls->Add(this->fcgCXAudioEncoderInternal);
            this->fcgPNAudioInternal->Location = System::Drawing::Point(0, 42);
            this->fcgPNAudioInternal->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNAudioInternal->Name = L"fcgPNAudioInternal";
            this->fcgPNAudioInternal->Size = System::Drawing::Size(562, 370);
            this->fcgPNAudioInternal->TabIndex = 81;
            // 
            // fcgLBAudioBitrateInternal
            // 
            this->fcgLBAudioBitrateInternal->AutoSize = true;
            this->fcgLBAudioBitrateInternal->Location = System::Drawing::Point(427, 103);
            this->fcgLBAudioBitrateInternal->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBAudioBitrateInternal->Name = L"fcgLBAudioBitrateInternal";
            this->fcgLBAudioBitrateInternal->Size = System::Drawing::Size(47, 22);
            this->fcgLBAudioBitrateInternal->TabIndex = 76;
            this->fcgLBAudioBitrateInternal->Text = L"kbps";
            // 
            // fcgNUAudioBitrateInternal
            // 
            this->fcgNUAudioBitrateInternal->Location = System::Drawing::Point(322, 101);
            this->fcgNUAudioBitrateInternal->Margin = System::Windows::Forms::Padding(5);
            this->fcgNUAudioBitrateInternal->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1536, 0, 0, 0 });
            this->fcgNUAudioBitrateInternal->Name = L"fcgNUAudioBitrateInternal";
            this->fcgNUAudioBitrateInternal->Size = System::Drawing::Size(97, 28);
            this->fcgNUAudioBitrateInternal->TabIndex = 74;
            this->fcgNUAudioBitrateInternal->Tag = L"chValue";
            this->fcgNUAudioBitrateInternal->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCXAudioEncModeInternal
            // 
            this->fcgCXAudioEncModeInternal->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioEncModeInternal->FormattingEnabled = true;
            this->fcgCXAudioEncModeInternal->Location = System::Drawing::Point(26, 98);
            this->fcgCXAudioEncModeInternal->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXAudioEncModeInternal->Name = L"fcgCXAudioEncModeInternal";
            this->fcgCXAudioEncModeInternal->Size = System::Drawing::Size(281, 30);
            this->fcgCXAudioEncModeInternal->TabIndex = 73;
            this->fcgCXAudioEncModeInternal->Tag = L"chValue";
            this->fcgCXAudioEncModeInternal->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXAudioEncModeInternal_SelectedIndexChanged);
            // 
            // fcgLBAudioEncModeInternal
            // 
            this->fcgLBAudioEncModeInternal->AutoSize = true;
            this->fcgLBAudioEncModeInternal->Location = System::Drawing::Point(13, 61);
            this->fcgLBAudioEncModeInternal->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBAudioEncModeInternal->Name = L"fcgLBAudioEncModeInternal";
            this->fcgLBAudioEncModeInternal->Size = System::Drawing::Size(49, 22);
            this->fcgLBAudioEncModeInternal->TabIndex = 75;
            this->fcgLBAudioEncModeInternal->Text = L"モード";
            // 
            // fcgCXAudioEncoderInternal
            // 
            this->fcgCXAudioEncoderInternal->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioEncoderInternal->FormattingEnabled = true;
            this->fcgCXAudioEncoderInternal->Location = System::Drawing::Point(26, 12);
            this->fcgCXAudioEncoderInternal->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXAudioEncoderInternal->Name = L"fcgCXAudioEncoderInternal";
            this->fcgCXAudioEncoderInternal->Size = System::Drawing::Size(256, 30);
            this->fcgCXAudioEncoderInternal->TabIndex = 70;
            this->fcgCXAudioEncoderInternal->Tag = L"chValue";
            this->fcgCXAudioEncoderInternal->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXAudioEncoderInternal_SelectedIndexChanged);
            // 
            // fcgtabPageAudioOther
            // 
            this->fcgtabPageAudioOther->Controls->Add(this->panel2);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgLBBatAfterAudioString);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgLBBatBeforeAudioString);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgBTBatAfterAudioPath);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgTXBatAfterAudioPath);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgLBBatAfterAudioPath);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgCBRunBatAfterAudio);
            this->fcgtabPageAudioOther->Controls->Add(this->panel1);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgBTBatBeforeAudioPath);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgTXBatBeforeAudioPath);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgLBBatBeforeAudioPath);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgCBRunBatBeforeAudio);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgCXAudioPriority);
            this->fcgtabPageAudioOther->Controls->Add(this->fcgLBAudioPriority);
            this->fcgtabPageAudioOther->Location = System::Drawing::Point(4, 31);
            this->fcgtabPageAudioOther->Margin = System::Windows::Forms::Padding(5);
            this->fcgtabPageAudioOther->Name = L"fcgtabPageAudioOther";
            this->fcgtabPageAudioOther->Padding = System::Windows::Forms::Padding(5);
            this->fcgtabPageAudioOther->Size = System::Drawing::Size(562, 419);
            this->fcgtabPageAudioOther->TabIndex = 1;
            this->fcgtabPageAudioOther->Text = L"その他";
            this->fcgtabPageAudioOther->UseVisualStyleBackColor = true;
            // 
            // panel2
            // 
            this->panel2->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->panel2->Location = System::Drawing::Point(23, 203);
            this->panel2->Margin = System::Windows::Forms::Padding(5);
            this->panel2->Name = L"panel2";
            this->panel2->Size = System::Drawing::Size(512, 1);
            this->panel2->TabIndex = 73;
            // 
            // fcgLBBatAfterAudioString
            // 
            this->fcgLBBatAfterAudioString->AutoSize = true;
            this->fcgLBBatAfterAudioString->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Italic | System::Drawing::FontStyle::Underline)),
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(128)));
            this->fcgLBBatAfterAudioString->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
            this->fcgLBBatAfterAudioString->Location = System::Drawing::Point(451, 325);
            this->fcgLBBatAfterAudioString->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBBatAfterAudioString->Name = L"fcgLBBatAfterAudioString";
            this->fcgLBBatAfterAudioString->Size = System::Drawing::Size(40, 23);
            this->fcgLBBatAfterAudioString->TabIndex = 72;
            this->fcgLBBatAfterAudioString->Text = L" 後& ";
            this->fcgLBBatAfterAudioString->TextAlign = System::Drawing::ContentAlignment::TopCenter;
            // 
            // fcgLBBatBeforeAudioString
            // 
            this->fcgLBBatBeforeAudioString->AutoSize = true;
            this->fcgLBBatBeforeAudioString->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Italic | System::Drawing::FontStyle::Underline)),
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(128)));
            this->fcgLBBatBeforeAudioString->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
            this->fcgLBBatBeforeAudioString->Location = System::Drawing::Point(451, 222);
            this->fcgLBBatBeforeAudioString->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBBatBeforeAudioString->Name = L"fcgLBBatBeforeAudioString";
            this->fcgLBBatBeforeAudioString->Size = System::Drawing::Size(40, 23);
            this->fcgLBBatBeforeAudioString->TabIndex = 64;
            this->fcgLBBatBeforeAudioString->Text = L" 前& ";
            this->fcgLBBatBeforeAudioString->TextAlign = System::Drawing::ContentAlignment::TopCenter;
            // 
            // fcgBTBatAfterAudioPath
            // 
            this->fcgBTBatAfterAudioPath->Location = System::Drawing::Point(491, 360);
            this->fcgBTBatAfterAudioPath->Margin = System::Windows::Forms::Padding(5);
            this->fcgBTBatAfterAudioPath->Name = L"fcgBTBatAfterAudioPath";
            this->fcgBTBatAfterAudioPath->Size = System::Drawing::Size(46, 35);
            this->fcgBTBatAfterAudioPath->TabIndex = 71;
            this->fcgBTBatAfterAudioPath->Tag = L"chValue";
            this->fcgBTBatAfterAudioPath->Text = L"...";
            this->fcgBTBatAfterAudioPath->UseVisualStyleBackColor = true;
            this->fcgBTBatAfterAudioPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTBatAfterAudioPath_Click);
            // 
            // fcgTXBatAfterAudioPath
            // 
            this->fcgTXBatAfterAudioPath->AllowDrop = true;
            this->fcgTXBatAfterAudioPath->Location = System::Drawing::Point(185, 361);
            this->fcgTXBatAfterAudioPath->Margin = System::Windows::Forms::Padding(5);
            this->fcgTXBatAfterAudioPath->Name = L"fcgTXBatAfterAudioPath";
            this->fcgTXBatAfterAudioPath->Size = System::Drawing::Size(302, 28);
            this->fcgTXBatAfterAudioPath->TabIndex = 70;
            this->fcgTXBatAfterAudioPath->Tag = L"chValue";
            // 
            // fcgLBBatAfterAudioPath
            // 
            this->fcgLBBatAfterAudioPath->AutoSize = true;
            this->fcgLBBatAfterAudioPath->Location = System::Drawing::Point(55, 367);
            this->fcgLBBatAfterAudioPath->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBBatAfterAudioPath->Name = L"fcgLBBatAfterAudioPath";
            this->fcgLBBatAfterAudioPath->Size = System::Drawing::Size(94, 22);
            this->fcgLBBatAfterAudioPath->TabIndex = 69;
            this->fcgLBBatAfterAudioPath->Text = L"バッチファイル";
            // 
            // fcgCBRunBatAfterAudio
            // 
            this->fcgCBRunBatAfterAudio->AutoSize = true;
            this->fcgCBRunBatAfterAudio->Location = System::Drawing::Point(23, 324);
            this->fcgCBRunBatAfterAudio->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBRunBatAfterAudio->Name = L"fcgCBRunBatAfterAudio";
            this->fcgCBRunBatAfterAudio->Size = System::Drawing::Size(308, 26);
            this->fcgCBRunBatAfterAudio->TabIndex = 68;
            this->fcgCBRunBatAfterAudio->Tag = L"chValue";
            this->fcgCBRunBatAfterAudio->Text = L"音声エンコード終了後、バッチ処理を行う";
            this->fcgCBRunBatAfterAudio->UseVisualStyleBackColor = true;
            // 
            // panel1
            // 
            this->panel1->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->panel1->Location = System::Drawing::Point(23, 307);
            this->panel1->Margin = System::Windows::Forms::Padding(5);
            this->panel1->Name = L"panel1";
            this->panel1->Size = System::Drawing::Size(512, 1);
            this->panel1->TabIndex = 67;
            // 
            // fcgBTBatBeforeAudioPath
            // 
            this->fcgBTBatBeforeAudioPath->Location = System::Drawing::Point(491, 259);
            this->fcgBTBatBeforeAudioPath->Margin = System::Windows::Forms::Padding(5);
            this->fcgBTBatBeforeAudioPath->Name = L"fcgBTBatBeforeAudioPath";
            this->fcgBTBatBeforeAudioPath->Size = System::Drawing::Size(46, 35);
            this->fcgBTBatBeforeAudioPath->TabIndex = 66;
            this->fcgBTBatBeforeAudioPath->Tag = L"chValue";
            this->fcgBTBatBeforeAudioPath->Text = L"...";
            this->fcgBTBatBeforeAudioPath->UseVisualStyleBackColor = true;
            this->fcgBTBatBeforeAudioPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTBatBeforeAudioPath_Click);
            // 
            // fcgTXBatBeforeAudioPath
            // 
            this->fcgTXBatBeforeAudioPath->AllowDrop = true;
            this->fcgTXBatBeforeAudioPath->Location = System::Drawing::Point(185, 259);
            this->fcgTXBatBeforeAudioPath->Margin = System::Windows::Forms::Padding(5);
            this->fcgTXBatBeforeAudioPath->Name = L"fcgTXBatBeforeAudioPath";
            this->fcgTXBatBeforeAudioPath->Size = System::Drawing::Size(302, 28);
            this->fcgTXBatBeforeAudioPath->TabIndex = 65;
            this->fcgTXBatBeforeAudioPath->Tag = L"chValue";
            // 
            // fcgLBBatBeforeAudioPath
            // 
            this->fcgLBBatBeforeAudioPath->AutoSize = true;
            this->fcgLBBatBeforeAudioPath->Location = System::Drawing::Point(55, 264);
            this->fcgLBBatBeforeAudioPath->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBBatBeforeAudioPath->Name = L"fcgLBBatBeforeAudioPath";
            this->fcgLBBatBeforeAudioPath->Size = System::Drawing::Size(94, 22);
            this->fcgLBBatBeforeAudioPath->TabIndex = 63;
            this->fcgLBBatBeforeAudioPath->Text = L"バッチファイル";
            // 
            // fcgCBRunBatBeforeAudio
            // 
            this->fcgCBRunBatBeforeAudio->AutoSize = true;
            this->fcgCBRunBatBeforeAudio->Location = System::Drawing::Point(23, 222);
            this->fcgCBRunBatBeforeAudio->Margin = System::Windows::Forms::Padding(5);
            this->fcgCBRunBatBeforeAudio->Name = L"fcgCBRunBatBeforeAudio";
            this->fcgCBRunBatBeforeAudio->Size = System::Drawing::Size(308, 26);
            this->fcgCBRunBatBeforeAudio->TabIndex = 62;
            this->fcgCBRunBatBeforeAudio->Tag = L"chValue";
            this->fcgCBRunBatBeforeAudio->Text = L"音声エンコード開始前、バッチ処理を行う";
            this->fcgCBRunBatBeforeAudio->UseVisualStyleBackColor = true;
            // 
            // fcgCXAudioPriority
            // 
            this->fcgCXAudioPriority->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioPriority->FormattingEnabled = true;
            this->fcgCXAudioPriority->Location = System::Drawing::Point(252, 29);
            this->fcgCXAudioPriority->Margin = System::Windows::Forms::Padding(5);
            this->fcgCXAudioPriority->Name = L"fcgCXAudioPriority";
            this->fcgCXAudioPriority->Size = System::Drawing::Size(202, 30);
            this->fcgCXAudioPriority->TabIndex = 13;
            this->fcgCXAudioPriority->Tag = L"chValue";
            // 
            // fcgLBAudioPriority
            // 
            this->fcgLBAudioPriority->AutoSize = true;
            this->fcgLBAudioPriority->Location = System::Drawing::Point(61, 34);
            this->fcgLBAudioPriority->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
            this->fcgLBAudioPriority->Name = L"fcgLBAudioPriority";
            this->fcgLBAudioPriority->Size = System::Drawing::Size(95, 22);
            this->fcgLBAudioPriority->TabIndex = 12;
            this->fcgLBAudioPriority->Text = L"音声優先度";
            // 
            // fcgPNHideToolStripBorder
            // 
            this->fcgPNHideToolStripBorder->Location = System::Drawing::Point(0, 34);
            this->fcgPNHideToolStripBorder->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNHideToolStripBorder->Name = L"fcgPNHideToolStripBorder";
            this->fcgPNHideToolStripBorder->Size = System::Drawing::Size(1530, 6);
            this->fcgPNHideToolStripBorder->TabIndex = 52;
            this->fcgPNHideToolStripBorder->Visible = false;
            // 
            // fcgPNH264LevelProfile
            // 
            this->fcgPNH264LevelProfile->Controls->Add(this->fcgLBCodecProfile);
            this->fcgPNH264LevelProfile->Controls->Add(this->fcgLBCodecLevel);
            this->fcgPNH264LevelProfile->Controls->Add(this->fcgCXCodecProfile);
            this->fcgPNH264LevelProfile->Controls->Add(this->fcgCXCodecLevel);
            this->fcgPNH264LevelProfile->Location = System::Drawing::Point(520, 200);
            this->fcgPNH264LevelProfile->Margin = System::Windows::Forms::Padding(5);
            this->fcgPNH264LevelProfile->Name = L"fcgPNH264LevelProfile";
            this->fcgPNH264LevelProfile->Size = System::Drawing::Size(383, 95);
            this->fcgPNH264LevelProfile->TabIndex = 148;
            // 
            // frmConfig
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(144, 144);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Dpi;
            this->ClientSize = System::Drawing::Size(1512, 901);
            this->Controls->Add(this->fcgPNHideToolStripBorder);
            this->Controls->Add(this->fcgtabControlAudio);
            this->Controls->Add(this->fcgLBguiExBlog);
            this->Controls->Add(this->fcgtabControlMux);
            this->Controls->Add(this->fcgtabControlVCE);
            this->Controls->Add(this->fcgLBVersion);
            this->Controls->Add(this->fcgLBVersionDate);
            this->Controls->Add(this->fcgBTDefault);
            this->Controls->Add(this->fcgBTOK);
            this->Controls->Add(this->fcgBTCancel);
            this->Controls->Add(this->fcgTXCmd);
            this->Controls->Add(this->fcgtoolStripSettings);
            this->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->Margin = System::Windows::Forms::Padding(5);
            this->MaximizeBox = false;
            this->Name = L"frmConfig";
            this->ShowIcon = false;
            this->Text = L"Aviutl 出力 プラグイン";
            this->Load += gcnew System::EventHandler(this, &frmConfig::frmConfig_Load);
            this->fcgtoolStripSettings->ResumeLayout(false);
            this->fcgtoolStripSettings->PerformLayout();
            this->fcgtabControlMux->ResumeLayout(false);
            this->fcgtabPageMP4->ResumeLayout(false);
            this->fcgtabPageMP4->PerformLayout();
            this->fcgtabPageMKV->ResumeLayout(false);
            this->fcgtabPageMKV->PerformLayout();
            this->fcgtabPageMux->ResumeLayout(false);
            this->fcgtabPageMux->PerformLayout();
            this->fcgtabPageBat->ResumeLayout(false);
            this->fcgtabPageBat->PerformLayout();
            this->fcgtabPageInternal->ResumeLayout(false);
            this->fcgtabPageInternal->PerformLayout();
            this->fcgtabControlVCE->ResumeLayout(false);
            this->tabPageVideoEnc->ResumeLayout(false);
            this->tabPageVideoEnc->PerformLayout();
            this->fcgPNQVBR->ResumeLayout(false);
            this->fcgPNQVBR->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQVBRQuality))->EndInit();
            this->fcgPNHEVCLevelProfile->ResumeLayout(false);
            this->fcgPNHEVCLevelProfile->PerformLayout();
            this->fcgPNAV1LevelProfile->ResumeLayout(false);
            this->fcgPNAV1LevelProfile->PerformLayout();
            this->fcggroupBoxColorMatrix->ResumeLayout(false);
            this->fcggroupBoxColorMatrix->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNURefFrames))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVBVBufSize))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQPMax))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQPMin))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgpictureBoxVCEEnabled))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgpictureBoxVCEDisabled))->EndInit();
            this->fcgGroupBoxAspectRatio->ResumeLayout(false);
            this->fcgGroupBoxAspectRatio->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAspectRatioY))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAspectRatioX))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUGopLength))->EndInit();
            this->fcgPNBframes->ResumeLayout(false);
            this->fcgPNBframes->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUBframes))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUBDeltaQP))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUBRefDeltaQP))->EndInit();
            this->fcgPNH264Features->ResumeLayout(false);
            this->fcgPNH264Features->PerformLayout();
            this->fcgPNBitrate->ResumeLayout(false);
            this->fcgPNBitrate->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUBitrate))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUMaxkbps))->EndInit();
            this->fcgPNQP->ResumeLayout(false);
            this->fcgPNQP->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQPI))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQPP))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUQPB))->EndInit();
            this->tabPageVideoEnc2->ResumeLayout(false);
            this->tabPageVideoEnc2->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUSlices))->EndInit();
            this->fcggroupBoxPreAnalysis->ResumeLayout(false);
            this->fcggroupBoxPreAnalysis->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUPALookahead))->EndInit();
            this->tabPageVpp->ResumeLayout(false);
            this->tabPageVpp->PerformLayout();
            this->fcggroupBoxVppDeband->ResumeLayout(false);
            this->fcgPNVppLibplaceboDeband->ResumeLayout(false);
            this->fcgPNVppLibplaceboDeband->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppLibplaceboDebandRadius))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppLibplaceboDebandThreshold))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppLibplaceboDebandGrainC))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppLibplaceboDebandGrainY))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppLibplaceboDebandIteration))->EndInit();
            this->fcgPNVppDeband->ResumeLayout(false);
            this->fcgPNVppDeband->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDebandDitherC))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDebandDitherY))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDebandThreCr))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDebandThreCb))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDebandThreY))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDebandRange))->EndInit();
            this->fcggroupBoxVppDetailEnahance->ResumeLayout(false);
            this->fcgPNVppWarpsharp->ResumeLayout(false);
            this->fcgPNVppWarpsharp->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppWarpsharpDepth))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppWarpsharpThreshold))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppWarpsharpType))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppWarpsharpBlur))->EndInit();
            this->fcgPNVppDetailEnhance->ResumeLayout(false);
            this->fcgPNVppDetailEnhance->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDetailEnhanceRadius))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDetailEnhanceAttenuation))->EndInit();
            this->fcgPNVppEdgelevel->ResumeLayout(false);
            this->fcgPNVppEdgelevel->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppEdgelevelWhite))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppEdgelevelThreshold))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppEdgelevelBlack))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppEdgelevelStrength))->EndInit();
            this->fcgPNVppUnsharp->ResumeLayout(false);
            this->fcgPNVppUnsharp->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppUnsharpThreshold))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppUnsharpWeight))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppUnsharpRadius))->EndInit();
            this->fcggroupBoxVppDenoise->ResumeLayout(false);
            this->fcgPNVppDenoiseFFT3D->ResumeLayout(false);
            this->fcgPNVppDenoiseFFT3D->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseFFT3DOverlap))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseFFT3DAmount))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseFFT3DSigma))->EndInit();
            this->fcgPNVppDenoiseNLMeans->ResumeLayout(false);
            this->fcgPNVppDenoiseNLMeans->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseNLMeansH))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseNLMeansSigma))->EndInit();
            this->fcgPNVppDenoiseDct->ResumeLayout(false);
            this->fcgPNVppDenoiseDct->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseDctSigma))->EndInit();
            this->fcgPNVppPreProcess->ResumeLayout(false);
            this->fcgPNVppPreProcess->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppPreProcessSensitivity))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppPreProcessStrength))->EndInit();
            this->fcgPNVppDenoisePmd->ResumeLayout(false);
            this->fcgPNVppDenoisePmd->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoisePmdThreshold))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoisePmdStrength))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoisePmdApplyCount))->EndInit();
            this->fcgPNVppDenoiseConv3D->ResumeLayout(false);
            this->fcgPNVppDenoiseConv3D->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseConv3DThreshCTemporal))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseConv3DThreshCSpatial))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseConv3DThreshYTemporal))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseConv3DThreshYSpatial))->EndInit();
            this->fcgPNVppDenoiseSmooth->ResumeLayout(false);
            this->fcgPNVppDenoiseSmooth->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseSmoothQP))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseSmoothQuality))->EndInit();
            this->fcgPNVppDenoiseKnn->ResumeLayout(false);
            this->fcgPNVppDenoiseKnn->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseKnnThreshold))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseKnnStrength))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDenoiseKnnRadius))->EndInit();
            this->fcggroupBoxVppDeinterlace->ResumeLayout(false);
            this->fcggroupBoxVppDeinterlace->PerformLayout();
            this->fcgPNVppDecomb->ResumeLayout(false);
            this->fcgPNVppDecomb->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDecombDthreshold))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppDecombThreshold))->EndInit();
            this->fcgPNVppAfs->ResumeLayout(false);
            this->fcgPNVppAfs->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgTBVppAfsThreCMotion))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgTBVppAfsThreYMotion))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgTBVppAfsThreDeint))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgTBVppAfsThreShift))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgTBVppAfsCoeffShift))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsRight))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsLeft))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsBottom))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsUp))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgTBVppAfsMethodSwitch))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsThreCMotion))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsThreShift))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsThreDeint))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsThreYMotion))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsCoeffShift))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUVppAfsMethodSwitch))->EndInit();
            this->fcgPNVppYadif->ResumeLayout(false);
            this->fcgPNVppYadif->PerformLayout();
            this->fcgPNVppNnedi->ResumeLayout(false);
            this->fcgPNVppNnedi->PerformLayout();
            this->fcggroupBoxResize->ResumeLayout(false);
            this->fcggroupBoxResize->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUResizeH))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUResizeW))->EndInit();
            this->tabPageExOpt->ResumeLayout(false);
            this->tabPageExOpt->PerformLayout();
            this->fcggroupBoxCmdEx->ResumeLayout(false);
            this->fcggroupBoxCmdEx->PerformLayout();
            this->fcgCSExeFiles->ResumeLayout(false);
            this->fcgtabControlAudio->ResumeLayout(false);
            this->fcgtabPageAudioMain->ResumeLayout(false);
            this->fcgtabPageAudioMain->PerformLayout();
            this->fcgPNAudioExt->ResumeLayout(false);
            this->fcgPNAudioExt->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAudioBitrate))->EndInit();
            this->fcgPNAudioInternal->ResumeLayout(false);
            this->fcgPNAudioInternal->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->fcgNUAudioBitrateInternal))->EndInit();
            this->fcgtabPageAudioOther->ResumeLayout(false);
            this->fcgtabPageAudioOther->PerformLayout();
            this->fcgPNH264LevelProfile->ResumeLayout(false);
            this->fcgPNH264LevelProfile->PerformLayout();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion
    private:
        const SYSTEM_DATA *sys_dat;
        std::vector<tstring> *list_lng;
        CONF_GUIEX *conf;
        LocalSettings LocalStg;
        DarkenWindowStgReader *dwStgReader;
        AuoTheme themeMode;
        bool CurrentPipeEnabled;
        bool stgChanged;
        String^ CurrentStgDir;
        ToolStripMenuItem^ CheckedStgMenuItem;
        CONF_GUIEX *cnf_stgSelected;
        String^ lastQualityStr;
        VidEncInfo encInfo;
        Task<VidEncInfo>^ taskEncInfo;
        CancellationTokenSource^ taskEncInfoCancell;
    private:
        System::Void CheckTheme();
        System::Void SetAllMouseMove(Control ^top, const AuoTheme themeTo);
        System::Void fcgMouseEnter_SetColor(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgMouseLeave_SetColor(System::Object^  sender, System::EventArgs^  e);
        System::Void TabControl_DarkDrawItem(System::Object^ sender, DrawItemEventArgs^ e);

        System::Void LoadLangText();
        System::Int32 GetCurrentAudioDefaultBitrate();
        delegate System::Void qualityTimerChangeDelegate();
        System::Void InitComboBox();
        System::Void setAudioExtDisplay();
        System::Void AudioExtEncodeModeChanged();
        System::Void setAudioIntDisplay();
        System::Void AudioIntEncodeModeChanged();
        System::Void InitStgFileList();
        System::Void RebuildStgFileDropDown(String^ stgDir);
        System::Void RebuildStgFileDropDown(ToolStripDropDownItem^ TS, String^ dir);
        System::Void SetLocalStg();
        System::Void LoadLocalStg();
        System::Void SaveLocalStg();
        System::Boolean CheckLocalStg();
        System::Void SetTXMaxLen(TextBox^ TX, int max_len);
        System::Void SetTXMaxLenAll();
        System::Void InitForm();
        System::Void ConfToFrm(CONF_GUIEX *cnf);
        System::String^ FrmToConf(CONF_GUIEX *cnf);
        System::Void SetChangedEvent(Control^ control, System::EventHandler^ _event);
        System::Void SetAllCheckChangedEvents(Control ^top);
        System::Void SaveToStgFile(String^ stgName);
        System::Void DeleteStgFile(ToolStripMenuItem^ mItem);
        System::Boolean EnableSettingsNoteChange(bool Enable);
        System::Void fcgTSLSettingsNotes_DoubleClick(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgTSTSettingsNotes_Leave(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgTSTSettingsNotes_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e);
        System::Void fcgTSTSettingsNotes_TextChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void GetfcgTSLSettingsNotes(TCHAR *notes, int nSize);
        System::Void SetfcgTSLSettingsNotes(const TCHAR *notes);
        System::Void SetfcgTSLSettingsNotes(String^ notes);
        System::Void fcgTSBSave_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgTSBSaveNew_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgTSBDelete_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgTSSettings_DropDownItemClicked(System::Object^  sender, System::Windows::Forms::ToolStripItemClickedEventArgs^  e);
        System::Void UncheckAllDropDownItem(ToolStripItem^ mItem);
        ToolStripMenuItem^ fcgTSSettingsSearchItem(String^ stgPath, ToolStripItem^ mItem);
        ToolStripMenuItem^ fcgTSSettingsSearchItem(String^ stgPath);
        System::Void CheckTSSettingsDropDownItem(ToolStripMenuItem^ mItem);
        System::Void CheckTSItemsEnabled(CONF_GUIEX *current_conf);
        System::Void fcgChangeMuxerVisible(System::Object^  sender, System::EventArgs^  e);

        System::Void InitLangList();
        System::Void SaveSelectedLanguage(const TCHAR *language_text);
        System::Void SetSelectedLanguage(const TCHAR *language_text);
        System::Void CheckTSLanguageDropDownItem(ToolStripMenuItem^ mItem);
        System::Void fcgTSLanguage_DropDownItemClicked(System::Object^  sender, System::Windows::Forms::ToolStripItemClickedEventArgs^  e);

        System::Void SetHelpToolTipsColorMatrix(Control^ control, const CX_DESC *list, const wchar_t *type);
        System::Void SetHelpToolTips();
        System::Void ShowExehelp(String^ ExePath, String^ args);
        System::Void fcgTSBOtherSettings_Click(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgChangeEnabled(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgTSBBitrateCalc_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void CloseBitrateCalc();
        System::Void SetfbcBTABEnable(bool enable, int max);
        System::Void SetfbcBTVBEnable(bool enable);
        System::Void fcgCBAudio2pass_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgCXAudioEncoder_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgCXAudioEncMode_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgCXAudioEncoderInternal_SelectedIndexChanged(System::Object ^sender, System::EventArgs ^e);
        System::Void fcgCXAudioEncModeInternal_SelectedIndexChanged(System::Object ^sender, System::EventArgs ^e);
        System::Void fcgCBAudioUseExt_CheckedChanged(System::Object ^sender, System::EventArgs ^e);
        System::Void AdjustLocation();
        System::Void ActivateToolTip(bool Enable);
        System::Void SetStgEscKey(bool Enable);
        System::Void SetToolStripEvents(ToolStrip^ TS, System::Windows::Forms::MouseEventHandler^ _event);
        System::Void fcgCXCodec_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void fcgCBPreAnalysis_CheckedChanged(System::Object^  sender, System::EventArgs^  e);
        System::Void SetVidEncInfo(VidEncInfo info);
        delegate System::Void SetVidEncInfoDelegate(VidEncInfo info);
        VidEncInfo GetVidEncInfo();
        System::Void GetVidEncInfoAsync();
    public:
        System::Void InitData(CONF_GUIEX *set_config, const SYSTEM_DATA *system_data);
        System::Void SetVideoBitrate(int bitrate);
        System::Void SetAudioBitrate(int bitrate);
        System::Void InformfbcClosed();
    private:
        System::Void fcgTSItem_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
            EnableSettingsNoteChange(false);
        }
    private:
        System::Void frmConfig_KeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e) {
            if (e->KeyCode == Keys::Escape)
                this->Close();
            if ((e->KeyData & (Keys::Control | Keys::Shift | Keys::Enter)) == (Keys::Control | Keys::Shift | Keys::Enter))
                fcgBTOK_Click(sender, nullptr);
        }
    private:
        System::Void NUSelectAll(System::Object^  sender, System::EventArgs^  e) {
             NumericUpDown^ NU = dynamic_cast<NumericUpDown^>(sender);
             NU->Select(0, NU->Text->Length);
         }
    private:
        System::Void setComboBox(ComboBox^ CX, const ENC_OPTION_STR * list) {
            CX->BeginUpdate();
            const int prevIdx = CX->SelectedIndex;
            CX->Items->Clear();
            for (int i = 0; list[i].desc; i++) {
                String^ string = nullptr;
                if (list[i].mes != AUO_MES_UNKNOWN) {
                    string = LOAD_CLI_STRING(list[i].mes);
                }
                if (string == nullptr || string->Length == 0) {
                    string = String(list[i].desc).ToString();
                }
                CX->Items->Add(string);
            }
            SetCXIndex(CX, prevIdx);
            CX->EndUpdate();
        }
    private:
        System::Void setComboBox(ComboBox^ CX, const ENC_OPTION_STR2 * list) {
            CX->BeginUpdate();
            const int prevIdx = CX->SelectedIndex;
            CX->Items->Clear();
            for (int i = 0; list[i].desc; i++) {
                String^ string = nullptr;
                if (list[i].mes != AUO_MES_UNKNOWN) {
                    string = LOAD_CLI_STRING(list[i].mes);
                }
                if (string == nullptr || string->Length == 0) {
                    string = String(list[i].desc).ToString();
                }
                CX->Items->Add(string);
            }
            SetCXIndex(CX, prevIdx);
            CX->EndUpdate();
        }
    private:
        System::Void setComboBox(ComboBox^ CX, const CX_DESC * list) {
            CX->BeginUpdate();
            const int prevIdx = CX->SelectedIndex;
            CX->Items->Clear();
            for (int i = 0; list[i].desc; i++)
                CX->Items->Add(String(list[i].desc).ToString());
            SetCXIndex(CX, prevIdx);
            CX->EndUpdate();
        }
    private:
        System::Void setComboBox(ComboBox ^CX, const CX_DESC *list, const TCHAR *ignore) {
            CX->BeginUpdate();
            const int prevIdx = CX->SelectedIndex;
            CX->Items->Clear();
            for (int i = 0; list[i].desc; i++) {
                if (ignore && _tcscmp(ignore, list[i].desc) == 0) {
                    //インデックスの順番を保持するため、途中の場合は"-----"をいれておく
                    if (list[i + 1].desc) {
                        CX->Items->Add(String("-----").ToString());
                    }
                } else {
                    CX->Items->Add(String(list[i].desc).ToString());
                }
            }
            SetCXIndex(CX, prevIdx);
            CX->EndUpdate();
        }
    private:
        System::Void setComboBox(ComboBox^ CX, const CX_DESC * list, int count) {
            CX->BeginUpdate();
            const int prevIdx = CX->SelectedIndex;
            CX->Items->Clear();
            for (int i = 0; list[i].desc && i < count; i++)
                CX->Items->Add(String(list[i].desc).ToString());
            SetCXIndex(CX, prevIdx);
            CX->EndUpdate();
        }
    private:
        System::Void setComboBox(ComboBox^ CX, const char * const * list) {
            CX->BeginUpdate();
            const int prevIdx = CX->SelectedIndex;
            CX->Items->Clear();
            for (int i = 0; list[i]; i++)
                CX->Items->Add(String(list[i]).ToString());
            SetCXIndex(CX, prevIdx);
            CX->EndUpdate();
        }
    private:
        System::Void setComboBox(ComboBox^ CX, const WCHAR * const * list) {
            CX->BeginUpdate();
            const int prevIdx = CX->SelectedIndex;
            CX->Items->Clear();
            for (int i = 0; list[i]; i++)
                CX->Items->Add(String(list[i]).ToString());
            SetCXIndex(CX, prevIdx);
            CX->EndUpdate();
        }
        System::Void SetCXIndex(ComboBox^ CX, const std::optional<bool>& value) {
            int index = -1;
            if (value.has_value()) {
                if (value.value()) {
                    index = get_cx_index(list_option_auto_off_on, L"on");
                } else {
                    index = get_cx_index(list_option_auto_off_on, L"off");
                }
            } else {
                index = get_cx_index(list_option_auto_off_on, L"auto");
            }
            SetCXIndex(CX, index);
        }
        System::Void SetCXIndex(ComboBox^ CX, const std::optional<int>& value) {
            int index = -1;
            if (value.has_value()) {
                if (value.value() != 0) {
                    index = get_cx_index(list_option_auto_off_on, L"on");
                } else {
                    index = get_cx_index(list_option_auto_off_on, L"off");
                }
            } else {
                index = get_cx_index(list_option_auto_off_on, L"auto");
            }
            SetCXIndex(CX, index);
        }
        System::Void SetOptValueCX(std::optional<bool>& value, ComboBox^ CX) {
            const int index = CX->SelectedIndex;
            const int codingopt = list_option_auto_off_on[clamp(index, 0, _countof(list_option_auto_off_on) - 1)].value;
            if (codingopt == get_cx_value(list_option_auto_off_on, L"on")) {
                value = true;
            } else if (codingopt == get_cx_value(list_option_auto_off_on, L"off")) {
                value = false;
            } else {
                value.reset();
            }
        }
        System::Void SetOptValueCX(std::optional<int>& value, ComboBox^ CX) {
            const int index = CX->SelectedIndex;
            const int codingopt = list_option_auto_off_on[clamp(index, 0, _countof(list_option_auto_off_on) - 1)].value;
            if (codingopt == get_cx_value(list_option_auto_off_on, L"auto")) {
                value.reset();
            } else {
                value = codingopt;
            }
        }
    private:
        System::Void setPriorityList(ComboBox^ CX) {
            CX->BeginUpdate();
            const int prevIdx = CX->SelectedIndex;
            CX->Items->Clear();
            for (int i = 0; priority_table[i].text; i++) {
                String^ string = nullptr;
                if (priority_table[i].mes != AUO_MES_UNKNOWN) {
                    string = LOAD_CLI_STRING(priority_table[i].mes);
                }
                if (string == nullptr || string->Length == 0) {
                    string = String(priority_table[i].text).ToString();
                }
                CX->Items->Add(string);
            }
            SetCXIndex(CX, prevIdx);
            CX->EndUpdate();
        }
    private:
        System::Void setMuxerCmdExNames(ComboBox^ CX, int muxer_index) {
            CX->BeginUpdate();
            const int prevIdx = CX->SelectedIndex;
            CX->Items->Clear();
            MUXER_SETTINGS *mstg = &sys_dat->exstg->s_mux[muxer_index];
            for (int i = 0; i < mstg->ex_count; i++)
                CX->Items->Add(String(mstg->ex_cmd[i].name).ToString());
            SetCXIndex(CX, prevIdx);
            CX->EndUpdate();
        }
    private:
        System::Void setAudioEncoderNames() {
            fcgCXAudioEncoder->BeginUpdate();
            const int prevIdx = fcgCXAudioEncoder->SelectedIndex;
            fcgCXAudioEncoder->Items->Clear();
            //fcgCXAudioEncoder->Items->AddRange(reinterpret_cast<array<String^>^>(LocalStg.audEncName->ToArray(String::typeid)));
            fcgCXAudioEncoder->Items->AddRange(LocalStg.audEncName->ToArray());
            SetCXIndex(fcgCXAudioEncoder, prevIdx);
            fcgCXAudioEncoder->EndUpdate();

            fcgCXAudioEncoderInternal->BeginUpdate();
            const int prevIdxInternal = fcgCXAudioEncoderInternal->SelectedIndex;
            fcgCXAudioEncoderInternal->Items->Clear();
            for (int i = 0; i < sys_dat->exstg->s_aud_int_count; i++) {
                fcgCXAudioEncoderInternal->Items->Add(String(sys_dat->exstg->s_aud_int[i].dispname).ToString());
            }
            SetCXIndex(fcgCXAudioEncoderInternal, prevIdxInternal);
            fcgCXAudioEncoderInternal->EndUpdate();
        }
    private:
        System::Void TX_LimitbyBytes(System::Object^  sender, System::ComponentModel::CancelEventArgs^ e) {
            int maxLength = 0;
            int stringBytes = 0;
            TextBox^ TX = nullptr;
            if ((TX = dynamic_cast<TextBox^>(sender)) == nullptr)
                return;
            stringBytes = CountStringBytes(TX->Text);
            maxLength = TX->MaxLength;
            if (stringBytes > maxLength - 1) {
                e->Cancel = true;
                MessageBox::Show(this, LOAD_CLI_STRING(AUO_CONFIG_TEXT_LIMIT_LENGTH), LOAD_CLI_STRING(AUO_GUIEX_ERROR), MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }
    private:
        System::Boolean openExeFile(TextBox^ TX, String^ ExeName) {
            //WinXPにおいて、OpenFileDialogはCurrentDirctoryを勝手に変更しやがるので、
            //一度保存し、あとから再適用する
            String^ CurrentDir = Directory::GetCurrentDirectory();
            OpenFileDialog ofd;
            ofd.Multiselect = false;
            ofd.FileName = ExeName;
            ofd.Filter = MakeExeFilter(ExeName);
            if (Directory::Exists(LocalStg.LastAppDir))
                ofd.InitialDirectory = Path::GetFullPath(LocalStg.LastAppDir);
            else if (File::Exists(TX->Text))
                ofd.InitialDirectory = Path::GetFullPath(Path::GetDirectoryName(TX->Text));
            else
                ofd.InitialDirectory = String(sys_dat->aviutl_dir).ToString();
            bool ret = (ofd.ShowDialog() == System::Windows::Forms::DialogResult::OK);
            if (ret) {
                if (sys_dat->exstg->s_local.get_relative_path)
                    ofd.FileName = GetRelativePath(ofd.FileName, CurrentDir);
                LocalStg.LastAppDir = Path::GetDirectoryName(Path::GetFullPath(ofd.FileName));
                TX->Text = ofd.FileName;
                TX->SelectionStart = TX->Text->Length;
            }
            Directory::SetCurrentDirectory(CurrentDir);
            return ret;
        }
    private:
        System::Void frmConfig::ExeTXPathEnter() {
            fcgTXVideoEncoderPath_Enter(nullptr, nullptr);
            fcgTXAudioEncoderPath_Enter(nullptr, nullptr);
            fcgTXMP4MuxerPath_Enter(nullptr, nullptr);
            fcgTXTC2MP4Path_Enter(nullptr, nullptr);
            fcgTXMP4RawPath_Enter(nullptr, nullptr);
            fcgTXMKVMuxerPath_Enter(nullptr, nullptr);
        }
    private:
        System::Void frmConfig::ExeTXPathLeave() {
            fcgTXVideoEncoderPath_Leave(nullptr, nullptr);
            fcgTXAudioEncoderPath_Leave(nullptr, nullptr);
            fcgTXMP4MuxerPath_Leave(nullptr, nullptr);
            fcgTXTC2MP4Path_Leave(nullptr, nullptr);
            fcgTXMP4RawPath_Leave(nullptr, nullptr);
            fcgTXMKVMuxerPath_Leave(nullptr, nullptr);
        }
    private:
        System::Void fcgBTVideoEncoderPath_Click(System::Object^  sender, System::EventArgs^  e) {
            openExeFile(fcgTXVideoEncoderPath, LocalStg.vidEncName);
        }
    private:
        System::Void fcgTXVideoEncoderPath_Enter(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXVideoEncoderPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                fcgTXVideoEncoderPath->Text = L"";
            }
        }
    private:
        System::Void fcgTXVideoEncoderPath_Leave(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXVideoEncoderPath->Text->Length == 0) {
                fcgTXVideoEncoderPath->Text = LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH);
            }
        }
    private:
        System::Void fcgTXAudioEncoderPath_Enter(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXAudioEncoderPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                fcgTXAudioEncoderPath->Text = L"";
            }
        }
    private:
        System::Void fcgTXAudioEncoderPath_Leave(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXAudioEncoderPath->Text->Length == 0) {
                fcgTXAudioEncoderPath->Text = LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH);
            }
        }
    private:
        System::Void fcgTXMP4MuxerPath_Enter(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMP4MuxerPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                fcgTXMP4MuxerPath->Text = L"";
            }
        }
    private:
        System::Void fcgTXMP4MuxerPath_Leave(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMP4MuxerPath->Text->Length == 0) {
                fcgTXMP4MuxerPath->Text = LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH);
            }
        }
    private:
        System::Void fcgTXTC2MP4Path_Enter(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXTC2MP4Path->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                fcgTXTC2MP4Path->Text = L"";
            }
        }
    private:
        System::Void fcgTXTC2MP4Path_Leave(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXTC2MP4Path->Text->Length == 0) {
                fcgTXTC2MP4Path->Text = LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH);
            }
        }
    private:
        System::Void fcgTXMP4RawPath_Enter(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMP4RawPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                fcgTXMP4RawPath->Text = L"";
            }
        }
    private:
        System::Void fcgTXMP4RawPath_Leave(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMP4RawPath->Text->Length == 0) {
                fcgTXMP4RawPath->Text = LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH);
            }
        }
    private:
        System::Void fcgTXMKVMuxerPath_Enter(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMKVMuxerPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                fcgTXMKVMuxerPath->Text = L"";
            }
        }
    private:
        System::Void fcgTXMKVMuxerPath_Leave(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMKVMuxerPath->Text->Length == 0) {
                fcgTXMKVMuxerPath->Text = LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH);
            }
        }
    private:
        System::Void fcgBTMP4MuxerPath_Click(System::Object^  sender, System::EventArgs^  e) {
            openExeFile(fcgTXMP4MuxerPath, LocalStg.MP4MuxerExeName);
        }
    private:
        System::Void fcgBTTC2MP4Path_Click(System::Object^  sender, System::EventArgs^  e) {
            openExeFile(fcgTXTC2MP4Path, LocalStg.TC2MP4ExeName);
        }
    private:
        System::Void fcgBTMP4RawMuxerPath_Click(System::Object^  sender, System::EventArgs^  e) {
            openExeFile(fcgTXMP4RawPath, LocalStg.MP4RawExeName);
        }
    private:
        System::Void fcgBTAudioEncoderPath_Click(System::Object^  sender, System::EventArgs^  e) {
            int index = fcgCXAudioEncoder->SelectedIndex;
            openExeFile(fcgTXAudioEncoderPath, LocalStg.audEncExeName[index]);
        }
    private:
        System::Void fcgBTMKVMuxerPath_Click(System::Object^  sender, System::EventArgs^  e) {
            openExeFile(fcgTXMKVMuxerPath, LocalStg.MKVMuxerExeName);
        }
    private:
        System::Void openTempFolder(TextBox^ TX) {
            FolderBrowserDialog^ fbd = fcgfolderBrowserTemp;
            if (Directory::Exists(TX->Text))
                fbd->SelectedPath = TX->Text;
            if (fbd->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
                if (sys_dat->exstg->s_local.get_relative_path)
                    fbd->SelectedPath = GetRelativePath(fbd->SelectedPath);
                TX->Text = fbd->SelectedPath;
                TX->SelectionStart = TX->Text->Length;
            }
        }
    private:
        System::Void fcgBTCustomAudioTempDir_Click(System::Object^  sender, System::EventArgs^  e) {
            openTempFolder(fcgTXCustomAudioTempDir);
        }
    private:
        System::Void fcgBTMP4BoxTempDir_Click(System::Object^  sender, System::EventArgs^  e) {
            openTempFolder(fcgTXMP4BoxTempDir);
        }
    private:
        System::Void fcgBTCustomTempDir_Click(System::Object^  sender, System::EventArgs^  e) {
            openTempFolder(fcgTXCustomTempDir);
        }
    private:
        System::Boolean openAndSetFilePath(TextBox^ TX, String^ fileTypeName) {
            return openAndSetFilePath(TX, fileTypeName, nullptr, nullptr);
        }
    private:
        System::Boolean openAndSetFilePath(TextBox^ TX, String^ fileTypeName, String^ ext) {
            return openAndSetFilePath(TX, fileTypeName, ext, nullptr);
        }
    private:
        System::Boolean openAndSetFilePath(TextBox^ TX, String^ fileTypeName, String^ ext, String^ dir) {
            //WinXPにおいて、OpenFileDialogはCurrentDirctoryを勝手に変更しやがるので、
            //一度保存し、あとから再適用する
            String^ CurrentDir = Directory::GetCurrentDirectory();
            //設定
            if (ext == nullptr)
                ext = L".*";
            OpenFileDialog^ ofd = fcgOpenFileDialog;
            ofd->FileName = L"";
            if (dir != nullptr && Directory::Exists(dir))
                ofd->InitialDirectory = dir;
            if (TX->Text->Length) {
                String^ fileName = nullptr;
                try {
                    fileName = Path::GetFileName(TX->Text);
                } catch (...) {
                    //invalid charによる例外は破棄
                }
                if (fileName != nullptr)
                    ofd->FileName = fileName;
            }
            ofd->Multiselect = false;
            ofd->Filter = fileTypeName + L"(*" + ext + L")|*" + ext;
            bool ret = (ofd->ShowDialog() == System::Windows::Forms::DialogResult::OK);
            if (ret) {
                if (sys_dat->exstg->s_local.get_relative_path)
                    ofd->FileName = GetRelativePath(ofd->FileName, CurrentDir);
                TX->Text = ofd->FileName;
                TX->SelectionStart = TX->Text->Length;
            }
            Directory::SetCurrentDirectory(CurrentDir);
            return ret;
        }
    private:
        System::Void fcgBTBatBeforePath_Click(System::Object^  sender, System::EventArgs^  e) {
            if (openAndSetFilePath(fcgTXBatBeforePath, LOAD_CLI_STRING(AUO_CONFIG_BAT_FILE), ".bat", LocalStg.LastBatDir))
                LocalStg.LastBatDir = Path::GetDirectoryName(fcgTXBatBeforePath->Text);
        }
    private:
        System::Void fcgBTBatAfterPath_Click(System::Object^  sender, System::EventArgs^  e) {
            if (openAndSetFilePath(fcgTXBatAfterPath, LOAD_CLI_STRING(AUO_CONFIG_BAT_FILE), ".bat", LocalStg.LastBatDir))
                LocalStg.LastBatDir = Path::GetDirectoryName(fcgTXBatAfterPath->Text);
        }
    private:
        System::Void fcgBTBatBeforeAudioPath_Click(System::Object^  sender, System::EventArgs^  e) {
            if (openAndSetFilePath(fcgTXBatBeforeAudioPath, LOAD_CLI_STRING(AUO_CONFIG_BAT_FILE), ".bat", LocalStg.LastBatDir))
                LocalStg.LastBatDir = Path::GetDirectoryName(fcgTXBatBeforeAudioPath->Text);
        }
    private:
        System::Void fcgBTBatAfterAudioPath_Click(System::Object^  sender, System::EventArgs^  e) {
            if (openAndSetFilePath(fcgTXBatAfterAudioPath, LOAD_CLI_STRING(AUO_CONFIG_BAT_FILE), ".bat", LocalStg.LastBatDir))
                LocalStg.LastBatDir = Path::GetDirectoryName(fcgTXBatAfterAudioPath->Text);
        }
    private:
        System::Void SetCXIndex(ComboBox^ CX, int index) {
            if (CX->Items->Count > 0) {
                CX->SelectedIndex = clamp(index, 0, CX->Items->Count - 1);
            }
        }
    private:
        System::Void SetNUValue(NumericUpDown^ NU, Decimal d) {
            NU->Value = clamp(d, NU->Minimum, NU->Maximum);
        }
    private:
        System::Void SetNUValue(NumericUpDown^ NU, int i) {
            NU->Value = clamp(Convert::ToDecimal(i), NU->Minimum, NU->Maximum);
        }
    private:
        System::Void SetNUValue(NumericUpDown^ NU, unsigned int i) {
            NU->Value = clamp(Convert::ToDecimal(i), NU->Minimum, NU->Maximum);
        }
    private:
        System::Void SetNUValue(NumericUpDown^ NU, float f) {
            NU->Value = clamp(Convert::ToDecimal(f), NU->Minimum, NU->Maximum);
        }
    private:
        System::Void frmConfig_Load(System::Object^  sender, System::EventArgs^  e) {
            InitForm();
        }
    private:
        System::Void fcgBTOK_Click(System::Object^  sender, System::EventArgs^  e) {
            if (CheckLocalStg())
                return;
            init_CONF_GUIEX(conf, false);
            FrmToConf(conf);
            SaveLocalStg();
            ZeroMemory(conf->oth.notes, sizeof(conf->oth.notes));
            this->Close();
        }
    private:
        System::Void fcgBTCancel_Click(System::Object^  sender, System::EventArgs^  e) {
            this->Close();
        }
    private:
        System::Void fcgBTDefault_Click(System::Object^  sender, System::EventArgs^  e) {
            CONF_GUIEX confDefault;
            init_CONF_GUIEX(&confDefault, FALSE);
            ConfToFrm(&confDefault);
        }
    private:
        System::Void ChangePresetNameDisplay(bool changed) {
            if (CheckedStgMenuItem != nullptr) {
                fcgTSSettings->Text = (changed) ? L"[" + CheckedStgMenuItem->Text + L"]*" : CheckedStgMenuItem->Text;
                fcgTSBSave->Enabled = changed;
            }
        }
    private:
        System::Void fcgRebuildCmd(System::Object^  sender, System::EventArgs^  e) {
            CONF_GUIEX rebuild;
            init_CONF_GUIEX(&rebuild, FALSE);
            fcgTXCmd->Text = FrmToConf(&rebuild);
            if (CheckedStgMenuItem != nullptr)
                ChangePresetNameDisplay(memcmp(&rebuild, cnf_stgSelected, sizeof(CONF_GUIEX)) != 0);
        }
    private:
        System::Void CheckOtherChanges(System::Object^  sender, System::EventArgs^  e) {
            if (CheckedStgMenuItem == nullptr)
                return;
            CONF_GUIEX check_change;
            init_CONF_GUIEX(&check_change, FALSE);
            FrmToConf(&check_change);
            ChangePresetNameDisplay(memcmp(&check_change, cnf_stgSelected, sizeof(CONF_GUIEX)) != 0);
        }
    private:
        System::Void fcgTXCmd_DoubleClick(System::Object^  sender, System::EventArgs^  e) {
            int offset = (fcgTXCmd->Multiline) ? -fcgTXCmdfulloffset : fcgTXCmdfulloffset;
            fcgTXCmd->Height += offset;
            this->Height += offset;
            fcgTXCmd->Multiline = !fcgTXCmd->Multiline;
        }
    private:
        System::Void fcgTSSettings_Click(System::Object^  sender, System::EventArgs^  e) {
            if (EnableSettingsNoteChange(false))
                fcgTSSettings->ShowDropDown();
        }
    private:
        System::Void fcgTXVideoEncoderPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXVideoEncoderPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                LocalStg.vidEncPath = L"";
                fcgTXVideoEncoderPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Disabled);
            } else {
                fcgTXVideoEncoderPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Normal);
                LocalStg.vidEncPath = fcgTXVideoEncoderPath->Text;
                fcgTXVideoEncoderPath->ContextMenuStrip = (File::Exists(fcgTXVideoEncoderPath->Text)) ? fcgCSExeFiles : nullptr;
            }
            GetVidEncInfoAsync();
        }
    private:
        System::Void fcgTXAudioEncoderPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            if (fcgCXAudioEncoder->SelectedIndex < 0) return;
            if (fcgTXAudioEncoderPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                LocalStg.audEncPath[fcgCXAudioEncoder->SelectedIndex] = L"";
                fcgTXAudioEncoderPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Disabled);
            } else {
                fcgTXAudioEncoderPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Normal);
                LocalStg.audEncPath[fcgCXAudioEncoder->SelectedIndex] = fcgTXAudioEncoderPath->Text;
                fcgBTAudioEncoderPath->ContextMenuStrip = (File::Exists(fcgTXAudioEncoderPath->Text)) ? fcgCSExeFiles : nullptr;
            }
        }
    private:
        System::Void fcgTXMP4MuxerPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMP4MuxerPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                LocalStg.MP4MuxerPath = L"";
                fcgTXMP4MuxerPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Disabled);
            } else {
                fcgTXMP4MuxerPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Normal);
                LocalStg.MP4MuxerPath = fcgTXMP4MuxerPath->Text;
                fcgBTMP4MuxerPath->ContextMenuStrip = (File::Exists(fcgTXMP4MuxerPath->Text)) ? fcgCSExeFiles : nullptr;
            }
        }
    private:
        System::Void fcgTXTC2MP4Path_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXTC2MP4Path->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                LocalStg.TC2MP4Path = L"";
                fcgTXTC2MP4Path->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Disabled);
            } else {
                fcgTXTC2MP4Path->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Normal);
                LocalStg.TC2MP4Path = fcgTXTC2MP4Path->Text;
                fcgBTTC2MP4Path->ContextMenuStrip = (File::Exists(fcgTXTC2MP4Path->Text)) ? fcgCSExeFiles : nullptr;
            }
        }
    private:
        System::Void fcgTXMP4RawMuxerPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMP4RawPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                LocalStg.MP4RawPath = L"";
                fcgTXMP4RawPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Disabled);
            } else {
                fcgTXMP4RawPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Normal);
                LocalStg.MP4RawPath = fcgTXMP4RawPath->Text;
                fcgBTMP4RawPath->ContextMenuStrip = (File::Exists(fcgTXMP4RawPath->Text)) ? fcgCSExeFiles : nullptr;
            }
        }
    private:
        System::Void fcgTXMKVMuxerPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            if (fcgTXMKVMuxerPath->Text == LOAD_CLI_STRING(AUO_CONFIG_CX_USE_DEFAULT_EXE_PATH)) {
                LocalStg.MKVMuxerPath = L"";
                fcgTXMKVMuxerPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Disabled);
            } else {
                fcgTXMKVMuxerPath->ForeColor = getTextBoxForeColor(themeMode, dwStgReader, DarkenWindowState::Normal);
                LocalStg.MKVMuxerPath = fcgTXMKVMuxerPath->Text;
                fcgBTMKVMuxerPath->ContextMenuStrip = (File::Exists(fcgTXMKVMuxerPath->Text)) ? fcgCSExeFiles : nullptr;
            }
        }
    private:
        System::Void fcgTXMP4BoxTempDir_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            LocalStg.CustomMP4TmpDir = fcgTXMP4BoxTempDir->Text;
        }
    private:
        System::Void fcgTXCustomAudioTempDir_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            LocalStg.CustomAudTmpDir = fcgTXCustomAudioTempDir->Text;
        }
    private:
        System::Void fcgTXCustomTempDir_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            LocalStg.CustomTmpDir = fcgTXCustomTempDir->Text;
        }
    private:
        System::Void fcgSetDragDropFilename_Enter(System::Object^  sender, DragEventArgs^  e) {
            e->Effect = (e->Data->GetDataPresent(DataFormats::FileDrop)) ? DragDropEffects::Copy : DragDropEffects::None;
        }
    private:
        System::Void fcgSetDragDropFilename_DragDrop(System::Object^  sender, DragEventArgs^  e) {
            TextBox^ TX = dynamic_cast<TextBox^>(sender);
            array<System::String ^>^ filelist = dynamic_cast<array<System::String ^>^>(e->Data->GetData(DataFormats::FileDrop, false));
            if (filelist == nullptr || TX == nullptr)
                return;
            String^ filePath = filelist[0]; //複数だった場合は先頭のものを使用
            if (sys_dat->exstg->s_local.get_relative_path)
                filePath = GetRelativePath(filePath);
            TX->Text = filePath;
        }
    private:
        System::Void fcgTSExeFileshelp_Click(System::Object^  sender, System::EventArgs^  e) {
            System::Windows::Forms::ToolStripMenuItem^ TS = dynamic_cast<System::Windows::Forms::ToolStripMenuItem^>(sender);
            if (TS == nullptr) return;
            System::Windows::Forms::ContextMenuStrip^ CS = dynamic_cast<System::Windows::Forms::ContextMenuStrip^>(TS->Owner);
            if (CS == nullptr) return;

            //Name, args, Path の順番
            array<ExeControls>^ ControlList = {
                { fcgBTVideoEncoderPath->Name,   fcgTXVideoEncoderPath->Text,   sys_dat->exstg->s_enc.help_cmd },
                { fcgBTAudioEncoderPath->Name,   fcgTXAudioEncoderPath->Text,   sys_dat->exstg->s_aud_ext[fcgCXAudioEncoder->SelectedIndex].cmd_help },
                { fcgBTMP4MuxerPath->Name,       fcgTXMP4MuxerPath->Text,       sys_dat->exstg->s_mux[MUXER_MP4].help_cmd },
                { fcgBTTC2MP4Path->Name,         fcgTXTC2MP4Path->Text,         sys_dat->exstg->s_mux[MUXER_TC2MP4].help_cmd },
                { fcgBTMP4RawPath->Name,         fcgTXMP4RawPath->Text,         sys_dat->exstg->s_mux[MUXER_MP4_RAW].help_cmd },
                { fcgBTMKVMuxerPath->Name,       fcgTXMKVMuxerPath->Text,       sys_dat->exstg->s_mux[MUXER_MKV].help_cmd },
            };
            for (int i = 0; i < ControlList->Length; i++) {
                if (NULL == String::Compare(CS->SourceControl->Name, ControlList[i].Name)) {
                    ShowExehelp(ControlList[i].Path, String(ControlList[i].args).ToString());
                    return;
                }
            }
            MessageBox::Show(LOAD_CLI_STRING(AUO_CONFIG_HELP_CMD_UNSET), LOAD_CLI_STRING(AUO_GUIEX_ERROR), MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    private:
        System::Void fcgLBguiExBlog_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e) {
            fcgLBguiExBlog->LinkVisited = true;
            try {
                System::Diagnostics::Process::Start(String(sys_dat->exstg->blog_url).ToString());
            } catch (...) {
                //まあ放置
            };
        }
    private:
        System::Void fcgTBVppAfsScroll(System::Object ^sender, System::EventArgs ^e) {
            System::Windows::Forms::TrackBar^ senderTB = dynamic_cast<System::Windows::Forms::TrackBar^>(sender);
            if (senderTB == nullptr) return;

            array<TrackBarNU> ^targetList = {
                { fcgTBVppAfsMethodSwitch, fcgNUVppAfsMethodSwitch },
                { fcgTBVppAfsCoeffShift,   fcgNUVppAfsCoeffShift },
                { fcgTBVppAfsThreShift,    fcgNUVppAfsThreShift },
                { fcgTBVppAfsThreDeint,    fcgNUVppAfsThreDeint },
                { fcgTBVppAfsThreYMotion,  fcgNUVppAfsThreYMotion },
                { fcgTBVppAfsThreCMotion,  fcgNUVppAfsThreCMotion }
            };
            for (int i = 0; i < targetList->Length; i++) {
                if (NULL == String::Compare(senderTB->Name, targetList[i].TB->Name)) {
                    SetNUValue(targetList[i].NU, senderTB->Value);
                    return;
                }
            }
        }
    private:
        System::Void fcgNUVppAfsValueChanged(System::Object ^sender, System::EventArgs ^e) {
            System::Windows::Forms::NumericUpDown ^senderNU = dynamic_cast<System::Windows::Forms::NumericUpDown ^>(sender);
            if (senderNU == nullptr) return;

            array<TrackBarNU> ^targetList = {
                { fcgTBVppAfsMethodSwitch, fcgNUVppAfsMethodSwitch },
                { fcgTBVppAfsCoeffShift,   fcgNUVppAfsCoeffShift },
                { fcgTBVppAfsThreShift,    fcgNUVppAfsThreShift },
                { fcgTBVppAfsThreDeint,    fcgNUVppAfsThreDeint },
                { fcgTBVppAfsThreYMotion,  fcgNUVppAfsThreYMotion },
                { fcgTBVppAfsThreCMotion,  fcgNUVppAfsThreCMotion }
            };
            for (int i = 0; i < targetList->Length; i++) {
                if (NULL == String::Compare(senderNU->Name, targetList[i].NU->Name)) {
                    targetList[i].TB->Value = (int)senderNU->Value;
                    return;
                }
            }
        }
};
}
