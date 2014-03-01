#ifndef _SEMANTICBAGOFWORDSTRAINER_H_
#define _SEMANTICBAGOFWORDSTRAINER_H_

#include "EagleeyeMacro.h"
#include "SignalFactory.h"
#include "ClassifierTrainer.h"
#include "Learning/SemanticBagOfWords.h"
#include "Learning/CrossValidation.h"

namespace eagleeye
{
class EAGLEEYE_API SemanticBagOfWordsTrainer:public ClassifierTrainer
{
public:
	typedef SemanticBagOfWordsTrainer							Self;
	typedef ClassifierTrainer									Superclass;

	SemanticBagOfWordsTrainer();
	virtual ~SemanticBagOfWordsTrainer();

	EAGLEEYE_CLASSIDENTITY(SemanticBagOfWordsTrainer);

	/**
	 *	@brief set cross validation mode
	 */
	void setCrossValidation(bool switch_flag,bool disturb_order = false,CrossValidationMode mode = K_10_FOLDER);

	void setParameters(int class_num,int words_num,int clique_num);

	/**
	 *	@brief training process
	 */
	virtual void train();

protected:
	/**
	 *	@brief check whether it needs to be processed
	 *	@note if the model parameters file have been existed, it wouldn't execute again
	 *	It would walk into the next node directly
	 */
	virtual bool isNeedProcessed();

	/**
	 *	@brief check whether some preliminay conditions have been satisfied
	 */
	virtual bool selfcheck();

private:
	SemanticBagOfWordsTrainer(const SemanticBagOfWordsTrainer&);
	void operator=(const SemanticBagOfWordsTrainer&);

	int m_switch_flag;
	CrossValidationMode m_cross_validation_mode;
	bool m_disturb_order_flag;

	int m_class_num;
	int m_words_num;
	int m_clique_num;
};
}

#endif