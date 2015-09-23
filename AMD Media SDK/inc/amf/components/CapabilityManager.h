/*
***************************************************************************************************
*
* Copyright (c) 2014 Advanced Micro Devices, Inc. (unpublished)
*
*  All rights reserved.  This notice is intended as a precaution against inadvertent publication and
*  does not imply publication or any waiver of confidentiality.  The year included in the foregoing
*  notice is the year of creation of the work.
*
***************************************************************************************************
*/
/**
***************************************************************************************************
* @file  CapabilityManager.h
* @brief AMF Component Capability Manager
***************************************************************************************************
*/

#pragma once

#include "Interface.h"

#include "VideoDecoderCaps.h"
#include "VideoEncoderCaps.h"
#include "VideoConverterCaps.h"

namespace amf
{
    /*  This is the main interface to Capability Manager
    */
    class AMFCapabilityManager : public virtual AMFInterface
    {
    public:
        AMF_DECLARE_IID(0x8e2fe2e6, 0xb88e, 0x49b6, 0xbb, 0x7e, 0x9e, 0x53, 0xf8, 0xb1, 0xb3, 0x95)

        /*  Capability manager needs to be initialized for a specific device or NULL for the default device.
            Unlike a context, it can only be initialized for one device at a time, subsequent calls to Init*()
            will result in error. To reinitialize for a different device either create a different instance of
            AMFCapabilityManager or call Terminate(). 
        */  
        virtual AMF_RESULT AMF_STD_CALL InitDX9(void* pDX9Device) = 0;
        virtual AMF_RESULT AMF_STD_CALL InitDX11(void* pDX11Device, AMF_DX_VERSION dxVersionRequired = AMF_DX11_0) = 0;
        virtual AMF_RESULT AMF_STD_CALL InitXV(void* pXVDevice) = 0;
        virtual AMF_RESULT AMF_STD_CALL InitGralloc(void* pGrallocDevice) = 0;

        virtual AMF_RESULT AMF_STD_CALL Terminate() = 0;

        //  Get decoder capabilities for a specific codec:
        virtual AMF_RESULT AMF_STD_CALL GetDecoderCaps(const wchar_t *componentID, AMFDecoderCaps** caps) = 0;

        //  Get encoder capabilities for a specific codec:
        virtual AMF_RESULT AMF_STD_CALL GetEncoderCaps(const wchar_t *componentID, AMFEncoderCaps** caps) = 0;

        //  Get encoder capabilities for a specific codec:
        virtual AMF_RESULT AMF_STD_CALL GetConverterCaps(AMFVideoConverterCaps** caps) = 0;
    };
    typedef AMFInterfacePtr_T<AMFCapabilityManager> AMFCapabilityManagerPtr;
}


extern "C"
{
    // Creates an instance of capability manager:
    AMF_CORE_LINK AMF_RESULT AMF_CDECL_CALL AMFCreateCapsManager(amf::AMFCapabilityManager** ppCapsManager);
}

