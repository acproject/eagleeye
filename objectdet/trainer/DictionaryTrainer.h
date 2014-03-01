#ifndef _DICTIONARYTRAINER_H_
#define _DICTIONARYTRAINER_H_

#include "EagleeyeMacro.h"
#include "SignalFactory.h"
#include "SymbolTrainer.h"
#include "DescriptorExtractor.h"
#include "Array.h"
#include "Matrix.h"
#include <string>

namespace eagleeye
{
class EAGLEEYE_API DictionaryTrainer:public SymbolTrainer
{
public:
	DictionaryTrainer();
	virtual ~DictionaryTrainer();

	/**
	 *	@brief define class identity
	 */
	EAGLEEYE_CLASSIDENTITY(DictionaryTrainer);

	/**
	 *	@brief set output port
	 */
	EAGLEEYE_OUTPUT_PORT_TYPE(InfoSignal<std::string>,0,DIC_FILE_INFO);

	/**
	 *	@brief set the dictionary capacity
	 */
	void setDictionaryCapacity(const int capacity);
	void getDictionaryCapacity(int& capacity);

	/**
	 *	@brief set the dictionary name
	 */
	void setDictionaryName(const char* dic_name);

	/**
	 *	@brief set/get descriptor extractor
	 */
	void setDescriptorExtractor(DescriptorExtractor* descriptor);
	DescriptorExtractor* getDescriptorExtractor();

protected:
	DescriptorExtractor* m_descriptor;
	int m_dictionary_capacity;
};
}

#endif