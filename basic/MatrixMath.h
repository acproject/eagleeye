#ifndef _MATRIXMATH_H_
#define _MATRIXMATH_H_

#include "EagleeyeMacro.h"

#include "Types.h"
#include "Matrix.h"
#include <math.h>
#include <vector>
#include <algorithm>
#include <limits>
#include <stdarg.h>
#include "Variable.h"
#include "Array.h"
#include "permutohedral.h"
#include "Variable.h"
#include <assert.h>
#include "MatrixAuxiliary.h"
#include "TraitCenter.h"

namespace eagleeye
{
/**
*	@fn void fft1D(const Complex<double>* const t_data,\n
*	const Complex<double>* f_data,unsigned int r);
*	@brief Complete 1D fast fft
*	@param t_data the complex data in space field
*	@param f_data the complex data in frequency field
*	@param r the power of 2
*	@note The algorithm comes from "Visual C++ 数字图像处理"\n
*	It's very slow.
*/
void fft1D(const Complex<double>* const t_data,Complex<double>* const f_data,unsigned int r);

/**
*	@fn void ifft1D(const Complex<double>* const f_data,\n
*	const Complex<double>* t_data,unsigned int r);
*	@brief Complete 1D ifft
*	@param f_data the complex data in frequency field
*	@param t_data the complex data in space field
*	@param r the power of 2
*	@note The algorithm comes from "Visual C++ 数字图像处理"\n
*	It's very slow.
*/
void ifft1D(const Complex<double>* const f_data,Complex<double>* const t_data,unsigned int r);

/**
*	@fn template <typename T> \n
*	Matrix<Complex> fft(Matrix<T> m,unsigned int row,unsigned int col);
*	@brief Complete fast fft for the matrix m 
*	@param m image matrix
*	@param row_r the power of 2 (It defines an important parameter of FFT)
*	@param col_r the power of 2 (It defines an important parameter of FFT)
*	@note The algorithm comes from "Visual C++ 数字图像处理"\n
*	It would truncate m, or pad m with zeros to create an \n
*	 (2^row_r)-by-(2^col_r) array before doing the transform\n
*	 It's very slow.
*/
template <typename T>
Matrix<Complex<double>> fft2D(Matrix<T> m,unsigned int row_r,unsigned int col_r);

/**
*	@fn template<typename T>\n
*	Matrix<Complex> ifft(Matrix<T> m,unsigned int row,unsigned int col);
*	@brief Complete ifft for the matrix m
*	@param m frequency matrix
*	@param row_r the power of 2 (It defines an important parameter of FFT)
*	@param col_r the power of 2 (It defines an important parameter of FFT)
*	@note The algorithm comes from "Visual C++ 数字图像处理"
*	It would truncate m, or pad m with zeros to create an \n
*	 (2^row_r)-by-(2^col_r) array before doing the transform\n
*	 It's very slow.
*/
Matrix<Complex<double>> ifft2D(Matrix<Complex<double>> m,unsigned int row_r,unsigned int col_r);

/**
 *	@fn template<typename T>\n
 *		Matrix<float> matchTemplate(const Matrix<T> match_target,\n
 *		const Matrix<T> match_template);
 *	@brief template match algorithm
 *	@param (in)match_target the target image
 *	@param (in)match_template the template image
 *	@return the similarity matrix
 *	@note reference "Fast Normalized Corss_Correlation".\n
 *	property:It is not invariant to changes in image amplitude such\n
 *	as those caused by changing lighting conditions.
 */
template<typename T>
Matrix<float> matchTemplate(const Matrix<T> match_target,const Matrix<T> match_template);

/**
 *	@fn template<typename T>\n
 *		Matrix<float> matchTemplateSQDIFF(const Matrix<T> match_target,const Matrix<T> match_template);
 *	@brief template match algorithm
 *	@param (in)match_target the target image
 *	@param (in)match_template the template image
 *	@return the similarity matrix
 *	@note the measure standard is square difference.
 */
template<typename T>
Matrix<float> matchTemplateSQDIFF(const Matrix<T> match_target,const Matrix<T> match_template);

/**
 *	@fn template<typename T>\n
 *	Matrix<float> matchTemplateSQDIFF(const Matrix<T> match_target,\n
 *	const Matrix<T> match_template,\n
 *	unsigned int row_interval,\n
 *	unsigned int col_interval);
 *	@brief template match algorithm at sampling points
 *	@param match_target the target image
 *	@param match_template the template image
 *	@param row_interval the sample inteval along the col direction
 *	@param col_interval the sample interval along the row direction
 *	@return the similarity matrix at sampling points
 *	@note the measure standard is square difference.
 */
template<typename T>
Matrix<float> matchTemplateSQDIFF(const Matrix<T> match_target,
	const Matrix<T> match_template,
	unsigned int row_interval,
	unsigned int col_interval);

/**
 *	@fn template<typename T>\n
 *		void integralImage(const Matrix<T> image,\n
 *		Matrix<float>& sum,\n
 *		Matrix<float>& sqsum=Matrix<float>());
 *	@brief compute integral image
 *	@param (in)image 
 *	@param (out)sum the sum integral image
 *	@param (out)sqsum the square sum integral image
 */
template<typename T>
void integralImage(const Matrix<T> image,
	Matrix<float>& sum,
	Matrix<float>& sqsum=Matrix<float>());

/**
 *	@fn template<typename T>\n
 *	Matrix<float> conv2DInSpace(const Matrix<T> match_target,\n
 *	const Matrix<T> kernel);
 *	@brief This convolution algorithm is completed in the space field.
 *	@param (in)match_target the target image
 *	@param (in)kernel the kernel matrix
 *	@return the convolution result
 *	@note This algorithm is completed in space field.
 */
template<typename T>
Matrix<float> conv2DInSpace(const Matrix<T>& img,const Matrix<T>& kernel);

/**
 *	@brief convolution by using DFT(from OpenCV)
 *	@note have the corresponding version of GPU
 */
EAGLEEYE_API void convolution2D(Matrix<float>& src,Matrix<float>& kernel,Matrix<float>& response);
EAGLEEYE_API void convolution2DBank(Matrix<float>& src,std::vector<Matrix<float>>& kernel_bank,std::vector<Matrix<float>>& response_bank);

/**
*	@fn template<typename T>\n
*	Matrix<T> gaussFilter(const Matrix<T> img,const unsigned int kernel_width,const unsigned int kernel_height);
*	@brief
*	@param (in)img
*	@param (in)kernel_width
*	@param (in)kernel_height
*	@return the filter result
*	@note the variance of this gauss filter is 1.
 */
template<typename T>
Matrix<T> gaussFilter(const Matrix<T>& img,const unsigned int kernel_width,const unsigned int kernel_height);

/**
 *	@brief get the sort index
 *	@note make sure data is a vector
 */
template<class CompareT>
std::vector<unsigned int> sort(const Matrix<typename CompareT::ElementType>& data);

/**
 *	@brief This struct would be used in the sort algorithm
 */
template<class T>
struct SortElement
{
	SortElement(){};
	SortElement(T v,unsigned int i):value(v),index(i){};
	T value;
	unsigned int index;
};

/**
 *	@brief This struct is used in the sort function
 *	@note we have to define operator < for T type
 */
template<typename T>
struct AscendingSortPredict
{
	typedef		T		ElementType;
	bool operator()(const SortElement<T>& a,const SortElement<T>& b)
	{
		return a.value < b.value;	
	}
};

/**
 *	@brief This struct is used in the sort function
 *	@note we have to define operator > for T type
 */
template<typename T>
struct DescendingSortPredict
{
	typedef  T			ElementType;
	bool operator()(const SortElement<T>& a,const SortElement<T>& b)
	{
		return a.value > b.value;
	}
};

template<typename T>
struct LinearInterpolation
{
	LinearInterpolation(Matrix<T> src):matrix_src(src)
	{
		rows = src.rows();
		cols = src.cols();
	}

	template<typename ScalarType>
	T operator()(const ScalarType row_pos,const ScalarType col_pos)
	{
		unsigned int left_pos = unsigned int(floor(col_pos));
		left_pos = EAGLEEYE_MAX(left_pos,0);
		left_pos = EAGLEEYE_MIN(left_pos,cols - 1);

		unsigned int right_pos = unsigned int(floor(col_pos + 0.5));
		right_pos = EAGLEEYE_MAX(right_pos,0);
		right_pos = EAGLEEYE_MIN(right_pos,cols - 1);

		unsigned int top_pos = unsigned int(floor(row_pos));
		top_pos = EAGLEEYE_MAX(top_pos,0);
		top_pos = EAGLEEYE_MIN(top_pos,rows - 1);

		unsigned int down_pos = unsigned int(floor(row_pos + 0.5));
		down_pos = EAGLEEYE_MAX(down_pos,0);
		down_pos = EAGLEEYE_MIN(down_pos,rows - 1);

		//Perhaps this kind of method has some poor efficiency
		T left_top_value = matrix_src(left_pos,top_pos);
		T left_down_value = matrix_src(left_pos,down_pos);
		T right_top_value = matrix_src(right_pos,top_pos);
		T right_down_value = matrix_src(right_pos,down_pos);

		float r_delta = row_pos - top_pos;
		float c_delta = col_pos - left_pos;

		return (1 - r_delta) * (1 - c_delta) * left_top_value + 
			(1 - r_delta) * c_delta * right_top_value + 
			r_delta * (1 - c_delta) * left_down_value + 
			r_delta * c_delta * right_down_value;
	}

	Matrix<T> matrix_src;
	unsigned int rows;
	unsigned int cols;
};

/**
 *	@brief This function would complete upsampling for the image matrix src
 *	@param src the image matrix
 *	@param r the power of 2
 *	@return return the upsampling result
 *	@note It would adopt linear interpolation algorithm
 */
template<typename InterpolationType,typename T>
Matrix<T> upSample(const Matrix<T> src,	unsigned int row_r=1,unsigned int col_r=1);

/**
 *	@brief downSample/upInterpolate image
 *	@param src the image matrix
 *	@param row_interval row interval
 *	@param col_interval col interval
 */
template<typename T>
Matrix<T> downSample(const Matrix<T> src, unsigned int row_interval = 1, unsigned int col_interval = 1);
template<typename T>
Matrix<T> upInterpolate(const Matrix<T> src, unsigned int row_interval = 1, unsigned int col_interval = 1);

/**
 *	@brief This function would compute the direction description in 
 *	the defined position on the image
 *	@param (in)image the image matrix
 *	@param (in)pos some position coordinate
 *	@param (in)region_width the width of the local statistical region
 *	@param (in)region_height the height of the local statistical region
 *	@return the corresponding feature vector
 *	@note This algorithm is like to SIFT.	Reference:Distinctive \n
 *	Image Features from Scale-Invariant Keypoints. 
 */
template<typename T>
std::vector<VecF> computeDirectionDescription(
	const Matrix<T> image,std::vector<PointPos> pos,
	unsigned int region_width,unsigned int region_height);

/**
 *	@fn template<typename T>\n
 *	std::vector<float> computeInfoQuantity(const Matrix<T> image,std::vector<PosType> pos,\n
 *	unsigned int region_width,unsigned int region_height)\n
 *	@brief compute the info quantity at some predefined position of image
 *	@param image image matrix
 *	@param pos some predefined position
 *	@param region_width the width of local window
 *	@param region_height the height of local window
 *	@return 
 */
template<typename T>
std::vector<float> computeInfoQuantity(const Matrix<T> image,std::vector<PointPos> pos,
	unsigned int region_width,unsigned int region_height);

/**
 *	@fn	template<typename T>\n
 *	Matrix<float> computeInfoQuantity(const Matrix<T> image,\n
 *	unsigned int row_interval,unsigned int col_interval, \n
 *	unsigned int region_width,unsigned int region_height);
 *	@brief compute the info quantity at mesh covered over the image
 *	@param image image matrix
 *	@param row_interval the sampling interval
 *	@param col_interval the sampling interval
 *	@param region_width the width of local window
 *	@param region_height the height of local window
 *	@return
 *	@note It would call computeInfoQuantity(const Matrix<T> image,std::vector<PosType> pos,\n
 *	unsigned int region_width,unsigned int region_height)
 */
template<typename T>
Matrix<float> computeInfoQuantity(const Matrix<T> image,
	unsigned int row_interval,unsigned int col_interval, 
	unsigned int region_width,unsigned int region_height);

/**
*	@fn template<typename T,unsigned int Bins>\n
*	float infoEntropy(T histogram[Bins]);
*	@brief compute the information entropy of histogram
*	@param histogram the histogram
*	@return information entropy
 */
template<typename T,unsigned int BINS_NUM>
float infoEntropy(T histogram[BINS_NUM]);

/**
 *	@fn	template<typename T>\n
 *	float computeSSIM(const Matrix<T> target_image,const Matrix<T> detect_image);
 *	@brief compute structural similarity
 *	@param target_image the target image
 *	@param detect_image the detect image
 *	@param max_value the max pixel value
 *	@param min_value the min pixel value
 *	@return the structural similarity
 *	@note Reference: Image Quality Assessment: From Error Visibility to \n
 *	Structural Similarity
 */
template<typename T>
float computeSSIM(const Matrix<T> target_image,const Matrix<T> detect_image,
	float max_value,float min_value);

/**
 *	@fn	template<typename T>\n
 *	Matrix<float> matchTemplateSSIM(const Matrix<T> match_target,\n
 *	const Matrix<T> match_template,\n
 *	unsigned int row_interval,\n
 *	unsigned int col_interval);
 *	@brief Using structural similarity measure to complement template match
 *	@param match_target the target image
 *	@param match_template the template image
 *	@param row_interval the sample interval for row
 *	@param col_interval the sample interval for col
 *	@note In this function body, it would call computeSSIM(....) to compute 
 *	the structural similarity between the two corresponding patches.
 */
template<typename T>
Matrix<float> matchTemplateSSIM(const Matrix<T> match_target,
	const Matrix<T> match_template,
	unsigned int row_interval,
	unsigned int col_interval);

/**
 *	@brief Get the gradient image
 *	@param (in)src_image the original image
 *	@param (out)gradient_image the gradient image generated by this function
 *	@note we should clearly know "Gradient"
 */
template<typename T>
Matrix<Gradient> computeGradient(const Matrix<T> src_image);

/**
 *	@brief get gradient magnitude image(central difference)
 */
template<typename T>
Matrix<float> computeGradientMag(const Matrix<T>& img);

/**
 *	@brief extract edges from image(from opencv)
 */
EAGLEEYE_API Matrix<unsigned char> canny(Matrix<unsigned char> img,
										 double threshold1,double threshold2,
										int aperture_size = 3,bool l2gradient = false);

/**
 *	@brief Compute the standard histogram of image
 *	@param (in)image the image
 *	@param (in)minvalue the defined min value
 *	@param (in)maxvalue the defined max value
 *	@param (out)histogram
 */
template<typename T,unsigned int BINS_NUM>
void computeHistogram(const Matrix<T>& image,float minvalue,
	float maxvalue,int histogram[BINS_NUM]);

/**
 *	@brief Compute the weight histogram of image
 *	@param (in)image the image
 *	@param (in)weight the weight matrix for the corresponding image
 *	@param (in)minvalue the defined min value
 *	@param (in)maxvalue the defined max value
 *	@param (out)histogram
 *	@note image and weight must possess the same size
 */
template<typename T,unsigned int BINS_NUM>
void computeHistogram(const Matrix<T>& image,const Matrix<float>& weight,
	float minvalue,float maxvalue,float histogram[BINS_NUM]);

/**
 *	@brief Compute the European Distance between src_m and target_m
 *	@param (in)src_m the source matrix
 *	@param (in)target_m the target matrix
 *	@return European Distance
 *	@note src_m and target_m must possess the same size
 */
template<typename T>
float computeEuropeanDistance(const Matrix<T>& src_m,const Matrix<T>& target_m);

/**
 *	@brief compute row and col mean
 */
template<typename T>
Matrix<float> rowmean(const Matrix<T> m);
template<typename T>
Matrix<float> colmean(const Matrix<T> m);

/**
 *	@brief compute mean of data
 */
template<typename T>
float mean(const Matrix<T> m);

/**
 *	@brief compute row and col variance
 */
template<typename T>
Matrix<float> rowvar(const Matrix<T>& data);
template<typename T>
Matrix<float> colvar(const Matrix<T>& data);

/**
 *	@brief compute variance of data
 */
template<typename T>
float variance(const Matrix<T> m);

/**
 *	@brief Rotate the image 180 degree
 *	@param (in)image
 *	@return A new matrix image
 */
template<typename T>
Matrix<T> rot180(const Matrix<T> image);

template<typename T>
std::vector<unsigned int> findRange(const Matrix<T> m,const T minvalue,const T maxvalue);

template<typename T>
std::vector<unsigned int> findLess(const Matrix<T> m,const T value);

template<typename T>
std::vector<unsigned int> findGreater(const Matrix<T> m,const T value);

template<typename T>
void setValue(const Matrix<T> m,const std::vector<unsigned int>& index,const T value);

template<typename T>
void getMaxMin(const Matrix<T>& m,T& maxvalue,T& minvalue);

template<typename T>
Matrix<T> maxMatrix(const Matrix<T>& left,const Matrix<T>& right);

template<typename T>
Matrix<T> minMatrix(const Matrix<T>& left,const Matrix<T>& right);

/**
 *	@brief sum/max/min the src matrix 
 */
template<typename T>
Matrix<T> summat(const Matrix<T>& srcmat,Order d=ROW);
template<typename T>
Matrix<T> maxmat(const Matrix<T>& srcmat,Order d=ROW,Matrix<int>& max_index=Matrix<int>());
template<typename T>
Matrix<T> minmat(const Matrix<T>& srcmat,Order d=ROW,Matrix<int>& min_index=Matrix<int>());

template<typename T>
T sum(const Matrix<T>& srcmat);


/**
 *	@brief Special Type
 */
enum SpecialType
{
	EAGLEEYE_AVERAGE,
	EAGLEEYE_GAUSSIAN,
	EAGLEEYE_DISK,
	EAGLEEYE_LAPLACIAN,
	EAGLEEYE_LOG,
	EAGLEEYE_MOTION,
	EAGLEEYE_PREWITT,
	EAGLEEYE_SOBEL,
	EAGLEEYE_UNSHARP
};
/**
 *	@brief generate special matrix
 *	@param ker kernel
 *	@param kernel_size the size of matrix
 *	@param var_num the number of other parameters
 *	@param ... other parameters(double)
 *	@detail
 *	fspecial(EAGLEEYE_AVERAGE, kernel_size,var_num = 1,double(mean_val)) \n
 *	fspecial(EAGLEEYE_GAUSSIAN, kernel_size,var_num = 1, double(variance)) \n
 *	fspecial(EAGLEEYE_DISK, kernel_size,var_num = 1, ) \n
 *	fspecial(EAGLEEYE_LAPLACIAN, kernel_size, var_num = 1, double(alpha)) \n
 *	-- kernel_size must be 3*3 \n
 *	-- alpha must be in the range 0.0 to 1.0\n
 *	fspecial(EAGLEEYE_LOG,kernel_size,var_num = 1,double(sigma) \n
 *	fspecial(EAGLEEYE_PREWITT) \n
 *	fspecial(EAGLEEYE_SOBEL)
 */
EAGLEEYE_API Matrix<float> fspecial(SpecialType ker,int kernel_size[2] = NULL,int var_num=0,...);

/**
 *	@brief find the response max position in the region
 */
EAGLEEYE_API void responseMax(const Matrix<float>& img,const Matrix<float>& kernel,float& max_res,int& r_index,int& c_index);

/**
 *	@brief split to some sub matrix
 */
template<typename T>
void findHotSpot(const Matrix<T>& big_mat,int sub_size[2],int sub_nums,
	std::vector<Matrix<T>>& sub_mats,std::vector<std::pair<int,int>>& sub_offsets);

/**
 *	@brief implement filter operation based on Gaussian Kernel on the lattice.
 *	@note ArrayPos Array<float, n>; ArrayVal Array<float,m>
 */
template<class ArrayPos,class ArrayVal>
Matrix<ArrayVal> gKernelFilter(const Matrix<ArrayPos>& pos_mat,const Matrix<ArrayVal>& val_mat);

/**
 *	@brief generate random matrix
 *	@param var the random variable
 *	@param rows the matrix rows
 *	@param cols the matrix cols
 */
template<class T>
Matrix<T> randmat(int rows,int cols,Variable<T> var);

enum ThresholdMethod
{
	OSTU,
	ENTROPIC_ANALYSIS
};
/**
 *	@brief find foreground and background based on histogram automatically
 */
template<class T>
void autoBWSplit(const Matrix<T>& img,float& split_threshold,
				 float& foreground_mean,
				 float& background_mean,
				 ThresholdMethod t_method = OSTU,
				 bool auto_fore_background_judge = false,
				 float his_limit_ratio = 0.0f);

enum ProbabilityEstimatorModel
{
	SINGLE_GAUSSIAN_MODEL,
	GAUSSIAN_KDE_MODEL
};
/**
 *	@brief finding decision bounds
 *	@param positive_samples (1-dimension)
 *	@param negative_samples	(1-dimension)
 *	@param p_m probability estimator model GAUSSIAN,GAUSSIAN_KDE
 *	@param optimum_split_flag whether using the optimum split position\n
 *			(minimizing the misclassification rate)
 *	@param para if optimum_split_flag = false, it has to suffer from some restriction
 *	@return split position
 */
EAGLEEYE_API Matrix<float> autoProbabilityDecision(const Matrix<float>& positive_samples,
								const Matrix<float>& negative_samples, 
								ProbabilityEstimatorModel p_m = SINGLE_GAUSSIAN_MODEL,
								bool optimum_decision_flag = true,void* para = NULL);

/**
 *	@brief translation image with shift_x and shift_y
 */
Matrix<float> shiftPad(const Matrix<float>& img,float shift_x,float shift_y);

/**
 *	@brief split to grid
 *	@note like meshgrid in matlab
 */
void meshgrid(int x_start,int x_end, int y_start, int y_end, 
			  Matrix<int>& x, Matrix<int>& y);

/**
 *	@brief map mess label matrix to ordered label matrix
 */
EAGLEEYE_API void maptoOrder(Matrix<int>& mess_to_order,int& num);

enum InterpMethod
{
	LINEAR_INTERPOLATION,
	BILINEAR_INTERPOLATION,
	SPLINE_INTERPOLATION,
	NEAREST_NEIGHBOR_INTERPOLATION
};
/**
 *	@brief interpolation function
 */
EAGLEEYE_API void interp(const Matrix<float>& grid_index_vec,const Matrix<float>& grid_val_vec,
			const Matrix<float>& index_vec,Matrix<float>& val_vec,
			InterpMethod interp_type = LINEAR_INTERPOLATION,float a = 0.0f,float b = 0.0f);

/**
 *	@brief resize the image by using bilinear interpolation
 */
template<typename T>
Matrix<T> resize(const Matrix<T>& img,float scale,InterpMethod interp_method = BILINEAR_INTERPOLATION);
template<typename T>
Matrix<T> resize(const Matrix<T>& img,int after_r,int after_c,InterpMethod interp_method = BILINEAR_INTERPOLATION);

/**
 *	@brief finding max value	
 */
EAGLEEYE_API void qdmax(const Matrix<float>& x,const Matrix<float>& y,float& xm,float& ym);

/**
 *	@brief squeeze image
 *	@note we shouldn't make img and squeezed_img share one memory block
 */
template<typename T>
void squeezeRegion(const Matrix<T>& img, T squeezed_label,int squeezed_size,Matrix<T>& squeezed_img);

enum KLMode
{
	DEFAULT_KL,
	SYM_KL,
	JS_KL
};
/**
 *	@brief Kullback-Leibler or Jensen-Shannon divergence between two distributions\
 *	@detail kldiv(p1,p2,DEFAULT_KL) return the Kullback-Leibler divergence between
 *	two distributions P1,P2. P1 is a length-M vector of probabilities representing distribution 1, 
 *	and P2 is a length-M vector of probabilities representing distribution 2. The Kullback-Leibler
 *	divergence is given by:
 *	KL(P1(x),P2(x)) = sum[P1(x).log(P1(x)/P2(x))]
 *	kldiv(p1,p2,SYM_KL) returns a symmetric variant of the Kullback-Leibler divergence, given by [KL(P1,P2)+KL(P2,P1)]/2. 
 *	See Johnson and Sinanovic (2001).
 *  kldiv(p1,p2,JS_KL) returns the Jensen-Shannon divergence, given by [KL(P1,Q)+KL(P2,Q)]/2, where Q = (P1+P2)/2. 
 *  See the Wikipedia article for
 *  "Kullback-Leibler divergence". This is equal to 1/2 the so-called "Jeffrey divergence".
 *  See Rubner et al.(2000).
 *	@note the elements of probability vectors p1 and p2 must each sum to 1 +/- 0.0001.\n
 *	this function comes from "KLDIV" written by David Fass
 *	@par example(matlab style):
 *	p1 = ones(1,5) / 5;
 *	p2 = [0,0,0.5,0.2,0.3] + eps;
 *	KL = kldiv(p1,p2);
 *	KL = 19.4899
 */
EAGLEEYE_API float kldiv(const Matrix<float>& p1,const Matrix<float>& p2,KLMode kl_mode = DEFAULT_KL);
}

#include "MatrixMath.hpp"
#endif