#include "LbfgsWrap.h"
#include "Print.h"

namespace eagleeye
{
LbfgsWrap::LbfgsWrap()
{
	m_x = NULL;
	lbfgs_parameter_t param = {
		10, 1e-5, 0, 1e-5,
		100, LBFGS_LINESEARCH_BACKTRACKING_WOLFE, 40,
		1e-20, 1e20, 1e-4, 0.9, 0.9, 1.0e-16,
		0.0, 0, -1};

	m_param = param;
}
LbfgsWrap::~LbfgsWrap()
{
	if (m_x != NULL)
	{
		lbfgs_free(m_x);
		m_x = NULL;
	}
}

int LbfgsWrap::progress(const lbfgsfloatval_t *x, 
	const lbfgsfloatval_t *g, 
	const lbfgsfloatval_t fx, 
	const lbfgsfloatval_t xnorm, 
	const lbfgsfloatval_t gnorm, 
	const lbfgsfloatval_t step, 
	int n, int k, int ls)
{
	EAGLEEYE_INFO("Iteration %d: \n",k);
	EAGLEEYE_INFO("  xnorm = %f, gnorm = %f, step = %f\n", xnorm, gnorm, step);
	EAGLEEYE_INFO("  objective function value = %f\n\n", fx);
	return 0;
}

void LbfgsWrap::setUnkownX(Matrix<float> x)
{
	m_unknow_x = x;
}
void LbfgsWrap::setUnkownX(int num)
{
	m_unknow_x = Matrix<float>(num,1,float(0));
}

void LbfgsWrap::setUnkownX(const float* x,int num)
{
	m_unknow_x = Matrix<float>(num,1);
	for (int i = 0; i < num; ++i)
	{
		m_unknow_x(i) = x[i];
	}
}

Matrix<float> LbfgsWrap::getEstimatedX()
{
	return m_unknow_x;
}

int LbfgsWrap::run()
{
	lbfgsfloatval_t fx;
	if (m_x)
	{
		lbfgs_free(m_x);
	}
	m_x = lbfgs_malloc(m_unknow_x.rows() * m_unknow_x.cols());
	
	if (!m_x)
	{
		EAGLEEYE_ERROR("failed to allocate a memory block for variables. \n");
		return 1;
	}

	int rows = m_unknow_x.rows();
	int cols = m_unknow_x.cols();
	for (int i = 0; i < rows; ++i)
	{
		float* tr_data = m_unknow_x.row(i);
		for (int j = 0; j < cols; ++j)
		{
			m_x[i * cols + j] = tr_data[j];
		}
	}

	/*
        Start the L-BFGS optimization; this will invoke the callback functions
        evaluate() and progress() when necessary.
     */
	int ret = lbfgs(rows * cols,m_x,&fx,_evaluate,_progress,this,&m_param);

	for (int i = 0; i < rows; ++i)
	{
		float* tr_data = m_unknow_x.row(i);
		for (int j = 0; j < cols; ++j)
		{
			tr_data[j] = float(m_x[i * cols + j]);
		}
	}

	//report the result
	EAGLEEYE_INFO("L-BFGS optimization terminated with status code = %d\n", ret);
	EAGLEEYE_INFO("loss = %f",fx);

	return ret;
}

void LbfgsWrap::setMaxIters(int max_iters_num/* =100 */)
{
	m_param.max_iterations = max_iters_num;
}
void LbfgsWrap::setLineSearchType(int line_search/* =LBFGS_LINESEARCH_DEFAULT */)
{
	m_param.linesearch = line_search;
}
void LbfgsWrap::setPreciseOfInverseHessian(int m)
{
	m_param.m = m;
}
}