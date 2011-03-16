#ifndef _CL_FUNCTIONS_H_
#define _CL_FUNCTIONS_H_

#include <CL\cl.h>

struct cl_system
{
  cl_context opencl_context;
  cl_command_queue command_queue;
  cl_kernel opencl_kernel;
};

extern struct cl_system cl_components;

void runTestKernel();
unsigned int readFile(char *path, char *buffer);
void initOpenCL();
void tearDownCL();

#endif