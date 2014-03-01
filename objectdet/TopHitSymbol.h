#ifndef _TOPHITSYMBOL_H_
#define _TOPHITSYMBOL_H_

#include "EagleeyeMacro.h"
#include "ObjectDetSymbol.h"
#include "Matrix.h"
#include "LinkRule.h"

namespace eagleeye
{
/**
 *	@brief top hit strategy
 *	@note only support superpixel search space
 */
class EAGLEEYE_API TopHitSymbol:public ObjectDetSymbol
{
public:
	TopHitSymbol(const char* name = "TopHit");
	virtual ~TopHitSymbol();

	/**
	 *	@brief get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(TopHitSymbol);

	/**
	 *	@brief get score at predefined position
	 */
	virtual void* getSymbolScore(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat /* = Matrix<float>() */);

	/**
	 *	@brief find latent info by prior-info
	 *	@param info DetSymbolInfo
	 */
	virtual void findModelLatentInfo(void* info);

	/**
	 *	@brief set top hit k
	 */
	void setTopHitK(const int k);

	/**
	 *	@brief set this unit data
	 *	@note This function would be called by Grammar Tree implicitly
	 *	derived from superclass
	 */
	virtual void  setUnitData(void* data, SampleState sample_state,void* auxiliary_data);

	/**
	 *	@brief get output info
	 */
	virtual void getModelOutput(std::vector<void*>& output_info);

protected:
	virtual void getUnitPara(MemoryBlock& param_block);
	virtual void setUnitPara(MemoryBlock param_block);

	Matrix<float> m_sym_score;

private:
	TopHitSymbol(const TopHitSymbol&);
	void operator=(const TopHitSymbol&);

	int m_k_top_hit;
	std::map<int,int> m_top_hit_link;
	std::map<int,int> m_first_index;
	std::map<int,float> m_first_score;
	int m_level_num;
	Matrix<float> m_k_top_hit_score;		//output predict
};
}

#endif