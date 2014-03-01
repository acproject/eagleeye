#ifndef _LBFGSWRAP_H_
#define _LBFGSWRAP_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "Learning/lbfgs/lbfgs.h"

namespace eagleeye
{
class EAGLEEYE_API LbfgsWrap
{
public:
	LbfgsWrap();
	virtual ~LbfgsWrap();

	/**
	 *	@brief initialize the unknown x
	 */
	void setUnkownX(Matrix<float> x);
	void setUnkownX(int num);
	void setUnkownX(const float* x,int num);

	/**
	 *	@brief get estimated x
	 */
	Matrix<float> getEstimatedX();

	/**
	 *	@brief run non-constrain optimization algorithm LBFGS
	 */
	int run();

	/**
	 *	@brief set max iterations number
	 */
	void setMaxIters(int max_iters_num=100);

	/**
	 *	@brief set line search type
	 */
	void setLineSearchType(int line_search=LBFGS_LINESEARCH_DEFAULT);

	/**
	 *	@brief set the number of corrections to approximate the inverse
	 *	hessian matrix
	 *	@detail This parameter controls the size of the limited memories
	 *	The default value is 6. Large values will result in excessive 
	 *	computing time.
	 */
	void setPreciseOfInverseHessian(int m);

protected:
	/**
	 *	@brief compute loss value and derivative with respect to variables(g)
	 *	@param x training samples (x1,x2,x3...;x1,x2,x3...;x1,x2,x3...;...)
	 *	@param g the loss value derivative with respect to variables
	 *	@param n num(samples) * num(variables)
	 *	@param step
	 */
	virtual lbfgsfloatval_t evaluate(const lbfgsfloatval_t *x,
		lbfgsfloatval_t *g,
		const int n,
		const lbfgsfloatval_t step)=0;

	/**
	 *	@brief display some intermediate results
	 *	@note If you want to display some intermediate results, you could
	 *	overload this function.
	 */
	virtual int progress(const lbfgsfloatval_t *x,
		const lbfgsfloatval_t *g,
		const lbfgsfloatval_t fx,
		const lbfgsfloatval_t xnorm,
		const lbfgsfloatval_t gnorm,
		const lbfgsfloatval_t step,
		int n,
		int k,
		int ls);
private:
	static lbfgsfloatval_t _evaluate(
		void *instance,
		const lbfgsfloatval_t *x,
		lbfgsfloatval_t *g,
		const int n,
		const lbfgsfloatval_t step
		)
	{
		return reinterpret_cast<LbfgsWrap*>(instance)->evaluate(x, g, n, step);
	}

	static int _progress(
		void *instance,
		const lbfgsfloatval_t *x,
		const lbfgsfloatval_t *g,
		const lbfgsfloatval_t fx,
		const lbfgsfloatval_t xnorm,
		const lbfgsfloatval_t gnorm,
		const lbfgsfloatval_t step,
		int n,
		int k,
		int ls
		)
	{
		return reinterpret_cast<LbfgsWrap*>(instance)->progress(x, g, fx, xnorm, gnorm, step, n, k, ls);
	}

	Matrix<float> m_unknow_x;
	lbfgsfloatval_t* m_x;
	
	lbfgs_parameter_t m_param;
};
}


#endif