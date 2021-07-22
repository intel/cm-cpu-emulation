/*===================== begin_copyright_notice ==================================

 Copyright (c) 2021, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>

#define OCL_CHECK(status)                                                      \
  if ((status) != 0) {                                                           \
    fprintf(stderr, "%s:%d: OCL error %d\n", __FILE__, __LINE__, (int)(status)); \
    exit(1);                                                                   \
  }

#ifndef SIMDW_DISPATCH
#define SIMDW_DISPATCH 1
#endif

int main() {

#ifdef SPV
#ifdef OCL
  fprintf(stderr, " OFFLINE OCL SPV \n");
#else
  fprintf(stderr, " OFFLINE CM SPV \n");
#endif
#else /*SPV*/
#ifdef OCL
  fprintf(stderr, " OFFLINE OCL BIN \n");
#else
  fprintf(stderr, " OFFLINE CM BIN \n");
#endif
#endif

  // Fetch the Platform and Device IDs; we only want one.
  cl_platform_id platform;
  cl_uint platforms;
  cl_int error = clGetPlatformIDs(1, &platform, &platforms);
  OCL_CHECK(error);
  cl_device_id device;
  cl_uint devices;
  OCL_CHECK(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, &devices));
  cl_context_properties properties[] = {CL_CONTEXT_PLATFORM,
                                        (cl_context_properties)platform, 0};
  // Note that nVidia's OpenCL requires the platform property
  cl_context context =
      clCreateContext(properties, 1, &device, NULL, NULL, &error);
  OCL_CHECK(error);
  cl_command_queue cq =
      clCreateCommandQueueWithProperties(context, device, nullptr, &error);
  OCL_CHECK(error);

#ifndef BINNAME
#error "Need to set BINNAME"
#else
  const char *fn = BINNAME;
#endif

  FILE *fp = fopen(fn, "rb");
  assert(fp);

  size_t binsize;
  fseek(fp, 0, SEEK_END);
  binsize = ftell(fp);
  rewind(fp);

  std::vector<uint8_t> progbin(binsize);
  fread(progbin.data(), 1, binsize, fp);
  fclose(fp);

  cl_int errNum = 0;

  constexpr int NPROGS = 1;
  uint8_t *progs[NPROGS] = {progbin.data()};
  size_t progssize[NPROGS] = {binsize};
  cl_program prog = clCreateProgramWithBinary(context,
                                              NPROGS,
                                              &device,
                                              progssize,
                                              (const unsigned char **)progs,
                                              &error,
                                              &errNum);
  OCL_CHECK(error);

  error = clBuildProgram(prog, 0, NULL,
#ifdef OCL
                         nullptr,
#else
                         "-cmc",
#endif
                         NULL, NULL);
  if (error != 0) {
    fprintf(stderr, " error= %d\n", (int)error);
    size_t log_length = 0;
    OCL_CHECK(clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG, 0, 0,
                                    &log_length));

    std::vector<char> log(log_length);

    OCL_CHECK(clGetProgramBuildInfo(prog, device, CL_PROGRAM_BUILD_LOG,
                                    log_length, &log[0], 0));

    fprintf(stderr,
            "Error happened during the build of OpenCL prog.\n"
            "Build log:\n %s\n",
            log.data());
    exit(0);
  }

  constexpr unsigned width = 4096;
  int hBufA[width];
  int hBufB[width];
  int hBufC[width], goldBufC[width];
  size_t bufsize = width * sizeof(int);

  for (size_t i = 0; i < width; ++i) {
    hBufA[i] = i + 1;
    hBufB[i] = 2 * (width + i + 1);
    goldBufC[i] = hBufA[i] + hBufB[i];
    hBufC[i] = -1;
  }

  // allocate buffer
  cl_mem dBufA =
      clCreateBuffer(context, CL_MEM_READ_ONLY, bufsize, NULL, &error);
  OCL_CHECK(error);
  cl_mem dBufB =
      clCreateBuffer(context, CL_MEM_READ_ONLY, bufsize, NULL, &error);
  OCL_CHECK(error);
  cl_mem dBufC =
      clCreateBuffer(context, CL_MEM_WRITE_ONLY, bufsize, NULL, &error);
  OCL_CHECK(error);

  // Send input data to OpenCL (async, don't alter the buffer!)
  OCL_CHECK(clEnqueueWriteBuffer(cq, dBufA, CL_TRUE, 0, bufsize, hBufA, 0, NULL,
                                 NULL));
  OCL_CHECK(clEnqueueWriteBuffer(cq, dBufB, CL_TRUE, 0, bufsize, hBufB, 0, NULL,
                                 NULL));
  OCL_CHECK(clEnqueueWriteBuffer(cq, dBufC, CL_TRUE, 0, bufsize, hBufC, 0, NULL,
                                 NULL));

  // get a handle and map parameters for the kernel
  cl_kernel k_vadd = clCreateKernel(prog, "vadd", &error);
  OCL_CHECK(error);
  OCL_CHECK(clSetKernelArg(k_vadd, 0, sizeof(cl_mem), &dBufA));
  OCL_CHECK(clSetKernelArg(k_vadd, 1, sizeof(cl_mem), &dBufB));
  OCL_CHECK(clSetKernelArg(k_vadd, 2, sizeof(cl_mem), &dBufC));

  // Perform the operation
#ifdef OCL
  size_t globalsize[1] = {width};
  size_t localsize[1] = {8};
#else
  size_t globalsize[1] = {SIMDW_DISPATCH * width / 32};
  size_t localsize[1] = {SIMDW_DISPATCH};
#endif
  fprintf(stderr, " globalsize= %d %d\n", (int)globalsize[0],
          (int)globalsize[1]);
  cl_event e = nullptr;
  OCL_CHECK(clEnqueueNDRangeKernel(cq, k_vadd, 1, NULL, globalsize, localsize,
                                   0, NULL, &e));
  error = clWaitForEvents(1, &e);
  OCL_CHECK(error);
  error = clReleaseEvent(e);
  OCL_CHECK(error);

  error =
      clEnqueueReadBuffer(cq, dBufC, CL_TRUE, 0, bufsize, hBufC, 0, NULL, &e);
  OCL_CHECK(clWaitForEvents(1, &e));
  OCL_CHECK(clReleaseEvent(e));

  OCL_CHECK(clEnqueueReadBuffer(cq, dBufC, CL_TRUE, 0, bufsize, hBufC, 0, NULL,
                                NULL));
  OCL_CHECK(clFinish(cq));

  bool fail = false;
  for (size_t i = 0; i < width; ++i) {
    fprintf(stderr, "i: %d  gold= %d comp=%d  %s\n", int(i), goldBufC[i],
            hBufC[i], (hBufC[i] != goldBufC[i] ? "FAIL" : ""));
    fail |= hBufC[i] != goldBufC[i];
  }
  fprintf(stderr, fail ? "FAIL\n" : "OK\n");
  return fail;
}
