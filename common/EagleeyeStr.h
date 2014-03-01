#ifndef _EAGLEEYESTR_H_
#define _EAGLEEYESTR_H_

#include "EagleeyeMacro.h"

#include <string>
#include <tchar.h>
#include <windows.h>
#include "Matrix.h"
#include <string>
#include <vector>

namespace eagleeye
{
#ifndef eagleeye_string
#ifdef UNICODE
#define eagleeye_string std::wstring
#define eagleeye_stream std::wstringstream
#else
#define eagleeye_string std::string
#define eagleeye_stream std::stringstream
#endif
#endif

EAGLEEYE_API std::string wchar2ansi(LPCWSTR str);
EAGLEEYE_API std::string wchar2ansi(const std::wstring& str);
EAGLEEYE_API std::string wchar2ansi(const std::string& str);

EAGLEEYE_API std::wstring ansi2wchar(LPCSTR str);
EAGLEEYE_API std::wstring ansi2wchar(const std::string& str);
EAGLEEYE_API std::wstring ansi2wchar(const std::wstring& str);

/**
 *	@brief split string to number (number is separated by space or comma or tab or semicolon)
 */
EAGLEEYE_API Matrix<float> splitstr(const char* str);

EAGLEEYE_API eagleeye_string extractPath(TCHAR* pathname,int len);
EAGLEEYE_API std::string extractName(const char* path);
EAGLEEYE_API std::string extractNameWithoutExt(const char* path);
EAGLEEYE_API std::vector<std::string> splitpath(const char* path);
EAGLEEYE_API std::string extractPath(const char* path);

EAGLEEYE_API void copystr(char* target,const char* src,int max_len = EAGLEEYE_MAX_STR);
}


#endif