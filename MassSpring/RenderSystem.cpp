#include "RenderSystem.h"
#include "MassSpring.h"

struct input input_state;
struct window main_window;

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

  glPointSize(5);
  glColor3f(1.0, 1.0, 0.0);
  glBindBuffer(GL_ARRAY_BUFFER, simulation.position_buffer);
  glVertexPointer(4, GL_FLOAT, 0, 0);
  glEnableClientState(GL_VERTEX_ARRAY);
  glDrawArrays(GL_POINTS, 0, simulation.num_points);
  glDisableClientState(GL_VERTEX_ARRAY);

  glutSwapBuffers();
  glutPostRedisplay();
}

void initGLUT()
{
  //Initialize main window
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  main_window.width = 640;
  main_window.height = 480;
  glutInitWindowSize(main_window.width, main_window.height);
  glutInitWindowPosition(0,0);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glutCreateWindow("OpenCL Mass Spring Particle System");

  //Check extensions and vertex buffer object support
  if(glewInit() != GLEW_OK)
  {
    printf("Failed to initialize GLEW");
    exit(1);
  }
  if(!glewIsSupported("GL_ARB_vertex_buffer_object"))
  {
    printf("OpenGL Vertex Buffer Object not supported. Exiting");
    exit(10);
  }
  
  //Create Vertex Buffer object
  GLfloat data[4] = {0.0, -0.8, 0.0, 1.0};
  glGenBuffers(1, &(simulation.position_buffer));
  glBindBuffer(GL_ARRAY_BUFFER, simulation.position_buffer);
  glBufferData(GL_ARRAY_BUFFER, simulation.num_points * 4 * sizeof(GLfloat), &data, GL_DYNAMIC_DRAW);

  glutDisplayFunc(render);
  glutIdleFunc(nextFrame);

  //Set up transformation matricies
  glViewport(0, 0, main_window.width, main_window.height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(90.0, (float)main_window.width/(float)main_window.height, 0.1, 10.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(1.0, 0.5, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void tearDownGL()
{
  glDeleteBuffers(1, &(simulation.position_buffer));
}