#ifndef _FEATUREDETECTORSURFOPENCV_H_
#define _FEATUREDETECTORSURFOPENCV_H_

#include "EagleeyeMacro.h"
#include "FeatureDetector.h"
namespace eagleeye
{
class EAGLEEYE_API SurfFeatureDetectorOpenCV:public FeatureDetector
{
public:
	SurfFeatureDetectorOpenCV();
	~SurfFeatureDetectorOpenCV();

	virtual void detect(const Matrix<float>& img,std::vector<KeyPoint>& keypoints);
	
	void setHessianThreshold(float hessian_threshold = 400);

private:
	float m_heissian_threshold;
};
}

#endif