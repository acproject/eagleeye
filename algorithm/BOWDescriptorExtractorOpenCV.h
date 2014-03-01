#ifndef _DESCRIPTOREXTRACTBOWOPENCV_H_
#define _DESCRIPTOREXTRACTBOWOPENCV_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "DescriptorExtractor.h"
#include <string>
#include <opencv2/opencv.hpp>

namespace eagleeye
{
class EAGLEEYE_API BOWDescriptorExtractorOpenCV
{
public:
	BOWDescriptorExtractorOpenCV(DescriptorExtractor* des_extract=NULL);
	virtual ~BOWDescriptorExtractorOpenCV();

	/**
	 *	@brief compute image descriptors
	 */
	void compute(const Matrix<float>& img, std::vector<KeyPoint>& keypoints, Matrix<float>& img_descriptors,
		std::vector<std::vector<int> >* point_idxs_of_clusters=0);

	/**
	 *	@brief compute every superpixel descriptors of image
	 */
	void compute(const Matrix<float>& superpixel_img, const Matrix<int>& superpixel_index_img, int superpixel_num,
		std::vector<KeyPoint>& keypoints, Matrix<float>& superpixel_descriptors,std::vector<bool>& superpixel_flag);

	/**
	 *	@brief set min statistic number ratio
	 */
	void setMinStatisticNumRatio(float ratio);

	void setDescriptorExtractor(DescriptorExtractor* des_extract);

	/**
	 *	@brief Set/Get vocabulary
	 */
	void setVocabulary(const Matrix<float>& vocabulary);
	Matrix<float> getVocabulary();

	/**
	 *	@brief get the size of every descriptor
	 */
	virtual int descriptorSize();
	
protected:
	int m_descriptor_size;						

	cv::Ptr<cv::DescriptorMatcher> m_des_matcher;
	DescriptorExtractor* m_des_extractor;

	int m_min_statistic_num;
};
}

#endif