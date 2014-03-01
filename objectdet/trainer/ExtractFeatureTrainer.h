#ifndef _EXTRACTFEATURETRAINER_H_
#define _EXTRACTFEATURETRAINER_H_

#include "EagleeyeMacro.h"
#include "Trainer/SymbolTrainer.h"
#include "SignalFactory.h"
#include "Symbol.h"
#include "ObjectDetSymbol.h"

#include <string>

namespace eagleeye
{
class EAGLEEYE_API ExtractFeatureTrainer:public SymbolTrainer
{
public:
	typedef ExtractFeatureTrainer				Self;
	typedef SymbolTrainer						Superclass;

	ExtractFeatureTrainer();
	virtual ~ExtractFeatureTrainer();

	/**
	 *	@brief get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(ExtractFeatureTrainer);

	/**
	 *	@brief define output info signal type
	 */
	EAGLEEYE_OUTPUT_PORT_TYPE(InfoSignal<std::string>, 0, SAMPLES_INFO);
	EAGLEEYE_OUTPUT_PORT_TYPE(InfoSignal<std::string>,1,SAMPLES_WHITING_INFO);

	/**
	 *	@brief set superpixel generator
	 */
	void setSuperpixelGenerator(AnyNode* superpixel_generator);

	/**
	 *	@brief set symbol pointer
	 */
	void setHoldSymbol(Symbol* hold_symbol);

	/**
	 *	@brief the concrete code is implemented in this function
	 */
	virtual void train();

protected:
	/**
	 *	@brief check whether it needs to be processed
	 *	@note if the model parameters file have been existed, it wouldn't
	 *	execute again. It would walk into the next node directly.
	 */
	virtual bool isNeedProcessed();
	
	/**
	 *	@brief check whether some preliminary conditions have been
	 *	satisfied.
	 */
	virtual bool selfcheck();

	/**
	 *	@brief parse training samples
	 */
	virtual void parseImageInSuperpixelSpace();

	/**
	 *	@brief after processing this node, we should clear some 
	 *	temporary variables
	 */
	virtual void clearSomething();

private:
	Matrix<float> m_samples_presentation;			/**< samples representation*/
	Matrix<float> m_samples_label;					/**< samples labels*/

	std::string m_trainer_samples_info;
	std::string m_trainer_whiting_info;

	Symbol* m_hold_symbol;							/**< controlled symbol*/

	AnyNode* m_superpixel_generator;
};
}

#endif