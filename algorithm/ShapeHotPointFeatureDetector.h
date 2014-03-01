#ifndef _SHAPEHOTPOINTFEATUREDETECTOR_H_
#define _SHAPEHOTPOINTFEATUREDETECTOR_H_

#include "EagleeyeMacro.h"
#include "FeatureDetector.h"
#include <vector>
#include "Array.h"
#include "MatrixMath.h"

namespace eagleeye
{
class EAGLEEYE_API ShapeHotPointFeatureDetector:public FeatureDetector
{
public:
	ShapeHotPointFeatureDetector();
	virtual ~ShapeHotPointFeatureDetector();

	/**
	 *	@brief detect keypoints in every superpixel
	 */
	virtual void detect(const Matrix<float>& img,std::vector<KeyPoint>& keypoints);

	/**
	 *	@brief set sampling limit ratio
	 */
	void setSamplingLimitRatio(float ratio);

	/**
	 *	@brief we don't extract any point in image bounds
	 */
	void setImageBounds(int bound_width,int bound_height);

	/**
	 *	@brief set canny threshold
	 */
	void setCannyThreshold(double low_threshold,double high_threshold);

	/**
	 *	@brief set random jitter switch
	 *	@note if it's true, we would sample points around edges. Otherwise,
	 *	it could only sample points at edges.
	 */
	void setRandomJitterSwitch(bool flag,float jitter_degree,int jitter_num = 10);

private:
	int m_jitter_num;
	int m_jitter_switch;
	float m_jitter_degree;
	double m_canny_low_threshold;
	double m_canny_high_threshold;

	int m_img_bound_width;
	int m_img_bound_height;

	float m_sampling_limit_ratio;
};
}

#endif