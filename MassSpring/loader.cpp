#include "MassSpring.h"
#include "CLFunctions.h"
#include "loader.h"

void generateJelloCube(char *springsfilename, char *colorsfilename)
{
  FILE *springfile;
  FILE *colorfile;
  int num_vertecies = 0;
  int num_springs = 0;
  int num_colors = 0;
  int vert_per_dim = 0;
  float (*vertexPositions)[4];
  int (*springs)[2]; //vertecies of springs
  int *springColors; //colors for spings listed in order of 'springs'
  int *batchcount;  //# of springs in each batch
  int *fillcount;
  int **batches; //array containing spring index from 'springs' in separate batches
  cl_int error;

  springfile = fopen(springsfilename, "r");
  if( !springfile)
  {
    printf ("Error reading spring input files\n");
    return;
  }
  colorfile = fopen(colorsfilename, "r");
  if( !colorfile)
  {
    printf ("Error reading color input files\n");
    fclose(springfile);
    return;
  }

  //// READ FILES
  fscanf(springfile, "%d %d %d\n", &vert_per_dim, &num_vertecies, &num_springs);
  fscanf(colorfile, "%d\n", &num_colors);

  vertexPositions = new float[num_vertecies][4];
  springs = new int[num_springs][2];
  springColors = new int[num_springs];
  batchcount = new int[num_colors];
  fillcount = new int[num_colors];
  for(int i = 0; i < num_colors; i++)
  {
    batchcount[i] = 0;
    fillcount[i] = 0;
  }

  for(int i = 0; i < num_springs; i++)
  {
    fscanf(springfile, "%d %d\n", &(springs[i][0]), &(springs[i][1]));
    fscanf(colorfile, "%d\n", &(springColors[i]));
    batchcount[springColors[i]]++;
    //printf("%d %d %d\n", springs[i][0], springs[i][1], springColors[i]);
  }
  fclose(springfile);
  fclose(colorfile);
  
  //// PROCESS INFORMATION
  //Set up vertex posisitons based on evenly spaced cube vertecies
  int v2 = vert_per_dim*vert_per_dim;
  int v = vert_per_dim;
  for(int i = 0; i < num_vertecies; i++)
  {
    vertexPositions[i][0] = (float)(i/v2)/(float)v ;
    vertexPositions[i][1] = (float)((i%v2)/v)/(float)v;
    vertexPositions[i][2] = (float)(i%v)/(float)v;
    vertexPositions[i][3] = 1.0f;
    //printf("%f %f %f\n", vertexPositions[i][0], vertexPositions[i][1], vertexPositions[i][2]);
  }
  //Count size of each spring batch
  batches = new int*[num_colors];
  for(int i = 0; i < num_colors; i++)
  {
    batches[i] = new int[batchcount[i]];
  }
  //Create batch information to hold spring memberships of each batch
  for(int i = 0; i < num_springs; i++)
  {
    batches[springColors[i]][fillcount[springColors[i]]++]= i;
  }

  //STORE SYSTEM INFORMATION AND INFORMATION ONTO VIDEOCARD
  simulation.num_points = num_vertecies;
  simulation.num_batches = num_colors;
  simulation.batch_sizes = new int[num_colors];
  for(int i = 0; i < num_colors; i++)
  {
    simulation.batch_sizes[i] = batchcount[i];
  }
  
  //Put vertex positions into vertex buffer
  //Create OpenGL buffer
  glGenBuffers(1, &(simulation.position_buffer));
  glBindBuffer(GL_ARRAY_BUFFER, simulation.position_buffer);
  glBufferData(GL_ARRAY_BUFFER, simulation.num_points * 4 * sizeof(GLfloat), vertexPositions, GL_DYNAMIC_DRAW);
  //Create OpenCL buffer object attached to OpenGL vertex buffer object
  simulation.position = clCreateFromGLBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE,
                                             simulation.position_buffer, &error);
  if(error) printf("Position Buffer Error: %d", error);
  //Make vertexPositions 0 so we can use them as source for position and velocity
  for(int i = 0; i < simulation.num_points; i++)
  {
    vertexPositions[i][0] = 0.0;
    vertexPositions[i][1] = 0.0;
    vertexPositions[i][2] = 0.0;
    vertexPositions[i][3] = 0.0;
  }
  //Create other OpenCL buffers for acceleration, velocity and midpoint buffers
  simulation.velocity = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                       sizeof(cl_float) * 4 * simulation.num_points, vertexPositions, &error);
  if(error) printf("Veclocity Buffer Error: %d", error);
  simulation.acceleration = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                           sizeof(cl_float) * 4 * simulation.num_points, vertexPositions, &error);
  if(error) printf("Acceleration Buffer Error: %d", error);
  simulation.bufferP = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                      sizeof(cl_float) * 4 * simulation.num_points, vertexPositions, &error);
  if(error) printf("P Buffer Error: %d", error);
  simulation.bufferV = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                      sizeof(cl_float) * 4 * simulation.num_points, vertexPositions, &error);
  if(error) printf("V Buffer Error: %d", error);

  //Put spring information into videocard (spring information is the 2 vertecies it connects)
  //At same time fill in spring properties
  //We need to put information as individual batches
  simulation.springBatches = new cl_mem[num_colors];
  simulation.springPropertyBatches = new cl_mem[num_colors];
  float (*spring_properties)[4];
  int (*batchedsprings)[2];
  for(int i = 0; i < num_colors; i++)
  {
    spring_properties = new float[batchcount[i]][4];
    batchedsprings = new int[batchcount[i]][2];
    //for each batch copy the member springs into bachedspring list of springs
    for(int j = 0; j < batchcount[i]; j++)
    {
      spring_properties[j][0] = 0.1; //reset length 
      //really all the springs should have differnt rest lenghts but this is for testing
      spring_properties[j][1] = 100.0; //spring force
      spring_properties[j][2] = 10.0; //damepning force
      spring_properties[j][3] = 0.0; //<empty>
      batchedsprings[j][0] = springs[(batches[i][j])][0];
      batchedsprings[j][1] = springs[(batches[i][j])][1];
    }
    simulation.springBatches[i] = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
      sizeof(cl_int) * 2 * batchcount[i], batchedsprings, &error);
    if(error) printf("Spring Buffer Error: %d", error);
    simulation.springPropertyBatches[i] = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
      sizeof(cl_float) * 4 * batchcount[i], spring_properties, &error);
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
  delete batchcount;
  delete springColors;
  delete[] springs;
  delete[] vertexPositions;
}