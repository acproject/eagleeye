#ifndef _OBJECTCLASSIFYGRAMMARTREE_H_
#define _OBJECTCLASSIFYGRAMMARTREE_H_

#include "EagleeyeMacro.h"
#include "GrammarTree.h"
#include "GrammarUnit.h"
#include "ObjectDetSymbol.h"
#include "Matrix.h"

namespace eagleeye
{
class EAGLEEYE_API ObjectDetSuperpixelGrammarTree:public GrammarTree
{
public:
	ObjectDetSuperpixelGrammarTree(const char* name,const char* model_folder,ObjectDetSymbol* root = NULL);
	~ObjectDetSuperpixelGrammarTree();

	/**
	 *	@brief parse data
	 */
	virtual void parseData(void* data,int width,int height,void* auxiliary_data);

	/**
	 *	@brief construct object classify grammar tree
	 */
	virtual void setRootSymbol(Symbol* root);

	/**
	 *	@brief get predict label
	 */
	Matrix<float> getPredictLabel(); 

private:
	ObjectDetSymbol* m_root_symbol;
};
}

#endif