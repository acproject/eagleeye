#include "BOWDescriptorExtractorOpenCV.h"
#include "Matlab/MatlabInterface.h"
#include <iostream>

namespace eagleeye
{
BOWDescriptorExtractorOpenCV::BOWDescriptorExtractorOpenCV(DescriptorExtractor* des_extract)
{
	m_des_matcher = cv::DescriptorMatcher::create("BruteForce");
	m_des_extractor = des_extract;

	m_descriptor_size = 0;
	m_min_statistic_num = 100;
}
BOWDescriptorExtractorOpenCV::~BOWDescriptorExtractorOpenCV()
{
}

void BOWDescriptorExtractorOpenCV::setDescriptorExtractor(DescriptorExtractor* des_extract)
{
	m_des_extractor = des_extract;
}

void BOWDescriptorExtractorOpenCV::compute(const Matrix<float>& img,std::vector<KeyPoint>& keypoints, 
										   Matrix<float>& img_descriptors, 
										   std::vector<std::vector<int> >* point_idxs_of_clusters/* =0 */)
{
	m_descriptor_size = descriptorSize();

	//compute descriptors at key points in the image
	Matrix<float> descriptors;
	m_des_extractor->compute(img,keypoints,descriptors);

	//using opencv structure
	cv::Mat cv_img_des(descriptors.rows(),descriptors.cols(),CV_32F,descriptors.dataptr());

	std::vector<cv::DMatch> cv_matches;
	m_des_matcher->match(cv_img_des,cv_matches);

	if (point_idxs_of_clusters)
	{
		point_idxs_of_clusters->clear();
		point_idxs_of_clusters->resize(m_descriptor_size);
	}

	img_descriptors=Matrix<float>(1,m_descriptor_size,float(0));
	cv::Mat cv_img_descriptors(1,m_descriptor_size,CV_32F,img_descriptors.dataptr());
	float* cv_d_ptr=(float*)cv_img_descriptors.data;

	for( size_t i = 0; i < cv_matches.size(); i++ )
	{
		int query_idx = cv_matches[i].queryIdx;
		int train_idx = cv_matches[i].trainIdx; // cluster index

		cv_d_ptr[train_idx] = cv_d_ptr[train_idx] + 1.f;

		if( point_idxs_of_clusters )
			(*point_idxs_of_clusters)[train_idx].push_back( query_idx );
	}

	//normalize image descriptor
	for (int i = 0; i < m_descriptor_size; ++i)
	{
		cv_d_ptr[i] /= descriptors.rows();
	}
}

void BOWDescriptorExtractorOpenCV::compute(const Matrix<float>& superpixel_img, 
										   const Matrix<int>& superpixel_index_img,
										   int superpixel_num,
										   std::vector<KeyPoint>& keypoints, 
										   Matrix<float>& superpixel_descriptors,
										   std::vector<bool>& superpixel_flag)
{
	superpixel_flag.resize(superpixel_num,false);

	m_descriptor_size = descriptorSize();

	//compute descriptors for the image
	Matrix<float> descriptors;
	m_des_extractor->compute(superpixel_img, keypoints, descriptors);

	int keypoints_num = keypoints.size();

	std::vector<int> kp_count(keypoints_num);

	for (int kp_index = 0; kp_index < keypoints_num; ++kp_index)
	{
		cv::Mat cv_descriptor(1, descriptors.cols(), CV_32F, descriptors.row(kp_index));
		std::vector<cv::DMatch> cv_matches;
		m_des_matcher->match(cv_descriptor,cv_matches);

		int train_idx = cv_matches[0].trainIdx; // cluster index
		
		kp_count[kp_index] = train_idx;
	}

	superpixel_descriptors = Matrix<float>(superpixel_num, m_descriptor_size, 0.0f);
	std::vector<int> counts;
	counts.resize(superpixel_num,0);
	
	for (int kp_index = 0; kp_index < keypoints_num; ++kp_index)
	{
		int keypoint_c_index = int(keypoints[kp_index].pt[0]);
		int keypoint_r_index = int(keypoints[kp_index].pt[1]);

		int superpixel_index = superpixel_index_img.at(keypoint_r_index,keypoint_c_index);
		counts[superpixel_index]++;

		float* superpixel_descriptors_data = superpixel_descriptors.row(superpixel_index);

		int valid_dim_index = kp_count[kp_index];
		superpixel_descriptors_data[valid_dim_index] += 1.0f;
	}

	//normalize superpixel descriptor
	for (int superpixel_index = 0; superpixel_index < superpixel_num; ++superpixel_index)
	{
		if (counts[superpixel_index] != 0)
		{
			float* superpixel_descriptors_data = superpixel_descriptors.row(superpixel_index);
			for (int i = 0; i < m_descriptor_size; ++i)
			{
				superpixel_descriptors_data[i] /= counts[superpixel_index];
			}
		}
	}

	//check whether superpixel descriptor is valid
	for (int superpixel_index = 0; superpixel_index < superpixel_num; ++superpixel_index)
	{
		if (counts[superpixel_index] > m_min_statistic_num)
		{
			superpixel_flag[superpixel_index] = true;
		}
		else
		{
			superpixel_flag[superpixel_index] = false;
		}
	}
}


int BOWDescriptorExtractorOpenCV::descriptorSize()
{
	return m_descriptor_size;
}

void BOWDescriptorExtractorOpenCV::setVocabulary(const Matrix<float>& vocabulary)
{
	int rows = vocabulary.rows();
	int cols = vocabulary.cols();
	m_descriptor_size = rows;

	cv::Mat cv_vocabulary(rows,cols,CV_32F);
	memcpy(cv_vocabulary.data,vocabulary.dataptr(),sizeof(float)*rows*cols);

	m_des_matcher->clear();
	m_des_matcher->add(std::vector<cv::Mat>(1,cv_vocabulary));
}

Matrix<float> BOWDescriptorExtractorOpenCV::getVocabulary()
{
	std::vector<cv::Mat> vocabulary = m_des_matcher->getTrainDescriptors();
	cv::Mat vocabulary_mat = vocabulary[0];
	return Matrix<float>::mapfrom(vocabulary_mat.rows,vocabulary_mat.cols,vocabulary_mat.data);
}

}