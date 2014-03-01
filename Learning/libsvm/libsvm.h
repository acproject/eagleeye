#ifndef _LIBSVM_H_
#define _LIBSVM_H_

#include "EagleeyeMacro.h"
#include "EagleeyeStr.h"
#include "Learning/LearningMacro.h"
#include "EagleeyeIO.h"
#include <map>

namespace eagleeye
{
class EAGLEEYE_API libsvm
{
public:
	enum SVMType
	{
		LIBSVM_C_SVC,
		LIBSVM_NU_SVC,
		LIBSVM_ONE_CLASS
	};
	enum KernelType
	{
		LIBSVM_LINEAR,
		LIBSVM_POLY,
		LIBSVM_RBF,
		LIBSVM_SIGMOD
	};

	libsvm();
	virtual ~libsvm();

	/**
	 *	@brief learn this model by using samples_label and samples_mat
	 *	@note learn model. After calling this function, we could call 
	 *	outcomeAnalyze(...).
	 */
	virtual bool learn();

	/**
	 *	@brief set SVM kernel type
	 */
	void setKernelType(KernelType ker);

	/**
	 *	@brief set svm type
	 */
	void setSVMType(SVMType svm_type);

	/**
	 *	@brief predict the labels of samples by using "Predict Model"
	 *	@note This function could be called independently. 
	 */
	virtual void predict(const Matrix<float> samples,Matrix<int>& predict_label,Matrix<float>& predict_estimate);

	/**
	 *	@brief set training samples matrix and label matrix
	 */
	virtual void setSamplesMat(Matrix<float> samples_label,Matrix<float> samples_mat);

	/**
	 *	@brief get class number
	 */
	int getClassNum();

	/**
	 *	@brief set probability estimate flag
	 */
	void setProbabilityEstimate(bool flag);

	/**
	 *	@brief Save/Read the learning model
	 *	@note The user has to call these two routines manually
	 */
	virtual void saveSVMModel(const char* model_file);
	virtual void readSVMModel(const char* model_file);

protected:
	/**
	 *	@brief selfcheck would be called in learn() implicitly
	 */
	virtual bool selfcheck();

	/**
	 *	@brief destroy libsvm model
	 *	@note This function would be called automatically.
	 */
	virtual void destroyAllResource();

private:
	std::string m_svm_parameter_str;		/**< svm parameter string*/
	KernelType m_kernel_type;				/**< svm kernel type*/
	SVMType m_svm_type;						/**< svm type*/

	bool m_predict_paobability;				/**< confidence degree flag*/

	Matrix<float> m_samples_label;
	Matrix<float> m_samples_feature;
	int m_samples_num;
	int m_feature_dim;
};
}

#endif