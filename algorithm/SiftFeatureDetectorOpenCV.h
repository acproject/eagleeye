#ifndef _FEATUREDETECTOROPENCV_H_
#define _FEATUREDETECTOROPENCV_H_

#include "EagleeyeMacro.h"
#include "FeatureDetector.h"
#include <vector>

namespace eagleeye
{
class EAGLEEYE_API SiftFeatureDetectorOpenCV:public FeatureDetector
{
public:
	SiftFeatureDetectorOpenCV();
	virtual ~SiftFeatureDetectorOpenCV();

	virtual void detect(const Matrix<float>& img, std::vector<KeyPoint>& keypoints);

	void setDetectorParams(float contrast_threshold,float edge_threshold,float sigma);

private:
	float m_edge_threshold;
	float m_contrast_threshold;
	float m_sigma;
};

}

#endif