#include "ProbabilityEstimator.h"
#include <math.h>

namespace eagleeye
{
SingleGaussianEstimator::SingleGaussianEstimator()
{
	m_mean = 0.0f;
	m_variance = 1.0f;
}

float SingleGaussianEstimator::p(float sampling_pos)
{
	float p_val = m_first_const_val * exp(m_second_const_val * (sampling_pos - m_mean) * (sampling_pos - m_mean));
	return p_val;
}

void SingleGaussianEstimator::building(const Matrix<float>& sampling_data)
{
	assert(sampling_data.rows() == 1 || sampling_data.cols() == 1);
	//copy sampling data
	if (sampling_data.rows() == 1)
	{
		m_sampling_data = Matrix<float>(1,sampling_data.cols(),(void*)sampling_data.row(0),true);
	}
	else
	{
		m_sampling_data = Matrix<float>(1,sampling_data.rows(),(void*)sampling_data.row(0),true);
	}

	int num = m_sampling_data.cols();
	float* data = m_sampling_data.row(0);
	m_mean = 0.0f;

	for (int i = 0; i < num; ++i)
	{
		m_mean += data[i] / float(num);
	}

	m_variance = 0.0f;
	for (int i = 0; i < num; ++i)
	{
		m_variance += (data[i] - m_mean) * (data[i] - m_mean);
	}
	m_variance = m_variance / float(num - 1);

	m_first_const_val =  1.0f / sqrt(2.0f * float(EAGLEEYE_PI) * m_variance);
	m_second_const_val = -1.0f / (2.0f * m_variance);
}

Matrix<float> SingleGaussianEstimator::getSamplingData()
{
	return m_sampling_data;
}

GaussianKDE::GaussianKDE(float h_2)
{
	m_first_const_val = 1.0f / sqrt(2.0f * float(EAGLEEYE_PI) * h_2);
	m_second_const_val = -1.0f / (2.0f * h_2);
}
void GaussianKDE::building(const Matrix<float>& sampling_data)
{
	assert((sampling_data.cols() == 1) || (sampling_data.rows() == 1));
	if(sampling_data.rows() == 1)
		m_sampling_data = Matrix<float>(1,sampling_data.cols(),(void*)sampling_data.row(0),true);
	else
		m_sampling_data = Matrix<float>(1,sampling_data.rows(),(void*)sampling_data.row(0),true);

	m_sampling_num = m_sampling_data.cols();
}
float GaussianKDE::p(float sampling_pos)
{
	float p_val = 0.0f;
	float* data = m_sampling_data.dataptr();
	for (int i = 0; i < m_sampling_num; ++i)
	{
		p_val += m_first_const_val * exp(m_second_const_val * (data[i] - sampling_pos) * (data[i] - sampling_pos));		
	}

	return p_val / m_sampling_num;
}

Matrix<float> GaussianKDE::getSamplingData()
{
	return m_sampling_data;
}

}