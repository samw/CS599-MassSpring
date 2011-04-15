#include "input.h"
#include "RenderSystem.h"

#include <math.h>


struct input input_state;

void keyboardFunction(unsigned char key, int x, int y)
{
  switch (key)
  {
  case 27:
    exit(0);
    break;
  case 'z':
    input_state.mlook = (input_state.mlook + 1) % 2;
    if(input_state.mlook) glutSetCursor(GLUT_CURSOR_NONE);
    else glutSetCursor(GLUT_CURSOR_CROSSHAIR);
    break;
  case 'v':
    window_camera.view_mode = (window_camera.view_mode+1)%3;
  }
}

void keyboardSpecialFunction(int key, int x, int y)
{
  switch (key)
  {
  case GLUT_KEY_UP:
    moveCameraForward(0.025);
    break;
  case GLUT_KEY_DOWN:
    moveCameraForward(-0.025);
    break;
  case GLUT_KEY_LEFT:
    moveCamera(-0.025, 0.0);
    break;
  case GLUT_KEY_RIGHT:
    moveCamera(0.025, 0.0);
    break;
  }
}

/* converts mouse drags into information about camera movement*/
void mouseMotionDrag(int x, int y)
{
  float movement[2] = {x-input_state.mouseX, y-input_state.mouseY};

  if (input_state.middle)
  {
    moveCamera( movement[0]/100.0,-movement[1]/100.0);
  }
  else
  if(input_state.right)
  {
    if(input_state.mlook)
      moveCameraForward(-movement[1]/100);
    else
      rotateCamera(movement[0]/100, -(movement[1]/100));
  }
  input_state.mouseX = x;
  input_state.mouseY = y;
}

void mouseMotion (int x, int y)
{
  if(input_state.mlook)
  {
    float movement[2] = {x-input_state.mouseX, y-input_state.mouseY};
    rotateCamera(movement[0]/100, -(movement[1]/100));
    int dist = abs(x - (main_window.width/2)) + abs(y - (main_window.height/2));
    if(dist > 100)
    {
      input_state.mouseX = main_window.width/2;
      input_state.mouseY = main_window.height/2;
      glutWarpPointer(main_window.width/2,main_window.height/2);
      return;
    }
  }
  input_state.mouseX = x;
  input_state.mouseY = y;
}

void mouseButton(int button, int state, int x, int y)
{
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      input_state.left = (state==GLUT_DOWN);
      if(input_state.left == GLUT_DOWN)
        window_camera.view_mode *= -1;
      break;
    case GLUT_MIDDLE_BUTTON:
      input_state.middle = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      input_state.right = (state==GLUT_DOWN);
      break;
  }
  input_state.mouseX = x;
  input_state.mouseY = y;
}