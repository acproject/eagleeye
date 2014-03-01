#ifndef _DENSEFEATUREDETECTOR_H_
#define _DENSEFEATUREDETECTOR_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "FeatureDetector.h"

namespace eagleeye
{
class EAGLEEYE_API DenseFeatureDetector:public FeatureDetector
{
public:
	DenseFeatureDetector();
	virtual ~DenseFeatureDetector();

	/**
	 *	@brief detect keypoints 
	 */
	virtual void detect(const Matrix<float>& img,std::vector<KeyPoint>& keypoints);	
	void detect(int rows,int cols,std::vector<KeyPoint>& keypoints);

	/**
	 *	@brief set dense detector parameters
	 *	@param init_scale initial scale
	 *	@param scale_levels number of scale levels
	 *	@param scale_mul scale multiplier (second level: init_scale*scale_mul,...)
	 *	@param init_xy_step sample step along x and y direction
	 *	@param init_img_bound initial image bound
	 *	@param search_radius computing the main direction in this region
	 *	@param vary_xy_step_with_scale whether sample step is changed with scale
	 *	@param vary_img_bound_with_scale whether image bound is changed with scale
	 */
	void setDetectorParams(float init_scale = 1,int scale_levels = 1,float scale_mul = 1,int init_xy_step = 10,
		int init_img_bound = 10,int search_radius = 10,
		bool vary_xy_step_with_scale = false,
		bool vary_img_bound_with_scale = false);

private:
	float m_init_scale;
	int m_scale_levels;
	float m_scale_mul;
	int m_init_xy_step;
	int m_init_img_bound;
	bool m_vary_xy_step_with_scale;
	bool m_vary_img_bound_with_scale;
	int m_search_radius;
};
}

#endif