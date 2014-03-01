#include "commoncu.cuh"

#include <stdlib.h>
#include <stdio.h>
#include <cutil_inline.h>

namespace eagleeye
{
bool checkCUDAProfile(int dev, int min_runtime_ver, int min_compute)
{
	int runtime_ver = 0;     

	cudaDeviceProp device_prop;
	cudaGetDeviceProperties(&device_prop, dev);

	fprintf(stderr,"\nDevice %d: \"%s\"\n", dev, device_prop.name);
	cudaRuntimeGetVersion(&runtime_ver);

	if (min_runtime_ver>runtime_ver||min_compute>device_prop.major)
	{
		fprintf(stderr,"  CUDA Runtime Version     :\t%d.%d\n", runtime_ver/1000, (runtime_ver%100)/10);
		fprintf(stderr,"  CUDA Compute Capability  :\t%d.%d\n", device_prop.major, device_prop.minor);
		return false;
	}

	return true;
}

int findCapableDevice(int argc,char **argv)
{
	int device_count=0;
	cudaError_t error_id=cudaGetDeviceCount(&device_count);
	if (error_id!=cudaSuccess)
	{
		printf("cudaGetDeviceCount returned %d\n->%s\n",(int)error_id,cudaGetErrorString(error_id));
		return -1;
	}

	if (device_count==0)
	{
		fprintf(stderr,"There is no device supporting CUDA.\n");
		return -1;
	}
	else
	{
		fprintf(stderr,"Found %d CUDA Capable Device(s).\n",device_count);
	}

	int best_dev=-1;
	cudaDeviceProp best_device_prop;
	for (int dev=0;dev<device_count;++dev)
	{
		cudaDeviceProp device_prop;
		cudaGetDeviceProperties(&device_prop,dev);

		if ((best_dev==-1)||(best_device_prop.major<device_prop.major))
		{
			best_dev=dev;
			best_device_prop=device_prop;
		}
	}

	if (best_dev!=-1)
	{
		fprintf(stderr,"Setting active device to %d\n",best_dev);
	}

	return best_dev;
}

bool iniCuda(int argc,char ** argv)
{
	int dev=findCapableDevice(argc,argv);
	if (dev!=-1)
	{
		cudaSetDevice(dev);
		return true;
	}
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////
__global__ void conjugate_kernel(fComplex* d_data,unsigned int count)
{
	const unsigned int x=blockDim.x*blockIdx.x+threadIdx.x;

	if (x<count)
	{
		d_data[x].y=-d_data[x].y;
	}
}

void conjugate(fComplex* d_data,unsigned int count)
{
	dim3 threads(256,1);
	dim3 grid(iDivUp(count,threads.x),1);	

	conjugate_kernel<<<grid,threads>>>(d_data,count);
}

//////////////////////////////////////////////////////////////////////////

__global__ void subtractScalar_kernel(float* d_data,unsigned int d_h,unsigned int d_w,float value)
{
	const unsigned int y=blockDim.y*blockIdx.y+threadIdx.y;
	const unsigned int x=blockDim.x*blockIdx.x+threadIdx.x;

	if (y<d_h&&x<d_w)
	{
		d_data[y*d_h+x]-=value;
	}
}

void subtractScalar(float* d_data,unsigned int d_h,unsigned int d_w,float value)
{
	dim3 threads(32,8);
	dim3 grid(iDivUp(d_w,threads.x),iDivUp(d_h,threads.y));

	subtractScalar_kernel<<<grid,threads>>>(d_data,d_h,d_w,value);
}

//////////////////////////////////////////////////////////////////////////

__global__ void multiply_kernel(fComplex* a,fComplex* b,fComplex* c,unsigned int count)
{
	const unsigned int x=blockDim.x*blockIdx.x+threadIdx.x;

	if (x<count)
	{
		unsigned int index=x;
		float real_part,image_part;

		real_part=a[index].x*b[index].x-a[index].y*b[index].y;
		image_part=a[index].x*b[index].y+b[index].x*a[index].y;

		c[index].x=real_part;
		c[index].y=image_part;
	}
}

void multiply(fComplex* d_multiply_term1,fComplex* d_multiply_term2,fComplex* d_result,unsigned int count)
{
	dim3 threads(256,1);
	dim3 grid(iDivUp(count,threads.x),1);

	multiply_kernel<<<grid,threads>>>(d_multiply_term1,d_multiply_term2,d_result,count);
}

//////////////////////////////////////////////////////////////////////////
__global__ void multiplyAndScale_kernel(fComplex* a,fComplex* b,float scale,fComplex* c,unsigned int count)
{
	const unsigned int x=blockDim.x*blockIdx.x+threadIdx.x;

	if (x<count)
	{
		unsigned int index=x;
		float real_part,image_part;

		real_part=a[index].x*b[index].x-a[index].y*b[index].y;
		image_part=a[index].x*b[index].y+b[index].x*a[index].y;

		c[index].x=real_part*scale;
		c[index].y=image_part*scale;
	}
}

void multiplyAndScale(fComplex* a,fComplex* b,float scale,fComplex* c,unsigned int count)
{
	dim3 threads(256,1);
	dim3 grid(iDivUp(count,threads.x),1);

	multiplyAndScale_kernel<<<grid,threads>>>(a,b,scale,c,count);
}

//////////////////////////////////////////////////////////////////////////
__global__ void padData_kernel(float* d_dst,unsigned int dst_h,unsigned int dst_w,
	float* d_src,unsigned int src_h,unsigned int src_w,int offset_h,int offset_w)
{
	const unsigned int y=blockDim.y*blockIdx.y+threadIdx.y;
	const unsigned int x=blockDim.x*blockIdx.x+threadIdx.x;

	if (y<src_h&&x<src_w)
	{
		d_dst[(y+offset_h)*dst_w+x+offset_w]=LOAD_FLOAT(y*src_w+x);
	}
}

void padData(float *d_dst,unsigned int dst_h,unsigned int dst_w,float *d_src,unsigned int src_h,unsigned int src_w,int offset_h,int offset_w)
{
	dim3 threads(32, 8);
	dim3 grid(iDivUp(src_w, threads.x), iDivUp(src_h, threads.y));

	SET_FLOAT_BASE;

	padData_kernel<<<grid,threads>>>(d_dst,dst_h,dst_w,d_src,src_h,src_w,offset_h,offset_w);
}
}
