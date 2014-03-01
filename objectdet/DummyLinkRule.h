#ifndef _DUMMYLINKRULE_H_
#define _DUMMYLINKRULE_H_

#include "EagleeyeMacro.h"
#include "LinkRule.h"

namespace eagleeye
{
/**
 *	@brief dummy link rule, there aren't some valid functions
 */
class EAGLEEYE_API DummyLinkRule:public LinkRule
{
public:
	DummyLinkRule(const char* name = "Dummy");
	virtual ~DummyLinkRule();

	/**
	 *	@brief set/get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(DummyLinkRule);

	/**
	 *	@brief dummy functions
	 */
	virtual void findModelLatentInfo(void* info);
	virtual void* getParseScore(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat /* = Matrix<float>() */);

private:
	DummyLinkRule(const DummyLinkRule&);
	void operator=(const DummyLinkRule&);
};
}

#endif