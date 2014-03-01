#ifndef _OBJECTDETGAMBLESELECT_H_
#define _OBJECTDETGAMBLESELECT_H_

#include "EagleeyeMacro.h"
#include "Symbol.h"
#include <vector>
#include "LinkRule.h"
#include "DataPyramid.h"
#include "ObjectDetSymbol.h"
#include "Learning/MultiArmedBandit.h"
#include <string>
#include <vector>

namespace eagleeye
{
enum GamblingRewardMode
{
	PIXEL_BASED_GAMBLING_REWARD,
	SUPERPIXEL_BASED_GAMBLING_REWARD,
	RECT_REGION_BASED_GAMBLING_REWARD
};
class EAGLEEYE_API ObjectDetGamblerStrategy:public ObjectDetSymbol
{
public:
	ObjectDetGamblerStrategy(const char* name);
	virtual ~ObjectDetGamblerStrategy();

	/**
	 *	@brief get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(ObjectDetGamblerStrategy);

	/**
	 *	@brief set gambling reward mode
	 *	@note gambling reward is different for different position index mode
	 */
	void setGamblingRewardMode(GamblingRewardMode gambling_reward_mode);

	/**
	 *	@brief get symbol score pyramid of this symbol
	 */
	virtual void* getSymbolScore(SearchSpace pos_index,SearchMode select_mode,Matrix<float> pos_mat = Matrix<float>());

	/**
	 *	@brief find latent info by prior-info
	 *	@param info DetSymboInfo
	 */
	virtual void findModelLatentInfo(void* info);

	/**
	 *	@brief using for MAB model
	 */
	void setGamblerGammar(float gammar);

protected:
	/**
	 *	@brief learn gamble strategy
	 */
	virtual void learnUnit(const char* samples_file);

	/**
	 *	@brief initialize this gamble select unit
	 *	@note load model
	 */
	virtual void initializeUnit();

	ScorePyramid m_score_pyr; 
	
	MultiArmedBandit* m_gambler;
	int m_gambler_select;
	
	GamblingRewardMode m_gambling_reward_mode;

private:
	ObjectDetGamblerStrategy(const ObjectDetGamblerStrategy&);
	void operator=(const ObjectDetGamblerStrategy&);
};
}

#endif