// -----------------------------------------------------------------------------------------
//     VCEEnc by rigaya
// -----------------------------------------------------------------------------------------
// The MIT License
//
// Copyright (c) 2014-2017 rigaya
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// IABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// ------------------------------------------------------------------------------------------

#ifndef _CPU_INFO_H_
#define _CPU_INFO_H_

#include <stdint.h>
#include <tchar.h>

typedef struct cache_info_t {
    uint16_t count;
    uint8_t  level;
    uint8_t  associativity;
    uint16_t linesize;
    uint16_t type;
    uint32_t size;
} cache_info_t;

typedef struct {
    uint32_t nodes;
    uint32_t physical_cores;
    uint32_t logical_cores;
    uint32_t max_cache_level;
    cache_info_t caches[4];
} cpu_info_t;


bool get_cpu_info(cpu_info_t *cpu_info);

int getCPUInfo(TCHAR *buffer, size_t nSize);

template <size_t size>
int inline getCPUInfo(TCHAR(&buffer)[size]) {
    return getCPUInfo(buffer, size);
}

double getCPUDefaultClock();
double getCPUMaxTurboClock(unsigned int num_thread);

typedef struct PROCESS_TIME {
    UINT64 creation, exit, kernel, user;
} PROCESS_TIME;

BOOL GetProcessTime(PROCESS_TIME *time);
BOOL GetProcessTime(HANDLE hProcess, PROCESS_TIME *time);
double GetProcessAvgCPUUsage(HANDLE hProcess, PROCESS_TIME *start = nullptr);
double GetProcessAvgCPUUsage(PROCESS_TIME *start);

#endif //_CPU_INFO_H_
