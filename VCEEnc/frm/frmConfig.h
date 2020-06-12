// -----------------------------------------------------------------------------------------
//     VCEEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2014-2017 rigaya
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
// IABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// ------------------------------------------------------------------------------------------

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

#include "frmConfig_helper.h"

#include "vce_param.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::IO;
using namespace System::Threading::Tasks;


namespace VCEEnc {

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
            InitData(_conf, _sys_dat);
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
            if (cnf_stgSelected) free(cnf_stgSelected); cnf_stgSelected = NULL;
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

























private: System::Windows::Forms::TabPage^  tabPageExOpt;
private: System::Windows::Forms::Label^  fcgLBTempDir;

private: System::Windows::Forms::Button^  fcgBTCustomTempDir;
private: System::Windows::Forms::TextBox^  fcgTXCustomTempDir;

private: System::Windows::Forms::ComboBox^  fcgCXTempDir;


private: System::Windows::Forms::Label^  fcgLBGOPLengthAuto;






























private: System::Windows::Forms::NumericUpDown^  fcgNUSlices;

private: System::Windows::Forms::Label^  fcgLBSlices;

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





private: System::Windows::Forms::TabPage^  fcgtabPageMPG;
private: System::Windows::Forms::Button^  fcgBTMPGMuxerPath;

private: System::Windows::Forms::TextBox^  fcgTXMPGMuxerPath;

private: System::Windows::Forms::Label^  fcgLBMPGMuxerPath;

private: System::Windows::Forms::ComboBox^  fcgCXMPGCmdEx;

private: System::Windows::Forms::Label^  label3;
private: System::Windows::Forms::CheckBox^  fcgCBMPGMuxerExt;

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
private: System::Windows::Forms::CheckBox^  fcgCBAuoTcfileout;

private: System::Windows::Forms::Label^  fcgLBDeblock;

private: System::Windows::Forms::ComboBox^  fcgCXMotionEst;
private: System::Windows::Forms::Label^  fcgLBMotionEst;
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
private: System::Windows::Forms::CheckBox^  fcgCBVBAQ;

private: System::Windows::Forms::Label^  fcgLBPreAnalysis;
private: System::Windows::Forms::Label^  fcgLBCodec;
private: System::Windows::Forms::ComboBox^  fcgCXCodec;
private: System::Windows::Forms::Panel^  fcgPNHEVCLevelProfile;
private: System::Windows::Forms::ComboBox^  fcgCXHEVCLevel;
private: System::Windows::Forms::ComboBox^  fcgCXHEVCProfile;
private: System::Windows::Forms::Label^  fcgLBHEVCLevel;
private: System::Windows::Forms::Label^  fcgLBHEVCProfile;
private: System::Windows::Forms::Panel^  fcgPNBframes;



private: System::Windows::Forms::NumericUpDown^  fcgNURefFrames;
private: System::Windows::Forms::Label^  fcgLBRefFrames;




private: System::Windows::Forms::Panel^  fcgPNH264Features;
private: System::Windows::Forms::CheckBox^  fcgCBResize;
private: System::Windows::Forms::GroupBox^  fcggroupboxResize;
private: System::Windows::Forms::Label^  fcgLBResize;
private: System::Windows::Forms::NumericUpDown^  fcgNUResizeH;

private: System::Windows::Forms::NumericUpDown^  fcgNUResizeW;
private: System::Windows::Forms::CheckBox^  fcgCBTimerPeriodTuning;
private: System::Windows::Forms::Button^  fcgBTVideoEncoderPath;
private: System::Windows::Forms::TextBox^  fcgTXVideoEncoderPath;
private: System::Windows::Forms::Label^  fcgLBVideoEncoderPath;
private: System::Windows::Forms::CheckBox^  fcgCBPreAnalysis;
private: System::Windows::Forms::Label^  fcgLBQPMin;
private: System::Windows::Forms::GroupBox^  fcggroupBoxColor;
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
private: System::Windows::Forms::Label ^label20;
private: System::Windows::Forms::Label ^label19;
private: System::Windows::Forms::Label ^label4;
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
private: System::Windows::Forms::Label ^label1;
private: System::Windows::Forms::ComboBox ^fcgCXAudioEncoderInternal;
private: System::Windows::Forms::CheckBox ^fcgCBAudioUseExt;
private: System::Windows::Forms::TabPage ^fcgtabPageInternal;
private: System::Windows::Forms::ComboBox ^fcgCXInternalCmdEx;
private: System::Windows::Forms::Label ^fcgLBInternalCmdEx;
































































































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
            System::ComponentModel::ComponentResourceManager ^resources = (gcnew System::ComponentModel::ComponentResourceManager(frmConfig::typeid));
            this->fcgtoolStripSettings = (gcnew System::Windows::Forms::ToolStrip());
            this->fcgTSBSave = (gcnew System::Windows::Forms::ToolStripButton());
            this->fcgTSBSaveNew = (gcnew System::Windows::Forms::ToolStripButton());
            this->fcgTSBDelete = (gcnew System::Windows::Forms::ToolStripButton());
            this->fcgtoolStripSeparator1 = (gcnew System::Windows::Forms::ToolStripSeparator());
            this->fcgTSSettings = (gcnew System::Windows::Forms::ToolStripDropDownButton());
            this->fcgTSBBitrateCalc = (gcnew System::Windows::Forms::ToolStripButton());
            this->toolStripSeparator2 = (gcnew System::Windows::Forms::ToolStripSeparator());
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
            this->fcgtabPageMPG = (gcnew System::Windows::Forms::TabPage());
            this->fcgBTMPGMuxerPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXMPGMuxerPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBMPGMuxerPath = (gcnew System::Windows::Forms::Label());
            this->fcgCXMPGCmdEx = (gcnew System::Windows::Forms::ComboBox());
            this->label3 = (gcnew System::Windows::Forms::Label());
            this->fcgCBMPGMuxerExt = (gcnew System::Windows::Forms::CheckBox());
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
            this->fcgPNQP = (gcnew System::Windows::Forms::Panel());
            this->fcgLBQPI = (gcnew System::Windows::Forms::Label());
            this->fcgNUQPI = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUQPP = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUQPB = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBQPP = (gcnew System::Windows::Forms::Label());
            this->fcgLBQPB = (gcnew System::Windows::Forms::Label());
            this->fcgCXVideoFormat = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBVideoFormat = (gcnew System::Windows::Forms::Label());
            this->fcggroupBoxColor = (gcnew System::Windows::Forms::GroupBox());
            this->fcgCBFullrange = (gcnew System::Windows::Forms::CheckBox());
            this->fcgLBFullrange = (gcnew System::Windows::Forms::Label());
            this->fcgCXTransfer = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXColorPrim = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXColorMatrix = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBTransfer = (gcnew System::Windows::Forms::Label());
            this->fcgLBColorPrim = (gcnew System::Windows::Forms::Label());
            this->fcgLBColorMatrix = (gcnew System::Windows::Forms::Label());
            this->fcgLBQPMin = (gcnew System::Windows::Forms::Label());
            this->fcgCBPreAnalysis = (gcnew System::Windows::Forms::CheckBox());
            this->fcgBTVideoEncoderPath = (gcnew System::Windows::Forms::Button());
            this->fcgTXVideoEncoderPath = (gcnew System::Windows::Forms::TextBox());
            this->fcgLBVideoEncoderPath = (gcnew System::Windows::Forms::Label());
            this->fcgNURefFrames = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBRefFrames = (gcnew System::Windows::Forms::Label());
            this->fcgLBCodec = (gcnew System::Windows::Forms::Label());
            this->fcgCXCodec = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBPreAnalysis = (gcnew System::Windows::Forms::Label());
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
            this->fcgCXMotionEst = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBMotionEst = (gcnew System::Windows::Forms::Label());
            this->fcgLBDeblock = (gcnew System::Windows::Forms::Label());
            this->fcgpictureBoxVCEEnabled = (gcnew System::Windows::Forms::PictureBox());
            this->fcgpictureBoxVCEDisabled = (gcnew System::Windows::Forms::PictureBox());
            this->fcgCBDeblock = (gcnew System::Windows::Forms::CheckBox());
            this->fcgNUSlices = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBSlices = (gcnew System::Windows::Forms::Label());
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
            this->fcgPNBitrate = (gcnew System::Windows::Forms::Panel());
            this->fcgLBBitrate = (gcnew System::Windows::Forms::Label());
            this->fcgNUBitrate = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBBitrate2 = (gcnew System::Windows::Forms::Label());
            this->fcgNUMaxkbps = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBMaxkbps = (gcnew System::Windows::Forms::Label());
            this->fcgLBMaxBitrate2 = (gcnew System::Windows::Forms::Label());
            this->fcgPNBframes = (gcnew System::Windows::Forms::Panel());
            this->fcgLBBframes = (gcnew System::Windows::Forms::Label());
            this->fcgNUBframes = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCBBPyramid = (gcnew System::Windows::Forms::CheckBox());
            this->fcgLBBPyramid = (gcnew System::Windows::Forms::Label());
            this->fcgLBBDeltaQP = (gcnew System::Windows::Forms::Label());
            this->fcgNUBDeltaQP = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgLBBRefDeltaQP = (gcnew System::Windows::Forms::Label());
            this->fcgNUBRefDeltaQP = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgPNHEVCLevelProfile = (gcnew System::Windows::Forms::Panel());
            this->fcgCXHEVCLevel = (gcnew System::Windows::Forms::ComboBox());
            this->fcgCXHEVCProfile = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBHEVCLevel = (gcnew System::Windows::Forms::Label());
            this->fcgLBHEVCProfile = (gcnew System::Windows::Forms::Label());
            this->fcgPNH264Features = (gcnew System::Windows::Forms::Panel());
            this->fcgLBVBAQ = (gcnew System::Windows::Forms::Label());
            this->fcgCBVBAQ = (gcnew System::Windows::Forms::CheckBox());
            this->tabPageExOpt = (gcnew System::Windows::Forms::TabPage());
            this->fcgCBPSNR = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBSSIM = (gcnew System::Windows::Forms::CheckBox());
            this->fcggroupBoxVppDeinterlace = (gcnew System::Windows::Forms::GroupBox());
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
            this->label20 = (gcnew System::Windows::Forms::Label());
            this->label19 = (gcnew System::Windows::Forms::Label());
            this->label4 = (gcnew System::Windows::Forms::Label());
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
            this->fcgCBTimerPeriodTuning = (gcnew System::Windows::Forms::CheckBox());
            this->fcgCBResize = (gcnew System::Windows::Forms::CheckBox());
            this->fcggroupboxResize = (gcnew System::Windows::Forms::GroupBox());
            this->fcgCXVppResizeAlg = (gcnew System::Windows::Forms::ComboBox());
            this->fcgLBResize = (gcnew System::Windows::Forms::Label());
            this->fcgNUResizeH = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgNUResizeW = (gcnew System::Windows::Forms::NumericUpDown());
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
            this->fcgPNAudioInternal = (gcnew System::Windows::Forms::Panel());
            this->fcgLBAudioBitrateInternal = (gcnew System::Windows::Forms::Label());
            this->fcgNUAudioBitrateInternal = (gcnew System::Windows::Forms::NumericUpDown());
            this->fcgCXAudioEncModeInternal = (gcnew System::Windows::Forms::ComboBox());
            this->label1 = (gcnew System::Windows::Forms::Label());
            this->fcgCXAudioEncoderInternal = (gcnew System::Windows::Forms::ComboBox());
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
            this->fcgtoolStripSettings->SuspendLayout();
            this->fcgtabControlMux->SuspendLayout();
            this->fcgtabPageMP4->SuspendLayout();
            this->fcgtabPageMKV->SuspendLayout();
            this->fcgtabPageMPG->SuspendLayout();
            this->fcgtabPageMux->SuspendLayout();
            this->fcgtabPageBat->SuspendLayout();
            this->fcgtabPageInternal->SuspendLayout();
            this->fcgtabControlVCE->SuspendLayout();
            this->tabPageVideoEnc->SuspendLayout();
            this->fcgPNQP->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUQPI))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUQPP))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUQPB))->BeginInit();
            this->fcggroupBoxColor->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNURefFrames))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVBVBufSize))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUQPMax))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUQPMin))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgpictureBoxVCEEnabled))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgpictureBoxVCEDisabled))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUSlices))->BeginInit();
            this->fcgGroupBoxAspectRatio->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUAspectRatioY))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUAspectRatioX))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUGopLength))->BeginInit();
            this->fcgPNBitrate->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUBitrate))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUMaxkbps))->BeginInit();
            this->fcgPNBframes->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUBframes))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUBDeltaQP))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUBRefDeltaQP))->BeginInit();
            this->fcgPNHEVCLevelProfile->SuspendLayout();
            this->fcgPNH264Features->SuspendLayout();
            this->tabPageExOpt->SuspendLayout();
            this->fcggroupBoxVppDeinterlace->SuspendLayout();
            this->fcgPNVppAfs->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgTBVppAfsThreCMotion))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgTBVppAfsThreYMotion))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgTBVppAfsThreDeint))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgTBVppAfsThreShift))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgTBVppAfsCoeffShift))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsRight))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsLeft))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsBottom))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsUp))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgTBVppAfsMethodSwitch))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsThreCMotion))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsThreShift))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsThreDeint))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsThreYMotion))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsCoeffShift))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsMethodSwitch))->BeginInit();
            this->fcggroupboxResize->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUResizeH))->BeginInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUResizeW))->BeginInit();
            this->fcgCSExeFiles->SuspendLayout();
            this->fcgtabControlAudio->SuspendLayout();
            this->fcgtabPageAudioMain->SuspendLayout();
            this->fcgPNAudioInternal->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUAudioBitrateInternal))->BeginInit();
            this->fcgPNAudioExt->SuspendLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUAudioBitrate))->BeginInit();
            this->fcgtabPageAudioOther->SuspendLayout();
            this->SuspendLayout();
            // 
            // fcgtoolStripSettings
            // 
            this->fcgtoolStripSettings->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem ^  >(10) {
                this->fcgTSBSave,
                    this->fcgTSBSaveNew, this->fcgTSBDelete, this->fcgtoolStripSeparator1, this->fcgTSSettings, this->fcgTSBBitrateCalc, this->toolStripSeparator2,
                    this->fcgTSBOtherSettings, this->fcgTSLSettingsNotes, this->fcgTSTSettingsNotes
            });
            this->fcgtoolStripSettings->Location = System::Drawing::Point(0, 0);
            this->fcgtoolStripSettings->Name = L"fcgtoolStripSettings";
            this->fcgtoolStripSettings->Size = System::Drawing::Size(1008, 25);
            this->fcgtoolStripSettings->TabIndex = 1;
            this->fcgtoolStripSettings->Text = L"toolStrip1";
            // 
            // fcgTSBSave
            // 
            this->fcgTSBSave->Image = (cli::safe_cast<System::Drawing::Image ^>(resources->GetObject(L"fcgTSBSave.Image")));
            this->fcgTSBSave->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSBSave->Name = L"fcgTSBSave";
            this->fcgTSBSave->Size = System::Drawing::Size(84, 22);
            this->fcgTSBSave->Text = L"上書き保存";
            this->fcgTSBSave->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSBSave_Click);
            // 
            // fcgTSBSaveNew
            // 
            this->fcgTSBSaveNew->Image = (cli::safe_cast<System::Drawing::Image ^>(resources->GetObject(L"fcgTSBSaveNew.Image")));
            this->fcgTSBSaveNew->ImageTransparentColor = System::Drawing::Color::Black;
            this->fcgTSBSaveNew->Name = L"fcgTSBSaveNew";
            this->fcgTSBSaveNew->Size = System::Drawing::Size(75, 22);
            this->fcgTSBSaveNew->Text = L"新規保存";
            this->fcgTSBSaveNew->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSBSaveNew_Click);
            // 
            // fcgTSBDelete
            // 
            this->fcgTSBDelete->Image = (cli::safe_cast<System::Drawing::Image ^>(resources->GetObject(L"fcgTSBDelete.Image")));
            this->fcgTSBDelete->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSBDelete->Name = L"fcgTSBDelete";
            this->fcgTSBDelete->Size = System::Drawing::Size(51, 22);
            this->fcgTSBDelete->Text = L"削除";
            this->fcgTSBDelete->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSBDelete_Click);
            // 
            // fcgtoolStripSeparator1
            // 
            this->fcgtoolStripSeparator1->Name = L"fcgtoolStripSeparator1";
            this->fcgtoolStripSeparator1->Size = System::Drawing::Size(6, 25);
            // 
            // fcgTSSettings
            // 
            this->fcgTSSettings->Image = (cli::safe_cast<System::Drawing::Image ^>(resources->GetObject(L"fcgTSSettings.Image")));
            this->fcgTSSettings->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSSettings->Name = L"fcgTSSettings";
            this->fcgTSSettings->Size = System::Drawing::Size(79, 22);
            this->fcgTSSettings->Text = L"プリセット";
            this->fcgTSSettings->DropDownItemClicked += gcnew System::Windows::Forms::ToolStripItemClickedEventHandler(this, &frmConfig::fcgTSSettings_DropDownItemClicked);
            this->fcgTSSettings->Click += gcnew System::EventHandler(this, &frmConfig::fcgTSSettings_Click);
            // 
            // fcgTSBBitrateCalc
            // 
            this->fcgTSBBitrateCalc->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->fcgTSBBitrateCalc->CheckOnClick = true;
            this->fcgTSBBitrateCalc->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->fcgTSBBitrateCalc->Image = (cli::safe_cast<System::Drawing::Image ^>(resources->GetObject(L"fcgTSBBitrateCalc.Image")));
            this->fcgTSBBitrateCalc->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSBBitrateCalc->Name = L"fcgTSBBitrateCalc";
            this->fcgTSBBitrateCalc->Size = System::Drawing::Size(96, 22);
            this->fcgTSBBitrateCalc->Text = L"ビットレート計算機";
            this->fcgTSBBitrateCalc->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgTSBBitrateCalc_CheckedChanged);
            // 
            // toolStripSeparator2
            // 
            this->toolStripSeparator2->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->toolStripSeparator2->Name = L"toolStripSeparator2";
            this->toolStripSeparator2->Size = System::Drawing::Size(6, 25);
            // 
            // fcgTSBOtherSettings
            // 
            this->fcgTSBOtherSettings->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
            this->fcgTSBOtherSettings->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
            this->fcgTSBOtherSettings->Image = (cli::safe_cast<System::Drawing::Image ^>(resources->GetObject(L"fcgTSBOtherSettings.Image")));
            this->fcgTSBOtherSettings->ImageTransparentColor = System::Drawing::Color::Magenta;
            this->fcgTSBOtherSettings->Name = L"fcgTSBOtherSettings";
            this->fcgTSBOtherSettings->Size = System::Drawing::Size(76, 22);
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
            this->fcgTSLSettingsNotes->Size = System::Drawing::Size(45, 22);
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
            this->fcgTSTSettingsNotes->Size = System::Drawing::Size(200, 25);
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
            this->fcgtabControlMux->Controls->Add(this->fcgtabPageMPG);
            this->fcgtabControlMux->Controls->Add(this->fcgtabPageMux);
            this->fcgtabControlMux->Controls->Add(this->fcgtabPageBat);
            this->fcgtabControlMux->Controls->Add(this->fcgtabPageInternal);
            this->fcgtabControlMux->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgtabControlMux->Location = System::Drawing::Point(622, 339);
            this->fcgtabControlMux->Name = L"fcgtabControlMux";
            this->fcgtabControlMux->SelectedIndex = 0;
            this->fcgtabControlMux->Size = System::Drawing::Size(384, 201);
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
            this->fcgtabPageMP4->Location = System::Drawing::Point(4, 23);
            this->fcgtabPageMP4->Name = L"fcgtabPageMP4";
            this->fcgtabPageMP4->Padding = System::Windows::Forms::Padding(3);
            this->fcgtabPageMP4->Size = System::Drawing::Size(376, 174);
            this->fcgtabPageMP4->TabIndex = 0;
            this->fcgtabPageMP4->Text = L"mp4";
            this->fcgtabPageMP4->UseVisualStyleBackColor = true;
            // 
            // fcgBTMP4RawPath
            // 
            this->fcgBTMP4RawPath->Location = System::Drawing::Point(340, 102);
            this->fcgBTMP4RawPath->Name = L"fcgBTMP4RawPath";
            this->fcgBTMP4RawPath->Size = System::Drawing::Size(30, 23);
            this->fcgBTMP4RawPath->TabIndex = 23;
            this->fcgBTMP4RawPath->Text = L"...";
            this->fcgBTMP4RawPath->UseVisualStyleBackColor = true;
            this->fcgBTMP4RawPath->Visible = false;
            this->fcgBTMP4RawPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMP4RawMuxerPath_Click);
            // 
            // fcgTXMP4RawPath
            // 
            this->fcgTXMP4RawPath->AllowDrop = true;
            this->fcgTXMP4RawPath->Location = System::Drawing::Point(136, 103);
            this->fcgTXMP4RawPath->Name = L"fcgTXMP4RawPath";
            this->fcgTXMP4RawPath->Size = System::Drawing::Size(202, 21);
            this->fcgTXMP4RawPath->TabIndex = 22;
            this->fcgTXMP4RawPath->Visible = false;
            this->fcgTXMP4RawPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4RawMuxerPath_TextChanged);
            this->fcgTXMP4RawPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXMP4RawPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            // 
            // fcgLBMP4RawPath
            // 
            this->fcgLBMP4RawPath->AutoSize = true;
            this->fcgLBMP4RawPath->Location = System::Drawing::Point(4, 106);
            this->fcgLBMP4RawPath->Name = L"fcgLBMP4RawPath";
            this->fcgLBMP4RawPath->Size = System::Drawing::Size(49, 14);
            this->fcgLBMP4RawPath->TabIndex = 21;
            this->fcgLBMP4RawPath->Text = L"～の指定";
            this->fcgLBMP4RawPath->Visible = false;
            // 
            // fcgCBMP4MuxApple
            // 
            this->fcgCBMP4MuxApple->AutoSize = true;
            this->fcgCBMP4MuxApple->Location = System::Drawing::Point(254, 34);
            this->fcgCBMP4MuxApple->Name = L"fcgCBMP4MuxApple";
            this->fcgCBMP4MuxApple->Size = System::Drawing::Size(109, 18);
            this->fcgCBMP4MuxApple->TabIndex = 20;
            this->fcgCBMP4MuxApple->Tag = L"chValue";
            this->fcgCBMP4MuxApple->Text = L"Apple形式に対応";
            this->fcgCBMP4MuxApple->UseVisualStyleBackColor = true;
            // 
            // fcgBTMP4BoxTempDir
            // 
            this->fcgBTMP4BoxTempDir->Location = System::Drawing::Point(340, 146);
            this->fcgBTMP4BoxTempDir->Name = L"fcgBTMP4BoxTempDir";
            this->fcgBTMP4BoxTempDir->Size = System::Drawing::Size(30, 23);
            this->fcgBTMP4BoxTempDir->TabIndex = 8;
            this->fcgBTMP4BoxTempDir->Text = L"...";
            this->fcgBTMP4BoxTempDir->UseVisualStyleBackColor = true;
            this->fcgBTMP4BoxTempDir->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMP4BoxTempDir_Click);
            // 
            // fcgTXMP4BoxTempDir
            // 
            this->fcgTXMP4BoxTempDir->Location = System::Drawing::Point(107, 147);
            this->fcgTXMP4BoxTempDir->Name = L"fcgTXMP4BoxTempDir";
            this->fcgTXMP4BoxTempDir->Size = System::Drawing::Size(227, 21);
            this->fcgTXMP4BoxTempDir->TabIndex = 7;
            this->fcgTXMP4BoxTempDir->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4BoxTempDir_TextChanged);
            // 
            // fcgCXMP4BoxTempDir
            // 
            this->fcgCXMP4BoxTempDir->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXMP4BoxTempDir->FormattingEnabled = true;
            this->fcgCXMP4BoxTempDir->Location = System::Drawing::Point(145, 119);
            this->fcgCXMP4BoxTempDir->Name = L"fcgCXMP4BoxTempDir";
            this->fcgCXMP4BoxTempDir->Size = System::Drawing::Size(206, 22);
            this->fcgCXMP4BoxTempDir->TabIndex = 6;
            this->fcgCXMP4BoxTempDir->Tag = L"chValue";
            // 
            // fcgLBMP4BoxTempDir
            // 
            this->fcgLBMP4BoxTempDir->AutoSize = true;
            this->fcgLBMP4BoxTempDir->Location = System::Drawing::Point(25, 122);
            this->fcgLBMP4BoxTempDir->Name = L"fcgLBMP4BoxTempDir";
            this->fcgLBMP4BoxTempDir->Size = System::Drawing::Size(105, 14);
            this->fcgLBMP4BoxTempDir->TabIndex = 18;
            this->fcgLBMP4BoxTempDir->Text = L"mp4box一時フォルダ";
            // 
            // fcgBTTC2MP4Path
            // 
            this->fcgBTTC2MP4Path->Location = System::Drawing::Point(340, 80);
            this->fcgBTTC2MP4Path->Name = L"fcgBTTC2MP4Path";
            this->fcgBTTC2MP4Path->Size = System::Drawing::Size(30, 23);
            this->fcgBTTC2MP4Path->TabIndex = 5;
            this->fcgBTTC2MP4Path->Text = L"...";
            this->fcgBTTC2MP4Path->UseVisualStyleBackColor = true;
            this->fcgBTTC2MP4Path->Visible = false;
            this->fcgBTTC2MP4Path->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTTC2MP4Path_Click);
            // 
            // fcgTXTC2MP4Path
            // 
            this->fcgTXTC2MP4Path->AllowDrop = true;
            this->fcgTXTC2MP4Path->Location = System::Drawing::Point(136, 81);
            this->fcgTXTC2MP4Path->Name = L"fcgTXTC2MP4Path";
            this->fcgTXTC2MP4Path->Size = System::Drawing::Size(202, 21);
            this->fcgTXTC2MP4Path->TabIndex = 4;
            this->fcgTXTC2MP4Path->Visible = false;
            this->fcgTXTC2MP4Path->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXTC2MP4Path_TextChanged);
            this->fcgTXTC2MP4Path->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXTC2MP4Path->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            // 
            // fcgBTMP4MuxerPath
            // 
            this->fcgBTMP4MuxerPath->Location = System::Drawing::Point(340, 58);
            this->fcgBTMP4MuxerPath->Name = L"fcgBTMP4MuxerPath";
            this->fcgBTMP4MuxerPath->Size = System::Drawing::Size(30, 23);
            this->fcgBTMP4MuxerPath->TabIndex = 3;
            this->fcgBTMP4MuxerPath->Text = L"...";
            this->fcgBTMP4MuxerPath->UseVisualStyleBackColor = true;
            this->fcgBTMP4MuxerPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMP4MuxerPath_Click);
            // 
            // fcgTXMP4MuxerPath
            // 
            this->fcgTXMP4MuxerPath->AllowDrop = true;
            this->fcgTXMP4MuxerPath->Location = System::Drawing::Point(136, 59);
            this->fcgTXMP4MuxerPath->Name = L"fcgTXMP4MuxerPath";
            this->fcgTXMP4MuxerPath->Size = System::Drawing::Size(202, 21);
            this->fcgTXMP4MuxerPath->TabIndex = 2;
            this->fcgTXMP4MuxerPath->Tag = L"";
            this->fcgTXMP4MuxerPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMP4MuxerPath_TextChanged);
            this->fcgTXMP4MuxerPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXMP4MuxerPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            // 
            // fcgLBTC2MP4Path
            // 
            this->fcgLBTC2MP4Path->AutoSize = true;
            this->fcgLBTC2MP4Path->Location = System::Drawing::Point(4, 84);
            this->fcgLBTC2MP4Path->Name = L"fcgLBTC2MP4Path";
            this->fcgLBTC2MP4Path->Size = System::Drawing::Size(49, 14);
            this->fcgLBTC2MP4Path->TabIndex = 4;
            this->fcgLBTC2MP4Path->Text = L"～の指定";
            this->fcgLBTC2MP4Path->Visible = false;
            // 
            // fcgLBMP4MuxerPath
            // 
            this->fcgLBMP4MuxerPath->AutoSize = true;
            this->fcgLBMP4MuxerPath->Location = System::Drawing::Point(4, 62);
            this->fcgLBMP4MuxerPath->Name = L"fcgLBMP4MuxerPath";
            this->fcgLBMP4MuxerPath->Size = System::Drawing::Size(49, 14);
            this->fcgLBMP4MuxerPath->TabIndex = 3;
            this->fcgLBMP4MuxerPath->Text = L"～の指定";
            // 
            // fcgCXMP4CmdEx
            // 
            this->fcgCXMP4CmdEx->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXMP4CmdEx->FormattingEnabled = true;
            this->fcgCXMP4CmdEx->Location = System::Drawing::Point(213, 7);
            this->fcgCXMP4CmdEx->Name = L"fcgCXMP4CmdEx";
            this->fcgCXMP4CmdEx->Size = System::Drawing::Size(157, 22);
            this->fcgCXMP4CmdEx->TabIndex = 1;
            this->fcgCXMP4CmdEx->Tag = L"chValue";
            // 
            // fcgLBMP4CmdEx
            // 
            this->fcgLBMP4CmdEx->AutoSize = true;
            this->fcgLBMP4CmdEx->Location = System::Drawing::Point(139, 10);
            this->fcgLBMP4CmdEx->Name = L"fcgLBMP4CmdEx";
            this->fcgLBMP4CmdEx->Size = System::Drawing::Size(68, 14);
            this->fcgLBMP4CmdEx->TabIndex = 1;
            this->fcgLBMP4CmdEx->Text = L"拡張オプション";
            // 
            // fcgCBMP4MuxerExt
            // 
            this->fcgCBMP4MuxerExt->AutoSize = true;
            this->fcgCBMP4MuxerExt->Location = System::Drawing::Point(10, 9);
            this->fcgCBMP4MuxerExt->Name = L"fcgCBMP4MuxerExt";
            this->fcgCBMP4MuxerExt->Size = System::Drawing::Size(113, 18);
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
            this->fcgtabPageMKV->Location = System::Drawing::Point(4, 23);
            this->fcgtabPageMKV->Name = L"fcgtabPageMKV";
            this->fcgtabPageMKV->Padding = System::Windows::Forms::Padding(3);
            this->fcgtabPageMKV->Size = System::Drawing::Size(376, 174);
            this->fcgtabPageMKV->TabIndex = 1;
            this->fcgtabPageMKV->Text = L"mkv";
            this->fcgtabPageMKV->UseVisualStyleBackColor = true;
            // 
            // fcgBTMKVMuxerPath
            // 
            this->fcgBTMKVMuxerPath->Location = System::Drawing::Point(340, 76);
            this->fcgBTMKVMuxerPath->Name = L"fcgBTMKVMuxerPath";
            this->fcgBTMKVMuxerPath->Size = System::Drawing::Size(30, 23);
            this->fcgBTMKVMuxerPath->TabIndex = 3;
            this->fcgBTMKVMuxerPath->Text = L"...";
            this->fcgBTMKVMuxerPath->UseVisualStyleBackColor = true;
            this->fcgBTMKVMuxerPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMKVMuxerPath_Click);
            // 
            // fcgTXMKVMuxerPath
            // 
            this->fcgTXMKVMuxerPath->Location = System::Drawing::Point(131, 77);
            this->fcgTXMKVMuxerPath->Name = L"fcgTXMKVMuxerPath";
            this->fcgTXMKVMuxerPath->Size = System::Drawing::Size(207, 21);
            this->fcgTXMKVMuxerPath->TabIndex = 2;
            this->fcgTXMKVMuxerPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMKVMuxerPath_TextChanged);
            this->fcgTXMKVMuxerPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXMKVMuxerPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            // 
            // fcgLBMKVMuxerPath
            // 
            this->fcgLBMKVMuxerPath->AutoSize = true;
            this->fcgLBMKVMuxerPath->Location = System::Drawing::Point(4, 80);
            this->fcgLBMKVMuxerPath->Name = L"fcgLBMKVMuxerPath";
            this->fcgLBMKVMuxerPath->Size = System::Drawing::Size(49, 14);
            this->fcgLBMKVMuxerPath->TabIndex = 19;
            this->fcgLBMKVMuxerPath->Text = L"～の指定";
            // 
            // fcgCXMKVCmdEx
            // 
            this->fcgCXMKVCmdEx->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXMKVCmdEx->FormattingEnabled = true;
            this->fcgCXMKVCmdEx->Location = System::Drawing::Point(213, 43);
            this->fcgCXMKVCmdEx->Name = L"fcgCXMKVCmdEx";
            this->fcgCXMKVCmdEx->Size = System::Drawing::Size(157, 22);
            this->fcgCXMKVCmdEx->TabIndex = 1;
            this->fcgCXMKVCmdEx->Tag = L"chValue";
            // 
            // fcgLBMKVMuxerCmdEx
            // 
            this->fcgLBMKVMuxerCmdEx->AutoSize = true;
            this->fcgLBMKVMuxerCmdEx->Location = System::Drawing::Point(139, 46);
            this->fcgLBMKVMuxerCmdEx->Name = L"fcgLBMKVMuxerCmdEx";
            this->fcgLBMKVMuxerCmdEx->Size = System::Drawing::Size(68, 14);
            this->fcgLBMKVMuxerCmdEx->TabIndex = 17;
            this->fcgLBMKVMuxerCmdEx->Text = L"拡張オプション";
            // 
            // fcgCBMKVMuxerExt
            // 
            this->fcgCBMKVMuxerExt->AutoSize = true;
            this->fcgCBMKVMuxerExt->Location = System::Drawing::Point(10, 45);
            this->fcgCBMKVMuxerExt->Name = L"fcgCBMKVMuxerExt";
            this->fcgCBMKVMuxerExt->Size = System::Drawing::Size(113, 18);
            this->fcgCBMKVMuxerExt->TabIndex = 0;
            this->fcgCBMKVMuxerExt->Tag = L"chValue";
            this->fcgCBMKVMuxerExt->Text = L"外部muxerを使用";
            this->fcgCBMKVMuxerExt->UseVisualStyleBackColor = true;
            // 
            // fcgtabPageMPG
            // 
            this->fcgtabPageMPG->Controls->Add(this->fcgBTMPGMuxerPath);
            this->fcgtabPageMPG->Controls->Add(this->fcgTXMPGMuxerPath);
            this->fcgtabPageMPG->Controls->Add(this->fcgLBMPGMuxerPath);
            this->fcgtabPageMPG->Controls->Add(this->fcgCXMPGCmdEx);
            this->fcgtabPageMPG->Controls->Add(this->label3);
            this->fcgtabPageMPG->Controls->Add(this->fcgCBMPGMuxerExt);
            this->fcgtabPageMPG->Location = System::Drawing::Point(4, 23);
            this->fcgtabPageMPG->Name = L"fcgtabPageMPG";
            this->fcgtabPageMPG->Size = System::Drawing::Size(376, 174);
            this->fcgtabPageMPG->TabIndex = 4;
            this->fcgtabPageMPG->Text = L"mpg";
            this->fcgtabPageMPG->UseVisualStyleBackColor = true;
            // 
            // fcgBTMPGMuxerPath
            // 
            this->fcgBTMPGMuxerPath->Location = System::Drawing::Point(341, 92);
            this->fcgBTMPGMuxerPath->Name = L"fcgBTMPGMuxerPath";
            this->fcgBTMPGMuxerPath->Size = System::Drawing::Size(30, 23);
            this->fcgBTMPGMuxerPath->TabIndex = 23;
            this->fcgBTMPGMuxerPath->Text = L"...";
            this->fcgBTMPGMuxerPath->UseVisualStyleBackColor = true;
            this->fcgBTMPGMuxerPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTMPGMuxerPath_Click);
            // 
            // fcgTXMPGMuxerPath
            // 
            this->fcgTXMPGMuxerPath->Location = System::Drawing::Point(132, 93);
            this->fcgTXMPGMuxerPath->Name = L"fcgTXMPGMuxerPath";
            this->fcgTXMPGMuxerPath->Size = System::Drawing::Size(207, 21);
            this->fcgTXMPGMuxerPath->TabIndex = 22;
            this->fcgTXMPGMuxerPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXMPGMuxerPath_TextChanged);
            this->fcgTXMPGMuxerPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXMPGMuxerPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            // 
            // fcgLBMPGMuxerPath
            // 
            this->fcgLBMPGMuxerPath->AutoSize = true;
            this->fcgLBMPGMuxerPath->Location = System::Drawing::Point(5, 96);
            this->fcgLBMPGMuxerPath->Name = L"fcgLBMPGMuxerPath";
            this->fcgLBMPGMuxerPath->Size = System::Drawing::Size(49, 14);
            this->fcgLBMPGMuxerPath->TabIndex = 25;
            this->fcgLBMPGMuxerPath->Text = L"～の指定";
            // 
            // fcgCXMPGCmdEx
            // 
            this->fcgCXMPGCmdEx->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXMPGCmdEx->FormattingEnabled = true;
            this->fcgCXMPGCmdEx->Location = System::Drawing::Point(214, 59);
            this->fcgCXMPGCmdEx->Name = L"fcgCXMPGCmdEx";
            this->fcgCXMPGCmdEx->Size = System::Drawing::Size(157, 22);
            this->fcgCXMPGCmdEx->TabIndex = 21;
            this->fcgCXMPGCmdEx->Tag = L"chValue";
            // 
            // label3
            // 
            this->label3->AutoSize = true;
            this->label3->Location = System::Drawing::Point(140, 62);
            this->label3->Name = L"label3";
            this->label3->Size = System::Drawing::Size(68, 14);
            this->label3->TabIndex = 24;
            this->label3->Text = L"拡張オプション";
            // 
            // fcgCBMPGMuxerExt
            // 
            this->fcgCBMPGMuxerExt->AutoSize = true;
            this->fcgCBMPGMuxerExt->Location = System::Drawing::Point(11, 61);
            this->fcgCBMPGMuxerExt->Name = L"fcgCBMPGMuxerExt";
            this->fcgCBMPGMuxerExt->Size = System::Drawing::Size(113, 18);
            this->fcgCBMPGMuxerExt->TabIndex = 20;
            this->fcgCBMPGMuxerExt->Tag = L"chValue";
            this->fcgCBMPGMuxerExt->Text = L"外部muxerを使用";
            this->fcgCBMPGMuxerExt->UseVisualStyleBackColor = true;
            // 
            // fcgtabPageMux
            // 
            this->fcgtabPageMux->Controls->Add(this->fcgCXMuxPriority);
            this->fcgtabPageMux->Controls->Add(this->fcgLBMuxPriority);
            this->fcgtabPageMux->Controls->Add(this->fcgCBMuxMinimize);
            this->fcgtabPageMux->Location = System::Drawing::Point(4, 23);
            this->fcgtabPageMux->Name = L"fcgtabPageMux";
            this->fcgtabPageMux->Size = System::Drawing::Size(376, 174);
            this->fcgtabPageMux->TabIndex = 2;
            this->fcgtabPageMux->Text = L"Mux共通設定";
            this->fcgtabPageMux->UseVisualStyleBackColor = true;
            // 
            // fcgCXMuxPriority
            // 
            this->fcgCXMuxPriority->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXMuxPriority->FormattingEnabled = true;
            this->fcgCXMuxPriority->Location = System::Drawing::Point(102, 64);
            this->fcgCXMuxPriority->Name = L"fcgCXMuxPriority";
            this->fcgCXMuxPriority->Size = System::Drawing::Size(198, 22);
            this->fcgCXMuxPriority->TabIndex = 1;
            this->fcgCXMuxPriority->Tag = L"chValue";
            // 
            // fcgLBMuxPriority
            // 
            this->fcgLBMuxPriority->AutoSize = true;
            this->fcgLBMuxPriority->Location = System::Drawing::Point(15, 67);
            this->fcgLBMuxPriority->Name = L"fcgLBMuxPriority";
            this->fcgLBMuxPriority->Size = System::Drawing::Size(62, 14);
            this->fcgLBMuxPriority->TabIndex = 1;
            this->fcgLBMuxPriority->Text = L"Mux優先度";
            // 
            // fcgCBMuxMinimize
            // 
            this->fcgCBMuxMinimize->AutoSize = true;
            this->fcgCBMuxMinimize->Location = System::Drawing::Point(18, 26);
            this->fcgCBMuxMinimize->Name = L"fcgCBMuxMinimize";
            this->fcgCBMuxMinimize->Size = System::Drawing::Size(59, 18);
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
            this->fcgtabPageBat->Location = System::Drawing::Point(4, 23);
            this->fcgtabPageBat->Name = L"fcgtabPageBat";
            this->fcgtabPageBat->Size = System::Drawing::Size(376, 174);
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
            this->fcgLBBatAfterString->Location = System::Drawing::Point(304, 112);
            this->fcgLBBatAfterString->Name = L"fcgLBBatAfterString";
            this->fcgLBBatAfterString->Size = System::Drawing::Size(27, 15);
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
            this->fcgLBBatBeforeString->Location = System::Drawing::Point(304, 14);
            this->fcgLBBatBeforeString->Name = L"fcgLBBatBeforeString";
            this->fcgLBBatBeforeString->Size = System::Drawing::Size(27, 15);
            this->fcgLBBatBeforeString->TabIndex = 19;
            this->fcgLBBatBeforeString->Text = L" 前& ";
            this->fcgLBBatBeforeString->TextAlign = System::Drawing::ContentAlignment::TopCenter;
            // 
            // fcgPNSeparator
            // 
            this->fcgPNSeparator->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->fcgPNSeparator->Location = System::Drawing::Point(18, 88);
            this->fcgPNSeparator->Name = L"fcgPNSeparator";
            this->fcgPNSeparator->Size = System::Drawing::Size(342, 1);
            this->fcgPNSeparator->TabIndex = 18;
            // 
            // fcgBTBatBeforePath
            // 
            this->fcgBTBatBeforePath->Location = System::Drawing::Point(330, 55);
            this->fcgBTBatBeforePath->Name = L"fcgBTBatBeforePath";
            this->fcgBTBatBeforePath->Size = System::Drawing::Size(30, 23);
            this->fcgBTBatBeforePath->TabIndex = 17;
            this->fcgBTBatBeforePath->Tag = L"chValue";
            this->fcgBTBatBeforePath->Text = L"...";
            this->fcgBTBatBeforePath->UseVisualStyleBackColor = true;
            this->fcgBTBatBeforePath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTBatBeforePath_Click);
            // 
            // fcgTXBatBeforePath
            // 
            this->fcgTXBatBeforePath->AllowDrop = true;
            this->fcgTXBatBeforePath->Location = System::Drawing::Point(126, 56);
            this->fcgTXBatBeforePath->Name = L"fcgTXBatBeforePath";
            this->fcgTXBatBeforePath->Size = System::Drawing::Size(202, 21);
            this->fcgTXBatBeforePath->TabIndex = 16;
            this->fcgTXBatBeforePath->Tag = L"chValue";
            this->fcgTXBatBeforePath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXBatBeforePath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            // 
            // fcgLBBatBeforePath
            // 
            this->fcgLBBatBeforePath->AutoSize = true;
            this->fcgLBBatBeforePath->Location = System::Drawing::Point(40, 59);
            this->fcgLBBatBeforePath->Name = L"fcgLBBatBeforePath";
            this->fcgLBBatBeforePath->Size = System::Drawing::Size(61, 14);
            this->fcgLBBatBeforePath->TabIndex = 15;
            this->fcgLBBatBeforePath->Text = L"バッチファイル";
            // 
            // fcgCBWaitForBatBefore
            // 
            this->fcgCBWaitForBatBefore->AutoSize = true;
            this->fcgCBWaitForBatBefore->Location = System::Drawing::Point(40, 30);
            this->fcgCBWaitForBatBefore->Name = L"fcgCBWaitForBatBefore";
            this->fcgCBWaitForBatBefore->Size = System::Drawing::Size(150, 18);
            this->fcgCBWaitForBatBefore->TabIndex = 14;
            this->fcgCBWaitForBatBefore->Tag = L"chValue";
            this->fcgCBWaitForBatBefore->Text = L"バッチ処理の終了を待機する";
            this->fcgCBWaitForBatBefore->UseVisualStyleBackColor = true;
            // 
            // fcgCBRunBatBefore
            // 
            this->fcgCBRunBatBefore->AutoSize = true;
            this->fcgCBRunBatBefore->Location = System::Drawing::Point(18, 6);
            this->fcgCBRunBatBefore->Name = L"fcgCBRunBatBefore";
            this->fcgCBRunBatBefore->Size = System::Drawing::Size(179, 18);
            this->fcgCBRunBatBefore->TabIndex = 13;
            this->fcgCBRunBatBefore->Tag = L"chValue";
            this->fcgCBRunBatBefore->Text = L"エンコード開始前、バッチ処理を行う";
            this->fcgCBRunBatBefore->UseVisualStyleBackColor = true;
            // 
            // fcgBTBatAfterPath
            // 
            this->fcgBTBatAfterPath->Location = System::Drawing::Point(330, 146);
            this->fcgBTBatAfterPath->Name = L"fcgBTBatAfterPath";
            this->fcgBTBatAfterPath->Size = System::Drawing::Size(30, 23);
            this->fcgBTBatAfterPath->TabIndex = 10;
            this->fcgBTBatAfterPath->Tag = L"chValue";
            this->fcgBTBatAfterPath->Text = L"...";
            this->fcgBTBatAfterPath->UseVisualStyleBackColor = true;
            this->fcgBTBatAfterPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTBatAfterPath_Click);
            // 
            // fcgTXBatAfterPath
            // 
            this->fcgTXBatAfterPath->AllowDrop = true;
            this->fcgTXBatAfterPath->Location = System::Drawing::Point(126, 147);
            this->fcgTXBatAfterPath->Name = L"fcgTXBatAfterPath";
            this->fcgTXBatAfterPath->Size = System::Drawing::Size(202, 21);
            this->fcgTXBatAfterPath->TabIndex = 9;
            this->fcgTXBatAfterPath->Tag = L"chValue";
            this->fcgTXBatAfterPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXBatAfterPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            // 
            // fcgLBBatAfterPath
            // 
            this->fcgLBBatAfterPath->AutoSize = true;
            this->fcgLBBatAfterPath->Location = System::Drawing::Point(40, 150);
            this->fcgLBBatAfterPath->Name = L"fcgLBBatAfterPath";
            this->fcgLBBatAfterPath->Size = System::Drawing::Size(61, 14);
            this->fcgLBBatAfterPath->TabIndex = 8;
            this->fcgLBBatAfterPath->Text = L"バッチファイル";
            // 
            // fcgCBWaitForBatAfter
            // 
            this->fcgCBWaitForBatAfter->AutoSize = true;
            this->fcgCBWaitForBatAfter->Location = System::Drawing::Point(40, 122);
            this->fcgCBWaitForBatAfter->Name = L"fcgCBWaitForBatAfter";
            this->fcgCBWaitForBatAfter->Size = System::Drawing::Size(150, 18);
            this->fcgCBWaitForBatAfter->TabIndex = 7;
            this->fcgCBWaitForBatAfter->Tag = L"chValue";
            this->fcgCBWaitForBatAfter->Text = L"バッチ処理の終了を待機する";
            this->fcgCBWaitForBatAfter->UseVisualStyleBackColor = true;
            // 
            // fcgCBRunBatAfter
            // 
            this->fcgCBRunBatAfter->AutoSize = true;
            this->fcgCBRunBatAfter->Location = System::Drawing::Point(18, 98);
            this->fcgCBRunBatAfter->Name = L"fcgCBRunBatAfter";
            this->fcgCBRunBatAfter->Size = System::Drawing::Size(179, 18);
            this->fcgCBRunBatAfter->TabIndex = 6;
            this->fcgCBRunBatAfter->Tag = L"chValue";
            this->fcgCBRunBatAfter->Text = L"エンコード終了後、バッチ処理を行う";
            this->fcgCBRunBatAfter->UseVisualStyleBackColor = true;
            // 
            // fcgtabPageInternal
            // 
            this->fcgtabPageInternal->Controls->Add(this->fcgCXInternalCmdEx);
            this->fcgtabPageInternal->Controls->Add(this->fcgLBInternalCmdEx);
            this->fcgtabPageInternal->Location = System::Drawing::Point(4, 23);
            this->fcgtabPageInternal->Name = L"fcgtabPageInternal";
            this->fcgtabPageInternal->Size = System::Drawing::Size(376, 174);
            this->fcgtabPageInternal->TabIndex = 5;
            this->fcgtabPageInternal->Text = L"mux";
            this->fcgtabPageInternal->UseVisualStyleBackColor = true;
            // 
            // fcgCXInternalCmdEx
            // 
            this->fcgCXInternalCmdEx->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXInternalCmdEx->FormattingEnabled = true;
            this->fcgCXInternalCmdEx->Location = System::Drawing::Point(83, 16);
            this->fcgCXInternalCmdEx->Name = L"fcgCXInternalCmdEx";
            this->fcgCXInternalCmdEx->Size = System::Drawing::Size(157, 22);
            this->fcgCXInternalCmdEx->TabIndex = 4;
            this->fcgCXInternalCmdEx->Tag = L"chValue";
            // 
            // fcgLBInternalCmdEx
            // 
            this->fcgLBInternalCmdEx->AutoSize = true;
            this->fcgLBInternalCmdEx->Location = System::Drawing::Point(9, 19);
            this->fcgLBInternalCmdEx->Name = L"fcgLBInternalCmdEx";
            this->fcgLBInternalCmdEx->Size = System::Drawing::Size(68, 14);
            this->fcgLBInternalCmdEx->TabIndex = 5;
            this->fcgLBInternalCmdEx->Text = L"拡張オプション";
            // 
            // fcgTXCmd
            // 
            this->fcgTXCmd->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left)
                | System::Windows::Forms::AnchorStyles::Right));
            this->fcgTXCmd->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->fcgTXCmd->Location = System::Drawing::Point(9, 545);
            this->fcgTXCmd->Name = L"fcgTXCmd";
            this->fcgTXCmd->ReadOnly = true;
            this->fcgTXCmd->Size = System::Drawing::Size(992, 21);
            this->fcgTXCmd->TabIndex = 4;
            this->fcgTXCmd->DoubleClick += gcnew System::EventHandler(this, &frmConfig::fcgTXCmd_DoubleClick);
            // 
            // fcgBTCancel
            // 
            this->fcgBTCancel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
            this->fcgBTCancel->Location = System::Drawing::Point(771, 570);
            this->fcgBTCancel->Name = L"fcgBTCancel";
            this->fcgBTCancel->Size = System::Drawing::Size(84, 28);
            this->fcgBTCancel->TabIndex = 5;
            this->fcgBTCancel->Text = L"キャンセル";
            this->fcgBTCancel->UseVisualStyleBackColor = true;
            this->fcgBTCancel->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTCancel_Click);
            // 
            // fcgBTOK
            // 
            this->fcgBTOK->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
            this->fcgBTOK->Location = System::Drawing::Point(893, 570);
            this->fcgBTOK->Name = L"fcgBTOK";
            this->fcgBTOK->Size = System::Drawing::Size(84, 28);
            this->fcgBTOK->TabIndex = 6;
            this->fcgBTOK->Text = L"OK";
            this->fcgBTOK->UseVisualStyleBackColor = true;
            this->fcgBTOK->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTOK_Click);
            // 
            // fcgBTDefault
            // 
            this->fcgBTDefault->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
            this->fcgBTDefault->Location = System::Drawing::Point(9, 572);
            this->fcgBTDefault->Name = L"fcgBTDefault";
            this->fcgBTDefault->Size = System::Drawing::Size(112, 28);
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
            this->fcgLBVersionDate->Location = System::Drawing::Point(445, 579);
            this->fcgLBVersionDate->Name = L"fcgLBVersionDate";
            this->fcgLBVersionDate->Size = System::Drawing::Size(47, 14);
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
            this->fcgLBVersion->Location = System::Drawing::Point(136, 579);
            this->fcgLBVersion->Name = L"fcgLBVersion";
            this->fcgLBVersion->Size = System::Drawing::Size(47, 14);
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
            this->fcgtabControlVCE->Controls->Add(this->tabPageExOpt);
            this->fcgtabControlVCE->Location = System::Drawing::Point(4, 31);
            this->fcgtabControlVCE->Name = L"fcgtabControlVCE";
            this->fcgtabControlVCE->SelectedIndex = 0;
            this->fcgtabControlVCE->Size = System::Drawing::Size(616, 509);
            this->fcgtabControlVCE->TabIndex = 49;
            // 
            // tabPageVideoEnc
            // 
            this->tabPageVideoEnc->Controls->Add(this->fcgPNQP);
            this->tabPageVideoEnc->Controls->Add(this->fcgCXVideoFormat);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBVideoFormat);
            this->tabPageVideoEnc->Controls->Add(this->fcggroupBoxColor);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBQPMin);
            this->tabPageVideoEnc->Controls->Add(this->fcgCBPreAnalysis);
            this->tabPageVideoEnc->Controls->Add(this->fcgBTVideoEncoderPath);
            this->tabPageVideoEnc->Controls->Add(this->fcgTXVideoEncoderPath);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBVideoEncoderPath);
            this->tabPageVideoEnc->Controls->Add(this->fcgNURefFrames);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBRefFrames);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBCodec);
            this->tabPageVideoEnc->Controls->Add(this->fcgCXCodec);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBPreAnalysis);
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
            this->tabPageVideoEnc->Controls->Add(this->fcgCXMotionEst);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBMotionEst);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBDeblock);
            this->tabPageVideoEnc->Controls->Add(this->fcgpictureBoxVCEEnabled);
            this->tabPageVideoEnc->Controls->Add(this->fcgpictureBoxVCEDisabled);
            this->tabPageVideoEnc->Controls->Add(this->fcgCBDeblock);
            this->tabPageVideoEnc->Controls->Add(this->fcgNUSlices);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBSlices);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBGOPLengthAuto);
            this->tabPageVideoEnc->Controls->Add(this->fcgGroupBoxAspectRatio);
            this->tabPageVideoEnc->Controls->Add(this->fcgCXInterlaced);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBInterlaced);
            this->tabPageVideoEnc->Controls->Add(this->fcgNUGopLength);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBGOPLength);
            this->tabPageVideoEnc->Controls->Add(this->fcgCXCodecLevel);
            this->tabPageVideoEnc->Controls->Add(this->fcgCXCodecProfile);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBCodecLevel);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBCodecProfile);
            this->tabPageVideoEnc->Controls->Add(this->fcgLBEncMode);
            this->tabPageVideoEnc->Controls->Add(this->fcgCXEncMode);
            this->tabPageVideoEnc->Controls->Add(this->fcgPNBitrate);
            this->tabPageVideoEnc->Controls->Add(this->fcgPNBframes);
            this->tabPageVideoEnc->Controls->Add(this->fcgPNHEVCLevelProfile);
            this->tabPageVideoEnc->Controls->Add(this->fcgPNH264Features);
            this->tabPageVideoEnc->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->tabPageVideoEnc->Location = System::Drawing::Point(4, 24);
            this->tabPageVideoEnc->Name = L"tabPageVideoEnc";
            this->tabPageVideoEnc->Padding = System::Windows::Forms::Padding(3);
            this->tabPageVideoEnc->Size = System::Drawing::Size(608, 481);
            this->tabPageVideoEnc->TabIndex = 0;
            this->tabPageVideoEnc->Text = L"動画エンコード";
            this->tabPageVideoEnc->UseVisualStyleBackColor = true;
            // 
            // fcgPNQP
            // 
            this->fcgPNQP->Controls->Add(this->fcgLBQPI);
            this->fcgPNQP->Controls->Add(this->fcgNUQPI);
            this->fcgPNQP->Controls->Add(this->fcgNUQPP);
            this->fcgPNQP->Controls->Add(this->fcgNUQPB);
            this->fcgPNQP->Controls->Add(this->fcgLBQPP);
            this->fcgPNQP->Controls->Add(this->fcgLBQPB);
            this->fcgPNQP->Location = System::Drawing::Point(8, 199);
            this->fcgPNQP->Name = L"fcgPNQP";
            this->fcgPNQP->Size = System::Drawing::Size(289, 79);
            this->fcgPNQP->TabIndex = 20;
            // 
            // fcgLBQPI
            // 
            this->fcgLBQPI->AutoSize = true;
            this->fcgLBQPI->Location = System::Drawing::Point(10, 4);
            this->fcgLBQPI->Name = L"fcgLBQPI";
            this->fcgLBQPI->Size = System::Drawing::Size(66, 14);
            this->fcgLBQPI->TabIndex = 21;
            this->fcgLBQPI->Text = L"QP I frame";
            // 
            // fcgNUQPI
            // 
            this->fcgNUQPI->Location = System::Drawing::Point(124, 2);
            this->fcgNUQPI->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 51, 0, 0, 0 });
            this->fcgNUQPI->Name = L"fcgNUQPI";
            this->fcgNUQPI->Size = System::Drawing::Size(77, 21);
            this->fcgNUQPI->TabIndex = 22;
            this->fcgNUQPI->Tag = L"reCmd";
            this->fcgNUQPI->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUQPP
            // 
            this->fcgNUQPP->Location = System::Drawing::Point(124, 29);
            this->fcgNUQPP->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 51, 0, 0, 0 });
            this->fcgNUQPP->Name = L"fcgNUQPP";
            this->fcgNUQPP->Size = System::Drawing::Size(77, 21);
            this->fcgNUQPP->TabIndex = 24;
            this->fcgNUQPP->Tag = L"reCmd";
            this->fcgNUQPP->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUQPB
            // 
            this->fcgNUQPB->Location = System::Drawing::Point(124, 55);
            this->fcgNUQPB->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 51, 0, 0, 0 });
            this->fcgNUQPB->Name = L"fcgNUQPB";
            this->fcgNUQPB->Size = System::Drawing::Size(77, 21);
            this->fcgNUQPB->TabIndex = 26;
            this->fcgNUQPB->Tag = L"reCmd";
            this->fcgNUQPB->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBQPP
            // 
            this->fcgLBQPP->AutoSize = true;
            this->fcgLBQPP->Location = System::Drawing::Point(10, 31);
            this->fcgLBQPP->Name = L"fcgLBQPP";
            this->fcgLBQPP->Size = System::Drawing::Size(69, 14);
            this->fcgLBQPP->TabIndex = 23;
            this->fcgLBQPP->Text = L"QP P frame";
            // 
            // fcgLBQPB
            // 
            this->fcgLBQPB->AutoSize = true;
            this->fcgLBQPB->Location = System::Drawing::Point(10, 57);
            this->fcgLBQPB->Name = L"fcgLBQPB";
            this->fcgLBQPB->Size = System::Drawing::Size(74, 14);
            this->fcgLBQPB->TabIndex = 25;
            this->fcgLBQPB->Text = L"QP B frames";
            // 
            // fcgCXVideoFormat
            // 
            this->fcgCXVideoFormat->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVideoFormat->FormattingEnabled = true;
            this->fcgCXVideoFormat->Location = System::Drawing::Point(458, 330);
            this->fcgCXVideoFormat->Name = L"fcgCXVideoFormat";
            this->fcgCXVideoFormat->Size = System::Drawing::Size(121, 22);
            this->fcgCXVideoFormat->TabIndex = 141;
            this->fcgCXVideoFormat->Tag = L"reCmd";
            // 
            // fcgLBVideoFormat
            // 
            this->fcgLBVideoFormat->AutoSize = true;
            this->fcgLBVideoFormat->Location = System::Drawing::Point(360, 333);
            this->fcgLBVideoFormat->Name = L"fcgLBVideoFormat";
            this->fcgLBVideoFormat->Size = System::Drawing::Size(73, 14);
            this->fcgLBVideoFormat->TabIndex = 140;
            this->fcgLBVideoFormat->Text = L"videoformat";
            // 
            // fcggroupBoxColor
            // 
            this->fcggroupBoxColor->Controls->Add(this->fcgCBFullrange);
            this->fcggroupBoxColor->Controls->Add(this->fcgLBFullrange);
            this->fcggroupBoxColor->Controls->Add(this->fcgCXTransfer);
            this->fcggroupBoxColor->Controls->Add(this->fcgCXColorPrim);
            this->fcggroupBoxColor->Controls->Add(this->fcgCXColorMatrix);
            this->fcggroupBoxColor->Controls->Add(this->fcgLBTransfer);
            this->fcggroupBoxColor->Controls->Add(this->fcgLBColorPrim);
            this->fcggroupBoxColor->Controls->Add(this->fcgLBColorMatrix);
            this->fcggroupBoxColor->Location = System::Drawing::Point(354, 355);
            this->fcggroupBoxColor->Name = L"fcggroupBoxColor";
            this->fcggroupBoxColor->Size = System::Drawing::Size(241, 121);
            this->fcggroupBoxColor->TabIndex = 142;
            this->fcggroupBoxColor->TabStop = false;
            this->fcggroupBoxColor->Text = L"色設定";
            // 
            // fcgCBFullrange
            // 
            this->fcgCBFullrange->AutoSize = true;
            this->fcgCBFullrange->Location = System::Drawing::Point(106, 101);
            this->fcgCBFullrange->Name = L"fcgCBFullrange";
            this->fcgCBFullrange->Size = System::Drawing::Size(15, 14);
            this->fcgCBFullrange->TabIndex = 176;
            this->fcgCBFullrange->Tag = L"reCmd";
            this->fcgCBFullrange->UseVisualStyleBackColor = true;
            // 
            // fcgLBFullrange
            // 
            this->fcgLBFullrange->AutoSize = true;
            this->fcgLBFullrange->Location = System::Drawing::Point(18, 100);
            this->fcgLBFullrange->Name = L"fcgLBFullrange";
            this->fcgLBFullrange->Size = System::Drawing::Size(55, 14);
            this->fcgLBFullrange->TabIndex = 175;
            this->fcgLBFullrange->Text = L"fullrange";
            // 
            // fcgCXTransfer
            // 
            this->fcgCXTransfer->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXTransfer->FormattingEnabled = true;
            this->fcgCXTransfer->Location = System::Drawing::Point(105, 72);
            this->fcgCXTransfer->Name = L"fcgCXTransfer";
            this->fcgCXTransfer->Size = System::Drawing::Size(121, 22);
            this->fcgCXTransfer->TabIndex = 2;
            this->fcgCXTransfer->Tag = L"reCmd";
            // 
            // fcgCXColorPrim
            // 
            this->fcgCXColorPrim->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXColorPrim->FormattingEnabled = true;
            this->fcgCXColorPrim->Location = System::Drawing::Point(105, 44);
            this->fcgCXColorPrim->Name = L"fcgCXColorPrim";
            this->fcgCXColorPrim->Size = System::Drawing::Size(121, 22);
            this->fcgCXColorPrim->TabIndex = 1;
            this->fcgCXColorPrim->Tag = L"reCmd";
            // 
            // fcgCXColorMatrix
            // 
            this->fcgCXColorMatrix->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXColorMatrix->FormattingEnabled = true;
            this->fcgCXColorMatrix->Location = System::Drawing::Point(105, 16);
            this->fcgCXColorMatrix->Name = L"fcgCXColorMatrix";
            this->fcgCXColorMatrix->Size = System::Drawing::Size(121, 22);
            this->fcgCXColorMatrix->TabIndex = 0;
            this->fcgCXColorMatrix->Tag = L"reCmd";
            // 
            // fcgLBTransfer
            // 
            this->fcgLBTransfer->AutoSize = true;
            this->fcgLBTransfer->Location = System::Drawing::Point(18, 75);
            this->fcgLBTransfer->Name = L"fcgLBTransfer";
            this->fcgLBTransfer->Size = System::Drawing::Size(49, 14);
            this->fcgLBTransfer->TabIndex = 2;
            this->fcgLBTransfer->Text = L"transfer";
            // 
            // fcgLBColorPrim
            // 
            this->fcgLBColorPrim->AutoSize = true;
            this->fcgLBColorPrim->Location = System::Drawing::Point(18, 47);
            this->fcgLBColorPrim->Name = L"fcgLBColorPrim";
            this->fcgLBColorPrim->Size = System::Drawing::Size(61, 14);
            this->fcgLBColorPrim->TabIndex = 1;
            this->fcgLBColorPrim->Text = L"colorprim";
            // 
            // fcgLBColorMatrix
            // 
            this->fcgLBColorMatrix->AutoSize = true;
            this->fcgLBColorMatrix->Location = System::Drawing::Point(18, 19);
            this->fcgLBColorMatrix->Name = L"fcgLBColorMatrix";
            this->fcgLBColorMatrix->Size = System::Drawing::Size(70, 14);
            this->fcgLBColorMatrix->TabIndex = 0;
            this->fcgLBColorMatrix->Text = L"colormatrix";
            // 
            // fcgLBQPMin
            // 
            this->fcgLBQPMin->AutoSize = true;
            this->fcgLBQPMin->Location = System::Drawing::Point(97, 367);
            this->fcgLBQPMin->Name = L"fcgLBQPMin";
            this->fcgLBQPMin->Size = System::Drawing::Size(29, 14);
            this->fcgLBQPMin->TabIndex = 39;
            this->fcgLBQPMin->Text = L"下限";
            // 
            // fcgCBPreAnalysis
            // 
            this->fcgCBPreAnalysis->AutoSize = true;
            this->fcgCBPreAnalysis->Location = System::Drawing::Point(459, 231);
            this->fcgCBPreAnalysis->Name = L"fcgCBPreAnalysis";
            this->fcgCBPreAnalysis->Size = System::Drawing::Size(15, 14);
            this->fcgCBPreAnalysis->TabIndex = 123;
            this->fcgCBPreAnalysis->Tag = L"reCmd";
            this->fcgCBPreAnalysis->UseVisualStyleBackColor = true;
            // 
            // fcgBTVideoEncoderPath
            // 
            this->fcgBTVideoEncoderPath->Location = System::Drawing::Point(274, 78);
            this->fcgBTVideoEncoderPath->Name = L"fcgBTVideoEncoderPath";
            this->fcgBTVideoEncoderPath->Size = System::Drawing::Size(30, 23);
            this->fcgBTVideoEncoderPath->TabIndex = 2;
            this->fcgBTVideoEncoderPath->Text = L"...";
            this->fcgBTVideoEncoderPath->UseVisualStyleBackColor = true;
            this->fcgBTVideoEncoderPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTVideoEncoderPath_Click);
            // 
            // fcgTXVideoEncoderPath
            // 
            this->fcgTXVideoEncoderPath->AllowDrop = true;
            this->fcgTXVideoEncoderPath->Location = System::Drawing::Point(21, 80);
            this->fcgTXVideoEncoderPath->Name = L"fcgTXVideoEncoderPath";
            this->fcgTXVideoEncoderPath->Size = System::Drawing::Size(248, 21);
            this->fcgTXVideoEncoderPath->TabIndex = 1;
            this->fcgTXVideoEncoderPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXVideoEncoderPath_TextChanged);
            this->fcgTXVideoEncoderPath->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_DragDrop);
            this->fcgTXVideoEncoderPath->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &frmConfig::fcgSetDragDropFilename_Enter);
            // 
            // fcgLBVideoEncoderPath
            // 
            this->fcgLBVideoEncoderPath->AutoSize = true;
            this->fcgLBVideoEncoderPath->Location = System::Drawing::Point(9, 62);
            this->fcgLBVideoEncoderPath->Name = L"fcgLBVideoEncoderPath";
            this->fcgLBVideoEncoderPath->Size = System::Drawing::Size(49, 14);
            this->fcgLBVideoEncoderPath->TabIndex = 0;
            this->fcgLBVideoEncoderPath->Text = L"～の指定";
            // 
            // fcgNURefFrames
            // 
            this->fcgNURefFrames->Location = System::Drawing::Point(133, 310);
            this->fcgNURefFrames->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16, 0, 0, 0 });
            this->fcgNURefFrames->Name = L"fcgNURefFrames";
            this->fcgNURefFrames->Size = System::Drawing::Size(76, 21);
            this->fcgNURefFrames->TabIndex = 34;
            this->fcgNURefFrames->Tag = L"reCmd";
            this->fcgNURefFrames->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNURefFrames->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            // 
            // fcgLBRefFrames
            // 
            this->fcgLBRefFrames->AutoSize = true;
            this->fcgLBRefFrames->Location = System::Drawing::Point(14, 312);
            this->fcgLBRefFrames->Name = L"fcgLBRefFrames";
            this->fcgLBRefFrames->Size = System::Drawing::Size(51, 14);
            this->fcgLBRefFrames->TabIndex = 33;
            this->fcgLBRefFrames->Text = L"参照距離";
            // 
            // fcgLBCodec
            // 
            this->fcgLBCodec->AutoSize = true;
            this->fcgLBCodec->Location = System::Drawing::Point(13, 117);
            this->fcgLBCodec->Name = L"fcgLBCodec";
            this->fcgLBCodec->Size = System::Drawing::Size(47, 14);
            this->fcgLBCodec->TabIndex = 3;
            this->fcgLBCodec->Text = L"コーデック";
            // 
            // fcgCXCodec
            // 
            this->fcgCXCodec->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXCodec->FormattingEnabled = true;
            this->fcgCXCodec->Items->AddRange(gcnew cli::array< System::Object ^  >(3) { L"高品質", L"標準", L"高速" });
            this->fcgCXCodec->Location = System::Drawing::Point(81, 114);
            this->fcgCXCodec->Name = L"fcgCXCodec";
            this->fcgCXCodec->Size = System::Drawing::Size(160, 22);
            this->fcgCXCodec->TabIndex = 4;
            this->fcgCXCodec->Tag = L"reCmd";
            this->fcgCXCodec->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXCodec_SelectedIndexChanged);
            // 
            // fcgLBPreAnalysis
            // 
            this->fcgLBPreAnalysis->AutoSize = true;
            this->fcgLBPreAnalysis->Location = System::Drawing::Point(359, 231);
            this->fcgLBPreAnalysis->Name = L"fcgLBPreAnalysis";
            this->fcgLBPreAnalysis->Size = System::Drawing::Size(51, 14);
            this->fcgLBPreAnalysis->TabIndex = 122;
            this->fcgLBPreAnalysis->Text = L"先行探索";
            // 
            // fcgNUVBVBufSize
            // 
            this->fcgNUVBVBufSize->Location = System::Drawing::Point(132, 283);
            this->fcgNUVBVBufSize->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 100000, 0, 0, 0 });
            this->fcgNUVBVBufSize->Name = L"fcgNUVBVBufSize";
            this->fcgNUVBVBufSize->Size = System::Drawing::Size(77, 21);
            this->fcgNUVBVBufSize->TabIndex = 31;
            this->fcgNUVBVBufSize->Tag = L"reCmd";
            this->fcgNUVBVBufSize->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBSkipFrame
            // 
            this->fcgLBSkipFrame->AutoSize = true;
            this->fcgLBSkipFrame->Location = System::Drawing::Point(359, 278);
            this->fcgLBSkipFrame->Name = L"fcgLBSkipFrame";
            this->fcgLBSkipFrame->Size = System::Drawing::Size(71, 14);
            this->fcgLBSkipFrame->TabIndex = 126;
            this->fcgLBSkipFrame->Text = L"スキップフレーム";
            // 
            // fcgLBVBVBufSize
            // 
            this->fcgLBVBVBufSize->AutoSize = true;
            this->fcgLBVBVBufSize->Location = System::Drawing::Point(14, 287);
            this->fcgLBVBVBufSize->Name = L"fcgLBVBVBufSize";
            this->fcgLBVBVBufSize->Size = System::Drawing::Size(84, 14);
            this->fcgLBVBVBufSize->TabIndex = 30;
            this->fcgLBVBVBufSize->Text = L"VBVバッファサイズ";
            // 
            // fcgLBVBVBufSizeKbps
            // 
            this->fcgLBVBVBufSizeKbps->AutoSize = true;
            this->fcgLBVBVBufSizeKbps->Location = System::Drawing::Point(216, 285);
            this->fcgLBVBVBufSizeKbps->Name = L"fcgLBVBVBufSizeKbps";
            this->fcgLBVBVBufSizeKbps->Size = System::Drawing::Size(32, 14);
            this->fcgLBVBVBufSizeKbps->TabIndex = 32;
            this->fcgLBVBVBufSizeKbps->Text = L"kbps";
            // 
            // fcgCBSkipFrame
            // 
            this->fcgCBSkipFrame->AutoSize = true;
            this->fcgCBSkipFrame->Location = System::Drawing::Point(459, 279);
            this->fcgCBSkipFrame->Name = L"fcgCBSkipFrame";
            this->fcgCBSkipFrame->Size = System::Drawing::Size(15, 14);
            this->fcgCBSkipFrame->TabIndex = 127;
            this->fcgCBSkipFrame->Tag = L"reCmd";
            this->fcgCBSkipFrame->UseVisualStyleBackColor = true;
            // 
            // fcgCBAFS
            // 
            this->fcgCBAFS->AutoSize = true;
            this->fcgCBAFS->Location = System::Drawing::Point(362, 5);
            this->fcgCBAFS->Name = L"fcgCBAFS";
            this->fcgCBAFS->Size = System::Drawing::Size(183, 18);
            this->fcgCBAFS->TabIndex = 70;
            this->fcgCBAFS->Tag = L"reCmd";
            this->fcgCBAFS->Text = L"自動フィールドシフト(afs)を使用する";
            this->fcgCBAFS->UseVisualStyleBackColor = true;
            // 
            // fcgNUQPMax
            // 
            this->fcgNUQPMax->Location = System::Drawing::Point(235, 364);
            this->fcgNUQPMax->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 51, 0, 0, 0 });
            this->fcgNUQPMax->Name = L"fcgNUQPMax";
            this->fcgNUQPMax->Size = System::Drawing::Size(58, 21);
            this->fcgNUQPMax->TabIndex = 42;
            this->fcgNUQPMax->Tag = L"reCmd";
            this->fcgNUQPMax->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBQPMax
            // 
            this->fcgLBQPMax->AutoSize = true;
            this->fcgLBQPMax->Location = System::Drawing::Point(199, 367);
            this->fcgLBQPMax->Name = L"fcgLBQPMax";
            this->fcgLBQPMax->Size = System::Drawing::Size(29, 14);
            this->fcgLBQPMax->TabIndex = 41;
            this->fcgLBQPMax->Text = L"上限";
            // 
            // fcgLBQPMinMAX
            // 
            this->fcgLBQPMinMAX->AutoSize = true;
            this->fcgLBQPMinMAX->Location = System::Drawing::Point(14, 367);
            this->fcgLBQPMinMAX->Name = L"fcgLBQPMinMAX";
            this->fcgLBQPMinMAX->Size = System::Drawing::Size(48, 14);
            this->fcgLBQPMinMAX->TabIndex = 38;
            this->fcgLBQPMinMAX->Text = L"QP 最小";
            // 
            // fcgNUQPMin
            // 
            this->fcgNUQPMin->Location = System::Drawing::Point(132, 364);
            this->fcgNUQPMin->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 51, 0, 0, 0 });
            this->fcgNUQPMin->Name = L"fcgNUQPMin";
            this->fcgNUQPMin->Size = System::Drawing::Size(53, 21);
            this->fcgNUQPMin->TabIndex = 40;
            this->fcgNUQPMin->Tag = L"reCmd";
            this->fcgNUQPMin->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBQualityPreset
            // 
            this->fcgLBQualityPreset->AutoSize = true;
            this->fcgLBQualityPreset->Location = System::Drawing::Point(13, 173);
            this->fcgLBQualityPreset->Name = L"fcgLBQualityPreset";
            this->fcgLBQualityPreset->Size = System::Drawing::Size(29, 14);
            this->fcgLBQualityPreset->TabIndex = 7;
            this->fcgLBQualityPreset->Text = L"品質";
            // 
            // fcgCXQualityPreset
            // 
            this->fcgCXQualityPreset->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXQualityPreset->FormattingEnabled = true;
            this->fcgCXQualityPreset->Items->AddRange(gcnew cli::array< System::Object ^  >(3) { L"高品質", L"標準", L"高速" });
            this->fcgCXQualityPreset->Location = System::Drawing::Point(81, 170);
            this->fcgCXQualityPreset->Name = L"fcgCXQualityPreset";
            this->fcgCXQualityPreset->Size = System::Drawing::Size(160, 22);
            this->fcgCXQualityPreset->TabIndex = 8;
            this->fcgCXQualityPreset->Tag = L"reCmd";
            // 
            // fcgCXMotionEst
            // 
            this->fcgCXMotionEst->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXMotionEst->FormattingEnabled = true;
            this->fcgCXMotionEst->Location = System::Drawing::Point(81, 454);
            this->fcgCXMotionEst->Name = L"fcgCXMotionEst";
            this->fcgCXMotionEst->Size = System::Drawing::Size(160, 22);
            this->fcgCXMotionEst->TabIndex = 61;
            this->fcgCXMotionEst->Tag = L"reCmd";
            // 
            // fcgLBMotionEst
            // 
            this->fcgLBMotionEst->AutoSize = true;
            this->fcgLBMotionEst->Location = System::Drawing::Point(13, 457);
            this->fcgLBMotionEst->Name = L"fcgLBMotionEst";
            this->fcgLBMotionEst->Size = System::Drawing::Size(48, 14);
            this->fcgLBMotionEst->TabIndex = 60;
            this->fcgLBMotionEst->Text = L"動き予測";
            // 
            // fcgLBDeblock
            // 
            this->fcgLBDeblock->AutoSize = true;
            this->fcgLBDeblock->Location = System::Drawing::Point(360, 254);
            this->fcgLBDeblock->Name = L"fcgLBDeblock";
            this->fcgLBDeblock->Size = System::Drawing::Size(76, 14);
            this->fcgLBDeblock->TabIndex = 124;
            this->fcgLBDeblock->Text = L"デブロックフィルタ";
            // 
            // fcgpictureBoxVCEEnabled
            // 
            this->fcgpictureBoxVCEEnabled->Image = (cli::safe_cast<System::Drawing::Image ^>(resources->GetObject(L"fcgpictureBoxVCEEnabled.Image")));
            this->fcgpictureBoxVCEEnabled->Location = System::Drawing::Point(16, 5);
            this->fcgpictureBoxVCEEnabled->Name = L"fcgpictureBoxVCEEnabled";
            this->fcgpictureBoxVCEEnabled->Size = System::Drawing::Size(153, 53);
            this->fcgpictureBoxVCEEnabled->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
            this->fcgpictureBoxVCEEnabled->TabIndex = 133;
            this->fcgpictureBoxVCEEnabled->TabStop = false;
            // 
            // fcgpictureBoxVCEDisabled
            // 
            this->fcgpictureBoxVCEDisabled->Image = (cli::safe_cast<System::Drawing::Image ^>(resources->GetObject(L"fcgpictureBoxVCEDisabled.Image")));
            this->fcgpictureBoxVCEDisabled->Location = System::Drawing::Point(16, 5);
            this->fcgpictureBoxVCEDisabled->Name = L"fcgpictureBoxVCEDisabled";
            this->fcgpictureBoxVCEDisabled->Size = System::Drawing::Size(153, 53);
            this->fcgpictureBoxVCEDisabled->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
            this->fcgpictureBoxVCEDisabled->TabIndex = 130;
            this->fcgpictureBoxVCEDisabled->TabStop = false;
            // 
            // fcgCBDeblock
            // 
            this->fcgCBDeblock->AutoSize = true;
            this->fcgCBDeblock->Location = System::Drawing::Point(459, 255);
            this->fcgCBDeblock->Name = L"fcgCBDeblock";
            this->fcgCBDeblock->Size = System::Drawing::Size(15, 14);
            this->fcgCBDeblock->TabIndex = 125;
            this->fcgCBDeblock->Tag = L"reCmd";
            this->fcgCBDeblock->UseVisualStyleBackColor = true;
            // 
            // fcgNUSlices
            // 
            this->fcgNUSlices->Location = System::Drawing::Point(459, 203);
            this->fcgNUSlices->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            this->fcgNUSlices->Name = L"fcgNUSlices";
            this->fcgNUSlices->Size = System::Drawing::Size(70, 21);
            this->fcgNUSlices->TabIndex = 121;
            this->fcgNUSlices->Tag = L"reCmd";
            this->fcgNUSlices->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUSlices->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1, 0, 0, 0 });
            // 
            // fcgLBSlices
            // 
            this->fcgLBSlices->AutoSize = true;
            this->fcgLBSlices->Location = System::Drawing::Point(359, 205);
            this->fcgLBSlices->Name = L"fcgLBSlices";
            this->fcgLBSlices->Size = System::Drawing::Size(50, 14);
            this->fcgLBSlices->TabIndex = 120;
            this->fcgLBSlices->Text = L"スライス数";
            // 
            // fcgLBGOPLengthAuto
            // 
            this->fcgLBGOPLengthAuto->AutoSize = true;
            this->fcgLBGOPLengthAuto->Location = System::Drawing::Point(214, 340);
            this->fcgLBGOPLengthAuto->Name = L"fcgLBGOPLengthAuto";
            this->fcgLBGOPLengthAuto->Size = System::Drawing::Size(66, 14);
            this->fcgLBGOPLengthAuto->TabIndex = 37;
            this->fcgLBGOPLengthAuto->Text = L"※\"0\"で自動";
            // 
            // fcgGroupBoxAspectRatio
            // 
            this->fcgGroupBoxAspectRatio->Controls->Add(this->fcgLBAspectRatio);
            this->fcgGroupBoxAspectRatio->Controls->Add(this->fcgNUAspectRatioY);
            this->fcgGroupBoxAspectRatio->Controls->Add(this->fcgNUAspectRatioX);
            this->fcgGroupBoxAspectRatio->Controls->Add(this->fcgCXAspectRatio);
            this->fcgGroupBoxAspectRatio->Location = System::Drawing::Point(362, 27);
            this->fcgGroupBoxAspectRatio->Name = L"fcgGroupBoxAspectRatio";
            this->fcgGroupBoxAspectRatio->Size = System::Drawing::Size(219, 76);
            this->fcgGroupBoxAspectRatio->TabIndex = 80;
            this->fcgGroupBoxAspectRatio->TabStop = false;
            this->fcgGroupBoxAspectRatio->Text = L"アスペクト比";
            // 
            // fcgLBAspectRatio
            // 
            this->fcgLBAspectRatio->AutoSize = true;
            this->fcgLBAspectRatio->Location = System::Drawing::Point(102, 48);
            this->fcgLBAspectRatio->Name = L"fcgLBAspectRatio";
            this->fcgLBAspectRatio->Size = System::Drawing::Size(12, 14);
            this->fcgLBAspectRatio->TabIndex = 3;
            this->fcgLBAspectRatio->Text = L":";
            // 
            // fcgNUAspectRatioY
            // 
            this->fcgNUAspectRatioY->Location = System::Drawing::Point(120, 46);
            this->fcgNUAspectRatioY->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65535, 0, 0, 0 });
            this->fcgNUAspectRatioY->Name = L"fcgNUAspectRatioY";
            this->fcgNUAspectRatioY->Size = System::Drawing::Size(70, 21);
            this->fcgNUAspectRatioY->TabIndex = 2;
            this->fcgNUAspectRatioY->Tag = L"reCmd";
            this->fcgNUAspectRatioY->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUAspectRatioX
            // 
            this->fcgNUAspectRatioX->Location = System::Drawing::Point(26, 46);
            this->fcgNUAspectRatioX->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65535, 0, 0, 0 });
            this->fcgNUAspectRatioX->Name = L"fcgNUAspectRatioX";
            this->fcgNUAspectRatioX->Size = System::Drawing::Size(70, 21);
            this->fcgNUAspectRatioX->TabIndex = 1;
            this->fcgNUAspectRatioX->Tag = L"reCmd";
            this->fcgNUAspectRatioX->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCXAspectRatio
            // 
            this->fcgCXAspectRatio->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAspectRatio->FormattingEnabled = true;
            this->fcgCXAspectRatio->Location = System::Drawing::Point(26, 20);
            this->fcgCXAspectRatio->Name = L"fcgCXAspectRatio";
            this->fcgCXAspectRatio->Size = System::Drawing::Size(174, 22);
            this->fcgCXAspectRatio->TabIndex = 0;
            this->fcgCXAspectRatio->Tag = L"reCmd";
            // 
            // fcgCXInterlaced
            // 
            this->fcgCXInterlaced->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXInterlaced->FormattingEnabled = true;
            this->fcgCXInterlaced->Location = System::Drawing::Point(458, 109);
            this->fcgCXInterlaced->Name = L"fcgCXInterlaced";
            this->fcgCXInterlaced->Size = System::Drawing::Size(121, 22);
            this->fcgCXInterlaced->TabIndex = 91;
            this->fcgCXInterlaced->Tag = L"reCmd";
            // 
            // fcgLBInterlaced
            // 
            this->fcgLBInterlaced->AutoSize = true;
            this->fcgLBInterlaced->Location = System::Drawing::Point(359, 112);
            this->fcgLBInterlaced->Name = L"fcgLBInterlaced";
            this->fcgLBInterlaced->Size = System::Drawing::Size(64, 14);
            this->fcgLBInterlaced->TabIndex = 90;
            this->fcgLBInterlaced->Text = L"フレームタイプ";
            // 
            // fcgNUGopLength
            // 
            this->fcgNUGopLength->Location = System::Drawing::Point(132, 337);
            this->fcgNUGopLength->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1000, 0, 0, 0 });
            this->fcgNUGopLength->Name = L"fcgNUGopLength";
            this->fcgNUGopLength->Size = System::Drawing::Size(77, 21);
            this->fcgNUGopLength->TabIndex = 36;
            this->fcgNUGopLength->Tag = L"reCmd";
            this->fcgNUGopLength->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBGOPLength
            // 
            this->fcgLBGOPLength->AutoSize = true;
            this->fcgLBGOPLength->Location = System::Drawing::Point(14, 340);
            this->fcgLBGOPLength->Name = L"fcgLBGOPLength";
            this->fcgLBGOPLength->Size = System::Drawing::Size(41, 14);
            this->fcgLBGOPLength->TabIndex = 35;
            this->fcgLBGOPLength->Text = L"GOP長";
            // 
            // fcgCXCodecLevel
            // 
            this->fcgCXCodecLevel->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXCodecLevel->FormattingEnabled = true;
            this->fcgCXCodecLevel->Location = System::Drawing::Point(458, 171);
            this->fcgCXCodecLevel->Name = L"fcgCXCodecLevel";
            this->fcgCXCodecLevel->Size = System::Drawing::Size(121, 22);
            this->fcgCXCodecLevel->TabIndex = 114;
            this->fcgCXCodecLevel->Tag = L"reCmd";
            // 
            // fcgCXCodecProfile
            // 
            this->fcgCXCodecProfile->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXCodecProfile->FormattingEnabled = true;
            this->fcgCXCodecProfile->Location = System::Drawing::Point(458, 139);
            this->fcgCXCodecProfile->Name = L"fcgCXCodecProfile";
            this->fcgCXCodecProfile->Size = System::Drawing::Size(121, 22);
            this->fcgCXCodecProfile->TabIndex = 112;
            this->fcgCXCodecProfile->Tag = L"reCmd";
            // 
            // fcgLBCodecLevel
            // 
            this->fcgLBCodecLevel->AutoSize = true;
            this->fcgLBCodecLevel->Location = System::Drawing::Point(359, 174);
            this->fcgLBCodecLevel->Name = L"fcgLBCodecLevel";
            this->fcgLBCodecLevel->Size = System::Drawing::Size(33, 14);
            this->fcgLBCodecLevel->TabIndex = 113;
            this->fcgLBCodecLevel->Text = L"レベル";
            // 
            // fcgLBCodecProfile
            // 
            this->fcgLBCodecProfile->AutoSize = true;
            this->fcgLBCodecProfile->Location = System::Drawing::Point(359, 142);
            this->fcgLBCodecProfile->Name = L"fcgLBCodecProfile";
            this->fcgLBCodecProfile->Size = System::Drawing::Size(53, 14);
            this->fcgLBCodecProfile->TabIndex = 111;
            this->fcgLBCodecProfile->Text = L"プロファイル";
            // 
            // fcgLBEncMode
            // 
            this->fcgLBEncMode->AutoSize = true;
            this->fcgLBEncMode->Location = System::Drawing::Point(13, 145);
            this->fcgLBEncMode->Name = L"fcgLBEncMode";
            this->fcgLBEncMode->Size = System::Drawing::Size(32, 14);
            this->fcgLBEncMode->TabIndex = 5;
            this->fcgLBEncMode->Text = L"モード";
            // 
            // fcgCXEncMode
            // 
            this->fcgCXEncMode->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXEncMode->FormattingEnabled = true;
            this->fcgCXEncMode->Items->AddRange(gcnew cli::array< System::Object ^  >(3) { L"高品質", L"標準", L"高速" });
            this->fcgCXEncMode->Location = System::Drawing::Point(81, 142);
            this->fcgCXEncMode->Name = L"fcgCXEncMode";
            this->fcgCXEncMode->Size = System::Drawing::Size(160, 22);
            this->fcgCXEncMode->TabIndex = 6;
            this->fcgCXEncMode->Tag = L"reCmd";
            this->fcgCXEncMode->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgChangeEnabled);
            // 
            // fcgPNBitrate
            // 
            this->fcgPNBitrate->Controls->Add(this->fcgLBBitrate);
            this->fcgPNBitrate->Controls->Add(this->fcgNUBitrate);
            this->fcgPNBitrate->Controls->Add(this->fcgLBBitrate2);
            this->fcgPNBitrate->Controls->Add(this->fcgNUMaxkbps);
            this->fcgPNBitrate->Controls->Add(this->fcgLBMaxkbps);
            this->fcgPNBitrate->Controls->Add(this->fcgLBMaxBitrate2);
            this->fcgPNBitrate->Location = System::Drawing::Point(8, 199);
            this->fcgPNBitrate->Name = L"fcgPNBitrate";
            this->fcgPNBitrate->Size = System::Drawing::Size(289, 54);
            this->fcgPNBitrate->TabIndex = 10;
            // 
            // fcgLBBitrate
            // 
            this->fcgLBBitrate->AutoSize = true;
            this->fcgLBBitrate->Location = System::Drawing::Point(5, 4);
            this->fcgLBBitrate->Name = L"fcgLBBitrate";
            this->fcgLBBitrate->Size = System::Drawing::Size(54, 14);
            this->fcgLBBitrate->TabIndex = 11;
            this->fcgLBBitrate->Text = L"ビットレート";
            // 
            // fcgNUBitrate
            // 
            this->fcgNUBitrate->Location = System::Drawing::Point(124, 2);
            this->fcgNUBitrate->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65536000, 0, 0, 0 });
            this->fcgNUBitrate->Name = L"fcgNUBitrate";
            this->fcgNUBitrate->Size = System::Drawing::Size(77, 21);
            this->fcgNUBitrate->TabIndex = 12;
            this->fcgNUBitrate->Tag = L"reCmd";
            this->fcgNUBitrate->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBBitrate2
            // 
            this->fcgLBBitrate2->AutoSize = true;
            this->fcgLBBitrate2->Location = System::Drawing::Point(207, 4);
            this->fcgLBBitrate2->Name = L"fcgLBBitrate2";
            this->fcgLBBitrate2->Size = System::Drawing::Size(32, 14);
            this->fcgLBBitrate2->TabIndex = 13;
            this->fcgLBBitrate2->Text = L"kbps";
            // 
            // fcgNUMaxkbps
            // 
            this->fcgNUMaxkbps->Location = System::Drawing::Point(124, 29);
            this->fcgNUMaxkbps->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65536000, 0, 0, 0 });
            this->fcgNUMaxkbps->Name = L"fcgNUMaxkbps";
            this->fcgNUMaxkbps->Size = System::Drawing::Size(77, 21);
            this->fcgNUMaxkbps->TabIndex = 15;
            this->fcgNUMaxkbps->Tag = L"reCmd";
            this->fcgNUMaxkbps->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBMaxkbps
            // 
            this->fcgLBMaxkbps->AutoSize = true;
            this->fcgLBMaxkbps->Location = System::Drawing::Point(5, 33);
            this->fcgLBMaxkbps->Name = L"fcgLBMaxkbps";
            this->fcgLBMaxkbps->Size = System::Drawing::Size(76, 14);
            this->fcgLBMaxkbps->TabIndex = 14;
            this->fcgLBMaxkbps->Text = L"最大ビットレート";
            // 
            // fcgLBMaxBitrate2
            // 
            this->fcgLBMaxBitrate2->AutoSize = true;
            this->fcgLBMaxBitrate2->Location = System::Drawing::Point(207, 31);
            this->fcgLBMaxBitrate2->Name = L"fcgLBMaxBitrate2";
            this->fcgLBMaxBitrate2->Size = System::Drawing::Size(32, 14);
            this->fcgLBMaxBitrate2->TabIndex = 16;
            this->fcgLBMaxBitrate2->Text = L"kbps";
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
            this->fcgPNBframes->Location = System::Drawing::Point(7, 390);
            this->fcgPNBframes->Name = L"fcgPNBframes";
            this->fcgPNBframes->Size = System::Drawing::Size(330, 57);
            this->fcgPNBframes->TabIndex = 50;
            // 
            // fcgLBBframes
            // 
            this->fcgLBBframes->AutoSize = true;
            this->fcgLBBframes->Location = System::Drawing::Point(7, 6);
            this->fcgLBBframes->Name = L"fcgLBBframes";
            this->fcgLBBframes->Size = System::Drawing::Size(58, 14);
            this->fcgLBBframes->TabIndex = 51;
            this->fcgLBBframes->Text = L"Bフレーム数";
            // 
            // fcgNUBframes
            // 
            this->fcgNUBframes->Location = System::Drawing::Point(125, 3);
            this->fcgNUBframes->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 3, 0, 0, 0 });
            this->fcgNUBframes->Name = L"fcgNUBframes";
            this->fcgNUBframes->Size = System::Drawing::Size(53, 21);
            this->fcgNUBframes->TabIndex = 52;
            this->fcgNUBframes->Tag = L"reCmd";
            this->fcgNUBframes->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCBBPyramid
            // 
            this->fcgCBBPyramid->AutoSize = true;
            this->fcgCBBPyramid->Location = System::Drawing::Point(303, 8);
            this->fcgCBBPyramid->Name = L"fcgCBBPyramid";
            this->fcgCBBPyramid->Size = System::Drawing::Size(15, 14);
            this->fcgCBBPyramid->TabIndex = 54;
            this->fcgCBBPyramid->Tag = L"reCmd";
            this->fcgCBBPyramid->UseVisualStyleBackColor = true;
            // 
            // fcgLBBPyramid
            // 
            this->fcgLBBPyramid->AutoSize = true;
            this->fcgLBBPyramid->Location = System::Drawing::Point(224, 6);
            this->fcgLBBPyramid->Name = L"fcgLBBPyramid";
            this->fcgLBBPyramid->Size = System::Drawing::Size(66, 14);
            this->fcgLBBPyramid->TabIndex = 53;
            this->fcgLBBPyramid->Text = L"ピラミッド参照";
            // 
            // fcgLBBDeltaQP
            // 
            this->fcgLBBDeltaQP->AutoSize = true;
            this->fcgLBBDeltaQP->Location = System::Drawing::Point(7, 33);
            this->fcgLBBDeltaQP->Name = L"fcgLBBDeltaQP";
            this->fcgLBBDeltaQP->Size = System::Drawing::Size(116, 14);
            this->fcgLBBDeltaQP->TabIndex = 55;
            this->fcgLBBDeltaQP->Text = L"QPオフセット    Bフレーム";
            // 
            // fcgNUBDeltaQP
            // 
            this->fcgNUBDeltaQP->Location = System::Drawing::Point(126, 30);
            this->fcgNUBDeltaQP->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
            this->fcgNUBDeltaQP->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, System::Int32::MinValue });
            this->fcgNUBDeltaQP->Name = L"fcgNUBDeltaQP";
            this->fcgNUBDeltaQP->Size = System::Drawing::Size(52, 21);
            this->fcgNUBDeltaQP->TabIndex = 56;
            this->fcgNUBDeltaQP->Tag = L"reCmd";
            this->fcgNUBDeltaQP->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgLBBRefDeltaQP
            // 
            this->fcgLBBRefDeltaQP->AutoSize = true;
            this->fcgLBBRefDeltaQP->Location = System::Drawing::Point(193, 33);
            this->fcgLBBRefDeltaQP->Name = L"fcgLBBRefDeltaQP";
            this->fcgLBBRefDeltaQP->Size = System::Drawing::Size(69, 14);
            this->fcgLBBRefDeltaQP->TabIndex = 57;
            this->fcgLBBRefDeltaQP->Text = L"参照Bフレーム";
            // 
            // fcgNUBRefDeltaQP
            // 
            this->fcgNUBRefDeltaQP->Location = System::Drawing::Point(267, 31);
            this->fcgNUBRefDeltaQP->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, 0 });
            this->fcgNUBRefDeltaQP->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 10, 0, 0, System::Int32::MinValue });
            this->fcgNUBRefDeltaQP->Name = L"fcgNUBRefDeltaQP";
            this->fcgNUBRefDeltaQP->Size = System::Drawing::Size(52, 21);
            this->fcgNUBRefDeltaQP->TabIndex = 58;
            this->fcgNUBRefDeltaQP->Tag = L"reCmd";
            this->fcgNUBRefDeltaQP->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgPNHEVCLevelProfile
            // 
            this->fcgPNHEVCLevelProfile->Controls->Add(this->fcgCXHEVCLevel);
            this->fcgPNHEVCLevelProfile->Controls->Add(this->fcgCXHEVCProfile);
            this->fcgPNHEVCLevelProfile->Controls->Add(this->fcgLBHEVCLevel);
            this->fcgPNHEVCLevelProfile->Controls->Add(this->fcgLBHEVCProfile);
            this->fcgPNHEVCLevelProfile->Location = System::Drawing::Point(346, 135);
            this->fcgPNHEVCLevelProfile->Name = L"fcgPNHEVCLevelProfile";
            this->fcgPNHEVCLevelProfile->Size = System::Drawing::Size(255, 63);
            this->fcgPNHEVCLevelProfile->TabIndex = 110;
            // 
            // fcgCXHEVCLevel
            // 
            this->fcgCXHEVCLevel->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXHEVCLevel->FormattingEnabled = true;
            this->fcgCXHEVCLevel->Location = System::Drawing::Point(112, 36);
            this->fcgCXHEVCLevel->Name = L"fcgCXHEVCLevel";
            this->fcgCXHEVCLevel->Size = System::Drawing::Size(121, 22);
            this->fcgCXHEVCLevel->TabIndex = 104;
            this->fcgCXHEVCLevel->Tag = L"reCmd";
            // 
            // fcgCXHEVCProfile
            // 
            this->fcgCXHEVCProfile->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXHEVCProfile->FormattingEnabled = true;
            this->fcgCXHEVCProfile->Location = System::Drawing::Point(112, 4);
            this->fcgCXHEVCProfile->Name = L"fcgCXHEVCProfile";
            this->fcgCXHEVCProfile->Size = System::Drawing::Size(121, 22);
            this->fcgCXHEVCProfile->TabIndex = 102;
            this->fcgCXHEVCProfile->Tag = L"reCmd";
            // 
            // fcgLBHEVCLevel
            // 
            this->fcgLBHEVCLevel->AutoSize = true;
            this->fcgLBHEVCLevel->Location = System::Drawing::Point(13, 39);
            this->fcgLBHEVCLevel->Name = L"fcgLBHEVCLevel";
            this->fcgLBHEVCLevel->Size = System::Drawing::Size(33, 14);
            this->fcgLBHEVCLevel->TabIndex = 103;
            this->fcgLBHEVCLevel->Text = L"レベル";
            // 
            // fcgLBHEVCProfile
            // 
            this->fcgLBHEVCProfile->AutoSize = true;
            this->fcgLBHEVCProfile->Location = System::Drawing::Point(13, 7);
            this->fcgLBHEVCProfile->Name = L"fcgLBHEVCProfile";
            this->fcgLBHEVCProfile->Size = System::Drawing::Size(53, 14);
            this->fcgLBHEVCProfile->TabIndex = 101;
            this->fcgLBHEVCProfile->Text = L"プロファイル";
            // 
            // fcgPNH264Features
            // 
            this->fcgPNH264Features->Controls->Add(this->fcgLBVBAQ);
            this->fcgPNH264Features->Controls->Add(this->fcgCBVBAQ);
            this->fcgPNH264Features->Location = System::Drawing::Point(344, 297);
            this->fcgPNH264Features->Name = L"fcgPNH264Features";
            this->fcgPNH264Features->Size = System::Drawing::Size(258, 26);
            this->fcgPNH264Features->TabIndex = 130;
            // 
            // fcgLBVBAQ
            // 
            this->fcgLBVBAQ->AutoSize = true;
            this->fcgLBVBAQ->Location = System::Drawing::Point(16, 6);
            this->fcgLBVBAQ->Name = L"fcgLBVBAQ";
            this->fcgLBVBAQ->Size = System::Drawing::Size(37, 14);
            this->fcgLBVBAQ->TabIndex = 131;
            this->fcgLBVBAQ->Text = L"VBAQ";
            // 
            // fcgCBVBAQ
            // 
            this->fcgCBVBAQ->AutoSize = true;
            this->fcgCBVBAQ->Location = System::Drawing::Point(115, 7);
            this->fcgCBVBAQ->Name = L"fcgCBVBAQ";
            this->fcgCBVBAQ->Size = System::Drawing::Size(15, 14);
            this->fcgCBVBAQ->TabIndex = 132;
            this->fcgCBVBAQ->Tag = L"reCmd";
            this->fcgCBVBAQ->UseVisualStyleBackColor = true;
            // 
            // tabPageExOpt
            // 
            this->tabPageExOpt->Controls->Add(this->fcgCBPSNR);
            this->tabPageExOpt->Controls->Add(this->fcgCBSSIM);
            this->tabPageExOpt->Controls->Add(this->fcggroupBoxVppDeinterlace);
            this->tabPageExOpt->Controls->Add(this->fcgCBTimerPeriodTuning);
            this->tabPageExOpt->Controls->Add(this->fcgCBResize);
            this->tabPageExOpt->Controls->Add(this->fcggroupboxResize);
            this->tabPageExOpt->Controls->Add(this->fcgCBAuoTcfileout);
            this->tabPageExOpt->Controls->Add(this->fcgLBTempDir);
            this->tabPageExOpt->Controls->Add(this->fcgBTCustomTempDir);
            this->tabPageExOpt->Controls->Add(this->fcgTXCustomTempDir);
            this->tabPageExOpt->Controls->Add(this->fcgCXTempDir);
            this->tabPageExOpt->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
                static_cast<System::Byte>(128)));
            this->tabPageExOpt->Location = System::Drawing::Point(4, 24);
            this->tabPageExOpt->Name = L"tabPageExOpt";
            this->tabPageExOpt->Size = System::Drawing::Size(608, 481);
            this->tabPageExOpt->TabIndex = 1;
            this->tabPageExOpt->Text = L"その他";
            this->tabPageExOpt->UseVisualStyleBackColor = true;
            // 
            // fcgCBPSNR
            // 
            this->fcgCBPSNR->AutoSize = true;
            this->fcgCBPSNR->Location = System::Drawing::Point(49, 278);
            this->fcgCBPSNR->Name = L"fcgCBPSNR";
            this->fcgCBPSNR->Size = System::Drawing::Size(50, 18);
            this->fcgCBPSNR->TabIndex = 17;
            this->fcgCBPSNR->Tag = L"reCmd";
            this->fcgCBPSNR->Text = L"psnr";
            this->fcgCBPSNR->UseVisualStyleBackColor = true;
            // 
            // fcgCBSSIM
            // 
            this->fcgCBSSIM->AutoSize = true;
            this->fcgCBSSIM->Location = System::Drawing::Point(49, 252);
            this->fcgCBSSIM->Name = L"fcgCBSSIM";
            this->fcgCBSSIM->Size = System::Drawing::Size(50, 18);
            this->fcgCBSSIM->TabIndex = 16;
            this->fcgCBSSIM->Tag = L"reCmd";
            this->fcgCBSSIM->Text = L"ssim";
            this->fcgCBSSIM->UseVisualStyleBackColor = true;
            // 
            // fcggroupBoxVppDeinterlace
            // 
            this->fcggroupBoxVppDeinterlace->Controls->Add(this->fcgLBVppDeinterlace);
            this->fcggroupBoxVppDeinterlace->Controls->Add(this->fcgCXVppDeinterlace);
            this->fcggroupBoxVppDeinterlace->Controls->Add(this->fcgPNVppAfs);
            this->fcggroupBoxVppDeinterlace->Location = System::Drawing::Point(343, 6);
            this->fcggroupBoxVppDeinterlace->Name = L"fcggroupBoxVppDeinterlace";
            this->fcggroupBoxVppDeinterlace->Size = System::Drawing::Size(262, 338);
            this->fcggroupBoxVppDeinterlace->TabIndex = 20;
            this->fcggroupBoxVppDeinterlace->TabStop = false;
            this->fcggroupBoxVppDeinterlace->Text = L"インタレ解除";
            // 
            // fcgLBVppDeinterlace
            // 
            this->fcgLBVppDeinterlace->AutoSize = true;
            this->fcgLBVppDeinterlace->Location = System::Drawing::Point(15, 19);
            this->fcgLBVppDeinterlace->Name = L"fcgLBVppDeinterlace";
            this->fcgLBVppDeinterlace->Size = System::Drawing::Size(54, 14);
            this->fcgLBVppDeinterlace->TabIndex = 0;
            this->fcgLBVppDeinterlace->Text = L"解除モード";
            // 
            // fcgCXVppDeinterlace
            // 
            this->fcgCXVppDeinterlace->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppDeinterlace->FormattingEnabled = true;
            this->fcgCXVppDeinterlace->Location = System::Drawing::Point(83, 15);
            this->fcgCXVppDeinterlace->Name = L"fcgCXVppDeinterlace";
            this->fcgCXVppDeinterlace->Size = System::Drawing::Size(164, 22);
            this->fcgCXVppDeinterlace->TabIndex = 1;
            this->fcgCXVppDeinterlace->Tag = L"reCmd";
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
            this->fcgPNVppAfs->Controls->Add(this->label20);
            this->fcgPNVppAfs->Controls->Add(this->label19);
            this->fcgPNVppAfs->Controls->Add(this->label4);
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
            this->fcgPNVppAfs->Location = System::Drawing::Point(6, 41);
            this->fcgPNVppAfs->Name = L"fcgPNVppAfs";
            this->fcgPNVppAfs->Size = System::Drawing::Size(251, 294);
            this->fcgPNVppAfs->TabIndex = 10;
            // 
            // fcgTBVppAfsThreCMotion
            // 
            this->fcgTBVppAfsThreCMotion->AutoSize = false;
            this->fcgTBVppAfsThreCMotion->Location = System::Drawing::Point(60, 179);
            this->fcgTBVppAfsThreCMotion->Maximum = 1024;
            this->fcgTBVppAfsThreCMotion->Name = L"fcgTBVppAfsThreCMotion";
            this->fcgTBVppAfsThreCMotion->Size = System::Drawing::Size(115, 18);
            this->fcgTBVppAfsThreCMotion->TabIndex = 24;
            this->fcgTBVppAfsThreCMotion->TickStyle = System::Windows::Forms::TickStyle::None;
            this->fcgTBVppAfsThreCMotion->Scroll += gcnew System::EventHandler(this, &frmConfig::fcgTBVppAfsScroll);
            // 
            // fcgLBVppAfsThreCMotion
            // 
            this->fcgLBVppAfsThreCMotion->AutoSize = true;
            this->fcgLBVppAfsThreCMotion->Location = System::Drawing::Point(9, 179);
            this->fcgLBVppAfsThreCMotion->Name = L"fcgLBVppAfsThreCMotion";
            this->fcgLBVppAfsThreCMotion->Size = System::Drawing::Size(33, 14);
            this->fcgLBVppAfsThreCMotion->TabIndex = 23;
            this->fcgLBVppAfsThreCMotion->Text = L"C動き";
            // 
            // fcgTBVppAfsThreYMotion
            // 
            this->fcgTBVppAfsThreYMotion->AutoSize = false;
            this->fcgTBVppAfsThreYMotion->Location = System::Drawing::Point(60, 154);
            this->fcgTBVppAfsThreYMotion->Maximum = 1024;
            this->fcgTBVppAfsThreYMotion->Name = L"fcgTBVppAfsThreYMotion";
            this->fcgTBVppAfsThreYMotion->Size = System::Drawing::Size(115, 18);
            this->fcgTBVppAfsThreYMotion->TabIndex = 21;
            this->fcgTBVppAfsThreYMotion->TickStyle = System::Windows::Forms::TickStyle::None;
            this->fcgTBVppAfsThreYMotion->Scroll += gcnew System::EventHandler(this, &frmConfig::fcgTBVppAfsScroll);
            // 
            // fcgLBVppAfsThreYmotion
            // 
            this->fcgLBVppAfsThreYmotion->AutoSize = true;
            this->fcgLBVppAfsThreYmotion->Location = System::Drawing::Point(9, 154);
            this->fcgLBVppAfsThreYmotion->Name = L"fcgLBVppAfsThreYmotion";
            this->fcgLBVppAfsThreYmotion->Size = System::Drawing::Size(33, 14);
            this->fcgLBVppAfsThreYmotion->TabIndex = 20;
            this->fcgLBVppAfsThreYmotion->Text = L"Y動き";
            // 
            // fcgTBVppAfsThreDeint
            // 
            this->fcgTBVppAfsThreDeint->AutoSize = false;
            this->fcgTBVppAfsThreDeint->Location = System::Drawing::Point(60, 129);
            this->fcgTBVppAfsThreDeint->Maximum = 1024;
            this->fcgTBVppAfsThreDeint->Name = L"fcgTBVppAfsThreDeint";
            this->fcgTBVppAfsThreDeint->Size = System::Drawing::Size(115, 18);
            this->fcgTBVppAfsThreDeint->TabIndex = 18;
            this->fcgTBVppAfsThreDeint->TickStyle = System::Windows::Forms::TickStyle::None;
            this->fcgTBVppAfsThreDeint->Scroll += gcnew System::EventHandler(this, &frmConfig::fcgTBVppAfsScroll);
            // 
            // fcgLBVppAfsThreDeint
            // 
            this->fcgLBVppAfsThreDeint->AutoSize = true;
            this->fcgLBVppAfsThreDeint->Location = System::Drawing::Point(9, 129);
            this->fcgLBVppAfsThreDeint->Name = L"fcgLBVppAfsThreDeint";
            this->fcgLBVppAfsThreDeint->Size = System::Drawing::Size(50, 14);
            this->fcgLBVppAfsThreDeint->TabIndex = 17;
            this->fcgLBVppAfsThreDeint->Text = L"縞(解除)";
            // 
            // fcgTBVppAfsThreShift
            // 
            this->fcgTBVppAfsThreShift->AutoSize = false;
            this->fcgTBVppAfsThreShift->Location = System::Drawing::Point(60, 104);
            this->fcgTBVppAfsThreShift->Maximum = 1024;
            this->fcgTBVppAfsThreShift->Name = L"fcgTBVppAfsThreShift";
            this->fcgTBVppAfsThreShift->Size = System::Drawing::Size(115, 18);
            this->fcgTBVppAfsThreShift->TabIndex = 15;
            this->fcgTBVppAfsThreShift->TickStyle = System::Windows::Forms::TickStyle::None;
            this->fcgTBVppAfsThreShift->Scroll += gcnew System::EventHandler(this, &frmConfig::fcgTBVppAfsScroll);
            // 
            // fcgLBVppAfsThreShift
            // 
            this->fcgLBVppAfsThreShift->AutoSize = true;
            this->fcgLBVppAfsThreShift->Location = System::Drawing::Point(9, 104);
            this->fcgLBVppAfsThreShift->Name = L"fcgLBVppAfsThreShift";
            this->fcgLBVppAfsThreShift->Size = System::Drawing::Size(46, 14);
            this->fcgLBVppAfsThreShift->TabIndex = 14;
            this->fcgLBVppAfsThreShift->Text = L"縞(ｼﾌﾄ)";
            // 
            // fcgTBVppAfsCoeffShift
            // 
            this->fcgTBVppAfsCoeffShift->AutoSize = false;
            this->fcgTBVppAfsCoeffShift->Location = System::Drawing::Point(60, 79);
            this->fcgTBVppAfsCoeffShift->Maximum = 256;
            this->fcgTBVppAfsCoeffShift->Name = L"fcgTBVppAfsCoeffShift";
            this->fcgTBVppAfsCoeffShift->Size = System::Drawing::Size(115, 18);
            this->fcgTBVppAfsCoeffShift->TabIndex = 12;
            this->fcgTBVppAfsCoeffShift->TickStyle = System::Windows::Forms::TickStyle::None;
            this->fcgTBVppAfsCoeffShift->Scroll += gcnew System::EventHandler(this, &frmConfig::fcgTBVppAfsScroll);
            // 
            // fcgLBVppAfsCoeffShift
            // 
            this->fcgLBVppAfsCoeffShift->AutoSize = true;
            this->fcgLBVppAfsCoeffShift->Location = System::Drawing::Point(9, 79);
            this->fcgLBVppAfsCoeffShift->Name = L"fcgLBVppAfsCoeffShift";
            this->fcgLBVppAfsCoeffShift->Size = System::Drawing::Size(40, 14);
            this->fcgLBVppAfsCoeffShift->TabIndex = 11;
            this->fcgLBVppAfsCoeffShift->Text = L"判定比";
            // 
            // label20
            // 
            this->label20->AutoSize = true;
            this->label20->Location = System::Drawing::Point(163, 29);
            this->label20->Name = L"label20";
            this->label20->Size = System::Drawing::Size(18, 14);
            this->label20->TabIndex = 6;
            this->label20->Text = L"右";
            // 
            // label19
            // 
            this->label19->AutoSize = true;
            this->label19->Location = System::Drawing::Point(57, 29);
            this->label19->Name = L"label19";
            this->label19->Size = System::Drawing::Size(18, 14);
            this->label19->TabIndex = 4;
            this->label19->Text = L"左";
            // 
            // label4
            // 
            this->label4->AutoSize = true;
            this->label4->Location = System::Drawing::Point(163, 4);
            this->label4->Name = L"label4";
            this->label4->Size = System::Drawing::Size(18, 14);
            this->label4->TabIndex = 2;
            this->label4->Text = L"下";
            // 
            // fcgLBVppAfsUp
            // 
            this->fcgLBVppAfsUp->AutoSize = true;
            this->fcgLBVppAfsUp->Location = System::Drawing::Point(57, 4);
            this->fcgLBVppAfsUp->Name = L"fcgLBVppAfsUp";
            this->fcgLBVppAfsUp->Size = System::Drawing::Size(18, 14);
            this->fcgLBVppAfsUp->TabIndex = 0;
            this->fcgLBVppAfsUp->Text = L"上";
            // 
            // fcgNUVppAfsRight
            // 
            this->fcgNUVppAfsRight->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 8, 0, 0, 0 });
            this->fcgNUVppAfsRight->Location = System::Drawing::Point(181, 27);
            this->fcgNUVppAfsRight->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 8192, 0, 0, 0 });
            this->fcgNUVppAfsRight->Name = L"fcgNUVppAfsRight";
            this->fcgNUVppAfsRight->Size = System::Drawing::Size(60, 21);
            this->fcgNUVppAfsRight->TabIndex = 7;
            this->fcgNUVppAfsRight->Tag = L"reCmd";
            this->fcgNUVppAfsRight->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUVppAfsLeft
            // 
            this->fcgNUVppAfsLeft->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 8, 0, 0, 0 });
            this->fcgNUVppAfsLeft->Location = System::Drawing::Point(77, 27);
            this->fcgNUVppAfsLeft->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 8192, 0, 0, 0 });
            this->fcgNUVppAfsLeft->Name = L"fcgNUVppAfsLeft";
            this->fcgNUVppAfsLeft->Size = System::Drawing::Size(60, 21);
            this->fcgNUVppAfsLeft->TabIndex = 5;
            this->fcgNUVppAfsLeft->Tag = L"reCmd";
            this->fcgNUVppAfsLeft->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUVppAfsBottom
            // 
            this->fcgNUVppAfsBottom->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 8, 0, 0, 0 });
            this->fcgNUVppAfsBottom->Location = System::Drawing::Point(181, 2);
            this->fcgNUVppAfsBottom->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4096, 0, 0, 0 });
            this->fcgNUVppAfsBottom->Name = L"fcgNUVppAfsBottom";
            this->fcgNUVppAfsBottom->Size = System::Drawing::Size(60, 21);
            this->fcgNUVppAfsBottom->TabIndex = 3;
            this->fcgNUVppAfsBottom->Tag = L"reCmd";
            this->fcgNUVppAfsBottom->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUVppAfsUp
            // 
            this->fcgNUVppAfsUp->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 8, 0, 0, 0 });
            this->fcgNUVppAfsUp->Location = System::Drawing::Point(77, 2);
            this->fcgNUVppAfsUp->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 4096, 0, 0, 0 });
            this->fcgNUVppAfsUp->Name = L"fcgNUVppAfsUp";
            this->fcgNUVppAfsUp->Size = System::Drawing::Size(60, 21);
            this->fcgNUVppAfsUp->TabIndex = 1;
            this->fcgNUVppAfsUp->Tag = L"reCmd";
            this->fcgNUVppAfsUp->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgTBVppAfsMethodSwitch
            // 
            this->fcgTBVppAfsMethodSwitch->AutoSize = false;
            this->fcgTBVppAfsMethodSwitch->Location = System::Drawing::Point(60, 54);
            this->fcgTBVppAfsMethodSwitch->Maximum = 256;
            this->fcgTBVppAfsMethodSwitch->Name = L"fcgTBVppAfsMethodSwitch";
            this->fcgTBVppAfsMethodSwitch->Size = System::Drawing::Size(115, 18);
            this->fcgTBVppAfsMethodSwitch->TabIndex = 9;
            this->fcgTBVppAfsMethodSwitch->TickStyle = System::Windows::Forms::TickStyle::None;
            this->fcgTBVppAfsMethodSwitch->Scroll += gcnew System::EventHandler(this, &frmConfig::fcgTBVppAfsScroll);
            // 
            // fcgCBVppAfs24fps
            // 
            this->fcgCBVppAfs24fps->AutoSize = true;
            this->fcgCBVppAfs24fps->Location = System::Drawing::Point(149, 231);
            this->fcgCBVppAfs24fps->Name = L"fcgCBVppAfs24fps";
            this->fcgCBVppAfs24fps->Size = System::Drawing::Size(67, 18);
            this->fcgCBVppAfs24fps->TabIndex = 29;
            this->fcgCBVppAfs24fps->Tag = L"reCmd";
            this->fcgCBVppAfs24fps->Text = L"24fps化";
            this->fcgCBVppAfs24fps->UseVisualStyleBackColor = true;
            // 
            // fcgCBVppAfsTune
            // 
            this->fcgCBVppAfsTune->AutoSize = true;
            this->fcgCBVppAfsTune->Location = System::Drawing::Point(149, 273);
            this->fcgCBVppAfsTune->Name = L"fcgCBVppAfsTune";
            this->fcgCBVppAfsTune->Size = System::Drawing::Size(73, 18);
            this->fcgCBVppAfsTune->TabIndex = 32;
            this->fcgCBVppAfsTune->Tag = L"reCmd";
            this->fcgCBVppAfsTune->Text = L"調整モード";
            this->fcgCBVppAfsTune->UseVisualStyleBackColor = true;
            // 
            // fcgCBVppAfsSmooth
            // 
            this->fcgCBVppAfsSmooth->AutoSize = true;
            this->fcgCBVppAfsSmooth->Location = System::Drawing::Point(11, 273);
            this->fcgCBVppAfsSmooth->Name = L"fcgCBVppAfsSmooth";
            this->fcgCBVppAfsSmooth->Size = System::Drawing::Size(77, 18);
            this->fcgCBVppAfsSmooth->TabIndex = 31;
            this->fcgCBVppAfsSmooth->Tag = L"reCmd";
            this->fcgCBVppAfsSmooth->Text = L"スムージング";
            this->fcgCBVppAfsSmooth->UseVisualStyleBackColor = true;
            // 
            // fcgCBVppAfsDrop
            // 
            this->fcgCBVppAfsDrop->AutoSize = true;
            this->fcgCBVppAfsDrop->Location = System::Drawing::Point(11, 252);
            this->fcgCBVppAfsDrop->Name = L"fcgCBVppAfsDrop";
            this->fcgCBVppAfsDrop->Size = System::Drawing::Size(56, 18);
            this->fcgCBVppAfsDrop->TabIndex = 30;
            this->fcgCBVppAfsDrop->Tag = L"reCmd";
            this->fcgCBVppAfsDrop->Text = L"間引き";
            this->fcgCBVppAfsDrop->UseVisualStyleBackColor = true;
            // 
            // fcgCBVppAfsShift
            // 
            this->fcgCBVppAfsShift->AutoSize = true;
            this->fcgCBVppAfsShift->Location = System::Drawing::Point(11, 231);
            this->fcgCBVppAfsShift->Name = L"fcgCBVppAfsShift";
            this->fcgCBVppAfsShift->Size = System::Drawing::Size(89, 18);
            this->fcgCBVppAfsShift->TabIndex = 28;
            this->fcgCBVppAfsShift->Tag = L"reCmd";
            this->fcgCBVppAfsShift->Text = L"フィールドシフト";
            this->fcgCBVppAfsShift->UseVisualStyleBackColor = true;
            // 
            // fcgLBVppAfsAnalyze
            // 
            this->fcgLBVppAfsAnalyze->AutoSize = true;
            this->fcgLBVppAfsAnalyze->Location = System::Drawing::Point(9, 207);
            this->fcgLBVppAfsAnalyze->Name = L"fcgLBVppAfsAnalyze";
            this->fcgLBVppAfsAnalyze->Size = System::Drawing::Size(41, 14);
            this->fcgLBVppAfsAnalyze->TabIndex = 26;
            this->fcgLBVppAfsAnalyze->Text = L"解除Lv";
            // 
            // fcgNUVppAfsThreCMotion
            // 
            this->fcgNUVppAfsThreCMotion->Location = System::Drawing::Point(181, 179);
            this->fcgNUVppAfsThreCMotion->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1024, 0, 0, 0 });
            this->fcgNUVppAfsThreCMotion->Name = L"fcgNUVppAfsThreCMotion";
            this->fcgNUVppAfsThreCMotion->Size = System::Drawing::Size(60, 21);
            this->fcgNUVppAfsThreCMotion->TabIndex = 25;
            this->fcgNUVppAfsThreCMotion->Tag = L"reCmd";
            this->fcgNUVppAfsThreCMotion->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppAfsThreCMotion->ValueChanged += gcnew System::EventHandler(this, &frmConfig::fcgNUVppAfsValueChanged);
            // 
            // fcgNUVppAfsThreShift
            // 
            this->fcgNUVppAfsThreShift->Location = System::Drawing::Point(181, 104);
            this->fcgNUVppAfsThreShift->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1024, 0, 0, 0 });
            this->fcgNUVppAfsThreShift->Name = L"fcgNUVppAfsThreShift";
            this->fcgNUVppAfsThreShift->Size = System::Drawing::Size(60, 21);
            this->fcgNUVppAfsThreShift->TabIndex = 16;
            this->fcgNUVppAfsThreShift->Tag = L"reCmd";
            this->fcgNUVppAfsThreShift->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppAfsThreShift->ValueChanged += gcnew System::EventHandler(this, &frmConfig::fcgNUVppAfsValueChanged);
            // 
            // fcgNUVppAfsThreDeint
            // 
            this->fcgNUVppAfsThreDeint->Location = System::Drawing::Point(181, 129);
            this->fcgNUVppAfsThreDeint->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1024, 0, 0, 0 });
            this->fcgNUVppAfsThreDeint->Name = L"fcgNUVppAfsThreDeint";
            this->fcgNUVppAfsThreDeint->Size = System::Drawing::Size(60, 21);
            this->fcgNUVppAfsThreDeint->TabIndex = 19;
            this->fcgNUVppAfsThreDeint->Tag = L"reCmd";
            this->fcgNUVppAfsThreDeint->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppAfsThreDeint->ValueChanged += gcnew System::EventHandler(this, &frmConfig::fcgNUVppAfsValueChanged);
            // 
            // fcgNUVppAfsThreYMotion
            // 
            this->fcgNUVppAfsThreYMotion->Location = System::Drawing::Point(181, 154);
            this->fcgNUVppAfsThreYMotion->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1024, 0, 0, 0 });
            this->fcgNUVppAfsThreYMotion->Name = L"fcgNUVppAfsThreYMotion";
            this->fcgNUVppAfsThreYMotion->Size = System::Drawing::Size(60, 21);
            this->fcgNUVppAfsThreYMotion->TabIndex = 22;
            this->fcgNUVppAfsThreYMotion->Tag = L"reCmd";
            this->fcgNUVppAfsThreYMotion->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppAfsThreYMotion->ValueChanged += gcnew System::EventHandler(this, &frmConfig::fcgNUVppAfsValueChanged);
            // 
            // fcgLBVppAfsMethodSwitch
            // 
            this->fcgLBVppAfsMethodSwitch->AutoSize = true;
            this->fcgLBVppAfsMethodSwitch->Location = System::Drawing::Point(9, 54);
            this->fcgLBVppAfsMethodSwitch->Name = L"fcgLBVppAfsMethodSwitch";
            this->fcgLBVppAfsMethodSwitch->Size = System::Drawing::Size(40, 14);
            this->fcgLBVppAfsMethodSwitch->TabIndex = 8;
            this->fcgLBVppAfsMethodSwitch->Text = L"切替点";
            // 
            // fcgCXVppAfsAnalyze
            // 
            this->fcgCXVppAfsAnalyze->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppAfsAnalyze->FormattingEnabled = true;
            this->fcgCXVppAfsAnalyze->Location = System::Drawing::Point(60, 204);
            this->fcgCXVppAfsAnalyze->Name = L"fcgCXVppAfsAnalyze";
            this->fcgCXVppAfsAnalyze->Size = System::Drawing::Size(181, 22);
            this->fcgCXVppAfsAnalyze->TabIndex = 27;
            this->fcgCXVppAfsAnalyze->Tag = L"reCmd";
            // 
            // fcgNUVppAfsCoeffShift
            // 
            this->fcgNUVppAfsCoeffShift->Location = System::Drawing::Point(181, 79);
            this->fcgNUVppAfsCoeffShift->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 256, 0, 0, 0 });
            this->fcgNUVppAfsCoeffShift->Name = L"fcgNUVppAfsCoeffShift";
            this->fcgNUVppAfsCoeffShift->Size = System::Drawing::Size(60, 21);
            this->fcgNUVppAfsCoeffShift->TabIndex = 13;
            this->fcgNUVppAfsCoeffShift->Tag = L"reCmd";
            this->fcgNUVppAfsCoeffShift->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppAfsCoeffShift->ValueChanged += gcnew System::EventHandler(this, &frmConfig::fcgNUVppAfsValueChanged);
            // 
            // fcgNUVppAfsMethodSwitch
            // 
            this->fcgNUVppAfsMethodSwitch->Location = System::Drawing::Point(181, 54);
            this->fcgNUVppAfsMethodSwitch->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 256, 0, 0, 0 });
            this->fcgNUVppAfsMethodSwitch->Name = L"fcgNUVppAfsMethodSwitch";
            this->fcgNUVppAfsMethodSwitch->Size = System::Drawing::Size(60, 21);
            this->fcgNUVppAfsMethodSwitch->TabIndex = 10;
            this->fcgNUVppAfsMethodSwitch->Tag = L"reCmd";
            this->fcgNUVppAfsMethodSwitch->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            this->fcgNUVppAfsMethodSwitch->ValueChanged += gcnew System::EventHandler(this, &frmConfig::fcgNUVppAfsValueChanged);
            // 
            // fcgCBTimerPeriodTuning
            // 
            this->fcgCBTimerPeriodTuning->AutoSize = true;
            this->fcgCBTimerPeriodTuning->Location = System::Drawing::Point(49, 197);
            this->fcgCBTimerPeriodTuning->Name = L"fcgCBTimerPeriodTuning";
            this->fcgCBTimerPeriodTuning->Size = System::Drawing::Size(92, 18);
            this->fcgCBTimerPeriodTuning->TabIndex = 14;
            this->fcgCBTimerPeriodTuning->Tag = L"chValue";
            this->fcgCBTimerPeriodTuning->Text = L"高精度タイマー";
            this->fcgCBTimerPeriodTuning->UseVisualStyleBackColor = true;
            // 
            // fcgCBResize
            // 
            this->fcgCBResize->AutoSize = true;
            this->fcgCBResize->Location = System::Drawing::Point(16, 5);
            this->fcgCBResize->Name = L"fcgCBResize";
            this->fcgCBResize->Size = System::Drawing::Size(58, 18);
            this->fcgCBResize->TabIndex = 0;
            this->fcgCBResize->Tag = L"reCmd";
            this->fcgCBResize->Text = L"リサイズ";
            this->fcgCBResize->UseVisualStyleBackColor = true;
            // 
            // fcggroupboxResize
            // 
            this->fcggroupboxResize->Controls->Add(this->fcgCXVppResizeAlg);
            this->fcggroupboxResize->Controls->Add(this->fcgLBResize);
            this->fcggroupboxResize->Controls->Add(this->fcgNUResizeH);
            this->fcggroupboxResize->Controls->Add(this->fcgNUResizeW);
            this->fcggroupboxResize->Location = System::Drawing::Point(4, 6);
            this->fcggroupboxResize->Name = L"fcggroupboxResize";
            this->fcggroupboxResize->Size = System::Drawing::Size(333, 54);
            this->fcggroupboxResize->TabIndex = 1;
            this->fcggroupboxResize->TabStop = false;
            // 
            // fcgCXVppResizeAlg
            // 
            this->fcgCXVppResizeAlg->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXVppResizeAlg->FormattingEnabled = true;
            this->fcgCXVppResizeAlg->Location = System::Drawing::Point(183, 22);
            this->fcgCXVppResizeAlg->Name = L"fcgCXVppResizeAlg";
            this->fcgCXVppResizeAlg->Size = System::Drawing::Size(144, 22);
            this->fcgCXVppResizeAlg->TabIndex = 3;
            this->fcgCXVppResizeAlg->Tag = L"reCmd";
            // 
            // fcgLBResize
            // 
            this->fcgLBResize->AutoSize = true;
            this->fcgLBResize->Location = System::Drawing::Point(85, 25);
            this->fcgLBResize->Name = L"fcgLBResize";
            this->fcgLBResize->Size = System::Drawing::Size(13, 14);
            this->fcgLBResize->TabIndex = 1;
            this->fcgLBResize->Text = L"x";
            // 
            // fcgNUResizeH
            // 
            this->fcgNUResizeH->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16, 0, 0, 0 });
            this->fcgNUResizeH->Location = System::Drawing::Point(104, 23);
            this->fcgNUResizeH->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65535, 0, 0, 0 });
            this->fcgNUResizeH->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65536, 0, 0, System::Int32::MinValue });
            this->fcgNUResizeH->Name = L"fcgNUResizeH";
            this->fcgNUResizeH->Size = System::Drawing::Size(70, 21);
            this->fcgNUResizeH->TabIndex = 2;
            this->fcgNUResizeH->Tag = L"reCmd";
            this->fcgNUResizeH->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgNUResizeW
            // 
            this->fcgNUResizeW->Increment = System::Decimal(gcnew cli::array< System::Int32 >(4) { 16, 0, 0, 0 });
            this->fcgNUResizeW->Location = System::Drawing::Point(10, 23);
            this->fcgNUResizeW->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65535, 0, 0, 0 });
            this->fcgNUResizeW->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 65536, 0, 0, System::Int32::MinValue });
            this->fcgNUResizeW->Name = L"fcgNUResizeW";
            this->fcgNUResizeW->Size = System::Drawing::Size(70, 21);
            this->fcgNUResizeW->TabIndex = 0;
            this->fcgNUResizeW->Tag = L"reCmd";
            this->fcgNUResizeW->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCBAuoTcfileout
            // 
            this->fcgCBAuoTcfileout->AutoSize = true;
            this->fcgCBAuoTcfileout->Location = System::Drawing::Point(49, 225);
            this->fcgCBAuoTcfileout->Name = L"fcgCBAuoTcfileout";
            this->fcgCBAuoTcfileout->Size = System::Drawing::Size(98, 18);
            this->fcgCBAuoTcfileout->TabIndex = 15;
            this->fcgCBAuoTcfileout->Tag = L"chValue";
            this->fcgCBAuoTcfileout->Text = L"タイムコード出力";
            this->fcgCBAuoTcfileout->UseVisualStyleBackColor = true;
            // 
            // fcgLBTempDir
            // 
            this->fcgLBTempDir->AutoSize = true;
            this->fcgLBTempDir->Location = System::Drawing::Point(13, 95);
            this->fcgLBTempDir->Name = L"fcgLBTempDir";
            this->fcgLBTempDir->Size = System::Drawing::Size(60, 14);
            this->fcgLBTempDir->TabIndex = 10;
            this->fcgLBTempDir->Text = L"一時フォルダ";
            // 
            // fcgBTCustomTempDir
            // 
            this->fcgBTCustomTempDir->Location = System::Drawing::Point(213, 148);
            this->fcgBTCustomTempDir->Name = L"fcgBTCustomTempDir";
            this->fcgBTCustomTempDir->Size = System::Drawing::Size(29, 23);
            this->fcgBTCustomTempDir->TabIndex = 13;
            this->fcgBTCustomTempDir->Text = L"...";
            this->fcgBTCustomTempDir->UseVisualStyleBackColor = true;
            this->fcgBTCustomTempDir->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTCustomTempDir_Click);
            // 
            // fcgTXCustomTempDir
            // 
            this->fcgTXCustomTempDir->Location = System::Drawing::Point(28, 149);
            this->fcgTXCustomTempDir->Name = L"fcgTXCustomTempDir";
            this->fcgTXCustomTempDir->Size = System::Drawing::Size(182, 21);
            this->fcgTXCustomTempDir->TabIndex = 12;
            this->fcgTXCustomTempDir->Tag = L"";
            this->fcgTXCustomTempDir->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXCustomTempDir_TextChanged);
            // 
            // fcgCXTempDir
            // 
            this->fcgCXTempDir->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXTempDir->FormattingEnabled = true;
            this->fcgCXTempDir->Location = System::Drawing::Point(16, 121);
            this->fcgCXTempDir->Name = L"fcgCXTempDir";
            this->fcgCXTempDir->Size = System::Drawing::Size(209, 22);
            this->fcgCXTempDir->TabIndex = 11;
            this->fcgCXTempDir->Tag = L"chValue";
            // 
            // fcgCSExeFiles
            // 
            this->fcgCSExeFiles->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem ^  >(1) { this->fcgTSExeFileshelp });
            this->fcgCSExeFiles->Name = L"fcgCSx264";
            this->fcgCSExeFiles->Size = System::Drawing::Size(131, 26);
            // 
            // fcgTSExeFileshelp
            // 
            this->fcgTSExeFileshelp->Name = L"fcgTSExeFileshelp";
            this->fcgTSExeFileshelp->Size = System::Drawing::Size(130, 22);
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
            this->fcgLBguiExBlog->Location = System::Drawing::Point(639, 579);
            this->fcgLBguiExBlog->Name = L"fcgLBguiExBlog";
            this->fcgLBguiExBlog->Size = System::Drawing::Size(85, 14);
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
            this->fcgtabControlAudio->Location = System::Drawing::Point(622, 31);
            this->fcgtabControlAudio->Name = L"fcgtabControlAudio";
            this->fcgtabControlAudio->SelectedIndex = 0;
            this->fcgtabControlAudio->Size = System::Drawing::Size(380, 302);
            this->fcgtabControlAudio->TabIndex = 51;
            // 
            // fcgtabPageAudioMain
            // 
            this->fcgtabPageAudioMain->Controls->Add(this->fcgPNAudioInternal);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgCBAudioUseExt);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgCBFAWCheck);
            this->fcgtabPageAudioMain->Controls->Add(this->fcgPNAudioExt);
            this->fcgtabPageAudioMain->Location = System::Drawing::Point(4, 24);
            this->fcgtabPageAudioMain->Name = L"fcgtabPageAudioMain";
            this->fcgtabPageAudioMain->Padding = System::Windows::Forms::Padding(3);
            this->fcgtabPageAudioMain->Size = System::Drawing::Size(372, 274);
            this->fcgtabPageAudioMain->TabIndex = 0;
            this->fcgtabPageAudioMain->Text = L"音声";
            this->fcgtabPageAudioMain->UseVisualStyleBackColor = true;
            // 
            // fcgPNAudioInternal
            // 
            this->fcgPNAudioInternal->Controls->Add(this->fcgLBAudioBitrateInternal);
            this->fcgPNAudioInternal->Controls->Add(this->fcgNUAudioBitrateInternal);
            this->fcgPNAudioInternal->Controls->Add(this->fcgCXAudioEncModeInternal);
            this->fcgPNAudioInternal->Controls->Add(this->label1);
            this->fcgPNAudioInternal->Controls->Add(this->fcgCXAudioEncoderInternal);
            this->fcgPNAudioInternal->Location = System::Drawing::Point(0, 28);
            this->fcgPNAudioInternal->Name = L"fcgPNAudioInternal";
            this->fcgPNAudioInternal->Size = System::Drawing::Size(374, 246);
            this->fcgPNAudioInternal->TabIndex = 81;
            // 
            // fcgLBAudioBitrateInternal
            // 
            this->fcgLBAudioBitrateInternal->AutoSize = true;
            this->fcgLBAudioBitrateInternal->Location = System::Drawing::Point(285, 69);
            this->fcgLBAudioBitrateInternal->Name = L"fcgLBAudioBitrateInternal";
            this->fcgLBAudioBitrateInternal->Size = System::Drawing::Size(34, 15);
            this->fcgLBAudioBitrateInternal->TabIndex = 76;
            this->fcgLBAudioBitrateInternal->Text = L"kbps";
            // 
            // fcgNUAudioBitrateInternal
            // 
            this->fcgNUAudioBitrateInternal->Location = System::Drawing::Point(214, 67);
            this->fcgNUAudioBitrateInternal->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1536, 0, 0, 0 });
            this->fcgNUAudioBitrateInternal->Name = L"fcgNUAudioBitrateInternal";
            this->fcgNUAudioBitrateInternal->Size = System::Drawing::Size(65, 23);
            this->fcgNUAudioBitrateInternal->TabIndex = 74;
            this->fcgNUAudioBitrateInternal->Tag = L"chValue";
            this->fcgNUAudioBitrateInternal->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCXAudioEncModeInternal
            // 
            this->fcgCXAudioEncModeInternal->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioEncModeInternal->FormattingEnabled = true;
            this->fcgCXAudioEncModeInternal->Location = System::Drawing::Point(18, 66);
            this->fcgCXAudioEncModeInternal->Name = L"fcgCXAudioEncModeInternal";
            this->fcgCXAudioEncModeInternal->Size = System::Drawing::Size(189, 23);
            this->fcgCXAudioEncModeInternal->TabIndex = 73;
            this->fcgCXAudioEncModeInternal->Tag = L"chValue";
            this->fcgCXAudioEncModeInternal->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXAudioEncModeInternal_SelectedIndexChanged);
            // 
            // label1
            // 
            this->label1->AutoSize = true;
            this->label1->Location = System::Drawing::Point(9, 41);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(34, 15);
            this->label1->TabIndex = 75;
            this->label1->Text = L"モード";
            // 
            // fcgCXAudioEncoderInternal
            // 
            this->fcgCXAudioEncoderInternal->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioEncoderInternal->FormattingEnabled = true;
            this->fcgCXAudioEncoderInternal->Location = System::Drawing::Point(18, 8);
            this->fcgCXAudioEncoderInternal->Name = L"fcgCXAudioEncoderInternal";
            this->fcgCXAudioEncoderInternal->Size = System::Drawing::Size(172, 23);
            this->fcgCXAudioEncoderInternal->TabIndex = 70;
            this->fcgCXAudioEncoderInternal->Tag = L"chValue";
            this->fcgCXAudioEncoderInternal->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXAudioEncoderInternal_SelectedIndexChanged);
            // 
            // fcgCBAudioUseExt
            // 
            this->fcgCBAudioUseExt->AutoSize = true;
            this->fcgCBAudioUseExt->Location = System::Drawing::Point(10, 5);
            this->fcgCBAudioUseExt->Name = L"fcgCBAudioUseExt";
            this->fcgCBAudioUseExt->Size = System::Drawing::Size(147, 19);
            this->fcgCBAudioUseExt->TabIndex = 79;
            this->fcgCBAudioUseExt->Tag = L"chValue";
            this->fcgCBAudioUseExt->Text = L"外部エンコーダを使用する";
            this->fcgCBAudioUseExt->UseVisualStyleBackColor = true;
            this->fcgCBAudioUseExt->CheckedChanged += gcnew System::EventHandler(this, &frmConfig::fcgCBAudioUseExt_CheckedChanged);
            // 
            // fcgCBFAWCheck
            // 
            this->fcgCBFAWCheck->AutoSize = true;
            this->fcgCBFAWCheck->Location = System::Drawing::Point(254, 5);
            this->fcgCBFAWCheck->Name = L"fcgCBFAWCheck";
            this->fcgCBFAWCheck->Size = System::Drawing::Size(87, 19);
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
            this->fcgPNAudioExt->Location = System::Drawing::Point(0, 28);
            this->fcgPNAudioExt->Name = L"fcgPNAudioExt";
            this->fcgPNAudioExt->Size = System::Drawing::Size(374, 246);
            this->fcgPNAudioExt->TabIndex = 82;
            // 
            // fcgCXAudioDelayCut
            // 
            this->fcgCXAudioDelayCut->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioDelayCut->FormattingEnabled = true;
            this->fcgCXAudioDelayCut->Location = System::Drawing::Point(292, 107);
            this->fcgCXAudioDelayCut->Name = L"fcgCXAudioDelayCut";
            this->fcgCXAudioDelayCut->Size = System::Drawing::Size(70, 23);
            this->fcgCXAudioDelayCut->TabIndex = 65;
            this->fcgCXAudioDelayCut->Tag = L"chValue";
            // 
            // fcgLBAudioDelayCut
            // 
            this->fcgLBAudioDelayCut->AutoSize = true;
            this->fcgLBAudioDelayCut->Location = System::Drawing::Point(225, 110);
            this->fcgLBAudioDelayCut->Name = L"fcgLBAudioDelayCut";
            this->fcgLBAudioDelayCut->Size = System::Drawing::Size(66, 15);
            this->fcgLBAudioDelayCut->TabIndex = 75;
            this->fcgLBAudioDelayCut->Text = L"ディレイカット";
            // 
            // fcgCBAudioEncTiming
            // 
            this->fcgCBAudioEncTiming->AutoSize = true;
            this->fcgCBAudioEncTiming->Location = System::Drawing::Point(243, 28);
            this->fcgCBAudioEncTiming->Name = L"fcgCBAudioEncTiming";
            this->fcgCBAudioEncTiming->Size = System::Drawing::Size(43, 15);
            this->fcgCBAudioEncTiming->TabIndex = 74;
            this->fcgCBAudioEncTiming->Text = L"処理順";
            // 
            // fcgCXAudioEncTiming
            // 
            this->fcgCXAudioEncTiming->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioEncTiming->FormattingEnabled = true;
            this->fcgCXAudioEncTiming->Location = System::Drawing::Point(287, 25);
            this->fcgCXAudioEncTiming->Name = L"fcgCXAudioEncTiming";
            this->fcgCXAudioEncTiming->Size = System::Drawing::Size(68, 23);
            this->fcgCXAudioEncTiming->TabIndex = 73;
            this->fcgCXAudioEncTiming->Tag = L"chValue";
            // 
            // fcgCXAudioTempDir
            // 
            this->fcgCXAudioTempDir->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioTempDir->FormattingEnabled = true;
            this->fcgCXAudioTempDir->Location = System::Drawing::Point(136, 182);
            this->fcgCXAudioTempDir->Name = L"fcgCXAudioTempDir";
            this->fcgCXAudioTempDir->Size = System::Drawing::Size(150, 23);
            this->fcgCXAudioTempDir->TabIndex = 67;
            this->fcgCXAudioTempDir->Tag = L"chValue";
            // 
            // fcgTXCustomAudioTempDir
            // 
            this->fcgTXCustomAudioTempDir->Location = System::Drawing::Point(65, 210);
            this->fcgTXCustomAudioTempDir->Name = L"fcgTXCustomAudioTempDir";
            this->fcgTXCustomAudioTempDir->Size = System::Drawing::Size(245, 23);
            this->fcgTXCustomAudioTempDir->TabIndex = 68;
            // 
            // fcgBTCustomAudioTempDir
            // 
            this->fcgBTCustomAudioTempDir->Location = System::Drawing::Point(317, 208);
            this->fcgBTCustomAudioTempDir->Name = L"fcgBTCustomAudioTempDir";
            this->fcgBTCustomAudioTempDir->Size = System::Drawing::Size(29, 23);
            this->fcgBTCustomAudioTempDir->TabIndex = 70;
            this->fcgBTCustomAudioTempDir->Text = L"...";
            this->fcgBTCustomAudioTempDir->UseVisualStyleBackColor = true;
            this->fcgBTCustomAudioTempDir->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTCustomAudioTempDir_Click);
            // 
            // fcgCBAudioUsePipe
            // 
            this->fcgCBAudioUsePipe->AutoSize = true;
            this->fcgCBAudioUsePipe->Location = System::Drawing::Point(131, 108);
            this->fcgCBAudioUsePipe->Name = L"fcgCBAudioUsePipe";
            this->fcgCBAudioUsePipe->Size = System::Drawing::Size(78, 19);
            this->fcgCBAudioUsePipe->TabIndex = 64;
            this->fcgCBAudioUsePipe->Tag = L"chValue";
            this->fcgCBAudioUsePipe->Text = L"パイプ処理";
            this->fcgCBAudioUsePipe->UseVisualStyleBackColor = true;
            // 
            // fcgNUAudioBitrate
            // 
            this->fcgNUAudioBitrate->Location = System::Drawing::Point(213, 131);
            this->fcgNUAudioBitrate->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) { 1536, 0, 0, 0 });
            this->fcgNUAudioBitrate->Name = L"fcgNUAudioBitrate";
            this->fcgNUAudioBitrate->Size = System::Drawing::Size(65, 23);
            this->fcgNUAudioBitrate->TabIndex = 62;
            this->fcgNUAudioBitrate->Tag = L"chValue";
            this->fcgNUAudioBitrate->TextAlign = System::Windows::Forms::HorizontalAlignment::Right;
            // 
            // fcgCBAudio2pass
            // 
            this->fcgCBAudio2pass->AutoSize = true;
            this->fcgCBAudio2pass->Location = System::Drawing::Point(60, 108);
            this->fcgCBAudio2pass->Name = L"fcgCBAudio2pass";
            this->fcgCBAudio2pass->Size = System::Drawing::Size(59, 19);
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
            this->fcgCXAudioEncMode->Location = System::Drawing::Point(17, 130);
            this->fcgCXAudioEncMode->Name = L"fcgCXAudioEncMode";
            this->fcgCXAudioEncMode->Size = System::Drawing::Size(189, 23);
            this->fcgCXAudioEncMode->TabIndex = 61;
            this->fcgCXAudioEncMode->Tag = L"chValue";
            this->fcgCXAudioEncMode->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXAudioEncMode_SelectedIndexChanged);
            // 
            // fcgLBAudioEncMode
            // 
            this->fcgLBAudioEncMode->AutoSize = true;
            this->fcgLBAudioEncMode->Location = System::Drawing::Point(5, 110);
            this->fcgLBAudioEncMode->Name = L"fcgLBAudioEncMode";
            this->fcgLBAudioEncMode->Size = System::Drawing::Size(34, 15);
            this->fcgLBAudioEncMode->TabIndex = 69;
            this->fcgLBAudioEncMode->Text = L"モード";
            // 
            // fcgBTAudioEncoderPath
            // 
            this->fcgBTAudioEncoderPath->Location = System::Drawing::Point(325, 64);
            this->fcgBTAudioEncoderPath->Name = L"fcgBTAudioEncoderPath";
            this->fcgBTAudioEncoderPath->Size = System::Drawing::Size(30, 23);
            this->fcgBTAudioEncoderPath->TabIndex = 60;
            this->fcgBTAudioEncoderPath->Text = L"...";
            this->fcgBTAudioEncoderPath->UseVisualStyleBackColor = true;
            this->fcgBTAudioEncoderPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTAudioEncoderPath_Click);
            // 
            // fcgTXAudioEncoderPath
            // 
            this->fcgTXAudioEncoderPath->AllowDrop = true;
            this->fcgTXAudioEncoderPath->Location = System::Drawing::Point(17, 66);
            this->fcgTXAudioEncoderPath->Name = L"fcgTXAudioEncoderPath";
            this->fcgTXAudioEncoderPath->Size = System::Drawing::Size(303, 23);
            this->fcgTXAudioEncoderPath->TabIndex = 59;
            this->fcgTXAudioEncoderPath->TextChanged += gcnew System::EventHandler(this, &frmConfig::fcgTXAudioEncoderPath_TextChanged);
            // 
            // fcgLBAudioEncoderPath
            // 
            this->fcgLBAudioEncoderPath->AutoSize = true;
            this->fcgLBAudioEncoderPath->Location = System::Drawing::Point(13, 49);
            this->fcgLBAudioEncoderPath->Name = L"fcgLBAudioEncoderPath";
            this->fcgLBAudioEncoderPath->Size = System::Drawing::Size(53, 15);
            this->fcgLBAudioEncoderPath->TabIndex = 66;
            this->fcgLBAudioEncoderPath->Text = L"～の指定";
            // 
            // fcgCBAudioOnly
            // 
            this->fcgCBAudioOnly->AutoSize = true;
            this->fcgCBAudioOnly->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
            this->fcgCBAudioOnly->Location = System::Drawing::Point(254, 3);
            this->fcgCBAudioOnly->Name = L"fcgCBAudioOnly";
            this->fcgCBAudioOnly->Size = System::Drawing::Size(95, 19);
            this->fcgCBAudioOnly->TabIndex = 57;
            this->fcgCBAudioOnly->Tag = L"chValue";
            this->fcgCBAudioOnly->Text = L"音声のみ出力";
            this->fcgCBAudioOnly->UseVisualStyleBackColor = true;
            // 
            // fcgCXAudioEncoder
            // 
            this->fcgCXAudioEncoder->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioEncoder->FormattingEnabled = true;
            this->fcgCXAudioEncoder->Location = System::Drawing::Point(18, 8);
            this->fcgCXAudioEncoder->Name = L"fcgCXAudioEncoder";
            this->fcgCXAudioEncoder->Size = System::Drawing::Size(172, 23);
            this->fcgCXAudioEncoder->TabIndex = 55;
            this->fcgCXAudioEncoder->Tag = L"chValue";
            this->fcgCXAudioEncoder->SelectedIndexChanged += gcnew System::EventHandler(this, &frmConfig::fcgCXAudioEncoder_SelectedIndexChanged);
            // 
            // fcgLBAudioTemp
            // 
            this->fcgLBAudioTemp->AutoSize = true;
            this->fcgLBAudioTemp->Location = System::Drawing::Point(8, 185);
            this->fcgLBAudioTemp->Name = L"fcgLBAudioTemp";
            this->fcgLBAudioTemp->Size = System::Drawing::Size(125, 15);
            this->fcgLBAudioTemp->TabIndex = 72;
            this->fcgLBAudioTemp->Text = L"音声一時ファイル出力先";
            // 
            // fcgLBAudioBitrate
            // 
            this->fcgLBAudioBitrate->AutoSize = true;
            this->fcgLBAudioBitrate->Location = System::Drawing::Point(285, 135);
            this->fcgLBAudioBitrate->Name = L"fcgLBAudioBitrate";
            this->fcgLBAudioBitrate->Size = System::Drawing::Size(34, 15);
            this->fcgLBAudioBitrate->TabIndex = 71;
            this->fcgLBAudioBitrate->Text = L"kbps";
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
            this->fcgtabPageAudioOther->Location = System::Drawing::Point(4, 24);
            this->fcgtabPageAudioOther->Name = L"fcgtabPageAudioOther";
            this->fcgtabPageAudioOther->Padding = System::Windows::Forms::Padding(3);
            this->fcgtabPageAudioOther->Size = System::Drawing::Size(372, 274);
            this->fcgtabPageAudioOther->TabIndex = 1;
            this->fcgtabPageAudioOther->Text = L"その他";
            this->fcgtabPageAudioOther->UseVisualStyleBackColor = true;
            // 
            // panel2
            // 
            this->panel2->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->panel2->Location = System::Drawing::Point(15, 135);
            this->panel2->Name = L"panel2";
            this->panel2->Size = System::Drawing::Size(342, 1);
            this->panel2->TabIndex = 73;
            // 
            // fcgLBBatAfterAudioString
            // 
            this->fcgLBBatAfterAudioString->AutoSize = true;
            this->fcgLBBatAfterAudioString->Font = (gcnew System::Drawing::Font(L"Meiryo UI", 9, static_cast<System::Drawing::FontStyle>((System::Drawing::FontStyle::Italic | System::Drawing::FontStyle::Underline)),
                System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(128)));
            this->fcgLBBatAfterAudioString->ForeColor = System::Drawing::SystemColors::ControlDarkDark;
            this->fcgLBBatAfterAudioString->Location = System::Drawing::Point(301, 217);
            this->fcgLBBatAfterAudioString->Name = L"fcgLBBatAfterAudioString";
            this->fcgLBBatAfterAudioString->Size = System::Drawing::Size(27, 15);
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
            this->fcgLBBatBeforeAudioString->Location = System::Drawing::Point(301, 148);
            this->fcgLBBatBeforeAudioString->Name = L"fcgLBBatBeforeAudioString";
            this->fcgLBBatBeforeAudioString->Size = System::Drawing::Size(27, 15);
            this->fcgLBBatBeforeAudioString->TabIndex = 64;
            this->fcgLBBatBeforeAudioString->Text = L" 前& ";
            this->fcgLBBatBeforeAudioString->TextAlign = System::Drawing::ContentAlignment::TopCenter;
            // 
            // fcgBTBatAfterAudioPath
            // 
            this->fcgBTBatAfterAudioPath->Location = System::Drawing::Point(327, 240);
            this->fcgBTBatAfterAudioPath->Name = L"fcgBTBatAfterAudioPath";
            this->fcgBTBatAfterAudioPath->Size = System::Drawing::Size(30, 23);
            this->fcgBTBatAfterAudioPath->TabIndex = 71;
            this->fcgBTBatAfterAudioPath->Tag = L"chValue";
            this->fcgBTBatAfterAudioPath->Text = L"...";
            this->fcgBTBatAfterAudioPath->UseVisualStyleBackColor = true;
            this->fcgBTBatAfterAudioPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTBatAfterAudioPath_Click);
            // 
            // fcgTXBatAfterAudioPath
            // 
            this->fcgTXBatAfterAudioPath->AllowDrop = true;
            this->fcgTXBatAfterAudioPath->Location = System::Drawing::Point(123, 241);
            this->fcgTXBatAfterAudioPath->Name = L"fcgTXBatAfterAudioPath";
            this->fcgTXBatAfterAudioPath->Size = System::Drawing::Size(202, 23);
            this->fcgTXBatAfterAudioPath->TabIndex = 70;
            this->fcgTXBatAfterAudioPath->Tag = L"chValue";
            // 
            // fcgLBBatAfterAudioPath
            // 
            this->fcgLBBatAfterAudioPath->AutoSize = true;
            this->fcgLBBatAfterAudioPath->Location = System::Drawing::Point(37, 245);
            this->fcgLBBatAfterAudioPath->Name = L"fcgLBBatAfterAudioPath";
            this->fcgLBBatAfterAudioPath->Size = System::Drawing::Size(67, 15);
            this->fcgLBBatAfterAudioPath->TabIndex = 69;
            this->fcgLBBatAfterAudioPath->Text = L"バッチファイル";
            // 
            // fcgCBRunBatAfterAudio
            // 
            this->fcgCBRunBatAfterAudio->AutoSize = true;
            this->fcgCBRunBatAfterAudio->Location = System::Drawing::Point(15, 216);
            this->fcgCBRunBatAfterAudio->Name = L"fcgCBRunBatAfterAudio";
            this->fcgCBRunBatAfterAudio->Size = System::Drawing::Size(217, 19);
            this->fcgCBRunBatAfterAudio->TabIndex = 68;
            this->fcgCBRunBatAfterAudio->Tag = L"chValue";
            this->fcgCBRunBatAfterAudio->Text = L"音声エンコード終了後、バッチ処理を行う";
            this->fcgCBRunBatAfterAudio->UseVisualStyleBackColor = true;
            // 
            // panel1
            // 
            this->panel1->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
            this->panel1->Location = System::Drawing::Point(15, 205);
            this->panel1->Name = L"panel1";
            this->panel1->Size = System::Drawing::Size(342, 1);
            this->panel1->TabIndex = 67;
            // 
            // fcgBTBatBeforeAudioPath
            // 
            this->fcgBTBatBeforeAudioPath->Location = System::Drawing::Point(327, 173);
            this->fcgBTBatBeforeAudioPath->Name = L"fcgBTBatBeforeAudioPath";
            this->fcgBTBatBeforeAudioPath->Size = System::Drawing::Size(30, 23);
            this->fcgBTBatBeforeAudioPath->TabIndex = 66;
            this->fcgBTBatBeforeAudioPath->Tag = L"chValue";
            this->fcgBTBatBeforeAudioPath->Text = L"...";
            this->fcgBTBatBeforeAudioPath->UseVisualStyleBackColor = true;
            this->fcgBTBatBeforeAudioPath->Click += gcnew System::EventHandler(this, &frmConfig::fcgBTBatBeforeAudioPath_Click);
            // 
            // fcgTXBatBeforeAudioPath
            // 
            this->fcgTXBatBeforeAudioPath->AllowDrop = true;
            this->fcgTXBatBeforeAudioPath->Location = System::Drawing::Point(123, 173);
            this->fcgTXBatBeforeAudioPath->Name = L"fcgTXBatBeforeAudioPath";
            this->fcgTXBatBeforeAudioPath->Size = System::Drawing::Size(202, 23);
            this->fcgTXBatBeforeAudioPath->TabIndex = 65;
            this->fcgTXBatBeforeAudioPath->Tag = L"chValue";
            // 
            // fcgLBBatBeforeAudioPath
            // 
            this->fcgLBBatBeforeAudioPath->AutoSize = true;
            this->fcgLBBatBeforeAudioPath->Location = System::Drawing::Point(37, 176);
            this->fcgLBBatBeforeAudioPath->Name = L"fcgLBBatBeforeAudioPath";
            this->fcgLBBatBeforeAudioPath->Size = System::Drawing::Size(67, 15);
            this->fcgLBBatBeforeAudioPath->TabIndex = 63;
            this->fcgLBBatBeforeAudioPath->Text = L"バッチファイル";
            // 
            // fcgCBRunBatBeforeAudio
            // 
            this->fcgCBRunBatBeforeAudio->AutoSize = true;
            this->fcgCBRunBatBeforeAudio->Location = System::Drawing::Point(15, 148);
            this->fcgCBRunBatBeforeAudio->Name = L"fcgCBRunBatBeforeAudio";
            this->fcgCBRunBatBeforeAudio->Size = System::Drawing::Size(217, 19);
            this->fcgCBRunBatBeforeAudio->TabIndex = 62;
            this->fcgCBRunBatBeforeAudio->Tag = L"chValue";
            this->fcgCBRunBatBeforeAudio->Text = L"音声エンコード開始前、バッチ処理を行う";
            this->fcgCBRunBatBeforeAudio->UseVisualStyleBackColor = true;
            // 
            // fcgCXAudioPriority
            // 
            this->fcgCXAudioPriority->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
            this->fcgCXAudioPriority->FormattingEnabled = true;
            this->fcgCXAudioPriority->Location = System::Drawing::Point(168, 19);
            this->fcgCXAudioPriority->Name = L"fcgCXAudioPriority";
            this->fcgCXAudioPriority->Size = System::Drawing::Size(136, 23);
            this->fcgCXAudioPriority->TabIndex = 13;
            this->fcgCXAudioPriority->Tag = L"chValue";
            // 
            // fcgLBAudioPriority
            // 
            this->fcgLBAudioPriority->AutoSize = true;
            this->fcgLBAudioPriority->Location = System::Drawing::Point(41, 22);
            this->fcgLBAudioPriority->Name = L"fcgLBAudioPriority";
            this->fcgLBAudioPriority->Size = System::Drawing::Size(67, 15);
            this->fcgLBAudioPriority->TabIndex = 12;
            this->fcgLBAudioPriority->Text = L"音声優先度";
            // 
            // frmConfig
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(96, 96);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Dpi;
            this->ClientSize = System::Drawing::Size(1008, 601);
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
            this->fcgtabPageMPG->ResumeLayout(false);
            this->fcgtabPageMPG->PerformLayout();
            this->fcgtabPageMux->ResumeLayout(false);
            this->fcgtabPageMux->PerformLayout();
            this->fcgtabPageBat->ResumeLayout(false);
            this->fcgtabPageBat->PerformLayout();
            this->fcgtabPageInternal->ResumeLayout(false);
            this->fcgtabPageInternal->PerformLayout();
            this->fcgtabControlVCE->ResumeLayout(false);
            this->tabPageVideoEnc->ResumeLayout(false);
            this->tabPageVideoEnc->PerformLayout();
            this->fcgPNQP->ResumeLayout(false);
            this->fcgPNQP->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUQPI))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUQPP))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUQPB))->EndInit();
            this->fcggroupBoxColor->ResumeLayout(false);
            this->fcggroupBoxColor->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNURefFrames))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVBVBufSize))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUQPMax))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUQPMin))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgpictureBoxVCEEnabled))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgpictureBoxVCEDisabled))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUSlices))->EndInit();
            this->fcgGroupBoxAspectRatio->ResumeLayout(false);
            this->fcgGroupBoxAspectRatio->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUAspectRatioY))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUAspectRatioX))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUGopLength))->EndInit();
            this->fcgPNBitrate->ResumeLayout(false);
            this->fcgPNBitrate->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUBitrate))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUMaxkbps))->EndInit();
            this->fcgPNBframes->ResumeLayout(false);
            this->fcgPNBframes->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUBframes))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUBDeltaQP))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUBRefDeltaQP))->EndInit();
            this->fcgPNHEVCLevelProfile->ResumeLayout(false);
            this->fcgPNHEVCLevelProfile->PerformLayout();
            this->fcgPNH264Features->ResumeLayout(false);
            this->fcgPNH264Features->PerformLayout();
            this->tabPageExOpt->ResumeLayout(false);
            this->tabPageExOpt->PerformLayout();
            this->fcggroupBoxVppDeinterlace->ResumeLayout(false);
            this->fcggroupBoxVppDeinterlace->PerformLayout();
            this->fcgPNVppAfs->ResumeLayout(false);
            this->fcgPNVppAfs->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgTBVppAfsThreCMotion))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgTBVppAfsThreYMotion))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgTBVppAfsThreDeint))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgTBVppAfsThreShift))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgTBVppAfsCoeffShift))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsRight))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsLeft))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsBottom))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsUp))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgTBVppAfsMethodSwitch))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsThreCMotion))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsThreShift))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsThreDeint))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsThreYMotion))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsCoeffShift))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUVppAfsMethodSwitch))->EndInit();
            this->fcggroupboxResize->ResumeLayout(false);
            this->fcggroupboxResize->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUResizeH))->EndInit();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUResizeW))->EndInit();
            this->fcgCSExeFiles->ResumeLayout(false);
            this->fcgtabControlAudio->ResumeLayout(false);
            this->fcgtabPageAudioMain->ResumeLayout(false);
            this->fcgtabPageAudioMain->PerformLayout();
            this->fcgPNAudioInternal->ResumeLayout(false);
            this->fcgPNAudioInternal->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUAudioBitrateInternal))->EndInit();
            this->fcgPNAudioExt->ResumeLayout(false);
            this->fcgPNAudioExt->PerformLayout();
            (cli::safe_cast<System::ComponentModel::ISupportInitialize ^>(this->fcgNUAudioBitrate))->EndInit();
            this->fcgtabPageAudioOther->ResumeLayout(false);
            this->fcgtabPageAudioOther->PerformLayout();
            this->ResumeLayout(false);
            this->PerformLayout();

        }
#pragma endregion
    private:
        const SYSTEM_DATA *sys_dat;
        CONF_GUIEX *conf;
        LocalSettings LocalStg;
        bool CurrentPipeEnabled;
        bool stgChanged;
        String^ CurrentStgDir;
        ToolStripMenuItem^ CheckedStgMenuItem;
        CONF_GUIEX *cnf_stgSelected;
        String^ lastQualityStr;
#ifdef HIDE_MPEG2
        TabPage^ tabPageMpgMux;
#endif
        VidEncInfo encInfo;
        Task<VidEncInfo>^ taskEncInfo;
        CancellationTokenSource^ taskEncInfoCancell;
    private:
        System::Int32 GetCurrentAudioDefaultBitrate();
        delegate System::Void qualityTimerChangeDelegate();
        System::Void InitComboBox();
        bool AudioIntEncoderEnabled(const AUDIO_SETTINGS *astg, bool isAuoLinkMode);
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
        System::Void GetfcgTSLSettingsNotes(char *notes, int nSize);
        System::Void SetfcgTSLSettingsNotes(const char *notes);
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
        }
    private:
        System::Void NUSelectAll(System::Object^  sender, System::EventArgs^  e) {
             NumericUpDown^ NU = dynamic_cast<NumericUpDown^>(sender);
             NU->Select(0, NU->Text->Length);
         }
    private:
        System::Void setComboBox(ComboBox^ CX, const X264_OPTION_STR * list) {
            CX->BeginUpdate();
            CX->Items->Clear();
            for (int i = 0; list[i].desc; i++)
                CX->Items->Add(String(list[i].desc).ToString());
            CX->EndUpdate();
        }
    private:
        System::Void setComboBox(ComboBox^ CX, const CX_DESC * list) {
            CX->BeginUpdate();
            CX->Items->Clear();
            for (int i = 0; list[i].desc; i++)
                CX->Items->Add(String(list[i].desc).ToString());
            CX->EndUpdate();
        }
    private:
        System::Void setComboBox(ComboBox ^CX, const CX_DESC *list, const TCHAR *ignore) {
            CX->BeginUpdate();
            CX->Items->Clear();
            for (int i = 0; list[i].desc; i++) {
                if (ignore && strcmp(ignore, list[i].desc) == 0) {
                    //インデックスの順番を保持するため、途中の場合は"-----"をいれておく
                    if (list[i + 1].desc) {
                        CX->Items->Add(String("-----").ToString());
                    }
                } else {
                    CX->Items->Add(String(list[i].desc).ToString());
                }
            }
            CX->EndUpdate();
        }
    private:
        System::Void setComboBox(ComboBox^ CX, const CX_DESC * list, int count) {
            CX->BeginUpdate();
            CX->Items->Clear();
            for (int i = 0; list[i].desc && i < count; i++)
                CX->Items->Add(String(list[i].desc).ToString());
            CX->EndUpdate();
        }
    private:
        System::Void setComboBox(ComboBox^ CX, const WCHAR * const * list) {
            CX->BeginUpdate();
            CX->Items->Clear();
            for (int i = 0; list[i]; i++)
                CX->Items->Add(String(list[i]).ToString());
            CX->EndUpdate();
        }
    private:
        System::Void setComboBox(ComboBox^ CX, const char * const * list) {
            CX->BeginUpdate();
            CX->Items->Clear();
            for (int i = 0; list[i]; i++)
                CX->Items->Add(String(list[i]).ToString());
            CX->EndUpdate();
        }
    private:
        System::Void setPriorityList(ComboBox^ CX) {
            CX->BeginUpdate();
            CX->Items->Clear();
            for (int i = 0; priority_table[i].text; i++)
                CX->Items->Add(String(priority_table[i].text).ToString());
            CX->EndUpdate();
        }
    private:
        System::Void setMuxerCmdExNames(ComboBox^ CX, int muxer_index) {
            CX->BeginUpdate();
            CX->Items->Clear();
            MUXER_SETTINGS *mstg = &sys_dat->exstg->s_mux[muxer_index];
            for (int i = 0; i < mstg->ex_count; i++)
                CX->Items->Add(String(mstg->ex_cmd[i].name).ToString());
            CX->EndUpdate();
        }
    private:
        System::Void setAudioEncoderNames() {
            fcgCXAudioEncoder->BeginUpdate();
            fcgCXAudioEncoder->Items->Clear();
            //fcgCXAudioEncoder->Items->AddRange(reinterpret_cast<array<String^>^>(LocalStg.audEncName->ToArray(String::typeid)));
            fcgCXAudioEncoder->Items->AddRange(LocalStg.audEncName->ToArray());
            fcgCXAudioEncoder->EndUpdate();

            fcgCXAudioEncoderInternal->BeginUpdate();
            fcgCXAudioEncoderInternal->Items->Clear();
            for (int i = 0; i < sys_dat->exstg->s_aud_int_count; i++) {
                if (AudioIntEncoderEnabled(&sys_dat->exstg->s_aud_int[i], false)) {
                    fcgCXAudioEncoderInternal->Items->Add(String(sys_dat->exstg->s_aud_int[i].dispname).ToString());
                } else {
                    fcgCXAudioEncoderInternal->Items->Add(String(L"-----").ToString());
                }
            }
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
                MessageBox::Show(this, L"入力された文字数が多すぎます。減らしてください。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
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
        System::Void fcgBTVideoEncoderPath_Click(System::Object^  sender, System::EventArgs^  e) {
            openExeFile(fcgTXVideoEncoderPath, LocalStg.vidEncName);
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
        System::Void fcgBTMPGMuxerPath_Click(System::Object^  sender, System::EventArgs^  e) {
            openExeFile(fcgTXMPGMuxerPath, LocalStg.MPGMuxerExeName);
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
        System::Void fcgBTBatAfterPath_Click(System::Object^  sender, System::EventArgs^  e) {
            if (openAndSetFilePath(fcgTXBatAfterPath, L"バッチファイル", ".bat", LocalStg.LastBatDir))
                LocalStg.LastBatDir = Path::GetDirectoryName(fcgTXBatAfterPath->Text);
        }
    private:
        System::Void fcgBTBatBeforePath_Click(System::Object^  sender, System::EventArgs^  e) {
            if (openAndSetFilePath(fcgTXBatBeforePath, L"バッチファイル", ".bat", LocalStg.LastBatDir))
                LocalStg.LastBatDir = Path::GetDirectoryName(fcgTXBatBeforePath->Text);
        }
    private:
        System::Void fcgBTBatBeforeAudioPath_Click(System::Object^  sender, System::EventArgs^  e) {
            if (openAndSetFilePath(fcgTXBatBeforeAudioPath, L"バッチファイル", ".bat", LocalStg.LastBatDir))
                LocalStg.LastBatDir = Path::GetDirectoryName(fcgTXBatBeforeAudioPath->Text);
        }
    private:
        System::Void fcgBTBatAfterAudioPath_Click(System::Object^  sender, System::EventArgs^  e) {
            if (openAndSetFilePath(fcgTXBatAfterAudioPath, L"バッチファイル", ".bat", LocalStg.LastBatDir))
                LocalStg.LastBatDir = Path::GetDirectoryName(fcgTXBatAfterAudioPath->Text);
        }
    private:
        System::Void SetCXIndex(ComboBox^ CX, int index) {
            CX->SelectedIndex = clamp(index, 0, CX->Items->Count - 1);
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
            LocalStg.vidEncPath = fcgTXVideoEncoderPath->Text;
            fcgBTVideoEncoderPath->ContextMenuStrip = (File::Exists(fcgTXVideoEncoderPath->Text)) ? fcgCSExeFiles : nullptr;
            GetVidEncInfoAsync();
        }
    private:
        System::Void fcgTXAudioEncoderPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            LocalStg.audEncPath[fcgCXAudioEncoder->SelectedIndex] = fcgTXAudioEncoderPath->Text;
            fcgBTAudioEncoderPath->ContextMenuStrip = (File::Exists(fcgTXAudioEncoderPath->Text)) ? fcgCSExeFiles : nullptr;
        }
    private:
        System::Void fcgTXMP4MuxerPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            LocalStg.MP4MuxerPath = fcgTXMP4MuxerPath->Text;
            fcgBTMP4MuxerPath->ContextMenuStrip = (File::Exists(fcgTXMP4MuxerPath->Text)) ? fcgCSExeFiles : nullptr;
        }
    private:
        System::Void fcgTXTC2MP4Path_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            LocalStg.TC2MP4Path = fcgTXTC2MP4Path->Text;
            fcgBTTC2MP4Path->ContextMenuStrip = (File::Exists(fcgTXTC2MP4Path->Text)) ? fcgCSExeFiles : nullptr;
        }
    private:
        System::Void fcgTXMP4RawMuxerPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            LocalStg.MP4RawPath = fcgTXMP4RawPath->Text;
            fcgBTMP4RawPath->ContextMenuStrip = (File::Exists(fcgTXMP4RawPath->Text)) ? fcgCSExeFiles : nullptr;
        }
    private:
        System::Void fcgTXMKVMuxerPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            LocalStg.MKVMuxerPath = fcgTXMKVMuxerPath->Text;
            fcgBTMKVMuxerPath->ContextMenuStrip = (File::Exists(fcgTXMKVMuxerPath->Text)) ? fcgCSExeFiles : nullptr;
        }
    private:
        System::Void fcgTXMPGMuxerPath_TextChanged(System::Object^  sender, System::EventArgs^  e) {
            LocalStg.MPGMuxerPath = fcgTXMPGMuxerPath->Text;
            fcgBTMPGMuxerPath->ContextMenuStrip = (File::Exists(fcgTXMPGMuxerPath->Text)) ? fcgCSExeFiles : nullptr;
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
                { fcgBTVideoEncoderPath->Name,   fcgTXVideoEncoderPath->Text,   sys_dat->exstg->s_vid.help_cmd },
                { fcgBTAudioEncoderPath->Name,   fcgTXAudioEncoderPath->Text,   sys_dat->exstg->s_aud_ext[fcgCXAudioEncoder->SelectedIndex].cmd_help },
                { fcgBTMP4MuxerPath->Name,       fcgTXMP4MuxerPath->Text,       sys_dat->exstg->s_mux[MUXER_MP4].help_cmd },
                { fcgBTTC2MP4Path->Name,         fcgTXTC2MP4Path->Text,         sys_dat->exstg->s_mux[MUXER_TC2MP4].help_cmd },
                { fcgBTMP4RawPath->Name,         fcgTXMP4RawPath->Text,         sys_dat->exstg->s_mux[MUXER_MP4_RAW].help_cmd },
                { fcgBTMKVMuxerPath->Name,       fcgTXMKVMuxerPath->Text,       sys_dat->exstg->s_mux[MUXER_MKV].help_cmd },
                { fcgBTMPGMuxerPath->Name,       fcgTXMPGMuxerPath->Text,       sys_dat->exstg->s_mux[MUXER_MPG].help_cmd }
            };
            for (int i = 0; i < ControlList->Length; i++) {
                if (NULL == String::Compare(CS->SourceControl->Name, ControlList[i].Name)) {
                    ShowExehelp(ControlList[i].Path, String(ControlList[i].args).ToString());
                    return;
                }
            }
            MessageBox::Show(L"ヘルプ表示用のコマンドが設定されていません。", L"エラー", MessageBoxButtons::OK, MessageBoxIcon::Error);
        }
    private:
        System::Void fcgLBguiExBlog_LinkClicked(System::Object^  sender, System::Windows::Forms::LinkLabelLinkClickedEventArgs^  e) {
            fcgLBguiExBlog->LinkVisited = true;
            try {
                System::Diagnostics::Process::Start(String(sys_dat->exstg->blog_url).ToString());
            } catch (...) {
                //いちいちメッセージとか出さない
            };
        }
    private:
        System::Void fcgTBVppAfsScroll(System::Object ^sender, System::EventArgs ^e) {
            System::Windows::Forms::TrackBar ^senderTB = dynamic_cast<System::Windows::Forms::TrackBar ^>(sender);
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
