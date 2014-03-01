#ifndef _DENSECRF_H_
#define _DENSECRF_H_

#include "EagleeyeMacro.h"
#include "permutohedral.h"
#include "Matrix.h"
#include <vector>

namespace eagleeye
{
/**
 *	@brief Pairwise potential base class
 */
class PairwisePotential
{
public:
	PairwisePotential(){};
	virtual ~PairwisePotential(){};
	virtual void apply( float * out_values, const float * in_values,float* tmp,int value_size ) const = 0;
};

class SemiMetricFunction
{
public:
	SemiMetricFunction(){};
	virtual ~SemiMetricFunction();
	// For two probabilities apply the semi metric transform: v_i = sum_j mu_ij u_j
	virtual void apply( float * out_values, const float * in_values, int value_size ) const = 0;
};

/**
 *	@brief all nodes are linked
 *	@note reference
 *	Philipp Krahenbuhl Efficient Inference in Fully Connected CRFs with Gaussian Edge Potentials
 */
class EAGLEEYE_API DenseCRF
{
public:
	/**
	 *	@brief create a dense CRF model of size node_size with labels_num labels
	 */
	DenseCRF(int node_num,int labels_num);
	virtual ~DenseCRF();

	/**
	 *	@brief Add a pair wise potential defined over some feature space\n
	 *	The potential will have the form: w*exp(-0.5*|f_i-f_j|^2)
	 *	The kernel shape should be captured by transforming the
	 *	features before passing them into this function
	 */
	void addPairwiseEnergy(const float* features,int dimension,float w=1.0f,const SemiMetricFunction* fun=NULL);
	void addPairwiseEnergy(PairwisePotential* potential);
	
	/**
	 *	@brief add a pair wise potential
	 *	@note every row represents one feature vector
	 */
	void addPairwiseEnergy(const Matrix<float>& features,float w=1.0f,const SemiMetricFunction* fun=NULL);

	/**
	 *	@brief set the unary potential for all variables and labels
	 *	(memory order is [x0l0 x0l1 x0l2 .. x1l0 x1l1 ...])
	 */
	void setUnaryEnergy(const float* unary);
	
	/**
	 *	@brief set the unary potential for all variables and labels
	 *	@note every row represents one node
	 */
	void setUnaryEnergy(const Matrix<float>& unary);

	/**
	 *	@brief set the unary potential for a specific variable
	 */
	void setUnaryEnergy(int node_index,const float* unary);

	/**
	 *	@brief run inference and return the probabilities
	 */
	float* runInference(int n_iterations,float relax); 

	/**
	 *	@brief infer probability
	 */
	float infer(const Matrix<int>& node_labels);
	
	/**
	 *	@brief finding node labels with maximum joint probability
	 */
	float maxInfer(Matrix<int>& node_labels);

	/**
	 *	@brief run MAP inference and return the map for each node
	 */
	void map(int n_iterations,short int* result,float relax=1.0f);
	
	/**
	 *	@brief run MAP inference and return the map for each node
	 *	@note write result to matrix
	 */
	void map(int n_iterations,Matrix<short int>& result,float relax=1.0f);

protected:
	/**
	 *	@brief auxiliary functions
	 *	@note see paper
	 */
	void expAndNormalize(float* out,const float* in,float scale=1.0f,float relax=1.0f);
	
	/**
	 *	@brief step by step inference
	 */
	void startInference();
	void stepInference(float relax=1.0f);

	int m_nodes_num;			/**< the nodes number*/
	int m_labels_num;			/**< the labels number*/
	float * m_unary;			/**< unary potential energy*/
	float* m_additional_unary;
	float* m_current;			/**< estimated marginal probability*/
	float* m_next;				/**< potential energy*/
	float* m_tmp;				/**< tmp memory*/
	
	std::vector<PairwisePotential*> m_pairwise;

private:
	DenseCRF(const DenseCRF&);
	void operator=(const DenseCRF&);
};

//////////////////////////////////////////////////////////////////////////
/**
 *	@brief construct CRF on image
 */
class EAGLEEYE_API DenseCRF2D:public DenseCRF
{
public:
	/**
	 *	@brief create a 2d dense CRF model of sie rows*cols with labels_num labels
	 */
	DenseCRF2D(int rows,int cols,int labels_num);
	~DenseCRF2D(){};

	/**
	 *	@brief add a Gaussian pairwise potential with standard deviation sx and sy
	 */
	void addDistancePairwise(float sx,float sy,float w,
							const SemiMetricFunction* fun=NULL);

	/**
	 *	@brief add a Bilateral pairwise potential with spacial standard deviations sx,
	 *	sy and color standard deviations sr,sg,sb
	 */
	void addBilateralPairwise(float sx,float sy,float sr,float sg,float sb,
							const Matrix<ERGB>& im,float w,
							const SemiMetricFunction* fun=NULL );
	
	/**
	 *	@brief add a gray pairwise potential
	 */
	void addGrayPairwise(float gray_dev,
							const Matrix<float>& im,float w,
							const SemiMetricFunction* fun=NULL);

	/**
	 *	@brief add a pixel with 3 channel pairwise potential
	 */
	void addColorPairwise(float s_one,float s_two,float s_three,
							const Matrix<Array<float,3>>& im,float w,
							const SemiMetricFunction* fun=NULL);
	void addRGBPairwise(float sr,float sg,float sb,
							const Matrix<ERGB>& im,float w,
							const SemiMetricFunction* fun=NULL);

	/**
	 *	@brief get image labels
	 */
	void immap(Matrix<short int>& result);
	
private:
	int m_rows;
	int m_cols;
};
}
#endif