#ifndef _AUTOMAKEOBJECTDETTERMINALSYMBOLS_H_
#define _AUTOMAKEOBJECTDETTERMINALSYMBOLS_H_

#include "EagleeyeMacro.h"

#include "Matrix.h"
#include "Symbol.h"
#include "ObjectDetTerminalSymbol.h"
#include "Array.h"
#include <vector>
#include <map>
#include <string>

namespace eagleeye
{
class EAGLEEYE_API AutoMakeObjectDetTerminalSymbolsTrainer
{
public:
	AutoMakeObjectDetTerminalSymbolsTrainer();
	~AutoMakeObjectDetTerminalSymbolsTrainer();

	/**
	 *	@brief get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(AutoMakeObjectDetTerminalSymbolsTrainer);

	/**
	 *	@brief set terminal symbols number
	 */
	void setObjectDetTerminalSymbolsNum(int num);

	/**
	 *	@brief learn how to customize terminal symbols
	 */
	void learn(const char* model_folder,const char* parse_file);

	/**
	 *	@brief get object det terminal symbol
	 */
	ObjectDetTerminalSymbol* getObjectDetTerminalSymbol(int index);

protected:
	/**
	 *	@brief according to regions size, split samples automatically
	 */
	void autoSplit(std::map<std::string,std::vector<Array<int,4>>> samples);

	int m_objectdet_terminal_sym_num;
	ObjectDetTerminalSymbol** m_objectdet_terminal_syms;
	std::vector<std::pair<int,int>> m_objectdet_wins;

	float m_min_ratio;
	float m_max_ratio;
	
private:
	AutoMakeObjectDetTerminalSymbolsTrainer(const AutoMakeObjectDetTerminalSymbolsTrainer&);
	void operator=(const AutoMakeObjectDetTerminalSymbolsTrainer&);
};
}

#endif