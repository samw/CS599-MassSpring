#include "MassSpring.h"
#include "CLFunctions.h"
#include "loader.h"
#include <math.h>

void generateJelloCube(char *springsfilename, char *colorsfilename)
{
  FILE *spring_file;
  FILE *color_file;
  int num_vertecies = 0;
  int num_springs = 0;
  int num_colors = 0;
  int vert_per_dim = 0;
  float (*vertex_positions)[4];
  float (*vertex_init)[4];
  int (*springs)[2]; //vertecies of springs
  int *spring_colors; //colors for spings listed in order of 'springs'
  int *batch_count;  //# of springs in each batch
  int *fill_count;
  int **batches; //array containing spring index from 'springs' in separate batches
  cl_int error;

  spring_file = fopen(springsfilename, "r");
  if( !spring_file)
  {
    printf ("Error reading spring input files\n");
    return;
  }
  color_file = fopen(colorsfilename, "r");
  if( !color_file)
  {
    printf ("Error reading color input files\n");
    fclose(spring_file);
    return;
  }

  //// READ FILES
  fscanf(spring_file, "%d %d %d\n", &vert_per_dim, &num_vertecies, &num_springs);
  fscanf(color_file, "%d\n", &num_colors);
  
  vertex_positions = new float[num_vertecies][4];
  vertex_init = new float[num_vertecies][4];
  springs = new int[num_springs][2];
  spring_colors = new int[num_springs];
  batch_count = new int[num_colors];
  fill_count = new int[num_colors];
  for(int i = 0; i < num_colors; i++)
  {
    batch_count[i] = 0;
    fill_count[i] = 0;
  }

  for(int i = 0; i < num_springs; i++)
  {
    fscanf(spring_file, "%d %d\n", &(springs[i][0]), &(springs[i][1]));
    fscanf(color_file, "%d\n", &(spring_colors[i]));
    batch_count[spring_colors[i]]++;
    //printf("%d %d %d\n", springs[i][0], springs[i][1], spring_colors[i]);
  }
  fclose(spring_file);
  fclose(color_file);
  
  //// PROCESS INFORMATION
  //Set up vertex posisitons based on evenly spaced cube vertecies
  int v2 = vert_per_dim*vert_per_dim;
  int v = vert_per_dim;
  for(int i = 0; i < num_vertecies; i++)
  {
    vertex_positions[i][0] = (float)(i/v2)/(float)v ;
    vertex_positions[i][1] = (float)((i%v2)/v)/(float)v;
    vertex_positions[i][2] = (float)(i%v)/(float)v;
    vertex_positions[i][3] = 1.0f;
    //printf("%f %f %f\n", vertex_positions[i][0], vertex_positions[i][1], vertex_positions[i][2]);
  }
  //Count size of each spring batch
  batches = new int*[num_colors];
  for(int i = 0; i < num_colors; i++)
  {
    batches[i] = new int[batch_count[i]];
  }
  //Create batch information to hold spring memberships of each batch
  for(int i = 0; i < num_springs; i++)
  {
    batches[spring_colors[i]][fill_count[spring_colors[i]]++]= i;
  }

  //STORE SYSTEM INFORMATION AND INFORMATION ONTO VIDEOCARD
  simulation.num_points = num_vertecies;
  simulation.num_batches = num_colors;
  simulation.batch_sizes = new int[num_colors];
  for(int i = 0; i < num_colors; i++)
  {
    simulation.batch_sizes[i] = batch_count[i];
  }
  
  //Put vertex positions into vertex buffer
  //Create OpenGL buffer
  glGenBuffers(1, &(simulation.position_buffer));
  glBindBuffer(GL_ARRAY_BUFFER, simulation.position_buffer);
  glBufferData(GL_ARRAY_BUFFER, simulation.num_points * 4 * sizeof(GLfloat), vertex_positions, GL_DYNAMIC_DRAW);
  //Create OpenCL buffer object attached to OpenGL vertex buffer object
  simulation.position = clCreateFromGLBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE,
                                             simulation.position_buffer, &error);
  if(error) printf("Position Buffer Error: %d", error);
  //Make vertex_init 0 so we can use them as source for position and velocity
  for(int i = 0; i < simulation.num_points; i++)
  {
    vertex_init[i][0] = 0.0;
    vertex_init[i][1] = 0.0;
    vertex_init[i][2] = 0.0;
    vertex_init[i][3] = 0.0;
  }
  //Create other OpenCL buffers for acceleration, velocity and midpoint buffers
  simulation.velocity = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                       sizeof(cl_float) * 4 * simulation.num_points, vertex_init, &error);
  if(error) printf("Veclocity Buffer Error: %d", error);
  simulation.acceleration = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                           sizeof(cl_float) * 4 * simulation.num_points, vertex_init, &error);
  if(error) printf("Acceleration Buffer Error: %d", error);
  simulation.bufferP = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                      sizeof(cl_float) * 4 * simulation.num_points, vertex_init, &error);
  if(error) printf("P Buffer Error: %d", error);
  simulation.bufferV = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                      sizeof(cl_float) * 4 * simulation.num_points, vertex_init, &error);
  if(error) printf("V Buffer Error: %d", error);
  simulation.f1p = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                      sizeof(cl_float) * 4 * simulation.num_points, vertex_init, &error);
  if(error) printf("f1p Buffer Error: %d", error);
  simulation.f1v = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                      sizeof(cl_float) * 4 * simulation.num_points, vertex_init, &error);
  if(error) printf("f1v Buffer Error: %d", error);
  simulation.f2p = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                      sizeof(cl_float) * 4 * simulation.num_points, vertex_init, &error);
  if(error) printf("f2p Buffer Error: %d", error);
  simulation.f2v = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                      sizeof(cl_float) * 4 * simulation.num_points, vertex_init, &error);
  if(error) printf("f2v Buffer Error: %d", error);
  simulation.f3p = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                      sizeof(cl_float) * 4 * simulation.num_points, vertex_init, &error);
  if(error) printf("f3p Buffer Error: %d", error);
  simulation.f3v = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                      sizeof(cl_float) * 4 * simulation.num_points, vertex_init, &error);
  if(error) printf("f3v Buffer Error: %d", error);
  simulation.f4p = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                      sizeof(cl_float) * 4 * simulation.num_points, vertex_init, &error);
  if(error) printf("f4p Buffer Error: %d", error);
  simulation.f4v = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                      sizeof(cl_float) * 4 * simulation.num_points, vertex_init, &error);
  if(error) printf("f4v Buffer Error: %d", error);

  //Put spring information into videocard (spring information is the 2 vertecies it connects)
  //At same time fill in spring properties
  //We need to put information as individual batches
  simulation.springBatches = new cl_mem[num_colors];
  simulation.springPropertyBatches = new cl_mem[num_colors];
  float (*spring_properties)[4];
  int (*batchedsprings)[2];
  for(int i = 0; i < num_colors; i++)
  {
    spring_properties = new float[batch_count[i]][4];
    batchedsprings = new int[batch_count[i]][2];
    //for each batch copy the member springs into bachedspring list of springs
    for(int j = 0; j < batch_count[i]; j++)
    {
      //Copy vertex # from spring list to batched springl list
      batchedsprings[j][0] = springs[(batches[i][j])][0];
      batchedsprings[j][1] = springs[(batches[i][j])][1];
      //spring_properties[j][0] = 0.1; //reset length 
      //really all the springs should have differnt rest lenghts but this is for testing
      //find actual rest distance between vertecies
#define SQUARE(x) (x*x)
      float rest = sqrt(
        SQUARE((vertex_positions[batchedsprings[j][0]][0] - vertex_positions[batchedsprings[j][1]][0])) +
        SQUARE((vertex_positions[batchedsprings[j][0]][1] - vertex_positions[batchedsprings[j][1]][1])) +
        SQUARE((vertex_positions[batchedsprings[j][0]][2] - vertex_positions[batchedsprings[j][1]][2])) );
#undef SQUARE
      spring_properties[j][0] = 0.6 * rest; //reset length 
      spring_properties[j][1] = 100.0; //spring force
      spring_properties[j][2] = 10.0; //damepning force
      spring_properties[j][3] = 0.0; //<empty>
    }
    simulation.springBatches[i] = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
      sizeof(cl_int) * 2 * batch_count[i], batchedsprings, &error);
    if(error) printf("Spring Buffer Error: %d", error);
    simulation.springPropertyBatches[i] = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
      sizeof(cl_float) * 4 * batch_count[i], spring_properties, &error);
    if(error) printf("Spring Properties Buffer Error: %d", error);
    delete[] batchedsprings;
    delete[] spring_properties;
  }

  //DONE NOW CLEAN UP
  for(int i = 0; i < num_colors; i++)
  {
    delete batches[i];
  }
  delete[] batches;
  delete batch_count;
  delete spring_colors;
  delete[] springs;
  delete[] vertex_init;
  delete[] vertex_positions;
}