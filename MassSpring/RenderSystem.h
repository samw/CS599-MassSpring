#ifndef _RENDERSYSTEM_H_
#define _RENDERSYSTEM_H_

#include <GL\glew.h>
#include <GL\wglew.h>
#include <gl\glut.h>

struct input
{
  int mouseX;
  int mouseY;
  int left;
  int right;
  int middle;
};
extern struct input input_state;

struct window
{
  int height;
  int width;
};
extern struct window main_window;

struct camera
{
  float position[3];
  float look[3];
  float up[3];
};
extern struct camera window_camera;

void render();
bool initGLUT();
void setCamera();
void rotateCamera(float xdiff, float ydiff);
void moveCamera(float xdiff, float ydiff);
void moveCameraForward(float amount);
void tearDownGL();
void mouseMotionDrag(int x, int y);
void mouseMotion (int x, int y);
void mouseButton(int button, int state, int x, int y);

#endif