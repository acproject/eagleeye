#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include <vector>

namespace eagleeye
{
class EAGLEEYE_API Dictionary
{
public:
	Dictionary(int dictionary_size);
	virtual ~Dictionary();

	void add(const Matrix<float>& descriptors);

	/**
	 *	@brief train dictionary by using all elements
	 */
	virtual Matrix<float> train() = 0;

	/**
	 *	@brief Save/Load dictionary
	 */
	static void saveDictionary(const Matrix<float>& dic, const char* dic_path);
	static Matrix<float> loadDictionary(const char* dic_path);

protected:
	int m_dictionary_capacity;							/**< the dictionary capacity*/
	std::vector<Matrix<float>> m_elements;				/**< content elements*/
};
}


#endif