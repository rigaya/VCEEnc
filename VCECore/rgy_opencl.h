// -----------------------------------------------------------------------------------------
// NVEnc by rigaya
// -----------------------------------------------------------------------------------------
//
// The MIT License
//
// Copyright (c) 2014-2016 rigaya
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
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// ------------------------------------------------------------------------------------------

#ifndef __RGY_OPENCL_H__
#define __RGY_OPENCL_H__

#include "rgy_version.h"

#if ENABLE_OPENCL

#include <CL/opencl.h>
#include <vector>
#include <array>
#include <memory>
#include "rgy_osdep.h"
#include "rgy_log.h"

#ifndef CL_EXTERN
#define CL_EXTERN extern
#endif

CL_EXTERN cl_int (CL_API_CALL* f_clGetPlatformIDs)(cl_uint num_entries, cl_platform_id *platforms, cl_uint *num_platforms);
CL_EXTERN cl_int (CL_API_CALL* f_clGetPlatformInfo) (cl_platform_id platform, cl_platform_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clGetDeviceIDs) (cl_platform_id platform, cl_device_type device_type, cl_uint num_entries, cl_device_id *devices, cl_uint *num_devices);
CL_EXTERN cl_int (CL_API_CALL* f_clGetDeviceInfo) (cl_device_id device, cl_device_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);

CL_EXTERN cl_context (CL_API_CALL* f_clCreateContext) (const cl_context_properties * properties, cl_uint num_devices, const cl_device_id * devices, void (CL_CALLBACK * pfn_notify)(const char *, const void *, size_t, void *), void * user_data, cl_int * errcode_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clReleaseContext) (cl_context context);
CL_EXTERN cl_command_queue (CL_API_CALL* f_clCreateCommandQueue)(cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_int * errcode_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clReleaseCommandQueue) (cl_command_queue command_queue);

CL_EXTERN cl_program(CL_API_CALL* f_clCreateProgramWithSource) (cl_context context, cl_uint count, const char **strings, const size_t *lengths, cl_int *errcode_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clBuildProgram) (cl_program program, cl_uint num_devices, const cl_device_id *device_list, const char *options, void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data), void* user_data);
CL_EXTERN cl_int (CL_API_CALL* f_clGetProgramBuildInfo) (cl_program program, cl_device_id device, cl_program_build_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clGetProgramInfo)(cl_program program, cl_program_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clReleaseProgram) (cl_program program);

CL_EXTERN cl_mem (CL_API_CALL* f_clCreateBuffer) (cl_context context, cl_mem_flags flags, size_t size, void *host_ptr, cl_int *errcode_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clReleaseMemObject) (cl_mem memobj);
CL_EXTERN cl_kernel (CL_API_CALL* f_clCreateKernel) (cl_program program, const char *kernel_name, cl_int *errcode_ret);
CL_EXTERN cl_int (CL_API_CALL* f_clReleaseKernel) (cl_kernel kernel);
CL_EXTERN cl_int (CL_API_CALL* f_clSetKernelArg) (cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void *arg_value);
CL_EXTERN cl_int (CL_API_CALL* f_clEnqueueNDRangeKernel)(cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, const size_t *global_work_offset, const size_t *global_work_size, const size_t *local_work_size, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event * event);
CL_EXTERN cl_int(CL_API_CALL* f_clEnqueueTask) (cl_command_queue command_queue, cl_kernel kernel, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);

CL_EXTERN cl_int(CL_API_CALL* f_clEnqueueReadBuffer) (cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_read, size_t offset, size_t size, void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
CL_EXTERN cl_int(CL_API_CALL* f_clEnqueueWriteBuffer) (cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_write, size_t offset, size_t size, const void *ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
CL_EXTERN cl_int(CL_API_CALL *f_clEnqueueCopyBuffer)(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer, size_t src_offset, size_t dst_offset, size_t size, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);
CL_EXTERN cl_int(CL_API_CALL *f_clEnqueueCopyBufferRect)(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_buffer, const size_t *src_origin, const size_t *dst_origin, const size_t *region, size_t src_row_pitch, size_t src_slice_pitch, size_t dst_row_pitch, size_t dst_slice_pitch, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event);

CL_EXTERN cl_int(CL_API_CALL *f_clWaitForEvents)(cl_uint num_events, const cl_event *event_list);
CL_EXTERN cl_int(CL_API_CALL *f_clGetEventInfo)(cl_event event, cl_event_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
CL_EXTERN cl_event(CL_API_CALL *f_clCreateUserEvent)(cl_context context, cl_int *errcode_ret);
CL_EXTERN cl_int(CL_API_CALL *f_clRetainEvent)(cl_event event);
CL_EXTERN cl_int(CL_API_CALL *f_clReleaseEvent)(cl_event event);
CL_EXTERN cl_int(CL_API_CALL *f_clSetUserEventStatus)(cl_event event, cl_int execution_status);
CL_EXTERN cl_int(CL_API_CALL *f_clGetEventProfilingInfo)(cl_event event, cl_profiling_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);

CL_EXTERN cl_int(CL_API_CALL *f_clFlush)(cl_command_queue command_queue);
CL_EXTERN cl_int(CL_API_CALL *f_clFinish)(cl_command_queue command_queue);

#define clGetPlatformIDs f_clGetPlatformIDs
#define clGetPlatformInfo f_clGetPlatformInfo
#define clGetDeviceIDs f_clGetDeviceIDs
#define clGetDeviceInfo f_clGetDeviceInfo

#define clCreateContext f_clCreateContext
#define clReleaseContext f_clReleaseContext
#define clCreateCommandQueue f_clCreateCommandQueue
#define clReleaseCommandQueue f_clReleaseCommandQueue

#define clCreateProgramWithSource f_clCreateProgramWithSource
#define clBuildProgram f_clBuildProgram
#define clGetProgramBuildInfo f_clGetProgramBuildInfo
#define clGetProgramInfo f_clGetProgramInfo
#define clReleaseProgram f_clReleaseProgram

#define clCreateBuffer f_clCreateBuffer
#define clReleaseMemObject f_clReleaseMemObject
#define clCreateKernel f_clCreateKernel
#define clReleaseKernel f_clReleaseKernel
#define clSetKernelArg f_clSetKernelArg
#define clEnqueueNDRangeKernel f_clEnqueueNDRangeKernel
#define clEnqueueTask f_clEnqueueTask

#define clEnqueueReadBuffer f_clEnqueueReadBuffer
#define clEnqueueWriteBuffer f_clEnqueueWriteBuffer
#define clEnqueueCopyBuffer f_clEnqueueCopyBuffer
#define clEnqueueCopyBufferRect f_clEnqueueCopyBufferRect

#define clWaitForEvents f_clWaitForEvents
#define clGetEventInfo f_clGetEventInfo
#define clCreateUserEvent f_clCreateUserEvent
#define clRetainEvent f_clRetainEvent
#define clReleaseEvent f_clReleaseEvent
#define clSetUserEventStatus f_clSetUserEventStatus
#define clGetEventProfilingInfo f_clGetEventProfilingInfo

#define clFlush f_clFlush
#define clFinish f_clFinish

struct RGYOpenCLDeviceInfo {
    cl_device_type type;
    int vendor_id;
    int max_compute_units;
    int max_clock_frequency;
    int max_samplers;
    uint64_t global_mem_size;
    size_t profiling_timer_resolution;
    std::string name;
    std::string vendor;
    std::string driver_version;
    std::string profile;
    std::string version;
    std::string extensions;
};

class RGYOpenCLDevice {
public:
    RGYOpenCLDevice(cl_device_id device);
    virtual ~RGYOpenCLDevice() {};
    RGYOpenCLDeviceInfo info();
    tstring infostr();
protected:
    cl_device_id m_device;
};

struct RGYOpenCLPlatformInfo {
    std::string profile;
    std::string version;
    std::string name;
    std::string vendor;
    std::string extension;
};

class RGYOpenCLPlatform {
public:
    RGYOpenCLPlatform(cl_platform_id platform, shared_ptr<RGYLog> pLog);
    virtual ~RGYOpenCLPlatform() {};
    cl_int createDeviceList(cl_device_type device_type);
    cl_platform_id get() const { return m_platform; };
    std::vector<cl_device_id>& devs() { return m_devices; };
    cl_device_id dev(int idx) { return m_devices[idx]; };
    const std::vector<cl_device_id>& devs() const { return m_devices; };
    void setDevs(std::vector<cl_device_id> &devs) { m_devices = devs; };
    bool isVendor(const char *vendor);
protected:
    RGYOpenCLPlatformInfo info();

    cl_platform_id m_platform;
    std::vector<cl_device_id> m_devices;
    shared_ptr<RGYLog> m_pLog;
};

struct RGYWorkSize {
    size_t w[3];
    RGYWorkSize() {
        w[0] = w[1] = w[2] = 1;
    }
    RGYWorkSize(size_t x) {
        w[0] = x;
        w[1] = w[2] = 1;
    };
    RGYWorkSize(size_t x, size_t y) {
        w[0] = x;
        w[1] = y;
        w[2] = 1;
    }
    RGYWorkSize(size_t x, size_t y, size_t z) {
        w[0] = x;
        w[1] = y;
        w[2] = z;
    }
    const size_t *operator()() const {
        return &w[0];
    }
};

class RGYOpenCLKernelLauncher {
public:
    RGYOpenCLKernelLauncher(cl_kernel kernel, std::string kernelName, cl_command_queue queue, const RGYWorkSize &local, const RGYWorkSize &global, shared_ptr<RGYLog> pLog);
    virtual ~RGYOpenCLKernelLauncher() {};

    cl_int launch(std::vector<void *> arg_ptrs = std::vector<void *>(), std::vector<size_t> arg_size = std::vector<size_t>()) const;

    template <typename... ArgTypes>
    cl_int operator()(ArgTypes... args) const {
        return launch(args...);
    }
    template <typename... ArgTypes>
    cl_int launch(ArgTypes... args) const {
        return this->launch(
            std::vector<void *>({ (void *)&args... }),
            std::vector<size_t>({ sizeof(args)... })
        );
    }
protected:
    cl_kernel m_kernel;
    std::string m_kernelName;
    cl_command_queue m_queue;
    RGYWorkSize m_local;
    RGYWorkSize m_global;
    shared_ptr<RGYLog> m_pLog;
};

class RGYOpenCLKernel {
public:
    RGYOpenCLKernel() : m_kernel(), m_kernelName(), m_pLog() {};
    RGYOpenCLKernel(cl_kernel kernel, std::string kernelName, shared_ptr<RGYLog> pLog);
    virtual ~RGYOpenCLKernel() {};
    RGYOpenCLKernelLauncher config(cl_command_queue queue, const RGYWorkSize& local, const RGYWorkSize& global);
protected:
    cl_kernel m_kernel;
    std::string m_kernelName;
    shared_ptr<RGYLog> m_pLog;
};

class RGYOpenCLProgram {
public:
    RGYOpenCLProgram(cl_program program, shared_ptr<RGYLog> pLog);
    virtual ~RGYOpenCLProgram() {};

    RGYOpenCLKernel kernel(const char *kernelName);
protected:
    cl_program m_program;
    shared_ptr<RGYLog> m_pLog;
};

typedef std::unique_ptr<std::remove_pointer<cl_context>::type, decltype(clReleaseContext)> unique_context;
typedef std::unique_ptr<std::remove_pointer<cl_command_queue>::type, decltype(clReleaseCommandQueue)> unique_queue;

class RGYOpenCLContext {
public:
    RGYOpenCLContext(unique_ptr<RGYOpenCLPlatform> platform, shared_ptr<RGYLog> pLog);
    virtual ~RGYOpenCLContext();

    cl_int createContext();
    cl_context context() const { return m_context.get(); };
    cl_command_queue queue(int idx=0) const { return m_queue[idx].get(); };
    RGYOpenCLPlatform *platform() const { return m_platform.get(); };

    unique_ptr<RGYOpenCLProgram> build(const std::string& source);
    unique_ptr<RGYOpenCLProgram> buildFile(const tstring &filename);
    unique_ptr<RGYOpenCLProgram> buildResource(const TCHAR *name, const TCHAR *type);
protected:
    unique_ptr<RGYOpenCLProgram> build(const char *data, const size_t size);

    unique_ptr<RGYOpenCLPlatform> m_platform;
    unique_context m_context;
    std::vector<unique_queue> m_queue;
    shared_ptr<RGYLog> m_pLog;
};

class RGYOpenCL {
public:
    static HMODULE openCLHandle;
    static bool openCLCrush;

    RGYOpenCL();
    RGYOpenCL(shared_ptr<RGYLog> pLog);
    virtual ~RGYOpenCL();

    std::vector<unique_ptr<RGYOpenCLPlatform>> getPlatforms(const char *vendor = nullptr);
protected:
    shared_ptr<RGYLog> m_pLog;
};

int initOpenCLGlobal();

#endif //ENABLE_OPENCL

#endif //__RGY_OPENCL_H__
