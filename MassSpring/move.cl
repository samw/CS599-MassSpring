//Simple kernet that takes a vertex buffer and moves it.

__kernel void move_vertex(__global float4* vertex, float4 direction, float dt)
{
  unsigned int x = get_global_id(0);
  fast_normalize(direction);
  vertex[x] = vertex[x] + direction * x * dt;
}