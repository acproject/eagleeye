#ifndef _COLORSPACE_H_
#define _COLORSPACE_H_

#include "EagleeyeMacro.h"

#include "Matrix.h"
#include "Array.h"
namespace eagleeye
{
/**
 *	@brief RGB to XYZ conversion
 */
EAGLEEYE_API Matrix<Array<float,3>> rgb2xyz(const Matrix<ERGB>& rgb);

/**
 *	@brief RGB to CIELAB
 */
EAGLEEYE_API Matrix<Array<float,3>> rgb2lab(const Matrix<ERGB>& rgb);

/**
 *	@brief build color table automatically
 */
Matrix<ERGB> autoBuildColorTable(int class_num);
}
#endif