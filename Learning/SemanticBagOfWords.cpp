#include "Learning/SemanticBagOfWords.h"

namespace eagleeye
{
SemanticBagOfWords::SemanticBagOfWords(int words_num,int states_num)
:MarginalCRF(MEANFIELD,LOSS_CL)
{
	m_words_num = words_num;
	m_pairwise_num = words_num * (words_num - 1);
	m_words_states = states_num;

	//construct semantic graph
	constructSemanticGraph();
}
SemanticBagOfWords::~SemanticBagOfWords()
{

}

void SemanticBagOfWords::semanticLearn(const Matrix<int>& words_state_samples,
									   const Matrix<float>& words_fre_samples,
									   const Matrix<float>& words_dis_samples,
									   const Matrix<float>& words_angle_samples)
{
	int samples_num = words_state_samples.rows();
	//words state
	//word1		word2	word3	word4 ...
	//label		label	label	label

	std::vector<Matrix<int>> nodes_target_states(samples_num);
	for (int sample_index = 0; sample_index < samples_num; ++sample_index)
	{
		nodes_target_states[sample_index] = words_state_samples(Range(sample_index,sample_index + 1),Range(0,m_words_num));
		nodes_target_states[sample_index].clone();
	}

	//words frequency
	//				word1	word2	word3 ...
	//frequency		 val	 val	 val
	std::vector<Matrix<float>> unary_pot_samples;
	unary_pot_samples.resize(samples_num);
	for (int sample_index = 0; sample_index < samples_num; ++sample_index)
	{
		unary_pot_samples[sample_index] = words_fre_samples(Range(sample_index,sample_index + 1),Range(0,m_words_num));
		unary_pot_samples[sample_index].clone();
	}

	//word pair distance
	//			word12	word13 ... word1n	word21	word23 ...
	//distance	 val	  val		val		 val	 val
	
	//word pair angle
	//			word12	word13 ... word1n	word21	word23 ...
	//angle		 val	  val		val		 val	 val
	std::vector<Matrix<float>> clique_pot_samples;
	clique_pot_samples.resize(samples_num);
	for (int sample_index = 0; sample_index < samples_num; ++sample_index)
	{
		Matrix<float> clique_pot(2,m_pairwise_num);
		clique_pot(Range(0,1),Range(0,m_pairwise_num)).copy(words_dis_samples(Range(sample_index,sample_index + 1),Range(0,m_pairwise_num)));
		clique_pot(Range(1,2),Range(0,m_pairwise_num)).copy(words_angle_samples(Range(sample_index,sample_index + 1),Range(0,m_pairwise_num)));

		clique_pot_samples[sample_index] = clique_pot;
	}

	Matrix<float> unary_coe,clique_coe;
	train(nodes_target_states,unary_pot_samples,clique_pot_samples,unary_coe,clique_coe);
}

Matrix<float> SemanticBagOfWords::semanticInfer(const Matrix<int>& words_state_samples, const Matrix<float>& words_fre_samples, const Matrix<float>& words_dis_samples, const Matrix<float>& words_angle_samples)
{
	int samples_num = words_state_samples.rows();
	//words frequency
	//				word1	word2	word3 ...
	//frequency		 val	 val	 val

	//word pair distance
	//			word12	word13 ... word1n	word21	word23 ...
	//distance	 val	  val		val		 val	 val

	//word pair angle
	//			word12	word13 ... word1n	word21	word23 ...
	//angle		 val	  val		val		 val	 val
	Matrix<float> probability_samples(samples_num,1);
	for (int sample_index = 0; sample_index < samples_num; ++sample_index)
	{
		Matrix<int> target_states = words_state_samples(Range(sample_index,sample_index + 1),Range(0,m_words_num));
		Matrix<float> unary_pot = words_fre_samples(Range(sample_index,sample_index + 1),Range(0,m_words_num));
		Matrix<float> clique_pot(2,m_pairwise_num);
		clique_pot(Range(0,1),Range(0,m_pairwise_num)).copy(words_dis_samples(Range(sample_index,sample_index + 1),Range(0,m_pairwise_num)));
		clique_pot(Range(1,2),Range(0,m_pairwise_num)).copy(words_angle_samples(Range(sample_index,sample_index + 1),Range(0,m_pairwise_num)));
		
		float p = targetProbability(target_states,unary_pot,clique_pot);
		probability_samples(sample_index,0) = p;
	}

	return probability_samples;
}

void SemanticBagOfWords::constructSemanticGraph()
{
	//construct full graph
	m_n_nodes = m_words_num;
	m_n_cliques = m_words_num * (m_words_num - 1);
	m_n_states = m_words_states;

	m_pairs = Matrix<int>(m_n_cliques,2);
	int pair_count = 0;
	for (int i = 0; i < m_words_num; ++i)
	{
		for (int j = 0; j < m_words_num; ++j)
		{
			if (i != j)
			{
				//left node
				m_pairs(pair_count,0) = i;
				//right node
				m_pairs(pair_count,1) = j;

				pair_count++;
			}
		}
	}

	//fill m_n1
	m_n1 = Matrix<int>(m_n_nodes,m_n_nodes,int(-1));
	Matrix<int> wherenode(m_n_nodes,1,int(0));
	for (int pair_index = 0; pair_index < pair_count; ++pair_index)
	{
		m_n1(m_pairs(pair_index,0),wherenode(m_pairs(pair_index,0),0)) = pair_index;
		wherenode(m_pairs(pair_index,0),0) += 1;
	}

	//fill m_n2
	m_n2 = Matrix<int>(m_n_nodes,m_n_nodes,int(-1));
	wherenode.setzeros();

	for (int pair_index = 0; pair_index < pair_count; ++pair_index)
	{
		m_n2(m_pairs(pair_index,1),wherenode(m_pairs(pair_index,1),0)) = pair_index;
		wherenode(m_pairs(pair_index,1),0) += 1;
	}

	//initialize all relevant matrix
	m_theta_i = Matrix<float>(m_n_states,m_n_nodes,float(0));
	m_theta_ij = Matrix<float>(m_n_states * m_n_states,m_n_cliques,float(0));
	m_dtheta_i = Matrix<float>(m_n_states,m_n_nodes,float(0));
	m_dtheta_ij = Matrix<float>(m_n_states * m_n_states,m_n_cliques,float(0));

	m_psi_i = Matrix<float>(m_n_states,m_n_nodes,float(0));
	m_psi_ij = Matrix<float>(m_n_states*m_n_states,m_n_cliques,float(0));

	m_b_i = Matrix<float>(m_n_states,m_n_nodes,float(0));
	m_b_ij = Matrix<float>(m_n_states * m_n_states,m_n_cliques,float(0));

	m_b_i_z = Matrix<float>(m_n_nodes,1,float(0));
	m_b_ij_z = Matrix<float>(m_n_cliques,1,float(0));

	m_db_i = Matrix<float>(m_n_states,m_n_nodes,float(0));
	m_db_ij = Matrix<float>(m_n_states*m_n_states,m_n_cliques,float(0));
}

}