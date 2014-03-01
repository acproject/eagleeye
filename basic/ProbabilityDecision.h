#ifndef _PROBABILITYDECISION_H_
#define _PROBABILITYDECISION_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
namespace eagleeye
{
class ProbabilityEstimator;
class ProbabilityDecision
{
public:
	ProbabilityDecision(ProbabilityEstimator* positive_generator,ProbabilityEstimator* negative_generator,int bins_num = 0);
	~ProbabilityDecision();
	
	/**
	 *	@brief set bins number for statistic
	 */
	void setBinsNum(int bins_num);
	
	/**
	 *	@brief finding decision bounds (min misclassification rate)
	 */
	Matrix<float> decisionMinMisClassificationRate();
		
	/**
	 *	@brief finding decision bounds (min expected loss)
	 */
	Matrix<float> decisionMinExpectedLoss(const Matrix<float>& loss_mat);

	/**
	 *	@brief finding decision bounds (reject option)
	 */
	Matrix<float> decisionRejectOption(float reject_probability_threshold);

protected:
	void findingMinMax(float& class_min_val,float& class_max_val);
	
private:
	ProbabilityEstimator* m_positive_generator;
	ProbabilityEstimator* m_negative_generator;
	int m_bins_num;
};
}

#endif