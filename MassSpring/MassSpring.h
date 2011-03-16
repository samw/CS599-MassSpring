
#ifndef _MASSSPRING_H_
#define _MASSSPRING_H_

#include <stdio.h>
#include <stdlib.h>

#include <cl\opencl.h>
#include <GL\glew.h>
#include <GL\wglew.h>
#include <gl\glut.h>

struct system
{
  int num_points;
  GLuint position_buffer;
  cl_mem position;
  cl_mem velocity;
  cl_mem acceleration;
};
extern struct system simulation;

void initSystem();
void nextFrame();
void cleanup();

#endif