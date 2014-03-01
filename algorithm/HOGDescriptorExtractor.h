#ifndef _HOGDESCRIPTOREXTRACTOR_H_
#define _HOGDESCRIPTOREXTRACTOR_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "DescriptorExtractor.h"

namespace eagleeye
{
class EAGLEEYE_API HOGDescriptorExtractor:public DescriptorExtractor
{
public:
	HOGDescriptorExtractor();
	virtual ~HOGDescriptorExtractor();

	virtual int descriptorSize();

protected:
	virtual void computeImpl(const Matrix<float>& img,std::vector<KeyPoint>& keypoints, Matrix<float>& img_descriptors);
};
}

#endif