__kernel void acceleration_kernel(__global float4 *acceleration, float4 initializer)
{
   int element = get_global_id(0);
   acceleration[element] = initializer;
}