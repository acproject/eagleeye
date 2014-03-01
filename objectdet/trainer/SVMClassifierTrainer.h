#ifndef _SVMCLASSIFIERTRAINER_H_
#define _SVMCLASSIFIERTRAINER_H_

#include "EagleeyeMacro.h"
#include "SignalFactory.h"
#include "ClassifierTrainer.h"
#include "Learning/libsvm/LibSVM.h"
#include "Learning/CrossValidation.h"

namespace eagleeye
{
class EAGLEEYE_API SVMClassifierTrainer:public ClassifierTrainer
{
public:
	typedef SVMClassifierTrainer							Self;
	typedef ClassifierTrainer								Superclass;

	SVMClassifierTrainer(); 
	virtual ~SVMClassifierTrainer();

	/**
	 *	@brief get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(SVMClassifierTrainer);
	
	/**
	 *	@brief set cross validation mode
	 */
	void setCrossValidation(bool switch_flag,bool disturb_order = false,CrossValidationMode mode = K_10_FOLDER);

	/**
	 *	@brief whether we would resample samples
	 */
	void setResamplingSamples(bool flag);

	/**
	 *	@brief enable top hit strategy
	 */
	void setTopHitStrategy(bool flag,int k_top_hit = 3);

	/**
	 *	@brief set/get libsvm kernel type
	 */
	void setlibsvmKernelType(const libsvm::KernelType kernel_type);
	void setlibsvmKernelType(const int kernel_type);
	void getlibsvmKernelType(int& kernel_type);

	/**
	 *	@brief set/get libsvm type
	 */
	void setlibsvmType(const libsvm::SVMType svm_type);
	void setlibsvmType(const int svm_type);
	void getlibsvmType(int& svm_type);

	/**
	 *	@brief training process
	 */
	virtual void train();

protected:
	/**
	 *	@brief check whether it needs to be processed.
	 *	@note if the model parameters file have been existed, it wouldn't execute again.
	 *	It would walk into the next node directly.
	 */
	virtual bool isNeedProcessed();

	/**
	 *	@brief check whether some preliminary conditions have been satisfied.
	 */
	virtual bool selfcheck();

private:
	SVMClassifierTrainer(const SVMClassifierTrainer&);
	void operator=(const SVMClassifierTrainer&);

	libsvm::KernelType m_kernel_type;
	libsvm::SVMType m_svm_type;

	bool m_switch_flag;
	CrossValidationMode m_cross_validation_mode;
	bool m_disturb_order_flag;

	bool m_resample_flag;
	bool m_top_hit_mode;
	int m_k_top_hit;
};
}

#endif