#include "HOGDescriptorExtractor.h"

namespace eagleeye
{
HOGDescriptorExtractor::HOGDescriptorExtractor()
{

}
HOGDescriptorExtractor::~HOGDescriptorExtractor()
{

}

int HOGDescriptorExtractor::descriptorSize()
{
	return 0;
}

void HOGDescriptorExtractor::computeImpl(const Matrix<float>& img,std::vector<KeyPoint>& keypoints, Matrix<float>& img_descriptors)
{

}
}