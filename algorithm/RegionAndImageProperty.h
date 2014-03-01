#ifndef _REGIONANDIMAGEPROPERTY_H_
#define _REGIONANDIMAGEPROPERTY_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"

namespace eagleeye
{
/**
 *	@brief 	get connected components of 2-D binary image
 *	@param binary_img binary image
 *	@param label connected components image
 *	@param label_num the number of connected components
 *	@param neighborhood the neighborhood number
 *	@detail
 *	labeling scheme 
 *	+-+-+-+ 
 *	|D|C|E| 
 *	+-+-+-+ 
 *	|B|A| | 
 *	+-+-+-+ 
 *	| | | | 
 *	+-+-+-+ 
 *	A is the center pixel of a neighborhood.  In the 3 versions of 
 *	connectedness: 
 *	4:  A connects to B and C 
 *	8:  A connects to B, C, D, and E 
 */
EAGLEEYE_API void bwlabel(const Matrix<unsigned char>& binary_img, Matrix<int>& label, 
						  int& label_num, int neighborhood=4);

}

#endif