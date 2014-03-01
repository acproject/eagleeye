#ifndef _COMMONCU_CUH_
#define _COMMONCU_CUH_
#include "EagleeyeMacro.h"

namespace eagleeye
{
#define THREAD_NUM_IN_BLOCK 16
#define MAX_THREAD_NUM_IN_BLOCK 1024

#define  USE_TEXTURE 0

#if(USE_TEXTURE)
	texture<float, 1, cudaReadModeElementType> tex_float;
#define   LOAD_FLOAT(i) tex1Dfetch(tex_float, i)
#define  SET_FLOAT_BASE cutilSafeCall( cudaBindTexture(0, tex_float, d_src) )
#else
#define  LOAD_FLOAT(i) d_src[i]
#define SET_FLOAT_BASE
#endif

#ifdef __CUDACC__
	typedef float2 fComplex;
#else
	typedef struct{
		float x;
		float y;
	} fComplex;
#endif


/**
 *	@fn bool checkCUDAProfile(int dev, int min_runtime, int min_compute)
 *	@brief check cuda profile
 *	@param dev device index
 *	@param min_runtime_ver the minimum runtime version
 *	@param min_compute the minimum compute capability
 */
bool checkCUDAProfile(int dev, int min_runtime_ver, int min_compute);

/**
 *	@fn int findCapableDevice(int argc,char **argv)
 *	@brief find all capable device
 */
int findCapableDevice(int argc,char **argv);

/**
 *	@fn bool iniCuda(int argc,char ** argv)
 *	@brief initialize cuda device
 */
bool iniCuda(int argc,char ** argv);


/**
*	@fn int iDivUp(int a, int b);
*	@brief Some help function for splitting computing mesh 
*	@param a
*	@param b
*/
inline int iDivUp(int a, int b)
{
	return (a % b != 0) ? (a / b + 1) : (a / b);
}

/**
*	@fn int iAlignUp(int a, int b);
*	@brief Some help function for splitting computing mesh
*	@param a
*	@param b
*/
inline int iAlignUp(int a, int b)
{
	return (a % b != 0) ?  (a - a % b + b) : a;
}

/**
*	@fn int snapTransformSize(int data_size);
*	@brief Some help function for using CUFFT
*	@param data_size
*/
inline int snapTransformSize(int data_size)
{
	int hi_bit;
	unsigned int low_pot, hi_pot;

	data_size = iAlignUp(data_size, 16);

	for(hi_bit = 31; hi_bit >= 0; hi_bit--)
		if(data_size & (1U << hi_bit)) break;

	low_pot = 1U << hi_bit;
	if(low_pot == (unsigned int)data_size)
		return data_size;

	hi_pot = 1U << (hi_bit + 1);
	if(hi_pot <= 1024)
		return hi_pot;
	else 
		return iAlignUp(data_size, 512);
}

/**
*	@fn void conjugate(fComplex* d_data,unsigned int count);
*	@brief Compute the conjugate of input complex data
*	@param (in)d_data(device) the complex data
*	@param (in)count the number of the complex data
*	@note the result would be written into input data\n
*	This function would be run in GPU
*/
void conjugate(fComplex* d_data,unsigned int count);

/**
*	@fn void subtractScalar(float* d_data,unsigned int d_h,unsigned int d_w,floatvalue );
*	@brief make d_data subtract the value
*	@param (in)d_data(device)
*	@param (in)d_h
*	@param (in)d_w
*	@param (in)value
*	@note This function would be run in GPU
*/
void subtractScalar(float* d_data,unsigned int d_h,unsigned int d_w,float value);

/**
*	@fn void multiply(fComplex* d_multiply_term1,fComplex* d_multiply_term2,\n
*		fComplex* d_result,unsigned int count);
*	@brief Make every element in the d_multiply_term1 multiply the corresponding element\n
*	in the d_multiply_term2, and write the result into d_result
*	@param (in)d_multiply_term1(device)
*	@param (in)d_multiply_term2(device)
*	@param (in)d_result(device)
*	@param count the number of data in d_multiply_term1
*	@note This function would br run in GPU
*/
void multiply(fComplex* d_multiply_term1,fComplex* d_multiply_term2,
			  fComplex* d_result,unsigned int count);

void multiplyAndScale(fComplex* a,fComplex* b,float scale,fComplex* c,unsigned int count);

/**
*	@fn void padData(float *d_dst,float *d_src,unsigned int fft_h,unsigned int fft_w,\n
*		unsigned int data_h,unsigned int data_w);
*	@brief Fill a large data block by using d_src
*	@param (in&out)d_dst(device) A large data block. It would be filled after calling this function
*	@param (in)dst_h the height of d_dst
*	@param (in)dst_w the width of d_dst	
*	@param (in)d_src(device) The data src. We would use this data to fill "d_dst"
*	@param (in)src_h the height of d_src
*	@param (in)src_w the width of d_src 
*	@note This function would be run in GPU
*/
void padData(float *d_dst,unsigned int dst_h,unsigned int dst_w,
			 float *d_src,unsigned int src_h,unsigned int src_w,int offset_h = 0,int offset_w = 0);

}

#endif