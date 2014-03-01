#ifndef _LOSSFUNCTION_H_
#define _LOSSFUNCTION_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include <math.h>

namespace eagleeye
{
/**
 *	@brief loss functions based on marginal distribution
 */
class LossFunction
{
public:
	LossFunction(const Matrix<int>& pairs,int n_states);
	virtual ~LossFunction(){};
	virtual void loss(const Matrix<float>& b_i,const Matrix<float>& b_ij,
		float& l,Matrix<float>& d_b_i,Matrix<float>& d_b_ij)=0;
	void setTargetStates(const Matrix<int>& target_states){m_x = target_states;};

protected:
	Matrix<int> m_x;
	const Matrix<int> m_pairs;
	const int m_n_states;
};

class LossULFunction:public LossFunction
{
public:
	LossULFunction(const Matrix<int>& pairs,int n_states)
		:LossFunction(pairs,n_states){}
	virtual ~LossULFunction(){};

	virtual void loss(const Matrix<float>& b_i,const Matrix<float>& b_ij,
		float& l,Matrix<float>& d_b_i,Matrix<float>& d_b_ij);
};

class LossCLFunction:public LossFunction
{
public:
	LossCLFunction(const Matrix<int>& pairs,int n_states)
		:LossFunction(pairs,n_states){}
	virtual ~LossCLFunction(){};

	virtual void loss(const Matrix<float>& b_i,const Matrix<float>& b_ij,
		float& l,Matrix<float>& d_b_i,Matrix<float>& d_b_ij);
};

class LossUQUADFunction:public LossFunction
{
public:
	LossUQUADFunction(const Matrix<int>& pairs,int n_states)
		:LossFunction(pairs,n_states){}
	virtual ~LossUQUADFunction(){};

	virtual void loss(const Matrix<float>& b_i,const Matrix<float>& b_ij,
		float& l,Matrix<float>& d_b_i,Matrix<float>& d_b_ij);
};

class LossCQUADFunction:public LossFunction
{
public:
	LossCQUADFunction(const Matrix<int>& pairs,int n_states)
		:LossFunction(pairs,n_states){};
	virtual ~LossCQUADFunction(){};

	virtual void loss(const Matrix<float>& b_i,const Matrix<float>& b_ij, 
		float& l,Matrix<float>& d_b_i,Matrix<float>& d_b_ij);
};

}
#endif