__kernel void collision_kernel(__global float4 *position,  __global float4 *velocity, __global float4 *acceleration,
						__global int2 *col_info, __global int4 *tets, int offsetT, int offsetV)
{
	int spot = get_global_id(0);
	int p,t;
	p = col_info[spot].s1;
	t = col_info[spot].s0;
	float4 t1, t2, t3, t4;
	int temp;
	float4 pos;
	temp = tets[t].s0 + offsetT;
	t1 = position[temp];
	temp = tets[t].s1 + offsetT;
	t2 = position[temp];
	temp = tets[t].s2 + offsetT;
	t3 = position[temp];
	temp = tets[t].s3 + offsetT;
	t4 = position[temp];
	pos = position[p];
	float x1,x2,y1,y2,z1,z2;
	x1 = min(t1.s0,min(t2.s0,min(t3.s0,t4.s0)));
	y1 = min(t1.s1,min(t2.s1,min(t3.s1,t4.s1)));
	z1 = min(t1.s2,min(t2.s2,min(t3.s2,t4.s2)));
	x2 = max(t1.s0,max(t2.s0,max(t3.s0,t4.s0)));
	y2 = max(t1.s1,max(t2.s1,max(t3.s1,t4.s1)));
	z2 = max(t1.s2,max(t2.s2,max(t3.s2,t4.s2)));
	if( pos.s0 < x2 && pos.s0 > x1 && pos.s1 > y1 && pos.s1 < y2 && pos.s2  > z1 && pos.s2 < z2){
		//got rid of buggy response code. it was driving me crazy.	
	}
	
}