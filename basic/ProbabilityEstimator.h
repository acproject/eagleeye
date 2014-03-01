#ifndef _PROBABILITYESTIMATOR_H_
#define _PROBABILITYESTIMATOR_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"

namespace eagleeye
{
class ProbabilityEstimator
{
public:
	ProbabilityEstimator(){};
	~ProbabilityEstimator(){};

	virtual void building(const Matrix<float>& sampling_data){};
	virtual float p(float sampling_pos){return 0.0f;};
	virtual Matrix<float> getSamplingData(){return Matrix<float>();};
	Matrix<float> m_sampling_data;
};

class SingleGaussianEstimator:public ProbabilityEstimator
{
public:
	SingleGaussianEstimator();
	~SingleGaussianEstimator(){};

	virtual void building(const Matrix<float>& sampling_data);
	virtual float p(float sampling_pos);
	virtual Matrix<float> getSamplingData();

private:
	float m_mean;
	float m_variance;

	float m_first_const_val;
	float m_second_const_val;
};

/**
 *	@brief using gaussian kernel density estimator
 *	@note see Pattern Recognization and Machine Learning P122
 */
class GaussianKDE:public ProbabilityEstimator
{
public:
	GaussianKDE(float h_2);
	~GaussianKDE(){};

	virtual void building(const Matrix<float>& sampling_data);
	virtual float p(float sampling_pos);
	virtual Matrix<float> getSamplingData();

private:
	Matrix<float> m_sampling_data;
	int m_sampling_num;

	float m_first_const_val;
	float m_second_const_val;
};

}

#endif