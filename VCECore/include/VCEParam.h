#ifndef __VCE_PARAM_H__
#define __VCE_PARAM_H__

enum {
	VCE_RC_CQP = 0,
	VCE_RC_CBR = 3,
	VCE_RC_VBR = 4,
};

typedef struct {
	char *desc;
	int value;
} CX_DESC;

const CX_DESC list_avc_profile[] = {
	{ "Baseline", 66 },
	{ "Main",     77 },
	//{ "High",     100 },
	{ NULL, NULL }
};

const CX_DESC list_avc_level[] = { 
	{ "auto", 0   },
	{ "1",    10  },
	{ "1b",   9   },
	{ "1.1",  11  },
	{ "1.2",  12  },
	{ "1.3",  13  },
	{ "2",    20  },
	{ "2.1",  21  },
	{ "2.2",  22  },
	{ "3",    30  },
	{ "3.1",  31  },
	{ "3.2",  32  },
	{ "4",    40  },
	{ "4.1",  41  },
	{ "4.2",  42  },
	{ "5",    50  },
	{ "5.1",  51  },
	{ NULL, NULL }
};

const uint16 COLOR_VALUE_AUTO = USHRT_MAX;
const int HD_HEIGHT_THRESHOLD = 720;
const int HD_INDEX = 2;
const int SD_INDEX = 3;
const CX_DESC list_colorprim[] = {
	{ "undef",     2  },
	{ "auto",      COLOR_VALUE_AUTO },
	{ "bt709",     1  },
	{ "smpte170m", 6  },
	{ "bt470m",    4  },
	{ "bt470bg",   5  },
	{ "smpte240m", 7  },
	{ "film",      8  },
	{ NULL, NULL }
};
const CX_DESC list_transfer[] = {
	{ "undef",     2  },
	{ "auto",      COLOR_VALUE_AUTO },
	{ "bt709",     1  },
	{ "smpte170m", 6  },
	{ "bt470m",    4  },
	{ "bt470bg",   5  },
	{ "smpte240m", 7  },
	{ "linear",    8  },
	{ "log100",    9  },
	{ "log316",    10 },
	{ NULL, NULL }
};
const CX_DESC list_colormatrix[] = {
	{ "undef",     2  },
	{ "auto",      COLOR_VALUE_AUTO },
	{ "bt709",     1  },
	{ "smpte170m", 6  },
	{ "bt470bg",   5  },
	{ "smpte240m", 7  },
	{ "YCgCo",     8  },
	{ "fcc",       4  },
	{ "GBR",       0  },
	{ NULL, NULL }
};
const CX_DESC list_videoformat[] = {
	{ "undef",     5  },
	{ "ntsc",      2  },
	{ "component", 0  },
	{ "pal",       1  },
	{ "secam",     3  },
	{ "mac",       4  },
	{ NULL, NULL } 
};

const CX_DESC list_mv_presicion[] = {
	{ "full-pel", 0 },
	{ "half-pel", 1 },
	{ "Q-pel",    2 },
	{ NULL, NULL }
};

const CX_DESC list_vce_mode[] = {
	{ "none", 0 },
	{ "full", 1 },
	{ "entropy", 2 },
	{ NULL, NULL }
};

const CX_DESC list_vce_rc_method[] = {
	{ "CQP", VCE_RC_CQP },
	{ "CBR", VCE_RC_CBR },
	{ "VBR", VCE_RC_VBR },
	{ NULL, NULL }
};

const CX_DESC list_vce_rc_submode[] = {
	{ "Level 0",   0 },
	{ "Level 1", 120 },
	{ "Level 2", 254 },
	{ NULL, NULL }
};

const CX_DESC list_vce_lsm_vert[] = {
	{ " 5x3", 0 },
	{ " 9x5", 1 },
	{ "13x7", 2 },
	{ NULL, NULL }
};

const CX_DESC list_interlaced[] = {
	{ "Progressive", OVE_PICTURE_STRUCTURE_H264_FRAME },
	//{ "tff",         OVE_PICTURE_STRUCTURE_H264_TOP_FIELD },
	//{ "bff",         OVE_PICTURE_STRUCTURE_H264_BOTTOM_FIELD },
	{ NULL, NULL }
};

static int get_cx_index(const CX_DESC * list, int v) {
	for (int i = 0; list[i].desc; i++)
		if (list[i].value == v)
			return i;
	return 0;
}

static int PARSE_ERROR_FLAG = INT_MIN;
static int get_value_from_chr(const CX_DESC *list, const char *chr) {
	for (int i = 0; list[i].desc; i++)
		if (_stricmp(list[i].desc, chr) == 0)
			return list[i].value;
	return PARSE_ERROR_FLAG;
}

#endif //__VCE_PARAM_H__
