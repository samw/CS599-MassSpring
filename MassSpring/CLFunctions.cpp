#include "CLFunctions.h"
#include "MassSpring.h"

struct cl_system cl_components;

void runTestKernel()
{
  float value[4];
  float timestep;
  glFinish();
  clEnqueueAcquireGLObjects(cl_components.command_queue, 1, &(simulation.position), 0, NULL, NULL);

  ////Test movement to test reading and writing the opengl vertex buffer through open cl, no kernel used here
  //clEnqueueReadBuffer(simulation.command_queue, simulation.position, true,
  //                    0 * sizeof(float), 4 * sizeof(float), &value,
  //                    0, NULL, NULL);
  //value[1] += 0.001;
  //value[2] += 0.001;
  //clEnqueueWriteBuffer(simulation.command_queue, simulation.position, true,
  //                     0 * sizeof(float), 4 * sizeof(float), &value,
  //                     0, NULL, NULL);

  value[0] = 1.0;
  value[1] = 1.0;
  value[2] = 0.0;
  value[3] = 0.0;
  timestep = 0.01;
  size_t worksize[1] = {simulation.num_points};
  clSetKernelArg(cl_components.opencl_kernel, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.opencl_kernel, 1, sizeof(float[4]), value);
  clSetKernelArg(cl_components.opencl_kernel, 2, sizeof(float), &timestep);
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.opencl_kernel, 1, NULL, worksize, NULL, 0, NULL, NULL);

  clEnqueueReleaseGLObjects(cl_components.command_queue, 1, &(simulation.position), 0, NULL, NULL);
  clFinish(cl_components.command_queue);
}


unsigned int readFile(char *path, char *buffer)
{
  unsigned int size;
  FILE *input;
  input = fopen(path, "rb");
  if(!input) return 0;
  fseek(input, 0, SEEK_END);
  size = ftell(input);
  rewind(input);
  size = fread(buffer, sizeof(char), size, input);
  fclose(input);
  return size;
}

void initOpenCL()
{
  cl_int error;
  char buffer[100];

  //Enumerate Platforms
  cl_uint max_platforms = 10;
  cl_uint num_platforms = 0;
  cl_platform_id * platforms;
  platforms = new cl_platform_id[max_platforms];
  error = clGetPlatformIDs(max_platforms, platforms, &num_platforms);
  if(num_platforms == 0)
  {
    printf("no open cl devices");
    exit(100);
  }
  printf("OpenCL Platforms:\n");
  for(unsigned int i = 0; i < num_platforms; i++)
  {
    error = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 100, buffer, NULL);
    printf(buffer);
    printf(": ");
    error = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, 100, buffer, NULL);
    printf(buffer);
    printf(" (%d) \n", platforms[i]);
  }

  //Enumerate Devices
  cl_uint max_devices = 10;
  cl_uint num_devices = 0;
  cl_device_id * devices;
  devices = new cl_device_id[max_devices];
  error = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, max_devices, devices, &num_devices);
  if(num_devices == 0)
  {
    printf("no open cl devices");
    exit(100);
  }
  printf("Devices on Platform %d:\n", platforms[0]);
  for(unsigned int i = 0; i < num_platforms; i++)
  {
    error = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 100, buffer, NULL);
    printf(buffer);
    printf("\n");
  }

  //Create CL context using platform 0
  cl_context_properties context_properties[7] =
  {
    CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
    CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
    CL_CONTEXT_PLATFORM,(cl_context_properties)platforms[0],
    0
  };
  cl_components.opencl_context = clCreateContext(context_properties, 1, devices, NULL, NULL, &error);
  if(error)
  {
    printf("OpenCL Error: %d\n", error);
  }
  cl_components.command_queue = clCreateCommandQueue(cl_components.opencl_context, devices[0], NULL, &error);
  if(error)
  {
    printf("OpenCL Error: %d\n", error);
  }

  //Create OpenCL buffer object attached to OpenGL vertex buffer object
  simulation.position = clCreateFromGLBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE, simulation.position_buffer, NULL);

  //Compile source and create kernel
  cl_program moveprogram = NULL;
  char source[1000];
  size_t source_size = readFile("move.cl", source);
  if(source_size == 0) exit(-1);
  const char *sourcelist[1] = {source};
  moveprogram = clCreateProgramWithSource(cl_components.opencl_context, 1, sourcelist, &source_size, NULL);
  error = clBuildProgram(moveprogram, 1, devices, "", NULL, NULL);
  if(error)
  {
    char buildinfo[1000];
    size_t outputsize;
    clGetProgramBuildInfo(moveprogram, devices[0], CL_PROGRAM_BUILD_LOG, 1000, buildinfo, &outputsize);
    printf("Compile Error for moveprogram: %d\n", error);
    fwrite(buildinfo, min(outputsize, 1000), 1, stdout); 
  }
  //Create kernel from program
  cl_components.opencl_kernel = clCreateKernel(moveprogram, "move_vertex", &error);  if(error)
  {
    printf("Kernel Error: %d\n", error);
  }

  delete platforms;
  delete devices;
}

void tearDownCL()
{
  clReleaseMemObject(simulation.position);
  clReleaseContext(cl_components.opencl_context);
}
