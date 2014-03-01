#ifndef _INFOTHEORYMATH_H_
#define _INFOTHEORYMATH_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "MatrixMath.h"

namespace eagleeye
{
/**
 *	@brief normalize object states
 *	@note normaliseObjectStates takes an input vector and writes an output vector
 *	which is a normalized version of the input, and returns the number of states
 *	A normalized array has min value = 0, max value = number of states
 *	and all values are integers
 */
EAGLEEYE_API int normalizeObjectStates(const Matrix<float>& input_vec,Matrix<int>& output_vec);

/**
 *	@brief mergeObjectStates takes in two arrays and writes the joint state of those arrays to the output vector
 */
EAGLEEYE_API int mergeObjectStates(const Matrix<float>& first_vec,const Matrix<float>& second_vec,Matrix<float>& output_vec);

struct JointProbabilityInfo
{
	Matrix<float> joint_proba_mat;
	Matrix<float> first_proba_vec;
	Matrix<float> second_proba_vec;
};

/**
 * @brief calculateJointProbability returns the joint probability vector of two vectors
 * and the marginal probability vectors in a struct.
 * @note It is the base operation for all information theory calculations involving two or more variables.
 */
EAGLEEYE_API JointProbabilityInfo calculateJointProbability(const Matrix<float>& first_vec,const Matrix<float>& second_vec);

/**
 *	@brief calculateProbability returns the probability vector from one vector.
 *	@note It is the base operation for all information theory calculations involving one variable
 */
EAGLEEYE_API Matrix<float> calculateProbability(const Matrix<float>& data_vec);

/**
 *	@brief compute the (joint) entropy for the joint distribution corresponding to probability vector P. 
 *	@note The elements of probability vector p must sum to  +/- 0.0001.
 */
EAGLEEYE_API float entropy(const Matrix<float>& proba);

/**
 *	@brief calculateEntropy returns the entropy in log base 2 of dataVector H(X)
 */
EAGLEEYE_API float calculateEntropy(const Matrix<float>& data_vec);
/**
 *	@brief calculateJointEntropy returns the entropy in log base 2 of the joint 
 *	variable of first_vec and second_vec H(XY)
 */
EAGLEEYE_API float calculateJointEntropy(const Matrix<float>& first_vec,const Matrix<float>& second_vec);

/**
 *	@brief calculateConditionalEntropy returns the entropy in log base 2 of data_vec conditioned 
 *	on condition_vec, H(X|Y)
 */
EAGLEEYE_API float calculateConditionalEntorpy(const Matrix<float>& data_vec,const Matrix<float>& condition_vec);

/**
 *	@brief mi returns the log base 2 mutual information between data_vec and target_vec, I(X,Y)
 *	@detail Mutual information is a measure of the inherent dependence expressed in the joint \n
 *	distribution of X and Y relative to the joint distribution of X and Y under the assumption of independence.\n
 *	Intuitively,mutual information measures the information that X and Y share: \n
 *	it measures how much knowing one of these variables reduces uncertainty about the other\n
 *	@note data_vec.cols() == target_vec.cols();
 */
EAGLEEYE_API float mi(const Matrix<float>& data_vec,const Matrix<float>& target_vec);

/**
 *	@brief cmi returns the log base 2 mutual information between data_vec and target_vec, conditioned on
 *	condition_vec, I(X;Y|Z)
 *	@note data_vec.cols() == target_vec.cols() == condition_vec.cols()
 */
EAGLEEYE_API float cmi(const Matrix<float>& data_vec,const Matrix<float>& target_vec,const Matrix<float>& condition_vec);

}

#endif
