#include "BOWDictionaryOpenCV.h"
#include "opencv2/opencv.hpp"

namespace eagleeye
{
BOWDictionaryOpenCV::BOWDictionaryOpenCV(int dictionary_size)
:Dictionary(dictionary_size)
{
}
BOWDictionaryOpenCV::~BOWDictionaryOpenCV()
{
}

Matrix<float> BOWDictionaryOpenCV::train()
{
	int elements_num = 0;
	int elements_dim = 0;
	std::vector<Matrix<float>>::iterator iter,iend(m_elements.end());
	for (iter = m_elements.begin(); iter != iend; ++iter)
	{
		elements_num += (*iter).rows();
	}

	elements_dim = m_elements[0].cols();

	int start = 0;
	cv::Mat cv_raw_elements(elements_num,elements_dim,CV_32F);
	for (iter = m_elements.begin(); iter != iend; ++iter)
	{
		memcpy(cv_raw_elements.ptr<float>(start),(*iter).dataptr(),sizeof(float) * (*iter).rows() * (*iter).cols());
		start += (*iter).rows();
	}

	//clear all invalid memory
	m_elements.clear();

	cv::BOWKMeansTrainer bow_trainer(m_dictionary_capacity);
	cv::Mat cv_dic = bow_trainer.cluster(cv_raw_elements);

	Matrix<float> dic(cv_dic.rows,cv_dic.cols);
	for (int i = 0; i < m_dictionary_capacity; ++i)
	{
		memcpy(dic.row(i),cv_dic.ptr<float>(i),sizeof(float) * cv_dic.cols);
	}

	return dic;
}
}