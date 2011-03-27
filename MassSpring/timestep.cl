//Performs one step of Euler Integration
__kernel void euler_kernel(__global float4 *positions,
                           __global float4 *velocities,
                           __global float4 *accelerations,
                           float timestep)
{
  int vertex = get_global_id(0);
  positions[vertex] += timestep * velocities[vertex];
  velocities[vertex] += timestep * accelerations[vertex];
}

__kernel void midpoint_kernel_1(__global float4 *positions,
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

__kernel void midpoint_kernel_2(__global float4 *positions,
                                __global float4 *velocities,
                                __global float4 *accelerations,
						                    __global float4 *bufferV,
                                float timestep)
{
	int vertex = get_global_id(0);
	positions[vertex] += timestep * bufferV[vertex];
	velocities[vertex] += timestep * accelerations[vertex];
}

__kernel void rk4_kernel_1(__global float4 *positions,
                           __global float4 *velocities,
                           __global float4 *accelerations,
						   __global float4 *f1p,
						   __global float4 *f1v,
						   __global float4 *bufferP,
						   __global float4 *bufferV,
						   float timestep)
{
	int vertex = get_global_id(0);
	f1p[vertex] = velocities[vertex] * timestep;
	f1v[vertex] = accelerations[vertex] * timestep;
	bufferP[vertex] = f1p[vertex] * 0.5;
	bufferV[vertex] = f1v[vertex] * 0.5;
	bufferP[vertex] += positions[vertex];
	bufferV[vertex] += velocities[vertex];
}

__kernel void rk4_kernel_2(__global float4 *positions,
                           __global float4 *velocities,
                           __global float4 *accelerations,
						   __global float4 *f2p,
						   __global float4 *f2v,
						   __global float4 *bufferP,
						   __global float4 *bufferV,
						   float timestep)
{
	int vertex = get_global_id(0);
	f2p[vertex] = bufferV[vertex] * timestep;
	f2v[vertex] = accelerations[vertex] * timestep;
	bufferP[vertex] = f2p[vertex] * 0.5;
	bufferV[vertex] = f2v[vertex] * 0.5;
	bufferP[vertex] += positions[vertex];
	bufferV[vertex] += velocities[vertex];
}

__kernel void rk4_kernel_3(__global float4 *positions,
                           __global float4 *velocities,
                           __global float4 *accelerations,
						   __global float4 *f3p,
						   __global float4 *f3v,
						   __global float4 *bufferP,
						   __global float4 *bufferV,
						   float timestep)
{
	int vertex = get_global_id(0);
	f3p[vertex] = bufferV[vertex] * timestep;
	f3v[vertex] = accelerations[vertex] * timestep;
	bufferP[vertex] = f3p[vertex] * 0.5;
	bufferV[vertex] = f3v[vertex] * 0.5;
	bufferP[vertex] += positions[vertex];
	bufferV[vertex] += velocities[vertex];
}

__kernel void rk4_kernel_4(__global float4 *positions,
                           __global float4 *velocities,
                           __global float4 *accelerations,
						   __global float4 *f1p,
						   __global float4 *f1v,
						   __global float4 *f2p,
						   __global float4 *f2v,
						   __global float4 *f3p,
						   __global float4 *f3v,
						   __global float4 *f4p,
						   __global float4 *f4v,
						   __global float4 *bufferP,
						   __global float4 *bufferV,
						   float timestep)
{
	int vertex = get_global_id(0);
	f4p[vertex] = bufferV[vertex] * timestep;
	f4v[vertex] = accelerations[vertex] * timestep;
	bufferP[vertex] = 2 * f2p[vertex];
	bufferV[vertex] = 2 * f3p[vertex];
	bufferP[vertex] += bufferV[vertex];
	bufferP[vertex] += f1p[vertex];
	bufferP[vertex] += f4p[vertex];
	bufferP[vertex] = bufferP[vertex] * (1.0/ 6.0);
	positions[vertex] += bufferP[vertex];
	bufferP[vertex] = 2 * f2v[vertex];
	bufferV[vertex] = 2 * f3v[vertex];
	bufferP[vertex] += bufferV[vertex];
	bufferP[vertex] += f1v[vertex];
	bufferP[vertex] += f4v[vertex];
	bufferP[vertex] = bufferP[vertex] * (1.0/ 6.0);
	velocities[vertex] += bufferP[vertex];
}