#include "DenseCRF.h"

#include "fastmath.h"
#include "permutohedral.h"
#include "util.h"

namespace eagleeye
{
/**
 *	@brief construct pairwise potential based on Potts model
 *	@note attention please. the code isn't consistent with
 *	what paper says.
 */
class PottsPotential:public PairwisePotential
{
public:
	PottsPotential(const float* features,int d,int n,float w,bool per_pixel_normalization = true)
		:m_n(n),m_w(w)
	{
		m_lattice.init(features,d,n);
		m_norm = allocate(n);
		for (int i = 0; i < n; ++i)
		{
			m_norm[i] = 1;
		}

		//compute the normalization factor
		m_lattice.compute(m_norm,m_norm,1);
		if (per_pixel_normalization)
		{
			//use a per pixel normalization
			for ( int i = 0; i < n; ++i)
			{
				m_norm[i] = 1.0f / (m_norm[i] + eagleeye_eps);
			}
		}
		else
		{
			float mean_norm = 0.0f;
			for (int i = 0; i < n; ++i)
			{
				mean_norm += m_norm[i];
			}
			mean_norm = n / mean_norm;

			//use a per pixel normalization
			for (int i = 0; i < n; ++i)
			{
				m_norm[i] = mean_norm;
			}
		}
	}
	virtual ~PottsPotential()
	{
		deallocate(m_norm);
	}

	void apply(float* out_values,const float* in_values,float* tmp,int value_size) const
	{
		m_lattice.compute(tmp,in_values,value_size);
		for (int i = 0, k = 0; i < m_n; ++i)
		{
			for (int j = 0; j < value_size; ++j, ++k)
			{
				out_values[k] -= (m_w * m_norm[i] * tmp[k] - m_w * m_norm[i] * in_values[k]);
			}
		}
	}

protected:
	Permutohedral m_lattice;
	int m_n;
	float m_w;
	float* m_norm;
};

class SemiMetricPotential:public PottsPotential
{
public:
	SemiMetricPotential(const float* features,int d,int n,float w,const SemiMetricFunction* fun,bool per_pixel_normaliation=true)
		:PottsPotential(features,d,n,w,per_pixel_normaliation),m_fun(fun){}

	void apply(float* out_values,const float* in_values,float* tmp,int value_size) const
	{
		m_lattice.compute(tmp,in_values,value_size);

		//to the metric transform
		float* tmp2 = new float[value_size];
		for (int i = 0; i < m_n; ++i)
		{
			float* out = out_values + i * value_size;
			float* t1 = tmp + i * value_size;
			m_fun->apply(tmp2,t1,value_size);
			for (int j = 0; j < value_size; ++j)
			{
				out[j] -= m_w * m_norm[i] * tmp2[j];
			}
		}
		delete []tmp2;
	}

protected:
	const SemiMetricFunction* m_fun;
};

//////////////////////////////////////////////////////////////////////////
DenseCRF::DenseCRF(int node_num,int labels_num)
	:m_nodes_num(node_num),m_labels_num(labels_num)
{
	m_unary = allocate(m_nodes_num * m_labels_num);
	m_additional_unary = allocate(m_nodes_num * m_labels_num);
	m_current = allocate(m_nodes_num * m_labels_num);
	m_next = allocate(m_nodes_num * m_labels_num);
	m_tmp = allocate(m_nodes_num * m_labels_num);

	//set the additional_unary to zero
	memset(m_additional_unary,0,sizeof(float) * m_nodes_num * m_labels_num);
}
DenseCRF::~DenseCRF()
{
	deallocate(m_unary);
	deallocate(m_additional_unary);
	deallocate(m_current);
	deallocate(m_next);
	deallocate(m_tmp);
	for (unsigned int i = 0; i < m_pairwise.size(); ++i)
	{
		delete m_pairwise[i];
	}
}

//pairwise potentials
void DenseCRF::addPairwiseEnergy(const float* features,int dimension,float w/* =1.0f */,const SemiMetricFunction* fun/* =NULL */)
{
	if (fun)
	{
		addPairwiseEnergy(new SemiMetricPotential(features,dimension,m_nodes_num,w,fun));
	}
	else
	{
		addPairwiseEnergy(new PottsPotential(features,dimension,m_nodes_num,w));
	}
}

void DenseCRF::addPairwiseEnergy(const Matrix<float>& features,float w,const SemiMetricFunction* fun)
{
	assert(features.rows() == m_nodes_num);
	addPairwiseEnergy(features.dataptr(),features.cols(),w,fun);
}

void DenseCRF::addPairwiseEnergy(PairwisePotential* potential)
{
	m_pairwise.push_back(potential);
}

//unary potentials
void DenseCRF::setUnaryEnergy(const float* unary)
{
	memcpy(m_unary,unary,m_nodes_num * m_labels_num * sizeof(float));
}

void DenseCRF::setUnaryEnergy(const Matrix<float>& unary)
{
	assert(m_nodes_num == unary.rows());
	assert(m_labels_num == unary.cols());

	for (int i = 0; i < m_nodes_num; ++i)
	{
		memcpy(m_unary + i * m_labels_num,unary.row(i),sizeof(float) * m_labels_num);
	}
}

void DenseCRF::setUnaryEnergy(int node_index,const float* unary)
{
	memcpy(m_unary + node_index * m_labels_num,unary,m_labels_num * sizeof(float));
}

//inference
float* DenseCRF::runInference(int n_iterations,float relax)
{
	startInference();
	for (int it = 0; it < n_iterations; ++it)
	{
		stepInference(relax);
	}

	return m_current;
}

void DenseCRF::expAndNormalize(float* out,const float* in,float scale/* =1.0f */,float relax/* =1.0f */)
{
	float* v = new float[m_labels_num];
	for (int i = 0; i < m_nodes_num; ++i)
	{
		const float* b = in + i * m_labels_num;
		//find the max and subtract it so that the exp doesn't explode
		float mx = scale * b[0];
		for (int j = 1; j < m_labels_num; ++j)
		{
			if (mx < scale * b[j])
			{
				mx = scale * b[j];
			}
		}
		float tt = 0.0f;
		for (int j = 0; j < m_labels_num; ++j)
		{
			v[j] = fast_exp(scale * b[j] - mx);
			tt += v[j];
		}

		//make it a probability
		for (int j = 0; j < m_labels_num; ++j)
		{
			v[j] /= tt;
		}

		float* a = out + i * m_labels_num;
		for (int j = 0; j < m_labels_num; ++j)
		{
			if (relax == 1.0f)
			{
				a[j] = v[j];
			}
			else
			{
				a[j] = (1 - relax) * a[j] + relax * v[j];
			}
		}
	}
	
	delete []v;
}

void DenseCRF::startInference()
{
	//initialize using the unary energies
	expAndNormalize(m_current,m_unary,1.0f);
}

void DenseCRF::stepInference(float relax/* =1.0 */)
{
#ifdef SSE_DENSE_CRF
	__m128* sse_next = (__m128*)m_next;
	__m128* sse_unary = (__m128*)m_unary;
	__m128* sse_additional_unary = (__m128*)m_additional_unary;
#endif

	//set the unary potential
#ifdef SSE_DENSE_CRF
	for( int i = 0; i < (m_nodes_num * m_labels_num - 1) / 4 + 1; i++ )
		sse_next[i] = - sse_unary[i] - sse_additional_unary[i];
#else
	for( int i = 0; i < m_nodes_num * m_labels_num; i++ )
		m_next[i] = -m_unary[i] - m_additional_unary[i];
#endif

	//add up all pairwise potentials
	for (unsigned int i = 0; i < m_pairwise.size(); ++i)
	{
		m_pairwise[i]->apply(m_next,m_current,m_tmp,m_labels_num);
	}

	//exponentiate and normalize
	expAndNormalize(m_current,m_next,1.0,relax);
}

void DenseCRF::map(int n_iterations,short int* result,float relax)
{
	//run inference
	float* prob = runInference(n_iterations,relax);

	//find the map
	for (int i = 0; i < m_nodes_num; ++i)
	{
		const float* p = prob + i * m_labels_num;
		//find the max and subtract it so that the exp doesn't explode
		float mx = p[0];
		int imx = 0;
		for (int j = 0; j < m_labels_num; ++j)
		{
			if (mx < p[j])
			{
				mx = p[j];
				imx = j;
			}
		}
		result[i] = imx;
	}
}
void DenseCRF::map(int n_iterations,Matrix<short int>& result,float relax)
{
	result = Matrix<short int>(m_nodes_num,1);
	short int* result_data = result.dataptr();
	map(n_iterations,result_data,relax);
}

float DenseCRF::infer(const Matrix<int>& node_labels)
{
	//run inference
/*	float* prob = runInference(n_iterations,relax);*/
	
	return 0;
}

float DenseCRF::maxInfer(Matrix<int>& node_labels)
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
DenseCRF2D::DenseCRF2D(int rows,int cols,int labels_num)
	:DenseCRF(rows * cols,labels_num)
{
	m_rows = rows;
	m_cols = cols;
}
void DenseCRF2D::addDistancePairwise(float sx,float sy,float w,const SemiMetricFunction* fun/* =NULL */)
{
	float* feature = new float[m_nodes_num * 2];
	for (int i = 0; i < m_rows; ++i)
	{
		for (int j = 0; j < m_cols; ++j)
		{
			feature[(i * m_cols + j) * 2 + 0] = float(i) / sx;
			feature[(i * m_cols + j) * 2 + 1] = float(j) / sy;
		}
	}

	addPairwiseEnergy(feature,2,w,fun);
	
	delete []feature;
}

void DenseCRF2D::addBilateralPairwise(float sx,float sy,float sr,float sg,float sb,const Matrix<ERGB>& im,float w,const SemiMetricFunction* fun/* =NULL */)
{
	float* feature = new float[m_nodes_num * 5];
	for (int i = 0; i < m_rows; ++i)
	{
		const ERGB* im_data = im.row(i);
		for (int j = 0; j < m_cols; ++j)
		{
			feature[(i * m_cols + j) * 5 + 0] = float(i) / sx;
			feature[(i * m_cols + j) * 5 + 1] = float(j) / sy;
			feature[(i * m_cols + j) * 5 + 2] = float(im_data[j][0]) / sr;
			feature[(i * m_cols + j) * 5 + 3] = float(im_data[j][1]) / sg;
			feature[(i * m_cols + j) * 5 + 4] = float(im_data[j][2]) / sb;
		}
	}

	addPairwiseEnergy(feature,5,w,fun);

	delete []feature;
}

void DenseCRF2D::addGrayPairwise(float gray_dev,const Matrix<float>& im,float w,const SemiMetricFunction* fun/* =NULL */)
{
	float* feature = new float[m_nodes_num];
	for (int i = 0; i < m_rows; ++i)
	{
		const float* im_data = im.row(i);
		for (int j = 0; j < m_cols; ++j)
		{
			feature[i * m_cols + j] = im_data[j] / gray_dev;
		}
	}

	addPairwiseEnergy(feature,1,w,fun);

	delete []feature;
}

void DenseCRF2D::addColorPairwise(float s_one,float s_two,float s_three,const Matrix<Array<float,3>>& im,float w,const SemiMetricFunction* fun/* =NULL */)
{
	float* feature = new float[m_nodes_num * 3];
	for (int i = 0; i < m_rows; ++i)
	{
		const Array<float,3>* im_data = im.row(i);
		for (int j = 0; j < m_cols; ++j)
		{
			feature[(i * m_cols + j) * 3 + 0] = im_data[j][0] / s_one;
			feature[(i * m_cols + j) * 3 + 1] = im_data[j][1] / s_two;
			feature[(i * m_cols + j) * 3 + 2] = im_data[j][2] / s_three;
		}
	}

	addPairwiseEnergy(feature,3,w,fun);

	delete []feature;
}
void DenseCRF2D::addRGBPairwise(float sr,float sg,float sb, const Matrix<ERGB>& im,float w, const SemiMetricFunction* fun/* =NULL */)
{
	float* feature = new float[m_nodes_num * 3];
	for (int i = 0; i < m_rows; ++i)
	{
		const ERGB* im_data = im.row(i);
		for (int j = 0; j < m_cols; ++j)
		{
			feature[(i * m_cols + j) * 3 + 0] = float(im_data[j][0]) / sr;
			feature[(i * m_cols + j) * 3 + 1] = float(im_data[j][1]) / sg;
			feature[(i * m_cols + j) * 3 + 2] = float(im_data[j][2]) / sb;
		}
	}

	addPairwiseEnergy(feature,3,w,fun);

	delete []feature;
}

void DenseCRF2D::immap(Matrix<short int>& result)
{
	map(30,result,1.0f);
	result = result.reshape(m_rows,m_cols);
}
}