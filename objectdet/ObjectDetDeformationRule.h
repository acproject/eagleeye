#ifndef _OBJECTDETDEFORMATIONRULE_H_
#define _OBJECTDETDEFORMATIONRULE_H_

#include "EagleeyeMacro.h"

#include "Types.h"
#include "Matrix.h"
#include "DataPyramid.h"
#include "LinkRule.h"
#include "ObjectDetSymbol.h"
#include <string>
#include <vector>

/**
 *	@brief position shifting rule
 */
namespace eagleeye
{
class EAGLEEYE_API ObjectDetDeformationRule:public LinkRule
{
public:
	ObjectDetDeformationRule(const char* name);
	~ObjectDetDeformationRule();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(ObjectDetDeformationRule);

	/**
	 *	@brief Find the latent info by using prior-info.
	 *	@note Sometimes, some key info in the rule is latent and \n
	 *	couldn't be observed directly. At this time, we have to find them \n
	 *	by some prior-info or some standards.For this rule, the latent \n
	 *	variables are dx dy. All of these could be get by \n
	 *	using m_optimum_detecting_r,m_optimum_detecting_c and m_optimum_detecting_level.\n
	 *	In addition, this function would also charges the info flowed in \n
	 *	this 'tree pipeline'
	 */
	virtual void findModelLatentInfo(void* info);

	/**
	 *	@brief Set/Get the rule "property"(weight parameter)
	 *	@note These weight would be gained from "Learning Framework"
	 */
	virtual Matrix<float> getUnitWeight();
	virtual void setUnitWeight(const Matrix<float>& weight);

	/**
	 *	@brief get feature at predefined position
	 */
	virtual Matrix<float> getUnitFeature(SearchSpace pos_index,SearchMode select_mode,Matrix<float> pos_mat = Matrix<float>());

	/**
	 *	@brief get score at predefined position
	 */
	virtual void* getParseScore(SearchSpace pos_index,SearchMode select_mode,Matrix<float> pos_mat = Matrix<float>());

protected:
	/**
	 *	@brief The main idea comes from Pedro's work
	 *	@detail Considering every part could shift, we have introduce 
	 *	distance penalty item. Therefore, we want to get the optimum 
	 *	shift. It adopts "divide and conquer" idea
	 */
	void dt(ScorePyramid pyr);

	void dt1d(const float* src,float* dst,int* ptr,int step,int n,float a,float b);
	
private:
	ScorePyramid m_rule_score_pyr;

	DynamicDataPyramid<int> m_optimum_shift_x_pyramid;
	DynamicDataPyramid<int> m_optimum_shift_y_pyramid;

	//these coefficient would be gained from "Learning Framework"
	float m_def_w_x0;
	float m_def_w_x1;
	float m_def_w_y0;
	float m_def_w_y1;

	int m_optimum_symbol_shift_x_det;	/**< latent variable*/
	int m_optimum_symbol_shift_y_det;	/**< latent variable*/

	Matrix<float> m_rule_parse_weight;			/**< LinkRule weight*/
	Matrix<float> m_rule_pase_feature;			/**< LinkRule feature*/
};
}

#endif