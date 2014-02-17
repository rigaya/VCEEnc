
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
* @file <OvEncodeConfig.cpp>                          
*                                       
* @brief This file contains functions for reading the configuration file
*         
********************************************************************************
*/
/*******************************************************************************
*                             INCLUDE FILES                                    *
*******************************************************************************/
#include "OvEncodeConfig.h"

using namespace std;

/******************************************************************************/
/* Encode configuration from command lines                                    */
/******************************************************************************/
/** 
 *******************************************************************************
 *  @fn     encodeConfig
 *  @brief  Reads configuration file provided by user 
 *           
 *  @param[in/out] pConfig        : Pointer to the configuration structure
 *  @param[in] configFilename : User configuration file name
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool encodeConfig (OvConfigCtrl *pConfig, int8 *configFilename)
{
	map<string, int32 > configTable;
	prepareConfigMap(&configTable);
	printf("Reading user-specified configuration file: %s\n", configFilename);
	if (readConfigFile (configFilename, pConfig,&configTable) != true)
		return false;
	return true;
}
/** 
 *******************************************************************************
 *  @fn     prepareConfigMap
 *  @brief  configuration mapping table, used for mapping values from user     
 *          configuration to config control structure 
 *           
 *  @param[in/out] pConfigTable   : Pointer to the configuration map table
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
void prepareConfigMap(map<string,int32>* pConfigTable)
{
	/**************************************************************************/
	/* EncodeSpecifications                                                   */  
	/**************************************************************************/
	pConfigTable->insert(pair<string,int32>("pictureHeight", 144)); 
	pConfigTable->insert(pair<string,int32>("pictureWidth", 176)); 
	pConfigTable->insert(pair<string,int32>("EncodeMode", 1)); 
	pConfigTable->insert(pair<string,int32>("level", 30)); 
	pConfigTable->insert(pair<string,int32>("profile", 66)); 
	pConfigTable->insert(pair<string,int32>("pictureFormat", 1)); 
	pConfigTable->insert(pair<string,int32>("requestedPriority", 1)); 
	
	/**************************************************************************/
	/* ConfigPicCtl                                                           */ 
	/**************************************************************************/
	pConfigTable->insert(pair<string,int32>("useConstrainedIntraPred", 0)); 
	pConfigTable->insert(pair<string,int32>("CABACEnable", 0)); 
	pConfigTable->insert(pair<string,int32>("CABACIDC", 0)); 
	pConfigTable->insert(pair<string,int32>("loopFilterDisable", 0)); 
	pConfigTable->insert(pair<string,int32>("encLFBetaOffset", 0)); 
	pConfigTable->insert(pair<string,int32>("encLFAlphaC0Offset", 0)); 
	pConfigTable->insert(pair<string,int32>("encIDRPeriod", 0)); 
	pConfigTable->insert(pair<string,int32>("encIPicPeriod", 0)); 
	pConfigTable->insert(pair<string,int32>("encHeaderInsertionSpacing", 0)); 
	pConfigTable->insert(pair<string,int32>("encCropLeftOffset", 0)); 
	pConfigTable->insert(pair<string,int32>("encCropRightOffset", 0)); 
	pConfigTable->insert(pair<string,int32>("encCropTopOffset", 0)); 
	pConfigTable->insert(pair<string,int32>("encCropBottomOffset", 0)); 
	pConfigTable->insert(pair<string,int32>("encNumMBsPerSlice", 99)); 
	pConfigTable->insert(pair<string,int32>("encNumSlicesPerFrame", 1)); 
	pConfigTable->insert(pair<string,int32>("encForceIntraRefresh", 0)); 
	pConfigTable->insert(pair<string,int32>("encForceIMBPeriod", 0)); 
	pConfigTable->insert(pair<string,int32>("encInsertVUIParam", 0)); 
	pConfigTable->insert(pair<string,int32>("encInsertSEIMsg", 0)); 
	
	/**************************************************************************/
	/* ConfigRateCtl                                                          */
	/**************************************************************************/
	pConfigTable->insert(pair<string,int32>("encRateControlMethod", 3)); 
	pConfigTable->insert(pair<string,int32>("encRateControlTargetBitRate", 768000)); 
	pConfigTable->insert(pair<string,int32>("encRateControlPeakBitRate", 0)); 
	pConfigTable->insert(pair<string,int32>("encRateControlFrameRateNumerator", 30)); 
	pConfigTable->insert(pair<string,int32>("encGOPSize", 0)); 
	pConfigTable->insert(pair<string,int32>("encRCOptions", 0)); 
	pConfigTable->insert(pair<string,int32>("encQP_I", 22)); 
	pConfigTable->insert(pair<string,int32>("encQP_P", 22)); 
	pConfigTable->insert(pair<string,int32>("encQP_B", 0)); 
	pConfigTable->insert(pair<string,int32>("encVBVBufferSize", 768000)); 
	pConfigTable->insert(pair<string,int32>("encRateControlFrameRateDenominator", 1)); 

	/**************************************************************************/
	/* ConfigMotionEstimation				                                  */
	/**************************************************************************/
	pConfigTable->insert(pair<string,int32>("IMEDecimationSearch", 1)); 
	pConfigTable->insert(pair<string,int32>("motionEstHalfPixel", 1)); 
	pConfigTable->insert(pair<string,int32>("motionEstQuarterPixel", 1)); 
	pConfigTable->insert(pair<string,int32>("disableFavorPMVPoint", 0)); 
	pConfigTable->insert(pair<string,int32>("forceZeroPointCenter", 1)); 
	pConfigTable->insert(pair<string,int32>("LSMVert", 0)); 
	pConfigTable->insert(pair<string,int32>("encSearchRangeX", 16)); 
	pConfigTable->insert(pair<string,int32>("encSearchRangeY", 16)); 
	pConfigTable->insert(pair<string,int32>("encSearch1RangeX", 0)); 
	pConfigTable->insert(pair<string,int32>("encSearch1RangeY", 0)); 
	pConfigTable->insert(pair<string,int32>("disable16x16Frame1", 0)); 
	pConfigTable->insert(pair<string,int32>("disableSATD", 0)); 
	pConfigTable->insert(pair<string,int32>("enableAMD", 0)); 
	pConfigTable->insert(pair<string,int32>("encDisableSubMode", 0)); 
	pConfigTable->insert(pair<string,int32>("encIMESkipX", 0)); 
	pConfigTable->insert(pair<string,int32>("encIMESkipY", 0)); 
	pConfigTable->insert(pair<string,int32>("encEnImeOverwDisSubm", 0)); 
	pConfigTable->insert(pair<string,int32>("encImeOverwDisSubmNo", 0)); 
	pConfigTable->insert(pair<string,int32>("encIME2SearchRangeX", 4)); 
	pConfigTable->insert(pair<string,int32>("encIME2SearchRangeY", 4)); 

	/**************************************************************************/
	/* ConfigRDO                                                              */
	/**************************************************************************/
	pConfigTable->insert(pair<string,int32>("encDisableTbePredIFrame", 0)); 
	pConfigTable->insert(pair<string,int32>("encDisableTbePredPFrame", 0)); 
	pConfigTable->insert(pair<string,int32>("useFmeInterpolY", 0)); 
	pConfigTable->insert(pair<string,int32>("useFmeInterpolUV", 0)); 
	pConfigTable->insert(pair<string,int32>("enc16x16CostAdj", 0)); 
	pConfigTable->insert(pair<string,int32>("encSkipCostAdj", 0)); 
    pConfigTable->insert(pair<string,int32>("encForce16x16skip", 0)); 

}
/** 
 *******************************************************************************
 *  @fn     encodeSetParam
 *  @brief  Setting up configuration parameters
 *           
 *  @param[in/out] pConfig   : Pointer to the configuration structure 
 *  @param[in] pConfigTable  : Pointer to the configuration map table
 *          
 *  @return void
 *******************************************************************************
 */
void encodeSetParam(OvConfigCtrl *pConfig, map<string,int32>* pConfigTable)
{

	/**************************************************************************/
    /* fill-in the general configuration structures                           */
	/**************************************************************************/
	map<string,int32> configTable = (map<string,int32>)*pConfigTable;
	pConfig->height									= configTable["pictureHeight"];
	pConfig->width									= configTable["pictureWidth"];
	pConfig->encodeMode								= (OVE_ENCODE_MODE) configTable["EncodeMode"];

	/**************************************************************************/
	/* fill-in the profile and level                                          */
	/**************************************************************************/
	pConfig->profileLevel.level						= configTable["level"];
	pConfig->profileLevel.profile					= configTable["profile"];

	pConfig->pictFormat								= (OVE_PICTURE_FORMAT)configTable["pictureFormat"];
	pConfig->priority								    = (OVE_ENCODE_TASK_PRIORITY)configTable["requestedPriority"];

	/**************************************************************************/
	/* fill-in the picture control structures                                 */
	/**************************************************************************/
	pConfig->pictControl.size                         = sizeof(OVE_CONFIG_PICTURE_CONTROL);
	pConfig->pictControl.useConstrainedIntraPred	    = configTable["useConstrainedIntraPred"];
	pConfig->pictControl.cabacEnable					= configTable["CABACEnable"];
	pConfig->pictControl.cabacIDC					    = configTable["CABACIDC"];
	pConfig->pictControl.loopFilterDisable			= configTable["loopFilterDisable"];
	pConfig->pictControl.encLFBetaOffset				= configTable["encLFBetaOffset"];
	pConfig->pictControl.encLFAlphaC0Offset			= configTable["encLFAlphaC0Offset"];
	pConfig->pictControl.encIDRPeriod				    = configTable["encIDRPeriod"];
	pConfig->pictControl.encIPicPeriod				= configTable["encIPicPeriod"];
	pConfig->pictControl.encHeaderInsertionSpacing    = configTable["encHeaderInsertionSpacing"];
	pConfig->pictControl.encCropLeftOffset			= configTable["encCropLeftOffset"];
	pConfig->pictControl.encCropRightOffset			= configTable["encCropRightOffset"];
	pConfig->pictControl.encCropTopOffset			    = configTable["encCropTopOffset"];
	pConfig->pictControl.encCropBottomOffset			= configTable["encCropBottomOffset"];
	pConfig->pictControl.encNumMBsPerSlice		    = configTable["encNumMBsPerSlice"];
	pConfig->pictControl.encNumSlicesPerFrame			= configTable["encNumSlicesPerFrame"];
	pConfig->pictControl.encForceIntraRefresh		    = configTable["encForceIntraRefresh"];
	pConfig->pictControl.encForceIMBPeriod			= configTable["encForceIMBPeriod"];
	pConfig->pictControl.encInsertVUIParam			= configTable["encInsertVUIParam"];
	pConfig->pictControl.encInsertSEIMsg              = configTable["encInsertSEIMsg"];

	/**************************************************************************/
	/* fill-in the rate control structures                                    */
	/**************************************************************************/
	pConfig->rateControl.size                         = sizeof(OVE_CONFIG_RATE_CONTROL);
	pConfig->rateControl.encRateControlMethod		    = configTable["encRateControlMethod"];
	pConfig->rateControl.encRateControlTargetBitRate	= configTable["encRateControlTargetBitRate"];
	pConfig->rateControl.encRateControlPeakBitRate    = configTable["encRateControlPeakBitRate"];
	pConfig->rateControl.encRateControlFrameRateNumerator = configTable["encRateControlFrameRateNumerator"];
	pConfig->rateControl.encGOPSize					= configTable["encGOPSize"];
	pConfig->rateControl.encRCOptions					= configTable["encRCOptions"];
	pConfig->rateControl.encQP_I						= configTable["encQP_I"];
	pConfig->rateControl.encQP_P						= configTable["encQP_P"];
	pConfig->rateControl.encQP_B						= configTable["encQP_B"];
	pConfig->rateControl.encVBVBufferSize			    = configTable["encVBVBufferSize"];
	pConfig->rateControl.encRateControlFrameRateDenominator = configTable["encRateControlFrameRateDenominator"];

	/**************************************************************************/
	/* fill-in the motion estimation control structures                       */
	/**************************************************************************/
	pConfig->meControl.size                           = sizeof(OVE_CONFIG_MOTION_ESTIMATION);
	pConfig->meControl.imeDecimationSearch			= configTable["IMEDecimationSearch"];
	pConfig->meControl.motionEstHalfPixel		    = configTable["motionEstHalfPixel"];
	pConfig->meControl.motionEstQuarterPixel		= configTable["motionEstQuarterPixel"];
	pConfig->meControl.disableFavorPMVPoint			= configTable["disableFavorPMVPoint"];
	pConfig->meControl.forceZeroPointCenter			= configTable["forceZeroPointCenter"];
	pConfig->meControl.lsmVert						= configTable["LSMVert"];
	pConfig->meControl.encSearchRangeX				= configTable["encSearchRangeX"];
	pConfig->meControl.encSearchRangeY				= configTable["encSearchRangeY"];
	pConfig->meControl.encSearch1RangeX				= configTable["encSearch1RangeX"];
	pConfig->meControl.encSearch1RangeY				= configTable["encSearch1RangeY"];
	pConfig->meControl.disable16x16Frame1			= configTable["disable16x16Frame1"];
	pConfig->meControl.disableSATD					= configTable["disableSATD"];
	pConfig->meControl.enableAMD					= configTable["enableAMD"];
	pConfig->meControl.encDisableSubMode			= configTable["encDisableSubMode"];
	pConfig->meControl.encIMESkipX					= configTable["encIMESkipX"];
	pConfig->meControl.encIMESkipY					= configTable["encIMESkipY"];
	pConfig->meControl.encEnImeOverwDisSubm			= configTable["encEnImeOverwDisSubm"];
	pConfig->meControl.encImeOverwDisSubmNo			= configTable["encImeOverwDisSubmNo"];
	pConfig->meControl.encIME2SearchRangeX			= configTable["encIME2SearchRangeX"];
	pConfig->meControl.encIME2SearchRangeY			= configTable["encIME2SearchRangeY"];

	/**************************************************************************/
	/* fill-in the RDO control structures                                     */
	/**************************************************************************/
	pConfig->rdoControl.size                        = sizeof(OVE_CONFIG_RDO);
	pConfig->rdoControl.encDisableTbePredIFrame		= configTable["encDisableTbePredIFrame"];
	pConfig->rdoControl.encDisableTbePredPFrame		= configTable["encDisableTbePredPFrame"];
	pConfig->rdoControl.useFmeInterpolY				= configTable["useFmeInterpolY"];
	pConfig->rdoControl.useFmeInterpolUV			= configTable["useFmeInterpolUV"];
	pConfig->rdoControl.enc16x16CostAdj				= configTable["enc16x16CostAdj"];
	pConfig->rdoControl.encSkipCostAdj				= configTable["encSkipCostAdj"];
	pConfig->rdoControl.encForce16x16skip			= (uint8)configTable["encForce16x16skip"];
}

/** 
 *******************************************************************************
 *  @fn     readConfigFile
 *  @brief  Reading in user-specified configuration file
 *           
 *  @param[in] fileName           : user specified configuration file name
 *  @param[in/out] pConfig       : Pointer to the configuration structure
 *  @param[in/out] pConfigTable   : Pointer to the configuration map table
 *          
 *  @return bool : true if successful; otherwise false.
 *******************************************************************************
 */
bool readConfigFile(int8 *fileName, OvConfigCtrl *pConfig,
                    std::map<std::string,int32>* pConfigTable)
{
    int8 name[1000];
    int32 index;
    int32 value;

    std::ifstream file;
    file.open(fileName);
	
    if (!file) 
    {
        printf("Error in reading the configuration file: %s\n", fileName);
        return false;
    }

    std::string line;
	map<string, int32>::iterator itMap;

    while (std::getline(file, line))  
    {
        std::string temp = line;
		index = 0;
   	    sscanf_s(line.c_str(), "%s %d", name, &value);
		itMap = pConfigTable->find(name);
		if(itMap != pConfigTable->end())
		{
		    itMap->second = value;
		}
	
    }

	/**************************************************************************/
    /* Set user specified configuratin                                        */
	/**************************************************************************/
    encodeSetParam(pConfig, pConfigTable);

    file.close();
    return true;
}
