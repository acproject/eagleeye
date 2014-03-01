#ifndef _GRAMMARTREELEARNING_H_
#define _GRAMMARTREELEARNING_H_

#include "EagleeyeMacro.h"

#include "Learning/LearningMacro.h"
#include <vector>
#include "GrammarUnit.h"
#include "Variable.h"

namespace eagleeye
{
/**
 *	@brief optimize LSVM objective function(Object Detection Grammar Tree) via gradient descent
 *	@note we use an adaptive cache mechanism. After an example scores
 *	beyond the margin multiple times it is removed from the training set for a 
 *	fixed number of iterations.
 *	error loss = sum(1 - (w * psi + b)*t) + regularization item
 */
class ObjectDetGrammarTreeLatentSVM
{
public:
	ObjectDetGrammarTreeLatentSVM(GrammarTreeStructureInfo gt_info,float C = 0.1f,float J = 0.1f);
	~ObjectDetGrammarTreeLatentSVM();

	/**
	 *	@brief learn this model by using samples_label and samples_mat
	 */
	void learn();
	
	/**
	 *	@brief set latent variables
	 */
	void setTrainingSamples(const Matrix<int>& labels,const Matrix<int>& latent_variables,const Matrix<float>& features);
	
	/**
	 *	@brief set the start weight
	 */
	void setIniWeight(Matrix<float> ini_weight);

	/**
	 *	@brief get the optimum weight 
	 *	@note we have to call this function after running "learn()" function
	 */
	Matrix<float> getOptimumWeight();

	/**
	 *	@brief load samples from file
	 */
	void loadTrainingSamples(const char* latent_variables_file,const char* samples_file);
	
	/**
	 *	@brief load/save weight from file
	 */
	void loadWeights(const char* weight_file);
	void saveWeights(const char* weight_file);

protected:
	struct _Collapse
	{
		char** seq;
		int num;
	};

	void makeCollapse();
	float computeLoss(float out[3],float C,float J,float* w);
	float exScore(const char* ex,float* w);

private:	
	float m_C;
	float m_J;

	GrammarTreeStructureInfo m_gt_info;
	
	char** m_mess_samples;
	char** m_sorted_samples;
	_Collapse* m_collapses;
	int m_collapses_num;
	int m_samples_num;
	int m_len;							/**< Bytes number */
	Matrix<float> m_weight;
	Matrix<float> m_low_weight_bounds;
	int m_feature_dim;
	int m_latent_variables_num;
	float m_default_weight_low_bound;
};
}

#endif