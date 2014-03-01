#ifndef _SYSTEMFUNCTIONS_H_
#define _SYSTEMFUNCTIONS_H_

#include "EagleeyeMacro.h"

#include "windows.h"
#include <string>
#include <sstream>
#include "EagleeyeStr.h"
#include "Print.h"

namespace eagleeye
{
/**
 *	@brief get file update time
 */
SYSTEMTIME getFileModifiedTime(const eagleeye_string& filepath);

/**
 *	@brief get the current system time
 */
SYSTEMTIME getCurrentTime();

#ifdef _SYSTEM_VERSION_INFO_
#pragma comment(lib, "version.lib")

/**
 *	@brief get version info of exe or dll
 *	@note this needs the help of "version.dll"
 */
bool getModuleVersion(HANDLE hmodule,std::string& info);
bool getModuleVersion(HANDLE hmodule,WORD* buffer);
bool getModuleVersion(std::string& info);
#endif

}

#endif