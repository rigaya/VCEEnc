
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
* @file <OvEncodePerf.cpp>                          
*                                       
* @brief This file contains functions for measuring the performance
*         
********************************************************************************
*/
/*******************************************************************************
*                             INCLUDE FILES                                    *
*******************************************************************************/
#include "OvEncodePerf.h"

/** 
 *******************************************************************************
 *  @fn     initProfileCnt
 *  @brief  Initialize the timers
 *           
 *  @param[in/out] profileCnt : Pointer to the structure containing profile counters
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
void initProfileCnt(OVprofile *profileCnt)
{
	int32 i;
	for(i=0;i<MAX_TIMING;i++)
	{
		profileCnt->callCount[i] = 0;
		profileCnt->sTime[i] = 0;
		profileCnt->accSum[i] = 0;
	}
}
/** 
 *******************************************************************************
 *  @fn     displayFps
 *  @brief  Calculates Frames per sec
 *           
 *  @param[in/out] profileCnt : Pointer to the structure containing profile counters
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
void displayFps(OVprofile *profileCnt,cl_device_id clDeviceID )
{
	static int32 dumped = 0; if(dumped) return; dumped = 1;
	uint32 gpuFreq;
	FILE * fp = stdout;
	float32 mean0, mean1;
	float32 perf0, perf1;
	if(fp) 
	{
		LARGE_INTEGER li, l2;
		QueryPerformanceFrequency(&li);
		getGpuFrequency(clDeviceID,&gpuFreq);
		float32 freq = (float32)li.QuadPart;
		QueryPerformanceCounter(&li);
		int64 stime = myRdtsc();
		Sleep(1000);
		QueryPerformanceCounter(&l2);
		int64 etime = myRdtsc();
		freq *= ((float32)(etime-stime))/((float32)(l2.QuadPart-li.QuadPart));
		fprintf(fp,"\nVCE Performance\n");
		printf("Processor Frequency: %5.2f MHz\n", freq/1000000);
		printf("GPU Frequency      : %6.2f  MHz\n", (float32)gpuFreq);
		for(int32 i = 0; i < MAX_TIMING; i++)
		{
			if(profileCnt->callCount[i]) {
				float32 count =  (float32)(profileCnt->callCount[i] - SKIP_TIMING);
				float32 mean  = ((float32) profileCnt->accSum[i])/count;
				if (i == 0)
				{
					mean0 = mean;
					perf0 = freq/mean0;
				}
				if (i == 1) {
					mean1 = mean;
					perf1 = freq/(mean0+mean1);
				}
			}
		}
		fprintf(fp, "VCE Frame Rate     : %5.2f [FPS]\n",perf1);

		if(fp != stdout)fclose(fp);
	}
}
/** 
 *******************************************************************************
 *  @fn     getGpuFrequency
 *  @brief  Returns the GPU clock frequency
 *           
 *  @param[in] clDeviceID : Device ID for gpu
 *  @param[in] gpuFreq    : Gpu frequency to be written by the function 
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
cl_int getGpuFrequency(cl_device_id clDeviceID, uint32 *gpuFreq)
{
	cl_int status;
	size_t size;
	status = clGetDeviceInfo(clDeviceID,
                CL_DEVICE_MAX_CLOCK_FREQUENCY,
                sizeof(uint32),
                gpuFreq, &size
                );
	return(status);
}
/** 
 *******************************************************************************
 *  @fn     captureTimeStop
 *  @brief  calculates difference between start and end timers 
 *           
 *  @param[in/out] profileCnt : Pointer to the structure containing profile counters
 *  @param[in]     type       : Timer type
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
void captureTimeStop(OVprofile *profileCnt, int32 type)
{
	if(profileCnt->callCount[type]++ >= SKIP_TIMING) 
	{ 
		int64 time = myRdtsc() - profileCnt->sTime[type]; 
		profileCnt->accSum[type] += time; 
	}
}
/** 
 *******************************************************************************
 *  @fn     captureTimeStart
 *  @brief  Records start of the timer
 *           
 *  @param[in/out] profileCnt : Pointer to the structure containing profile counters
 *  @param[in]     type       : Timer type
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
void captureTimeStart(OVprofile *profileCnt, int32 type)
{
	profileCnt->sTime[type] = myRdtsc();
}

