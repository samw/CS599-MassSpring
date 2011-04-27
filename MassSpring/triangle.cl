#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
__kernel void clear_normal_kernel(__global float *normals)
{
  int normal = get_global_id(0);
  normals[normal] = 0.0;
}
__kernel void calculate_normal_kernel(__global float4 *vertecies, __global uint *triangles, __global int *normals)
{
  int vertex = get_global_id(0) * 3;
  int vertexA = triangles[vertex ];
  int vertexB = triangles[vertex + 1];
  int vertexC = triangles[vertex + 2];
  float4 AB = vertecies[vertexB] - vertecies[vertexA];
  float4 AC = vertecies[vertexC] - vertecies[vertexA];
  float4 normal = normalize(cross(AB, AC))*10000;
  int4 discretized_normal = convert_int4_rtz(normal);
  atom_add(&normals[vertexA*3+0], discretized_normal.s0);
  atom_add(&normals[vertexA*3+1], discretized_normal.s1);
  atom_add(&normals[vertexA*3+2], discretized_normal.s2);
  atom_add(&normals[vertexB*3+0], discretized_normal.s0);
  atom_add(&normals[vertexB*3+1], discretized_normal.s1);
  atom_add(&normals[vertexB*3+2], discretized_normal.s2);
  atom_add(&normals[vertexC*3+0], discretized_normal.s0);
  atom_add(&normals[vertexC*3+1], discretized_normal.s1);
  atom_add(&normals[vertexC*3+2], discretized_normal.s2);
}

__kernel void int_to_float_kernel(__global float *fnormals, __global int *normals)
{
  int normal = get_global_id(0);
  fnormals[normal] = convert_float(normals[normal]);
}
