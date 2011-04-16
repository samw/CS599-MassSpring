#ifndef _MASSSPRING_H_
#define _MASSSPRING_H_

#include <cl\opencl.h>
#include "opengl_headers.h"

struct system
{
  int num_points;
  int num_draw_elements;
  int num_draw_triangles;
  GLuint position_buffer; //Buffer for vertex positions of all vertecies
  GLuint element_buffer;  //Buffer for vertecies on outside of model (used for drawing side as dots)
  GLuint color_id_buffer; //Buffer to color vertecies for mouse choosing
  GLuint triangle_buffer; //Buffer for vertex triplets creating triangles
  int last_vertex_selected;
  int vertex_pulling;

  cl_mem position;
  cl_mem velocity;
  cl_mem acceleration;
  cl_mem springs;
  cl_mem springProperties;

  int num_batches;
  int *batch_sizes;
  cl_mem *springBatches;
  cl_mem *springPropertyBatches;

  cl_mem bufferP;
  cl_mem bufferV;
  cl_mem f1p;
  cl_mem f1v;
  cl_mem f2p;
  cl_mem f2v;
  cl_mem f3p;
  cl_mem f3v;
  cl_mem f4p;
  cl_mem f4v;
};
extern struct system simulation;

void initSystem();

#endif