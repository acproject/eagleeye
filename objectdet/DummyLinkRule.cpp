#include "DummyLinkRule.h"

namespace eagleeye
{
DummyLinkRule::DummyLinkRule(const char* name):LinkRule(name)
{

}
DummyLinkRule::~DummyLinkRule()
{

}

void DummyLinkRule::findModelLatentInfo(void* info)
{
	std::vector<Symbol*>::iterator iter,iend(m_linked_symbols.end());
	for (iter = m_linked_symbols.begin(); iter != iend; ++iter)
	{
		(*iter)->findModelLatentInfo(info);
	}
}
void* DummyLinkRule::getParseScore(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat )
{
	return m_linked_symbols[0]->getSymbolScore(search_space,search_mode,pos_mat);
}

}