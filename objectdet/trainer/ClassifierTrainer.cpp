#include "ClassifierTrainer.h"

namespace eagleeye
{
ClassifierTrainer::ClassifierTrainer()
{
	//set input port number
	setNumberOfInputSignals(1);

	//set output port property
	setNumberOfOutputSignals(1);
	setOutputPort(makeOutputSignal(),OUTPUT_PORT_CLASSIFIER_INFO);
}
ClassifierTrainer::~ClassifierTrainer()
{

}
}