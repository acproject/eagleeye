#ifndef _ALGORITHM_H_
#define _ALGORITHM_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "AnyUnit.h"

namespace eagleeye
{
class EAGLEEYE_API Algorithm:public AnyUnit
{
public:
	Algorithm(const char* algorithm_name = "Algorithm");
	virtual ~Algorithm();

	/**
	 *	@brief set class identity
	 */
	EAGLEEYE_CLASSIDENTITY(Algorithm);
};
}


#endif