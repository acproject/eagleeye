#ifndef _CLASSIFIERTRAINER_H_
#define _CLASSIFIERTRAINER_H_

#include "EagleeyeMacro.h"
#include "SymbolTrainer.h"
#include "SignalFactory.h"
#include "EagleeyeIO.h"

namespace eagleeye
{
class EAGLEEYE_API ClassifierTrainer:public SymbolTrainer
{
public:
	typedef ClassifierTrainer							Self;
	typedef SymbolTrainer								Superclass;

	ClassifierTrainer();
	virtual ~ClassifierTrainer();

	/**
	 *	@brief get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(ClassifierTrainer);

	/**
	 *	@brief define input and output info signal type
	 */
	EAGLEEYE_INPUT_PORT_TYPE(InfoSignal<std::string>,0,SAMPLES_INFO);
	EAGLEEYE_OUTPUT_PORT_TYPE(InfoSignal<std::string>,0,CLASSIFIER_INFO);

private:
	ClassifierTrainer(const ClassifierTrainer&);
	void operator=(const ClassifierTrainer&);
};
}

#endif