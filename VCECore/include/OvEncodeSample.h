
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
* @file <OvEncodeSample.h>                          
*                                       
* @brief Contains declaration for h.264 encoding functions 
*         
********************************************************************************
*/
#ifndef _OVEENCODESAMPLE_h_ 
#define _OVEENCODESAMPLE_h_ 

/*******************************************************************************
*                             INCLUDE FILES                                    *
*******************************************************************************/
#include <io.h>
#include "OvEncodeTest.h"

/*******************************************************************************
*                             Function definition                              *
*******************************************************************************/
/** 
 *******************************************************************************
 *  @fn     getPlatform
 *  @brief  Get platform to run
 *           
 *  @param[in] platform : Platform id
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool getPlatform(cl_platform_id &platform);
/** 
 *******************************************************************************
 *  @fn     gpuCheck
 *  @brief  Checks for GPU present or not
 *           
 *  @param[in] platform : Platform id
 *  @param[out] dType   : Device type returned GPU/CPU
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool gpuCheck(cl_platform_id platform,cl_device_type* dType);
/** 
 *******************************************************************************
 *  @fn     getDeviceInfo
 *  @brief  returns device information
 *           
 *  @param[out] deviceInfo  : Device info
 *  @param[out] numDevices  : Number of devices present
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool getDeviceInfo(ovencode_device_info **deviceInfo,
					 uint32 *numDevices);
/** 
 *******************************************************************************
 *  @fn     getDeviceCap
 *  @brief  This function returns the device capabilities.
 *           
 *  @param[in] oveContext   : Encoder context 
 *  @param[in] oveDeviceID  : Device ID
 *  @param[out] encodeCaps : pointer to encoder capabilities structure
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool getDeviceCap(OPContextHandle oveContext,uint32 oveDeviceID,
				OVE_ENCODE_CAPS *encodeCaps);
/** 
 *******************************************************************************
 *  @fn     setEncodeConfig
 *  @brief  This function sets the encoder configuration by using user 
 *          supplied configuration information from .cfg file
 *           
 *  @param[in] session : Encoder session for which encoder configuration to be
 *                       set
 *  @param[in] pConfig : pointer to the user configuration from .cfg file
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool setEncodeConfig(ove_session session, OvConfigCtrl *pConfig);
/** 
 *******************************************************************************
 *  @fn     getFrameCount
 *  @brief  This function returns the number of frames present in the yuv file 
 *           
 *  @param[in] inFile : Pointer to the yuv input file
 *  @param[out] frameCount : frame count
 *  @param[in] width : width of the input image 
 *  @param[in] height : height of the input image 
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool getFrameCount(int8 *inFile,uint32 *frameNum,int32 width,int32 height);
/** 
 *******************************************************************************
 *  @fn     waitForEvent
 *  @brief  This function waits for the event completion 
 *           
 *  @param[in] inMapEvt : Event for which it has to wait for completion
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
void waitForEvent(cl_event inMapEvt);
/** 
 *******************************************************************************
 *  @fn     yuvToNV12
 *  @brief   Read yuv video file and converts it to NV12 format
 *           
 *  @param[in] fr  : File pointer to the input video file 
 *  @param[in] uiHeight : video frame height
 *  @param[in] uiWidth  : video frame width
 *  @param[in] alignedSurfaceWidth  : aligned frame width
 *  @param[out] *pBitstreamData : input surface buffer pointer
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */

bool yuvToNV12(FILE *fr, uint32 uiHeight, uint32 uiWidth, 
               uint32 alignedSurfaceWidth, int8 *pBitstreamData);
#endif