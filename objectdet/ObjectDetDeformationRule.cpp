#include "ObjectDetDeformationRule.h"
#include <math.h>

namespace eagleeye
{
ObjectDetDeformationRule::ObjectDetDeformationRule(const char* name)
	:LinkRule(name)
{
	//must be positive
	m_def_w_x0 = 0.005f;
	m_def_w_x1 = 0.005f;
	m_def_w_y0 = 0.005f;
	m_def_w_y1 = 0.005f;

	m_optimum_symbol_shift_x_det = 0;
	m_optimum_symbol_shift_y_det = 0;

	m_rule_parse_weight = Matrix<float>(1,PIXEL_FEATURE_HEAD_SIZE + 4,0.0f);
	m_rule_pase_feature = Matrix<float>(1,4,0.0f);
}

ObjectDetDeformationRule::~ObjectDetDeformationRule()
{

}

void* ObjectDetDeformationRule::getParseScore(SearchSpace pos_index,SearchMode select_mode,Matrix<float> pos_mat /* = Matrix<float>() */)
{
	if (pos_index != PIXEL_SPACE || select_mode != INDEPENDENT_SEARCH)
		return NULL;

	//if data isn't updated, it would return score pyramid directly
	if (!m_data_update_flag)
		return &m_rule_score_pyr;

	//get old score pyramid
	//we shouldn't modify the score pyramid from linked symbols
	//warning: in this rule, we don't pick some special position to compute score
	ScorePyramid symbol_score_pyr = *((ScorePyramid*)m_linked_symbols[0]->getSymbolScore(PIXEL_SPACE,INDEPENDENT_SEARCH));
	ScorePyramid score_pyr;
	score_pyr.deepcopy(symbol_score_pyr);
	int pyr_levels = symbol_score_pyr.levels();

	//apply deformation rule
	//update score pyramid
	//find the optimum shift for every pos. Of course, we also
	//want to get the optimum score after shifting.
	//We should know that "shifting" is operated at the same resolution level.

	//the algorithm of computing the optimum shift comes from "Pedro's work"
	dt(score_pyr);

	//copy score
	m_rule_score_pyr.shallowcopy(symbol_score_pyr);
	for (int level_index = 0; level_index < pyr_levels; ++level_index)
	{
		int rows = score_pyr[level_index].rows();
		int cols = score_pyr[level_index].cols();
		for (int i = 0; i < rows; ++i)
		{
			float* score_data = score_pyr[level_index].row(i);
			float* rule_score_data = m_rule_score_pyr[level_index].row(i);
			for (int j = 0; j < cols; ++j)
			{
				rule_score_data[j] = score_data[j];
			}
		}
	}
	
	//disable data update flag
	m_data_update_flag = false;
	return &m_rule_score_pyr;
}

void ObjectDetDeformationRule::findModelLatentInfo(void* info)
{
	//firstly, we get prior-info from the upper symbol
	//what is the prior-info?
	//the info is the optimum position of the upper level symbol
	//we should set the optimum position of the low level symbol

	DetSymbolInfo det_info = *((DetSymbolInfo*)info);

	//we should know that, according to the author's idea, every deformation rule link
	//one terminal symbol

	Matrix<float> score_mat = m_rule_score_pyr[det_info.level];

	int probe_y = det_info.y;
	int probe_x = det_info.x;

	if (score_mat(probe_y,probe_x) == det_info.val)
	{
		//the upper symbol optimum score comes from this rule
		ScorePyramid linked_score_pyr = *((ScorePyramid*)m_linked_symbols[0]->getSymbolScore(PIXEL_SPACE,INDEPENDENT_SEARCH));

		//According to the author's main idea, here,we don't need to
		//judge whether this rule works here.
		int py = det_info.y;
		int px = det_info.x;
		int pl = det_info.level;

		//call "findLatentVariable" function of all linked symbol,
		//we should help them find their own latent variable
		//now we should update info
		DetSymbolInfo s_info;
		s_info.x = m_optimum_shift_x_pyramid[pl].at(py,px);
		s_info.y = m_optimum_shift_y_pyramid[pl].at(py,px);
		s_info.level = pl;
		s_info.ds = det_info.ds;

		s_info.val = linked_score_pyr[pl](s_info.y,s_info.x);

		//get latent variables
		m_optimum_symbol_shift_x_det = m_optimum_shift_x_pyramid[pl].at(py,px) - px;
		m_optimum_symbol_shift_y_det = m_optimum_shift_y_pyramid[pl].at(py,px) - py;

		//there only exists one linked symbol	
		//continue to find latent variable
		m_linked_symbols[0]->findModelLatentInfo(&s_info);

		enableSubTree();
	}
	else
	{
		disableSubTree();
		return;
	}
}

Matrix<float> ObjectDetDeformationRule::getUnitWeight()
{
	m_rule_parse_weight(0) = m_def_w_x0;
	m_rule_parse_weight(1) = m_def_w_x1;
	m_rule_parse_weight(2) = m_def_w_y0;
	m_rule_parse_weight(3) = m_def_w_y1;

	return m_rule_parse_weight;
}

void ObjectDetDeformationRule::setUnitWeight(const Matrix<float>& weight)
{
	m_def_w_x0 = fabs(weight(0));
	m_def_w_x1 = fabs(weight(1));
	m_def_w_y0 = fabs(weight(2));
	m_def_w_y1 = fabs(weight(3));

	m_rule_parse_weight(0) = weight(0);
	m_rule_parse_weight(1) = weight(1);
	m_rule_parse_weight(2) = weight(2);
	m_rule_parse_weight(3) = weight(3);
}

Matrix<float> ObjectDetDeformationRule::getUnitFeature(SearchSpace pos_index,SearchMode select_mode,Matrix<float> pos_mat)
{	
	if (pos_index != PIXEL_SPACE || select_mode != OPTIMUM_SEARCH)
		return Matrix<float>();

	m_rule_pase_feature(PIXEL_FEATURE_DATA_OFFSET + 0) = -m_optimum_symbol_shift_x_det * m_optimum_symbol_shift_x_det * 0.1f;
	m_rule_pase_feature(PIXEL_FEATURE_DATA_OFFSET + 1) = -m_optimum_symbol_shift_x_det * 0.1f;
	m_rule_pase_feature(PIXEL_FEATURE_DATA_OFFSET + 2) = -m_optimum_symbol_shift_y_det * m_optimum_symbol_shift_y_det * 0.1f;
	m_rule_pase_feature(PIXEL_FEATURE_DATA_OFFSET + 3) = -m_optimum_symbol_shift_y_det * 0.1f;

	if (m_sample_state == EAGLEEYE_POSITIVE_SAMPLE)
		m_rule_pase_feature[4] = 1.0f;	//label
	else
		m_rule_pase_feature[4] = 0.0f;	//label
	return m_rule_pase_feature;
}

void ObjectDetDeformationRule::dt(ScorePyramid pyr)
{
	m_optimum_shift_x_pyramid.shallowcopy(pyr);
	m_optimum_shift_y_pyramid.shallowcopy(pyr);

	int levels=pyr.levels();
	for (int index=0;index<levels;++index)
	{
		if (pyr.flags(index))
		{
			//judge whether the current level is valid
			Matrix<float> score_img=pyr[index];
			int rows=score_img.rows();
			int cols=score_img.cols();

			Matrix<int> shift_x(rows,cols);
			Matrix<int> shift_y(rows,cols);

			//find the optimum shift along "row" direction
			Matrix<float> intermidinate_score_img(rows,cols);
			float* inter_score_img_data=NULL;
			float* score_img_data=NULL;

			for (int i=0;i<rows;++i)
			{
				score_img_data=score_img.row(i);
				int* ix_data=shift_x.row(i);
				inter_score_img_data=intermidinate_score_img.row(i);

				dt1d(score_img_data,inter_score_img_data,ix_data,1,cols,m_def_w_x0,m_def_w_x1);
			}

			//find the optimum shift along "col" direction
			inter_score_img_data=intermidinate_score_img.row(0);
			score_img_data=score_img.row(0);

			int* iy_data=shift_y.row(0);
			for (int i=0;i<cols;++i)
			{
				dt1d(inter_score_img_data+i,score_img_data+i,iy_data+i,cols,rows,m_def_w_y0,m_def_w_y1);
			}

			Matrix<int> optimum_shift_img_x(rows,cols);
			Matrix<int> optimum_shift_img_y(rows,cols);

			int* optimum_shift_img_x_data=optimum_shift_img_x.row(0);
			int* optimum_shift_img_y_data=optimum_shift_img_y.row(0);

			int* shift_x_data=shift_x.row(0);
			int* shift_y_data=shift_y.row(0);

			for (int i=0;i<rows;++i)
			{
				for (int j=0;j<cols;++j)
				{
					int p=i*cols+j;
					optimum_shift_img_y_data[p]=shift_y_data[p];
					optimum_shift_img_x_data[p]=shift_x_data[shift_y_data[p]*cols+j];
				}
			}

			m_optimum_shift_x_pyramid[index]=optimum_shift_img_x;
			m_optimum_shift_y_pyramid[index]=optimum_shift_img_y;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
static inline int square(int x) { return x*x; }
inline void dthelper(const float* src,float* dst,int* ptr,int step,int s1,int s2,int d1,int d2,float a,float b)
{
	if (d2 >= d1) 
	{
		int d = (d1+d2) >> 1;
		int s = s1;

		for (int p = s1+1; p <= s2; p++)
		{
			if (src[s*step] - a*square(d-s) - b*(abs(d-s)) < 
				src[p*step] - a*square(d-p) - b*(abs(d-p)))
				s = p;
		}

		dst[d*step] = src[s*step] - a*square(d-s) - b*(abs(d-s));
		ptr[d*step] = s;
		dthelper(src, dst, ptr, step, s1, s, d1, d-1, a, b);
		dthelper(src, dst, ptr, step, s, s2, d+1, d2, a, b);
	}
}

void ObjectDetDeformationRule::dt1d(const float* src,float* dst,int* ptr,int step,int n,float a,float b)
{
	dthelper(src, dst, ptr, step, 0, n-1, 0, n-1, a, b);
}
}
