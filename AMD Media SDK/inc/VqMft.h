#pragma once
#include <INITGUID.H>
#include <OBJBASE.H>

//  Metro
// PropertySet^ propertyset = ref new PropertySet;
// propertyset[AMF_EFFECT_STEADY_VIDEO]=true;
// propertyset[AMF_EFFECT_STEADY_VIDEO_DELAY]=2;
// AddVideoEffect("AMD.VQ.Effect",false, propertyset);

// Desktop
// all defined strings will be replaced with GUIDs
// IMFAttributes *attributes;
// pMyMFT->GetAttributes(&attributes);
// attributes->SetUINT32(AMF_EFFECT_STEADY_VIDEO,TRUE);
// attributes->SetUINT32(AMF_EFFECT_STEADY_VIDEO_DELAY,2);

#if defined(WINSTORE_APP)

#define AMF_EFFECT_STEADY_VIDEO                     L"AMF VQ Steady Video"                      // BOOL (default=FALSE)  enables=TRUE / disables=FALSE steady video effect
#define AMF_EFFECT_STEADY_VIDEO_STRENGTH            L"AMF VQ Steady Video Strength"             // INT (default=3) min=0 max=3 steady video effect strength
#define AMF_EFFECT_STEADY_VIDEO_DELAY               L"AMF VQ Steady Video Delay"                // INT (default=1) min=0 max=6 steady video effect delay
#define AMF_EFFECT_STEADY_VIDEO_ZOOM                L"AMF VQ Steady Video Zoom"                 // INT (default=100) min=90 max=100 steady video effect delay

#define AMF_EFFECT_DEBLOCKING                       L"AMF VQ De-blocking"                       // BOOL (default=FALSE)  enables=TRUE / disables=FALSE de-blocking effect
#define AMF_EFFECT_DEBLOCKING_STRENGTH              L"AMF VQ De-blocking Strength"              // INT (default=50) min=0 max=100 de-blocking effect strength

#define AMF_EFFECT_MOSQUITO_NOISE                   L"AMF VQ Mosquito Noise"                    // BOOL (default=FALSE)  enables=TRUE / disables=FALSE Mosquito Noise removal effect
#define AMF_EFFECT_MOSQUITO_NOISE_STRENGTH          L"AMF VQ Mosquito Noise Strength"           // INT (default=68) min=0 max=100 Mosquito Noise removal strength

#define AMF_EFFECT_DENOISE                          L"AMF VQ De-noise"                          // BOOL (default=FALSE)  enables=TRUE / disables=FALSE De-noise effect
#define AMF_EFFECT_DENOISE_STRENGTH                 L"AMF VQ De-noise Strength"                 // INT (default=50) min=1 max=100 De-noise strength

#define AMF_EFFECT_EDGE_ENHANCEMENT                 L"AMF VQ Edge enhancement"                  // BOOL (default=FALSE)  enables=TRUE / disables=FALSE Edge enhancement effect
#define AMF_EFFECT_EDGE_ENHANCEMENT_STRENGTH        L"AMF VQ Edge enhancement Strength"         // INT (default=50) min=1 max=100 Edge enhancement strength

#define AMF_EFFECT_DYNAMIC_CONTRAST                 L"AMF VQ Dynamic contrast"                  // BOOL (default=FALSE)  enables=TRUE / disables=FALSE Dynamic contrast

#define AMF_EFFECT_DEINTERLACING                    L"AMF VQ De-interlacing"                    // INT (default=AMF_EFFECT_DE_INTERLACING_AUTOMATIC)  values - see below De-interlacing effect type
#define AMF_EFFECT_DEINTERLACING_AUTOMATIC          0  // Value for AMF_EFFECT_DE_INTERLACING
#define AMF_EFFECT_DEINTERLACING_WEAVE              1  // Value for AMF_EFFECT_DE_INTERLACING
#define AMF_EFFECT_DEINTERLACING_BOBE               2  // Value for AMF_EFFECT_DE_INTERLACING
#define AMF_EFFECT_DEINTERLACING_ADAPTIVE           3  // Value for AMF_EFFECT_DE_INTERLACING
#define AMF_EFFECT_DEINTERLACING_MOTION_ADAPTIVE    4  // Value for AMF_EFFECT_DE_INTERLACING
#define AMF_EFFECT_DEINTERLACING_VECTOR_ADAPTIVE    5  // Value for AMF_EFFECT_DE_INTERLACING
#define AMF_EFFECT_DEINTERLACING_PULLDOWN_DETECTION L"AMF VQ De-interlacing Pulldown detection" // BOOL (default=TRUE)  enables=TRUE / disables=FALSE De-interlacing Pulldown detection

#define AMF_EFFECT_COLOR_VIBRANCE                   L"AMF VQ Color vibrance"                    // BOOL (default=FALSE)  enables=TRUE / disables=FALSE Color vibrance effect
#define AMF_EFFECT_COLOR_VIBRANCE_STRENGTH          L"AMF VQ Color vibrance Strength"           // INT (default=50) min=0 max=100 Color vibrance strength

#define AMF_EFFECT_SKINTONE_CORRECTION              L"AMF VQ Skintone correction"               // BOOL (default=FALSE)  enables=TRUE / disables=FALSE Skintone correction effect
#define AMF_EFFECT_SKINTONE_CORRECTION_STRENGTH     L"AMF VQ Skintone correction Strength"      // INT (default=50) min=0 max=100 Skintone correction strength


#define AMF_EFFECT_GAMMA_CORRECTION                 L"AMF VQ Gamma correction"                  // BOOL (default=FALSE)  enables=TRUE / disables=FALSE Gamma corection effect
#define AMF_EFFECT_GAMMA_CORRECTION_STRENGTH        L"AMF VQ Gamma correction Strength"         // DOUBLE (default=1.0) min=0.5 max=2.5 Gamma corection strength

#define AMF_EFFECT_BRIGHTER_WHITES                  L"AMF VQ Brighter Whites"                   // BOOL (default=FALSE)  enables=TRUE / disables=FALSE Brighter Whites effect

#define AMF_EFFECT_DYNAMIC_RANGE                    L"AMF VQ Dynamic range"                     // INT (default=AMF_EFFECT_DYNAMIC_RANGE_NONE)  values - see below Dynamic range effect type
#define AMF_EFFECT_DYNAMIC_RANGE_NONE               -1  // Value for AMF_EFFECT_DYNAMIC_RANGE
#define AMF_EFFECT_DYNAMIC_RANGE_FULL               0  // Value for AMF_EFFECT_DYNAMIC_RANGE (0-255)
#define AMF_EFFECT_DYNAMIC_RANGE_LIMITED            1  // Value for AMF_EFFECT_DYNAMIC_RANGE (16-235)

#define AMF_EFFECT_BRIGHTNESS                       L"AMF VQ Brightness"                        // DOUBLE (default=0) min=-100 max=100 Brightness effect
#define AMF_EFFECT_CONTRAST                         L"AMF VQ Contrast"                          // DOUBLE (default=1) min=0 max=2 Contrast effect
#define AMF_EFFECT_SATURATION                       L"AMF VQ Saturation"                        // DOUBLE (default=1) min=0 max=2 Saturation effect
#define AMF_EFFECT_TINT                             L"AMF VQ Tint"                              // DOUBLE (default=0.0) min=-30.0 max=30.0 Tint effect

#define AMF_EFFECT_FALSE_CONTOUR_REDUCTION          L"AMF VQ False contour reduction"           // BOOL (default=FALSE)  enables=TRUE / disables=FALSE False contour reduction effect
#define AMF_EFFECT_FALSE_CONTOUR_REDUCTION_STRENGTH L"AMF VQ False contour reduction Strength"  // INT (default=50) min=0 max=100 Skintone correction strength

#define AMF_EFFECT_SCALE                            L"AMF VQ Scale"                             // INT (default=AMF_EFFECT_SCALE_BILINEAR)  values - see below Scale effect type
#define AMF_EFFECT_SCALE_BILINEAR                   0  // Value for AMF_EFFECT_SCALE
#define AMF_EFFECT_SCALE_BICUBIC                    1  // Value for AMF_EFFECT_SCALE
#define AMF_EFFECT_SCALE_WIDTH                      L"AMF VQ Scale Width"                       // INT (default=no scaling)  width in pixels
#define AMF_EFFECT_SCALE_HEIGHT                     L"AMF VQ Scale Height"                      // INT (default=no scaling)  Height in pixels

#define AMF_EFFECT_DEMOMODE                         L"AMF VQ Demo mode"                         // BOOL () (default=FALSE)  enables=TRUE / disables=FALSE

#define AMF_EFFECT_DYNAMIC                          L"AMF VQ Dynamic"                           // BOOL (default=FALSE)  enables=TRUE / disables=FALSE this property set is dynamic and MFT should listen MapChanged event on PropertySetObject

#define AMF_EFFECT_OUTPUT_FORMAT                    L"AMF VQ Output Format"						// INT (default=AMF_EFFECT_OUTPUT_FORMAT_ALL)  forces MFT to use this format on negotiations
#define AMF_EFFECT_OUTPUT_FORMAT_ALL				0
#define AMF_EFFECT_OUTPUT_FORMAT_BGRA				1
#define AMF_EFFECT_OUTPUT_FORMAT_NV12				2
	
#define AMF_PROCESSING_TRANSFORM_AVR_TIME           L"AMF VQ Process average Time"              // average processing sample time in ms
#define AMF_PROCESSING_TRANSFORM_MIN_TIME           L"AMF VQ Process min Time"                  // min processing sample time in ms
#define AMF_PROCESSING_TRANSFORM_MAX_TIME           L"AMF VQ Process max Time"                  // max processing sample time in ms

#else // WINSTORE_APP

// AMF VQ Steady Video
// {43FE12E7-25AC-44E3-9826-8815223A110B}
DEFINE_GUID(AMF_EFFECT_STEADY_VIDEO, 0x43fe12e7, 0x25ac, 0x44e3, 0x98, 0x26, 0x88, 0x15, 0x22, 0x3a, 0x11, 0xb); // BOOL (default=FALSE)  enables=TRUE / disables=FALSE steady video effect
// {A0ACA7A3-B3F3-41D7-A5E5-54CEA88E0653}
DEFINE_GUID(AMF_EFFECT_STEADY_VIDEO_STRENGTH, 0xa0aca7a3, 0xb3f3, 0x41d7, 0xa5, 0xe5, 0x54, 0xce, 0xa8, 0x8e, 0x6, 0x53); // INT (default=3) min=0 max=3 steady video effect strength
// {37D84B90-60C0-474B-8E38-1F2675D94DD8}
DEFINE_GUID(AMF_EFFECT_STEADY_VIDEO_DELAY, 0x37d84b90, 0x60c0, 0x474b, 0x8e, 0x38, 0x1f, 0x26, 0x75, 0xd9, 0x4d, 0xd8); // INT (default=1) min=0 max=6 steady video effect delay
// {886842BC-C6EF-4D52-BF62-4DED4B52A610}
DEFINE_GUID(AMF_EFFECT_STEADY_VIDEO_ZOOM, 0x886842bc, 0xc6ef, 0x4d52, 0xbf, 0x62, 0x4d, 0xed, 0x4b, 0x52, 0xa6, 0x10); // INT (default=100) min=90 max=100 steady video effect delay

// AMF VQ De-blocking
// {9F83A115-5A06-4C15-8C2B-840BBB4363E5}
DEFINE_GUID(AMF_EFFECT_DEBLOCKING, 0x9f83a115, 0x5a06, 0x4c15, 0x8c, 0x2b, 0x84, 0xb, 0xbb, 0x43, 0x63, 0xe5); // BOOL (default=FALSE)  enables=TRUE / disables=FALSE de-blocking effect
// {BF94B103-FDA4-4567-B50E-AE85B5CB18CC}
DEFINE_GUID(AMF_EFFECT_DEBLOCKING_STRENGTH, 0xbf94b103, 0xfda4, 0x4567, 0xb5, 0xe, 0xae, 0x85, 0xb5, 0xcb, 0x18, 0xcc); // INT (default=50) min=0 max=100 de-blocking effect strength

// AMF VQ Mosquito Noise
// {EA9BC460-7503-4A35-AF58-994E6ACD8916}
DEFINE_GUID(AMF_EFFECT_MOSQUITO_NOISE, 0xea9bc460, 0x7503, 0x4a35, 0xaf, 0x58, 0x99, 0x4e, 0x6a, 0xcd, 0x89, 0x16); // BOOL (default=FALSE)  enables=TRUE / disables=FALSE Mosquito Noise removal effect
// {A88400C4-F96A-4076-8D52-F6E74304329A}
DEFINE_GUID(AMF_EFFECT_MOSQUITO_NOISE_STRENGTH, 0xa88400c4, 0xf96a, 0x4076, 0x8d, 0x52, 0xf6, 0xe7, 0x43, 0x4, 0x32, 0x9a); // INT (default=68) min=0 max=100 Mosquito Noise removal strength

// AMF VQ De-noise
// {F5463DCD-654D-4945-B17F-142EF1A53BF1}
DEFINE_GUID(AMF_EFFECT_DENOISE, 0xf5463dcd, 0x654d, 0x4945, 0xb1, 0x7f, 0x14, 0x2e, 0xf1, 0xa5, 0x3b, 0xf1); // BOOL (default=FALSE)  enables=TRUE / disables=FALSE De-noise effect
// {384FEA14-524C-4CE9-B451-6F461B7C2AD2}
DEFINE_GUID(AMF_EFFECT_DENOISE_STRENGTH, 0x384fea14, 0x524c, 0x4ce9, 0xb4, 0x51, 0x6f, 0x46, 0x1b, 0x7c, 0x2a, 0xd2); // INT (default=50) min=1 max=100 De-noise strength

// AMF VQ Edge enhancement
// {86D54E83-660E-4BE7-B343-B7A9BF504B84}
DEFINE_GUID(AMF_EFFECT_EDGE_ENHANCEMENT, 0x86d54e83, 0x660e, 0x4be7, 0xb3, 0x43, 0xb7, 0xa9, 0xbf, 0x50, 0x4b, 0x84); // BOOL (default=FALSE)  enables=TRUE / disables=FALSE Edge enhancement effect
// {41EAC443-ED34-4DCC-AE99-364BCDFBB375}
DEFINE_GUID(AMF_EFFECT_EDGE_ENHANCEMENT_STRENGTH, 0x41eac443, 0xed34, 0x4dcc, 0xae, 0x99, 0x36, 0x4b, 0xcd, 0xfb, 0xb3, 0x75); // INT (default=50) min=1 max=100 Edge enhancement strength


// AMF VQ Dynamic contrast
// {32EBC01D-A451-44B7-95D5-461F9411A1AD}
DEFINE_GUID(AMF_EFFECT_DYNAMIC_CONTRAST, 0x32ebc01d, 0xa451, 0x44b7, 0x95, 0xd5, 0x46, 0x1f, 0x94, 0x11, 0xa1, 0xad); // BOOL (default=FALSE)  enables=TRUE / disables=FALSE Dynamic contrast


// AMF VQ De-interlacing
// {AE8F3806-82A0-425D-9439-5BCA9540680D}
DEFINE_GUID(AMF_EFFECT_DEINTERLACING, 0xae8f3806, 0x82a0, 0x425d, 0x94, 0x39, 0x5b, 0xca, 0x95, 0x40, 0x68, 0xd); // INT (default=AMF_EFFECT_DE_INTERLACING_AUTOMATIC)  values - see below De-interlacing effect type
#define AMF_EFFECT_DEINTERLACING_AUTOMATIC          0  // Value for AMF_EFFECT_DE_INTERLACING
#define AMF_EFFECT_DEINTERLACING_WEAVE              1  // Value for AMF_EFFECT_DE_INTERLACING
#define AMF_EFFECT_DEINTERLACING_BOBE               2  // Value for AMF_EFFECT_DE_INTERLACING
#define AMF_EFFECT_DEINTERLACING_ADAPTIVE           3  // Value for AMF_EFFECT_DE_INTERLACING
#define AMF_EFFECT_DEINTERLACING_MOTION_ADAPTIVE    4  // Value for AMF_EFFECT_DE_INTERLACING
#define AMF_EFFECT_DEINTERLACING_VECTOR_ADAPTIVE    5  // Value for AMF_EFFECT_DE_INTERLACING
// {402C186F-3A45-474B-BFFD-1205C1E9CBC7}
DEFINE_GUID(AMF_EFFECT_DEINTERLACING_PULLDOWN_DETECTION, 0x402c186f, 0x3a45, 0x474b, 0xbf, 0xfd, 0x12, 0x5, 0xc1, 0xe9, 0xcb, 0xc7); // BOOL (default=TRUE)  enables=TRUE / disables=FALSE De-interlacing Pulldown detection


// AMF VQ Color vibrance
// {86038888-04CC-45AF-9A0C-2684E666A09E}
DEFINE_GUID(AMF_EFFECT_COLOR_VIBRANCE, 0x86038888, 0x4cc, 0x45af, 0x9a, 0xc, 0x26, 0x84, 0xe6, 0x66, 0xa0, 0x9e); // BOOL (default=FALSE)  enables=TRUE / disables=FALSE Color vibrance effect
// {6C7E8DC4-6ED6-4D0E-BB2F-79F0205D0612}
DEFINE_GUID(AMF_EFFECT_COLOR_VIBRANCE_STRENGTH, 0x6c7e8dc4, 0x6ed6, 0x4d0e, 0xbb, 0x2f, 0x79, 0xf0, 0x20, 0x5d, 0x6, 0x12); // INT (default=50) min=0 max=100 Color vibrance strength


// AMF VQ Skintone correction
// {885DCCEC-77A5-4CA4-B5CD-93AB32BDC8E7}
DEFINE_GUID(AMF_EFFECT_SKINTONE_CORRECTION, 0x885dccec, 0x77a5, 0x4ca4, 0xb5, 0xcd, 0x93, 0xab, 0x32, 0xbd, 0xc8, 0xe7); // BOOL (default=FALSE)  enables=TRUE / disables=FALSE Skintone correction effect
// {116E5685-4062-4A69-B513-11243ACEBF2D}
DEFINE_GUID(AMF_EFFECT_SKINTONE_CORRECTION_STRENGTH, 0x116e5685, 0x4062, 0x4a69, 0xb5, 0x13, 0x11, 0x24, 0x3a, 0xce, 0xbf, 0x2d); // INT (default=50) min=0 max=100 Skintone correction strength

// AMF VQ Gamma correction
// {161AF051-8202-4EDD-8E97-3918F6FF8B96}
DEFINE_GUID(AMF_EFFECT_GAMMA_CORRECTION, 0x161af051, 0x8202, 0x4edd, 0x8e, 0x97, 0x39, 0x18, 0xf6, 0xff, 0x8b, 0x96); // BOOL (default=FALSE)  enables=TRUE / disables=FALSE Gamma corection effect
// {12C8FFB9-8D79-47A1-9F03-1D0401E7C8E6}
DEFINE_GUID(AMF_EFFECT_GAMMA_CORRECTION_STRENGTH, 0x12c8ffb9, 0x8d79, 0x47a1, 0x9f, 0x3, 0x1d, 0x4, 0x1, 0xe7, 0xc8, 0xe6); // DOUBLE (default=1.0) min=0.5 max=2.5 Gamma corection strength

// AMF VQ Brighter Whites
// {37787660-62AF-4921-A8B3-3C7BFBF68D3B}
DEFINE_GUID(AMF_EFFECT_BRIGHTER_WHITES, 0x37787660, 0x62af, 0x4921, 0xa8, 0xb3, 0x3c, 0x7b, 0xfb, 0xf6, 0x8d, 0x3b); // BOOL (default=FALSE)  enables=TRUE / disables=FALSE Brighter Whites effect

// AMF VQ Dynamic range
// {23D65EFD-67D3-4D32-A286-BAE02D1680EF}
DEFINE_GUID(AMF_EFFECT_DYNAMIC_RANGE, 0x23d65efd, 0x67d3, 0x4d32, 0xa2, 0x86, 0xba, 0xe0, 0x2d, 0x16, 0x80, 0xef); // INT (default=AMF_EFFECT_DYNAMIC_RANGE_NONE)  values - see below Dynamic range effect type
#define AMF_EFFECT_DYNAMIC_RANGE_NONE               -1  // Value for AMF_EFFECT_DYNAMIC_RANGE
#define AMF_EFFECT_DYNAMIC_RANGE_FULL               0  // Value for AMF_EFFECT_DYNAMIC_RANGE (0-255)
#define AMF_EFFECT_DYNAMIC_RANGE_LIMITED            1  // Value for AMF_EFFECT_DYNAMIC_RANGE (16-235)

// AMF VQ Brightness
// {8F0A7F1E-28D3-4969-8855-5B7AAA226B06}
DEFINE_GUID(AMF_EFFECT_BRIGHTNESS, 0x8f0a7f1e, 0x28d3, 0x4969, 0x88, 0x55, 0x5b, 0x7a, 0xaa, 0x22, 0x6b, 0x6); // DOUBLE (default=0) min=-100 max=100 Brightness effect
// {A22714F7-D057-4ED1-B6CB-AF2454604D9D}
DEFINE_GUID(AMF_EFFECT_CONTRAST, 0xa22714f7, 0xd057, 0x4ed1, 0xb6, 0xcb, 0xaf, 0x24, 0x54, 0x60, 0x4d, 0x9d); // DOUBLE (default=1) min=0 max=2 Contrast effect
// {86523A09-F6C5-494D-A1A9-176F28F13F86}
DEFINE_GUID(AMF_EFFECT_SATURATION, 0x86523a09, 0xf6c5, 0x494d, 0xa1, 0xa9, 0x17, 0x6f, 0x28, 0xf1, 0x3f, 0x86); // DOUBLE (default=1) min=0 max=2 Saturation effect
// {99513DC3-74F7-41C7-A542-30B85E4C8CDF}
DEFINE_GUID(AMF_EFFECT_TINT, 0x99513dc3, 0x74f7, 0x41c7, 0xa5, 0x42, 0x30, 0xb8, 0x5e, 0x4c, 0x8c, 0xdf); // DOUBLE (default=0.0) min=-30.0 max=30.0 Tint effect

// AMF VQ False contour reduction
// {3F2D7D54-C1C7-4EA8-A605-91CF9BE3F36B}
DEFINE_GUID(AMF_EFFECT_FALSE_CONTOUR_REDUCTION, 0x3f2d7d54, 0xc1c7, 0x4ea8, 0xa6, 0x5, 0x91, 0xcf, 0x9b, 0xe3, 0xf3, 0x6b); // BOOL (default=FALSE)  enables=TRUE / disables=FALSE False contour reduction effect
// {FA1F0C49-42FF-4043-B109-489341F7C217}
DEFINE_GUID(AMF_EFFECT_FALSE_CONTOUR_REDUCTION_STRENGTH, 0xfa1f0c49, 0x42ff, 0x4043, 0xb1, 0x9, 0x48, 0x93, 0x41, 0xf7, 0xc2, 0x17); // INT (default=50) min=0 max=100 Skintone correction strength

// MF VQ Scale
// {D418CEE6-81B9-478D-88E4-0AC0A96E12D7}
DEFINE_GUID(AMF_EFFECT_SCALE, 0xd418cee6, 0x81b9, 0x478d, 0x88, 0xe4, 0xa, 0xc0, 0xa9, 0x6e, 0x12, 0xd7); // INT (default=AMF_EFFECT_SCALE_BILINEAR)  values - see below Scale effect type
#define AMF_EFFECT_SCALE_BILINEAR                   0  // Value for AMF_EFFECT_SCALE
#define AMF_EFFECT_SCALE_BICUBIC                    1  // Value for AMF_EFFECT_SCALE
// {C61FEBF4-3B6F-4ED1-B7F3-805E929B44AF}
DEFINE_GUID(AMF_EFFECT_SCALE_WIDTH, 0xc61febf4, 0x3b6f, 0x4ed1, 0xb7, 0xf3, 0x80, 0x5e, 0x92, 0x9b, 0x44, 0xaf); // INT (default=no scaling)  width in pixels
// {0A8E0F8F-8A97-4783-BDBF-E31C7A19CB54}
DEFINE_GUID(AMF_EFFECT_SCALE_HEIGHT, 0xa8e0f8f, 0x8a97, 0x4783, 0xbd, 0xbf, 0xe3, 0x1c, 0x7a, 0x19, 0xcb, 0x54); // INT (default=no scaling)  Height in pixels

// Demo mode
// {905ED157-1A7C-4BBB-8C4D-FE852978056F}
DEFINE_GUID(AMF_EFFECT_DEMOMODE, 0x905ed157, 0x1a7c, 0x4bbb, 0x8c, 0x4d, 0xfe, 0x85, 0x29, 0x78, 0x5, 0x6f); // BOOL () (default=FALSE)  enables=TRUE / disables=FALSE

//AMF_EFFECT_DYNAMIC
// IMFAttributes doesn't have event or callback to monitor changes so we add a special property for polling AMF_EFFECT_CHANGED
// {E445EFB0-B3B1-4CAA-B7EE-07071FC3A2D6}
DEFINE_GUID(AMF_EFFECT_DYNAMIC, 0xe445efb0, 0xb3b1, 0x4caa, 0xb7, 0xee, 0x7, 0x7, 0x1f, 0xc3, 0xa2, 0xd6); // BOOL (default=FALSE)  enables=TRUE / disables=FALSE this property set is dynaic and MFT should listen MapChanged event on PropertySetObject
// {1A2024DF-FA37-456C-8A1F-5807878F885D}
DEFINE_GUID(AMF_EFFECT_CHANGED, 0x1a2024df, 0xfa37, 0x456c, 0x8a, 0x1f, 0x58, 0x7, 0x87, 0x8f, 0x88, 0x5d); // BOOL (default=FALSE)  enables=TRUE / disables=FALSE this property set is dynaic and MFT should listen MapChanged event on PropertySetObject

// {A1017DCD-3899-48D3-A623-B953FFF8096C}
DEFINE_GUID(AMF_EFFECT_OUTPUT_FORMAT,0xa1017dcd, 0x3899, 0x48d3, 0xa6, 0x23, 0xb9, 0x53, 0xff, 0xf8, 0x9, 0x6c); // INT (default=AMF_EFFECT_OUTPUT_FORMAT_ALL)  forces MFT to use this format on negotiations
#define AMF_EFFECT_OUTPUT_FORMAT_ALL				0
#define AMF_EFFECT_OUTPUT_FORMAT_BGRA				1
#define AMF_EFFECT_OUTPUT_FORMAT_NV12				2

// Stats 
// {4F94874F-D75C-4E08-88F7-0937C887882B}
DEFINE_GUID(AMF_PROCESSING_TRANSFORM_AVR_TIME, 0x4f94874f, 0xd75c, 0x4e08, 0x88, 0xf7, 0x9, 0x37, 0xc8, 0x87, 0x88, 0x2b); // average processing sample time in ms
// {052797FB-00F9-42C1-A7BE-5723CAF3354C}
DEFINE_GUID(AMF_PROCESSING_TRANSFORM_MIN_TIME, 0x52797fb, 0xf9, 0x42c1, 0xa7, 0xbe, 0x57, 0x23, 0xca, 0xf3, 0x35, 0x4c); // min processing sample time in ms
// {72AD38E2-0BC7-4E01-AFDB-B878A6672F75}
DEFINE_GUID(AMF_PROCESSING_TRANSFORM_MAX_TIME, 0x72ad38e2, 0xbc7, 0x4e01, 0xaf, 0xdb, 0xb8, 0x78, 0xa6, 0x67, 0x2f, 0x75); // max processing sample time in ms

#endif // WINSTORE_APP

// build cache

typedef enum tagAMFCMRequestType
{
    AMF_CM_AVAILABLE    = 0,
    AMF_CM_REALTIME        ,
    AMF_CM_NONREALTIME     ,
}   AMFCMRequestType;

#if defined(WINSTORE_APP)
// Metro C++
//-------------------------------------------------------------------------------------------------
// Cache Builder
//-------------------------------------------------------------------------------------------------
MIDL_INTERFACE("BDAC410A-5D66-43A2-9236-294B5AF45815")
IAMFCacheBuilderEvents : public IInspectable
{
public:
    virtual HRESULT STDMETHODCALLTYPE Invoke(UINT nPercent) = 0;

};

MIDL_INTERFACE("3050A8F5-F860-40B6-A6A2-34FF8A59E885")
IAMFCacheBuilder : public IInspectable
{
public:
    virtual HRESULT STDMETHODCALLTYPE BuildCache(IAMFCacheBuilderEvents *callback) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsBuildCacheRequired() = 0; // return S_FALSE if build is not required
    virtual HRESULT STDMETHODCALLTYPE GetAsicCacheFileName(HSTRING *pAsicCacheFileName) = 0; // 
    virtual HRESULT STDMETHODCALLTYPE GetCachePath(HSTRING *pCachePath) = 0; // 
    virtual HRESULT STDMETHODCALLTYPE SetCachePath(HSTRING bCachePath) = 0; // 

};

//-------------------------------------------------------------------------------------------------
// Capability manager
//-------------------------------------------------------------------------------------------------

MIDL_INTERFACE("D78DA979-908C-4CA0-AF15-AF5987D97814")
IAMFCapabilityManager : public IInspectable
{
public:
    virtual HRESULT STDMETHODCALLTYPE Init(DWORD dwWidth, DWORD dwHeight, BOOL dwInterlaced = FALSE, DWORD deinterlacedMethod = 0) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsEnabled(AMFCMRequestType requestType,HSTRING Feature) = 0;

};
//-------------------------------------------------------------------------------------------------
// Factories
//-------------------------------------------------------------------------------------------------
extern const __declspec(selectany) WCHAR RuntimeClass_mftvqLib_AMFCapabilityManager[] = L"mftvqLib.AMFCapabilityManager";
extern const __declspec(selectany) WCHAR RuntimeClass_mftvqLib_AMFCacheBuilder[] = L"mftvqLib.AMFCacheBuilder";
extern const __declspec(selectany) WCHAR RuntimeClass_mftvqLib_AMFVideoTransform[] = L"mftvqLib.AMFVideoTransform";

#else // WINSTORE_APP
// COM C++
//-------------------------------------------------------------------------------------------------
// Cache Builder
//-------------------------------------------------------------------------------------------------
MIDL_INTERFACE("9E4ED341-77F6-4302-A8B9-8DD56E53EFAA")
IAMFCacheBuilderEvents : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE OnBuildCacheProgress(UINT nPercent) = 0;
};

MIDL_INTERFACE("3050A8F5-F860-40B6-A6A2-34FF8A59E885")
IAMFCacheBuilder : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE BuildCache(IUnknown *pDeviceManager,IAMFCacheBuilderEvents *callback) = 0; // compile shader kernels 
    virtual HRESULT STDMETHODCALLTYPE IsBuildCacheRequired(IUnknown *pDeviceManager) = 0; // return S_FALSE if build is not required
    virtual HRESULT STDMETHODCALLTYPE GetAsicCacheFileName(IUnknown *pDeviceManager, BSTR *pbstrAsicCacheFileName) = 0; // return current file name with binaries for possible download
    virtual HRESULT STDMETHODCALLTYPE GetCachePath(BSTR *pbstrCachePath) = 0; // path where cache is stored for possible download
    virtual HRESULT STDMETHODCALLTYPE SetCachePath(BSTR bstrCachePath) = 0; // path where cache is stored for possible download
};
//-------------------------------------------------------------------------------------------------
// Capability manager
//-------------------------------------------------------------------------------------------------
// {D78DA979-908C-4CA0-AF15-AF5987D97814}
MIDL_INTERFACE("D78DA979-908C-4CA0-AF15-AF5987D97814")
IAMFCapabilityManager : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Init(IUnknown *pDeviceManager,DWORD dwWidth,DWORD dwHeight, BOOL dwInterlaced = FALSE, DWORD deinterlacedMethod = 0) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsEnabled(AMFCMRequestType requestType,GUID Feature) = 0;
};

DEFINE_GUID(CLSID_AMFVideoTransform, 0x2012DC48, 0xE643, 0x4466, 0x8E, 0x6E, 0x01, 0x89, 0xCD, 0xDD, 0x16, 0x52);

//-------------------------------------------------------------------------------------------------
// Factories
//-------------------------------------------------------------------------------------------------

STDAPI AMFCreateCapabilityManagerMFT(REFIID riid, void** ppvObject);
STDAPI AMFCreateCacheBuilderMFT(REFIID riid, void** ppvObject);
STDAPI AMFCreateVideoTransformMFT(REFIID riid, void** ppvObject);

//MM to be deleted when samples switch to AMFCreateVideoTransformMFT
class DECLSPEC_UUID("2012DC48-E643-4466-8E6E-0189CDDD1652")AMFVideoTransform;

#endif // WINSTORE_APP
