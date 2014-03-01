#ifndef _SEMANTICBOWDESCRIPTOREXTRACTOR_H_
#define _SEMANTICBOWDESCRIPTOREXTRACTOR_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "DescriptorExtractor.h"
#include <string>
#include <opencv2/opencv.hpp>

namespace eagleeye
{
/**
 *	@brief extract semantic 'bagofwords' feature, which would be feed to
 *	SemanticBagOfWords
 */
class EAGLEEYE_API SemanticBOWDescriptorExtractor
{
public:
	SemanticBOWDescriptorExtractor(DescriptorExtractor* des_extract = NULL);
	virtual ~SemanticBOWDescriptorExtractor();

	/**
	 *	@brief set descriptor extractor
	 */
	void setDescriptorExtractor(DescriptorExtractor* des_extract);

	/**
	 *	@brief compute image descriptors
	 */
	void compute(const Matrix<float>& img, std::vector<KeyPoint>& keypoints, Matrix<float>& img_descriptors,
		std::vector<std::vector<int> >* point_idxs_of_clusters = 0);

	/**
	 *	@brief compute every superpixel descriptors of image
	 */
	void compute(const Matrix<float>& superpixel_img, const Matrix<int>& superpixel_index_img, int superpixel_num,
		std::vector<KeyPoint>& keypoints, Matrix<float>& superpixel_descriptors,std::vector<bool>& superpixel_flag);

	/**
	 *	@brief set minimum statistic number
	 *	@note if keypoints number in some superpixel is less than this value, we would
	 *	consider this superpixel description is invalid.
	 */
	void setMinimumStatisticNum(int num);

	/**
	 *	@brief set/get vocabulary
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
	int m_words_num;
	std::vector<Matrix<float>> m_wordspair_dis;
	std::vector<Matrix<float>> m_wordspair_angle;

	int m_wordpair_dis_dim;
	int m_wordpair_angle_dim;

	int m_min_statistic_num;

	float m_max_dis;
};
}

#endif