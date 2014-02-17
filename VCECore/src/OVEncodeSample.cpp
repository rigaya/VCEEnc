
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
* @file <OvEncodeSample.cpp>                          
*                                       
* @brief This file contains functions for encoding h264 video
*         
********************************************************************************
*/
/*******************************************************************************
*                             INCLUDE FILES                                    *
*******************************************************************************/
#include "OvEncodeSample.h"

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
bool getDevice(OVDeviceHandle *deviceHandle)
{
	bool status;

    /**************************************************************************/
    /* Get the Platform                                                       */
    /**************************************************************************/
	deviceHandle->platform = NULL;
	status = getPlatform(deviceHandle->platform);
	if(status == FALSE)
	{
		return false;
	}
    /**************************************************************************/
    /* STEP 1: Check for GPU                                                  */
    /**************************************************************************/
	cl_device_type dType = CL_DEVICE_TYPE_GPU;
	status = gpuCheck(deviceHandle->platform,&dType);
	if(status == FALSE)
	{
		return false;
	}
    /**************************************************************************/
    /* STEP 2: Get the number of devices                                      */ 
    /**************************************************************************/
    deviceHandle->numDevices = 0;
	deviceHandle->deviceInfo = NULL;

	/**************************************************************************/
	/* Memory for deviceInfo gets allocated inside the getDeviceInfo        */
	/* function depending on numDevices. This needs to be freed after the     */
	/* usage                                                                  */
	/**************************************************************************/
	status = getDeviceInfo(&deviceHandle->deviceInfo,&deviceHandle->numDevices);
	if(status == false)
	{
		return false;
	}
	return true;
}
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
                  OVDeviceHandle *deviceHandle)
{
	cl_device_id		 clDeviceID;
	bool status;
	cl_int err;
	*oveContext = NULL;
	/**************************************************************************/
    /* Create the CL Context - nothing works without a context handle.*/
    /**************************************************************************/
	/**************************************************************************/
	/* Create a variable for the open video encoder device id                 */
	/**************************************************************************/
 
    intptr_t properties[] =
    {
        CL_CONTEXT_PLATFORM, (cl_context_properties)deviceHandle->platform,
        0
    };

	/**************************************************************************/
    /* Create OpenCL context from device's id                                 */
	/**************************************************************************/
    clDeviceID = reinterpret_cast<cl_device_id>(deviceId);
    *oveContext  = clCreateContext(properties, 1, &clDeviceID, 0, 0, &err);
    if(*oveContext  == (cl_context)0) 
    {
        std::cout <<"\nCannot create cl_context" << std::endl;
        return false;
    }

    if(err != CL_SUCCESS) 
    {
        std::cout << "Error in clCreateContext " << err << std::endl; 
        return false;
    }
	if(deviceId == 0)
    {
        std::cout << "No suitable devices found!\n" << std::endl;
        return false;
    }
	/**************************************************************************/
	/* Read the device capabilities...                                        */
	/* Device capabilities should be used to validate against the             */
	/* configuration set by the user for the codec                            */
	/**************************************************************************/

	OVE_ENCODE_CAPS encodeCaps;
    OVE_ENCODE_CAPS_H264 encode_cap_full;
    encodeCaps.caps.encode_cap_full = (OVE_ENCODE_CAPS_H264 *)&encode_cap_full;
	status = getDeviceCap(*oveContext ,deviceId,&encodeCaps);
    
	if(!status)
    {
        std::cout << "OVEncodeGetDeviceCap failed!\n";
        return false;
    }
	
	return true;
}
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
	            uint32 deviceId,OvConfigCtrl *pConfig)
{
	cl_device_id clDeviceID = reinterpret_cast<cl_device_id>(deviceId);
	OVresult  res = 0;
	cl_int err;
	
	/**************************************************************************/
    /* Create an OVE Session                                                  */
    /**************************************************************************/
    encodeHandle->session = OVEncodeCreateSession(oveContext,  /**<Platform context */
                                    deviceId,               /**< device id */
                                    pConfig->encodeMode,    /**< encode mode */
                                    pConfig->profileLevel,  /**< encode profile */
                                    pConfig->pictFormat,    /**< encode format */
                                    pConfig->width,         /**< width */
                                    pConfig->height,        /**< height */
                                    pConfig->priority);     /**< encode task priority, ie. FOR POSSIBLY LOW LATENCY OVE_ENCODE_TASK_PRIORITY_LEVEL2 */
    if(encodeHandle->session == NULL) 
    {
        std::cout << "\nOVEncodeCreateSession failed.\n";
        return false;
    }
	/**************************************************************************/
    /* Configure the encoding engine based upon the config file               */
	/* specifications                                                         */
    /**************************************************************************/
	res = setEncodeConfig(encodeHandle->session,pConfig);
	if (!res)
    {
        printf("OVEncodeSendConfig returned error\n");
        return false;
    }

	/**************************************************************************/
    /* Create a command queue                                                 */
    /**************************************************************************/
   
    encodeHandle->clCmdQueue = clCreateCommandQueue((cl_context)oveContext,
		                               clDeviceID, 0, &err);
    if(err != CL_SUCCESS)
    {
        std::cout << "\nCreate command queue failed! Error :" << err << std::endl;
        return false;
    }

	/**************************************************************************/
	/* Make sure the surface is byte aligned                                  */
	/**************************************************************************/
    uint32 alignedSurfaceWidth = ((pConfig->width + (256 - 1)) & ~(256 - 1));
    uint32 alignedSurfaceHeight = (true) ? (pConfig->height + 31) & ~31 : 
									  (pConfig->height + 15) & ~15;
	/**************************************************************************/
	/* NV12 is 3/2                                                            */
	/**************************************************************************/
    int32 hostPtrSize = alignedSurfaceHeight * alignedSurfaceWidth * 3/2; 

    for(int32 i=0; i<MAX_INPUT_SURFACE; i++)
    {
        encodeHandle->inputSurfaces[i] = clCreateBuffer((cl_context)oveContext,
                                            CL_MEM_READ_WRITE,
                                            hostPtrSize, 
                                            NULL, 
                                            &err);
        if (err != CL_SUCCESS) 
        {
            std::cout << "\nclCreateBuffer returned error " << err;
            return false;
        }
    }
	return true;
}
#if FALSE
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
				 OvConfigCtrl *pConfig, OVprofile *profileCnt)
{	  
	bool status;
	cl_int err;
    uint32             numEventInWaitList = 0;
	
	OPMemHandle		            inputSurface;
	ove_session                 session=encodeHandle->session;

    OVresult  res = 0;
    OPEventHandle eventRunVideoProgram;
    
    /**************************************************************************/
    /* Create input buffer based upon the display surface                     */
    /**************************************************************************/
	/**************************************************************************/
	/* Make sure the surface is byte aligned                                  */
	/**************************************************************************/
    uint32 alignedSurfaceWidth = ((pConfig->width + (256 - 1)) & ~(256 - 1));
    uint32 alignedSurfaceHeight = (true) ? (pConfig->height + 31) & ~31 : 
									  (pConfig->height + 15) & ~15;
	
	/**************************************************************************/
	/* NV12 is 3/2                                                            */
	/**************************************************************************/
    int32 hostPtrSize = alignedSurfaceHeight * alignedSurfaceWidth * 3/2; 
    /**************************************************************************/
    /* Read the raw movie file                                                */
    /**************************************************************************/
	uint32 framecount=0;
	status = getFrameCount(inFile,&framecount,pConfig->width,pConfig->height);
	if(!status)
	{
		std::cout << "\nFailed to open file " << inFile << std::endl;
		return false;
	}
	/**************************************************************************/
    /* Don't read yet, we'll do it soon...frame by frame...                   */
	/**************************************************************************/
    FILE *fr = NULL;
    fopen_s(&fr, inFile, "rb");

    /**************************************************************************/
    /* Encode Picture                                                         */
    /**************************************************************************/

    /**************************************************************************/
    /* Setup the picture parameters                                           */
    /**************************************************************************/
    OVE_ENCODE_PARAMETERS_H264 pictureParameter;
    uint32 numEncodeTaskInputBuffers = 1; 
    OVE_INPUT_DESCRIPTION *encodeTaskInputBufferList  
            = (OVE_INPUT_DESCRIPTION *) malloc(sizeof(OVE_INPUT_DESCRIPTION) * 
				numEncodeTaskInputBuffers);

    /**************************************************************************/
    /* For the Query Output                                                   */
    /**************************************************************************/
    uint32 iTaskID;
    uint32 numTaskDescriptionsRequested = 1;
    uint32 numTaskDescriptionsReturned = 0;
	uint32 framenum = 0;
    OVE_OUTPUT_DESCRIPTION pTaskDescriptionList[1];

    /**************************************************************************/
    /* Okay, now it's time to read/encode frame by frame                      */
    /**************************************************************************/
	FILE *fw = NULL;
	if(fopen_s(&fw, outFile, "wb") || fw == NULL)
    {
		printf("Error opening the output file %s\n",outFile);
		return false;
    }
    
    while (framenum < framecount)
    {
        cl_event inMapEvt;
        cl_int   status;

		inputSurface = encodeHandle->inputSurfaces[framenum%MAX_INPUT_SURFACE];

		/**********************************************************************/
		/* Read the input file frame by frame                                 */
		/**********************************************************************/

        void* mapPtr = clEnqueueMapBuffer( encodeHandle->clCmdQueue,
                                            (cl_mem)inputSurface,
                                            CL_TRUE, //CL_FALSE,
                                            CL_MAP_READ | CL_MAP_WRITE,
                                            0,
                                            hostPtrSize,
                                            0,
                                            NULL,
                                            &inMapEvt,
                                            &status);

        status = clFlush(encodeHandle->clCmdQueue);
		waitForEvent(inMapEvt);
		status = clReleaseEvent(inMapEvt);
		
		/**********************************************************************/
        /* Read into the input surface buffer                                 */
		/**********************************************************************/
	    yuvToNV12(fr, pConfig->height, pConfig->width, alignedSurfaceWidth,
                  (int8 *)mapPtr);

        cl_event unmapEvent;
        status = clEnqueueUnmapMemObject(encodeHandle->clCmdQueue,
                                        (cl_mem)inputSurface,
                                        mapPtr,
                                        0,
                                        NULL,
                                        &unmapEvent);
		status = clFlush(encodeHandle->clCmdQueue);
		waitForEvent(unmapEvent);
		status = clReleaseEvent(unmapEvent);

		/**********************************************************************/
        /* use the input surface buffer as our Picture                        */
		/**********************************************************************/
		
        encodeTaskInputBufferList[0].bufferType = OVE_BUFFER_TYPE_PICTURE;
        encodeTaskInputBufferList[0].buffer.pPicture =  (OVE_SURFACE_HANDLE) inputSurface;

		/**********************************************************************/
        /* Setup the picture parameters                                       */
		/**********************************************************************/
        memset(&pictureParameter, 0, sizeof(OVE_ENCODE_PARAMETERS_H264));
        pictureParameter.size = sizeof(OVE_ENCODE_PARAMETERS_H264);
        pictureParameter.flags.value = 0;
        pictureParameter.flags.flags.reserved = 0;
        pictureParameter.insertSPS = (OVE_BOOL)(framenum == 0)?true:false;
        pictureParameter.pictureStructure = OVE_PICTURE_STRUCTURE_H264_FRAME;
        pictureParameter.forceRefreshMap = (OVE_BOOL)true;
        pictureParameter.forceIMBPeriod = 0;
        pictureParameter.forcePicType = OVE_PICTURE_TYPE_H264_NONE;

        framenum++;

		/**********************************************************************/
        /* encode a single picture.                                           */
		/**********************************************************************/

		/**********************************************************************/
		/* Start the timer before calling VCE for frame encode                */
		/**********************************************************************/
		captureTimeStart(profileCnt,0);
        res = OVEncodeTask(session,
                            numEncodeTaskInputBuffers,
                            encodeTaskInputBufferList,
                            &pictureParameter,
                            &iTaskID,
                            numEventInWaitList,
                            NULL,
                            &eventRunVideoProgram);
        if (!res) 
        {
            printf("\nOVEncodeTask returned error %fd", res);
            return false;
        }
		 
        /**********************************************************************/
        /* Wait for Encode session completes                                  */
        /**********************************************************************/

        err = clWaitForEvents(1, (cl_event *)&(eventRunVideoProgram));
        if(err != CL_SUCCESS) 
        {
            std::cout << "\nlWaitForEvents returned error " << err;
            return false;
        }
		captureTimeStop(profileCnt,0);
        /**********************************************************************/
        /* Query output                                                       */
        /**********************************************************************/

        numTaskDescriptionsReturned = 0;
        memset(pTaskDescriptionList,0,sizeof(OVE_OUTPUT_DESCRIPTION)*numTaskDescriptionsRequested);
        pTaskDescriptionList[0].size = sizeof(OVE_OUTPUT_DESCRIPTION);
		captureTimeStart(profileCnt,1);
        do
        {
            res = OVEncodeQueryTaskDescription(session,
                                                numTaskDescriptionsRequested,
                                                &numTaskDescriptionsReturned,
                                                pTaskDescriptionList);
            if (!res)
            {
                printf("\nOVEncodeQueryTaskDescription returned error %fd", err);
                return false;
            }
			
        } while(pTaskDescriptionList->status == OVE_TASK_STATUS_NONE);
		captureTimeStop(profileCnt,1);
        /**********************************************************************/
        /*  Write compressed frame to the output file                         */
        /**********************************************************************/

        for(uint32 i=0;i<numTaskDescriptionsReturned;i++)
        {
            if((pTaskDescriptionList[i].status == OVE_TASK_STATUS_COMPLETE) 
                && pTaskDescriptionList[i].size_of_bitstream_data > 0)
            {
                /**************************************************************/
                /* Write output data                                          */
                /**************************************************************/
			    fwrite(pTaskDescriptionList[i].bitstream_data,
						1, pTaskDescriptionList[i].size_of_bitstream_data,
						fw);
                
                res= OVEncodeReleaseTask( session, pTaskDescriptionList[i].taskID);
            }
        }
        if(eventRunVideoProgram)
		    clReleaseEvent((cl_event) eventRunVideoProgram);
    } /* End of read/encode/write loop*/
	fclose(fw);
	
    /**************************************************************************/
    /* Free memory resources now that we're through with them.                */
    /**************************************************************************/
    free(encodeTaskInputBufferList);
    fclose(fr);

    return true;
}
#endif
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
bool encodeClose(OVEncodeHandle *encodeHandle)
{
	bool oveErr;
	cl_int err;
	OPMemHandle *inputSurfaces = encodeHandle->inputSurfaces;

    for(int32 i=0; i<MAX_INPUT_SURFACE ;i++)
    {
        err = clReleaseMemObject((cl_mem)inputSurfaces[i]);
        if(err != CL_SUCCESS)
        {
            std::cout << "\nclReleaseMemObject returned error " << err;
            return false;
        }
    }

    err = clReleaseCommandQueue(encodeHandle->clCmdQueue);
	if(err != CL_SUCCESS)
    {
        std::cout << "Error releasing Command queue" << std::endl;
        return false;
    }

    oveErr = OVEncodeDestroySession(encodeHandle->session);
    if(!oveErr)
    {
        std::cout << "Error releasing OVE Session" << std::endl;
        return false;
    }
	return true;
}

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
bool encodeDestroy(OPContextHandle oveContext)
{
    cl_int err;
	
    if((cl_context)oveContext) 
    {
        err = clReleaseContext((cl_context)oveContext);
        if(err != CL_SUCCESS) 
        {
            std::cout << "Error releasing cl context" << std::endl;
            return false;
        }
    }
    return true;
}
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
void waitForEvent(cl_event inMapEvt)
{
	cl_int eventStatus = CL_QUEUED;
	cl_int   status;

	while(eventStatus != CL_COMPLETE)
	{
		status = clGetEventInfo(
	                inMapEvt, 
	                CL_EVENT_COMMAND_EXECUTION_STATUS, 
	                sizeof(cl_int),
	                &eventStatus,
	                NULL);
	}
}
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
bool getFrameCount(int8 *inFile,uint32 *frameCount,int32 width,
					 int32 height)
{
	FILE *rawDataFile = NULL;
	fopen_s(&rawDataFile, inFile, "rb");
	/**************************************************************************/
    /* get Filesize                                                           */
	/**************************************************************************/
    int64 filesize;
    if (rawDataFile)
    {
#ifdef _WIN32
        filesize = _filelengthi64(_fileno(rawDataFile));
#else
         fseek( rawDataFile, 0, SEEK_END );
         filesize = ftell( rawDataFile );
#endif
         fclose(rawDataFile);
    }
    else	
    {
         return false;
    }
	/**************************************************************************/
    /* get frame count                                                        */
	/**************************************************************************/
    float32 fHeight = 0.5;
    float32 fWidth = 0.5;
    float32 factor = 1+2*fHeight*fWidth;
    int32 framesize = width * height;
    *frameCount = (uint32) (filesize/(framesize*factor));
	return true;
}
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
bool setEncodeConfig(ove_session session, OvConfigCtrl *pConfig)
{
	uint32                    numOfConfigBuffers = 4;
    OVE_CONFIG                      configBuffers[4];
	OVresult res = 0;

	/**************************************************************************/
    /* send configuration values for this session                             */
	/**************************************************************************/
    configBuffers[0].config.pPictureControl     = &(pConfig->pictControl);
    configBuffers[0].configType                 = OVE_CONFIG_TYPE_PICTURE_CONTROL;
    configBuffers[1].config.pRateControl        = &(pConfig->rateControl);
    configBuffers[1].configType                 = OVE_CONFIG_TYPE_RATE_CONTROL;
    configBuffers[2].config.pMotionEstimation   = &(pConfig->meControl);
    configBuffers[2].configType                 = OVE_CONFIG_TYPE_MOTION_ESTIMATION;
    configBuffers[3].config.pRDO                = &(pConfig->rdoControl);
    configBuffers[3].configType                 = OVE_CONFIG_TYPE_RDO;
    res = OVEncodeSendConfig (session, numOfConfigBuffers, configBuffers);
    if (!res)
    {
        printf("OVEncodeSendConfig returned error\n");
        return false;
    }

    /**************************************************************************/
    /* Just verifying that the values have been set in the                    */
    /* encoding engine.                                                       */
    /**************************************************************************/
    OVE_CONFIG_PICTURE_CONTROL      pictureControlConfig;
    OVE_CONFIG_RATE_CONTROL         rateControlConfig;
    OVE_CONFIG_MOTION_ESTIMATION    meControlConfig;
    OVE_CONFIG_RDO                  rdoControlConfig;

	/**************************************************************************/
    /* get the picture control configuration.                                 */
	/**************************************************************************/
    memset(&pictureControlConfig, 0, sizeof(OVE_CONFIG_PICTURE_CONTROL));
    pictureControlConfig.size = sizeof(OVE_CONFIG_PICTURE_CONTROL);
    res = OVEncodeGetPictureControlConfig(session, &pictureControlConfig);

	/**************************************************************************/
    /* get the rate control configuration                                     */
	/**************************************************************************/
    memset(&rateControlConfig, 0, sizeof(OVE_CONFIG_RATE_CONTROL));
    rateControlConfig.size = sizeof(OVE_CONFIG_RATE_CONTROL);
    res = OVEncodeGetRateControlConfig(session, &rateControlConfig); 

	/**************************************************************************/
    /* get the MotionEstimation configuration                                 */
	/**************************************************************************/
    memset(&meControlConfig, 0, sizeof(OVE_CONFIG_MOTION_ESTIMATION));
    meControlConfig.size = sizeof(OVE_CONFIG_MOTION_ESTIMATION);
    res = OVEncodeGetMotionEstimationConfig(session, &meControlConfig); 

	/**************************************************************************/
    /* get the RDO configuration                                              */
	/**************************************************************************/
    memset(&rdoControlConfig, 0, sizeof(OVE_CONFIG_RDO));
    rdoControlConfig.size = sizeof(OVE_CONFIG_RDO);
    res = OVEncodeGetRDOControlConfig(session, &rdoControlConfig); 
	
	return(res);
}

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
				OVE_ENCODE_CAPS *encodeCaps)
{
    uint32 numCaps=1;
	bool status;
   
	/**************************************************************************/
    /* initialize the encode capabilities variable                            */
	/**************************************************************************/
    encodeCaps->EncodeModes          = OVE_AVC_FULL;
    encodeCaps->encode_cap_size      = sizeof(OVE_ENCODE_CAPS);
    encodeCaps->caps.encode_cap_full->max_picture_size_in_MB    = 0;
    encodeCaps->caps.encode_cap_full->min_picture_size_in_MB    = 0;
    encodeCaps->caps.encode_cap_full->num_picture_formats       = 0;
    encodeCaps->caps.encode_cap_full->num_Profile_level         = 0;
    encodeCaps->caps.encode_cap_full->max_bit_rate              = 0;
    encodeCaps->caps.encode_cap_full->min_bit_rate              = 0;
    encodeCaps->caps.encode_cap_full->supported_task_priority   = OVE_ENCODE_TASK_PRIORITY_NONE;

    for(int32 j=0; j<OVE_MAX_NUM_PICTURE_FORMATS_H264; j++)
        encodeCaps->caps.encode_cap_full->supported_picture_formats[j] = OVE_PICTURE_FORMAT_NONE;

    for(int32 j=0; j<OVE_MAX_NUM_PROFILE_LEVELS_H264; j++)
    {
        encodeCaps->caps.encode_cap_full->supported_profile_level[j].profile = 0;
        encodeCaps->caps.encode_cap_full->supported_profile_level[j].level   = 0;
    }

	/**************************************************************************/
	/* Get the device capabilities                                            */
	/**************************************************************************/
    status = OVEncodeGetDeviceCap(oveContext,
                                    oveDeviceID,
                                    encodeCaps->encode_cap_size,
                                    &numCaps,
                                    encodeCaps);
    return(status);
}
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
					 uint32 *numDevices)
{
	bool status;
	status = OVEncodeGetDeviceInfo(numDevices, 0);
    if(!status)
    {
        std::cout << "OVEncodeGetDeviceInfo failed!\n";
        return false;
    }
    else
    {
        if(*numDevices == 0)
        {
            std::cout << "No suitable devices found!\n" << std::endl;
            return false;
        }
    }
	/**************************************************************************/
    /* Get information about each device found                                */
	/**************************************************************************/
    *deviceInfo = new ovencode_device_info[*numDevices];
	memset(*deviceInfo,0,sizeof(ovencode_device_info)* (*numDevices));
    status = OVEncodeGetDeviceInfo(numDevices, *deviceInfo);
    if(!status)
    {
        std::cout << "OVEncodeGetDeviceInfo failed!\n";
        return false;
    }
	return true;
}
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
bool gpuCheck(cl_platform_id platform,cl_device_type* dType)
{
	cl_int err;
	cl_context_properties cps[3] = 
	{
		CL_CONTEXT_PLATFORM, 
		(cl_context_properties)platform, 
		0
	};

	cl_context context = clCreateContextFromType(cps,
												(*dType),
												NULL,
												NULL,
												&err);

	if(err == CL_DEVICE_NOT_FOUND)
	{
		*dType = CL_DEVICE_TYPE_CPU;
	}
	clReleaseContext(context);
	return true;
}
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
bool getPlatform(cl_platform_id &platform)
{
    cl_uint numPlatforms;
	cl_int err = clGetPlatformIDs(0, NULL, &numPlatforms);
    if (CL_SUCCESS != err)
	{
        fputs("clGetPlatformIDs() failed", stderr);
        return false;
    }
	/**************************************************************************/
    /* If there are platforms, make sure they are AMD.                        */
	/**************************************************************************/
    if (0 < numPlatforms) 
	{
        cl_platform_id* platforms = new cl_platform_id[numPlatforms];
        err = clGetPlatformIDs(numPlatforms, platforms, NULL);
        if (CL_SUCCESS != err) 
		{
            fputs("clGetPlatformIDs() failed", stderr);
            delete [] platforms;
            return false;
		}
		/**********************************************************************/
        /* Loop through all the platforms looking for an AMD system.          */
		/**********************************************************************/
        for (uint32 i = 0; i < numPlatforms; ++i) 
        {
            int8 pbuf[100];
            err = clGetPlatformInfo(platforms[i],
                                    CL_PLATFORM_VENDOR,
                                    sizeof(pbuf),
                                    pbuf,
                                    NULL);
			/******************************************************************/
            /* Stop at the first platform that is an AMD system.              */
			/******************************************************************/
            if (!strcmp(pbuf, "Advanced Micro Devices, Inc.")) 
            {
                platform = platforms[i];
                break;
            }
        }
        delete [] platforms;
    }

    if (NULL == platform) 
    {
        std::cout << "Couldn't find AMD platform, cannot proceed.\n";
        return false;
    }
	else
	{
		return true;
	}
}

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
               uint32 alignedSurfaceWidth, int8 *pBitstreamData)
{
    bool results=false;
    uint8  * pUplane = NULL;
    uint8  * pVplane = NULL;
    uint8  * pUVrow = NULL;
    uint32     uiUVSize = (uiWidth*uiHeight)>>2;
    uint32     uiHalfHeight = uiHeight>>1;
    uint32     uiHalfWidth  = uiWidth>>1;

    pUplane  = (uint8 *) malloc(uiUVSize);
    pVplane  = (uint8 *) malloc(uiUVSize);
    pUVrow   = (uint8 *) malloc(uiWidth);

    /**************************************************************************/
    /* read one NV12 frame                                                    */
    /**************************************************************************/
    if (fr)
    {
        /**********************************************************************/
        /* Y plane                                                            */
        /**********************************************************************/
        uint8* pBuf = (uint8 *) pBitstreamData;

        for (uint32 h=0; h<uiHeight; h++)
        {
            if (uiWidth != fread(pBuf, sizeof(int8), uiWidth, fr))
				return false;
            pBuf += alignedSurfaceWidth;
        }

        /**********************************************************************/
        /* Align the Y plane before adding the U & V data.                    */
        /* pBuf = (uint8 *)pBitstreamData + alignedSurfaceWidth *            */
        /* alignedSurfaceHeight;                                              */
        /**********************************************************************/
        /**********************************************************************/
        /* UV plane                                                           */
        /**********************************************************************/
        uint32 chromaWidth = uiHalfWidth;
        if (   uiUVSize != fread(pUplane, sizeof(int8), uiUVSize, fr)
			|| uiUVSize != fread(pVplane, sizeof(int8), uiUVSize, fr))
				return false;
        for (uint32 h=0; h<uiHalfHeight; h++)
        {
            for (uint32 i = 0; i < chromaWidth; ++i)
            {
                pUVrow[i*2]     = pUplane[chromaWidth * h + i];
                pUVrow[i*2 + 1] = pVplane[chromaWidth * h + i];
            }
            memcpy(pBuf, pUVrow, uiWidth);
            pBuf += alignedSurfaceWidth;
        }

        /**********************************************************************/
        /* we have been successful in reading the file                        */
        /**********************************************************************/
        results = true;
    }
    else
    {
        return results;
    }
	
	free(pUplane);
	free(pVplane);
	free(pUVrow);

    return results;
}
