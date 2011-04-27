#include "RenderSystem.h"
#include "MassSpring.h"
#include "input.h"

#include <stdio.h>
#include <math.h>


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

  glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  glBegin(GL_QUADS);
  glColor3f(1.0,1.0,1.0);
  glVertex3f(5,5,5);
  glVertex3f(5,5,-5);
  glVertex3f(-5,5,-5);
  glVertex3f(-5,5,5);

  glVertex3f(5,0,5);
  glVertex3f(5,0,-5);
  glVertex3f(-5,0,-5);
  glVertex3f(-5,0,5);

  glVertex3f(5,5,5);
  glVertex3f(5,0,5);
  glVertex3f(-5,0,5);
  glVertex3f(-5,5,5);

  glVertex3f(5,5,-5);
  glVertex3f(5,0,-5);
  glVertex3f(-5,0,-5);
  glVertex3f(-5,5,-5);

  glVertex3f(-5,5,-5);
  glVertex3f(-5,0,-5);
  glVertex3f(-5,0,5);
  glVertex3f(-5,5,5);

  glVertex3f(5,5,-5);
  glVertex3f(5,0,-5);
  glVertex3f(5,0,5);
  glVertex3f(5,5,5);
  glEnd();
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glShadeModel(GL_FLAT);

  glPointSize(2);
  glColor3f(1.0, 1.0, 0.0);

  //Draw all vertecies as points
  if(window_camera.view_mode == 0)
  {
    glBindBuffer(GL_ARRAY_BUFFER, simulation.position_buffer);
    glVertexPointer(4, GL_FLOAT, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDrawArrays(GL_POINTS, 0, simulation.num_points);
    glDisableClientState(GL_VERTEX_ARRAY);
  }

  //Draw surface vertecies as points
  if(window_camera.view_mode == 1)
  {
    glBindBuffer(GL_ARRAY_BUFFER, simulation.position_buffer);
    glVertexPointer(4, GL_FLOAT, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, simulation.element_buffer);
    glDrawElements(GL_POINTS, simulation.num_draw_elements, GL_UNSIGNED_INT, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
  }

  //Draw solid surface representation
  if(window_camera.view_mode == 2)
  {
    glEnable(GL_LIGHTING);
    glBindBuffer(GL_ARRAY_BUFFER, simulation.position_buffer);
    glVertexPointer(4, GL_FLOAT, 0, 0);
	//glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	//glBindBuffer(GL_ARRAY_BUFFER,simulation.normal_buffer);
	//glNormalPointer(GL_FLOAT,0,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, simulation.triangle_buffer);
    glDrawElements(GL_TRIANGLES, simulation.num_draw_triangles, GL_UNSIGNED_INT, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_LIGHTING);

	/*glEnable(GL_LIGHTING);
    glBindBuffer(GL_ARRAY_BUFFER, simulation.position_buffer);
    glVertexPointer(4, GL_FLOAT, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, simulation.tri_buf2);
    glDrawElements(GL_TRIANGLES, simulation.num_draw_triangles, GL_UNSIGNED_INT, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_LIGHTING);*/
  }

  // Entered selection mode, render vertex indecies instead and find selected vertex, but don't render
  if(window_camera.view_mode < 0)
  {
    glClearColor(1.0, 1.0, 1.0, 1.0); // max int is background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPointSize(20);
    glBindBuffer(GL_ARRAY_BUFFER, simulation.position_buffer);
    glVertexPointer(4, GL_FLOAT, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, simulation.color_id_buffer);
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, simulation.element_buffer);
    glDrawElements(GL_POINTS, simulation.num_draw_elements, GL_UNSIGNED_INT, 0);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    unsigned int index = 0;
    glReadPixels(input_state.mouseX, main_window.height - input_state.mouseY, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &index);
    if(index != 0xFFFFFFFF)
    {
      simulation.last_vertex_selected = index;
      simulation.vertex_pulling |= 2;
      printf("Vertex Selected: %u Color: %u, %u, %u, %u\n", index,
        (unsigned int) ((index >> 0) & 0xFF),
        (unsigned int) ((index >> 8) & 0xFF),
        (unsigned int) ((index >> 16) & 0xFF),
        (unsigned int) ((index >> 24) & 0xFF));
    }
    glClearColor(0.0, 0.0, 0.0, 1.0);
    window_camera.view_mode *= -1;
    glutPostRedisplay();
    return;
  }

  glutSwapBuffers();
  glutPostRedisplay();
}

bool initGLUT()
{
  //Initialize main window
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH);
  main_window.width = 640;
  main_window.height = 480;
  glutInitWindowSize(main_window.width, main_window.height);
  glutInitWindowPosition(0,0);
  glutCreateWindow("OpenCL Mass Spring Particle System");
  glClearColor(0.0, 0.0, 0.0, 1.0);

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

  input_state.mlook = 0;
  glutSetCursor(GLUT_CURSOR_CROSSHAIR);
  
  glutKeyboardFunc(keyboardFunction);
  glutSpecialFunc(keyboardSpecialFunction);
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

  window_camera.view_mode = 1;
 
  //SETUP LIGHTS
  // global ambient light
  GLfloat aGa[] = { 0.4, 0.1, 0.1, 0.0 };
  
  // light 's ambient, diffuse, specular
  GLfloat lKa0[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat lKd0[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat lKs0[] = { 1.0, 1.0, 1.0, 1.0 };

  GLfloat lKa1[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat lKd1[] = { 1.0, 0.0, 0.0, 1.0 };
  GLfloat lKs1[] = { 1.0, 0.0, 0.0, 1.0 };

  GLfloat lKa2[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat lKd2[] = { 0.0, 1.0, 0.0, 1.0 };
  GLfloat lKs2[] = { 0.0, 1.0, 0.0, 1.0 };

  GLfloat lKa3[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat lKd3[] = { 0.0, 0.0, 1.0, 1.0 };
  GLfloat lKs3[] = { 0.0, 0.0, 1.0, 1.0 };

  GLfloat lKa4[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat lKd4[] = { 0.3, 0.2, 0.1, 1.0 };
  GLfloat lKs4[] = { 0.5, 0.4, 0.2, 1.0 };

  GLfloat lKa5[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat lKd5[] = { 0.3, 0.2, 0.1, 1.0 };
  GLfloat lKs5[] = { 0.5, 0.4, 0.2, 1.0 };

  GLfloat lKa6[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat lKd6[] = { 0.3, 0.2, 0.1, 1.0 };
  GLfloat lKs6[] = { 0.5, 0.4, 0.2, 1.0 };

  GLfloat lKa7[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat lKd7[] = { 0.3, 0.2, 0.1, 1.0 };
  GLfloat lKs7[] = { 0.5, 0.4, 0.2, 1.0 };

  // light positions and directions
  GLfloat lP0[] = { -1.5, -1.5, 1.5, 1.0 };
  GLfloat lP1[] = { 1.5, -1.5, 1.5, 1.0 };
  GLfloat lP2[] = { -1.5, 1.5, 1.5, 1.0 };
  GLfloat lP3[] = { 1.5, 1.5, 1.5, 1.0 };
  GLfloat lP4[] = { 1.999, 1.999, -1.999, 1.0 };
  GLfloat lP5[] = { 1.999, -1.999, -1.999, 1.0 };
  GLfloat lP6[] = { -1.999, 1.999, -1.999, 1.0 };
  GLfloat lP7[] = { -1.999, -1.999, -1.999, 1.0 };
  
  // jelly material color

  GLfloat mKa[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat mKd[] = { 1.0, 0.8, 0.8, 1.0 };
  GLfloat mKs[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat mKe[] = { 0.0, 0.0, 0.0, 1.0 };

  /* set up lighting */
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, aGa);
  glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

  // set up cube color
  glMaterialfv(GL_FRONT, GL_AMBIENT, mKa);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mKd);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mKs);
  glMaterialfv(GL_FRONT, GL_EMISSION, mKe);
  glMaterialf(GL_FRONT, GL_SHININESS, 180);
    
  // macro to set up light i
  #define LIGHTSETUP(i)\
  glLightfv(GL_LIGHT##i, GL_POSITION, lP##i);\
  glLightfv(GL_LIGHT##i, GL_AMBIENT, lKa##i);\
  glLightfv(GL_LIGHT##i, GL_DIFFUSE, lKd##i);\
  glLightfv(GL_LIGHT##i, GL_SPECULAR, lKs##i);\
  glLightf(GL_LIGHT##i, GL_QUADRATIC_ATTENUATION, 0.1);\
  glEnable(GL_LIGHT##i)
  
  LIGHTSETUP (0);
  LIGHTSETUP (1);
  LIGHTSETUP (2);
  LIGHTSETUP (3);
  LIGHTSETUP (4);
  LIGHTSETUP (5);
  LIGHTSETUP (6);
  LIGHTSETUP (7);

  glEnable(GL_DEPTH_TEST);

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
  float up[3];
  float len;
  right[0] = window_camera.look[1] * window_camera.up[2] - window_camera.look[2] * window_camera.up[1];
  right[1] = window_camera.look[2] * window_camera.up[0] - window_camera.look[0] * window_camera.up[2];
  right[2] = window_camera.look[0] * window_camera.up[1] - window_camera.look[1] * window_camera.up[0];
  len = 1/sqrt(right[0]*right[0] + right[1]*right[1] + right[2]*right[2]);
  right[0] *= len;
  right[1] *= len;
  right[2] *= len;
  up[0] = right[1] * window_camera.look[2] - right[2] * window_camera.look[1];
  up[1] = right[2] * window_camera.look[0] - right[0] * window_camera.look[2];
  up[2] = right[0] * window_camera.look[1] - right[1] * window_camera.look[0];

  window_camera.position[0] += (up[0] * ydiff) + (right[0] * xdiff);
  window_camera.position[1] += (up[1] * ydiff) + (right[1] * xdiff);
  window_camera.position[2] += (up[2] * ydiff) + (right[2] * xdiff);
  setCamera();
}

void moveCameraForward(float amount)
{
  window_camera.position[0] += (window_camera.look[0] * amount);
  window_camera.position[1] += (window_camera.look[1] * amount);
  window_camera.position[2] += (window_camera.look[2] * amount);
  setCamera();
}
