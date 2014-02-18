#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <math.h>
#include <process.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <intrin.h>
#include "VCECore.h"
#include "VCEParam.h"
#include "OvEncodeSample.h"
#include "h264_level.h"

#pragma comment(lib, "OpenVideo.lib")
#pragma comment(lib, "OpenCL.lib")

#ifndef clamp
#define clamp(x, low, high) (((x) <= (high)) ? (((x) >= (low)) ? (x) : (low)) : (high))
#endif

func_vce_mes vce_print_mes = NULL;

typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

static DWORD CountSetBits(ULONG_PTR bitMask) {
    DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
    DWORD bitSetCount = 0;
    for (ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT; bitTest; bitTest >>= 1)
        bitSetCount += ((bitMask & bitTest) != 0);

    return bitSetCount;
}

static BOOL getProcessorCount(DWORD *physical_processor_core, DWORD *logical_processor_core) {
	*physical_processor_core = 0;
	*logical_processor_core = 0;

    LPFN_GLPI glpi = (LPFN_GLPI)GetProcAddress(GetModuleHandle("kernel32"), "GetLogicalProcessorInformation");
    if (NULL == glpi)
		return FALSE;

    DWORD returnLength = 0;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
	while (FALSE == glpi(buffer, &returnLength)) {
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			if (buffer) 
				free(buffer);
			if (NULL == (buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength)))
				return FALSE;
		}
	}

    DWORD logicalProcessorCount = 0;
    DWORD numaNodeCount = 0;
    DWORD processorCoreCount = 0;
    DWORD processorL1CacheCount = 0;
    DWORD processorL2CacheCount = 0;
    DWORD processorL3CacheCount = 0;
    DWORD processorPackageCount = 0;
    PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = buffer;
    for (DWORD byteOffset = 0; byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength;
		byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)) {
        switch (ptr->Relationship) {
        case RelationNumaNode:
            // Non-NUMA systems report a single record of this type.
            numaNodeCount++;
            break;
        case RelationProcessorCore:
            processorCoreCount++;
            // A hyperthreaded core supplies more than one logical processor.
            logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
            break;

        case RelationCache:
			{
            // Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
            PCACHE_DESCRIPTOR Cache = &ptr->Cache;
			processorL1CacheCount += (Cache->Level == 1);
			processorL2CacheCount += (Cache->Level == 2);
			processorL3CacheCount += (Cache->Level == 3);
            break;
			}
        case RelationProcessorPackage:
            // Logical processors share a physical package.
            processorPackageCount++;
            break;

        default:
            //Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.
            break;
        }
        ptr++;
    }

	*physical_processor_core = processorCoreCount;
	*logical_processor_core = logicalProcessorCount;

    return TRUE;
}

static int getCPUName(char *buf, size_t nSize) {
    int CPUInfo[4] = {-1};
    __cpuid(CPUInfo, 0x80000000);
    unsigned int nExIds = CPUInfo[0];
	if (nSize < 0x40)
		return 1;
	memset(buf, 0, 0x40);
    for (unsigned int i = 0x80000000; i <= nExIds; i++) {
        __cpuid(CPUInfo, i);
		int offset = 0;
		switch (i) {
			case 0x80000002: offset =  0; break;
			case 0x80000003: offset = 16; break;
			case 0x80000004: offset = 32; break;
			default:
				continue;
		}
		memcpy(buf + offset, CPUInfo, sizeof(CPUInfo)); 
	}
	//crop space beforce string
	for (int i = 0; buf[i]; i++) {
		if (buf[i] != ' ') {
			if (i)
				memmove(buf, buf + i, strlen(buf + i) + 1);
			break;
		}
	}
	//remove space witch continues.
	for (int i = 0; buf[i]; i++) {
		if (buf[i] == ' ') {
			int space_idx = i;
			while (buf[i+1] == ' ')
				i++;
			if (i != space_idx)
				memmove(buf + space_idx + 1, buf + i + 1, strlen(buf + i + 1) + 1);
		}
	}
	return 0;
}

static bool get_gpu_frequency(cl_device_id device_id, uint32 *gpu_freq) {
	size_t size;
	return 0 != clGetDeviceInfo(device_id, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(gpu_freq[0]), gpu_freq, &size);
}

void init_vce_prm(OvConfigCtrl *cnf) {
	if (cnf) {
		ZeroMemory(cnf, sizeof(cnf));
		cnf->encodeMode = OVE_AVC_FULL;
		cnf->priority = OVE_ENCODE_TASK_PRIORITY_LEVEL1;
		cnf->pictFormat = OVE_PICTURE_FORMAT_NV12;
		cnf->profileLevel.profile = 77;
		cnf->profileLevel.level = 0;

		cnf->pictControl.size = sizeof(cnf->pictControl);
		cnf->pictControl.cabacEnable = 1;
		cnf->pictControl.encNumSlicesPerFrame = 1;
		cnf->pictControl.encCropBottomOffset = 0;

		cnf->rateControl.size = sizeof(cnf->rateControl);
		cnf->rateControl.encRateControlTargetBitRate =  4000 * 1000;
		cnf->rateControl.encRateControlPeakBitRate   =  8000 * 1000;
		cnf->rateControl.encVBVBufferSize = cnf->rateControl.encRateControlPeakBitRate / 2;
		cnf->rateControl.encGOPSize = CL_UINT_MAX;
		cnf->rateControl.encQP_I = 24;
		cnf->rateControl.encQP_P = 26;
		cnf->rateControl.encQP_B = 0;

		cnf->meControl.size = sizeof(cnf->meControl);
		cnf->meControl.imeDecimationSearch = 1;
		cnf->meControl.motionEstHalfPixel = 1;
		cnf->meControl.motionEstQuarterPixel = 1;
		cnf->meControl.encSearchRangeX = 16;
		cnf->meControl.encSearchRangeY = 16;
		cnf->meControl.encDisableSubMode = 120;
		cnf->meControl.encEnImeOverwDisSubm = 1;
		cnf->meControl.encImeOverwDisSubmNo = 1;
		cnf->meControl.encIME2SearchRangeX = 4;
		cnf->meControl.encIME2SearchRangeY = 4;

		cnf->rdoControl.size = sizeof(cnf->rdoControl);
	}
}

#pragma warning (push)
#pragma warning (disable: 4100)
int vce_mes(FILE *fp, int log_level, double progress, const char *fmt, ... ) {
    // retrieve the variable arguments
    va_list args;
    va_start(args, fmt);
    
    int len = _vscprintf(fmt, args) + 1;// _vscprintf doesn't count terminating '\0'
    
    char *buffer = (char*)malloc(len * sizeof(buffer[0]));

    vsprintf_s(buffer, len, fmt, args); // C4996
    int ret = fprintf(fp, buffer);

    free(buffer);
	return ret;
}
#pragma warning (pop)

int vce_print(FILE *fp, int log_level, const char *format, ... ) {
    // retrieve the variable arguments
    va_list args;
    va_start(args, format);
    
    int len = _vscprintf(format, args) + 1;// _vscprintf doesn't count terminating '\0'
    
    char *buffer = (char*)malloc(len * sizeof(buffer[0]));

    vsprintf_s(buffer, len, format, args); // C4996
    int ret = vce_print_mes(fp, log_level, 0, buffer);

    free(buffer);
	return ret;
}

static void vce_info_init(vce_info_t *info, double _fps, int _total_frames) {
	info->filesize = 0;
	info->fps = _fps;
	info->total_frames = _total_frames;
	info->tm_last_update = timeGetTime() - 1000;
}

static void vce_info_enc_start(vce_info_t *info) {
	info->tm_start = timeGetTime();
}

static void vce_info_enc_fin(vce_info_t *info) {
	double encode_fps = info->output_frames * 1000.0 / (double)(timeGetTime() - info->tm_start);

	vce_print(stderr, VCE_LOG_INFO, "encoded %d frames, %0.2lf fps, %0.2lf kbps\n\n", 
		info->output_frames,
		encode_fps,
		(double)(info->filesize * 8) * info->fps / (1000.0 * info->output_frames)
		);
}

static void vce_info_print_status(vce_info_t *info) {
	DWORD tm_current = timeGetTime();
	if (tm_current - info->tm_last_update > 800 && info->output_frames) {
		double encode_fps = info->output_frames * 1000 / (double)(tm_current - info->tm_start);
		double current_kbps = (double)info->filesize * 8 * info->fps / (double)(info->output_frames * 1000.0);
		if (info->total_frames) {
			uint32 remaining_time = (uint32)((info->total_frames - info->output_frames) * 1000.0 / encode_fps);
			int hh = remaining_time / (60*60*1000);
			remaining_time -= hh * (60*60*1000);
			int mm = remaining_time / (60*1000);
			remaining_time -= mm * (60*1000);
			int ss = (remaining_time + 500) / 1000;
			info->tm_last_update = tm_current;
			vce_print_mes(stderr, VCE_LOG_INFO, info->output_frames / (double)info->total_frames,
				"%d/%d frames: %.2lf fps, %0.2lf kb/s, remain %d:%02d:%02d  \r",
				info->output_frames, info->total_frames,
				encode_fps, current_kbps,
				hh, mm, ss);
		} else {
			vce_print_mes(stderr, VCE_LOG_INFO, 0,
				"%d frames: %.2lf fps, %0.2lf kb/s  \r",
				info->output_frames,
				encode_fps, current_kbps);
		}
	}
}

void vce_close(vce_handle_t *vce_hnd) {
	if (vce_hnd) {
		if (vce_hnd->fp_read) { fclose(vce_hnd->fp_read); vce_hnd->fp_read = NULL; }
		if (vce_hnd->fp_write) { fclose(vce_hnd->fp_write); vce_hnd->fp_write = NULL; }
		//cl_device_id clDeviceID = reinterpret_cast<cl_device_id>(vce_hnd->deviceId);
		//displayFps(&vce_hnd->perfCounter, clDeviceID);
		encodeClose(&vce_hnd->encodeHandle);
		encodeDestroy(vce_hnd->oveContext);
		if (vce_hnd->deviceHandle.deviceInfo)
			delete [] vce_hnd->deviceHandle.deviceInfo;
		free(vce_hnd);
	}
}

static BOOL check_param(OvConfigCtrl *configCtrl, VCEExtParam *ext_prm) {
	BOOL error = FALSE;
	//checking resolution
	if (configCtrl->width == 0 || configCtrl->height == 0) {
		vce_print(stderr, VCE_LOG_ERROR, "invalid resolution... %dx%d\n", configCtrl->width, configCtrl->height);
		error = TRUE;
	} else {
		int w_mul = 2, h_mul = 2;
		if (is_interlaced(ext_prm->pic_struct)) h_mul *= 2;
		if (configCtrl->width % w_mul) {
			vce_print(stderr, VCE_LOG_ERROR, "invalid resolution... %dx%d, width cannot be divided by %d.\n", configCtrl->width, configCtrl->height, w_mul);
			error = TRUE;
		}
		if (configCtrl->height % h_mul) {
			vce_print(stderr, VCE_LOG_ERROR, "invalid resolution... %dx%d, height cannot be divided by %d.\n", configCtrl->width, configCtrl->height, h_mul);
			error = TRUE;
		}
	}
	if (configCtrl->height % 16) {
		const int aligned_height = (configCtrl->height + 15) & (~15);
		configCtrl->pictControl.encCropBottomOffset = (aligned_height - configCtrl->height) >> 1;
	}
	//checking basic param
	if (ext_prm->pic_struct == 0) ext_prm->pic_struct = OVE_PICTURE_STRUCTURE_H264_FRAME;
	if (configCtrl->pictFormat == 0) configCtrl->pictFormat = OVE_PICTURE_FORMAT_NV12;
	if (configCtrl->encodeMode == 0) configCtrl->encodeMode = OVE_AVC_FULL;
	if (configCtrl->pictControl.encNumSlicesPerFrame == 0) configCtrl->pictControl.encNumSlicesPerFrame = 1;
	if (configCtrl->pictControl.encNumMBsPerSlice == 0) {
		if (configCtrl->width != 0 || configCtrl->height != 0) {
			const int height_ceil = 16 * (1 + (is_interlaced(ext_prm->pic_struct) ? 1 : 0));
			configCtrl->pictControl.encNumMBsPerSlice = ((configCtrl->width + 15) / 16) * ((configCtrl->height + height_ceil - 1) / height_ceil) / configCtrl->pictControl.encNumSlicesPerFrame;
		}
	}
	
	//checking framerate
	if (TRUE) {
		static const struct {
			uint32 fps_num, fps_den;
		} VCE_VALID_FPS[] = {
			{ 15, 1 }, { 24, 1 }, { 25, 1 }, { 30, 1 }, { 50, 1 }, { 60, 1 },
			{ 15000, 1001 }, { 24000, 1001 }, { 30000, 1001 }, { 60000, 1001 }, { 0, 0 }
		};
		int best_fit = -1;
		double best_fit_delta = 1e-3;
		const double fps_set = configCtrl->rateControl.encRateControlFrameRateNumerator / (double)configCtrl->rateControl.encRateControlFrameRateDenominator;
		for (int i = 0; VCE_VALID_FPS[i].fps_num; i++) {
			double fps_delta = abs(VCE_VALID_FPS[i].fps_num / (double)VCE_VALID_FPS[i].fps_den - fps_set);
			if (fps_delta < best_fit_delta) {
				best_fit_delta = fps_delta;
				best_fit = i;
			}
		}
		if (best_fit < 0) {
			vce_print(stderr, VCE_LOG_ERROR, "invalid framerate... %d/%d\n", 
				configCtrl->rateControl.encRateControlFrameRateNumerator, 
				configCtrl->rateControl.encRateControlFrameRateDenominator);
			error = TRUE;
		} else {
			configCtrl->rateControl.encRateControlFrameRateNumerator = VCE_VALID_FPS[best_fit].fps_num;
			configCtrl->rateControl.encRateControlFrameRateDenominator = VCE_VALID_FPS[best_fit].fps_den;
		}
	}
	
	//checking encode mode and bitrate...
	if (configCtrl->rateControl.encRateControlMethod == VCE_RC_CQP) {
		configCtrl->rateControl.encQP_I = clamp(configCtrl->rateControl.encQP_I, 0, 51);
		configCtrl->rateControl.encQP_P = clamp(configCtrl->rateControl.encQP_P, 0, 51);
		configCtrl->rateControl.encQP_B = clamp(configCtrl->rateControl.encQP_B, 0, 51);
		configCtrl->rateControl.encVBVBufferSize = 0;
	} else if (configCtrl->rateControl.encRateControlMethod == VCE_RC_CBR
		    || configCtrl->rateControl.encRateControlMethod == VCE_RC_VBR) {
		if (configCtrl->rateControl.encRateControlTargetBitRate == 0) {
			vce_print(stderr, VCE_LOG_ERROR, "invalid bitrate set... %d kbps\n", configCtrl->rateControl.encRateControlTargetBitRate / 1000);
			error = TRUE;
		}
		configCtrl->rateControl.encRateControlPeakBitRate = max(configCtrl->rateControl.encRateControlPeakBitRate, configCtrl->rateControl.encRateControlTargetBitRate);
		configCtrl->rateControl.encVBVBufferSize = configCtrl->rateControl.encRateControlPeakBitRate / 2;
	} else {
		vce_print(stderr, VCE_LOG_ERROR, "invalid rate control method... %d\n", configCtrl->rateControl.encRateControlMethod);
		error = TRUE;
	}
	//GOP長のチェック
	if (configCtrl->rateControl.encGOPSize == -1) {
		uint32 fps_num = configCtrl->rateControl.encRateControlFrameRateNumerator;
		uint32 fps_den = configCtrl->rateControl.encRateControlFrameRateDenominator;
		if (fps_num != 0 && fps_den != 0) configCtrl->rateControl.encGOPSize = (int)((fps_num + (fps_den - 1)) / fps_den) * 10;
	}
	configCtrl->pictControl.encIDRPeriod = configCtrl->rateControl.encGOPSize;
	//checking level and profile
	if (configCtrl->profileLevel.level == 0) {
		configCtrl->profileLevel.level = calc_auto_level(configCtrl->width, configCtrl->height, 1, is_interlaced(ext_prm->pic_struct),
			configCtrl->rateControl.encRateControlFrameRateNumerator,
			configCtrl->rateControl.encRateControlFrameRateDenominator,
			0, configCtrl->rateControl.encVBVBufferSize);
	}
	if (configCtrl->profileLevel.profile == 0) configCtrl->profileLevel.profile = 77;
	//check vbv bufsize
	if (configCtrl->rateControl.encVBVBufferSize == 0) {
		int vbv_max = 0, vbv_buf = 0;
		get_vbv_value(&vbv_max, &vbv_buf, get_cx_index(list_avc_level, configCtrl->profileLevel.level),
			get_cx_index(list_avc_profile, configCtrl->profileLevel.profile));
		configCtrl->rateControl.encVBVBufferSize = vbv_buf * 1000;
	}
	return error;
}

static void print_vce_prm(const OvConfigCtrl *configCtrl, VCEExtParam *ext_prm) {
	char info[4096];
	uint32 info_len = 0;
#define PRINT_INFO(fmt, ...) { info_len += sprintf_s(info + info_len, _countof(info) - info_len, fmt, __VA_ARGS__); }
#define PRINT_INT_AUTO(fmt, i) { if (i) { info_len += sprintf_s(info + info_len, _countof(info) - info_len, fmt, i); } else { info_len += sprintf_s(info + info_len, _countof(info) - info_len, (fmt[strlen(fmt)-1]=='\n') ? "Auto\n" : "Auto"); } }

	PRINT_INFO(    "Input Frame Format    %s\n", "nv12");
	PRINT_INFO(    "Output Video          H.264/AVC %s @ Level %s\n",
		list_avc_profile[get_cx_index(list_avc_profile, configCtrl->profileLevel.profile)].desc,
		list_avc_level[get_cx_index(list_avc_level, configCtrl->profileLevel.level)].desc);
	PRINT_INFO(    "                      %dx%d%s %d:%d %0.3ffps (%d/%dfps)%s%s\n",
		configCtrl->width, configCtrl->height, (is_interlaced(ext_prm->pic_struct)) ? "i" : "p",
		1, 1, //Aspect Ratio
		configCtrl->rateControl.encRateControlFrameRateNumerator / (double)configCtrl->rateControl.encRateControlFrameRateDenominator,
		configCtrl->rateControl.encRateControlFrameRateNumerator, configCtrl->rateControl.encRateControlFrameRateDenominator,
		(is_interlaced(ext_prm->pic_struct)) ? "" : ", ",
		(is_interlaced(ext_prm->pic_struct)) ? "" : list_interlaced[get_cx_index(list_interlaced, ext_prm->pic_struct)].desc);
	PRINT_INFO(    "Encode Mode           %s\n", list_vce_rc_method[get_cx_index(list_vce_rc_method, configCtrl->rateControl.encRateControlMethod)].desc);
	if (configCtrl->rateControl.encRateControlMethod == VCE_RC_CQP) {
		PRINT_INFO("CQP Value             I:%d  P:%d\n", configCtrl->rateControl.encQP_I, configCtrl->rateControl.encQP_P);
		//PRINT_INFO("CQP Value             I:%d  P:%d  B:%d\n", configCtrl->rateControl.encQP_I, configCtrl->rateControl.encQP_P, configCtrl->rateControl.encQP_B);
	} else if (configCtrl->rateControl.encRateControlMethod == VCE_RC_CBR
		    || configCtrl->rateControl.encRateControlMethod == VCE_RC_VBR) {
		PRINT_INFO("Bitrate               %d kbps\n", (int)(configCtrl->rateControl.encRateControlTargetBitRate / 1000 + 0.5));
		PRINT_INFO("Max Bitrate           ");
		PRINT_INT_AUTO("%d kbps\n", (int)(configCtrl->rateControl.encRateControlPeakBitRate / 1000.0 + 0.5));
	}
	//PRINT_INFO(    "VBV Bufsize           %d kbps\n", (int)(configCtrl->rateControl.encVBVBufferSize / 1000 + 0.5));
	PRINT_INFO(    "CABAC                 %s\n", (configCtrl->pictControl.cabacEnable) ? "on" : "off");
	PRINT_INFO(    "GOP Length            ");
	PRINT_INT_AUTO("%d frames\n", configCtrl->rateControl.encGOPSize);
	PRINT_INFO(    "Deblock Filter        %s", (configCtrl->pictControl.loopFilterDisable) ? "off" : "on");
	if (!configCtrl->pictControl.loopFilterDisable)
		PRINT_INFO(                         "  %d:%d", configCtrl->pictControl.encLFAlphaC0Offset, configCtrl->pictControl.encLFBetaOffset);
	PRINT_INFO("\n");
	//PRINT_INFO(    "Luma Search Window    %s\n", list_vce_lsm_vert[get_cx_index(list_vce_lsm_vert, configCtrl->meControl.lsmVert)].desc);
	PRINT_INFO(    "MV Search Range       %dx%d\n", configCtrl->meControl.encSearchRangeX, configCtrl->meControl.encSearchRangeY);
	//PRINT_INFO(    "FME AdvModeDecision   %s\n", (configCtrl->meControl.enableAMD) ? "on" : "off");
	PRINT_INFO(    "SubMode               %s\n", list_vce_rc_submode[get_cx_index(list_vce_rc_submode, configCtrl->meControl.encDisableSubMode)].desc);

	vce_print(stderr, VCE_LOG_INFO, info);
#undef PRINT_INFO
#undef PRINT_INT_AUTO
}

static bool check_if_vce_dll_available() {
	//check for OpenVideo.dll
	HMODULE hModule = LoadLibrary("OpenVideo.dll");
	if (hModule == NULL)
		return false;
	FreeLibrary(hModule);
	return true;
}

bool check_if_vce_available() {
	bool ret = true;
	if (!check_if_vce_dll_available())
		return false;

	//check for device
	OVDeviceHandle device_handle = { 0 };
	if (false == getDevice(&device_handle))
		return false;

	uint32 device_id = device_handle.deviceInfo[0].device_id;
	OPContextHandle ove_context = { 0 };
	if (false == encodeCreate(&ove_context, device_id, &device_handle))
		ret = false;
	else
		encodeDestroy(ove_context);

	if (device_handle.deviceInfo)
		delete [] device_handle.deviceInfo;

	return ret;
}

uint32 get_vce_features() {
	uint32 features = 0x00;
	
	if (!check_if_vce_dll_available())
		return features;

	OVDeviceHandle deviceHandle;
	if (getDevice(&deviceHandle))
		return features;

	uint32 deviceId = deviceHandle.deviceInfo[0].device_id;
	
	OVE_ENCODE_CAPS cap_info;
	OVE_ENCODE_CAPS_H264 cap_h264;
	cap_info.caps.encode_cap_full = &cap_h264;

	OPContextHandle oveContext;
	if (   encodeCreate(&oveContext, deviceId, &deviceHandle)
		&& getDeviceCap(oveContext, deviceId, &cap_info)) {

		features |= VCE_H264_AVAILABLE;
	
		const int count_profile_level = min(cap_h264.num_Profile_level, OVE_MAX_NUM_PROFILE_LEVELS_H264);
		for (int i = 0; i < count_profile_level; i++) {
			if (100 == cap_h264.supported_profile_level[i].profile) {
				features |= VCE_FEATURE_HIGH_PROFILE;
				break;
			}
		}
		if (OVE_ENCODE_TASK_PRIORITY_LEVEL2 <= cap_h264.supported_task_priority)
			features |= VCE_FEATURE_LOW_LATENCY;

		encodeDestroy(oveContext);
	}
	delete [] deviceHandle.deviceInfo;

	return features;
}

vce_handle_t *vce_init(const OvConfigCtrl *_configCtrl,
			 bool *_abort,
			 func_vce_mes _print_mes,
			 func_vce_read _read_frame,
			 func_vce_write _write_frame,
			 const char *file_read,
			 const char *file_write,
			 const VCEExtParam *_ext_prm,
			 int _total_frames) {

	int ret = 0;

	vce_handle_t *vce_hnd = (vce_handle_t *)calloc(1, sizeof(vce_handle_t));
	vce_hnd->abort = _abort;
	vce_print_mes = (_print_mes) ? _print_mes : vce_mes;
	vce_hnd->read_frame = (_read_frame) ? _read_frame : yuvToNV12;
	vce_hnd->write_frame = (_write_frame) ? _write_frame : fwrite;
	memcpy(&vce_hnd->ConfigCtrl, _configCtrl, sizeof(vce_hnd->ConfigCtrl));
	memcpy(&vce_hnd->ExtPrm, _ext_prm, sizeof(vce_hnd->ExtPrm));
	if (check_param(&vce_hnd->ConfigCtrl, &vce_hnd->ExtPrm)) {
		vce_print(stderr, VCE_LOG_ERROR, "invalid param!.\n");
		ret = 1;
	}

	//check dll
	if (!check_if_vce_dll_available()) {
		vce_print(stderr, VCE_LOG_ERROR, "OpenVideo.dll not installed.\n");
		vce_print(stderr, VCE_LOG_ERROR, "Please install the fullpackage of Catalyst Control Center 12.8 or later.\n");
		vce_print(stderr, VCE_LOG_ERROR, "If it is already installed, please try reinstall or repair of Catalyst Control Center,\n");
		vce_print(stderr, VCE_LOG_ERROR, "it might not be installed properly.\n");
		ret = 1;
	}

	//check vista or later
	OSVERSIONINFO vInfo = { 0 };
	vInfo.dwOSVersionInfoSize = sizeof(vInfo);
	if (!GetVersionEx(&vInfo)) {
		vce_print(stderr, VCE_LOG_ERROR, "Unable to get Windows version information.\n");
		ret = 1;
	} else if (vInfo.dwMajorVersion < 6) {
		vce_print(stderr, VCE_LOG_ERROR, "Unsupported OS! Windows Vista or later required.\n");
		ret = 1;
	}


	if (!ret) {
		//initialize profile counters
		//initProfileCnt(&vce_hnd->perfCounter);

		//Initializing Encoder...
		if (false == getDevice(&vce_hnd->deviceHandle)) {
			vce_print(stderr, VCE_LOG_ERROR, "failed to initialize encoder.\n");
			vce_print(stderr, VCE_LOG_ERROR, "Please make sure your system has a GPU or APU which supports VCE.\n");
			ret = 1;
		}
	}
	if (!ret) {
		// Check deviceHandle.numDevices for number of devices and choose the
		// device on which user wants to create the encoder
		// In this case device 0 is choosen 
		vce_hnd->deviceId = vce_hnd->deviceHandle.deviceInfo[0].device_id;

		//Create the encoder context on the device specified by deviceID
		if (false == encodeCreate(&vce_hnd->oveContext, vce_hnd->deviceId, &vce_hnd->deviceHandle)) {
			vce_print(stderr, VCE_LOG_ERROR, "failed to encodeCreate.\n");
			ret = 1;
		//initialize the encoder session with configuration specified by the pConfigCtrl
		} else if (false == encodeOpen(&vce_hnd->encodeHandle, vce_hnd->oveContext, vce_hnd->deviceId, &vce_hnd->ConfigCtrl)) {
			vce_print(stderr, VCE_LOG_ERROR, "failed to encodeOpen.\n");
			ret = 1;
		}

		if (file_read) {
			if (NULL == strcmp(file_read, "-")) {
				if (_setmode( _fileno(stdin), _O_BINARY ) == 1) {
					vce_print(stderr, VCE_LOG_ERROR, "failed to switch stdin to binary mode.\n");
					ret = 1;
				} else {
					vce_hnd->fp_read = stdin;
				}
			} else if (fopen_s(&vce_hnd->fp_read, file_read, "rb") || !vce_hnd->fp_read) {
				vce_print(stderr, VCE_LOG_ERROR, "failed to open input file.\n");
				ret = 1;
			}
		} else if (_read_frame == NULL) {
			vce_print(stderr, VCE_LOG_ERROR, "input file not set.\n");
			ret = 1;
		}
		if (file_write) {
			if (NULL == strcmp(file_write, "-")) {
				if (_setmode( _fileno(stdout), _O_BINARY ) == 1) {
					vce_print(stderr, VCE_LOG_ERROR, "failed to switch stdout to binary mode.\n");
					ret = 1;
				} else {
					vce_hnd->fp_read = stdout;
				}
			} else if (fopen_s(&vce_hnd->fp_write, file_write, "wb") || !vce_hnd->fp_write) {
				vce_print(stderr, VCE_LOG_ERROR, "failed to open output file.\n");
				ret = 1;
			}
		} else if (_write_frame == NULL) {
			vce_print(stderr, VCE_LOG_ERROR, "output file not set.\n");
			ret = 1;
		}
	}
	if (ret) {
		if (vce_hnd->fp_read) fclose(vce_hnd->fp_read);
		if (vce_hnd->fp_write) fclose(vce_hnd->fp_write);
		delete vce_hnd;
	} else {
		print_vce_prm(&vce_hnd->ConfigCtrl, &vce_hnd->ExtPrm);
		vce_info_init(&vce_hnd->info, vce_hnd->ConfigCtrl.rateControl.encRateControlFrameRateNumerator / (double)vce_hnd->ConfigCtrl.rateControl.encRateControlFrameRateDenominator, _total_frames);
	}
	return (ret) ? NULL : vce_hnd;
}

typedef struct {
	vce_surface_t input_surface;
	cl_event inMapEvt;
	cl_event unmapEvent;
	OVE_ENCODE_PARAMETERS_H264 pictureParameter;
	uint32 numEncodeTaskInputBuffers;
	OVE_INPUT_DESCRIPTION encodeTaskInputBufferList[1];
	OPEventHandle eventRunVideoProgram;
	uint32 iTaskID;
    uint32 numTaskDescriptionsRequested;
    uint32 numTaskDescriptionsReturned;
	OVE_OUTPUT_DESCRIPTION TaskDescriptionList[1];
} vce_pipeline_task_t;

const int INPUT_BUF_SIZE = 8;
const int PIPELINE_LENGTH = MAX_INPUT_SURFACE - INPUT_BUF_SIZE - 1;
const int STEP_1_DELAY = 0;
const int STEP_2_DELAY = 1;
const int STEP_3_DELAY = 2;
const int STEP_4_DELAY = PIPELINE_LENGTH - 1;

static inline vce_pipeline_task_t *get_task(vce_pipeline_task_t *pipeline, int frame_n) {
	return &pipeline[frame_n % PIPELINE_LENGTH];
}

static inline bool is_suitable_frame_num(int frame_n, int max_frames) {
	return (0 <= frame_n) & (frame_n <= max_frames);
}

static inline void init_task(vce_pipeline_task_t *task) {
	memset(task, 0, sizeof(task[0]));
	task->numEncodeTaskInputBuffers = 1;
	task->numTaskDescriptionsRequested = 1;
    task->pictureParameter.size = sizeof(OVE_ENCODE_PARAMETERS_H264);
	//task->pictureParameter.flags.value = 0;
	//task->pictureParameter.flags.flags.reserved = 0;
	//task->pictureParameter.insertSPS = (OVE_BOOL)(i == 0)?true:false;
	//task->pictureParameter.pictureStructure = vce_hnd->ExtPrm.pic_struct;
	//task->pictureParameter.forceRefreshMap = (OVE_BOOL)true;
    //task->pictureParameter.forceIMBPeriod = 0;
    //task->pictureParameter.forcePicType = OVE_PICTURE_TYPE_H264_NONE;
}

static __forceinline void get_qp_counter(int64 *qpc) {
#if CHECK_PERFORMANCE
	QueryPerformanceCounter((LARGE_INTEGER *)qpc);
#endif
}

static __forceinline void add_qpctime(int64 *qpc, int64 add) {
#if CHECK_PERFORMANCE
	*qpc += add;
#endif
}

static inline bool get_input_buf_next_surf(vce_handle_t *vce_hnd, vce_surface_t *frame_buffer) {
	vce_input_buf_t *p_input_buf = &vce_hnd->thread.input_buf[vce_hnd->thread.frame_get % vce_hnd->thread.buffer_size];

	//_ftprintf(stderr, "GetNextFrame: wait for %d\n", m_pEncThread->m_nFrameGet);
	//_ftprintf(stderr, "wait for heInputDone, %d\n", m_pEncThread->m_nFrameGet);
	WaitForSingleObject(p_input_buf->he_input_done, INFINITE);
	//エラー・中断要求などでの終了
	if (*vce_hnd->abort)
		return false;
	//読み込み完了による終了
	if (vce_hnd->fin && vce_hnd->thread.frame_get == vce_hnd->info.input_frames)
		return false;
	*frame_buffer = p_input_buf->frame_buf;
	ZeroMemory(&p_input_buf->frame_buf, sizeof(p_input_buf->frame_buf));
	vce_hnd->thread.frame_get++;
	return true;
}

static inline void set_input_buf_next_surf(vce_handle_t *vce_hnd, vce_surface_t frame_buffer) {
	vce_input_buf_t *p_input_buf = &vce_hnd->thread.input_buf[vce_hnd->thread.frame_set % vce_hnd->thread.buffer_size];
	//_ftprintf(stderr, "Set heInputStart: %d\n", m_pEncThread->m_nFrameSet);
	//_ftprintf(stderr, "set surface %d, set event heInputStart %d\n", pSurface, m_pEncThread->m_nFrameSet);
	p_input_buf->frame_buf = frame_buffer;
	SetEvent(p_input_buf->he_input_start);
	vce_hnd->thread.frame_set++;
}

unsigned int __stdcall vce_thread_func(void *_prm) {
	vce_handle_t *vce_hnd = (vce_handle_t *)_prm;
    uint32 numEventInWaitList = 0;
	
	ove_session session = vce_hnd->encodeHandle.session;

    OVresult res = true;

	//Make sure the surface is byte aligned
	const uint32 alignedSurfaceWidth = (vce_hnd->ConfigCtrl.width + 255) & ~255;
    const uint32 alignedSurfaceHeight = (vce_hnd->ConfigCtrl.height + 31) & ~31;

	//nv12 size
	const int32 hostPtrSize = alignedSurfaceHeight * alignedSurfaceWidth * 3/2;

	vce_hnd->info.output_frames = 0;
	int current_frame = 0;
	int max_frames = INT_MAX - PIPELINE_LENGTH;
	const int h264_picstruct = vce_hnd->ExtPrm.pic_struct - OVE_PICTURE_STRUCTURE_H264_FRAME;

	//init pipeline
	vce_pipeline_task_t pipeline[PIPELINE_LENGTH] = { 0 };
	for (int i = 0; i < _countof(pipeline); i++)
		init_task(&pipeline[i]);

	int64 qpc_temp[8] = { 0 };
	int64 qpc_counter[13] = { 0 };
#if CHECK_PERFORMANCE
	int64 qpc_freq = 0;
	int64 qpc_start = 0;
	int64 qpc_fin = 0;
	QueryPerformanceFrequency((LARGE_INTEGER *)&qpc_freq);
#endif

	bool input_fin = false;

	// set frame buffer for input
	for (int i = 0; i < vce_hnd->thread.buffer_size; i++) {
		vce_surface_t surf = { 0 };
		cl_int status;
		surf.surface = vce_hnd->encodeHandle.inputSurfaces[(PIPELINE_LENGTH + 1 + i) % MAX_INPUT_SURFACE];
		cl_event in_map_evt = NULL;
		surf.map_ptr = clEnqueueMapBuffer(vce_hnd->encodeHandle.clCmdQueue,
                                    (cl_mem)surf.surface,
                                    CL_FALSE, //CL_TRUE,
                                    CL_MAP_READ | CL_MAP_WRITE,
                                    0,
                                    hostPtrSize,
                                    0,
                                    NULL,
                                    &in_map_evt,
                                    &status);
		status = clFlush(vce_hnd->encodeHandle.clCmdQueue);
		waitForEvent(in_map_evt);
		status = clReleaseEvent(in_map_evt);
		set_input_buf_next_surf(vce_hnd, surf);
	}

	vce_info_enc_start(&vce_hnd->info);
	get_qp_counter(&qpc_start);
	//main loop
	while (!(*vce_hnd->abort) && current_frame < max_frames + PIPELINE_LENGTH) {
        cl_int status = CL_SUCCESS;
		// pipeline step 1
		{
			const int step1_frame = current_frame - STEP_1_DELAY;
			if (is_suitable_frame_num(step1_frame, max_frames)) {
				vce_pipeline_task_t *const task = get_task(pipeline, step1_frame);
				task->input_surface.surface = vce_hnd->encodeHandle.inputSurfaces[step1_frame%MAX_INPUT_SURFACE];
				get_qp_counter(&qpc_temp[0]);
				task->input_surface.map_ptr = clEnqueueMapBuffer( vce_hnd->encodeHandle.clCmdQueue,
                                            (cl_mem)task->input_surface.surface,
                                            CL_FALSE, //CL_TRUE,
                                            CL_MAP_READ | CL_MAP_WRITE,
                                            0,
                                            hostPtrSize,
                                            0,
                                            NULL,
                                            &task->inMapEvt,
                                            &status);
				get_qp_counter(&qpc_temp[1]);
				status = clFlush(vce_hnd->encodeHandle.clCmdQueue);
				get_qp_counter(&qpc_temp[2]);
				add_qpctime(&qpc_counter[0], qpc_temp[1] - qpc_temp[0]);
				add_qpctime(&qpc_counter[1], qpc_temp[2] - qpc_temp[1]);
			}
		}
		//pipeline step 2
		{
			const int step2_frame = current_frame - STEP_2_DELAY;
			if (is_suitable_frame_num(step2_frame, max_frames)) {
				vce_pipeline_task_t *const task = get_task(pipeline, step2_frame);
				get_qp_counter(&qpc_temp[0]);
				waitForEvent(task->inMapEvt);
				status = clReleaseEvent(task->inMapEvt);
				task->inMapEvt = NULL;
				//Read into the input surface buffer
				get_qp_counter(&qpc_temp[1]);
				vce_surface_t get_cached_surf = { 0 };
				input_fin = !get_input_buf_next_surf(vce_hnd, &get_cached_surf);
				if (input_fin)
					max_frames = step2_frame - 1;
				set_input_buf_next_surf(vce_hnd, task->input_surface);
				task->input_surface = get_cached_surf;
				get_qp_counter(&qpc_temp[2]);

				status = clEnqueueUnmapMemObject(vce_hnd->encodeHandle.clCmdQueue,
												(cl_mem)task->input_surface.surface,
												task->input_surface.map_ptr,
												0,
												NULL,
												&task->unmapEvent);
				get_qp_counter(&qpc_temp[3]);
				status = clFlush(vce_hnd->encodeHandle.clCmdQueue);
				get_qp_counter(&qpc_temp[4]);
				add_qpctime(&qpc_counter[2], qpc_temp[1] - qpc_temp[0]);
				add_qpctime(&qpc_counter[3], qpc_temp[2] - qpc_temp[1]);
				add_qpctime(&qpc_counter[4], qpc_temp[3] - qpc_temp[2]);
				add_qpctime(&qpc_counter[5], qpc_temp[4] - qpc_temp[3]);
			}
		}
		//pipeline step 3
		{
			const int step3_frame = current_frame - STEP_3_DELAY;
			if (is_suitable_frame_num(step3_frame, max_frames)) {
				vce_pipeline_task_t *const task = get_task(pipeline, step3_frame);
				get_qp_counter(&qpc_temp[0]);
				waitForEvent(task->unmapEvent);
				status = clReleaseEvent(task->unmapEvent);
				task->unmapEvent = NULL;
				task->input_surface.map_ptr = NULL;
				//use the input surface buffer as our Picture
				task->encodeTaskInputBufferList[0].bufferType = OVE_BUFFER_TYPE_PICTURE;
				task->encodeTaskInputBufferList[0].buffer.pPicture = (OVE_SURFACE_HANDLE)task->input_surface.surface;
				//Setup the picture parameters
				const OVE_PICTURE_STRUCTURE_H264 PIC_STRUCT[3][2] = {
					{ OVE_PICTURE_STRUCTURE_H264_FRAME,        OVE_PICTURE_STRUCTURE_H264_FRAME        },
					{ OVE_PICTURE_STRUCTURE_H264_TOP_FIELD,    OVE_PICTURE_STRUCTURE_H264_BOTTOM_FIELD },
					{ OVE_PICTURE_STRUCTURE_H264_BOTTOM_FIELD, OVE_PICTURE_STRUCTURE_H264_TOP_FIELD    },
				};
				task->pictureParameter.pictureStructure = PIC_STRUCT[h264_picstruct][step3_frame & 0x01];
				task->pictureParameter.insertSPS = (step3_frame == 0);
				//encode a single picture.
				//captureTimeStart(&vce_hnd->perfCounter, 0);
				get_qp_counter(&qpc_temp[1]);
				if (false == (res = OVEncodeTask(session,
									task->numEncodeTaskInputBuffers,
									task->encodeTaskInputBufferList,
									&task->pictureParameter,
									&task->iTaskID,
									numEventInWaitList,
									NULL,
									&task->eventRunVideoProgram))) {
					vce_print(stderr, VCE_LOG_ERROR, "\nOVEncodeTask returned error %fd.\n", res);
					break;
				}
				task->input_surface.surface = NULL;
				get_qp_counter(&qpc_temp[2]);
				add_qpctime(&qpc_counter[6], qpc_temp[1] - qpc_temp[0]);
				add_qpctime(&qpc_counter[7], qpc_temp[2] - qpc_temp[1]);
			}
		}
		//pipeline step 4
		{
			const int step4_frame = current_frame - STEP_4_DELAY;
			if (is_suitable_frame_num(step4_frame, max_frames)) {
				get_qp_counter(&qpc_temp[0]);
				vce_pipeline_task_t *const task = get_task(pipeline, step4_frame);
				if (CL_SUCCESS != clWaitForEvents(1, (cl_event *)&(task->eventRunVideoProgram))) {
					vce_print(stderr, VCE_LOG_ERROR, "\nclWaitForEvents returned error.\n");
					res = false;
					break;
				}
				//captureTimeStop(&vce_hnd->perfCounter,0);
				//Query output
				task->numTaskDescriptionsReturned = 0;
				memset(task->TaskDescriptionList, 0, sizeof(task->TaskDescriptionList));
				task->TaskDescriptionList[0].size = sizeof(OVE_OUTPUT_DESCRIPTION);
				//captureTimeStart(&vce_hnd->perfCounter,1);
				get_qp_counter(&qpc_temp[1]);
				do
				{
					if (false == (res = OVEncodeQueryTaskDescription(session,
														task->numTaskDescriptionsRequested,
														&task->numTaskDescriptionsReturned,
														task->TaskDescriptionList)))
														break;
				} while(task->TaskDescriptionList->status == OVE_TASK_STATUS_NONE);
				//captureTimeStop(&vce_hnd->perfCounter, 1);
				if (!res) {
					vce_print(stderr, VCE_LOG_ERROR, "\nOVEncodeQueryTaskDescription returned error.\n");
					break;
				}
				get_qp_counter(&qpc_temp[2]);
				//Write compressed frame to the output file
				int64 added_filesize = 0;
				for (uint32 i = 0; i < task->numTaskDescriptionsReturned; i++) {
					if ((task->TaskDescriptionList[i].status == OVE_TASK_STATUS_COMPLETE) 
						&& task->TaskDescriptionList[i].size_of_bitstream_data > 0) {
							DWORD written_data_size = vce_hnd->write_frame(task->TaskDescriptionList[i].bitstream_data,
																			1, task->TaskDescriptionList[i].size_of_bitstream_data,
																			vce_hnd->fp_write);
							if (written_data_size != task->TaskDescriptionList[i].size_of_bitstream_data)
								res = false;
							added_filesize += written_data_size;
							res = OVEncodeReleaseTask(session, task->TaskDescriptionList[i].taskID);
					}
				}				
				if (res == false) {
					vce_print(stderr, VCE_LOG_ERROR, "\nfailed to write frame.\n");
					break;
				}
				get_qp_counter(&qpc_temp[3]);
				if (task->eventRunVideoProgram)
					clReleaseEvent((cl_event)task->eventRunVideoProgram);
				get_qp_counter(&qpc_temp[4]);
				init_task(task); //reset task
				vce_hnd->info.output_frames++;
				vce_hnd->info.filesize += added_filesize;
				add_qpctime(&qpc_counter[ 8], qpc_temp[1] - qpc_temp[0]);
				add_qpctime(&qpc_counter[ 9], qpc_temp[2] - qpc_temp[1]);
				add_qpctime(&qpc_counter[10], qpc_temp[3] - qpc_temp[2]);
				add_qpctime(&qpc_counter[11], qpc_temp[4] - qpc_temp[3]);
			}
		}
		current_frame++;
	}
	get_qp_counter(&qpc_fin);

	//Release all resources.
	for (int i = 0; i < PIPELINE_LENGTH; i++) {
		if (pipeline[i].input_surface.map_ptr) {
			if (pipeline[i].unmapEvent == NULL) {
				if (pipeline[i].inMapEvt) {
					waitForEvent(pipeline[i].inMapEvt);
					clReleaseEvent(pipeline[i].inMapEvt);
					pipeline[i].inMapEvt = NULL;
				}
				clEnqueueUnmapMemObject(vce_hnd->encodeHandle.clCmdQueue,
					(cl_mem)pipeline[i].input_surface.surface,
					pipeline[i].input_surface.map_ptr,
					0,
					NULL,
					&pipeline[i].unmapEvent);
				clFlush(vce_hnd->encodeHandle.clCmdQueue);
			}
			if (pipeline[i].unmapEvent) {
				waitForEvent(pipeline[i].unmapEvent);
				clReleaseEvent(pipeline[i].unmapEvent);
				pipeline[i].unmapEvent = NULL;
				pipeline[i].input_surface.map_ptr = NULL;
			}
		}
	}

	if (!res) {
		vce_hnd->fin = TRUE;
		for (int i = 0; i < vce_hnd->thread.buffer_size; i++)
			SetEvent(vce_hnd->thread.input_buf[i].he_input_start);
	} else {
#if CHECK_PERFORMANCE
		if (vce_hnd->ExtPrm.show_performance_info) {
			vce_print(stderr, VCE_LOG_INFO, "Encode Thread (pipeline) performance...\n");
			vce_print(stderr, VCE_LOG_INFO, "total                      : %12.3f ms\n", (qpc_fin - qpc_start) * 1000.0 / (double)qpc_freq);
			vce_print(stderr, VCE_LOG_INFO, "step1...\n");
			vce_print(stderr, VCE_LOG_INFO, "  clEnqueMapBuffer         : %12.3f ms\n", qpc_counter[ 0] * 1000.0 / (double)qpc_freq);
			vce_print(stderr, VCE_LOG_INFO, "  clFlush(1)               : %12.3f ms\n", qpc_counter[ 1] * 1000.0 / (double)qpc_freq);
			vce_print(stderr, VCE_LOG_INFO, "step2...\n");
			vce_print(stderr, VCE_LOG_INFO, "  wait inMapEvt            : %12.3f ms\n", qpc_counter[ 2] * 1000.0 / (double)qpc_freq);
			vce_print(stderr, VCE_LOG_INFO, "  readFrame                : %12.3f ms\n", qpc_counter[ 3] * 1000.0 / (double)qpc_freq);
			vce_print(stderr, VCE_LOG_INFO, "  clEnqueUnMapMemObj       : %12.3f ms\n", qpc_counter[ 4] * 1000.0 / (double)qpc_freq);
			vce_print(stderr, VCE_LOG_INFO, "  clFlush(2)               : %12.3f ms\n", qpc_counter[ 5] * 1000.0 / (double)qpc_freq);
			vce_print(stderr, VCE_LOG_INFO, "step3...\n");
			vce_print(stderr, VCE_LOG_INFO, "  wait unMapEvt            : %12.3f ms\n", qpc_counter[ 6] * 1000.0 / (double)qpc_freq);
			vce_print(stderr, VCE_LOG_INFO, "  OVEncodeTask             : %12.3f ms\n", qpc_counter[ 7] * 1000.0 / (double)qpc_freq);
			vce_print(stderr, VCE_LOG_INFO, "step4...\n");
			vce_print(stderr, VCE_LOG_INFO, "  wait eventRunVideoProgram: %12.3f ms\n", qpc_counter[ 8] * 1000.0 / (double)qpc_freq);
			vce_print(stderr, VCE_LOG_INFO, "  OVEncodeQTDescription    : %12.3f ms\n", qpc_counter[ 9] * 1000.0 / (double)qpc_freq);
			vce_print(stderr, VCE_LOG_INFO, "  WriteFrame               : %12.3f ms\n", qpc_counter[10] * 1000.0 / (double)qpc_freq);
			vce_print(stderr, VCE_LOG_INFO, "  ReleaseEvent             : %12.3f ms\n", qpc_counter[11] * 1000.0 / (double)qpc_freq);
			vce_print(stderr, VCE_LOG_INFO, "\n");
		}
#endif
	}
	_endthreadex((int)res);
	return res;
}

int vce_init_thread(vce_handle_t *vce_hnd) {
	vce_thread_t *thread = &vce_hnd->thread;
	ZeroMemory(thread, sizeof(thread[0]));
	thread->buffer_size = INPUT_BUF_SIZE;
	if (NULL == (thread->input_buf = (vce_input_buf_t *)_aligned_malloc(sizeof(thread->input_buf[0]) * thread->buffer_size, 64)))
		return 1;
	for (int i = 0; i < thread->buffer_size; i++) {
		if (   NULL == (thread->input_buf[i].he_input_start = CreateEvent(NULL, FALSE, FALSE, NULL))
			|| NULL == (thread->input_buf[i].he_input_done  = CreateEvent(NULL, FALSE, FALSE, NULL)))
			return 1;
	}
	if (NULL == (thread->th_vce = (HANDLE)_beginthreadex(NULL, NULL, vce_thread_func, vce_hnd, FALSE, NULL)))
		return 1;
	return 0;
}

void vce_close_thread(vce_thread_t *thread) {
	if (thread->th_vce) {
		WaitForSingleObject(thread->th_vce, INFINITE);
		CloseHandle(thread->th_vce);
		thread->th_vce = NULL;
	}
	if (thread->input_buf) {
		for (int i = 0; i < thread->buffer_size; i++) {
			if (thread->input_buf[i].he_input_start) CloseHandle(thread->input_buf[i].he_input_start);
			if (thread->input_buf[i].he_input_done)  CloseHandle(thread->input_buf[i].he_input_done);
		}
		_aligned_free(thread->input_buf);
	}
	ZeroMemory(thread, sizeof(thread[0]));
}

int vce_run(vce_handle_t *vce_hnd) {
	if (!vce_hnd) {
		vce_print(stderr, VCE_LOG_INFO, "invalid handle.\n");
		return 0;
	}
	if (vce_init_thread(vce_hnd)) {
		vce_print(stderr, VCE_LOG_INFO, "failed to start encode thread.\n");
	} else {
		//入力ループ
		const int input_height = vce_hnd->ConfigCtrl.height;
		const int input_width = vce_hnd->ConfigCtrl.width;
		const int aligned_width = (vce_hnd->ConfigCtrl.width + 255) & ~255;
		const int buf_size = vce_hnd->thread.buffer_size;
		vce_input_buf_t *input_buf_array = vce_hnd->thread.input_buf;
		vce_hnd->fin = false;

		int64 qpc_freq = 0;
		int64 qpc_start = 0;
		int64 qpc_fin = 0;
		int64 qpc_active = 0;
		int64 qpc_temp[2] = { 0 };
#if CHECK_PERFORMANCE
		QueryPerformanceFrequency((LARGE_INTEGER *)&qpc_freq);
		get_qp_counter(&qpc_start);
#endif
		
		for (vce_hnd->info.input_frames = 0; vce_hnd->fin == false; vce_hnd->info.input_frames += (vce_hnd->fin == false)) {
			vce_input_buf_t *p_input_buf = &input_buf_array[vce_hnd->info.input_frames % buf_size];
			//_ftprintf(stderr, "run loop: wait for %d\n", vce_hnd->info.input_frames);
			//_ftprintf(stderr, "wait for heInputStart %d\n", vce_hnd->info.input_frames);
			WaitForSingleObject(p_input_buf->he_input_start, INFINITE);
			//_ftprintf(stderr, "load next frame %d to %d\n", vce_hnd->info.input_frames, pInputBuf->pFrameSurface);
			get_qp_counter(&qpc_temp[0]);
			vce_hnd->fin = !vce_hnd->read_frame(vce_hnd->fp_read, input_height, input_width, aligned_width, (int8*)p_input_buf->frame_buf.map_ptr);
			//_ftprintf(stderr, "set for heInputDone %d\n", vce_hnd->info.input_frames);
			SetEvent(p_input_buf->he_input_done);
			vce_info_print_status(&vce_hnd->info);
			get_qp_counter(&qpc_temp[1]);
			add_qpctime(&qpc_active,  qpc_temp[1] - qpc_temp[0]);
		}
		get_qp_counter(&qpc_fin);
		//直ちに終了する
		//if (*vce_hnd->abort)
		//	for (int i = 0; i < buf_size; i++)
		//		SetEvent(input_buf_array[i].he_input_done);
		//RunEncodeの終了を待つ
		WaitForSingleObject(vce_hnd->thread.th_vce, INFINITE);
		vce_print(stderr, VCE_LOG_INFO, "\nEncoding %s.\n", (*vce_hnd->abort) ? "aborted" : "finished");
		vce_info_enc_fin(&vce_hnd->info);

		CloseHandle(vce_hnd->thread.th_vce);
		vce_hnd->thread.th_vce = NULL;
#if CHECK_PERFORMANCE
		if (vce_hnd->ExtPrm.show_performance_info) {
			char cpu_name[256] = { 0 };
			uint32 gpu_freq = 0;
			DWORD cpu_logical_core = 0, cpu_physical_core = 0;
			get_gpu_frequency((cl_device_id)vce_hnd->deviceId, &gpu_freq);
			getCPUName(cpu_name, _countof(cpu_name));
			getProcessorCount(&cpu_physical_core, &cpu_logical_core);
			vce_print(stderr, VCE_LOG_INFO, "CPU Info                   : %s (%dC/%dT)\n", cpu_name, cpu_physical_core, cpu_logical_core);
			vce_print(stderr, VCE_LOG_INFO, "GPU Speed                  : %d MHz\n", gpu_freq);
			vce_print(stderr, VCE_LOG_INFO, "\n");
			vce_print(stderr, VCE_LOG_INFO, "Main Thread Active Time    : %.2lf percent\n", qpc_active * 100.0 / (double)(qpc_fin - qpc_start));
			vce_print(stderr, VCE_LOG_INFO, "\n");
			vce_print(stderr, VCE_LOG_INFO, "total buffer size          : %2d frames\n", MAX_INPUT_SURFACE);
			vce_print(stderr, VCE_LOG_INFO, "  input thread buffer size : %2d frames\n", INPUT_BUF_SIZE);
			vce_print(stderr, VCE_LOG_INFO, "  pieline buffer size      : %2d frames\n", MAX_INPUT_SURFACE - INPUT_BUF_SIZE);
			vce_print(stderr, VCE_LOG_INFO, "\n");
			vce_print(stderr, VCE_LOG_INFO, "pipeline length            : %d\n", PIPELINE_LENGTH);
			vce_print(stderr, VCE_LOG_INFO, "  pipeline step1 delay     : %d\n", STEP_1_DELAY);
			vce_print(stderr, VCE_LOG_INFO, "  pipeline step2 delay     : %d\n", STEP_2_DELAY);
			vce_print(stderr, VCE_LOG_INFO, "  pipeline step3 delay     : %d\n", STEP_3_DELAY);
			vce_print(stderr, VCE_LOG_INFO, "  pipeline step4 delay     : %d\n", STEP_4_DELAY);
			vce_print(stderr, VCE_LOG_INFO, "\n");
		}
#endif
	}
	vce_close_thread(&vce_hnd->thread);
	return 1;
}
