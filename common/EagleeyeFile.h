#ifndef _EAGLEEYEFILE_H_
#define _EAGLEEYEFILE_H_

#include "EagleeyeMacro.h"

namespace eagleeye
{
/**
 *	@brief copy file
 */
EAGLEEYE_API bool EAGLEEYE_CopyFile(const char* src_file,const char* target_file);

/**
 *	@brief delete file
 */
EAGLEEYE_API bool EAGLEEYE_DeleteFile(const char* file_path);

/**
 *	@brief rename filename
 */
EAGLEEYE_API bool EAGLEEYE_RenameFile(const char* old_file_name,const char* new_file_name);

}

#endif