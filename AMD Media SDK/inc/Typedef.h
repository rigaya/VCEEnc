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
 * @file <Typedef.h>                          
 *                                       
 * @brief Contains typedefs for data types
 *         
 ********************************************************************************
 */
#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

/******************************************************************************/
/* Define sized-based typedefs up to 32-bits.                                 */
/******************************************************************************/
typedef char int8;
typedef signed char sint8;
typedef unsigned char uint8;

typedef signed short int16;
typedef unsigned short uint16;

typedef signed int int32;
typedef unsigned int uint32;

typedef float float32;
typedef double float64;

/******************************************************************************/
/* Define 64-bit typedefs, depending on the compiler and operating system.    */
/******************************************************************************/
#ifdef __GNUC__ 
typedef long long int64;
typedef unsigned long long uint64;

#else  /* not __GNUC__ */
#ifdef _WIN32 
typedef __int64 int64;
typedef unsigned __int64 uint64;

#else  /* not _WIN32 */
#error Unsupported compiler and/or operating system 
#endif /* end ifdef _WIN32 */

#endif /* end ifdef __GNUC__ */

/******************************************************************************/
/* Define other generic typedefs.                                             */
/******************************************************************************/
typedef unsigned long ulong;

/******************************************************************************/
/* End of _STANDARD_TYPEDEFS_DEFINED_                                         */
/******************************************************************************/
#endif 
