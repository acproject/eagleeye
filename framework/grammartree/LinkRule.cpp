#include "LinkRule.h"

namespace eagleeye
{
LinkRule::LinkRule(const char* name)
	:GrammarUnit(name)
{
	m_upper_symbol = NULL;
	m_once_flag = false;
}

LinkRule::~LinkRule()
{

}

void LinkRule::addLinkedSymbol(Symbol* s)
{
	m_linked_symbols.push_back(s);
}

void LinkRule::setUpperSymbol(Symbol* s)
{
	m_upper_symbol = s;
}

void LinkRule::saveUnitStructure(EagleeyeIO& io)
{
	//record this linkrule name
	io.write(m_unit_name);

	//record this linkrule type
	io.write(getClassIdentity());

	//record some necessary parameter info of this LinkRule
	//not including ParseWeight
	MemoryBlock param_block;
	getUnitPara(param_block);
	io.write(param_block.block(),param_block.blockSize());

	//record the number of links
	io.write(m_linked_symbols.size());

	//record symbol
	std::vector<Symbol*>::iterator symbol_iter,symbol_iend(m_linked_symbols.end());
	for (symbol_iter = m_linked_symbols.begin(); symbol_iter != symbol_iend; ++symbol_iter)
	{
		io.write((*symbol_iter)->getUnitName());
	}
}

void LinkRule::initialize()
{
	std::vector<Symbol*>::iterator iter,iend(m_linked_symbols.end());
	for (iter = m_linked_symbols.begin(); iter != iend; ++iter)
	{
		(*iter)->initialize();
	}

	initializeUnit();
}

void LinkRule::saveModelStructure(EagleeyeIO& io)
{
	saveUnitStructure(io);

	std::vector<Symbol*>::iterator iter,iend(m_linked_symbols.end());
	for (iter = m_linked_symbols.begin(); iter != iend; ++iter)
	{
		(*iter)->saveModelStructure(io);
	}	
}

void LinkRule::collectModelStructureInfo(std::vector<UnitStructureInfo>& info)
{
	UnitStructureInfo unit_info;
	unit_info.unit_name = m_unit_name;
	unit_info.unit_regmult = m_unit_reg_mult;
	unit_info.unit_learnmult = m_unit_learn_mult;
	unit_info.unit_size = getUnitWeight().cols();

	info.push_back(unit_info);

	std::vector<Symbol*>::iterator iter,iend(m_linked_symbols.end());
	for (iter = m_linked_symbols.begin();iter != iend; ++iter)
	{
		(*iter)->collectModelStructureInfo(info);
	}
}

void LinkRule::collectModelFeatureInfo(std::vector<UnitFeatureInfo>& info,SearchSpace search_space)
{
	UnitFeatureInfo unit_info;
	unit_info.unit_name = m_unit_name;
	unit_info.unit_feature = getUnitFeature(search_space,OPTIMUM_SEARCH,Matrix<float>());

	info.push_back(unit_info);

	std::vector<Symbol*>::iterator iter,iend(m_linked_symbols.end());
	for (iter = m_linked_symbols.begin(); iter != iend; ++iter)
	{
		(*iter)->collectModelFeatureInfo(info,search_space);
	}
}

void LinkRule::collectModelWeightInfo(std::vector<UnitWeightInfo>& weight_info)
{
	UnitWeightInfo unit_info;
	unit_info.unit_name = m_unit_name;
	unit_info.unit_weight = getUnitWeight();

	weight_info.push_back(unit_info);

	std::vector<Symbol*>::iterator iter,iend(m_linked_symbols.end());
	for (iter = m_linked_symbols.begin(); iter != iend; ++iter)
	{
		(*iter)->collectModelWeightInfo(weight_info);
	}
}

void LinkRule::setModelData(void* data,SampleState sample_state,void* auxiliary_data)
{
	setUnitData(data,sample_state,auxiliary_data);
	m_data_update_flag = true;

	std::vector<Symbol*>::iterator iter,iend(m_linked_symbols.end());
	for (iter = m_linked_symbols.begin(); iter != iend; ++iter)
	{
		(*iter)->setModelData(data,sample_state,auxiliary_data);
	}
}

void LinkRule::getModelOutput(std::vector<void*>& output_info)
{
	std::vector<Symbol*>::iterator iter,iend(m_linked_symbols.end());
	for (iter = m_linked_symbols.begin(); iter != iend; ++iter)
	{
		(*iter)->getModelOutput(output_info);
	}
}

void LinkRule::destroytModelRes()
{
	std::vector<Symbol*>::iterator iter,iend(m_linked_symbols.end());
	for (iter = m_linked_symbols.begin(); iter != iend; ++iter)
	{
		(*iter)->destroytModelRes();
		delete(*iter);
	}
}

void LinkRule::disassembleModel()
{
	std::vector<Symbol*>::iterator iter,iend(m_linked_symbols.end());
	for (iter = m_linked_symbols.begin(); iter != iend; ++iter)
	{
		(*iter)->disassembleModel();
	}

	m_linked_symbols.clear();
	m_upper_symbol = NULL;
}

GrammarUnit* LinkRule::isme(std::string unit_name)
{
	if (unit_name == m_unit_name)
	{
		return this;
	}

	std::vector<Symbol*>::iterator iter,iend(m_linked_symbols.end());
	for (iter = m_linked_symbols.begin(); iter != iend; ++iter)
	{
		GrammarUnit* me = (*iter)->isme(unit_name);
		if (me)
		{
			return me;
		}
	}

	return NULL;
}

void LinkRule::assignModelWeight(const std::map<std::string, Matrix<float>>& weight_map)
{
	std::map<std::string, Matrix<float>>::const_iterator iter_checker,iter_end(weight_map.end());
	iter_checker = weight_map.find(m_unit_name);
	if (iter_checker != iter_end)
	{
		setUnitWeight(iter_checker->second);
	}

	std::vector<Symbol*>::iterator iter,iend(m_linked_symbols.end());
	for (iter = m_linked_symbols.begin(); iter != iend; ++iter)
	{
		(*iter)->assignModelWeight(weight_map);
	}	
}

void LinkRule::assignModelPara(const std::map<std::string,MemoryBlock>& para_map)
{
	std::map<std::string,MemoryBlock>::const_iterator iter_checker,iter_end(para_map.end());
	iter_checker = para_map.find(m_unit_name);
	if (iter_checker != iter_end)
	{
		setUnitPara(iter_checker->second);
	}

	std::vector<Symbol*>::iterator iter,iend(m_linked_symbols.end());
	for (iter = m_linked_symbols.begin(); iter != iend; ++iter)
	{
		(*iter)->assignModelPara(para_map);
	}	
}

void LinkRule::enableSubTree()
{
	setModelState(GRAMMAR_UNIT_ACTIVE);
}

void LinkRule::disableSubTree()
{
	setModelState(GRAMMAR_UNIT_PASSIVE);
}

void LinkRule::setModelState(GrammarUnitState state)
{
	setUnitState(state);

	std::vector<Symbol*>::iterator iter,iend(m_linked_symbols.end());
	for (iter = m_linked_symbols.begin(); iter != iend; ++iter)
	{
		(*iter)->setModelState(state);
	}
}

void LinkRule::learn(const char* samples_file)
{
	std::vector<Symbol*>::iterator iter,iend(m_linked_symbols.end());
	for (iter = m_linked_symbols.begin(); iter != iend; ++iter)
	{
		(*iter)->learn(samples_file);
	}

	//learn all unkonw info of this unit
	if (!m_once_flag)
	{
		learnUnit(samples_file);
		m_once_flag = true;
	}
}

}
