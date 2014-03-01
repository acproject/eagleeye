#include "SiftDescriptorExtractorOpenCV.h"
#include "opencv2/opencv.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
SiftDescriptorExtractorOpenCV::SiftDescriptorExtractorOpenCV()
{
	m_edge_threshold = 10.0f;
	m_contrast_threshold = 0.04f;
	m_sigma = 1.6f;

	m_n_octavelayers = 1;

	m_des_size = 0;
}
SiftDescriptorExtractorOpenCV::~SiftDescriptorExtractorOpenCV()
{

}

void SiftDescriptorExtractorOpenCV::computeImpl( const Matrix<float>& img,std::vector<KeyPoint>& keypoints,
					 Matrix<float>& descriptors )
{
	int img_rows = img.rows();
	int img_cols = img.cols();
	cv::Mat cv_img(img.rows(),img.cols(),CV_8U);

	for (int i = 0; i < img_rows; ++i)
	{
		const float* img_data = img.row(i);
		unsigned char* cv_img_data = cv_img.ptr<unsigned char>(i);
		for (int j = 0; j < img_cols; ++j)
		{
			cv_img_data[j] = (unsigned char)img_data[j];
		}
	}

	std::vector<cv::KeyPoint> cv_keypoints;
	int key_points_num = keypoints.size();
	cv_keypoints.resize(key_points_num);

	for (int key_index = 0; key_index < key_points_num; ++key_index)
	{
		cv_keypoints[key_index].pt.x = keypoints[key_index].pt[0];
		cv_keypoints[key_index].pt.y = keypoints[key_index].pt[1];

		cv_keypoints[key_index].size = keypoints[key_index].size;
		cv_keypoints[key_index].angle = keypoints[key_index].angle;
		cv_keypoints[key_index].response = keypoints[key_index].response;
		cv_keypoints[key_index].octave = keypoints[key_index].octave;
		cv_keypoints[key_index].class_id = keypoints[key_index].class_id;
	}

	cv::Mat cv_des;
	cv::SIFT cv_sift_des(0,m_n_octavelayers,m_contrast_threshold,m_edge_threshold,m_sigma);
	cv_sift_des.compute(cv_img,cv_keypoints,cv_des);

	m_des_size = cv_des.cols;

	descriptors = Matrix<float>(cv_des.rows,cv_des.cols);
	for (int i = 0; i < cv_des.rows; ++i)
	{
		memcpy(descriptors.row(i), cv_des.ptr<float>(i),sizeof(float) * cv_des.cols);
	}
}

int SiftDescriptorExtractorOpenCV::descriptorSize()
{
	return m_des_size;
}

void SiftDescriptorExtractorOpenCV::setExtractorParams(int octave_layers,float contrast_threshold,float edge_threshold,float sigma)
{
	m_n_octavelayers = octave_layers;
	m_contrast_threshold = contrast_threshold;
	m_edge_threshold = edge_threshold;
	m_sigma = sigma;
}

void SiftDescriptorExtractorOpenCV::setUnitPara(MemoryBlock param_block)
{
	_Parameter* me_param_block = (_Parameter*)param_block.block();
	setExtractorParams(me_param_block->n_octavelayers,
		me_param_block->contrast_threshold,
		me_param_block->edge_threshold,
		me_param_block->sigma);
}
void SiftDescriptorExtractorOpenCV::getUnitPara(MemoryBlock& param_block)
{
	param_block = MemoryBlock(sizeof(_Parameter));
	_Parameter* me_param_block = (_Parameter*)param_block.block();

	me_param_block->contrast_threshold = m_contrast_threshold;
	me_param_block->edge_threshold = m_edge_threshold;
	me_param_block->n_octavelayers = m_n_octavelayers;
	me_param_block->sigma = m_sigma;
}

}