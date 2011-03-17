#include "MassSpring.h"
#include "RenderSystem.h"
#include "CLFunctions.h"

struct system simulation;

int main(int argc, char** argv)
{
  
  if(!initGLUT())
    exit(0);
  initOpenCL();
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
                                           sizeof(CL_FLOAT) * 4 * simulation.num_points, data, &error);

  for(int i = 0; i < simulation.num_points; i++)
  {
    data[(4*i) + 0] = 0.0;
    data[(4*i) + 1] = 0.0;
    data[(4*i) + 2] = 0.5 * i;
    data[(4*i) + 3] = 0.0;
  }
  simulation.velocity = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                           sizeof(CL_FLOAT) * 4 * simulation.num_points, data, &error);

  delete data;
}

void nextFrame()
{
  runTestKernel();
}

void cleanup()
{
  tearDownCL();
  tearDownGL();
}