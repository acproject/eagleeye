#ifndef _ONLINEDICTIONARY_H_
#define _ONLINEDICTIONARY_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "Learning/dictionary/Dictionary.h"

namespace eagleeye
{
class EAGLEEYE_API OnlineDictionary:public Dictionary
{
public:
	OnlineDictionary(int dictionary_size);
	~OnlineDictionary();
	
	/**
	 *	@brief online train dictionary
	 */
	void trainOnline(const Matrix<float>& element,Matrix<float>& dic,Matrix<int>& dic_counts);

	/**
	 *	@brief train dictionary
	 */
	virtual Matrix<float> train();
};
}

#endif