#include "DictionaryTrainer.h"

namespace eagleeye
{
	DictionaryTrainer::DictionaryTrainer()
	{
		m_dictionary_capacity = 1000;
		m_descriptor = NULL;

		//set output port property
		setNumberOfOutputSignals(1);
		setOutputPort(makeOutputSignal(),OUTPUT_PORT_DIC_FILE_INFO);
	}
	DictionaryTrainer::~DictionaryTrainer()
	{

	}
	void DictionaryTrainer::setDictionaryCapacity(int capacity)
	{
		m_dictionary_capacity = capacity;
	}
	void DictionaryTrainer::getDictionaryCapacity(int& capacity)
	{
		capacity = m_dictionary_capacity;
	}
	void DictionaryTrainer::setDictionaryName(const char* dic_name)
	{
		setTrainerName(dic_name);
	}

	void DictionaryTrainer::setDescriptorExtractor(DescriptorExtractor* descriptor)
	{
		m_descriptor = descriptor;
	}
	DescriptorExtractor* DictionaryTrainer::getDescriptorExtractor()
	{
		return m_descriptor;
	}
}
