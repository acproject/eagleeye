#ifndef _RULE_H_
#define _RULE_H_

#include "EagleeyeMacro.h"

#include "Symbol.h"
#include <vector>
#include "Matrix.h"
#include "Array.h"
#include "GrammarUnit.h"
#include <string>

namespace eagleeye
{
using namespace eagleeye;
class EAGLEEYE_API LinkRule:public GrammarUnit
{
	friend class Symbol;
public:
	LinkRule(const char* name = "LinkRule");
	~LinkRule();

	EAGLEEYE_CLASSIDENTITY(LinkRule);

	/**
	 *	@brief get the response score combining all symbols at some predefined positions
	 */
	virtual void* getParseScore(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat = Matrix<float>()){return NULL;};
	
	/**
	 *	@brief save the whole model structure
	 *	@note The model coefficients comes from "Learning Framework"
	 */
	virtual void saveModelStructure(EagleeyeIO& io);

	/**
	 *	@brief Traverse the whole tree, every unit would select its unit
	 *	weight from weight_map indexed by unit name
	 */
	virtual void assignModelWeight(const std::map<std::string, Matrix<float>>& weight_map);

	/**
	 *	@brief Traverse the whole tree, every unit would select its unit
	 *	parameters from para_map indexed by unit name
	 */
	virtual void assignModelPara(const std::map<std::string,MemoryBlock>& para_map);

	/**
	 *	@brief Traverse the whole tree to fill LinkRule Configure Info
	 */
	virtual void collectModelStructureInfo(std::vector<UnitStructureInfo>& info);

	/**
	 *	@brief Traverse the whole tree to fill LinkRule Data Info
	 */
	virtual void collectModelFeatureInfo(std::vector<UnitFeatureInfo>& info,SearchSpace search_space);

	/**
	 *	@brief Traverse the whole tree to collect every unit weight info
	 */
	virtual void collectModelWeightInfo(std::vector<UnitWeightInfo>& weight_info);

	/**
	 *	@brief Traverse the whole tree, every unit would destroy its unit 
	 *	resource.
	 */
	virtual void destroytModelRes();

	/**
	 *	@brief Traverse the whole tree and clear the link between units.
	 */
	virtual void disassembleModel();

	/**
	 *	@brief Traverse the whole tree and call initializeUnit respectively
	 *	@note first step
	 */
	virtual void initialize();

	/**
	 *	@brief Traverse the whole tree and call setUnitData.
	 *	@note second step
	 */
	virtual void setModelData(void* data,SampleState sample_state,void* auxiliary_data =NULL);

	/**
	 *	@brief Get the output of the whole tree
	 *	@note sixth step
	 */
	virtual void getModelOutput(std::vector<void*>& output_info);

	/**
	 *	@brief (core)learn all unkown info of this model
	 */
	virtual void learn(const char* samples_file);

	/**
	 *	@brief Enable/Disable all linked symbols
	 */
	void enableSubTree();
	void disableSubTree();

	/**
	 *	@brief Traverse the whole tree and set every unit state
	 */
	void setModelState(GrammarUnitState state);

	/**
	 *	@brief is me
	 */
	GrammarUnit* isme(std::string unit_name);

protected:
	/**
	 *	@brief Save coefficients of this unit
	 *	@note This function would be called implicitly.
	 *	This unit coefficients comes from "Learning Framework"
	 */
	virtual void saveUnitStructure(EagleeyeIO& io);

	/**
	 *	@brief add the upper symbol attached by this rule
	 *	@note This function would be called by 'addParsedSymbols' 
	 *	in the Symbol, implicitly.
	 */
	void setUpperSymbol(Symbol* s);

	/**
	 *	@brief add symbols charged by this link rule
	 *	@note This function would be called by 'addParsedSymbols' 
	 *	in the Symbol, implicitly.
	 */
	virtual void addLinkedSymbol(Symbol* s);

	std::vector<Symbol*> m_linked_symbols;	/**< Symbols linked by this rule*/

private:
	Symbol* m_upper_symbol;
	bool m_once_flag;
};
}

#endif