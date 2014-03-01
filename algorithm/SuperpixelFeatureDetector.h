#ifndef _SUPERPIXELFEATUREDETECTOR_H_
#define _SUPERPIXELFEATUREDETECTOR_H_

#include "EagleeyeMacro.h"
#include "FeatureDetector.h"
#include <vector>
#include "Array.h"

namespace eagleeye
{
class EAGLEEYE_API SuperpixelFeatureDetector:public FeatureDetector
{
public:
	SuperpixelFeatureDetector();
	virtual ~SuperpixelFeatureDetector();

	/**
	 *	@brief detect keypoints in every superpixel
	 */
	virtual void detect(const Matrix<float>& img,std::vector<KeyPoint>& keypoints);
	
	/**
	 *	@brief set superpixel image
	 */
	void setSuperpixelImage(Matrix<int> superpixel_img,int superpixel_num,int exclude_index = -1);

	/**
	 *	@brief we don't extract any point in image bounds
	 */
	void setImageBounds(int bound_width,int bound_height);
	
	/**
	 *	@brief set minimum superpixel size
	 */
	void setMinimumSuperpixel(int area_size);

	/**
	 *	@brief set sampling number in every superpixel
	 */
	void setSamplingNum(int sampling_num = 1);

private:
	Matrix<int> m_superpixel_img;
	int m_superpixel_num;
	int m_bound_width;
	int m_bound_height;

	int m_minimum_superpixel_area;
	int m_exclude_index;
	int m_sampling_num;
};
}

#endif