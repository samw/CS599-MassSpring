__kernel void acceleration_kernel(__global float4 *acceleration, float4 initializer)
{
   int element = get_global_id(0);
   acceleration[element] = initializer;
}

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

__kernel void calculate_pull_acceleration(__global float4 *position, __global float4 *velocity, __global float4 *pull_point, __global float4 *pull_acceleration, int index, float4 camera_position, float4 camera_forward)
{
  *pull_point = position[index];
  camera_forward = normalize(camera_forward);
  float4 buffer = position[index] - camera_position;
  float dotp = dot(camera_forward, buffer);
  buffer = camera_position + (camera_forward * dotp);

  float khook = 1000;
  float kdamp = 5.0;

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
  if(forcemagnitude > 80) forcemagnitude = 80;
  diff = diff * forcemagnitude;
  *pull_acceleration = diff;
}

__kernel void init_acceleration_kernel(
  __global float4 *positions, global float4 *velocitites, __global float4 *acceleration,
  __global float4 *pull_position, __global float4 *pull_value, float4 initializer)
{
   int element = get_global_id(0);
   float maxdistance = 0.2;
   float dist = distance( *pull_position, positions[element] );
   float4 pclamp;
   pclamp.s0 = clamp(positions[element].s0, -5.0, 5.0); 
   pclamp.s1 = clamp(positions[element].s1, -0.0, 5.0); 
   pclamp.s2 = clamp(positions[element].s2, -5.0, 5.0); 
   pclamp.s3 = 1.0;
   if( dist < maxdistance)
   {
     acceleration[element] = *pull_value;
     if(length(velocitites[element]) > 2.0)
     {
       velocitites[element] = (normalize(velocitites[element])*2.0);
     }
   }
   else
     acceleration[element] = initializer;

  if(pclamp.s0 != positions[element].s0 || pclamp.s1 != positions[element].s1 || pclamp.s2 != positions[element].s2)
  {
    float khook = 500;
    float kdamp = 20.0;

    float4 diff, vdiff;
    float len, forcemagnitude, dampingmagnitude;
    diff = positions[element] - pclamp;
    vdiff = velocitites[element];
    dampingmagnitude= dot(vdiff, diff);
    len = length(diff);
    if( len == 0.0 ) return;
    diff = normalize(diff);
    forcemagnitude = -khook * (len);
    forcemagnitude += -kdamp*dampingmagnitude/len;
    diff = diff * forcemagnitude;
    acceleration[element] = diff;
  }
}
