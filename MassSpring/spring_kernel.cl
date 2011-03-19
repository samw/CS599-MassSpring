void __kernel spring_kernel_single(__global float4 *position,  __global float4 *velocity, __global float4 *acceleration,
                                   int a, int b, float restLength, float khook, float kdamp)
{
  float4 diff, vdiff;
  float len, forcemagnitude, dampingmagnitude;
  //Find position difference pDIFFERENCE(a, b, diff);
  diff = position[a] - position[b];
  //Find Velocity difference pDIFFERENCE(av, bv, vdiff);
  vdiff = velocity[a] - velocity[b];
  //Find Velocity difference dot position difference: dampingmagnitude = vdiff.x*diff.x + vdiff.y*diff.y + vdiff.z*diff.z;
  dampingmagnitude= dot(vdiff, diff);
  //Position difference will be now normalized, with length storing it's length pNORMALIZEI(diff);
  len = length(diff);
  if( len == 0.0 ) return;
  diff = normalize(diff);
  //Spring force magnitude is khook * (position difference magnidute - restlength)
  forcemagnitude = -khook * (len - restLength);
  //Add damping to force
  //Damping force magnitude is kdamp * velocity difference DOT position difference / difference magnitude
  forcemagnitude += -kdamp*dampingmagnitude/len;
  //Create final force after spring and damping force added pMULTIPLY(diff, forcemagnitude, diff);
  diff = diff * forcemagnitude;
  acceleration[a] += diff;
  acceleration[b] -= diff;
}
//Note accumulation is not thread safe
//Kernels operating on the same points must execute time exclusive of eachother

void __kernel spring_kernel_batch(__global float4 *position,  __global float4 *velocity, __global float4 *acceleration,
                                  __global int2 *springs, __global float4 *spring_properties)
{
  float4 diff, vdiff, properties;
  float len, forcemagnitude, dampingmagnitude;
  int a, b;
  int spring = get_global_id(0);
  a = springs[spring].s0;
  b = springs[spring].s1;
  properties = spring_properties[spring];

  diff = position[a] - position[b];
  vdiff = velocity[a] - velocity[b];
  dampingmagnitude= dot(vdiff, diff);
  len = length(diff);
  if( len == 0.0 ) return;
  diff = normalize(diff);
  forcemagnitude = -(properties.s1) * (len - (properties.s0));
  forcemagnitude += -(properties.s2)*dampingmagnitude/len;
  diff = diff * forcemagnitude;
  acceleration[a] += diff;
  acceleration[b] -= diff;
}
//Note accumulation is not thread safe
//One solution is to keep separate batch of spring information sets that only contain each point once
//And compute sets in separate time exclusive executions of this kernel

void __kernel spring_kernel(__global float4 *position,  __global float4 *velocity,
                                  __global int2 *springs, __global float4 *spring_properties,
                                  __global float4 *spring_force)
{
  float4 diff, vdiff, properties;
  float len, forcemagnitude, dampingmagnitude;
  int a, b;
  int spring = get_global_id(0);
  a = springs[spring].s0;
  b = springs[spring].s1;

  diff = position[a] - position[b];
  vdiff = velocity[a] - velocity[b];
  dampingmagnitude= dot(vdiff, diff);
  len = length(diff);
  if( len == 0.0 ) return;
  diff = normalize(diff);
  forcemagnitude = -(spring_properties[spring].s1) * (len - (spring_properties[spring].s0));
  forcemagnitude += -(spring_properties[spring].s2)*dampingmagnitude/len;
  spring_force[spring] = diff * forcemagnitude;
}

// Blocksize should = SpringList length / work-items / 2;
void __kernel accumulate_a1(__global float4 *acceleration, __global float4 *spring_force,
                            __global int *vertex, __global int *spring, __global float* weight,
                            int blocksize)
{
  int block = get_global_id(0) * 2;
  int springindex = block * blocksize;
  int i;
  for(int i = 0; i < blocksize; i++)
    acceleration[vertex[springindex+i]] = spring_force[spring[springindex]] * weight[springindex];
}

void __kernel accumulate_a2(__global float4 *acceleration, __global float4 *spring_force,
                            __global int *vertex, __global int *spring, __global float* weight,
                            int blocksize)
{
  int block = (get_global_id(0) * 2) + 1;
  int springindex = block * blocksize;
  int i;
  for(int i = 0; i < blocksize; i++)
    acceleration[vertex[springindex+i]] = spring_force[spring[springindex]] * weight[springindex];
}