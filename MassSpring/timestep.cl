//Performs one step of Euler Integration
void __kernel euler_kernel(__global float4 *positions,
                           __global float4 *velocities,
                           __global float4 *accelerations,
                           float timestep)
{
  int vertex = get_global_id(0);
  positions[vertex] += timestep * velocities[vertex];
  velocities[vertex] += timestep * accelerations[vertex];
}