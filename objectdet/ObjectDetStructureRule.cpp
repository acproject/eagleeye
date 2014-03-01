#include "ObjectDetStructureRule.h"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
ObjectDetStructureRule::ObjectDetStructureRule(const char* name)
	:LinkRule(name)
{
	m_offset_w = 0.0f;
	m_rule_pase_feature = Matrix<float>(1,PIXEL_FEATURE_DATA_OFFSET + 1,1.0f);
	m_rule_parse_weight = Matrix<float>(1,1,0.0f);
}

ObjectDetStructureRule::~ObjectDetStructureRule()
{

}

void* ObjectDetStructureRule::getParseScore(SearchSpace pos_index,SearchMode select_mode,Matrix<float> pos_mat /* = Matrix<float>() */)
{
	if (pos_index != PIXEL_SPACE || select_mode != INDEPENDENT_SEARCH)
		return NULL;

	//if data isn't updated, it would return score pyramid directly
	if (!m_data_update_flag)
		return &m_structure_rule_score_pyr;

	//in general, position is in image space, we should transform it at some proper space
	//here, due to computing all position in feature space, we ignore pos_mat

	//traverse all linked symbols
	int symbols_num = m_linked_symbols.size();
	for (int symbol_index = 0; symbol_index < symbols_num; ++symbol_index)
	{
		//we shouldn't modify the score pyramid from the linked symbol
		//warning: here, we don't pick some special position to compute score
		ScorePyramid symbol_score_pyr = *((ScorePyramid*)m_linked_symbols[symbol_index]->getSymbolScore(PIXEL_SPACE,INDEPENDENT_SEARCH));

		int interval = symbol_score_pyr.interval;
		int levels = symbol_score_pyr.levels();

		if (symbol_index == 0)
		{
			//run only once
			//initialize m_score_pyr
			//shallowcopy only copy pyramid struct info from symbol_score_pya
			m_structure_rule_score_pyr.shallowcopy(symbol_score_pyr);
			m_structure_rule_score_pyr.setValue(-EAGLEEYE_FINF);
		}

		//apply structure rule
		//update score pyramid

		//we would compute the corresponding score at the high resolution level
		//if m_anchor_level == 0, we do at the same level
		//if m_anchor_level == 1, we do at the twice resolution level

		//we should clearly know that anchor is fixed offset.
		int anchor_r,anchor_c,anchor_level;
		ObjectDetSymbol* objectdet_symbol = dynamic_cast<ObjectDetSymbol*>(m_linked_symbols[symbol_index]);
		objectdet_symbol->getAnchor(anchor_r,anchor_c,anchor_level);

		//the step in the high resolution level 
		int step = int(pow(2.0f,anchor_level));

		//find the start search position
		//this is the fixed offset
		int start_r = anchor_r;
		int start_c = anchor_c;

		//this is the low resolution level
		int search_start_level = interval * anchor_level;
		for (int search_index = search_start_level; search_index < levels; ++search_index)
		{
			//we need to check whether the current level of the score pyramid is valid
			if (m_structure_rule_score_pyr.flags(search_index))
			{
				//find high resolution level
				//For every position at the low resolution level, we find its corresponding position
				//at the high resolution level, then compute its score.
				int high_resolution_level = search_index - interval * anchor_level;

				//get score matrix at low and high resolution respectively
				//we use the data from symbol_score_pya to modify the data from m_score_pyr.
				Matrix<float> score_at_low_resolution = m_structure_rule_score_pyr[search_index];
				Matrix<float> score_at_high_resolution = symbol_score_pyr[high_resolution_level];

				//find the search position under the high resolution
				int end_r = eagleeye_min(score_at_high_resolution.rows(),start_r + step * symbol_score_pyr[search_index].rows());
				int end_c = eagleeye_min(score_at_high_resolution.cols(),start_c + step * symbol_score_pyr[search_index].cols());

				for (int i = start_r; i < end_r; i += step)
				{
					for (int j = start_c; j < end_c; j += step)
					{
						float s = score_at_high_resolution(i,j);

						int i_low = (i - start_r) / step;
						int j_low = (j - start_c) / step;

						//update score at the low resolution
						//why we should do such?
						//we should know that we want to find subparts(linked symbols) at the high 
						//resolution level.If we find subparts at the high resolution level, we should
						//enhance the score of the root at the low resolution level. That's why we do this.
						if (score_at_low_resolution(i_low,j_low) < -EAGLEEYE_NEAR_INF)
						{
							score_at_low_resolution(i_low,j_low) = s;
						}
						else
							score_at_low_resolution(i_low,j_low) += s;
					}
				}
			}
		}
	}

	//disable data update flag
	m_data_update_flag = false;

	return &m_structure_rule_score_pyr;
}

Matrix<float> ObjectDetStructureRule::getUnitFeature(SearchSpace pos_index,SearchMode select_mode,Matrix<float> pos_mat /* = Matrix<float>() */)
{
	if (pos_index != PIXEL_SPACE || select_mode != OPTIMUM_SEARCH)
		return Matrix<float>();

	m_rule_pase_feature(PIXEL_FEATURE_DATA_OFFSET + 0) = 1;
	if (m_sample_state == EAGLEEYE_POSITIVE_SAMPLE)
		m_rule_pase_feature[4] = 1.0f;	//label
	else
		m_rule_pase_feature[4] = 0.0f;	//label
	return m_rule_pase_feature;
}

void ObjectDetStructureRule::findModelLatentInfo(void* info)
{
	//firstly, we get prior-info from the upper symbol
	//What is the prior-info?
	//the info is the optimum position of the upper level symbol
	// we would set the optimum position of the low level symbol(some parts)
	DetSymbolInfo det_info =* ((DetSymbolInfo*)info);

	//According to the author's idea, every structure rule associated with the upper
	//symbol holds one whole subparts system.
	//Therefore, we need to know which linkrule works now.
	//Firstly, we check where the upper symbol optimum position come from 
	Matrix<float> check_score_m = m_structure_rule_score_pyr[det_info.level];

	int rows = check_score_m.rows();
	int cols = check_score_m.cols();

	int probe_y = det_info.y;
	int probe_x = det_info.x;

	if (check_score_m(probe_y,probe_x) == det_info.val)
	{
		//the upper symbol optimum score comes from this rule
		int symbols_num = m_linked_symbols.size();
		for (int symbol_index = 0; symbol_index < symbols_num; ++symbol_index)
		{
			ObjectDetSymbol* objectdet_symbol = dynamic_cast<ObjectDetSymbol*>(m_linked_symbols[symbol_index]);
			ScorePyramid linked_symbol_score_pya = *((ScorePyramid*)objectdet_symbol->getSymbolScore(PIXEL_SPACE,INDEPENDENT_SEARCH));

			//we should clear know that anchor is the fixed offset
			//get the anchor of the lower level symbol(subparts)
			int anchor_x,anchor_y,anchor_level;
			objectdet_symbol->getAnchor(anchor_y,anchor_x,anchor_level);

			//get location of the linked symbol
			//Generally speaking, the subpart lays at the 2x resolution level in the pyramid
			//At this case, anchor_level=1
			//If we force the subpart lays at the same resolution level, we should set
			//anchor_level=0
			//px, py, pl is the subpart position(r,c,level)
			int px = det_info.x * int(pow(2.0f,anchor_level)) + anchor_x;//add the fixed offset(anchor_x)
			int py = det_info.y * int(pow(2.0f,anchor_level)) + anchor_y;//add the fixed offset(anchor_y)
			int pl = det_info.level - m_structure_rule_score_pyr.interval * anchor_level;

			DetSymbolInfo s_info;
			s_info.x = px;
			s_info.y = py;
			s_info.level = pl;

			s_info.val = linked_symbol_score_pya[pl].at(py,px);

			s_info.ds = anchor_level;

			//continue to help the low level symbols to find latent variables.
			objectdet_symbol->findModelLatentInfo(&s_info);
		}	

		enableSubTree();
	}
	else
	{
		disableSubTree();
		//the upper symbol optimum score doesn't come from this rule
		return;
	}
}

Matrix<float> ObjectDetStructureRule::getUnitWeight()
{
	m_rule_parse_weight(0) = m_offset_w;
	return m_rule_parse_weight;
}

void ObjectDetStructureRule::setUnitWeight(const Matrix<float>& weight)
{
	m_offset_w = weight(0);
	m_rule_parse_weight(0) = weight(0);
}
}
