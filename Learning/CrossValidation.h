#ifndef _CROSSVALIDATION_H_
#define _CROSSVALIDATION_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include <string>
#include <vector>
#include <map>

namespace eagleeye
{
class DummyLearner
{
public:
	DummyLearner(){};
	virtual ~DummyLearner(){};
	virtual void learn(const Matrix<float>& samples,const Matrix<float>& labels) = 0;
	virtual float evalution(const Matrix<float>& validation_samples,const Matrix<float>& labels) = 0;
	virtual void save(const char* model_path) = 0;
};

enum CrossValidationMode
{
	K_10_FOLDER,
	K_5_FOLDER,
	LEAST_ONE_OUT
};

class EAGLEEYE_API CrossValidation
{
public:
	CrossValidation(DummyLearner* dummy_learner,const Matrix<float>& samples,const Matrix<float>& labels);
	~CrossValidation();

	/**
	 *	@brief start cross validation
	 */
	void startCrossValidation(const char* folder_path);

	/**
	 *	@brief set cross validation mode
	 */
	void setCrossValidationMode(CrossValidationMode mode);

	/**
	 *	@brief disturb samples order
	 */
	void disturbOrder(bool flag);

	/**
	 *	@brief get optimum model index
	 */
	int getOptimumModelIndex();

protected:
	/**
	 *	@brief execute 10 folder cross validation
	 */
	void k10folder(const char* folder_path = NULL);

	/**
	 *	@brief execute 5 folder cross validation
	 */
	void k5folder(const char* folder_path = NULL);

	/**
	 *	@brief execute least-one-out cross validation
	 */
	void leastOneOut(const char* folder_path = NULL);

private:
	CrossValidationMode m_mode;
	Matrix<float> m_samples;
	Matrix<float> m_labels;
	DummyLearner* m_dummy_learner;

	float m_optimum_error;
	float m_maximum_error;
	float m_average_error;
	int m_optimum_index;
	
	bool m_disturb_order_flag;
};
}

#endif