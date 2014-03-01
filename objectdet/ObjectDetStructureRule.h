#ifndef _OBJECTDETSTRUCTURERULE_H_
#define _OBJECTDETSTRUCTURERULE_H_

#include "EagleeyeMacro.h"

#include "LinkRule.h"
#include "Matrix.h"
#include "MatrixMath.h"
#include "DataPyramid.h"
#include "ObjectDetSymbol.h"
#include <string>

namespace eagleeye
{
/**
 *	@brief fixed structure rule,finding fixed offset score
 *	@note it don't provide learning service
 */
class EAGLEEYE_API ObjectDetStructureRule:public LinkRule
{
public:
	ObjectDetStructureRule(const char* name);
	virtual ~ObjectDetStructureRule();

	/**
	 *	@brief Set/Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(ObjectDetStructureRule);

	/**
	 *	@brief Find the latent info by using prior-info
	 *	@note Sometimes, some key info in the rule is latent and
	 *	couldn't be observed directly. At this time, we have to find them
	 *	by some prior-info or some standards. In addition, this function 
	 *	also charges the info flowed in this 'tree pipeline'
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

private:
	ScorePyramid m_structure_rule_score_pyr;
	float m_offset_w;						/**< offset parameter*/

	Matrix<float> m_rule_parse_weight;			/**< LinkRule weight*/
	Matrix<float> m_rule_pase_feature;			/**< LinkRule feature*/
};
}

#endif