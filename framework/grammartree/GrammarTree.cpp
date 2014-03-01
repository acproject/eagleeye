#include "GrammarTree.h"
#include "LinkRule.h"
#include "Symbol.h"
#include "GrammarUnit.h"
#include "AnyUnit.h"
#include "UnitManager.h"
#include "EagleeyeIO.h"

namespace eagleeye
{
GrammarTree::GrammarTree(const char* gt_name,const char* model_folder,Symbol* root)
:AnyUnit(gt_name)
{
	m_gt_root = root;
	m_gt_name = gt_name;
	GrammarUnit::setModelFolder(model_folder);
	m_class_num = 2;

	setConfigFile((GrammarUnit::getModelFolder() + gt_name + "_config.xml").c_str());
}

GrammarTree::~GrammarTree()
{
	//destroy the whole tree
	if (m_gt_root)
	{
		m_gt_root->destroytModelRes();
		delete m_gt_root;
	}

	//clear some memory used by IO
	EagleeyeIO::destroyIOMemRes();
}

void GrammarTree::initialize()
{
	//load grammar tree structure
	loadGrammarTreeStructure();

	//analyze grammar tree structure
	analyzeGrammarTreeStructureInfo();

	//load additional parameters
	loadUnitConfig();

	//initialize all units
	m_gt_root->initialize();
}

void GrammarTree::analyzeGrammarTreeStructureInfo()
{
	//clear old grammar tree info
	m_gt_info = GrammarTreeStructureInfo();

	int sub_tree_num = getSubTreeNum();
	for (int sub_tree_index = 0; sub_tree_index < sub_tree_num; ++sub_tree_index)
	{
		std::vector<UnitStructureInfo> info = collectSubTreeStructureInfo(sub_tree_index);
		m_gt_info.subtrees_size.push_back(info.size());

		std::map<std::string,UnitStructureInfo> info_map;
		for (unsigned int unit_index = 0; unit_index < info.size(); ++unit_index)
		{
			std::string unit_name = info[unit_index].unit_name;
			info_map[unit_name] = info[unit_index];
		}

		//sort all units in this subtree by alphabetical order
		std::vector<std::string> order_name;
		for (unsigned int unit_index = 0; unit_index < info.size(); ++unit_index)
		{
			order_name.push_back(info[unit_index].unit_name);
		}
		std::sort(order_name.begin(),order_name.end());

		//construct grammar tree info
		for (unsigned int unit_index = 0; unit_index < info.size(); ++unit_index)
		{
			std::string unit_name = order_name[unit_index];
			m_gt_info.units_name.push_back(unit_name);
			m_gt_info.units_size.push_back(info_map[unit_name].unit_size);
			m_gt_info.units_regmult.push_back(info_map[unit_name].unit_regmult);
			m_gt_info.units_learnmult.push_back(info_map[unit_name].unit_learnmult);
		}
	}

	//compute unit offset
	int units_num = m_gt_info.units_size.size();
	m_gt_info.units_offset.resize(units_num,0);
	for (int unit_index = 1; unit_index < units_num; ++unit_index)
	{
		m_gt_info.units_offset[unit_index] += (m_gt_info.units_size[unit_index - 1] + m_gt_info.units_offset[unit_index - 1]);
	}

	//compute subtree offset
	m_gt_info.subtrees_offset.resize(sub_tree_num,0);
	for (int sub_tree_index = 1; sub_tree_index < sub_tree_num; ++sub_tree_index)
	{
		m_gt_info.subtrees_offset[sub_tree_index] += (m_gt_info.subtrees_size[sub_tree_index - 1] + m_gt_info.subtrees_offset[sub_tree_index - 1]);
	}

	m_gt_feature_size = 0;
	for (unsigned int i = 0; i < m_gt_info.units_size.size(); ++i)
	{
		m_gt_feature_size += m_gt_info.units_size[i];
	}
}

void GrammarTree::saveGrammarTreeStructure()
{
	if (m_gt_root)
	{
		EagleeyeIO io;
		io.createWriteHandle(GrammarUnit::getModelFolder() + m_gt_name + "_model.bin",false,WRITE_BINARY_MODE);
		
		//write the total units number of this GrammarTree
		int units_num = m_gt_info.units_size.size() + 1;
		io.write(units_num);
		m_gt_root->saveModelStructure(io);
		io.destroyHandle();
	}

	//save some additional parameters
	saveUnitConfig();
}

void GrammarTree::loadGrammarTreeStructure()
{
	EagleeyeIO io;
	io.createReadHandle(GrammarUnit::getModelFolder() + m_gt_name + "_model.bin",READ_BINARY_MODE);

	int units_num = 0;
	io.read(units_num);

	std::map<std::string,std::vector<std::string>> links_relations;
	std::map<std::string,AnyUnit*> units_relations;
	std::map<std::string,MemoryBlock> units_para;

	//read model file
	for (int unit_index = 0; unit_index < units_num; ++unit_index)
	{
		//read unit name
		std::string unit_name;
		io.read(unit_name);

		//read unit type
		std::string unit_type;
		io.read(unit_type);

		void* block_mem;
		int block_size;
		io.read(block_mem,block_size);
		MemoryBlock unit_para(block_size);
		memcpy(unit_para.block(),block_mem,block_size);

		units_para[unit_name] = unit_para;

		//read the number of links
		int links_num;
		io.read(links_num);

		//read link
		std::vector<std::string> other_units_names;
		for (int link_index = 0; link_index < links_num; ++link_index)
		{
			std::string other_unit_name;
			io.read(other_unit_name);
			other_units_names.push_back(other_unit_name);
		}

		AnyUnit* cur_unit = UnitManager::factory(unit_type.c_str(),unit_name.c_str());
		//find root symbol
		if (unit_index == 0)
		{
			setRootSymbol(dynamic_cast<Symbol*>(cur_unit));
		}

		units_relations[unit_name] = cur_unit;
		links_relations[unit_name] = other_units_names;
	}

	io.destroyHandle();

	//construct link relationship
	std::map<std::string,std::vector<std::string>>::iterator iter,iend(links_relations.end());
	for (iter = links_relations.begin(); iter != iend; ++iter)
	{
		//check whether it's Symbol
		AnyUnit* unit = units_relations[iter->first];
		Symbol* left_symbol_unit = dynamic_cast<Symbol*>(unit);

		if (left_symbol_unit)
		{
			//find all links
			std::vector<std::string> links_pool = iter->second;
			int linkes_num = links_pool.size();
			for (int i = 0; i < linkes_num; ++i)
			{
				std::string link_name = links_pool[i];
				LinkRule* link_unit = dynamic_cast<LinkRule*>(units_relations[link_name]);

				//find all link end node
				std::vector<std::string> units_pool = links_relations[link_name];

				for (unsigned int j = 0; j < units_pool.size(); ++j)
				{
					std::string unit_name = units_pool[j];
					Symbol* right_symbol_unit = dynamic_cast<Symbol*>(units_relations[unit_name]);
					left_symbol_unit->addParsedSymbols(link_unit,right_symbol_unit);
				}
			}
		}
	}

	//assign model para
	m_gt_root->assignModelPara(units_para);
}

void GrammarTree::saveGrammarTreeWeight(const Matrix<float>& gt_weight)
{
	//if there existed weight file, loading weight data firstly
	EagleeyeIO load_weight_io;
	load_weight_io.createReadHandle(GrammarUnit::getModelFolder() + m_gt_name + std::string("_modelcoe.bin"),READ_BINARY_MODE);
	std::map<std::string,Matrix<float>> weight_map;

	Matrix<float> unit_weight;
	std::string unit_name;
	while(load_weight_io.read(unit_weight,unit_name))
	{
		weight_map[unit_name] = unit_weight;
	}
	load_weight_io.destroyHandle();

	//split weight matrix to unit weight
	int units_num = m_gt_info.units_name.size();
	const float* gt_weight_data = gt_weight.row(0);
	for (int unit_index = 0; unit_index < units_num; ++unit_index)
	{
		if (m_gt_info.units_size[unit_index] > 0)
		{
			int offset = m_gt_info.units_offset[unit_index];
			int size = m_gt_info.units_size[unit_index];
			unit_weight = Matrix<float>(1,size,float(0.0f));
			memcpy(unit_weight.dataptr(),gt_weight_data + offset,sizeof(float) * size);	

			weight_map[m_gt_info.units_name[unit_index]] = unit_weight;
		}
	}

	EagleeyeIO write_weight_io;
	write_weight_io.createWriteHandle(GrammarUnit::getModelFolder() + m_gt_name + std::string("_modelcoe.bin"),false,WRITE_BINARY_MODE);
	std::map<std::string,Matrix<float>>::iterator iter,iend(weight_map.end());
	for (iter = weight_map.begin(); iter != iend; ++iter)
	{
		write_weight_io.write(iter->second,iter->first);
	}
	write_weight_io.destroyHandle();
}

void GrammarTree::loadGrammarTreeWeight()
{
	if (m_gt_root)
	{
		EagleeyeIO io;
		io.createReadHandle(GrammarUnit::getModelFolder() + m_gt_name + std::string("_modelcoe.bin"),READ_BINARY_MODE);
		std::map<std::string,Matrix<float>> weight_map;

		Matrix<float> unit_weight;
		std::string unit_name;
		while(io.read(unit_weight,unit_name))
		{
			weight_map[unit_name] = unit_weight;
		}
		io.destroyHandle();

		m_gt_root->assignModelWeight(weight_map);
	}
}

Matrix<float> GrammarTree::reOrganizeGrammarTreeFeature(SearchSpace search_space)
{
	if (m_gt_root)
	{
		Matrix<float> model_feature(1,m_gt_feature_size);
		int cur_index = 0;

		int sub_tree_num = getSubTreeNum();
		for (int sub_tree_index = 0; sub_tree_index < sub_tree_num; ++sub_tree_index)
		{
			std::vector<UnitFeatureInfo> info = collectSubTreeFeatureInfo(sub_tree_index,search_space);

			std::map<std::string,UnitFeatureInfo> unit_info_map;
			for (unsigned int unit_index = 0; unit_index < info.size(); ++unit_index)
			{
				std::string unit_name = info[unit_index].unit_name;
				unit_info_map[unit_name] = info[unit_index];
			}

			std::vector<std::string> order_name;
			for (unsigned int unit_index = 0; unit_index < info.size(); ++unit_index)
			{
				order_name.push_back(info[unit_index].unit_name);
			}
			//sort all units in this GrammarTree model by alphabetical order
			std::sort(order_name.begin(),order_name.end());

			//re-organize features of all units
			for (unsigned int unit_index = 0; unit_index < info.size(); ++unit_index)
			{
				std::string unit_name = order_name[unit_index];

				Matrix<float> unit_feature = unit_info_map[unit_name].unit_feature;	
				switch(search_space)
				{
				case PIXEL_SPACE:
					{
						int size = unit_feature.cols() - PIXEL_FEATURE_HEAD_SIZE;
						memcpy(model_feature.dataptr() + cur_index,unit_feature.dataptr() + PIXEL_FEATURE_DATA_OFFSET,sizeof(float) * size);
						cur_index += size;
						break;
					}
				case RECT_WINDOW_SPACE:
					{
						int size = unit_feature.cols() - RECT_WINDOW_FEATURE_HEAD_SIZE;
						memcpy(model_feature.dataptr() + cur_index,unit_feature.dataptr() + RECT_WINDOW_FEATURE_DATA_OFFSET,sizeof(float) * size);
						cur_index += size;
						break;
					}
				default:
					{
						return Matrix<float>();
					}
				}
			}
		}

		return model_feature;
	}

	return Matrix<float>();
}

void GrammarTree::combineGrammarTreeWeightFiles(std::vector<std::string> seperate_files,std::string to_file)
{
	int seperate_files_num = seperate_files.size();
	std::map<std::string,Matrix<float>> coe_map;

	for (int seperate_index = 0; seperate_index < seperate_files_num; ++seperate_index)
	{
		std::string coe_file = seperate_files[seperate_index];

		EagleeyeIO from_io;
		from_io.createReadHandle(coe_file,READ_BINARY_MODE);
		Matrix<float> unit_coe;
		std::string unit_name;
		while(from_io.read(unit_coe,unit_name))
		{
			coe_map[unit_name] = unit_coe;
		}
		from_io.destroyHandle();
	}

	EagleeyeIO to_io;
	to_io.createWriteHandle(to_file,false,WRITE_BINARY_MODE);
	std::map<std::string,Matrix<float>>::iterator iter,iend(coe_map.end());
	for (iter = coe_map.begin(); iter != iend; ++iter)
	{
		to_io.write(iter->second,iter->first);
	}
	to_io.destroyHandle();
}

void GrammarTree::setRootSymbol(Symbol* root)
{
	m_gt_root = root;
}
Symbol* GrammarTree::getRootSymbol()
{
	return m_gt_root;
}

GrammarTreeStructureInfo GrammarTree::getGrammarTreeStructureInfo()
{
	return m_gt_info;
}
Matrix<float> GrammarTree::getGrammarTreeWeight()
{
	if (m_gt_root)
	{
		Matrix<float> model_weight(1,m_gt_feature_size);
		int cur_index = 0;

		int sub_tree_num = getSubTreeNum();
		for (int sub_tree_index = 0; sub_tree_index < sub_tree_num; ++sub_tree_index)
		{
			std::vector<UnitWeightInfo> info = collectSubTreeWeightInfo(sub_tree_index);

			std::map<std::string,UnitWeightInfo> unit_info_map;
			for (unsigned int unit_index = 0; unit_index < info.size(); ++unit_index)
			{
				std::string unit_name = info[unit_index].unit_name;
				unit_info_map[unit_name] = info[unit_index];
			}

			std::vector<std::string> order_name;
			for (unsigned int unit_index = 0; unit_index < info.size(); ++unit_index)
			{
				order_name.push_back(info[unit_index].unit_name);
			}
			//sort all units in this GrammarTree model by alphabetical order
			std::sort(order_name.begin(),order_name.end());

			//re-organize weight of all units
			for (unsigned int unit_index = 0; unit_index < info.size(); ++unit_index)
			{
				std::string unit_name = order_name[unit_index];

				Matrix<float> unit_weight = unit_info_map[unit_name].unit_weight;	
				memcpy(model_weight.dataptr() + cur_index,unit_weight.dataptr(),sizeof(float) * unit_weight.cols());
				cur_index += unit_weight.cols();
			}
		}

		return model_weight;
	}

	return Matrix<float>();
}

void GrammarTree::learn(const char* parse_file)
{
	if (m_gt_root)
	{
		//if some symbols possess their own learn plan, 
		//notify them to start learning
		m_gt_root->learn(parse_file);

		//using grammar tree learner to learn this whole tree
		//do something
	}
}

void GrammarTree::disassembleGrammarTree()
{
	if (m_gt_root)
	{
		m_gt_root->disassembleModel();
		m_gt_root = NULL;
	}
}

int GrammarTree::getSubTreeNum()
{
	return m_gt_root->getLinkRulesSize();
}

std::vector<UnitStructureInfo> GrammarTree::collectSubTreeStructureInfo(int index)
{
	LinkRule* sub_tree = m_gt_root->getLinkRule(index);;
	std::vector<UnitStructureInfo> info;
	sub_tree->collectModelStructureInfo(info);

	return info;
}

std::vector<UnitFeatureInfo> GrammarTree::collectSubTreeFeatureInfo(int index,SearchSpace search_space)
{
	LinkRule* sub_tree = m_gt_root->getLinkRule(index);
	std::vector<UnitFeatureInfo> info;
	sub_tree->collectModelFeatureInfo(info,search_space);

	return info;
}

std::vector<UnitWeightInfo> GrammarTree::collectSubTreeWeightInfo(int index)
{
	LinkRule* sub_tree = m_gt_root->getLinkRule(index);
	std::vector<UnitWeightInfo> info;
	sub_tree->collectModelWeightInfo(info);

	return info;
}

}
