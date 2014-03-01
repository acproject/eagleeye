#include "TopHitSymbol.h"
#include "MatrixMath.h"

namespace eagleeye
{
TopHitSymbol::TopHitSymbol(const char* name):ObjectDetSymbol(name,NON_TERMINAL)
{
	m_k_top_hit = 3;
	m_level_num = 0;
}
TopHitSymbol::~TopHitSymbol()
{

}

void* TopHitSymbol::getSymbolScore(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat )
{
	if (search_space != SUPERPIXEL_SPACE)
		return NULL;

	std::vector<LinkRule*>::iterator iter,iend(m_rule_pool.end());
	int link_count = 0;
	for (iter = m_rule_pool.begin(); iter != iend; ++iter)
	{
		void* score_data = (*iter)->getParseScore(search_space,search_mode,pos_mat);
		Matrix<float> score_mat = *((Matrix<float>*)score_data);

		int num = score_mat.rows();
		Matrix<float> predict_probability(1,m_class_num,float(0.0f));

		if (link_count == 0)
		{
			//run only once
			m_sym_score = Matrix<float>(score_mat.rows(),SUPERPIXEL_SCORE_OFFSET + 2,float(-EAGLEEYE_FINF));
			m_k_top_hit_score = Matrix<float>(score_mat.rows(),SUPERPIXEL_SCORE_OFFSET + 2 * m_k_top_hit,float(-EAGLEEYE_FINF));
		}

		//compute top hit
		for (int i = 0; i < num; ++i)
		{
			if (link_count == 0)
			{
				//fill position info
				m_sym_score(i,0) = score_mat(i,0);							//superpixel index
				m_sym_score(i,1) = score_mat(i,1);							//superpixel level
				m_sym_score(i,2) = score_mat(i,2);							//superpixel level scale

				m_k_top_hit_score(i,0) = score_mat(i,0);					//superpixel index
				m_k_top_hit_score(i,1) = score_mat(i,1);					//superpixel level
				m_k_top_hit_score(i,2) = score_mat(i,2);					//superpixel level scale
			}

			//extract probability data
			for (int label_index = 0; label_index < m_class_num; ++label_index)
			{
				predict_probability(label_index) = score_mat(i,SUPERPIXEL_SCORE_OFFSET + label_index * 2 + 1);
			}

			//sort probability
			std::vector<unsigned int> order_index = sort<DescendingSortPredict<float>>(predict_probability);

			//get top-hit-k score
			float top_hit_k_socre = 0;
			for (int k = 0; k < m_k_top_hit; ++k)
			{
				top_hit_k_socre += predict_probability(order_index[k]);
			}

			if (m_sym_score(i,SUPERPIXEL_SCORE_OFFSET) < top_hit_k_socre)
			{
				m_sym_score(i,SUPERPIXEL_SCORE_OFFSET) = 0;
				m_sym_score(i,SUPERPIXEL_SCORE_OFFSET + 1) = top_hit_k_socre;
				
				int superpixel_index = int(score_mat(i,0));		//superpixel index
				int level_index = int(score_mat(i,1));			//superpixel level

				m_top_hit_link[superpixel_index * m_level_num + level_index] = link_count;
				m_first_index[superpixel_index * m_level_num + level_index] = order_index[0];
				m_first_score[superpixel_index * m_level_num + level_index] = predict_probability(order_index[0]);

				for (int k = 0; k < m_k_top_hit; ++k)
				{
					m_k_top_hit_score(i,SUPERPIXEL_SCORE_OFFSET + k * 2) = order_index[k];
					m_k_top_hit_score(i,SUPERPIXEL_SCORE_OFFSET + k * 2 + 1) = predict_probability(order_index[k]);
				}
			}
		}

		link_count++;
	}

	return &m_sym_score;
}

void TopHitSymbol::findModelLatentInfo(void* info)
{
	DetSymbolInfo top_hit_info = *((DetSymbolInfo*)info);
	int superpixel_index = top_hit_info.superpixel;
	int level_index = top_hit_info.level;

	int index = superpixel_index * m_level_num + level_index;

	int link_rule_index = m_top_hit_link[index];
	top_hit_info.label = m_first_index[index];
	top_hit_info.val = m_first_score[index];
	m_rule_pool[link_rule_index]->findModelLatentInfo(&top_hit_info);
}

void TopHitSymbol::getUnitPara(MemoryBlock& param_block)
{
	param_block = MemoryBlock(sizeof(int));
	int* para = (int*)param_block.block();
	
	*para = m_k_top_hit;
}
void TopHitSymbol::setUnitPara(MemoryBlock param_block)
{
	int* para = (int*)param_block.block();
	m_k_top_hit = *para;
}

void TopHitSymbol::setUnitData(void* data, SampleState sample_state,void* auxiliary_data)
{
	if (sample_state == EAGLEEYE_UNDEFINED_SAMPLE)
	{
		//get pyramid levels number
		AuxiliaryInfoInSuperpixelSpace* auxiliary_info = (AuxiliaryInfoInSuperpixelSpace*)auxiliary_data;
		m_level_num = auxiliary_info->superpixel_pyr.levels();
	}
}

void TopHitSymbol::setTopHitK(const int k)
{
	m_k_top_hit = k;
}

void TopHitSymbol::getModelOutput(std::vector<void*>& output_info)
{
	output_info.push_back(&m_k_top_hit_score);
}

}