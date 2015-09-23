/*******************************************************************************
 Copyright ｩ2014 Advanced Micro Devices, Inc. All rights reserved.

 Redistribution and use in source and binary forms, with or without 
 modification, are permitted provided that the following conditions are met:

 1   Redistributions of source code must retain the above copyright notice, 
 this list of conditions and the following disclaimer.
 2   Redistributions in binary form must reproduce the above copyright notice, 
 this list of conditions and the following disclaimer in the 
 documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/
/**  
 ********************************************************************************
 * @file <ErrorCodes.h>                          
 *                                       
 * @brief Contains error code macros
 *         
 ********************************************************************************
 */
#ifndef _ERRORCODES_H_
#define _ERRORCODES_H_

#define ERR_NO_ERROR                     0
#define ERR_COM_INITILIZATION_FAILED     0x10000
#define ERR_MFT_INITILIZATION_FAILED     0x20000
#define ERR_SESSOIN_INITIALIZATION_FAILED 0x30000
#define ERR_BUILDING_TOPOLOGY_FAILED     0x40000
#define ERR_TRANSCODING_FAILED           0x50000
#define ERR_VIDEOSTREAM_NOTFOUND         0x60000

#define ERR_NO_ERROR                     0
#define ERR_INITILIZATION_FAILED         0x0001
#define ERR_SETUP_FAILED                 0x0002
#define ERR_RUN_FAILED                   0X0003
#define ERR_DESTROY_FAILED               0X0004

/******************************************************************************
 * VCE-DEM Error codes                                                        *
 ******************************************************************************/
#define ERR_REMOTEDISPLAYCREATE_FAILED   1
#define ERR_CHANGERESOLUTION_FAILED      2
#define ERR_ENCODERCREATE_FAILED         3
#define ERR_ENCODERCONFIG_FAILED         4

#endif

