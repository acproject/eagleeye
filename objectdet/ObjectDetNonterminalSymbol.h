#ifndef _OBJECTDETNONTERMINALSYMBOL_H_
#define _OBJECTDETNONTERMINALSYMBOL_H_

#include "EagleeyeMacro.h"

#include "Symbol.h"
#include <vector>
#include "LinkRule.h"
#include "DataPyramid.h"
#include "ObjectDetSymbol.h"
#include <string>

namespace eagleeye
{
/**
 *	@brief using 'max' strategy to combine all linked rule score at all pixel
 *	@note PosIndex PIXEL_SPACE; SelectModel INDEPENDENT_SEARCH
 */
class EAGLEEYE_API ObjectDetNonterminalSymbol:public ObjectDetSymbol
{
public:
	struct _Parameter
	{
		int anchor_x;
		int anchor_y;
		int anchor_l;
	};

	ObjectDetNonterminalSymbol(const char* name);
	~ObjectDetNonterminalSymbol();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(ObjectDetNonterminalSymbol);

	/**
	 *	@brief get score at predefined position
	 */
	virtual void* getSymbolScore(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat = Matrix<float>());

	/**
	 *	@brief find latent info by prior-info
	 *	@param info DetSymboInfo
	 */
	virtual void findModelLatentInfo(void* info);

protected:
	/**
	 *	@brief Set/Get this unit parameter
	 */
	virtual void getUnitPara(MemoryBlock& param_block);
	virtual void setUnitPara(MemoryBlock param_block);

	ScorePyramid m_max_score_pyr;

private:
	ObjectDetNonterminalSymbol(const ObjectDetNonterminalSymbol&);
	void operator=(const ObjectDetNonterminalSymbol&);
};

}

#endif