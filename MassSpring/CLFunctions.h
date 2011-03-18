#ifndef _CL_FUNCTIONS_H_
#define _CL_FUNCTIONS_H_

#include <CL\cl.h>

struct cl_system
{
  cl_context opencl_context;
  cl_command_queue command_queue;
  cl_kernel opencl_kernel;
  cl_kernel single_spring_kernel;
  cl_kernel batch_spring_kernel;
  cl_kernel euler_kernel;
};

extern struct cl_system cl_components;

void runTestKernel();
bool loadCLCodeFile(char *file, cl_context context, int num_devices, cl_device_id *devices,
                    int num_kernels, char **kernel_names, cl_kernel **kernels);
unsigned int readFile(char *path, char *buffer);
bool initOpenCL();
void tearDownCL();

#endif