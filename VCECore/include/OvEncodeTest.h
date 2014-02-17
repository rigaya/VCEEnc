
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
* @file <OvEncodeTest.h>                          
*                                       
* @brief Contains declaration encode APIs
*         
********************************************************************************
*/
#ifndef _OVENCODETEST_h_ 
#define _OVENCODETEST_h_ 
/*******************************************************************************
*                             INCLUDE FILES                                    *
*******************************************************************************/
#include "OvEncodeConfig.h"
#include "OvEncodePerf.h"
#include "OvEncodeTypedef.h"
#include "OpenVideo\OVEncodeTypes.h"
#include "cl\cl.h"
/******************************************************************************/
/* Input surface used for encoder                                             */
/******************************************************************************/
#define			            MAX_INPUT_SURFACE      15

typedef struct OVDeviceHandle
{
    ovencode_device_info *deviceInfo; /**< Pointer to device info        */
	uint32                numDevices; /**< Number of devices available   */
	cl_platform_id        platform;   /**< Platform                      */
}OVDeviceHandle;
/******************************************************************************/
/* Encoder Hanlde for sharing context between create process and destroy      */
/******************************************************************************/
typedef struct OVEncodeHandle
{
	ove_session          session;       /**< Pointer to encoder session   */
	OPMemHandle		     inputSurfaces[MAX_INPUT_SURFACE]; /**< input buffer  */
	cl_command_queue     clCmdQueue;    /**< command queue  */
}OVEncodeHandle;
/** 
 *******************************************************************************
 *  @fn     getDevice
 *  @brief  returns the platform and devices found
 *           
 *  @param[in/out] deviceHandle : Hanlde for the device information
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool getDevice(OVDeviceHandle *deviceHandle);
/** 
 *******************************************************************************
 *  @fn     encodeCreate
 *  @brief  Creates encoder context 
 *           
 *  @param[in/out] oveContext   : Hanlde to the encoder context
 *  @param[in] deviceID         : Device on which encoder context to be created
 *  @param[in] deviceHandle     : Hanlde for the device information
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */

bool encodeCreate(OPContextHandle *oveContext,uint32 deviceId,
                    OVDeviceHandle *deviceHandle);
/** 
 *******************************************************************************
 *  @fn     encodeOpen
 *  @brief  Creates encoder session, buffers and initilizes 
 *          configuration for the encoder session
 *           
 *  @param[in/out] encodeHandle : Hanlde to the encoder instance
 *  @param[in] oveContext       : Hanlde to the encoder context
 *  @param[in] deviceID         : Device on which encoder context to be created
 *  @param[in] deviceHandle     : Hanlde for the device information
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool encodeOpen(OVEncodeHandle *encodeHandle,OPContextHandle oveContext,
	            uint32 deviceId,OvConfigCtrl *pConfig);
/** 
 *******************************************************************************
 *  @fn     encodeProcess
 *  @brief  Encode an input video file and output encoded H.264 video file
 *           
 *  @param[in] encodeHandle : Hanlde for the encoder 
 *  @param[in] inFile		: input video file to be encoded
 *  @param[out] outFile		: output encoded H.264 video file
 *  @param[in] pConfig		: pointer to custom configuration setting file
 *  @param[out] profileCnt  : pointer to profile couters 
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool encodeProcess(OVEncodeHandle *encodeHandle,int8 *inFile, int8 *outFile, 
				 OvConfigCtrl *pConfig, OVprofile *profileCnt);
/** 
 *******************************************************************************
 *  @fn     encodeClose
 *  @brief  This function destroys the resources used by the encoder session
 *
 *  @param[in] encodeHandle : Handle for the encoder context
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */

bool encodeClose(OVEncodeHandle *encodeHandle);
/** 
 *******************************************************************************
 *  @fn     encodeDestroy
 *  @brief  Destroy encoder context
 *
 *  @param[in] oveContext : Handle for the encoder context
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool encodeDestroy(OPContextHandle oveContext);


#endif