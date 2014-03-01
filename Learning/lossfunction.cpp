#include "lossfunction.h"
#include "Print.h"

namespace eagleeye
{
LossFunction::LossFunction(const Matrix<int>& pairs,int n_states)
						:m_pairs(pairs),m_n_states(n_states)
{}

//////////////////////////////////////////////////////////////////////////
void LossULFunction::loss(const Matrix<float>& b_i,const Matrix<float>& b_ij, 
	float& l,Matrix<float>& d_b_i,Matrix<float>& d_b_ij)
{
	d_b_i.setzeros();
	d_b_ij.setzeros();
	l = 0;

	int node_num = m_x.cols();
	const int* x_data = m_x.dataptr();
	for (int i = 0; i < node_num; ++i)
	{
		if (x_data[i] > -1)
		{
			l = l - log(b_i(x_data[i],i));
			d_b_ij(x_data[i],i) = -1.0f / b_i(x_data[i],i);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void LossCLFunction::loss(const Matrix<float>& b_i,const Matrix<float>& b_ij, 
	float& l,Matrix<float>& d_b_i,Matrix<float>& d_b_ij)
{
	l = 0.0f;
	d_b_i.setzeros();
	d_b_ij.setzeros();
	
	const int* x_data = m_x.row(0);
	int pairs_num = m_pairs.rows();
	for (int n = 0; n < pairs_num; ++n)
	{
		int left = m_pairs(n,0);
		int right = m_pairs(n,1);
		if(x_data[left] > -1 && x_data[right] > -1)
		{
			int who = x_data[left] + x_data[right] * m_n_states;

			if (b_ij(who,n) > 0)
			{
				l = l - log(b_ij(who,n));
			}

			if (b_ij(who,n) != 0)
			{
				d_b_ij(who,n) = -1.0f / b_ij(who,n);
			}
			else
			{
				d_b_ij(who,n) = 0;
			}
			
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void LossUQUADFunction::loss(const Matrix<float>& b_i,const Matrix<float>& b_ij, 
	float& l,Matrix<float>& d_b_i,Matrix<float>& d_b_ij)
{
	l=0;
	d_b_i.setzeros();
	d_b_ij.setzeros();

	int node_num=m_x.cols();
	const int* x_data=m_x.dataptr();
	for (int i=0;i<node_num;++i)
	{
		if (x_data[i]>-1)
		{
			float t=b_i(x_data[i],i);
			l=l+(-2*t+t*t);
			d_b_i(x_data[i],i)=-2+2*t;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void LossCQUADFunction::loss(const Matrix<float>& b_i,const Matrix<float>& b_ij, 
	float& l,Matrix<float>& d_b_i,Matrix<float>& d_b_ij)
{
	l=0;
	d_b_i.setzeros();
	d_b_ij.setzeros();

	const int* x_data=m_x.dataptr();
	int pairs_num=m_pairs.rows();
	for (int n=0;n<pairs_num;++n)
	{
		int left=m_pairs(n,0);
		int right=m_pairs(n,1);
		if(x_data[left]>-1&&x_data[right]>-1)
		{
			int who=x_data[left]+x_data[right]*m_n_states;
			
			float t=b_ij(who,n);
			l=l+(-2*t+t*t);
			d_b_ij(who,n)=-2+2*t;
		}
	}
}
}