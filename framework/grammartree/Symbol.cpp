#include "Symbol.h"
#include "LinkRule.h"
#include <algorithm>

namespace eagleeye
{
unsigned char Symbol::m_invalid_label = 255;

void Symbol::addParsedSymbols(LinkRule* rule,Symbol* symbol)
{
	if (m_symbol_type != TERMINAL)
	{
		//set the upper symbol attached by this rule 
		rule->setUpperSymbol(this);

		//link child symbols for this symbol by using this "rule"
		rule->addLinkedSymbol(symbol);
		std::vector<LinkRule*>::iterator iter_checker,iter_end(m_rule_pool.end());

		iter_checker = find(m_rule_pool.begin(),m_rule_pool.end(),rule);

		if (iter_checker == iter_end)
		{
			m_rule_pool.push_back(rule);

			//set the upper link attached by this symbol
			symbol->setUpperLink(rule);
		}
	}
}

void Symbol::setUpperLink(LinkRule* rule)
{
	m_upper_link = rule;
}

void Symbol::saveUnitStructure(EagleeyeIO& io)
{
	//record this symbol name
	io.write(m_unit_name);
	
	//record this symbol type
	io.write(getClassIdentity());

	//record some necessary parameter info of this symbol
	//not including SymbolWeight
	MemoryBlock param_block;
	getUnitPara(param_block);
	io.write(param_block.block(),param_block.blockSize());	

	//record the number of links
	io.write(m_rule_pool.size());

	//record link
	std::vector<LinkRule*>::iterator link_iter,link_iend(m_rule_pool.end());
	for (link_iter = m_rule_pool.begin(); link_iter != link_iend; ++link_iter)
	{
		io.write((*link_iter)->getUnitName());
	}
}

void Symbol::initialize()
{
	//initialize all units from the leaf node.
	std::vector<LinkRule*>::iterator iter,iend(m_rule_pool.end());
	for (iter = m_rule_pool.begin(); iter != iend; ++iter)
	{
		(*iter)->initialize();
	}

	//initialize this unit
	initializeUnit();
}

void Symbol::saveModelStructure(EagleeyeIO& io)
{
	//save this unit structure
	saveUnitStructure(io);

	//traverse the whole tree
	std::vector<LinkRule*>::iterator down_iter,down_iend(m_rule_pool.end());
	for (down_iter = m_rule_pool.begin(); down_iter != down_iend; ++down_iter)
	{
		(*down_iter)->saveModelStructure(io);
	}
}

void Symbol::collectModelStructureInfo(std::vector<UnitStructureInfo>& info)
{
	//add its own structure
	UnitStructureInfo unit_info;
	unit_info.unit_name = m_unit_name;
	unit_info.unit_regmult = m_unit_reg_mult;
	unit_info.unit_learnmult = m_unit_learn_mult;
	unit_info.unit_size = getUnitWeight().cols();
	
	info.push_back(unit_info);

	//traverse the whole tree
	std::vector<LinkRule*>::iterator iter,iend(m_rule_pool.end());
	for (iter = m_rule_pool.begin(); iter != iend; ++iter)
	{
		(*iter)->collectModelStructureInfo(info);
	}
}

void Symbol::collectModelFeatureInfo(std::vector<UnitFeatureInfo>& info,SearchSpace search_space)
{
	//add its own feature info
	UnitFeatureInfo unit_info;
	unit_info.unit_name = m_unit_name;
	unit_info.unit_feature = getUnitFeature(search_space,OPTIMUM_SEARCH,Matrix<float>());

	info.push_back(unit_info);

	//traverse the whole tree
	std::vector<LinkRule*>::iterator iter,iend(m_rule_pool.end());
	for (iter = m_rule_pool.begin(); iter != iend; ++iter)
	{
		(*iter)->collectModelFeatureInfo(info,search_space);
	}
}

void Symbol::collectModelWeightInfo(std::vector<UnitWeightInfo>& weight_info)
{
	//add its onw feature info
	UnitWeightInfo unit_info;
	unit_info.unit_name = m_unit_name;
	unit_info.unit_weight = getUnitWeight();

	weight_info.push_back(unit_info);

	//traverse the whole tree
	std::vector<LinkRule*>::iterator iter,iend(m_rule_pool.end());
	for (iter = m_rule_pool.begin(); iter != iend; ++iter)
	{
		(*iter)->collectModelWeightInfo(weight_info);
	}
}

void Symbol::getModelOutput(std::vector<void*>& output_info)
{
	//get model output from down to up
	//traverse the whole GrammaTree
	std::vector<LinkRule*>::iterator iter,iend(m_rule_pool.end());
	for (iter = m_rule_pool.begin(); iter != iend; ++iter)
	{
		(*iter)->getModelOutput(output_info);
	}

	//get this symbol output
	getSymbolOutput(output_info);
}

void Symbol::setModelData(void* data,SampleState sample_state,void* auxiliary_data/* =NULL */)
{
	//set this symbol sample
	setUnitData(data,sample_state,auxiliary_data);
	m_data_update_flag = true;

	//traverse the whole GrammaTree
	std::vector<LinkRule*>::iterator iter,iend(m_rule_pool.end());
	for (iter = m_rule_pool.begin(); iter != iend; ++iter)
	{
		(*iter)->setModelData(data,sample_state,auxiliary_data);
	}
}

void Symbol::destroytModelRes()
{
	//destroy all units from down to up
	std::vector<LinkRule*>::iterator iter,iend(m_rule_pool.end());
	for (iter = m_rule_pool.begin(); iter != iend; ++iter)
	{
		(*iter)->destroytModelRes();
		delete(*iter);
	}
}

void Symbol::disassembleModel()
{
	std::vector<LinkRule*>::iterator iter,iend(m_rule_pool.end());
	for (iter = m_rule_pool.begin(); iter != iend; ++iter)
	{
		(*iter)->disassembleModel();
	}

	m_rule_pool.clear();
	m_upper_link = NULL;
}

GrammarUnit* Symbol::isme(std::string unit_name)
{
	if (unit_name == m_unit_name)
	{
		return this;
	}

	std::vector<LinkRule*>::iterator iter,iend(m_rule_pool.end());
	for (iter = m_rule_pool.begin(); iter != iend; ++iter)
	{
		GrammarUnit* me = (*iter)->isme(unit_name);
		if (me)
		{
			return me;
		}
	}

	return NULL;
}

void Symbol::assignModelWeight(const std::map<std::string, Matrix<float>>& weight_map)
{
	//assign model coefficients to all units
	std::map<std::string,Matrix<float>>::const_iterator iter_checker,iter_end(weight_map.end());
	iter_checker = weight_map.find(m_unit_name);

	if(iter_checker != iter_end)
	{
		setUnitWeight(iter_checker->second);
	}

	std::vector<LinkRule*>::iterator iter,iend(m_rule_pool.end());
	for (iter = m_rule_pool.begin(); iter != iend; ++iter)
	{
		(*iter)->assignModelWeight(weight_map);
	}
}

void Symbol::assignModelPara(const std::map<std::string,MemoryBlock>& para_map)
{
	//assign model parameters to all units
	std::map<std::string,MemoryBlock>::const_iterator iter_checker,iter_end(para_map.end());
	iter_checker = para_map.find(m_unit_name);

	if(iter_checker != iter_end)
	{
		setUnitPara(iter_checker->second);
	}

	std::vector<LinkRule*>::iterator iter,iend(m_rule_pool.end());
	for (iter = m_rule_pool.begin(); iter != iend; ++iter)
	{
		(*iter)->assignModelPara(para_map);
	}
}

void Symbol::setModelState(GrammarUnitState state)
{
	setUnitState(state);

	std::vector<LinkRule*>::iterator iter,iend(m_rule_pool.end());
	for (iter = m_rule_pool.begin(); iter != iend; ++iter)
	{
		(*iter)->setModelState(state);
	}
}

void Symbol::learn(const char* samples_file)
{
	//learn this unit
	std::vector<LinkRule*>::iterator iter,iend(m_rule_pool.end());
	for (iter = m_rule_pool.begin(); iter != iend; ++iter)
	{
		(*iter)->learn(samples_file);
	}

	//
	if (!m_once_flag)
	{
		//in general, learnUnit would help initialize this unit
		//therefore, it would run only once.
		learnUnit(samples_file);
		m_once_flag = true;
	}
}

void Symbol::setInvalidLabel(unsigned char invalid_label)
{
	m_invalid_label = invalid_label;
}
unsigned char Symbol::getInvalidLabel()
{
	return m_invalid_label;
}

}

