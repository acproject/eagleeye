#ifndef _SYMBOL_H_
#define _SYMBOL_H_

#include "EagleeyeMacro.h"

#include "Matrix.h"
#include "Array.h"
#include "GrammarUnit.h"
#include <string>
#include <map>
#include <vector>

namespace eagleeye
{
#define SUPERPIXEL_FEATURE_HEAD_SIZE 4
#define SUPERPIXEL_FEATURE_DATA_OFFSET 4
#define SUPERPIXEL_SCORE_OFFSET 3
#define SUPERPIXEL_FEATURE_DATA(data) ((data + SUPERPIXEL_FEATURE_DATA_OFFSET))
#define SUPERPIXEL_FEATURE_LABEL(data) (data[3])
#define SUPERPIXEL_FEATURE_INDEX(data) (data[0])

#define PIXEL_FEATURE_HEAD_SIZE 5
#define PIXEL_FEATURE_DATA_OFFSET 5
#define PIXEL_FEATURE_DATA(data) ((data + PIXEL_FEATURE_DATA_OFFSET))
#define PIXEL_FEATURE_LABEL(data) (data[4])

#define RECT_WINDOW_FEATURE_HEAD_SIZE 7
#define RECT_WINDOW_FEATURE_DATA_OFFSET 7
#define RECT_WINDOW_FEATURE_DATA(data) ((data + RECT_WINDOW_FEATURE_DATA_OFFSET))
#define RECT_WINDOW_FEATURE_LABEL(data) (data[6])

class LinkRule;
enum SymbolType
{
	UNDEFINED_SYMBOL,
	NON_TERMINAL,
	TERMINAL
};

class EAGLEEYE_API Symbol:public GrammarUnit
{
	friend class LinkRule;

public:
	Symbol(const char* name = "Symbol",SymbolType type = TERMINAL)
		:GrammarUnit(name),
		m_symbol_type(type),
		m_upper_link(NULL){m_once_flag = false;};
	virtual ~Symbol(){};
	
	/**
	 * @brief get score at predefined position
	 */
	virtual void* getSymbolScore(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat = Matrix<float>()){return NULL;};

	/**
	 *	@brief get the type of this symbol
	 */
	SymbolType getSymbolType(){return m_symbol_type;};

	/**
	 *	@brief add parsed symbol with linkrule
	 *	@note only NonTerminal Symbol is allowed to add parsed symbols
	 */
	void addParsedSymbols(LinkRule* rule,Symbol* symbol);

	/**
	 *	@brief Traverse the whole tree and save tree structure
	 *	@note it would call saveUnitStructure
	 */
	virtual void saveModelStructure(EagleeyeIO& io);

	/**
	 *	@brief Traverse the whole tree, every unit would select 
	 *	its unit weight from weight_map
	 */
	virtual void assignModelWeight(const std::map<std::string, Matrix<float>>& weight_map);

	/**
	 *	@brief Traverse the whole tree, every unit would select its unit para
	 *	form para_map
	 */
	virtual void assignModelPara(const std::map<std::string,MemoryBlock>& para_map);

	/**
	 *	@brief Traverse the whole tree to collect Symbol Configure Info
	 */
	virtual void collectModelStructureInfo(std::vector<UnitStructureInfo>& info);

	/**
	 *	@brief Traverse the whole tree to collect symbol feature info
	 */
	virtual void collectModelFeatureInfo(std::vector<UnitFeatureInfo>& info,SearchSpace search_space);

	/**
	 *	@brief Traverse the whole tree to collect symbol weight info
	 */
	virtual void collectModelWeightInfo(std::vector<UnitWeightInfo>& weight_info);

	/**
	 *	@brief Traverse the whole tree,every unit would destroy its 
	 *	resource
	 */
	virtual void destroytModelRes();

	/**
	 *	@brief Traverse the whole tree and clear the link between units
	 */
	virtual void disassembleModel();

	/**
	 *	@brief Traverse the whole tree and initialize every unit
	 *	@note first step
	 */
	virtual void initialize();

	/**
	 *	@brief Traverse the whole tree and call setUnitData.
	 *	@note second step
	 */
	virtual void setModelData(void* data,SampleState sample_state,void* auxiliary_data=NULL);

	/**
	 *	@brief Traverse the whole tree and get all output info
	 *	@note Sixth step
	 */
	virtual void getModelOutput(std::vector<void*>& output_info);

	/**
	 *	@brief Traverse the whole tree and set every unit state
	 */
	virtual void setModelState(GrammarUnitState state);

	/**
	 *	@brief (core)learn all unknown info of this model
	 */
	virtual void learn(const char* parse_file);

	/**
	 *	@brief is me
	 */
	GrammarUnit* isme(std::string unit_name);

	/**
	 *	@brief get link rule
	 */
	LinkRule* getLinkRule(int link_index){return m_rule_pool[link_index];};
	/**
	 *	@brief get link rule number
	 */
	int getLinkRulesSize(){return m_rule_pool.size();};

	/**
	 *	@brief set invalid label
	 */
	static void setInvalidLabel(unsigned char invalid_label);
	static unsigned char getInvalidLabel();

protected:
	/**
	 *	@brief Save this unit coefficients
	 *	@note This coefficient comes from "Learning Framework". 
	 *	This function would called implicitly.
	 */
	virtual void saveUnitStructure(EagleeyeIO& io);

	/**
	 *	@brief set the upper link attached by this symbol
	 *	@note This function would be called by addParsedSymbol implicitly
	 */
	void setUpperLink(LinkRule* rule);

	/**
	 *	@brief get the output of this symbol
	 *	@note By calling this function, we should gain the ultimate goal.
	 *	For object detection, one terminal symbol
	 *	would correspond to one location in the image. Therefore,
	 *	this location would become the output of this function.
	 */
	virtual void getSymbolOutput(std::vector<void*>& output_info){};

	std::vector<LinkRule*> m_rule_pool;
	LinkRule* m_upper_link;

	static unsigned char m_invalid_label;

private:
	SymbolType m_symbol_type;				/**< This symbol type. Terminal or NonTerminal*/
	bool m_once_flag;
};

}

#endif