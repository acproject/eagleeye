#ifndef _MATRIXMATHHELP_CUH_
#define _MATRIXMATHHELP_CUH_
#include "EagleeyeMacro.h"
#include "Matrix.h"

namespace eagleeye
{
extern "C"
	void matchTemplateSQDIFFCu(const float* match_target,
	unsigned int target_rows,
	unsigned int target_cols,
	const float* match_template,
	unsigned int template_rows,
	unsigned int template_cols,
	float* similarity);
extern "C"
	void convolution2DCu(const float* src,
	const unsigned int src_rows,
	const unsigned int src_cols,
	const float* kernel,
	const unsigned int kernel_rows,
	const unsigned int kernel_cols,
	float* result);


/**
 *	@fn Matrix<float> matchTemplateSQDIFFCu(Matrix<float> match_target,Matrix<float> match_template)
 *	@brief Implementing template match function(This is a CUDA version)
 *	@param match_target the target image
 *	@param match_template the template image
 *	@return the similarity matrix
 */
Matrix<float> matchTemplateSQDIFFCu(const Matrix<float> match_target,const Matrix<float> match_template)
{
// 	//set the processing device
// 	cudaSetDevice(cutGetMaxGflopsDeviceId());

	unsigned int target_rows=match_target.rows();
	unsigned int target_cols=match_target.cols();

	unsigned int template_rows=match_template.rows();
	unsigned int template_cols=match_template.cols();

	Matrix<float> similarity(target_rows,target_cols);
	const float* target_data=match_target.dataptr();
	const float* template_data=match_template.dataptr();

	matchTemplateSQDIFFCu(target_data,target_rows,target_cols,
		template_data,template_rows,template_cols,similarity.dataptr());
	
// 	//reset the processing device
// 	cutilDeviceReset();
	return similarity;
}

Matrix<float> convolution2DCu(const Matrix<float> src,const Matrix<float> kernel)
{
// 	//set the processing device
// 	cudaSetDevice(cutGetMaxGflopsDeviceId());
	unsigned int src_rows=src.rows();
	unsigned int src_cols=src.cols();

	unsigned int kernel_rows=kernel.rows();
	unsigned int kernel_cols=kernel.cols();

	Matrix<float> result(src_rows,src_cols);
	const float* src_data=src.dataptr();
	const float* kernel_data=kernel.dataptr();

	convolution2DCu(src_data,src_rows,src_cols,
		kernel_data,kernel_rows,kernel_cols,result.dataptr());

// 	//reset the processing device
// 	cutilDeviceReset();
	return result;
}
}

#endif