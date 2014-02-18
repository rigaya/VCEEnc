#include <string>
#include <sstream>
#include "OvEncodeSample.h"
#include "VCECore.h"

#pragma comment(lib, "OpenVideo.lib")
#pragma comment(lib, "OpenCL.lib")

void show_vce_features() {
	uint32 featuresAvailable = get_vce_features();

	const std::map<uint32, std::string> feature_name = { 
		{ VCE_H264_AVAILABLE,       "VCE Available" },
		{ VCE_FEATURE_HIGH_PROFILE, "High Profile"  },
		{ VCE_FEATURE_LOW_LATENCY,  "Low Latency"   }  };

	std::stringstream ss;

	for (auto feature : feature_name) {
		ss << feature.second << ": " << ((featuresAvailable & feature.first) ? "yes" : "no") << std::endl;
	}

	std::cout << ss.str() << std::endl;
}


int main(int argc, char **argv) {

	show_vce_features();

	return 0;
}
