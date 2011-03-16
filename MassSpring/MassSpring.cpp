#include "MassSpring.h"
#include "RenderSystem.h"
#include "CLFunctions.h"

struct system simulation;

int main(int argc, char** argv)
{
  initSystem();
  initGLUT();
  initOpenCL();

  //atexit(cleanup);
  glutMainLoop();
  exit(-1);
}

void initSystem()
{
  simulation.num_points = 1;
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