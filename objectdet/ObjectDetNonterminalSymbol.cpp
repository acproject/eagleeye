#include "ObjectDetNonterminalSymbol.h"
#include "MatrixMath.h"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
ObjectDetNonterminalSymbol::ObjectDetNonterminalSymbol(const char* name)
	:ObjectDetSymbol(name,NON_TERMINAL)
{

}

ObjectDetNonterminalSymbol::~ObjectDetNonterminalSymbol()
{

}

void* ObjectDetNonterminalSymbol::getSymbolScore(SearchSpace pos_index,SearchMode select_mode,Matrix<float> pos_mat /* = Matrix<float>() */)
{
	if (pos_index != PIXEL_SPACE || select_mode != INDEPENDENT_SEARCH)
		return NULL;

	//if data isn't updated, it would return score pyramid directly
	if (!m_data_update_flag)
		return &m_max_score_pyr;

	//traverse all linked rules
	//update score pyramid
	std::vector<LinkRule*>::iterator iter,iend(m_rule_pool.end());
	for (iter = m_rule_pool.begin(); iter != iend; ++iter)
	{
		//max
		//we shouldn't modify the score pyramid from the link rule
		ScorePyramid parse_score_pyr = *((ScorePyramid*)(*iter)->getParseScore(PIXEL_SPACE,INDEPENDENT_SEARCH));

		int levels = parse_score_pyr.levels();

		if (iter == m_rule_pool.begin())
		{
			//run only once
			//initialize score pyramid
			//deepcopy would copy data from another pyramid
			m_max_score_pyr.deepcopy(parse_score_pyr);
		}

		for (int i = 0; i < levels; ++i)
		{
			if (parse_score_pyr.flags(i))
			{
				//check whether the current level is valid
				Matrix<float> parse_score = parse_score_pyr[i];
				int rows = parse_score.rows();
				int cols = parse_score.cols();

				Matrix<float> max_symbol_score = m_max_score_pyr[i];

				for (int r = 0; r < rows; ++r)
				{
					float* parse_score_data = parse_score.row(r);
					float* max_symbol_score_data = max_symbol_score.row(r);

					for (int c = 0; c < cols; ++c)
					{
						//only find max value
						max_symbol_score_data[c] = (max_symbol_score_data[c] > parse_score_data[c]) ? max_symbol_score_data[c]:parse_score_data[c];
					}
				}
			}
		}
	}
	
	//disable data update flag
	m_data_update_flag = false;
	return &m_max_score_pyr;
}

void ObjectDetNonterminalSymbol::findModelLatentInfo(void* info)
{
	//For the ordinary NonTerminal symbol, 
	// it don't need any latent info(optimum detection pos)
	//we help 'linkrule' find their latent info
	
	DetSymbolInfo def_info = *((DetSymbolInfo*)info);
	def_info.val = m_max_score_pyr[def_info.level](def_info.y,def_info.x);

	std::vector<LinkRule*>::iterator iter,iend(m_rule_pool.end());
	for (iter = m_rule_pool.begin(); iter != iend; ++iter)
	{
		(*iter)->findModelLatentInfo(&def_info);
	}
}

void ObjectDetNonterminalSymbol::getUnitPara(MemoryBlock& param_block)
{
	param_block = MemoryBlock(sizeof(_Parameter));
	_Parameter* me_param_block = (_Parameter*)param_block.block();
	
	me_param_block->anchor_x = m_anchor_c;
	me_param_block->anchor_y = m_anchor_r;
	me_param_block->anchor_l = m_anchor_level;
}
void ObjectDetNonterminalSymbol::setUnitPara(MemoryBlock param_block)
{
	_Parameter* me_param_block = (_Parameter*)param_block.block();
	m_anchor_c = me_param_block->anchor_x;
	m_anchor_r = me_param_block->anchor_y;
	m_anchor_level = me_param_block->anchor_l;
}
}
