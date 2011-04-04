#ifndef _RENDERSYSTEM_H_
#define _RENDERSYSTEM_H_

#include "opengl_headers.h"

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
  int view_mode;
};
extern struct camera window_camera;

void render();
bool initGLUT();
void setCamera();
void rotateCamera(float xdiff, float ydiff);
void moveCamera(float xdiff, float ydiff);
void moveCameraForward(float amount);

#endif
