#include "MatrixMath.cuh"
#include "cuda/commoncu.cuh"
#include <cufft.h>
#include <cutil_inline.h>

namespace eagleeye
{
void convolution2DCu(const float* src,
	const unsigned int src_rows,
	const unsigned int src_cols,
	const float* kernel,
	const unsigned int kernel_rows,
	const unsigned int kernel_cols,float* result)
{
	const int kernel_h=kernel_rows;
	const int kernel_w=kernel_cols;
	const int data_h=src_rows;
	const int data_w=src_cols;
	const int fft_h=snapTransformSize(data_h+kernel_h-1);
	const int fft_w=snapTransformSize(data_w+kernel_w-1);

	float* h_result;
	h_result=(float*)malloc(sizeof(float)*fft_h*fft_w);

	float* d_src;
	float* d_kernel;

	float* d_padded_src;
	float* d_padded_kernel;

	fComplex* d_src_spectrum;
	fComplex* d_kernel_spectrum;

	cutilSafeCall(cudaMalloc((void**)&d_src,data_h*data_w*sizeof(float)));
	cutilSafeCall(cudaMalloc((void**)&d_kernel,kernel_h*kernel_w*sizeof(float)));

	cutilSafeCall(cudaMalloc((void**)&d_padded_src,fft_h*fft_w*sizeof(float)));
	cutilSafeCall(cudaMalloc((void**)&d_padded_kernel,fft_h*fft_w*sizeof(float)));

	cutilSafeCall(cudaMalloc((void**)&d_src_spectrum,fft_h * (fft_w / 2 + 1)*sizeof(fComplex)));
	cutilSafeCall(cudaMalloc((void**)&d_kernel_spectrum,fft_h * (fft_w / 2 + 1)*sizeof(fComplex)));

	cutilSafeCall(cudaMemcpy(d_kernel,kernel,kernel_h*kernel_w*sizeof(float),cudaMemcpyHostToDevice));
	cutilSafeCall(cudaMemcpy(d_src,src,data_h*data_w*sizeof(float),cudaMemcpyHostToDevice));

	cutilSafeCall(cudaMemset(d_padded_kernel,0,fft_w*fft_h*sizeof(float)));
	cutilSafeCall(cudaMemset(d_padded_src,0,fft_w*fft_h*sizeof(float)));

	padData(d_padded_src,fft_h,fft_w,d_src,data_h,data_w,kernel_h/2,kernel_w/2);
	padData(d_padded_kernel,fft_h,fft_w,d_kernel,kernel_h,kernel_w);

	cufftHandle fftplan_fwd,fftplan_inv;
	cufftSafeCall(cufftPlan2d(&fftplan_fwd,fft_h,fft_w,CUFFT_R2C));
	cufftSafeCall(cufftPlan2d(&fftplan_inv,fft_h,fft_w,CUFFT_C2R));

	//running GPU FFT convolution
	cufftSafeCall(cufftExecR2C(fftplan_fwd,(cufftReal*)d_padded_src,(cufftComplex*)d_src_spectrum));
	cufftSafeCall(cufftExecR2C(fftplan_fwd,(cufftReal*)d_padded_kernel,(cufftComplex*)d_kernel_spectrum));

	cutilSafeCall(cutilDeviceSynchronize());
	conjugate(d_kernel_spectrum,fft_h * (fft_w / 2 + 1));
	multiplyAndScale(d_src_spectrum,d_kernel_spectrum,1.0f/(fft_h*fft_w),d_src_spectrum,fft_h * (fft_w / 2 + 1));


	cufftSafeCall(cufftExecC2R(fftplan_inv,(cufftComplex*)d_src_spectrum,(cufftReal*)d_padded_src));
	cutilSafeCall(cutilDeviceSynchronize());

	cutilSafeCall(cudaMemcpy(h_result,d_padded_src,fft_h*fft_w*sizeof(float),cudaMemcpyDeviceToHost));

	for (unsigned int i=0;i<src_rows;++i)
	{
		for (unsigned int j=0;j<src_cols;++j)
		{
			result[i*src_cols+j]=h_result[i*fft_w+j];
		}
	}

	cufftSafeCall(cufftDestroy(fftplan_fwd));
	cufftSafeCall(cufftDestroy(fftplan_inv));

	cutilSafeCall(cudaFree(d_src));
	cutilSafeCall(cudaFree(d_kernel));
	cutilSafeCall(cudaFree(d_padded_src));
	cutilSafeCall(cudaFree(d_padded_kernel));
	cutilSafeCall(cudaFree(d_src_spectrum));
	cutilSafeCall(cudaFree(d_kernel_spectrum));
	free(h_result);
}

void convolution2DBankCu(const float* src,
	const unsigned int src_rows,
	const unsigned int src_cols,
	float** kernels,
	const unsigned int kernels_num,
	const unsigned int kernel_rows,
	const unsigned int kernel_cols,
	float** result)
{
	const int kernel_h = kernel_rows;
	const int kernel_w = kernel_cols;
	const int data_h = src_rows;
	const int data_w = src_cols;
	const int fft_h = snapTransformSize(data_h + kernel_h - 1);
	const int fft_w = snapTransformSize(data_w + kernel_w - 1);

	float* h_result;
	h_result = (float*)malloc(sizeof(float) * fft_h * fft_w);

	float* d_src;
	float* d_kernel;

	float* d_padded_src;
	float* d_padded_kernel;
	float* d_padded_temp;

	fComplex* d_src_spectrum;
	fComplex* d_kernel_spectrum;
	fComplex* d_temp_spectrum;

	//allocate some space
	cutilSafeCall(cudaMalloc((void**)&d_src,data_h * data_w * sizeof(float)));
	cutilSafeCall(cudaMalloc((void**)&d_kernel,kernel_h * kernel_w * sizeof(float)));

	cutilSafeCall(cudaMalloc((void**)&d_padded_src,fft_h * fft_w * sizeof(float)));
	cutilSafeCall(cudaMalloc((void**)&d_padded_kernel,fft_h * fft_w * sizeof(float)));
	cutilSafeCall(cudaMalloc((void**)&d_padded_temp,fft_h * fft_w * sizeof(float)));

	cutilSafeCall(cudaMalloc((void**)&d_src_spectrum,fft_h * (fft_w / 2 + 1)*sizeof(fComplex)));
	cutilSafeCall(cudaMalloc((void**)&d_kernel_spectrum,fft_h * (fft_w / 2 + 1)*sizeof(fComplex)));
	cutilSafeCall(cudaMalloc((void**)&d_temp_spectrum,fft_h * (fft_w / 2 + 1)*sizeof(fComplex)));

	cutilSafeCall(cudaMemset(d_padded_kernel,0,fft_w * fft_h * sizeof(float)));
	cutilSafeCall(cudaMemset(d_padded_src,0,fft_w * fft_h * sizeof(float)));

	//copy image data from host to device
	cutilSafeCall(cudaMemcpy(d_src,src,data_h * data_w * sizeof(float),cudaMemcpyHostToDevice));
	//pad some additional data(how to pad is very important)
	padData(d_padded_src,fft_h,fft_w,d_src,data_h,data_w,kernel_h / 2,kernel_w / 2);

	cufftHandle fftplan_fwd,fftplan_inv;
	cufftSafeCall(cufftPlan2d(&fftplan_fwd,fft_h,fft_w,CUFFT_R2C));
	cufftSafeCall(cufftPlan2d(&fftplan_inv,fft_h,fft_w,CUFFT_C2R));

	//compute spectrum of image
	cufftSafeCall(cufftExecR2C(fftplan_fwd,(cufftReal*)d_padded_src,(cufftComplex*)d_src_spectrum));

	for (int kernel_index = 0; kernel_index < int(kernels_num); ++kernel_index)
	{
		//copy kernel data from host to device
		cutilSafeCall(cudaMemcpy(d_kernel,kernels[kernel_index],kernel_h*kernel_w*sizeof(float),cudaMemcpyHostToDevice));
		//pad some additional data
		padData(d_padded_kernel,fft_h,fft_w,d_kernel,kernel_h,kernel_w);

		//compute spectrum of kernel
		cufftSafeCall(cufftExecR2C(fftplan_fwd,(cufftReal*)d_padded_kernel,(cufftComplex*)d_kernel_spectrum));

		//compute convolution
		cutilSafeCall(cutilDeviceSynchronize());
		conjugate(d_kernel_spectrum,fft_h * (fft_w / 2 + 1));
		multiplyAndScale(d_src_spectrum,d_kernel_spectrum,1.0f / (fft_h * fft_w),d_temp_spectrum,fft_h * (fft_w / 2 + 1));


		cufftSafeCall(cufftExecC2R(fftplan_inv,(cufftComplex*)d_temp_spectrum,(cufftReal*)d_padded_temp));
		cutilSafeCall(cutilDeviceSynchronize());

		cutilSafeCall(cudaMemcpy(h_result,d_padded_temp,fft_h * fft_w * sizeof(float),cudaMemcpyDeviceToHost));

		for (unsigned int i = 0; i < src_rows; ++i)
		{
			for (unsigned int j = 0; j < src_cols; ++j)
			{
				result[kernel_index][i * src_cols + j] = h_result[i * fft_w + j];
			}
		}
	}

	cufftSafeCall(cufftDestroy(fftplan_fwd));
	cufftSafeCall(cufftDestroy(fftplan_inv));

	cutilSafeCall(cudaFree(d_src));
	cutilSafeCall(cudaFree(d_kernel));
	cutilSafeCall(cudaFree(d_padded_src));
	cutilSafeCall(cudaFree(d_padded_kernel));
	cutilSafeCall(cudaFree(d_padded_temp));
	cutilSafeCall(cudaFree(d_src_spectrum));
	cutilSafeCall(cudaFree(d_kernel_spectrum));
	cutilSafeCall(cudaFree(d_temp_spectrum));
	free(h_result);
}

void matchTemplateCu(const float* match_target,
	const unsigned int target_rows,
	const unsigned int target_cols,
	float* match_template,
	unsigned int template_rows,
	unsigned int template_cols,
	float* similarity)
{

}

//for the cuda 4.2
void matchTemplateSQDIFFCu(const float* match_target,
	unsigned int target_rows,
	unsigned int target_cols,
	const float* match_template,
	unsigned int template_rows,
	unsigned int template_cols,
	float* similarity)
{
	// 		//some preprocess for match_template image
	// 		const int kernel_h=template_rows;
	// 		const int kernel_w=template_cols;
	// 		const int data_h=target_rows;
	// 		const int data_w=target_cols;
	// 		const int fft_h=snapTransformSize(data_h+kernel_h-1);
	// 		const int fft_w=snapTransformSize(data_w+kernel_w-1);
	// 
	// 		float* h_similarity;
	// 		h_similarity=(float*)malloc(sizeof(float)*fft_h*fft_w);
	// 
	// 		float* d_match_target;
	// 		float* d_match_template;
	// 
	// 		float* d_padded_target;
	// 		float* d_padded_template;
	// 		
	// 		fComplex* d_target_spectrum;
	// 		fComplex* d_template_spectrum;
	// 
	// 		cutilSafeCall(cudaMalloc((void**)&d_match_target,data_h*data_w*sizeof(float)));
	// 		cutilSafeCall(cudaMalloc((void**)&d_match_template,kernel_h*kernel_w*sizeof(float)));
	// 
	// 		cutilSafeCall(cudaMalloc((void**)&d_padded_target,fft_h*fft_w*sizeof(float)));
	// 		cutilSafeCall(cudaMalloc((void**)&d_padded_template,fft_h*fft_w*sizeof(float)));
	// 		
	// 		cutilSafeCall(cudaMalloc((void**)&d_target_spectrum,fft_h * (fft_w / 2 + 1)*sizeof(fComplex)));
	// 		cutilSafeCall(cudaMalloc((void**)&d_template_spectrum,fft_h * (fft_w / 2 + 1)*sizeof(fComplex)));
	// 
	// 		cutilSafeCall(cudaMemcpy(d_match_template,match_template,kernel_h*kernel_w*sizeof(float),cudaMemcpyHostToDevice));
	// 		cutilSafeCall(cudaMemcpy(d_match_target,match_target,data_h*data_w*sizeof(float),cudaMemcpyHostToDevice));
	// 
	// 		cutilSafeCall(cudaMemset(d_padded_template,0,fft_w*fft_h*sizeof(float)));
	// 		cutilSafeCall(cudaMemset(d_padded_target,0,fft_w*fft_h*sizeof(float)));
	// 
	// 		padData(d_padded_target,fft_h,fft_w,d_match_target,data_h,data_w);
	// 		padData(d_padded_template,fft_h,fft_w,d_match_template,kernel_h,kernel_w);
	// 
	// 		cufftHandle fftplan_fwd,fftplan_inv;
	// 		cufftSafeCall(cufftPlan2d(&fftplan_fwd,fft_h,fft_w,CUFFT_R2C));
	// 		cufftSafeCall(cufftPlan2d(&fftplan_inv,fft_h,fft_w,CUFFT_C2R));
	// 
	// 		//running GPU FFT convolution
	// 		cufftSafeCall(cufftExecR2C(fftplan_fwd,(cufftReal*)d_padded_target,(cufftComplex*)d_target_spectrum));
	// 		cufftSafeCall(cufftExecR2C(fftplan_fwd,(cufftReal*)d_padded_template,(cufftComplex*)d_template_spectrum));
	// 
	// 		cutilSafeCall(cutilDeviceSynchronize());
	//  		conjugate(d_template_spectrum,fft_h * (fft_w / 2 + 1));
	//  		multiplyAndScale(d_target_spectrum,d_template_spectrum,1.0/(fft_h*fft_w),d_target_spectrum,fft_h * (fft_w / 2 + 1));
	// 
	// 
	// 		cufftSafeCall(cufftExecC2R(fftplan_inv,(cufftComplex*)d_target_spectrum,(cufftReal*)d_padded_target));
	// 		cutilSafeCall(cutilDeviceSynchronize());
	// 		
	// 		cutilSafeCall(cudaMemcpy(h_similarity,d_padded_target,fft_h*fft_w*sizeof(float),cudaMemcpyDeviceToHost));
	// 	
	// 		for (unsigned int i=0;i<target_rows;++i)
	// 		{
	// 			for (unsigned int j=0;j<target_cols;++j)
	// 			{
	// 				similarity[i*target_cols+j]=h_similarity[i*fft_w+j];
	// 			}
	// 		}
	// 
	// 		cufftSafeCall(cufftDestroy(fftplan_fwd));
	// 		cufftSafeCall(cufftDestroy(fftplan_inv));
	// 
	// 		cutilSafeCall(cudaFree(d_match_target));
	// 		cutilSafeCall(cudaFree(d_match_template));
	// 		cutilSafeCall(cudaFree(d_padded_target));
	// 		cutilSafeCall(cudaFree(d_padded_template));
	// 		cutilSafeCall(cudaFree(d_target_spectrum));
	// 		cutilSafeCall(cudaFree(d_template_spectrum));
	// 		free(h_similarity);

	convolution2DCu(match_target,target_rows,target_cols,
		match_template,template_rows,template_cols,
		similarity);

	//compute term 2
	//compute integral image
	float* sq_match_target=(float*)malloc(sizeof(float)*target_rows*target_cols);
	float* sq_integral=(float*)malloc(sizeof(float)*target_rows*target_cols);

	for (unsigned int i=0;i<target_rows;++i)
	{
		const float* row_match_target_data=match_target+i*target_cols;
		float* row_sq_match_target_data=sq_match_target+i*target_cols;

		for (unsigned int j=0;j<target_cols;++j)
		{
			row_sq_match_target_data[j]=row_match_target_data[j]*row_match_target_data[j];
		}
	}

	integralImage(sq_match_target,sq_integral,target_rows,target_cols);

	float* term2=(float*)malloc(sizeof(float)*target_rows*target_cols);
	for (unsigned int i=0;i<target_rows;++i)
	{
		float* row_term2_data=term2+i*target_cols;

		for (unsigned int j=0;j<target_cols;++j)
		{
			unsigned int extend_r=EAGLEEYE_MIN((i+template_rows),(target_rows-1));
			unsigned int extend_c=EAGLEEYE_MIN((j+template_cols),(target_cols-1));

			row_term2_data[j]=sq_integral[extend_r*target_cols+extend_c]-
				sq_integral[i*target_cols+extend_c]-
				sq_integral[extend_r*target_cols+j]+
				sq_integral[i*target_cols+j];
		}
	}

	//compute term3
	float term3=0;
	for (unsigned int i=0;i<template_rows;++i)
	{
		const float* row_template_data=match_template+i*template_cols;

		for (unsigned int j=0;j<template_cols;++j)
		{
			term3+=row_template_data[j]*row_template_data[j];
		}
	}

	for (unsigned int i=0;i<target_rows;++i)
	{
		float* row_similarity_data=similarity+i*target_cols;

		float* row_term2_data=term2+i*target_cols;

		for (unsigned int j=0;j<target_cols;++j)
		{
			row_similarity_data[j]=row_term2_data[j]-2*row_similarity_data[j]+term3;
		}
	}

	free(sq_integral);
	free(sq_match_target);
	free(term2);
}

void integralImage(float* imagedata,float* sum,unsigned int image_h,unsigned int image_w)
{
	memset(sum,0,sizeof(float)*image_h*image_w);

	for (unsigned int i=0;i<image_h;++i)
	{
		float* row_data_ptr=imagedata+i*image_w;
		float* row_sum_data_ptr=sum+i*image_w;

		float cii=0;
		if (i==0)
		{
			for (unsigned int j=0;j<image_w;++j)
			{
				if (j==0)
				{
					cii=row_data_ptr[j];
				}
				else
				{
					cii=cii+row_data_ptr[j];
				}

				row_sum_data_ptr[j]=cii;
			}
		}
		else
		{
			float* row_sum_up_data_ptr=sum+(i-1)*image_w;
			for (unsigned int j=0;j<image_w;++j)
			{
				if (j==0)
				{
					cii=row_data_ptr[j];
				}
				else
				{
					cii=cii+row_data_ptr[j];
				}

				row_sum_data_ptr[j]=row_sum_up_data_ptr[j]+cii;
			}
		}
	}
}
}
