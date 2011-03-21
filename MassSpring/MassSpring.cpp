#include "MassSpring.h"
#include "RenderSystem.h"
#include "CLFunctions.h"
#include "loader.h"

struct system simulation;

int main(int argc, char** argv)
{  
  if(!initGLUT())
    exit(0);
  if(!initOpenCL())
    exit(0);
  
  //generateJelloCube("cubeEdges","edgeColors");
  initSystem();
  
  glutIdleFunc(nextFrame);

  //atexit(cleanup);
  glutMainLoop();
  exit(-1);
}

void initSystem()
{
  cl_int error;
  simulation.num_points = 2;
  
  //Create Vertex Buffer object
  GLfloat *data = new GLfloat[4*simulation.num_points];
  for(int i = 0; i < simulation.num_points; i++)
  {
    data[(4*i) + 0] = 0.0;
    data[(4*i) + 1] = -0.8 + (i *0.25);
    data[(4*i) + 2] = 0.0;
    data[(4*i) + 3] = 1.0;
  }
  glGenBuffers(1, &(simulation.position_buffer));
  glBindBuffer(GL_ARRAY_BUFFER, simulation.position_buffer);
  glBufferData(GL_ARRAY_BUFFER, simulation.num_points * 4 * sizeof(GLfloat), data, GL_DYNAMIC_DRAW);
  
  //Create OpenCL buffer object attached to OpenGL vertex buffer object
  simulation.position = clCreateFromGLBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE,
                                             simulation.position_buffer, NULL);

   for(int i = 0; i < simulation.num_points; i++)
  {
    data[(4*i) + 0] = 0.0;
    data[(4*i) + 1] = 0.0;
    data[(4*i) + 2] = 0.0;
    data[(4*i) + 3] = 0.0;
  }
  simulation.acceleration = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                           sizeof(cl_float) * 4 * simulation.num_points, data, &error);
  simulation.bufferP = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                           sizeof(cl_float) * 4 * simulation.num_points, data, &error);
  simulation.bufferV = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                           sizeof(cl_float) * 4 * simulation.num_points, data, &error);
  for(int i = 0; i < simulation.num_points; i++)
  {
    data[(4*i) + 0] = 0.0;
    data[(4*i) + 1] = 0.0;
    data[(4*i) + 2] = 0.5 * i;
    data[(4*i) + 3] = 0.0;
  }
  simulation.velocity = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                           sizeof(cl_float) * 4 * simulation.num_points, data, &error);
  
  cl_int springs[1][2];
  springs[0][0] = 0;
  springs[0][1] = 1;
  simulation.springs = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                           sizeof(cl_int) * 1 * 2, springs, &error);
  cl_float spring_properties[1][4];
  spring_properties[0][0] = 0.25;
  spring_properties[0][1] = 100.0;
  spring_properties[0][2] = 2.0;
  spring_properties[0][3] = 0.0;
  simulation.springProperties = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                           sizeof(cl_float) * 1 * 4, spring_properties, &error);

  delete data;
}

void nextFrame()
{
  runTestKernelMidPoint();
}

void cleanup()
{
  tearDownCL();
  tearDownGL();
}