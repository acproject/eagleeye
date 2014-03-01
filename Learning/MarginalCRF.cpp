#include "MarginalCRF.h"
#include "Learning/lbfgs/LbfgsWrap.h"
#include "EagleeyeIO.h"
#include "Print.h"
#include "MatrixMath.h"
#include "Variable.h"
#include <math.h>
#include <cmath>
#include "Matlab/MatlabInterface.h"
namespace eagleeye
{
class CRFLFGS:public LbfgsWrap
{
public:
	CRFLFGS(MarginalCRF* crf){m_crf=crf;};
protected:
	virtual lbfgsfloatval_t evaluate(const lbfgsfloatval_t *x, 
		lbfgsfloatval_t *g, 
		const int n, 
		const lbfgsfloatval_t step)
	{
		//get the loss function derivative with respect to theta_i and theta_ij
		Matrix<float> unary_coe = m_crf->m_unary_coe;
		int num_unary_coe = unary_coe.rows() * unary_coe.cols();

		Matrix<float> clique_coe = m_crf->m_clique_coe;
		int num_clique_coe = clique_coe.rows() * clique_coe.cols();

		for (int i = 0; i < num_unary_coe; ++i)
		{
			unary_coe(i) = float(x[i]);
		}
		for (int i = 0; i < num_clique_coe; ++i)
		{
			clique_coe(i) = float(x[i + num_unary_coe]);
		}

		//evaluate on all training samples
		int training_sample_num = m_crf->m_unary_samples.size();
		std::vector<Matrix<float>> d_unary_weight(training_sample_num);
		std::vector<Matrix<float>> d_clique_weight(training_sample_num);
		std::vector<float> loss(training_sample_num,0.0f);
		for (int index = 0; index < training_sample_num; ++index)
		{
			m_crf->setUsingData(m_crf->m_unary_samples[index],m_crf->m_clique_samples[index],m_crf->m_states_samples[index]);
			
// 			putToMatlab(m_crf->m_unary_samples[index],"now_unary_s");
// 			putToMatlab(m_crf->m_clique_samples[index],"now_clique_s");

			m_crf->m_theta_i = unary_coe * m_crf->m_unary_pot;
			m_crf->m_theta_ij = clique_coe * m_crf->m_clique_pot;

// 			putToMatlab(m_crf->m_theta_i,"theta_i");
// 			putToMatlab(m_crf->m_theta_ij,"theta_ij");

			//using back propagation to get
			//the loss function derivative with respect to theta_i and theta_ij
			loss[index] = m_crf->backprop();
			
			if (_finite(m_crf->m_dtheta_i(0)) && _finite(m_crf->m_dtheta_ij(0)))
			{
				//transform to the loss function derivative with respect to 
				//implicit variable
				d_unary_weight[index] = m_crf->m_dtheta_i * m_crf->m_unary_pot_t;
				d_clique_weight[index] = m_crf->m_dtheta_ij * m_crf->m_clique_pot_t;
			}
			else
			{
				d_unary_weight[index] = Matrix<float>(m_crf->m_dtheta_i.rows(),m_crf->m_unary_pot_t.cols(),float(0.0f));
				d_clique_weight[index] = Matrix<float>(m_crf->m_dtheta_ij.rows(),m_crf->m_clique_pot_t.cols(),float(0.0f));
			}

		}

		for (int i = 0; i < num_unary_coe; ++i)
		{
			float d_val = 0;
			for (int index = 0; index < training_sample_num; ++index)
			{
				d_val += d_unary_weight[index](i);
			}
			g[i] = d_val / float(training_sample_num);
		}
		for (int i = 0; i < num_clique_coe; ++i)
		{
			float d_val = 0;
			for (int index = 0; index < training_sample_num; ++index)
			{
				d_val += d_clique_weight[index](i);
			}
			g[i + num_unary_coe] = d_val / float(training_sample_num);
		}
		
		float loss_val = 0.0f;
		for (int index = 0; index < training_sample_num; ++index)
		{
			loss_val += loss[index];
		}

		return loss_val / float(training_sample_num);
	}

private:
	MarginalCRF* m_crf;
};

MarginalCRF::MarginalCRF(InferenceType infer_type,LossFunType loss_fun_type)
{
	m_maxiter = 5;
	m_convthreshold = 0.0f;
	m_loss_fun = NULL;
	m_loss_fun_type = loss_fun_type;
	m_infer_type = infer_type;
	m_loss = 0.0f;
	m_back_propagation_count = 0;
	m_trw_rho = 0.5f;

	m_crf_lfgs = new CRFLFGS(this);
}
MarginalCRF::~MarginalCRF()
{
	if (m_loss_fun)
	{
		delete m_loss_fun;
	}

	if (m_crf_lfgs)
	{
		delete m_crf_lfgs;
	}
}

float MarginalCRF::backpropMeanfield()
{
	//get predicated marginal distribution including every node marginal distribution and 
	// every clique marginal distribution
	meanfieldInfer();

	//clear the old value
	m_dtheta_i.setzeros();
	m_dtheta_ij.setzeros();

	Matrix<float> b_i0(m_n_states,1,float(0));
	Matrix<float> backnorm(m_n_states,1,float(0));//see paper def

	//using loss function to get the marginal distribution gradient of every node and clique
	//m_loss	:the loss value
	//m_db_i	:the loss function derivative with respect to every node marginal distribution
	//m_db_ij	:the loss function derivative with respect to every clique marginal distribution
	m_db_i.setzeros();
	m_db_ij.setzeros();
	m_loss_fun->loss(m_b_i,m_b_ij,m_loss,m_db_i,m_db_ij);

	//sometimes, we only compute the loss function derivative with respect to every clique
	//we also want to get the derivative with respect to every node
	for (int clique_index = 0; clique_index < m_n_cliques; ++clique_index)
	{
		for (int yi = 0; yi < m_n_states; ++yi)
		{
			for (int yj = 0; yj < m_n_states; ++yj)
			{
				//left node
				int i = m_pairs(clique_index,0);
				//right node
				int j = m_pairs(clique_index,1);

				//	dL/db_i=dL/db_ij*b_j
				int index = yi + yj * m_n_states;
				m_db_i(yi,i) += m_db_ij(index,clique_index) * m_b_ij(index,clique_index) / m_b_i(yi,i);
				m_db_i(yj,j) += m_db_ij(index,clique_index) * m_b_ij(index,clique_index) / m_b_i(yj,j);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//backpropagation to get derivative with respect to theta
	for (int reps = 0; reps < m_maxiter; ++reps)
	{
		for (int j0 = m_n_nodes * 2 - 2; j0 >= 0; --j0)
		{
			int j = 0;
			if (j0 < m_n_nodes)
			{
				j = j0;
			}
			else
			{
				j = m_n_nodes - 2 - (j0 - m_n_nodes);
			}

			//j is node index
			//yj is state index
			for (int yj = 0; yj < m_n_states; ++yj)
			{
				b_i0(yj) = m_psi_i(yj,j);

				//j is considered as left node of an edge
				for (unsigned int k = 0; k < m_n1.cols(); ++k)
				{
					//d is clique index
					//every clique is a edge
					int d = m_n1(j,k);
					if (d == -1) continue;

					//get the right point of this edge
					int i = m_pairs(d,1);
					for (int yi = 0; yi < m_n_states; ++yi)
					{
						int index = yj + yi * m_n_states;
						b_i0(yj) = b_i0(yj) * pow(m_psi_ij(index,d),m_b_i(yi,i));
					}
				}

				//j is considered as right node of an edge
				for (int k = 0; k < int(m_n2.cols()); ++k)
				{
					//d is clique index
					//every clique is a edge
					int d = m_n2(j,k);
					if (d == -1) continue;

					//get the left point of this edge
					int i = m_pairs(d,0);
					for (int yi = 0; yi < m_n_states; ++yi)
					{
						int index = yi + yj * m_n_states;
						b_i0(yj) = b_i0(yj) * pow(m_psi_ij(index,d),m_b_i(yi,i));
					}						
				}
			}

			//normalize marginal probability of every node
			float sum_s = 0.0f;
			for (int yj = 0; yj < m_n_states; ++yj)
			{
				sum_s += b_i0(yj);
			}

			//compute g*c in backnorm(g,c)
			float inner = 0.0f;
			for (int yj = 0; yj < m_n_states; ++yj)
			{
				inner += m_db_i(yj,j) * (b_i0(yj)/sum_s);
			}

			//compute c.dot(g-g*c) in backnorm(g,c)
			for(int yj = 0; yj < m_n_states; ++yj)
			{
				backnorm(yj) = (m_db_i(yj,j) - inner) * (b_i0(yj)/sum_s);
			}

			for (int yj = 0; yj < m_n_states; ++yj)
			{
				for (int k = 0; k < int(m_n1.cols()); ++k)
				{
					int d = m_n1(j,k);
					if (d == -1) continue;
					
					//get right node of edge
					int i = m_pairs(d,1);
					for (int yi = 0;yi < m_n_states; ++yi)
					{
						int index = yj + yi * m_n_states;

						m_db_i(yi,i) += backnorm(yj) * m_theta_ij(index,d);
						m_dtheta_ij(index,d) += backnorm(yj) * m_b_i(yi,i);
					}
				}

				for (int k = 0; k < int(m_n2.cols()); ++k)
				{
					int d = m_n2(j,k);
					if (d == -1) continue;
					
					//get left node of edge
					int i = m_pairs(d,0);
					for (int yi = 0; yi < m_n_states; ++yi)
					{
						int index = yi + yj * m_n_states;

						m_db_i(yi,i) += backnorm(yj) * m_theta_ij(index,d);
						m_dtheta_ij(index,d) += backnorm(yj) * m_b_i(yi,i);
					}
				}
			}

			for (int yj = 0; yj < m_n_states; ++yj)
			{
				m_dtheta_i(yj,j) += backnorm(yj);
			}

			for (int yj = m_n_states - 1; yj >= 0; --yj)
			{
				m_back_propagation_count--;
				if (m_back_propagation_count < 0)
				{
					return m_loss;
				}

				//////////////////////////////////////////////////////////////////////////
				m_b_i(yj,j) = m_back_propagation_marginal(m_back_propagation_count);
				//////////////////////////////////////////////////////////////////////////
			}

			for (int yj = 0; yj < m_n_states; ++yj)
			{
				m_db_i(yj,j) = 0;
			}
		}
	}		
	return m_loss;
}

void MarginalCRF::meanfieldInfer()
{
	//clear back propagation marginal
	m_back_propagation_marginal.setzeros();
	m_back_propagation_count = 0;

	//transform to exp(..)
	int theta_i_rows = m_theta_i.rows();
	int theta_i_cols = m_theta_i.cols();
	for (int i = 0; i < theta_i_rows; ++i)
	{
		float* m_psi_i_data = m_psi_i.row(i);
		float* theta_i_data = m_theta_i.row(i);
		for (int j = 0; j < theta_i_cols; ++j)
		{
			m_psi_i_data[j] = exp(theta_i_data[j]);
		}
	}

	int theta_ij_rows = m_theta_ij.rows();
	int theta_ij_cols = m_theta_ij.cols();
	for (int i = 0; i < theta_ij_rows; ++i)
	{
		float* m_psi_ij_data = m_psi_ij.row(i);
		float* theta_ij_data = m_theta_ij.row(i);
		for (int j = 0; j < theta_ij_cols; ++j)
		{
			m_psi_ij_data[j] = exp(theta_ij_data[j]);
		}
	}

	putToMatlab(m_theta_i,"now_theta_i");
	putToMatlab(m_theta_ij,"now_theta_ij");

	//m_b_i and m_b_ij are marginal distribution
	//now we initialize m_b_i and m_b_ij
	int total = m_b_i.rows() * m_b_i.cols();
	float* b_i_data = m_b_i.dataptr();
	for (int i = 0; i < total; ++i)
	{
		b_i_data[i] = 1.0f;
	}
	total = m_b_ij.rows() * m_b_ij.cols();
	float* b_ij_data = m_b_ij.dataptr();
	for (int i = 0; i < total; ++i)
	{
		b_ij_data[i] = 1.0f;
	}

	//////////////////////////////////////////////////////////////////////////
	Matrix<double> b_i0(m_n_states,1);

	Matrix<float> b_i_save(m_b_i.rows(),m_b_i.cols());
	b_i_save.copy(m_b_i);
	float conv = 0;

	for ( int reps = 0; reps < m_maxiter; ++reps)
	{
		for (int j0 = 0; j0 < m_n_nodes * 2 - 1; ++j0)
		{
			int j;//the current node
			if (j0 < m_n_nodes)
			{
				j = j0;
			}
			else
			{
				j = m_n_nodes - 2 - (j0 - m_n_nodes);
			}

			//j is node index
			//yj is state index
			for (int yj = 0; yj < m_n_states; ++yj)
			{
				b_i0(yj) = m_psi_i(yj,j);

				//j is considered as left node of an edge
				for (int k = 0; k < int(m_n1.cols()); ++k)
				{
					//d is clique index
					//every clique is a edge
					int d = m_n1(j,k);
					if (d == -1) continue;

					//get the right point of this edge
					int i = m_pairs(d,1);
					for (int yi = 0; yi < m_n_states; ++yi)
					{
						int index = yj + yi * m_n_states;
						b_i0(yj) = b_i0(yj) * pow(m_psi_ij(index,d),m_b_i(yi,i));
					}
				}

				//j is considered as right node of an edge
				for (unsigned int k = 0; k < m_n2.cols(); ++k)
				{
					//d is clique index
					//every clique is a edge
					int d = m_n2(j,k);
					if (d == -1) continue;

					//get the left point of this edge
					int i = m_pairs(d,0);
					for (int yi = 0; yi < m_n_states; ++yi)
					{
						int index = yi + yj * m_n_states;
						b_i0(yj) = b_i0(yj) * pow(m_psi_ij(index,d),m_b_i(yi,i));
					}
				}
			}

			//normalize marginal probability of every node
			double sum_s = 0.0;
			for (int yj = 0; yj < m_n_states; ++yj)
			{
				sum_s += b_i0(yj);
			}
			for (int yj = 0; yj < m_n_states; ++yj)
			{
				m_b_i(yj,j) = float(b_i0(yj) / sum_s);
			}

			for (int yj = 0; yj < m_n_states; ++yj)
			{
				//prepare for back propagation
				//normalized marginal distribution
				m_back_propagation_marginal(m_back_propagation_count) = m_b_i(yj,j);
				m_back_propagation_count++;
			}
		}

		conv = 0;
		for (unsigned int m = 0; m < m_b_i.rows(); ++m)
		{
			for(unsigned int n = 0; n < m_b_i.cols(); ++n)
			{
				if (conv < abs(m_b_i(m,n) - b_i_save(m,n)))
				{
					conv = abs(m_b_i(m,n) - b_i_save(m,n));
				}
			}
		}

		if (conv < m_convthreshold)
		{
			break;
		}
		b_i_save.copy(m_b_i);
	}

	//////////////////////////////////////////////////////////////////////////
	//compute marginal probability of every clique	
	for (int clique_index = 0; clique_index < m_n_cliques; ++clique_index)
	{
		float tmp=0;

		//left node of every clique
		for (int yi = 0; yi < m_n_states; ++yi)
		{
			//right node of every clique
			for(int yj = 0; yj < m_n_states; ++yj)
			{
				int index = yi + yj * m_n_states;
				m_b_ij(index,clique_index) = m_b_i(yi,m_pairs(clique_index,0)) * m_b_i(yj,m_pairs(clique_index,1));

				tmp += m_b_ij(index,clique_index);
			}
		}

		//normalized
		for (int index = 0;index < m_n_states * m_n_states; ++index)
		{
			m_b_ij(index,clique_index) = m_b_ij(index,clique_index) / tmp;
		}
	}
}

//wrong find bugs
float MarginalCRF::backpropTRW()
{
	//get predicated marginal distribution including every node marginal distribution and 
	// every clique marginal distribution
	trwInfer();

	//clear the old value
	m_dtheta_i.setzeros();
	m_dtheta_ij.setzeros();

	Matrix<float> backnorm(m_n_states,1,float(0));//see paper def

	Matrix<float> s(m_n_states,m_n_states);
	Matrix<float> m0(m_n_states,1);


	//using loss function to get the marginal distribution gradient of every node and clique
	//m_loss	:the loss value
	//m_db_i	:the loss function derivative with respect to every node marginal distribution
	//m_db_ij	:the loss function derivative with respect to every clique marginal distribution
	m_loss = 0;
	m_db_i.setzeros();
	m_db_ij.setzeros();
	m_loss_fun->loss(m_b_i,m_b_ij,m_loss,m_db_i,m_db_ij);
	EAGLEEYE_INFO("loss %f \n",m_loss);

	//initialize for all derivatives, including m_dtheta_i, m_dtheta_ij, dn, m_db_i, m_db_ij
	// dm1 and dm2.
	// contribution from every node
	//formula:  normalized univiariate marginals
	
	Matrix<float> dn = m_db_i.dot(m_psi_i);
	m_dtheta_i = m_db_i.dot(m_b_i);
	m_dtheta_ij = m_db_ij.dot(m_b_ij);

	Matrix<float> dm1(m_n_states,m_n_cliques,float(1.0f));	//record info of the left node of edge
	Matrix<float> dm2(m_n_states,m_n_cliques,float(1.0f));	//record info of the right node of edge

	//contribution from every clique
	//formula: clique-wise marginals
	for (int c = 0; c < m_n_cliques; ++c)
	{
		int i = m_pairs(c,0);	//left node
		int j = m_pairs(c,1);	//right node

		for (int yi = 0; yi < m_n_states; ++yi)
		{
			for (int yj = 0; yj < m_n_states; ++yj)
			{
				int index = yi +yj * m_n_states;
				//from clique-wise marginals
				m_dtheta_i(yi,i) = m_dtheta_i(yi,i) + m_db_ij(index,c) * m_b_ij(index,c);
				m_dtheta_i(yj,j) = m_dtheta_i(yj,j) + m_db_ij(index,c) * m_b_ij(index,c);

				//from clique-wise marginals
				dn(yi,i) = dn(yi,i) + m_db_ij(index,c) * m_b_ij(index,c) / m_trw_n(yi,i);
				dn(yj,j) = dn(yj,j) + m_db_ij(index,c) * m_b_ij(index,c) / m_trw_n(yj,j);

				//from clique-wise marginals
				//first part -- computing derivatives with respect to m1 and m2
				// see clique-wise marginal formula
				dm1(yi,c) = dm1(yi,c) - m_db_ij(index,c) * m_b_ij(index,c) / m_trw_m1(yi,c);
				dm2(yj,c) = dm2(yj,c) - m_db_ij(index,c) * m_b_ij(index,c) / m_trw_m2(yj,c);
			}
		}
	}

	//second part -- computing derivatives with respect to m1 and m2
	//see clique-wise marginal formula
	for (int i = 0; i < int(m_b_i.cols()); ++i)
	{
		for (int yi = 0; yi < m_n_states; ++yi)
		{
			for (int k = 0; k < int(m_n1.cols()); ++k)
			{
				int d = m_n1(i,k);
				if (d == -1) continue;
				
				dm1(yi,d) += m_trw_rho * dn(yi,i) * m_trw_n(yi,i) / m_trw_m1(yi,d);
			}

			for (int k = 0; k< int(m_n2.cols()); ++k)
			{
				int d = m_n2(i,k);
				if (d == -1) continue;
				
				dm2(yi,d) += m_trw_rho * dn(yi,i) * m_trw_n(yi,i) / m_trw_m2(yi,d);
			}
		}
	}
	
	//convergence threshold
	//start back propagation
	for (int reps = 0; reps < m_maxiter; ++reps)
	{
		for (int c0 = 2 * m_n_cliques - 1; c0 >= 0; --c0)
		{
			int c,mode;
			if (c0 < m_n_cliques)
			{
				c = c0;
				mode = 1;
			}
			else
			{
				c = m_n_cliques - 1 - ( c0 - m_n_cliques );
				mode = 2;
			}
			int i = m_pairs(c,0);
			int j = m_pairs(c,1);

			if (mode == 1)
			{
				for (int yi = 0; yi < m_n_states; ++yi)
				{
					m_trw_n(yi,i) = 1;
				}

				for (int k = 0; k < int(m_n1.cols()); ++k)
				{
					int d = m_n1(i,k);
					if (d == -1) continue;
					
					for (int yi = 0; yi < m_n_states; ++yi)
					{
						m_trw_n(yi,i) *= m_trw_m1(yi,d);
					}						
				}
				for (int k = 0; k < int(m_n2.cols()); ++k)
				{
					int d = m_n2(i,k);
					if (d == -1) continue;
					
					for (int yi = 0; yi < m_n_states; ++yi)
					{
						m_trw_n(yi,i) *= m_trw_m2(yi,d);
					}	
				}

				if (m_trw_rho == 1)
				{
					//do nothing
				}
				else if(m_trw_rho == 0.5f)
				{
					for (int yi = 0; yi < m_n_states; ++yi)
					{
						m_trw_n(yi,i) = sqrt(m_trw_n(yi,i));
					}
				}
				else
				{
					for (int yi = 0; yi < m_n_states; ++yi)
					{
						m_trw_n(yi,i) = pow(m_trw_n(yi,i),m_trw_rho);
					}
				}

				//compute m(y_j)
				float sum_s = 0;
				for (int yj = 0; yj < m_n_states; ++yj)
				{
					m0(yj) = 0;
					for (int yi = 0; yi < m_n_states; ++yi)
					{
						int index = yi + yj * m_n_states;
						s(yi,yj) = m_rho_psi_ij(index,c) * m_psi_i(yi,i) * m_trw_n(yi,i) / m_trw_m1(yi,c);//see (18) in paper
						m0(yj)	+= s(yi,yj);
						sum_s	+= s(yi,yj);
					}
				}

				//see (18) in paper
				float inner = 0;
				for (int yj = 0; yj < m_n_states; ++yj)
				{
					inner += dm2(yj,c) * (m0(yj)/sum_s);
				}
				//compute (g-g*c) in backnorm(g,c)
				for(int yj = 0; yj < m_n_states; ++yj)
				{
					backnorm(yj) = dm2(yj,c) - inner;
				}

				for (int yj = 0;yj < m_n_states; ++yj)
				{
					for (int yi = 0; yi < m_n_states; ++yi)
					{
						int index = yi + yj * m_n_states;

						m_dtheta_ij(index,c) += backnorm(yj) * s(yi,yj) / m_trw_rho / sum_s;
						m_dtheta_i(yi,i) += backnorm(yj) * s(yi,yj) / sum_s;

						dn(yi) += backnorm(yj) * s(yi,yj) / m_trw_n(yi,i) / sum_s;
						dm1(yi,c) -= backnorm(yj) * s(yi,yj) / m_trw_m1(yi,c) /sum_s;
					}
				}

// 					for (int yi = 0; yi < m_n_states; ++yi)
// 					{
// 						for (int k = 0; k < m_n1.cols(); ++k)
// 						{
// 							int d = m_n1(i,k);
// 							if (d == -1) continue;
// 
// 							dm1(yi,d) += m_trw_rho * dn(yi,i) * m_trw_n(yi,i) / m_trw_m1(yi,d);
// 						}
// 
// 						for (int k = 0; k< m_n2.cols(); ++k)
// 						{
// 							int d = m_n2(i,k);
// 							if (d == -1) continue;
// 
// 							dm2(yi,d) += m_trw_rho * dn(yi,i) * m_trw_n(yi,i) / m_trw_m2(yi,d);	
// 						}
// 					}

				for (int yj=0;yj<m_n_states;++yj)
				{
					dm2(yj,c) = 0.0f;
				}

				//receive back propagation message
				for (int yj = m_n_states - 1; yj >= 0; --yj)
				{
					m_back_propagation_count--;
					if (m_back_propagation_count < 0)
					{
						return m_loss;
					}
					m_trw_m2(yj,c) = m_back_propagation_marginal(m_back_propagation_count);
				}
			}
			else
			{
				for (int yj = 0; yj < m_n_states; ++yj)
				{
					m_trw_n(yj,j) = 1;
				}
				for (int k = 0; k < int(m_n1.cols()); ++k)
				{
					int d = m_n1(j,k);
					if (d == -1) continue;
					
					for (int yj = 0; yj < m_n_states; ++yj)
					{
						m_trw_n(yj,j) *= m_trw_m1(yj,d);
					}
				}
				for (int k = 0;k < int(m_n2.cols()); ++k)
				{
					int d = m_n2(j,k);
					if (d == -1) continue;
					
					for (int yj = 0; yj < m_n_states; ++yj)
					{
						m_trw_n(yj,j) *= m_trw_m2(yj,d);
					}
				}

				if (m_trw_rho == 1)
				{
					//do nothing
				}
				else if(m_trw_rho == 0.5f)
				{
					for (int yj = 0; yj < m_n_states; ++yj)
					{
						m_trw_n(yj,j) = sqrt(m_trw_n(yj,j));
					}
				}
				else
				{
					for (int yj = 0; yj < m_n_states; ++yj)
					{
						m_trw_n(yj,j) = pow(m_trw_n(yj,j),m_trw_rho);
					}
				}

				//compute m(y_i)
				float sum_s = 0;
				for (int yi = 0; yi < m_n_states; ++yi)
				{
					m0(yi) = 0;
					for (int yj = 0; yj < m_n_states; ++yj)
					{
						int index = yi + yj * m_n_states;
						s(yi,yj) = m_rho_psi_ij(index,c) * m_psi_i(yj,j) * m_trw_n(yj,j) / m_trw_m2(yj,c);//see (18) in paper
						m0(yi)	+= s(yi,yj);
						sum_s	+= s(yi,yj);
					}
				}

				float inner = 0;
				for (int yi = 0; yi < m_n_states; ++yi)
				{
					inner += dm1(yi,c) * m0(yi)/sum_s;
				}
				//compute (g-g*c) in backnorm(g,c)
				for(int yi = 0; yi < m_n_states; ++yi)
				{
					backnorm(yi) = (dm1(yi,c) - inner);
				}

				for (int yj = 0; yj < m_n_states; ++yj)
				{
					for (int yi = 0; yi < m_n_states; ++yi)
					{
						int index = yi + yj * m_n_states;
						
						m_dtheta_ij(index,c) += backnorm(yi) * s(yi,yj) / m_trw_rho / sum_s;
						m_dtheta_i(yj,j) += backnorm(yi) * s(yi,yj) / sum_s;
						
						dn(yj) += backnorm(yi) *s(yi,yj) / m_trw_n(yj,j) / sum_s;
						dm2(yj,c) -= backnorm(yi) * s(yi,yj) / m_trw_m2(yj,c) / sum_s;
					}
				}

				for (int yj = 0; yj < m_n_states; ++yj)
				{
					for (int k = 0; k < int(m_n1.cols()); ++k)
					{
						int d = m_n1(j,k);
						if (d == -1) continue;
						
						dm1(yj,d) += m_trw_rho * dn(yj) * m_trw_n(yj,j) / m_trw_m1(yj,d);
					}
					for (int k = 0; k < int(m_n2.cols()); ++k)
					{
						int d = m_n2(j,k);
						if (d == -1) continue;

						dm2(yj,d) += m_trw_rho * dn(yj) * m_trw_n(yj,j) / m_trw_m2(yj,d);
					}
				}

				for (int yi = 0;yi < m_n_states;++yi)
				{
					dm1(yi,c) = 0.0f;
				}

				//receive back propagation message
				for (int yi = m_n_states-1; yi >= 0; --yi)
				{
					m_back_propagation_count--;
					if (m_back_propagation_count < 0)
					{
						return m_loss;
					}
					m_trw_m1(yi,c) = m_back_propagation_marginal(m_back_propagation_count);
				}
			}
		}
	}

	return m_loss;
}

void MarginalCRF::trwInfer()
{
	//clear back propagation marginal
	m_back_propagation_marginal.setzeros();
	m_back_propagation_count = 0;

	//fill m_psi_i and m_psi_ij
	//transform to exp(..)
	int theta_i_rows = m_theta_i.rows();
	int theta_i_cols = m_theta_i.cols();
	for (int i = 0; i < theta_i_rows; ++i)
	{
		float* m_psi_i_data = m_psi_i.row(i);
		float* theta_i_data = m_theta_i.row(i);
		for (int j = 0; j < theta_i_cols; ++j)
		{
			m_psi_i_data[j] = exp(theta_i_data[j]);
		}
	}

	int theta_ij_rows = m_theta_ij.rows();
	int theta_ij_cols = m_theta_ij.cols();
	for (int i = 0; i < theta_ij_rows; ++i)
	{
		float* m_psi_ij_data = m_psi_ij.row(i);
		float* m_rho_psi_ij_data = m_rho_psi_ij.row(i);
		float* theta_ij_data = m_theta_ij.row(i);

		for (int j = 0; j < theta_ij_cols; ++j)
		{
			//perhaps, you will be confused.
			//please see formula (18) in paper
			m_psi_ij_data[j] = exp(theta_ij_data[j]);
			m_rho_psi_ij_data[j] = exp(theta_ij_data[j] / m_trw_rho);
		}
	}

	//m_b_i and m_b_ij are marginal distribution
	//now we initialize m_b_i and m_b_ij with 1.0f
	int total = m_b_i.rows() * m_b_i.cols();
	float* b_i_data = m_b_i.dataptr();
	for (int i = 0; i < total; ++i)
	{
		b_i_data[i] = 1.0f;
	}
	total = m_b_ij.rows() * m_b_ij.cols();
	float* b_ij_data = m_b_ij.dataptr();
	for (int i = 0; i < total; ++i)
	{
		b_ij_data[i] = 1.0f;
	}

	//define some intermediate variables
	m_trw_n = Matrix<float>(m_n_states,m_n_nodes);
	Matrix<float> s(m_n_states,m_n_states);
	Matrix<float> m0(m_n_states,1);

	Matrix<float> b_i_save(m_b_i.rows(),m_b_i.cols());
	b_i_save.copy(m_b_i);
	float conv = 0;

	for (int reps = 0; reps < m_maxiter; ++reps)
	{
		for (int c0 = 0;c0 < m_n_cliques * 2; ++c0)
		{
			int c,mode;
			if (c0 < m_n_cliques)
			{
				c = c0;
				mode = 1;
			}
			else
			{
				c = m_n_cliques - 1 - ( c0 - m_n_cliques );
				mode = 2;
			}
			int i = m_pairs(c,0);	//left node
			int j = m_pairs(c,1);	//right node

			if (mode == 1)
			{
				for (int yi = 0;yi < m_n_states; ++yi)
				{
					m_trw_n(yi,i) = 1.0f;
				}
				for (int k = 0; k < int(m_n1.cols()); ++k)
				{
					//the clique contained node i as left node
					int d = m_n1(i,k);
					if (d == -1) continue;
					
					for (int yi = 0;yi < m_n_states; ++yi)
					{
						m_trw_n(yi,i) *= m_trw_m1(yi,d);
					}						
				}
				for (int k = 0; k < int(m_n2.cols()); ++k)
				{
					//the clique contained node i as right node
					int d = m_n2(i,k);
					if (d == -1) continue;
					
					for (int yi = 0; yi < m_n_states; ++yi)
					{
						m_trw_n(yi,i) *= m_trw_m2(yi,d);
					}
				}

				if (m_trw_rho == 1)
				{
					//do nothing
				}
				else if(m_trw_rho == 0.5f)
				{
					for (int yi = 0; yi < m_n_states; ++yi)
					{
						m_trw_n(yi,i) = sqrt(m_trw_n(yi,i));
					}
				}
				else
				{
					for (int yi = 0; yi < m_n_states; ++yi)
					{
						m_trw_n(yi,i) = pow(m_trw_n(yi,i),m_trw_rho);
					}
				}

				//compute mc(yj)
				float sum_s = 0;
				for (int yj = 0; yj < m_n_states; ++yj)
				{
					m0(yj) = 0;
					for (int yi = 0; yi < m_n_states; ++yi)
					{
						int index = yi + yj * m_n_states;
						s(yi,yj) = m_rho_psi_ij(index,c) * m_psi_i(yi,i) * m_trw_n(yi,i) / m_trw_m1(yi,c);//see (18) in paper
						m0(yj) += s(yi,yj);
						sum_s += s(yi,yj);
					}
				}

				//update inward message
				for (int yj = 0; yj < m_n_states; ++yj)
				{
					m_trw_m2(yj,c) = m0(yj) / sum_s;
				}

				//record normalized message
				for (int yj = 0; yj < m_n_states; ++yj)
				{
					//prepare for back propagation
					m_back_propagation_marginal(m_back_propagation_count) = m_trw_m2(yj,c);
					m_back_propagation_count++;
				}
			}
			else
			{
				for (int yj = 0; yj < m_n_states; ++yj)
				{
					m_trw_n(yj,j) = 1.0f;
				}

				//compute n. see (18) in paper
				for (int k=0;k< int(m_n1.cols());++k)
				{
					int d = m_n1(j,k);
					if (d == -1) continue;
					
					for (int yj = 0;yj < m_n_states;++yj)
					{
						m_trw_n(yj,j) *= m_trw_m1(yj,d);
					}						
				}
				for (int k = 0; k < int(m_n2.cols()); ++k)
				{
					int d = m_n2(j,k);
					if (d == -1) continue;

					for (int yj = 0; yj < m_n_states; ++yj)
					{
						m_trw_n(yj,j) *= m_trw_m2(yj,d);
					}
				}

				if (m_trw_rho == 1)
				{
					//do nothing
				}
				else if(m_trw_rho == 0.5f)
				{
					for (int yj = 0; yj < m_n_states; ++yj)
					{
						m_trw_n(yj,j) = sqrt(m_trw_n(yj,j));
					}
				}
				else
				{
					for (int yj = 0; yj < m_n_states; ++yj)
					{
						m_trw_n(yj,j) = pow(m_trw_n(yj,j),m_trw_rho);
					}
				}

				//compute mc(yi)
				float sum_s = 0;
				for (int yi = 0; yi < m_n_states; ++yi)
				{
					m0(yi) = 0;
					for (int yj = 0; yj < m_n_states; ++yj)
					{
						int index = yi + yj * m_n_states;
						s(yi,yj) = m_rho_psi_ij(index,c) * m_psi_i(yj,j) * m_trw_n(yj,j) / m_trw_m2(yj,c);
						m0(yi) += s(yi,yj);
						sum_s += s(yi,yj);
					}
				}

				for (int yi = 0; yi < m_n_states; ++yi)
				{
					m_trw_m1(yi,c) = m0(yi) / sum_s;
				}

				//record normalized message
				for (int yi = 0; yi < m_n_states; ++yi)
				{
					//prepare for back propagation
					m_back_propagation_marginal(m_back_propagation_count) = m_trw_m1(yi,c);
					m_back_propagation_count++;
				}

			}
		}

		//compute marginal distribution of every node and normalize
		m_b_i_z.setzeros();
		for (int node_index = 0; node_index < m_n_nodes; ++node_index)
		{
			for (int state_index = 0; state_index < m_n_states; ++state_index)
			{
				m_b_i(state_index,node_index) = m_trw_n(state_index,node_index) * m_psi_i(state_index,node_index);
				m_b_i_z(node_index) +=m_b_i(state_index,node_index);
			}

			for (int state_index = 0; state_index < m_n_states; ++state_index)
			{
				m_b_i(state_index,node_index) /= m_b_i_z(node_index);
			}
		}

		conv = 0;
		for (unsigned int m = 0; m < m_b_i.rows(); ++m)
		{
			for(unsigned int n = 0; n < m_b_i.cols(); ++n)
			{
				if (conv < abs(m_b_i(m,n) - b_i_save(m,n)))
				{
					conv = abs(m_b_i(m,n)-b_i_save(m,n));
				}
			}
		}

		if (conv < m_convthreshold)
		{
			break;
		}
		b_i_save.copy(m_b_i);
	}

	//stop here, we believe that it has been converged.

	//recompute all inwards messages
	for (int i = 0; i < m_n_nodes; ++i)
	{
		for (int yi = 0; yi < m_n_states; ++yi)
		{
			m_trw_n(yi,i) = 1.0f;

			for (int k = 0; k < int(m_n1.cols()); ++k)
			{
				int d = m_n1(i,k);
				if (d == -1) continue;
				
				m_trw_n(yi,i) *= m_trw_m1(yi,d);
			}

			for (int k = 0; k < int(m_n2.cols()); ++k)
			{
				int d = m_n2(i,k);
				if (d == -1) continue;
				
				m_trw_n(yi,i) *= m_trw_m2(yi,d);
			}
		}

		if (m_trw_rho == 1)
		{
			//do nothing
		}
		else if(m_trw_rho == 0.5f)
		{
			for (int yi = 0;yi < m_n_states;++yi)
			{
				m_trw_n(yi,i) = sqrt(m_trw_n(yi,i));
			}
		}
		else
		{
			for (int yi = 0;yi < m_n_states;++yi)
			{
				m_trw_n(yi,i) = pow(m_trw_n(yi,i),m_trw_rho);
			}
		}

	}

	//compute marginal distribution of every clique
	for (int c = 0; c < m_n_cliques; ++c)
	{
		int i = m_pairs(c,0);
		int j = m_pairs(c,1);
		for (int yi = 0; yi < m_n_states; ++yi)
		{
			for (int yj = 0; yj < m_n_states; ++yj)
			{
				int index = yi + yj * m_n_states;
				m_b_ij(index,c) = m_rho_psi_ij(index,c) * 
					m_psi_i(yi,i) *	m_psi_i(yj,j) * 
					m_trw_n(yi,i) * m_trw_n(yj,j) / m_trw_m1(yi,c) / m_trw_m2(yj,c);
			}
		}
	}

	//normalize marginal distribution of  clique
	m_b_ij_z.setzeros();
	int tmp = m_n_states * m_n_states;
	for (int clique_index = 0; clique_index < m_n_cliques; ++clique_index)
	{
		for (int state_index = 0; state_index < tmp; ++state_index)
		{
			m_b_ij_z(clique_index) += m_b_ij(state_index,clique_index);
		}

		for (int state_index = 0; state_index < tmp; ++state_index)
		{
			m_b_ij(state_index,clique_index) /= m_b_ij_z(clique_index);
		}
	}
}

void MarginalCRF::infer()
{
	//inference type
	//using back propagation to compute derivative with respect marginal distribution 
	//and implicit parameter of every node
	switch(m_infer_type)
	{
	case MEANFIELD:
		{
			m_back_propagation_marginal = Matrix<float>(m_maxiter * (2 * m_n_nodes - 1) * m_n_states,1,float(0));
			break;
		}
	case TRW:
		{
			m_back_propagation_marginal = Matrix<float>(m_maxiter * (2 * m_n_cliques) * m_n_states * m_n_states,1,float(0));
			m_rho_psi_ij = Matrix<float>(m_n_states,m_n_nodes,float(0));
			m_trw_m1 = Matrix<float>(m_n_states,m_n_cliques, float(1));
			m_trw_m2 = Matrix<float>(m_n_states,m_n_cliques, float(1));
			break;
		}
	}	

	switch(m_infer_type)
	{
	case MEANFIELD:
		{
			meanfieldInfer();
			break;
		}
	case TRW:
		{
			trwInfer();
			break;
		}
	}
}

float MarginalCRF::backprop()
{
	switch(m_infer_type)
	{
	case MEANFIELD:
		{
			return backpropMeanfield();
		}
	case TRW:
		{
			return backpropTRW();
		}
	}

	return 0;
}

void MarginalCRF::setUsingData(const Matrix<float>& unary_pot,const Matrix<float>& clique_pot,const Matrix<int>& target_states)
{
	//unary potential
	assert(m_n_nodes == unary_pot.cols());
	//feature * node
	m_unary_pot = unary_pot;
	m_unary_pot_t = m_unary_pot.t();	//easy to use

	//clique potential
	assert(m_n_cliques == clique_pot.cols());
	//clique feature * clique 
	m_clique_pot = clique_pot;
	m_clique_pot_t = m_clique_pot.t();	//easy to use

	//target states
	if (m_loss_fun && !target_states.isempty())
		m_loss_fun->setTargetStates(target_states);
}

void MarginalCRF::train(const Matrix<int>& target_states,
						const Matrix<float>& unary_pot,const Matrix<float>& pairwise_pot,
						Matrix<float>& unary_coe,Matrix<float>& clique_coe)
{
	m_unary_samples.resize(1);
	m_unary_samples[0] = unary_pot;

	m_clique_samples.resize(1);
	m_clique_samples[0] = pairwise_pot;

	m_states_samples.resize(1);
	m_states_samples[0] = target_states;

	train(m_states_samples,m_unary_samples,m_clique_samples,unary_coe,clique_coe);
}

void MarginalCRF::train(const std::vector<Matrix<int>>& target_states_samples, 
						const std::vector<Matrix<float>>& unary_samples, 
						const std::vector<Matrix<float>>& clique_samples, 
						Matrix<float>& unary_coe, Matrix<float>& clique_coe)
{
	m_unary_samples = unary_samples;
	m_clique_samples = clique_samples;
	m_states_samples = target_states_samples;
	
	//feature dimension
	m_unary_feature_dim = m_unary_samples[0].rows();
	m_clique_feature_dim = m_clique_samples[0].rows();

	//initialize some relevant matrix
	m_unary_coe = Matrix<float>(m_n_states,m_unary_feature_dim);
	m_clique_coe = Matrix<float>(m_n_states * m_n_states,m_clique_feature_dim);

	//inference type
	//using back propagation to compute derivative with respect marginal distribution 
	//and implicit parameter of every node
	switch(m_infer_type)
	{
	case MEANFIELD:
		{
			m_back_propagation_marginal = Matrix<float>(m_maxiter * (2 * m_n_nodes - 1) * m_n_states,1,float(0));
			break;
		}
	case TRW:
		{
			m_back_propagation_marginal = Matrix<float>(m_maxiter * (2 * m_n_cliques) * m_n_states * m_n_states,1,float(0));
			m_rho_psi_ij = Matrix<float>(m_n_states,m_n_nodes,float(0));
			m_trw_m1 = Matrix<float>(m_n_states,m_n_cliques, float(1));
			m_trw_m2 = Matrix<float>(m_n_states,m_n_cliques, float(1));
			break;
		}
	}	

	//build loss function
	switch(m_loss_fun_type)
	{
	case LOSS_UL:
		{
			if (m_loss_fun)
			{
				delete m_loss_fun;
			}
			m_loss_fun = new LossULFunction(m_pairs,m_n_states);
			break;
		}
	case LOSS_CL:
		{
			if (m_loss_fun)
			{
				delete m_loss_fun;
			}
			m_loss_fun = new LossCLFunction(m_pairs,m_n_states);
			break;
		}
	case LOSS_UQUAD:
		{
			if (m_loss_fun)
			{
				delete m_loss_fun;
			}
			m_loss_fun = new LossUQUADFunction(m_pairs,m_n_states);
			break;
		}
	case LOSS_CQUAD:
		{
			if (m_loss_fun)
			{
				delete m_loss_fun;
			}
			m_loss_fun = new LossCQUADFunction(m_pairs,m_n_states);
			break;
		}
	}

	//run optimization algorithm
	m_crf_lfgs->setUnkownX(m_unary_coe.rows() * m_unary_coe.cols() + 
		m_clique_coe.rows() * m_clique_coe.cols());

	m_crf_lfgs->run();

	//return unary weight and clique weight
	unary_coe = m_unary_coe;
	clique_coe = m_clique_coe;
}

void MarginalCRF::infer(const Matrix<float>& unary_pot,const Matrix<float>& clique_pot,
							Matrix<int>& states,Matrix<float>& probability)
{
	Matrix<int> target_states;//empty matrix
	setUsingData(unary_pot,clique_pot,target_states);

	//generate m_theta_i and m_theta_ij
	//using m_unary_weight and m_unary_pot to generate m_theta_i
	m_theta_i = m_unary_coe * m_unary_pot;
	//using m_clique_weight and m_clique_pot to generate m_theta_ij
	m_theta_ij = m_clique_coe * m_clique_pot;

	//infer the marginal distribution of every node
//		meanfieldInfer();
	infer();

	states = Matrix<int>(m_n_nodes,1,int(0));

	//marginal distribution of every node
	probability = m_b_i.t();

	//state of every node
	for (int node_index = 0; node_index < m_n_nodes; ++node_index)
	{
		float val = m_b_i(0,node_index);
		int infer_state = 0;
		for (int state_index = 1; state_index < m_n_states; ++state_index)
		{
			if (val < m_b_i(state_index,node_index))
			{
				val = m_b_i(state_index,node_index);
				infer_state = state_index;
			}
		}
		states(node_index,0) = infer_state;
	}
}

float MarginalCRF::targetProbability(const Matrix<int>& target_states,
									 const Matrix<float>& unary_pot, 
										const Matrix<float>& clique_pot)
{
	setUsingData(unary_pot,clique_pot,target_states);

	//generate m_theta_i and m_theta_ij
	//using m_unary_weight and m_unary_pot to generate m_theta_i
	m_theta_i = m_unary_coe * m_unary_pot;
	//using m_clique_weight and m_clique_pot to generate m_theta_ij
	m_theta_ij = m_clique_coe * m_clique_pot;

	//infer the marginal distribution of every node
	infer();

	//get joint probability
	float target_probability = 1.0f;
	for (int node_index = 0; node_index < m_n_nodes; ++node_index)
	{
		int state_index = target_states(node_index);
		target_probability *= m_b_i(state_index,node_index);
	}

	return target_probability;
}

void MarginalCRF::saveMarginalCRF(const char* crf_model_file)
{
	EagleeyeIO crf_model_o;
	crf_model_o.createWriteHandle(crf_model_file,false,WRITE_BINARY_MODE);
	
	crf_model_o.write(m_unary_coe);
	crf_model_o.write(m_clique_coe);

	crf_model_o.destroyHandle();
}
void MarginalCRF::loadMarginalCRF(const char* crf_model_file)
{
	EagleeyeIO crf_model_i;
	crf_model_i.createReadHandle(crf_model_file,READ_BINARY_MODE);

	crf_model_i.read(m_unary_coe);
	crf_model_i.read(m_clique_coe);

	crf_model_i.destroyHandle();

	m_n_states = m_unary_coe.rows();
}

void MarginalCRF::setModelParam(const Matrix<float>& unary_coe,const Matrix<float>& clique_coe)
{
	m_unary_coe  = unary_coe;
	m_clique_coe = clique_coe;

	m_n_states = m_unary_coe.rows();
}

void MarginalCRF::getModelParam(Matrix<float>& unary_coe,Matrix<float>& clique_coe)
{
	unary_coe = m_unary_coe;
	clique_coe = m_clique_coe;
}

//////////////////////////////////////////////////////////////////////////
ImageCRF::ImageCRF(InferenceType infer_type /* = MEANFIELD */,LossFunType loss_fun_type /* = LOSS_CL */)
:MarginalCRF(infer_type,loss_fun_type)
{

}
void ImageCRF::constructImgGraph(int rows,int cols,int states_num)
{
	m_n_states = states_num;
	m_n_nodes = rows * cols;
	m_n_cliques = (cols - 1) * rows + (rows - 1) * cols;

	m_pairs = Matrix<int>(m_n_cliques,2);
	int pair_count = 0;
	//fill m_pairs
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < cols - 1; ++j)
		{
			//left node
			m_pairs(pair_count,0) = i * cols + j;
			//right node
			m_pairs(pair_count,1) = i * cols + (j + 1);

			pair_count++;
		}
	}
	for (int i = 0; i < rows - 1; ++i)
	{
		for (int j = 0; j < cols; ++j)
		{
			//left node
			m_pairs(pair_count,0) = i * cols + j;
			//right node
			m_pairs(pair_count,1) = (i + 1) * cols + j;

			pair_count++;
		}
	}

	//fill m_n1
	m_n1 = Matrix<int>(m_n_nodes,4,int(-1));
	Matrix<int> wherenode(m_n_nodes,1,int(0));

	for (int pair_index = 0; pair_index < pair_count; ++pair_index)
	{
		m_n1(m_pairs(pair_index,0),wherenode(m_pairs(pair_index,0),0)) = pair_index;
		wherenode(m_pairs(pair_index,0),0) = wherenode(m_pairs(pair_index,0),0) + 1;
	}

	//fill m_n2
	m_n2 = Matrix<int>(m_n_nodes,4,int(-1));
	wherenode.setzeros();

	for (int pair_index = 0; pair_index < pair_count; ++pair_index)
	{
		m_n2(m_pairs(pair_index,1),wherenode(m_pairs(pair_index,1),0)) = pair_index;
		wherenode(m_pairs(pair_index,1),0) = wherenode(m_pairs(pair_index,1),0) + 1;
	}

	//initialize all relevant matrix
	m_theta_i = Matrix<float>(m_n_states,m_n_nodes,float(0));
	m_theta_ij = Matrix<float>(m_n_states * m_n_states,m_n_cliques,float(0));
	m_dtheta_i = Matrix<float>(m_n_states,m_n_nodes,float(0));
	m_dtheta_ij = Matrix<float>(m_n_states * m_n_states,m_n_cliques,float(0));

	m_psi_i = Matrix<float>(m_n_states,m_n_nodes,float(0));
	m_psi_ij = Matrix<float>(m_n_states * m_n_states,m_n_cliques,float(0));

	m_b_i = Matrix<float>(m_n_states,m_n_nodes,float(0));
	m_b_ij = Matrix<float>(m_n_states * m_n_states,m_n_cliques,float(0));

	m_b_i_z = Matrix<float>(m_n_nodes,1,float(0));
	m_b_ij_z = Matrix<float>(m_n_cliques,1,float(0));

	m_db_i = Matrix<float>(m_n_states,m_n_nodes,float(0));
	m_db_ij = Matrix<float>(m_n_states * m_n_states,m_n_cliques,float(0));
}

}