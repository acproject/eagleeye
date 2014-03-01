#ifndef _BOWDICTIONARYOPENCV_H_
#define _BOWDICTIONARYOPENCV_H_

#include "EagleeyeMacro.h"
#include "Learning/dictionary/Dictionary.h"

namespace eagleeye
{
class EAGLEEYE_API BOWDictionaryOpenCV:public Dictionary
{
public:
	BOWDictionaryOpenCV(int dictionary_size);
	~BOWDictionaryOpenCV();

	/**
	 *	@brief train bow dictionary
	 */
	virtual Matrix<float> train();
};

}

#endif