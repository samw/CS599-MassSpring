#ifndef _RENDERSYSTEM_H_
#define _RENDERSYSTEM_H_

#include "MassSpring.h"
#include <GL\glew.h>
#include <GL\wglew.h>
#include <gl\glut.h>

struct input
{
  int mouseX;
  int mouseY;
};
extern struct input input_state;

struct window
{
  int height;
  int width;
};
extern struct window main_window;

void render();
void initGLUT();
void tearDownGL();

#endif