#ifndef _FEATUREDESCRIPTION_H_
#define _FEATUREDESCRIPTION_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "Algorithm.h"
#include "FeatureDetector.h"
#include "MemoryBlock.h"
#include <vector>

namespace eagleeye
{
class EAGLEEYE_API DescriptorExtractor:public Algorithm
{
public:
	DescriptorExtractor();
	virtual ~DescriptorExtractor();
	
	/**
	 *	@brief set class identity
	 */
	EAGLEEYE_CLASSIDENTITY(DescriptorExtractor);

	/**
	 *	@brief compute the description of keypoint
	 *	@param img image
	 *	@param keypoints input collection of keypoints. Keypoints for which a descriptor cannot 
	 *	be computed are removed. Sometimes new keypoints can be added.
	 */
	void compute( const Matrix<float>& img,std::vector<KeyPoint>& keypoints,
		Matrix<float>& img_descriptors );

	/**
	 *	@brief compute every superpixel descriptors of image
	 */
	void compute(const Matrix<float>& superpixel_img, const Matrix<int>& superpixel_index_img, int superpixel_num,
		std::vector<KeyPoint>& keypoints, Matrix<float>& superpixel_descriptors,std::vector<bool>& superpixel_flag);

	virtual int descriptorSize(){return 0;};

protected:
	/**
	 *	@brief compute every keypoint description on the image
	 */
	virtual void computeImpl(const Matrix<float>& img,std::vector<KeyPoint>& keypoints,
		Matrix<float>& img_descriptors){};

	/**
	 *	@brief compute every superpixel description on the image
	 */
	virtual void computeImpl(const Matrix<float>& superpixel_img, const Matrix<int>& superpixel_index_img, 
		int superpixel_num,
		std::vector<KeyPoint>& keypoints, 
		Matrix<float>& superpixel_descriptors){};

};
}

#endif