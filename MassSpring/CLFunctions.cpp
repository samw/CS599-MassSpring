#include "CLFunctions.h"
#include "MassSpring.h"

struct cl_system cl_components;

void runTestKernel()
{
  //float value[4];
  float rest;
  float spring;
  float damp;
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

  ////Test movement using the simple move.cl kernel
  //value[0] = 1.0;
  //value[1] = 1.0;
  //value[2] = 0.0;
  //value[3] = 0.0;
  //timestep = 0.01;
  //size_t worksize[1] = {simulation.num_points};
  //clSetKernelArg(cl_components.opencl_kernel, 0, sizeof(cl_uint), &(simulation.position));
  //clSetKernelArg(cl_components.opencl_kernel, 1, sizeof(float[4]), value);
  //clSetKernelArg(cl_components.opencl_kernel, 2, sizeof(float), &timestep);
  //clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.opencl_kernel, 1, NULL, worksize, NULL, 0, NULL, NULL);


  //zero accelerations for first two points
  cl_float accelerations[8] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
  clEnqueueWriteBuffer(cl_components.command_queue, simulation.acceleration, true,
                       0 * sizeof(float), 8 * sizeof(cl_float), &accelerations, 0, NULL, NULL);
  //Calculate interaction for a single spring between first two points
  size_t springworksize[1] = {1};
  rest = 0.25;
  spring = 100.0;
  damp = 2.0;
  cl_int pointa = 0;
  cl_int pointb = 1;
  clSetKernelArg(cl_components.single_spring_kernel, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.single_spring_kernel, 1, sizeof(cl_uint), &(simulation.velocity));
  clSetKernelArg(cl_components.single_spring_kernel, 2, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.single_spring_kernel, 3, sizeof(cl_int), &pointa);
  clSetKernelArg(cl_components.single_spring_kernel, 4, sizeof(cl_int), &pointb);
  clSetKernelArg(cl_components.single_spring_kernel, 5, sizeof(cl_float), &rest);
  clSetKernelArg(cl_components.single_spring_kernel, 6, sizeof(cl_float), &spring);
  clSetKernelArg(cl_components.single_spring_kernel, 7, sizeof(cl_float), &damp);
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.single_spring_kernel,
                         1, NULL, springworksize, NULL, 0, NULL, NULL);
  clFinish(cl_components.command_queue);

  size_t eulerworksize[1] = {simulation.num_points};
  timestep = 0.01;
  clSetKernelArg(cl_components.euler_kernel, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.euler_kernel, 1, sizeof(cl_uint), &(simulation.velocity));
  clSetKernelArg(cl_components.euler_kernel, 2, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.euler_kernel, 3, sizeof(cl_float), &timestep);
  clFinish(cl_components.command_queue);
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.euler_kernel,
                         1, NULL, eulerworksize, NULL, 0, NULL, NULL);


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

  //Compile source and create kernel this is to use the old test kernel move.cl
  size_t source_size;
  char source[10000];
  const char *sourcelist[1] = {source};
  cl_program moveprogram = NULL;
  source_size = readFile("move.cl", source);
  if(source_size == 0) exit(-1);
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

  //Compile source and create kernel
  cl_program springprogram = NULL;
  source_size = readFile("spring_kernel.cl", source);
  if(source_size == 0) exit(-1);
  springprogram = clCreateProgramWithSource(cl_components.opencl_context, 1, sourcelist, &source_size, NULL);
  error = clBuildProgram(springprogram, 1, devices, "", NULL, NULL);
  if(error)
  {
    char buildinfo[1000];
    size_t outputsize;
    clGetProgramBuildInfo(springprogram, devices[0], CL_PROGRAM_BUILD_LOG, 1000, buildinfo, &outputsize);
    printf("Compile Error for moveprogram: %d\n", error);
    fwrite(buildinfo, min(outputsize, 1000), 1, stdout); 
  }
  //Create kernel from program
  cl_components.single_spring_kernel = clCreateKernel(springprogram, "spring_kernel_single", &error);
  if(error)
  {
    printf("Kernel Error: %d\n", error);
  }
  cl_components.batch_spring_kernel = clCreateKernel(springprogram, "spring_kernel_batch", &error);
  if(error)
  {
    printf("Kernel Error: %d\n", error);
  }

  //Compile source and create kernel
  cl_program timestepprogram = NULL;
  source_size = readFile("timestep.cl", source);
  if(source_size == 0) exit(-1);
  timestepprogram = clCreateProgramWithSource(cl_components.opencl_context, 1, sourcelist, &source_size, NULL);
  error = clBuildProgram(timestepprogram, 1, devices, "", NULL, NULL);
  if(error)
  {
    char buildinfo[1000];
    size_t outputsize;
    clGetProgramBuildInfo(timestepprogram, devices[0], CL_PROGRAM_BUILD_LOG, 1000, buildinfo, &outputsize);
    printf("Compile Error for moveprogram: %d\n", error);
    fwrite(buildinfo, min(outputsize, 1000), 1, stdout); 
  }
  //Create kernel from program
  cl_components.euler_kernel = clCreateKernel(timestepprogram, "euler_kernel", &error);
  if(error)
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
