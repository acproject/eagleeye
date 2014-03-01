#ifndef _MATRIXMATH_CUH_
#define _MATRIXMATH_CUH_
#include "EagleeyeMacro.h"

namespace eagleeye
{
/**
*	@fn void convolution2DCu(const float* src,\n
*	const unsigned int src_rows,\n
*	const unsigned int src_cols,\n
*	const float* kernel,\n
*	const unsigned int kernel_rows,\n
*	const unsigned int kernel_cols);
*	@brief 2D convolution using GPU
*	@param src the image data
*	@param src_rows the row number of image
*	@param src_cols the col number of image
*	@param kernel the kernel data
*	@param kernel_rows the row number of kernel
*	@param kernel_cols the col number of kernel
*	@param result the convolution result
*/
extern "C"
	void convolution2DCu(const float* src,
	const unsigned int src_rows,
	const unsigned int src_cols,
	const float* kernel,
	const unsigned int kernel_rows,
	const unsigned int kernel_cols,
	float* result);

extern "C"
	void convolution2DBankCu(const float* src,
	const unsigned int src_rows,
	const unsigned int src_cols,
	float** kernels,
	const unsigned int kernels_num,
	const unsigned int kernel_rows,
	const unsigned int kernel_cols,
	float** result);

/**
*/
void matchTemplateCu(const float* match_target,
	const unsigned int target_rows,
	const unsigned int target_cols,
	float* match_template,
	unsigned int template_rows,
	unsigned int template_cols,
	float* similarity);

/**
*	@fn extern "C"\n
*		void matchTemplateSQDIFFCu(float* match_target,\n
*		unsigned int target_rows,\n
*		unsigned int target_cols,\n
*		float* match_template,\n
*		unsigned int template_rows,\n
*		unsigned int template_cols,\n
*		float* similarity);
*	@brief Implement template match. This function would return\n
*	a similarity matrix, which displays the similarity extent between \n
*	the local region in the target image and the template.
*	@param (in)match_target the target image
*	@param (in)target_rows the row number of the target image
*	@param (in)target_cols the col number of the target image
*	@param (in)match_template the the template image
*	@param (in)template_rows the row number of the template image
*	@param (in)template_cols the col number of the template image
*	@param (out)similarity the similarity matrix
*	@note The similarity measure way is "Square Difference"
*/
extern "C"
	void matchTemplateSQDIFFCu(const float* match_target,
	unsigned int target_rows,
	unsigned int target_cols,
	const float* match_template,
	unsigned int template_rows,
	unsigned int template_cols,
	float* similarity);

/**
*	@fn void integralImage(float* imagedata,float* sum,unsigned int image_h,unsigned int image_w);
*	@brief Compute integral image.
*	@param (in)imagedata the image data(host)
*	@param (out)sum the integral image data(host)
*	@param (in)image_h the height of image
*	@param (in)image_w the width of image
*	@note This function would be run in the CPU
*/
void integralImage(float* imagedata,float* sum,unsigned int image_h,unsigned int image_w);
}

#endif