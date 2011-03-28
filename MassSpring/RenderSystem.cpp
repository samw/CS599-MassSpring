#include "MassSpring.h"
#include "RenderSystem.h"
#include <math.h>

struct input input_state;
struct window main_window;
struct camera window_camera;

void render()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBegin(GL_LINES);
  glColor3f(1.0, 0.0, 0.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.1, 0.0, 0.0);
  glColor3f(0.0, 1.0, 0.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, 0.1, 0.0);
  glColor3f(0.0, 0.0, 1.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, 0.0,0.1);
  glEnd();

  glPointSize(2);
  glColor3f(1.0, 1.0, 0.0);
  glBindBuffer(GL_ARRAY_BUFFER, simulation.position_buffer);
  glVertexPointer(4, GL_FLOAT, 0, 0);
  glEnableClientState(GL_VERTEX_ARRAY);
  glDrawArrays(GL_POINTS, 0, simulation.num_points);
  glDisableClientState(GL_VERTEX_ARRAY);

  glutSwapBuffers();
  glutPostRedisplay();
}

bool initGLUT()
{
  //Initialize main window
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  main_window.width = 640;
  main_window.height = 480;
  glutInitWindowSize(main_window.width, main_window.height);
  glutInitWindowPosition(0,0);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glutCreateWindow("OpenCL Mass Spring Particle System");

  glutDisplayFunc(render);

  //Set up transformation matricies
  glViewport(0, 0, main_window.width, main_window.height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(90.0, (float)main_window.width/(float)main_window.height, 0.1, 10.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  window_camera.position[0] = 0.5;
  window_camera.position[1] = 1.5;
  window_camera.position[2] = 1.5;
  
  window_camera.look[0] = -0.0;
  window_camera.look[1] = -0.70710678118654752440084436210485;
  window_camera.look[2] = -0.70710678118654752440084436210485;
  
  window_camera.up[0] = 0.0;
  window_camera.up[1] = 1.0;
  window_camera.up[2] = 0.0;
  setCamera();

  glutMotionFunc(mouseMotionDrag);
  glutPassiveMotionFunc(mouseMotion);
  glutMouseFunc(mouseButton);

  //Check extensions and vertex buffer object support
  if(glewInit() != GLEW_OK)
  {
    printf("Failed to initialize GLEW\n");
    return false;
  }
  if(!glewIsSupported("GL_ARB_vertex_buffer_object"))
  {
    printf("OpenGL Vertex Buffer Object not supported.\n");
    return false;
  }
  return true;
}

void setCamera()
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(window_camera.position[0], window_camera.position[1], window_camera.position[2],
    window_camera.position[0] + window_camera.look[0],
    window_camera.position[1] + window_camera.look[1],
    window_camera.position[2] + window_camera.look[2],
    window_camera.up[0], window_camera.up[1], window_camera.up[2]);
}

void rotateCamera(float xdiff, float ydiff)
{
  float right[3];
  float len;
  right[0] = window_camera.look[1] * window_camera.up[2] - window_camera.look[2] * window_camera.up[1];
  right[1] = window_camera.look[2] * window_camera.up[0] - window_camera.look[0] * window_camera.up[2];
  right[2] = window_camera.look[0] * window_camera.up[1] - window_camera.look[1] * window_camera.up[0];
  window_camera.look[0] += window_camera.up[0] * ydiff + right[0] * xdiff;
  window_camera.look[1] += window_camera.up[1] * ydiff + right[1] * xdiff;
  window_camera.look[2] += window_camera.up[2] * ydiff + right[2] * xdiff;
  len = 1/sqrt(window_camera.look[0] * window_camera.look[0] +
               window_camera.look[1] * window_camera.look[1] +
               window_camera.look[2] * window_camera.look[2] );
  window_camera.look[0] *= len;
  window_camera.look[1] *= len;
  window_camera.look[2] *= len;
  setCamera();
}

void moveCamera(float xdiff, float ydiff)
{
  float right[3];
  right[0] = window_camera.look[1] * window_camera.up[2] - window_camera.look[2] * window_camera.up[1];
  right[1] = window_camera.look[2] * window_camera.up[0] - window_camera.look[0] * window_camera.up[2];
  right[2] = window_camera.look[0] * window_camera.up[1] - window_camera.look[1] * window_camera.up[0];
  window_camera.position[0] += (window_camera.up[0] * ydiff) + (right[0] * xdiff);
  window_camera.position[1] += (window_camera.up[1] * ydiff) + (right[1] * xdiff);
  window_camera.position[2] += (window_camera.up[2] * ydiff) + (right[2] * xdiff);
  setCamera();
}

void moveCameraForward(float amount)
{
  window_camera.position[0] += (window_camera.look[0] * amount);
  window_camera.position[1] += (window_camera.look[1] * amount);
  window_camera.position[2] += (window_camera.look[2] * amount);
  setCamera();
}

void tearDownGL()
{
  glDeleteBuffers(1, &(simulation.position_buffer));
}

/* converts mouse drags into information about rotation/translation/scaling */
void mouseMotionDrag(int x, int y)
{
  float movement[2] = {x-input_state.mouseX, y-input_state.mouseY};

  if (input_state.right) // handle camera rotations
  {
    moveCamera( movement[0]/100.0,-movement[1]/100.0);
  }
  else
  if(input_state.left)
  {
    rotateCamera(movement[0]/100, -(movement[1]/100));
  }
  else
  if(input_state.middle)
  {
    moveCameraForward(-movement[1]/100);
  }

  input_state.mouseX = x;
  input_state.mouseY = y;
}

void mouseMotion (int x, int y)
{
  input_state.mouseX = x;
  input_state.mouseY = y;
}

void mouseButton(int button, int state, int x, int y)
{
  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      input_state.left = (state==GLUT_DOWN);
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