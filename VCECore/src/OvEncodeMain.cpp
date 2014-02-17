
/* ============================================================

Copyright (c) 2012 Advanced Micro Devices, Inc.  All rights reserved.

Redistribution and use of this material is permitted under the following 
conditions:

Redistributions must retain the above copyright notice and all terms of this 
license.

In no event shall anyone redistributing or accessing or using this material 
commence or participate in any arbitration or legal action relating to this 
material against Advanced Micro Devices, Inc. or any copyright holders or 
contributors. The foregoing shall survive any expiration or termination of 
this license or any agreement or access or use related to this material. 

ANY BREACH OF ANY TERM OF THIS LICENSE SHALL RESULT IN THE IMMEDIATE REVOCATION 
OF ALL RIGHTS TO REDISTRIBUTE, ACCESS OR USE THIS MATERIAL.

THIS MATERIAL IS PROVIDED BY ADVANCED MICRO DEVICES, INC. AND ANY COPYRIGHT 
HOLDERS AND CONTRIBUTORS "AS IS" IN ITS CURRENT CONDITION AND WITHOUT ANY 
REPRESENTATIONS, GUARANTEE, OR WARRANTY OF ANY KIND OR IN ANY WAY RELATED TO 
SUPPORT, INDEMNITY, ERROR FREE OR UNINTERRUPTED OPERA TION, OR THAT IT IS FREE 
FROM DEFECTS OR VIRUSES.  ALL OBLIGATIONS ARE HEREBY DISCLAIMED - WHETHER 
EXPRESS, IMPLIED, OR STATUTORY - INCLUDING, BUT NOT LIMITED TO, ANY IMPLIED 
WARRANTIES OF TITLE, MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
ACCURACY, COMPLETENESS, OPERABILITY, QUALITY OF SERVICE, OR NON-INFRINGEMENT. 
IN NO EVENT SHALL ADVANCED MICRO DEVICES, INC. OR ANY COPYRIGHT HOLDERS OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, PUNITIVE,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, REVENUE, DATA, OR PROFITS; OR 
BUSINESS INTERRUPTION) HOWEVER CAUSED OR BASED ON ANY THEORY OF LIABILITY 
ARISING IN ANY WAY RELATED TO THIS MATERIAL, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE. THE ENTIRE AND AGGREGATE LIABILITY OF ADVANCED MICRO DEVICES, 
INC. AND ANY COPYRIGHT HOLDERS AND CONTRIBUTORS SHALL NOT EXCEED TEN DOLLARS 
(US $10.00). ANYONE REDISTRIBUTING OR ACCESSING OR USING THIS MATERIAL ACCEPTS 
THIS ALLOCATION OF RISK AND AGREES TO RELEASE ADVANCED MICRO DEVICES, INC. AND 
ANY COPYRIGHT HOLDERS AND CONTRIBUTORS FROM ANY AND ALL LIABILITIES, 
OBLIGATIONS, CLAIMS, OR DEMANDS IN EXCESS OF TEN DOLLARS (US $10.00). THE 
FOREGOING ARE ESSENTIAL TERMS OF THIS LICENSE AND, IF ANY OF THESE TERMS ARE 
CONSTRUED AS UNENFORCEABLE, FAIL IN ESSENTIAL PURPOSE, OR BECOME VOID OR 
DETRIMENTAL TO ADVANCED MICRO DEVICES, INC. OR ANY COPYRIGHT HOLDERS OR 
CONTRIBUTORS FOR ANY REASON, THEN ALL RIGHTS TO REDISTRIBUTE, ACCESS OR USE 
THIS MATERIAL SHALL TERMINATE IMMEDIATELY. MOREOVER, THE FOREGOING SHALL 
SURVIVE ANY EXPIRATION OR TERMINATION OF THIS LICENSE OR ANY AGREEMENT OR 
ACCESS OR USE RELATED TO THIS MATERIAL.

NOTICE IS HEREBY PROVIDED, AND BY REDISTRIBUTING OR ACCESSING OR USING THIS 
MATERIAL SUCH NOTICE IS ACKNOWLEDGED, THAT THIS MATERIAL MAY BE SUBJECT TO 
RESTRICTIONS UNDER THE LAWS AND REGULATIONS OF THE UNITED STATES OR OTHER 
COUNTRIES, WHICH INCLUDE BUT ARE NOT LIMITED TO, U.S. EXPORT CONTROL LAWS SUCH 
AS THE EXPORT ADMINISTRATION REGULATIONS AND NATIONAL SECURITY CONTROLS AS 
DEFINED THEREUNDER, AS WELL AS STATE DEPARTMENT CONTROLS UNDER THE U.S. 
MUNITIONS LIST. THIS MATERIAL MAY NOT BE USED, RELEASED, TRANSFERRED, IMPORTED,
EXPORTED AND/OR RE-EXPORTED IN ANY MANNER PROHIBITED UNDER ANY APPLICABLE LAWS, 
INCLUDING U.S. EXPORT CONTROL LAWS REGARDING SPECIFICALLY DESIGNATED PERSONS, 
COUNTRIES AND NATIONALS OF COUNTRIES SUBJECT TO NATIONAL SECURITY CONTROLS. 
MOREOVER, THE FOREGOING SHALL SURVIVE ANY EXPIRATION OR TERMINATION OF ANY 
LICENSE OR AGREEMENT OR ACCESS OR USE RELATED TO THIS MATERIAL.

NOTICE REGARDING THE U.S. GOVERNMENT AND DOD AGENCIES: This material is 
provided with "RESTRICTED RIGHTS" and/or "LIMITED RIGHTS" as applicable to 
computer software and technical data, respectively. Use, duplication, 
distribution or disclosure by the U.S. Government and/or DOD agencies is 
subject to the full extent of restrictions in all applicable regulations, 
including those found at FAR52.227 and DFARS252.227 et seq. and any successor 
regulations thereof. Use of this material by the U.S. Government and/or DOD 
agencies is acknowledgment of the proprietary rights of any copyright holders 
and contributors, including those of Advanced Micro Devices, Inc., as well as 
the provisions of FAR52.227-14 through 23 regarding privately developed and/or 
commercial computer software.

This license forms the entire agreement regarding the subject matter hereof and 
supersedes all proposals and prior discussions and writings between the parties 
with respect thereto. This license does not affect any ownership, rights, title,
or interest in, or relating to, this material. No terms of this license can be 
modified or waived, and no breach of this license can be excused, unless done 
so in a writing signed by all affected parties. Each term of this license is 
separately enforceable. If any term of this license is determined to be or 
becomes unenforceable or illegal, such term shall be reformed to the minimum 
extent necessary in order for this license to remain in effect in accordance 
with its terms as modified by such reformation. This license shall be governed 
by and construed in accordance with the laws of the State of Texas without 
regard to rules on conflicts of law of any state or jurisdiction or the United 
Nations Convention on the International Sale of Goods. All disputes arising out 
of this license shall be subject to the jurisdiction of the federal and state 
courts in Austin, Texas, and all defenses are hereby waived concerning personal 
jurisdiction and venue of these courts.

============================================================ */

/**  
********************************************************************************
* @file <OvEncodeMain.cpp>                          
*                                       
* @brief This is a sample program showing how to use the encoder
*         
********************************************************************************
*/
#if FALSE

/*******************************************************************************
*                             INCLUDE FILES                                    *
*******************************************************************************/
#include "OvEncodeTest.h"
#include "OvEncodePerf.h"

int32 main(int32 argc, int8* argv[])
{
    int8 filename[128] = {0};
    int8 output[128] = {0};
	int8 configFilename[128] = {0};
	cl_device_id clDeviceID;
	
	/**************************************************************************/
	/* Create profile counters                                                */
	/**************************************************************************/
	OVprofile perfCounter;
	OVDeviceHandle deviceHandle;
	OPContextHandle      oveContext;
	/**************************************************************************/
	/* Create encoder handle                                                  */
	/**************************************************************************/
	OVEncodeHandle encodeHandle;

	bool status;

    /**************************************************************************/
    /* Currently the OpenEncode support is only for vista and w7              */
    /**************************************************************************/
    bool isVistaOrWin7 = false;
	OvConfigCtrl              configCtrl;

    /**************************************************************************/
    /* Find the version of Windows                                            */
    /**************************************************************************/
    OSVERSIONINFO vInfo;
    memset(&vInfo, 0, sizeof(vInfo));
    vInfo.dwOSVersionInfoSize = sizeof(vInfo);
    if(!GetVersionEx(&vInfo))
    {
        std::cout << "Error : Unable to get Windows version information\n" << std::endl;
        return 1;
    }

    /**************************************************************************/
    /* Tell the user that this only runs on Win7 or Vista                     */
    /**************************************************************************/
    if(vInfo.dwMajorVersion >= 6)
        isVistaOrWin7 = true;

    if(!isVistaOrWin7)
    {
        std::cout << "Error : Unsupported OS! Vista/Win7 required.\n";
        return 1;
    }

	/***************************************************************************/
	/* Helps on command line configuration usage cases                         */
	/* exe -i input_yuvfile.yuv -o output_bitstreamfile.h264 -c balanced.cfg   */
	/***************************************************************************/

    if (argc<=2)
    {
        std::cout<<"Help on encoding usages and configurations...\n";
        std::cout<<"ovEncodeVc.exe -i input_Yuv_file.yuv -o output_h264_encoded_file.h264 -c balanced.cfg.cfg\n";
        return 1;
    }

    /**************************************************************************/
    /* processing the command line and configuration file                     */
    /**************************************************************************/
    int32 index = 0;
	int32 argCheck=0;
    while (index < argc) 
    {
		if (strncmp(argv[index], "-h", 2) == 0 ) {
            printf("Help on encoding usages and configurations...\n");
            printf("ovEncode.exe -i input_Yuv_file.yuv -o output_h264_encoded_file.h264 -c balanced.cfg\n");
            return 1;
        }

        /**********************************************************************/
        /* processing working directory and input file                        */
        /**********************************************************************/
        if (strncmp (argv[index], "-i", 2) == 0) 
        {
            strcat_s(filename, 128, argv[index+1]);
			argCheck++;
        }

        /**********************************************************************/
        /* processing working directory and output file                       */
        /**********************************************************************/
        if (strncmp (argv[index], "-o", 2) == 0 ) 
        {
            strcat_s(output, 128, argv[index+1]);
			argCheck++;
        }

		if (strncmp(argv[index], "-c", 2) == 0 ) 
        {
			strcat_s(configFilename, 128, argv[index+1]);
			argCheck++;
        }

        index++;
    }
	if(argCheck != 3)
	{
		std::cout<<"Help on encoding usages and configurations...\n";
        std::cout<<"OVEncode.exe -i input_Yuv_file.yuv -o output_h264_encoded_file.h264 -c OVEncode_config_file.cfg\n";
		return 1;
	}


    /**************************************************************************/
    /* get the pointer to configuration controls                              */
    /**************************************************************************/
	
    OvConfigCtrl *pConfigCtrl = (OvConfigCtrl *) &configCtrl;
    memset (pConfigCtrl, 0, sizeof (OvConfigCtrl));
    if (encodeConfig(pConfigCtrl, configFilename) != true)
		return 1;

	/**************************************************************************/
	/* initialize profile counters                                            */
	/**************************************************************************/
	initProfileCnt(&perfCounter);

	/**************************************************************************/
	/* Query for the device information                                       */
	/* This function fills the device handle with number of devices available */
	/* and devices ids                                                        */
	/**************************************************************************/
	std::cout<<"Initializing Encoder...\n";
	status = getDevice(&deviceHandle);
	if (status == false)
	{
        return 1;
	}
	/**************************************************************************/
	/* Check deviceHandle.numDevices for number of devices and choose the     */
	/* device on which user wants to create the encoder                       */
	/* In this case device 0 is choosen                                       */
	/**************************************************************************/
	uint32 deviceId = deviceHandle.deviceInfo[0].device_id;
	
	/**************************************************************************/
	/* Create the encoder context on the device specified by deviceID         */
	/**************************************************************************/
	encodeCreate(&oveContext,deviceId,&deviceHandle);
	/**************************************************************************/
	/* initialize the encoder session with configuration specified by the     */
	/* pConfigCtrl                                                            */
	/**************************************************************************/
	encodeOpen(&encodeHandle,oveContext,deviceId,pConfigCtrl);

    /**************************************************************************/
    /* Encode a file.                                                         */
    /**************************************************************************/
	std::cout<<"Encoding...\n";
    status = encodeProcess(&encodeHandle,filename, output, pConfigCtrl,
						   &perfCounter);
	if (status == false)
	{
        return 1;
	}

	/**************************************************************************/
	/* Free the resources used by the encoder session                         */
	/**************************************************************************/
	status = encodeClose(&encodeHandle);
	if (status == false)
	{
        return 1;
	}
	/**************************************************************************/
	/* Destroy the encoder context                                            */
	/**************************************************************************/
	status = encodeDestroy(oveContext);
	/**************************************************************************/
    /* Free memory used for deviceInfo.                                       */
	/**************************************************************************/
	delete [] deviceHandle.deviceInfo;

	/**************************************************************************/
	/* All done, Check the status of destroy and display the Frame Rate       */
	/**************************************************************************/
	if (status == false)
	{
        return 1;
	}
	else
	{
		std::cout<<"Encoding complete. Output written to "<<output<<"\n";
		clDeviceID = reinterpret_cast<cl_device_id>(deviceId);
		displayFps(&perfCounter,clDeviceID);
	    return 0;
	}
}
#endif
