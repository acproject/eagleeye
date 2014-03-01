#ifndef _PCA_H_
#define _PCA_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"

namespace eagleeye
{
class EAGLEEYE_API PCA
{
public:
	PCA(float contribution_ratio = 0.9f);
	~PCA();

	/**
	 *	@brief compute principal component
	 */
	void compute(const Matrix<float>& data);

	/**
	 *	@brief using pc to decompose data
	 */
	static Matrix<float> decompose(Matrix<float>& data,Matrix<float>& mean_vec,Matrix<float>& pc);

	/**
	 *	@brief using pc to reconstruct data
	 */
	static Matrix<float> reconstruct(Matrix<float>& coeff,Matrix<float>& mean_vec,Matrix<float>& pc);
	
	static Matrix<float> reconstructErr(const Matrix<float>& data, const Matrix<float>& reconstruct_data);

	/**
	 *	@brief get principal component
	 */
	Matrix<float> pcaComponents();

	/**
	 *	@brief get principal component coefficient
	 */
	Matrix<float> pcaComponentCoe();
	
	/**
	 *	@brief get mean vector
	 */
	Matrix<float> pcaMeanVec();

private:
	Matrix<float> m_principal_component;
	Matrix<float> m_principle_component_coe;
	Matrix<float> m_mean_vec;
	float m_contribution_val;
};
}

#endif