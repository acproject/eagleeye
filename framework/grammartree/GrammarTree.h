#ifndef _GRAMMARTREE_H_
#define _GRAMMARTREE_H_

#include "EagleeyeMacro.h"

#include "Matrix.h"
#include "AnyUnit.h"
#include "GrammarUnit.h"
#include <string>
#include <map>
#include <vector>

namespace eagleeye
{
class LinkRule;
class Symbol;
class GrammarUnit;

class EAGLEEYE_API GrammarTree:public AnyUnit
{
public:
	/**
	 *	@brief define some basic type
	 *	@note you must do these
	 */
	typedef GrammarTree									Self;
	typedef AnyUnit										Superclass;

	GrammarTree(const char* grammar_tree_name,const char* model_folder,Symbol* root = NULL);
	virtual ~GrammarTree();

	/**
	 *	@brief set/get the root symbol of this grammar tree
	 */
	virtual void setRootSymbol(Symbol* root);
	Symbol* getRootSymbol();

	/**
	 *	@brief (first)initialize the whole grammar tree
	 *	@note 3 steps \n
	 *	(1) load grammar tree structure \n
	 *	(2) analyze grammar tree structure info \n
	 *	(3) all nodes in this tree initialize
	 */
	virtual void initialize();

	/**
	 *	@brief (second)parse the data
	 *	@note Calling this function after initializing grammar tree
	 */
	virtual void parseData(void* data,int width,int height,void* auxiliary_data = NULL) = 0;

	/**
	 *	@brief save/get grammar tree model
	 *	@note save grammar tree model structure
	 */
	virtual void saveGrammarTreeStructure();
	GrammarTreeStructureInfo getGrammarTreeStructureInfo();

	/**
	 *	@brief save/get grammar tree model weight
	 */
	void saveGrammarTreeWeight(const Matrix<float>& gt_coe);
	Matrix<float> getGrammarTreeWeight();

	/**
	 *	@brief combine some separate files
	 */
	static void combineGrammarTreeWeightFiles(std::vector<std::string> seperate_files,std::string to_file);

	/**
	 *	@brief train object classify grammar tree
	 */
	virtual void learn(const char* parse_file);

	/**
	 *	@brief disassemble the whole tree
	 */
	void disassembleGrammarTree();

protected:		
	/**
	 * @brief get the struct info about this grammar tree
	 * @note it would be called automatically in "initialize"
	 */
	void analyzeGrammarTreeStructureInfo();

	/**
	 * @brief load grammar tree from model file	
	 */
	virtual void loadGrammarTreeStructure();
	
	/**
	 * @brief load grammar tree weight from weight file
	 */
	void loadGrammarTreeWeight();

	/**
	 *	@brief build one feature vector
	 */
	Matrix<float> reOrganizeGrammarTreeFeature(SearchSpace search_space);

	/**
	 *	@brief Get the number of subtrees
	 */
	int getSubTreeNum();

	/**
	 *	@brief Get subtree structure info;
	 */
	std::vector<UnitStructureInfo> collectSubTreeStructureInfo(int index);

	/**
	 *	@brief Get subtree feature info
	 */
	std::vector<UnitFeatureInfo> collectSubTreeFeatureInfo(int index,SearchSpace search_space);

	/**
	 *	@brief Get subtree weight info
	 */
	std::vector<UnitWeightInfo> collectSubTreeWeightInfo(int index);

	Symbol* m_gt_root;
	std::string m_gt_name;

	GrammarTreeStructureInfo m_gt_info;			
	
	int m_gt_feature_size;						/**< weight size*/
	int m_class_num;
};
}


#endif