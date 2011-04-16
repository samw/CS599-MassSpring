__kernel void pull_vertex(__global float4 *position, __global float4 *velocity, __global float4 *acceleration, int index, float4 camera_position, float4 camera_forward)
{
  camera_forward = normalize(camera_forward);
  float4 buffer = position[index] - camera_position;
  float dotp = dot(camera_forward, buffer);
  buffer = camera_position + (camera_forward * dotp);

  float khook = 2000;
  float kdamp = 10;

  float4 diff, vdiff;
  float len, forcemagnitude, dampingmagnitude;
  diff = position[index] - buffer;
  vdiff = velocity[index];
  dampingmagnitude= dot(vdiff, diff);
  len = length(diff);
  if( len == 0.0 ) return;
  diff = normalize(diff);
  forcemagnitude = -khook * (len);
  forcemagnitude += -kdamp*dampingmagnitude/len;
  diff = diff * forcemagnitude;
  acceleration[index] += diff;
}