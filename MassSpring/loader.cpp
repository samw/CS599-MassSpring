#include "loader.h"
#include "MassSpring.h"
#include "CLFunctions.h"

#include <stdio.h>
#include <math.h>

#define DINO_NUM 10

void generateJelloCube(char *springsfilename, char *colorsfilename)
{
  FILE *spring_file;
  FILE *color_file;
  int num_vertecies = 0;
  int num_springs = 0;
  int num_colors = 0;
  int vert_per_dim = 0;
  float (*vertex_positions)[4];
  GLubyte (*vertex_colors)[4]; //used to color vertecies for coloring
  float (*vertex_init)[4];
  GLint *side_vertex_indecies; //used to fill IBO
  GLint *side_triangle_indecies; // used to fill IBO for triangles
  int side_vert_fill = 0;
  int side_triangle_fill = 0;
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

  side_vertex_indecies = new GLint[(6 * (vert_per_dim * vert_per_dim))];
  side_triangle_indecies = new GLint[(6 * (vert_per_dim * vert_per_dim)) * 6];
  vertex_positions = new float[num_vertecies][4];
  vertex_colors = new GLubyte[num_vertecies][4];
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
    vertex_positions[i][0] = (float)(i/v2)/(float)(v-1) ;
    vertex_positions[i][1] = (float)((i%v2)/v)/(float)(v-1);
    vertex_positions[i][2] = (float)(i%v)/(float)(v-1);
    vertex_positions[i][3] = 1.0f;
   
    vertex_colors[i][0] = (i >>  0) & 0xFF;
    vertex_colors[i][1] = (i >>  8) & 0xFF;
    vertex_colors[i][2] = (i >> 16) & 0xFF;
    vertex_colors[i][3] = (i >> 24) & 0xFF;
    //printf("%f %f %f\n", vertex_positions[i][0], vertex_positions[i][1], vertex_positions[i][2]);

    //Detect edge vertecies and triangles and create drawlists for them
    if( vertex_positions[i][0] == 0.0 || vertex_positions[i][0] == 1.0 ||
      vertex_positions[i][1] == 0.0 || vertex_positions[i][1] == 1.0 ||
      vertex_positions[i][2] == 0.0 || vertex_positions[i][2] == 1.0)
    {
      side_vertex_indecies[side_vert_fill++] = i;

      int xp = -1;
      int xn = -1;
      int yp = -1;
      int yn = -1;
      int zp = -1;
      int zn = -1;

      if(i/v2 < vert_per_dim - 1)
        xp = i + (v2);
      if(i/v2 > 0)
        xn = i - (v2);
      if((i%v2)/v < vert_per_dim - 1)
        yp = i + v;
      if((i%v2)/v > 0)
        yn = i - v;
      if(i%v < vert_per_dim - 1)
        zp = i + 1;
      if(i%v > 0)
        zn = i - 1;

      if( xp >= 0 && yp >= 0 && (vertex_positions[i][2] == 0.0 || vertex_positions[i][2] == 1.0))
      {
        side_triangle_indecies[side_triangle_fill++] = i;
        side_triangle_indecies[side_triangle_fill++] = xp;
        side_triangle_indecies[side_triangle_fill++] = yp;
      }
      if( xn >= 0 && yn >= 0 && (vertex_positions[i][2] == 0.0 || vertex_positions[i][2] == 1.0))
      {
        side_triangle_indecies[side_triangle_fill++] = i;
        side_triangle_indecies[side_triangle_fill++] = xn;
        side_triangle_indecies[side_triangle_fill++] = yn;
      }
      if( xp >= 0 && zp >= 0 && (vertex_positions[i][1] == 0.0 || vertex_positions[i][1] == 1.0))
      {
        side_triangle_indecies[side_triangle_fill++] = i;
        side_triangle_indecies[side_triangle_fill++] = xp;
        side_triangle_indecies[side_triangle_fill++] = zp;
      }
      if( xn >= 0 && zn >= 0 && (vertex_positions[i][1] == 0.0 || vertex_positions[i][1] == 1.0))
      {
        side_triangle_indecies[side_triangle_fill++] = i;
        side_triangle_indecies[side_triangle_fill++] = xn;
        side_triangle_indecies[side_triangle_fill++] = zn;
      }
      if( zp >= 0 && yp >= 0 && (vertex_positions[i][0] == 0.0 || vertex_positions[i][0] == 1.0))
      {
        side_triangle_indecies[side_triangle_fill++] = i;
        side_triangle_indecies[side_triangle_fill++] = yp;
        side_triangle_indecies[side_triangle_fill++] = zp;
      }
      if( zn >= 0 && yn >= 0 && (vertex_positions[i][0] == 0.0 || vertex_positions[i][0] == 1.0))
      {
        side_triangle_indecies[side_triangle_fill++] = i;
        side_triangle_indecies[side_triangle_fill++] = yn;
        side_triangle_indecies[side_triangle_fill++] = zn;
      }
    }
  }
  simulation.num_draw_elements = side_vert_fill;
  simulation.num_draw_triangles = side_triangle_fill;

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

  // Pull out a corner so we can see it snap back in
  //vertex_positions[num_vertecies-1][0] = 2.0f;
  //vertex_positions[num_vertecies-1][1] = 2.0f;
  //vertex_positions[num_vertecies-1][2] = 2.0f;
  //vertex_positions[num_vertecies-1][3] = 1.0f;
  
  //Put vertex positions into vertex buffer
  //Create OpenGL buffer
  glGenBuffers(1, &(simulation.position_buffer));
  glBindBuffer(GL_ARRAY_BUFFER, simulation.position_buffer);
  glBufferData(GL_ARRAY_BUFFER, simulation.num_points * 4 * sizeof(GLfloat), vertex_positions, GL_DYNAMIC_DRAW);
  //Create OpenCL buffer object attached to OpenGL vertex buffer object
  simulation.position = clCreateFromGLBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE,
                                             simulation.position_buffer, &error);

  //Create drawing index buffer for drawing vertex points
  glGenBuffers(1, &(simulation.element_buffer));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, simulation.element_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, side_vert_fill * sizeof(GLint), side_vertex_indecies, GL_STATIC_DRAW);

  //Create color buffer for coloring vertex points
  glGenBuffers(1, &(simulation.color_id_buffer));
  glBindBuffer(GL_ARRAY_BUFFER, simulation.color_id_buffer);
  glBufferData(GL_ARRAY_BUFFER, simulation.num_points * 4 * sizeof(GLubyte), vertex_colors, GL_STATIC_DRAW);

  //Create index buffer for drawing triangles
  glGenBuffers(1, &(simulation.triangle_buffer));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, simulation.triangle_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, side_triangle_fill * sizeof(GLint), side_triangle_indecies, GL_STATIC_DRAW);

  // Push corner back in to correct rest position
  vertex_positions[num_vertecies-1][0] = 1.0f;
  vertex_positions[num_vertecies-1][1] = 1.0f;
  vertex_positions[num_vertecies-1][2] = 1.0f;
  vertex_positions[num_vertecies-1][3] = 1.0f;

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

  float pull_init[4] = {0.0, 0.0, 0.0, 0.0};
  simulation.pull_value = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                         sizeof(cl_float4), &pull_init, &error);
  if(error) printf("Pull Init Buffer Error: %d", error);

  simulation.pull_position = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                         sizeof(cl_float4), &pull_init, &error);
  if(error) printf("Pull Position Buffer Error: %d", error);

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
#define SQUARE(x) (x*x)
      float rest = sqrt(
        SQUARE((vertex_positions[batchedsprings[j][0]][0] - vertex_positions[batchedsprings[j][1]][0])) +
        SQUARE((vertex_positions[batchedsprings[j][0]][1] - vertex_positions[batchedsprings[j][1]][1])) +
        SQUARE((vertex_positions[batchedsprings[j][0]][2] - vertex_positions[batchedsprings[j][1]][2])) );
#undef SQUARE
      spring_properties[j][0] = 1.0 * rest; //reset length 
      spring_properties[j][1] = 5000.0; //spring force
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
  delete[] vertex_colors;
  delete[] vertex_positions;
  delete[] side_vertex_indecies;
  
  delete[] side_triangle_indecies;
}

void generateDino(char *vertsfilename, char *springsfilename, char *colorsfilename, char *surfacefilename, char *surfacepfilename, char *surfacetfilename)
{
  FILE *verts_file;
  FILE *spring_file;
  FILE *color_file;
  FILE *surface_file;
  FILE *surfaceP_file;
  FILE *surfaceT_file;
  int num_vertecies = 0;
  int num_springs = 0;
  int num_colors = 0;
  int num_surftri = 0;
  int num_surfvert = 0;
  int num_surftet = 0;
  float (*vertex_positions)[4];
  int (*surface_tets)[4];
  int (*colinfo)[2];
  GLubyte (*vertex_colors)[4]; //used to color vertecies for coloring
  float (*vertex_init)[4];
  float (*nrm)[4];
  GLint *side_vertex_indecies; //used to fill IBO
  GLint *side_triangle_indecies; // used to fill IBO for triangles
  int side_vert_fill = 0;
  int side_triangle_fill = 0;
  int (*springs)[2]; //vertecies of springs
  int *spring_colors; //colors for spings listed in order of 'springs'
  int *batch_count;  //# of springs in each batch
  int *fill_count;
  int **batches; //array containing spring index from 'springs' in separate batches
  cl_int error;

  verts_file = fopen(vertsfilename,"r");
  if( !verts_file)
  {
	  printf(" Error reading vertex data\n");
	  return;
  }

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
  surface_file = fopen(surfacefilename,"r");
  if( !surface_file)
  {
	  printf(" Error reading surface input file\n");
	  return;
  }
  surfaceP_file = fopen(surfacepfilename,"r");
  if( !surfaceP_file)
  {
	  printf(" Error reading surface input file\n");
	  return;
  }
  surfaceT_file = fopen(surfacetfilename,"r");
  if( !surfaceT_file)
  {
	  printf(" Error reading surface input file\n");
	  return;
  }


  //// READ FILES
  fscanf(verts_file, "%d\n", &num_vertecies);
  fscanf(spring_file, "%d\n", &num_springs);
  fscanf(color_file, "%d\n", &num_colors);
  fscanf(surface_file, "%d\n", &num_surftri);
  fscanf(surfaceP_file, "%d\n", &num_surfvert);
  fscanf(surfaceT_file, "%d\n", &num_surftet);

  side_vertex_indecies = new GLint[num_surfvert*DINO_NUM];
  side_triangle_indecies = new GLint[3*DINO_NUM * num_surftri];
  nrm = new float[num_surftri*DINO_NUM][4];
  vertex_positions = new float[num_vertecies*DINO_NUM][4];
  vertex_colors = new GLubyte[num_vertecies*DINO_NUM][4];
  vertex_init = new float[num_vertecies*DINO_NUM][4];
  surface_tets = new int[num_surftet][4];
  colinfo = new int[num_surftet * num_surfvert][2];
  springs = new int[num_springs*DINO_NUM][2];
  spring_colors = new int[num_springs*DINO_NUM];
  batch_count = new int[num_colors];
  fill_count = new int[num_colors];
  for(int i = 0; i < num_colors; i++)
  {
    batch_count[i] = 0;
    fill_count[i] = 0;
  }
  
  for(int i = 0; i < num_vertecies; i++){
	  fscanf(verts_file, "%f %f %f\n", &vertex_positions[i][0], &vertex_positions[i][1], &vertex_positions[i][2]);

	  for(int j = 1; j < DINO_NUM; j++){
	  vertex_positions[i+(j*num_vertecies)][0] =vertex_positions[i][0];
	  vertex_positions[i+(j*num_vertecies)][1] =vertex_positions[i][1];
	  vertex_positions[i+(j*num_vertecies)][2] =vertex_positions[i][2];
	  vertex_positions[i+(j*num_vertecies)][3] =vertex_positions[i][3];
	  }

	  vertex_positions[i][3] = 1.0f;
	  vertex_positions[i][0] *=.3;
	  vertex_positions[i][1] *=.3;
	  vertex_positions[i][2] *=.3;
	  vertex_positions[i][1] += 2;
	  vertex_positions[i][0] -= 1.6;
	  int dd = .1;
	  for(int j = 1; j < DINO_NUM; j++){
		  if(j == 1){
	  vertex_positions[i+(j*num_vertecies)][3] = 1.0f;
	  vertex_positions[i+(j*num_vertecies)][0] *=-.3;
	  vertex_positions[i+(j*num_vertecies)][1] *=.3;
	  vertex_positions[i+(j*num_vertecies)][2] *=.3;
	  vertex_positions[i+(j*num_vertecies)][1] += 2;
      vertex_positions[i+(j*num_vertecies)][0] += 2;
		  }

		  if(j == 2){
	  vertex_positions[i+(j*num_vertecies)][3] = 1.0f;
	  vertex_positions[i+(j*num_vertecies)][0] *=-.3;
	  vertex_positions[i+(j*num_vertecies)][1] *=.3;
	  vertex_positions[i+(j*num_vertecies)][2] *=.3;
	  vertex_positions[i+(j*num_vertecies)][2] += 2;
	  vertex_positions[i+(j*num_vertecies)][1] += 2;
      vertex_positions[i+(j*num_vertecies)][0] += 2;
		  }
		  if(j == 3){
	  vertex_positions[i+(j*num_vertecies)][3] = 1.0f;
	  vertex_positions[i+(j*num_vertecies)][0] *=.3;
	  vertex_positions[i+(j*num_vertecies)][1] *=.3;
	  vertex_positions[i+(j*num_vertecies)][2] *=.3;
	  vertex_positions[i+(j*num_vertecies)][2] -= 2;
	  vertex_positions[i+(j*num_vertecies)][1] += 2;
      vertex_positions[i+(j*num_vertecies)][0] += 2;
		  }
		  if(j == 4){
	  vertex_positions[i+(j*num_vertecies)][3] = 1.0f;
	  vertex_positions[i+(j*num_vertecies)][0] *=.1;
	  vertex_positions[i+(j*num_vertecies)][1] *=.1;
	  vertex_positions[i+(j*num_vertecies)][2] *=.1;
	  vertex_positions[i+(j*num_vertecies)][2] += 2;
	  vertex_positions[i+(j*num_vertecies)][1] += 3;
      vertex_positions[i+(j*num_vertecies)][0] += 2;
		  }
		  if(j == 5){
	  vertex_positions[i+(j*num_vertecies)][3] = 1.0f;
	  vertex_positions[i+(j*num_vertecies)][0] *=.3;
	  vertex_positions[i+(j*num_vertecies)][1] *=.3;
	  vertex_positions[i+(j*num_vertecies)][2] *=.3;
	  vertex_positions[i+(j*num_vertecies)][2] += 2;
	  vertex_positions[i+(j*num_vertecies)][1] += 2;
      vertex_positions[i+(j*num_vertecies)][0] += 2;
		  }
		  
		  if(j > 5){
			  vertex_positions[i+(j*num_vertecies)][3] = 1.0f;
	  vertex_positions[i+(j*num_vertecies)][0] *=.2;
	  vertex_positions[i+(j*num_vertecies)][1] *=.2;
	  vertex_positions[i+(j*num_vertecies)][2] *=.2;
	  vertex_positions[i+(j*num_vertecies)][2] -= 2*dd;
	  vertex_positions[i+(j*num_vertecies)][1] += dd;
      vertex_positions[i+(j*num_vertecies)][0] += dd;
	  dd+= .1;
		  }
	  }

	  vertex_colors[i][0] = (i >>  0) & 0xFF;
	  vertex_colors[i][1] = (i >>  8) & 0xFF;
	  vertex_colors[i][2] = (i >> 16) & 0xFF;
	  vertex_colors[i][3] = (i >> 24) & 0xFF;

	  for(int j = 1; j < DINO_NUM; j++){
	  vertex_colors[i+(j*num_vertecies)][0] = (i+(j*num_vertecies) >>  0) & 0xFF;
	  vertex_colors[i+(j*num_vertecies)][1] = (i+(j*num_vertecies) >>  8) & 0xFF;
	  vertex_colors[i+(j*num_vertecies)][2] = (i+(j*num_vertecies) >> 16) & 0xFF;
	  vertex_colors[i+(j*num_vertecies)][3] = (i+(j*num_vertecies) >> 24) & 0xFF;
	  }
  }
  //printf("%f %f %f", vertex_positions[num_vertecies*2-1][0], vertex_positions[num_vertecies*2-1][1], vertex_positions[num_vertecies*2-1][2]);

  for(int i = 0; i < num_springs; i++)
  {
    fscanf(spring_file, "%d %d\n", &(springs[i][0]), &(springs[i][1]));
	fscanf(color_file, "%d\n", &(spring_colors[i]));
	batch_count[spring_colors[i]]++;
	for(int j = 1; j < DINO_NUM; j++){
	springs[i+(j*num_springs)][0] = springs[i][0]+(j*num_vertecies);
	springs[i+(j*num_springs)][1] = springs[i][1]+(j*num_vertecies);
	if(j % 2)
		spring_colors[i+(j*num_springs)] = spring_colors[i];
	else
		spring_colors[i+(j*num_springs)] = 23 - spring_colors[i];
	batch_count[spring_colors[i+(j*num_springs)]]++;
	}
  }
  fclose(spring_file);
  fclose(color_file);

  for(int i = 0; i < num_surftri * 3; i+=3){
	  int a,b,c;
	  fscanf(surface_file, "%d %d %d\n",&a,&b,&c);
	  side_triangle_indecies[i] = a;
	  side_triangle_indecies[i+1] =b;
	  side_triangle_indecies[i+2] = c;
	  for(int j = 1; j < DINO_NUM; j++){
	  side_triangle_indecies[i+(j*num_surftri*3)] = a+(j*num_vertecies);
	  side_triangle_indecies[i+(j*num_surftri*3) +1] = b+(j*num_vertecies);
	  side_triangle_indecies[i+(j*num_surftri*3) +2] = c+(j*num_vertecies);
	  }
  }
  
  for(int i =0; i < num_surftri*DINO_NUM; i++){
	  nrm[i][0] = 0.0;
	  nrm[i][1] = 0.0;
	  nrm[i][2] = 0.0;
	  nrm[i][3] = 0.0;
  }
 
  for(int i =0; i < num_surfvert; i++){
	  fscanf(surfaceP_file,"%d\n",&side_vertex_indecies[i]);
	  for(int j = 1; j < DINO_NUM; j++){
	  side_vertex_indecies[i+(j*num_surfvert)] = side_vertex_indecies[i]+(j*num_vertecies);
	  }
  }

  for(int i =0; i < num_surftet; i++){
	  fscanf(surfaceT_file,"%d %d %d %d\n", &surface_tets[i][0], &surface_tets[i][1],&surface_tets[i][2],&surface_tets[i][3]);
	  /*surface_tets[i+num_surftet][0] = surface_tets[i][0];
	  surface_tets[i+num_surftet][1] = surface_tets[i][1];
	  surface_tets[i+num_surftet][2] = surface_tets[i][2];
	  surface_tets[i+num_surftet][3] = surface_tets[i][3];*/
  }

  fclose(surface_file);
  fclose(surfaceP_file);
  fclose(surfaceT_file);

  int bn= 0;
  for(int i = 0; i < num_surftet; i++){
	  for(int j = 0; j < num_surfvert; j++){
		  colinfo[bn][0] = i;
		 // printf("%d\n",j);
		  colinfo[bn][1] = j;
		  bn++;
	  }
  }
  //printf("%d %d %d %d",num_surftet*num_surfvert ,bn, colinfo[num_surftet * num_surfvert -1][0], colinfo[num_surftet * num_surfvert -1][1]);
  
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

  for(int i = 0; i < num_springs; i++)
  {
	  for(int j = 1; j < DINO_NUM; j++){
    batches[spring_colors[i+(j*num_springs)]][fill_count[spring_colors[i+(j*num_springs)]]++]= i+(j*num_springs);
	  }
  }

  //STORE SYSTEM INFORMATION AND INFORMATION ONTO VIDEOCARD
  simulation.num_points = num_vertecies*DINO_NUM;
  simulation.num_batches = num_colors;
  simulation.num_draw_elements = num_surfvert*DINO_NUM;
  simulation.num_draw_triangles = 3 * num_surftri*DINO_NUM;
  simulation.num_stets = num_surftet;
  simulation.num_sverts = num_surfvert;
  simulation.num_realpoints = num_vertecies;

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

  //Create color buffer for coloring vertex points
  glGenBuffers(1, &(simulation.color_id_buffer));
  glBindBuffer(GL_ARRAY_BUFFER, simulation.color_id_buffer);
  glBufferData(GL_ARRAY_BUFFER, simulation.num_points * 4 * sizeof(GLubyte), vertex_colors, GL_STATIC_DRAW);

  glGenBuffers(1, &(simulation.triangle_buffer));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, simulation.triangle_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, simulation.num_draw_triangles * sizeof(GLint), side_triangle_indecies, GL_STATIC_DRAW);

  //Create drawing index buffer for drawing vertex points
  glGenBuffers(1, &(simulation.element_buffer));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, simulation.element_buffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, simulation.num_draw_elements * sizeof(GLint), side_vertex_indecies, GL_STATIC_DRAW);
   
  //if(error) printf("Position Buffer Error: %d", error);
  //Make vertex_init 0 so we can use them as source for position and velocity
  for(int i = 0; i < simulation.num_points; i++)
  {
    vertex_init[i][0] = 0.0;
    vertex_init[i][1] = 0.0;
    vertex_init[i][2] = 0.0;
    vertex_init[i][3] = 0.0;
  }

  simulation.surface_verts = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
	  sizeof(cl_int) * simulation.num_draw_elements, side_vertex_indecies, &error);
  if(error) printf("Stris buffer error %d",error);

  simulation.tets = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
	  sizeof(cl_int) * 4 * simulation.num_stets, surface_tets, &error);
  if(error) printf("Stets buffer error %d",error);

  simulation.col_info = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
	  sizeof(cl_int) * 2 * simulation.num_stets * simulation.num_sverts, colinfo, &error);
  if(error) printf("col_info buffer error %d",error);
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

  float pull_init[4] = {0.0, 0.0, 0.0, 0.0};
  simulation.pull_value = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                         sizeof(cl_float4), &pull_init, &error);
  if(error) printf("Pull Init Buffer Error: %d", error);

  simulation.pull_position = clCreateBuffer(cl_components.opencl_context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                         sizeof(cl_float4), &pull_init, &error);
  if(error) printf("Pull Position Buffer Error: %d", error);

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
#define SQUARE(x) (x*x)
      float rest = sqrt(
        SQUARE((vertex_positions[batchedsprings[j][0]][0] - vertex_positions[batchedsprings[j][1]][0])) +
        SQUARE((vertex_positions[batchedsprings[j][0]][1] - vertex_positions[batchedsprings[j][1]][1])) +
        SQUARE((vertex_positions[batchedsprings[j][0]][2] - vertex_positions[batchedsprings[j][1]][2])) );
#undef SQUARE
      spring_properties[j][0] = 1.0 * rest; //reset length 
      spring_properties[j][1] = 200000.0; //spring force
      spring_properties[j][2] = 100.0; //damepning force
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
  delete[] vertex_colors;
  delete[] vertex_positions;
  delete[] side_vertex_indecies;
  delete[] side_triangle_indecies;
  delete[] surface_tets;
}