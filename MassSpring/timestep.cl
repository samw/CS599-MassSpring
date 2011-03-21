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

void __kernel midpoint_kernel_1(__global float4 *positions,
                                __global float4 *velocities,
                                __global float4 *accelerations,
						                    __global float4 *bufferP,
						                    __global float4 *bufferV,
                                float timestep)
{
	int vertex = get_global_id(0);
	bufferP[vertex] = positions[vertex];
	bufferP[vertex] += timestep * velocities[vertex];
	bufferV[vertex] = velocities[vertex];
	bufferV[vertex] += timestep * accelerations[vertex];
}

void __kernel midpoint_kernel_2(__global float4 *positions,
                                __global float4 *velocities,
                                __global float4 *accelerations,
						                    __global float4 *bufferV,
                                float timestep)
{
	int vertex = get_global_id(0);
	positions[vertex] += timestep * bufferV[vertex];
	velocities[vertex] += timestep * accelerations[vertex];
}