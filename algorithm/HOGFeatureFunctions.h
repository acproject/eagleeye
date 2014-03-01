#ifndef _HOGFEATUREFUNCTIONS_H_
#define _HOGFEATUREFUNCTIONS_H_

#include "EagleeyeMacro.h"

#include "DataPyramid.h"
#include "Matrix.h"
#include "MatrixAuxiliary.h"
#include "MatrixMath.h"
#include "Types.h"
#include "Array.h"
#include <vector>
#include "Variable.h"
#include "EagleeyeCore.h"

namespace eagleeye
{
/**
 *	@brief generate a HOG feature with 32 dimension for every pixel
 *	@note img must be full
 */
EAGLEEYE_API 
Matrix<HOG32Vec> generateHOG32Features(const Matrix<float>& img,int sbin);

/**
 *	@brief generate HOG feature pyramid for one image
 *	@note img must be full
 */
EAGLEEYE_API 
HOG32Pyramid generateHOG32Pyramid(const Matrix<float>& img,int interval,int sbin,int padx=0,int pady=0);

/**
 *	@brief compute convolution between one HOG feature image and one HOG feature filter
 *	@note hog_feat and hog_weight must be full
 */
EAGLEEYE_API 
Matrix<float> convWithHOG32Features(const Matrix<HOG32Vec>& hog_feat,const Matrix<HOG32Vec>& hog_weight);


/**
 *	@brief Judge whether the overlap extent between detector window and bbox could \n
 *	satisfy the requirement.
 *	@param bbox the target region (the original scale)
 *	@param detector_win the detector window (the filter size) with "scale"
 *	@param detector_range the detector range with "scale"
 *	@param overlap the overlap threshold
 *	@param scale the scale stayed by detector(the filter)
 */
bool overlapDetect(int bbox[4],int detector_win[2],int detector_range[2],float overlap,float scale=1);


/**
 *	@brief find valid levels for detector in the Pyramid.
 */
std::vector<int> getValidLevels(const HOG32Pyramid& pyramid,
								const int sbin,
								int bbox[4],int detector_win[2],float overlap);
std::vector<int> getValidLevels(const HOG32Pyramid& pyramid,
								const int sbin,
								int start_level,int end_level,
								int bbox[4],int detector_win[2],float overlap);
}

#endif