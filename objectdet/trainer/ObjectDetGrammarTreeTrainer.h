#ifndef _OBJECTDETGRAMMARTREETRAINER_H_
#define _OBJECTDETGRAMMARTREETRAINER_H_

#include "EagleeyeMacro.h"
#include "Trainer/SymbolTrainer.h"
#include "SignalFactory.h"
#include "ObjectDetGrammarTree.h"
#include <string>

namespace eagleeye
{
class EAGLEEYE_API ObjectDetGrammarTreeTrainer:public SymbolTrainer
{
public:
	typedef ObjectDetGrammarTreeTrainer								Self;
	typedef SymbolTrainer											Superclass;

	ObjectDetGrammarTreeTrainer(ObjectDetGrammarTree* gt_tree);
	virtual ~ObjectDetGrammarTreeTrainer();

	/**
	 * @brief get class identity	
	*/
	EAGLEEYE_CLASSIDENTITY(ObjectDetGrammarTreeTrainer);

	/**
	 *	@brief set max iterators to find optimum weight
	 */
	void setMaxIterators(int max_iters);

	/**
	 *	@brief training object det grammar tree model coefficients
	 */
	virtual void train();

protected:
	/**
	 *	@brief check whether it needs to be processed
	 */
	virtual bool isNeedProcessed();

	/**
	 *	@brief check whether some preliminary conditions have been satisfied.	
	 */
	virtual bool selfcheck();

	/**
	 *	@brief parse training samples
	 */
	virtual void parseImageWithoutAnnotation();

	/**
	 *	@brief after processing this node, we should clear some temporary variables
	 */
	virtual void clearSomething();

	/**
	 *	@brief finding the optimum threshold for 
	 *	splitting positive and negative training samples
	 */
	void findingOptimumSplitThreshold();

private:
	ObjectDetGrammarTree* m_hold_gt_tree;

	std::string m_trainer_samples_info;
	std::string m_trainer_latent_info;
	std::string m_weight_info;

	int m_training_samples_num;
	int m_training_samples_dim;
	int m_latent_dim;

	int m_training_model_max_iters;
	int m_optimum_split_threshold;
	float m_beyond_detection_rate;

	std::vector<float> m_positive_scores;
	std::vector<float> m_negative_scores;
};
}

#endif