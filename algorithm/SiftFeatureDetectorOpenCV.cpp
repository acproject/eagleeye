#include "SiftFeatureDetectorOpenCV.h"
#include <stdio.h>
#include "opencv2/opencv.hpp"
#include "opencv2/features2d/features2d.hpp"  
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
SiftFeatureDetectorOpenCV::SiftFeatureDetectorOpenCV()
{
	m_edge_threshold=10.0f;
	m_contrast_threshold=0.04f;
	m_sigma=1.6f;
}

SiftFeatureDetectorOpenCV::~SiftFeatureDetectorOpenCV()
{

}

void SiftFeatureDetectorOpenCV::detect(const Matrix<float>& img, std::vector<KeyPoint>& keypoints)
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

	cv::SIFT cv_sift(0,m_n_octavelayers,m_contrast_threshold,m_edge_threshold,m_sigma);
	cv_sift.detect(cv_img,cv_keypoints);

	int key_points_num=cv_keypoints.size();
	keypoints.resize(key_points_num);

	for (int key_index = 0; key_index < key_points_num; ++key_index)
	{
		keypoints[key_index].pt[0]=cv_keypoints[key_index].pt.x;
		keypoints[key_index].pt[1]=cv_keypoints[key_index].pt.y;
		
		keypoints[key_index].size=cv_keypoints[key_index].size;
		keypoints[key_index].angle=cv_keypoints[key_index].angle;
		keypoints[key_index].response=cv_keypoints[key_index].response;
		keypoints[key_index].octave=cv_keypoints[key_index].octave;
		keypoints[key_index].class_id=cv_keypoints[key_index].class_id;
	}
}

void SiftFeatureDetectorOpenCV::setDetectorParams(float contrast_threshold,float edge_threshold,float sigma)
{
	m_contrast_threshold=contrast_threshold;
	m_edge_threshold=edge_threshold;
	m_sigma=sigma;
}

}
