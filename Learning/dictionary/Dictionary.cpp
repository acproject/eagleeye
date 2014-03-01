#include "Dictionary.h"
#include "EagleeyeIO.h"

namespace eagleeye
{
Dictionary::Dictionary(int dictionary_size)
{
	m_dictionary_capacity=dictionary_size;
}

Dictionary::~Dictionary()
{

}

void Dictionary::add(const Matrix<float>& descriptors)
{
	m_elements.push_back(descriptors);
}

void Dictionary::saveDictionary(const Matrix<float>& dic,const char* dic_path)
{
	EagleeyeIO::write(dic,dic_path,WRITE_BINARY_MODE);
}

Matrix<float> Dictionary::loadDictionary(const char* dic_path)
{
	Matrix<float> dic;
	EagleeyeIO::read(dic,dic_path,READ_BINARY_MODE);
	return dic;
}

}