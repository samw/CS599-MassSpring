#include "CLFunctions.h"
#include "MassSpring.h"
#include "input.h"
#include "RenderSystem.h"
#include <stdio.h>



#define TS .008

struct cl_system cl_components;

void configureTestKernel()
{
  //acceleration kernel init
  cl_float initvector[4] = {0.0,0.0,0.0,0.0};
  clSetKernelArg(cl_components.acceleration_kernel, 0, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.acceleration_kernel, 1, sizeof(cl_float4), initvector);
  
  //spring kernel init
  clSetKernelArg(cl_components.single_spring_kernel, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.single_spring_kernel, 1, sizeof(cl_uint), &(simulation.velocity));
  clSetKernelArg(cl_components.single_spring_kernel, 2, sizeof(cl_uint), &(simulation.acceleration));
  
  //spring batch kernel init
  clSetKernelArg(cl_components.batch_spring_kernel, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.batch_spring_kernel, 1, sizeof(cl_uint), &(simulation.velocity));
  clSetKernelArg(cl_components.batch_spring_kernel, 2, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.batch_spring_kernel, 3, sizeof(cl_uint), &(simulation.springs));
  clSetKernelArg(cl_components.batch_spring_kernel, 4, sizeof(cl_uint), &(simulation.springProperties));
  
  //euler kernel init
  float timestep = TS;
  clSetKernelArg(cl_components.euler_kernel, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.euler_kernel, 1, sizeof(cl_uint), &(simulation.velocity));
  clSetKernelArg(cl_components.euler_kernel, 2, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.euler_kernel, 3, sizeof(cl_float), &timestep);
}

void runTestKernel()
{
  float rest;
  float spring;
  float damp;
  glFinish();
  clEnqueueAcquireGLObjects(cl_components.command_queue, 1, &(simulation.position), 0, NULL, NULL);

  //zero accelerations for first two points
  size_t numvertecies[1] = {simulation.num_points};
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.acceleration_kernel,
                         1, NULL, numvertecies, NULL, 0, NULL, NULL);
  clFinish(cl_components.command_queue);

  //Calculate interaction for a single spring between first two points
  size_t springworksize[1] = {1};
  rest = 0.25;
  spring = 100.0;
  damp = 2.0;
  cl_int pointa = 0;
  cl_int pointb = 1;
  clSetKernelArg(cl_components.single_spring_kernel, 3, sizeof(cl_int), &pointa);
  clSetKernelArg(cl_components.single_spring_kernel, 4, sizeof(cl_int), &pointb);
  clSetKernelArg(cl_components.single_spring_kernel, 5, sizeof(cl_float), &rest);
  clSetKernelArg(cl_components.single_spring_kernel, 6, sizeof(cl_float), &spring);
  clSetKernelArg(cl_components.single_spring_kernel, 7, sizeof(cl_float), &damp);

  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.single_spring_kernel,
                         1, NULL, springworksize, NULL, 0, NULL, NULL);
  clFinish(cl_components.command_queue);
  
  //size_t springbatchsize[1] = {1};
  //clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.batch_spring_kernel,
  //                       1, NULL, springbatchsize, NULL, 0, NULL, NULL);
  //clFinish(cl_components.command_queue);

  size_t eulerworksize[1] = {simulation.num_points};
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.euler_kernel,
                         1, NULL, eulerworksize, NULL, 0, NULL, NULL);
  clFinish(cl_components.command_queue);

  clEnqueueReleaseGLObjects(cl_components.command_queue, 1, &(simulation.position), 0, NULL, NULL);
  clFinish(cl_components.command_queue);
}

void configureTestKernelMidPoint()
{
  //acceleration kernel
  cl_float initvector[4] = {0.0,0.0,0.0,0.0};
  clSetKernelArg(cl_components.acceleration_kernel, 0, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.acceleration_kernel, 1, sizeof(cl_float4), initvector);

  //spring kernel (non-changing arguments)
  clSetKernelArg(cl_components.batch_spring_kernel, 2, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.batch_spring_kernel, 3, sizeof(cl_uint), &(simulation.springs));
  clSetKernelArg(cl_components.batch_spring_kernel, 4, sizeof(cl_uint), &(simulation.springProperties));
  
  //midpoint1 kernel
  float timestep = TS /2;
  clSetKernelArg(cl_components.midpoint_kernel_1, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.midpoint_kernel_1, 1, sizeof(cl_uint), &(simulation.velocity));
  clSetKernelArg(cl_components.midpoint_kernel_1, 2, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.midpoint_kernel_1, 3, sizeof(cl_uint), &(simulation.bufferP));
  clSetKernelArg(cl_components.midpoint_kernel_1, 4, sizeof(cl_uint), &(simulation.bufferV));
  clSetKernelArg(cl_components.midpoint_kernel_1, 5, sizeof(cl_float), &timestep);

  //midpoint2 kernel
  timestep = TS;
  clSetKernelArg(cl_components.midpoint_kernel_2, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.midpoint_kernel_2, 1, sizeof(cl_uint), &(simulation.velocity));
  clSetKernelArg(cl_components.midpoint_kernel_2, 2, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.midpoint_kernel_2, 3, sizeof(cl_uint), &(simulation.bufferV));
  clSetKernelArg(cl_components.midpoint_kernel_2, 4, sizeof(cl_float), &timestep);
}

void runTestKernelMidPoint()
{
  glFinish();
  clEnqueueAcquireGLObjects(cl_components.command_queue, 1, &(simulation.position), 0, NULL, NULL);

  //zero accelerations for first two points
  size_t numvertecies[1] = {simulation.num_points};
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.acceleration_kernel,
                         1, NULL, numvertecies, NULL, 0, NULL, NULL);
  clFinish(cl_components.command_queue);

  clSetKernelArg(cl_components.batch_spring_kernel, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.batch_spring_kernel, 1, sizeof(cl_uint), &(simulation.velocity));

  for(int i = 0; i < simulation.num_batches; i++)
  {
    size_t springbatchsize[1] = {simulation.batch_sizes[i]};
    clSetKernelArg(cl_components.batch_spring_kernel, 3, sizeof(cl_uint), &(simulation.springBatches[i]));
    clSetKernelArg(cl_components.batch_spring_kernel, 4, sizeof(cl_uint), &(simulation.springPropertyBatches[i]));
    clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.batch_spring_kernel,
      1, NULL, springbatchsize, NULL, 0, NULL, NULL);
    clFinish(cl_components.command_queue);
  }

  //timestep to the midpoint at time t + TS/2
  size_t midpointworksize[1] = {simulation.num_points};
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.midpoint_kernel_1,
                         1, NULL, midpointworksize, NULL, 0, NULL, NULL);
  clFinish(cl_components.command_queue);

  //rezero acceleration for next calculation
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.acceleration_kernel,
                         1, NULL, numvertecies, NULL, 0, NULL, NULL);
  clFinish(cl_components.command_queue);
  
  clSetKernelArg(cl_components.batch_spring_kernel, 0, sizeof(cl_uint), &(simulation.bufferP));
  clSetKernelArg(cl_components.batch_spring_kernel, 1, sizeof(cl_uint), &(simulation.bufferV));

  for(int i = 0; i < simulation.num_batches; i++)
  {
    size_t springbatchsize[1] = {simulation.batch_sizes[i]};
    clSetKernelArg(cl_components.batch_spring_kernel, 3, sizeof(cl_uint), &(simulation.springBatches[i]));
    clSetKernelArg(cl_components.batch_spring_kernel, 4, sizeof(cl_uint), &(simulation.springPropertyBatches[i]));
    clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.batch_spring_kernel,
      1, NULL, springbatchsize, NULL, 0, NULL, NULL);
    clFinish(cl_components.command_queue);
  }

  //timestep to the final point
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.midpoint_kernel_2,
                         1, NULL, midpointworksize, NULL, 0, NULL, NULL);
  clFinish(cl_components.command_queue);

  clEnqueueReleaseGLObjects(cl_components.command_queue, 1, &(simulation.position), 0, NULL, NULL);
  clFinish(cl_components.command_queue);
}

void configureCubeTestKernel()
{
  //acceleration kernel init
  cl_float initvector[4] = {0.0,0.0,0.0,0.0};
  clSetKernelArg(cl_components.acceleration_kernel, 0, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.acceleration_kernel, 1, sizeof(cl_float4), initvector);
  
  //spring batch kernel init
  clSetKernelArg(cl_components.batch_spring_kernel, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.batch_spring_kernel, 1, sizeof(cl_uint), &(simulation.velocity));
  clSetKernelArg(cl_components.batch_spring_kernel, 2, sizeof(cl_uint), &(simulation.acceleration));
  
  //euler kernel init
  float timestep = TS;
  clSetKernelArg(cl_components.euler_kernel, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.euler_kernel, 1, sizeof(cl_uint), &(simulation.velocity));
  clSetKernelArg(cl_components.euler_kernel, 2, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.euler_kernel, 3, sizeof(cl_float), &timestep);
}

void runCubeTestKernel()
{
  glFinish();
  clEnqueueAcquireGLObjects(cl_components.command_queue, 1, &(simulation.position), 0, NULL, NULL);

  //zero accelerations for first two points
  size_t numvertecies[1] = {simulation.num_points};
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.acceleration_kernel,
                         1, NULL, numvertecies, NULL, 0, NULL, NULL);
  clFinish(cl_components.command_queue);

  for(int i = 0; i < simulation.num_batches; i++)
  {
    size_t springbatchsize[1] = {simulation.batch_sizes[i]};
    clSetKernelArg(cl_components.batch_spring_kernel, 3, sizeof(cl_uint), &(simulation.springBatches[i]));
    clSetKernelArg(cl_components.batch_spring_kernel, 4, sizeof(cl_uint), &(simulation.springPropertyBatches[i]));
    clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.batch_spring_kernel,
      1, NULL, springbatchsize, NULL, 0, NULL, NULL);
    clFinish(cl_components.command_queue);
  }

  size_t eulerworksize[1] = {simulation.num_points};
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.euler_kernel,
                         1, NULL, eulerworksize, NULL, 0, NULL, NULL);
  clFinish(cl_components.command_queue);

  clEnqueueReleaseGLObjects(cl_components.command_queue, 1, &(simulation.position), 0, NULL, NULL);
  clFinish(cl_components.command_queue);
}

void configureTestKernelRK4()
{
	//acceleration kernel
  cl_float initvector[4] = {0.0,0.0,0.0,0.0};
  clSetKernelArg(cl_components.acceleration_kernel, 0, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.acceleration_kernel, 1, sizeof(cl_float4), initvector);

  //set accelration kernel
  clSetKernelArg(cl_components.pull_vertex_kernel, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.pull_vertex_kernel, 1, sizeof(cl_uint), &(simulation.velocity));
  clSetKernelArg(cl_components.pull_vertex_kernel, 2, sizeof(cl_uint), &(simulation.acceleration));

  //spring kernel (non-changing arguments)
  clSetKernelArg(cl_components.batch_spring_kernel, 2, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.batch_spring_kernel, 3, sizeof(cl_uint), &(simulation.springs));
  clSetKernelArg(cl_components.batch_spring_kernel, 4, sizeof(cl_uint), &(simulation.springProperties));

  float timestep = TS;
  //kernel 1
  clSetKernelArg(cl_components.rk4_kernel_1, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.rk4_kernel_1, 1, sizeof(cl_uint), &(simulation.velocity));
  clSetKernelArg(cl_components.rk4_kernel_1, 2, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.rk4_kernel_1, 3, sizeof(cl_uint), &(simulation.f1p));
  clSetKernelArg(cl_components.rk4_kernel_1, 4, sizeof(cl_uint), &(simulation.f1v));
  clSetKernelArg(cl_components.rk4_kernel_1, 5, sizeof(cl_uint), &(simulation.bufferP));
  clSetKernelArg(cl_components.rk4_kernel_1, 6, sizeof(cl_uint), &(simulation.bufferV));
  clSetKernelArg(cl_components.rk4_kernel_1, 7, sizeof(cl_float), &timestep);
  
  //kernel 2
  clSetKernelArg(cl_components.rk4_kernel_2, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.rk4_kernel_2, 1, sizeof(cl_uint), &(simulation.velocity));
  clSetKernelArg(cl_components.rk4_kernel_2, 2, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.rk4_kernel_2, 3, sizeof(cl_uint), &(simulation.f2p));
  clSetKernelArg(cl_components.rk4_kernel_2, 4, sizeof(cl_uint), &(simulation.f2v));
  clSetKernelArg(cl_components.rk4_kernel_2, 5, sizeof(cl_uint), &(simulation.bufferP));
  clSetKernelArg(cl_components.rk4_kernel_2, 6, sizeof(cl_uint), &(simulation.bufferV));
  clSetKernelArg(cl_components.rk4_kernel_2, 7, sizeof(cl_float), &timestep);

  //kernel 3
  clSetKernelArg(cl_components.rk4_kernel_3, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.rk4_kernel_3, 1, sizeof(cl_uint), &(simulation.velocity));
  clSetKernelArg(cl_components.rk4_kernel_3, 2, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.rk4_kernel_3, 3, sizeof(cl_uint), &(simulation.f3p));
  clSetKernelArg(cl_components.rk4_kernel_3, 4, sizeof(cl_uint), &(simulation.f3v));
  clSetKernelArg(cl_components.rk4_kernel_3, 5, sizeof(cl_uint), &(simulation.bufferP));
  clSetKernelArg(cl_components.rk4_kernel_3, 6, sizeof(cl_uint), &(simulation.bufferV));
  clSetKernelArg(cl_components.rk4_kernel_3, 7, sizeof(cl_float), &timestep);

  //kernel 4
  clSetKernelArg(cl_components.rk4_kernel_4, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.rk4_kernel_4, 1, sizeof(cl_uint), &(simulation.velocity));
  clSetKernelArg(cl_components.rk4_kernel_4, 2, sizeof(cl_uint), &(simulation.acceleration));
  clSetKernelArg(cl_components.rk4_kernel_4, 3, sizeof(cl_uint), &(simulation.f1p));
  clSetKernelArg(cl_components.rk4_kernel_4, 4, sizeof(cl_uint), &(simulation.f1v));
  clSetKernelArg(cl_components.rk4_kernel_4, 5, sizeof(cl_uint), &(simulation.f2p));
  clSetKernelArg(cl_components.rk4_kernel_4, 6, sizeof(cl_uint), &(simulation.f2v));
  clSetKernelArg(cl_components.rk4_kernel_4, 7, sizeof(cl_uint), &(simulation.f3p));
  clSetKernelArg(cl_components.rk4_kernel_4, 8, sizeof(cl_uint), &(simulation.f3v));
  clSetKernelArg(cl_components.rk4_kernel_4, 9, sizeof(cl_uint), &(simulation.f4p));
  clSetKernelArg(cl_components.rk4_kernel_4, 10, sizeof(cl_uint), &(simulation.f4v));
  clSetKernelArg(cl_components.rk4_kernel_4, 11, sizeof(cl_uint), &(simulation.bufferP));
  clSetKernelArg(cl_components.rk4_kernel_4, 12, sizeof(cl_uint), &(simulation.bufferV));
  clSetKernelArg(cl_components.rk4_kernel_4, 13, sizeof(cl_float), &timestep);
}

void runTestKernelRK4()
{
  cl_event lastevent;
  cl_event currentevent;
  int numstepsperframe = 1;

  glFinish();
  for(int t = 0; t < numstepsperframe; t++)
  {

  clEnqueueAcquireGLObjects(cl_components.command_queue, 1, &(simulation.position), 0, NULL, &lastevent);

  //zero accelerations
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.acceleration_kernel,
                         1, NULL, (size_t *) &simulation.num_points, NULL, 1, &lastevent, &currentevent);
  clReleaseEvent(lastevent);
  lastevent = currentevent;

  //Apply acceleration if we are dragging a vertex around
  if(simulation.vertex_pulling == 3)
  {
    //printf("Pulling: ");
    double mouse[3];
    double mouse2[3];
    double modelview[16];
    double proj[16];
    int view[4];
    float cam_forward[4];
    float mouse_position[4];

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, view);

    clSetKernelArg(cl_components.pull_vertex_kernel, 3, sizeof(cl_int), &(simulation.last_vertex_selected));

    gluUnProject(input_state.mouseX, main_window.height - input_state.mouseY, 0.0,
      modelview, proj, view, &mouse[0], &mouse[1], &mouse[2]);
    //printf("%f, %f, %f\n", mouse[0], mouse[1], mouse[2]);
    mouse_position[0] = mouse[0];
    mouse_position[1] = mouse[1];
    mouse_position[2] = mouse[2];
    mouse_position[3] = 1.0;
    clSetKernelArg(cl_components.pull_vertex_kernel, 4, sizeof(cl_float4), (&mouse_position));

    gluUnProject(input_state.mouseX, main_window.height - input_state.mouseY, 10.0,
      modelview, proj, view, &mouse2[0], &mouse2[1], &mouse2[2]);
    cam_forward[0] = mouse2[0] - mouse[0];
    cam_forward[1] = mouse2[1] - mouse[1];
    cam_forward[2] = mouse2[2] - mouse[2];
    cam_forward[3] = 0.0;
    clSetKernelArg(cl_components.pull_vertex_kernel, 5, sizeof(cl_float4), &(cam_forward));

    //actually run the kernel for the specified mouse
    int work_size[1] = {1};
    clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.pull_vertex_kernel,
                         1, NULL, (size_t *) &work_size, NULL, 1, &lastevent, &currentevent);
    clReleaseEvent(lastevent);
    lastevent = currentevent;
  }

  clSetKernelArg(cl_components.batch_spring_kernel, 0, sizeof(cl_uint), &(simulation.position));
  clSetKernelArg(cl_components.batch_spring_kernel, 1, sizeof(cl_uint), &(simulation.velocity));
  
  for(int i = 0; i < simulation.num_batches; i++)
  {
    clSetKernelArg(cl_components.batch_spring_kernel, 3, sizeof(cl_uint), &(simulation.springBatches[i]));
    clSetKernelArg(cl_components.batch_spring_kernel, 4, sizeof(cl_uint), &(simulation.springPropertyBatches[i]));
    clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.batch_spring_kernel,
      1, NULL, (size_t*) &simulation.batch_sizes[i], NULL, 1, &lastevent, &currentevent);
    clReleaseEvent(lastevent);
    lastevent = currentevent;
  }

  clSetKernelArg(cl_components.batch_spring_kernel, 0, sizeof(cl_uint), &(simulation.bufferP));
  clSetKernelArg(cl_components.batch_spring_kernel, 1, sizeof(cl_uint), &(simulation.bufferV));

  //part 1 of rk4
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.rk4_kernel_1,
						1,NULL, (size_t *) &simulation.num_points, NULL, 1, &lastevent, &currentevent);
  clReleaseEvent(lastevent);
  lastevent = currentevent;

  //zero accel
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.acceleration_kernel,
                         1, NULL, (size_t *) &simulation.num_points, NULL, 1, &lastevent, &currentevent);
  clReleaseEvent(lastevent);
  lastevent = currentevent;
  //Apply mouse pull
  if(simulation.vertex_pulling == 3)
  {
    int work_size[1] = {1};
    clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.pull_vertex_kernel,
      1, NULL, (size_t *) &work_size, NULL, 1, &lastevent, &currentevent);
    clReleaseEvent(lastevent);
    lastevent = currentevent;
  }


  //recompute acceleration
  for(int i = 0; i < simulation.num_batches; i++)
  {
    clSetKernelArg(cl_components.batch_spring_kernel, 3, sizeof(cl_uint), &(simulation.springBatches[i]));
    clSetKernelArg(cl_components.batch_spring_kernel, 4, sizeof(cl_uint), &(simulation.springPropertyBatches[i]));
    clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.batch_spring_kernel,
      1, NULL, (size_t*) &simulation.batch_sizes[i], NULL, 1, &lastevent, &currentevent);
    clReleaseEvent(lastevent);
    lastevent = currentevent;
  }

  //part 2 of rk4
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.rk4_kernel_2,
						1,NULL, (size_t *) &simulation.num_points, NULL, 1, &lastevent, &currentevent);
  clReleaseEvent(lastevent);
  lastevent = currentevent;

  //zero accel
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.acceleration_kernel,
                         1, NULL, (size_t *) &simulation.num_points, NULL, 1, &lastevent, &currentevent);
  clReleaseEvent(lastevent);
  lastevent = currentevent;
  //Apply mouse pull
  if(simulation.vertex_pulling == 3)
  {
    int work_size[1] = {1};
    clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.pull_vertex_kernel,
      1, NULL, (size_t *) &work_size, NULL, 1, &lastevent, &currentevent);
    clReleaseEvent(lastevent);
    lastevent = currentevent;
  }

  //recompute acceleration
  for(int i = 0; i < simulation.num_batches; i++)
  {
    clSetKernelArg(cl_components.batch_spring_kernel, 3, sizeof(cl_uint), &(simulation.springBatches[i]));
    clSetKernelArg(cl_components.batch_spring_kernel, 4, sizeof(cl_uint), &(simulation.springPropertyBatches[i]));
    clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.batch_spring_kernel,
      1, NULL, (size_t*) &simulation.batch_sizes[i], NULL, 1, &lastevent, &currentevent);
    clReleaseEvent(lastevent);
    lastevent = currentevent;
  }

  //part 3 of rk4
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.rk4_kernel_3,
						1,NULL, (size_t *) &simulation.num_points, NULL, 1, &lastevent, &currentevent);
  clReleaseEvent(lastevent);
  lastevent = currentevent;

  //zero accel
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.acceleration_kernel,
                         1, NULL, (size_t *) &simulation.num_points, NULL, 1, &lastevent, &currentevent);
  clReleaseEvent(lastevent);
  lastevent = currentevent;
  //Apply mouse pull
  if(simulation.vertex_pulling == 3)
  {
    int work_size[1] = {1};
    clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.pull_vertex_kernel,
      1, NULL, (size_t *) &work_size, NULL, 1, &lastevent, &currentevent);
    clReleaseEvent(lastevent);
    lastevent = currentevent;
  }

  //recompute acceleration
  for(int i = 0; i < simulation.num_batches; i++)
  {
    clSetKernelArg(cl_components.batch_spring_kernel, 3, sizeof(cl_uint), &(simulation.springBatches[i]));
    clSetKernelArg(cl_components.batch_spring_kernel, 4, sizeof(cl_uint), &(simulation.springPropertyBatches[i]));
    clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.batch_spring_kernel,
      1, NULL, (size_t*) &simulation.batch_sizes[i], NULL, 1, &lastevent, &currentevent);
    clReleaseEvent(lastevent);
    lastevent = currentevent;
  }

  //part 4 of rk4
  clEnqueueNDRangeKernel(cl_components.command_queue, cl_components.rk4_kernel_4,
						1,NULL, (size_t *) &simulation.num_points, NULL, 1, &lastevent, &currentevent);
  clReleaseEvent(lastevent);
  lastevent = currentevent;


  clEnqueueReleaseGLObjects(cl_components.command_queue, 1, &(simulation.position), 1, &lastevent, NULL);
  clReleaseEvent(lastevent);
  clFinish(cl_components.command_queue);
  }
}

bool initOpenCL()
{
  cl_int error;
  char buffer[100];

  //Enumerate Platforms
  cl_uint max_platforms = 10;
  cl_uint num_platforms = 0;
  cl_platform_id * platforms;
  platforms = new cl_platform_id[max_platforms];
  error = clGetPlatformIDs(max_platforms, platforms, &num_platforms);
  if(num_platforms == 0)
  {
    printf("no open cl devices");
    return false;
  }
  printf("OpenCL Platforms:\n");
  for(unsigned int i = 0; i < num_platforms; i++)
  {
    error = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 100, buffer, NULL);
    printf(buffer);
    printf(": ");
    error = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, 100, buffer, NULL);
    printf(buffer);
    printf(" (%d) \n", platforms[i]);
  }

  //Enumerate Devices
  cl_uint max_devices = 10;
  cl_uint num_devices = 0;
  cl_device_id * devices;
  devices = new cl_device_id[max_devices];
  error = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, max_devices, devices, &num_devices);
  if(num_devices == 0)
  {
    printf("no GPU OpenCL devices");
    return false;
  }
  printf("Devices on Platform %d:\n", platforms[0]);
  for(unsigned int i = 0; i < num_platforms; i++)
  {
    error = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 100, buffer, NULL);
    printf(buffer);
    printf("\n");
  }

  //Create CL context using platform 0
  cl_context_properties context_properties[7] =
  {
    CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
    CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
    CL_CONTEXT_PLATFORM,(cl_context_properties)platforms[0],
    0
  };
  cl_components.opencl_context = clCreateContext(context_properties, 1, devices, NULL, NULL, &error);
  if(error)
  {
    printf("OpenCL Error: %d\n", error);
  }
  cl_components.command_queue = clCreateCommandQueue(cl_components.opencl_context, devices[0], NULL, &error);
  if(error)
  {
    printf("OpenCL Error: %d\n", error);
  }
  
  //Load all kernels from cl code files
  //char *move_kernel_names[1] = {"move_vertex"};
  //cl_kernel *move_kernels[1] = {&cl_components.opencl_kernel};
  //loadCLCodeFile("move.cl", cl_components.opencl_context, num_devices, devices, 1,
  //               move_kernel_names, move_kernels);

  char *acceleration_kernel_names[1] = {"acceleration_kernel"};
  cl_kernel *acceleration_kernels[1] = {&cl_components.acceleration_kernel};
  loadCLCodeFile("accelinit.cl", cl_components.opencl_context, num_devices, devices, 1,
                 acceleration_kernel_names, acceleration_kernels);
  
  char *spring_kernel_names[2] = {"spring_kernel_single", "spring_kernel_batch"};
  cl_kernel *spring_kernels[2] = {&cl_components.single_spring_kernel, &cl_components.batch_spring_kernel};
  loadCLCodeFile("spring_kernel.cl", cl_components.opencl_context, num_devices, devices, 2,
                 spring_kernel_names, spring_kernels);

  char *step_kernel_names[7] = {"euler_kernel", "midpoint_kernel_1", "midpoint_kernel_2","rk4_kernel_1","rk4_kernel_2","rk4_kernel_3","rk4_kernel_4"};
  cl_kernel *step_kernels[7] = {&cl_components.euler_kernel, &cl_components.midpoint_kernel_1, &cl_components.midpoint_kernel_2, &cl_components.rk4_kernel_1,
	  &cl_components.rk4_kernel_2, &cl_components.rk4_kernel_3, &cl_components.rk4_kernel_4};
  loadCLCodeFile("timestep.cl", cl_components.opencl_context, num_devices, devices, 7,
                 step_kernel_names, step_kernels);

  char *set_accleration_names[1] = {"pull_vertex"};
  cl_kernel *set_accleration_kernels[1] = {&cl_components.pull_vertex_kernel};
  loadCLCodeFile("set_acceleration.cl", cl_components.opencl_context, num_devices, devices, 1,
                 set_accleration_names, set_accleration_kernels);

  delete platforms;
  delete devices;
  
  return true;
}

bool loadCLCodeFile(char *file, cl_context context, int num_devices, cl_device_id *devices,
                    int num_kernels, char **kernel_names, cl_kernel **kernels)
{
  cl_int error = 0;
  //Compile source and create kernel
  size_t source_size;
  char source[10000];
  const char *sourcelist[1] = {source};

  printf("Compiling file %s\n", file);

  cl_program program = NULL;
  source_size = readFile(file, source);
  if(source_size == 0)
  {
    printf("Source File Read Error\n");
    return false;
  }
  program = clCreateProgramWithSource(context, 1, sourcelist, &source_size, &error);
  if(error)
  {
    printf("Program Creation Error: %d\n", error);
    return false;
  }
  error = clBuildProgram(program, num_devices, devices, "", NULL, NULL);
  if(error)
  {
    char buildinfo[1000];
    size_t outputsize;
    printf("Compile Error for kernel: %d\n", error);
    clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 1000, buildinfo, &outputsize);
    fwrite(buildinfo, min(outputsize, 1000), 1, stdout);
    printf("\n");
    return false;
  }
  //Create kernel from program
  for(int i = 0; i < num_kernels; i++)
  {
    *kernels[i] = clCreateKernel(program, kernel_names[i], &error);
    if(error)
    {
      printf("Kernel Error: %d\n", error);
    }
  }
  return true;
}

unsigned int readFile(char *path, char *buffer)
{
  unsigned int size;
  FILE *input;
  input = fopen(path, "rb");
  if(!input) return 0;
  fseek(input, 0, SEEK_END);
  size = ftell(input);
  rewind(input);
  size = fread(buffer, sizeof(char), size, input);
  fclose(input);
  return size;
}
