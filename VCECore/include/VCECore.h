#ifndef __VCE_CORE_H__
#define __VCE_CORE_H__

#include "OvEncodeTest.h"
#include "OvEncodePerf.h"

#define CHECK_PERFORMANCE 1

enum {
	VCE_LOG_INFO = 0,
	VCE_LOG_WARNING,
	VCE_LOG_ERROR
};

enum {
	VCE_H264_AVAILABLE       = 0x00000001,
	VCE_FEATURE_HIGH_PROFILE = 0x00000002,
	VCE_FEATURE_LOW_LATENCY  = 0x00000004,
};

uint32 get_vce_features();

typedef int (*func_vce_mes)(FILE *fp, int log_level, double progress, const char *fmt, ... );
typedef bool (*func_vce_read)(FILE *fr, uint32 uiHeight, uint32 uiWidth, 
               uint32 alignedSurfaceWidth, int8 *pBitstreamData);
typedef size_t (*func_vce_write)(const void *data, size_t nSize, size_t count, FILE * fp);

extern func_vce_mes vce_print_mes;

typedef struct {
	OVE_PICTURE_STRUCTURE_H264 pic_struct;
	BOOL show_performance_info;
} VCEExtParam;

typedef struct {
	DWORD tm_start;
	DWORD tm_last_update;
	double fps;
	int input_frames;
	int output_frames;
	int total_frames;
	uint64 filesize;
} vce_info_t;

typedef struct {
	void *surface;
	void *map_ptr;
} vce_surface_t;

typedef struct {
	vce_surface_t frame_buf;
	HANDLE he_input_start;
	HANDLE he_input_done;
	BYTE reserved[64-(sizeof(vce_surface_t)+sizeof(HANDLE)*2)];
} vce_input_buf_t;

typedef struct {
	HANDLE th_vce;
	vce_input_buf_t *input_buf;
	int32 frame_set;
	int32 frame_get;
	int32 buffer_size;
} vce_thread_t;

typedef struct {
	bool status;
	bool fin;
	func_vce_read read_frame;
	func_vce_write write_frame;
	//device id
	uint32 deviceId;
	//profile counters
	//OVprofile perfCounter;
	OVDeviceHandle deviceHandle;
	OPContextHandle oveContext;
	//encoder handle
	OVEncodeHandle encodeHandle;
	OvConfigCtrl ConfigCtrl;
	//File In/Out
	FILE *fp_read;
	FILE *fp_write;
	//pointer to abort flag
	bool *abort;
	//Extended Param;
	VCEExtParam ExtPrm;
	//Info
	vce_info_t info;
	//Threading
	vce_thread_t thread;
} vce_handle_t;

static inline bool is_interlaced(OVE_PICTURE_STRUCTURE_H264 pic_struct) {
	return (pic_struct == OVE_PICTURE_STRUCTURE_H264_TOP_FIELD || pic_struct == OVE_PICTURE_STRUCTURE_H264_BOTTOM_FIELD);
}

//initialzie parameter set.
void init_vce_prm(OvConfigCtrl *cnf);

//functions for running vce.
vce_handle_t *vce_init(const OvConfigCtrl *_configCtrl,
			 bool *abort,
			 func_vce_mes _print_mes,
			 func_vce_read _read_frame,
			 func_vce_write _write_frame,
			 const char *file_read,
			 const char *file_write,
			 const VCEExtParam *_ext_prm,
			 int _total_frames);

void vce_close(vce_handle_t *vce_hnd);

int vce_run(vce_handle_t *vce_hnd);

bool check_if_vce_available();
uint32 get_vce_features();

#endif //__VCE_CORE_H__
