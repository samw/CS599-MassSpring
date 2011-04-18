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
  if(forcemagnitude > 2) forcemagnitude = 2;
  diff = diff * forcemagnitude;
  *pull_acceleration = diff;
}

__kernel void init_acceleration_kernel(__global float4 *positions, __global float4 *acceleration, __global float4 *pull_position, __global float4 *pull_value, float4 initializer)
{
   int element = get_global_id(0);
   float maxdistance = 0.1;
   float dist = distance( *pull_position, positions[element] );
   if( dist < maxdistance)
     acceleration[element] = *pull_value * dot(normalize(*pull_value) , (normalize(cross( (positions[element] - *pull_position), cross(*pull_value, (positions[element] - *pull_position))))));
   else
     acceleration[element] = initializer;
}
