#ifndef _SERIALIMAGEREADNODE_H_
#define _SERIALIMAGEREADNODE_H_

#include "EagleeyeMacro.h"
#include "ImageReadNode.h"
#include <vector>
#include <string>

namespace eagleeye
{
enum FindFileMode
{
	FIND_CURRENT_DIR,
	FIND_ALL_DIR
};

/**
 *	@brief read serial images in the search folder
 */
template<class T>
class SerialImageReadNode:public ImageReadNode<T>
{
public:
	SerialImageReadNode();
	virtual ~SerialImageReadNode();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(SerialImageReadNode);

	/**
	 *	@brief set the search folder
	 */
	void setSearchFolder(const char* search_folder,const char* search_word);

	/**
	 *	@brief read the next file
	 */
	bool next();

	/**
	 *	@brief set find file mode
	 */
	void setFindMode(FindFileMode find_mode);

	unsigned int getFileNum(){return m_files_list.size();};

	/**
	 *	@brief build file list in the search folder
	 */
	virtual void passonNodeInfo();

	virtual void executeNodeInfo();

	/**
	 *	@brief make some self check, such as judge whether support the predefined file type.
	 */
	virtual bool selfcheck();

private:
	SerialImageReadNode(const SerialImageReadNode&);
	void operator=(const SerialImageReadNode&);

	std::string m_search_folder;				/**< the search folder*/
	std::string m_search_word;					/**< the search word '*.img'*/

	std::vector<std::string> m_files_list;		/**< the file list in the search folder*/
	int m_current_file_index;					/**< the current file index*/
	
	bool m_search_flag;
	FindFileMode m_find_mode;
};
}

#include "SerialImageReadNode.hpp"
#endif