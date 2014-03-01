#ifndef _GLOBALFUNCTION_H_
#define _GLOBALFUNCTION_H_

#include "Core.h"
#include "CoreMacro.h"
#include <string>
#include "boost/algorithm/string.hpp"
#include "boost/algorithm/string_regex.hpp"
#include "boost/regex.hpp"
#include "boost/format.hpp"
#include "boost/lexical_cast.hpp"

namespace core
{
std::string extractFileNameFromPath(std::string path);
std::string removeExt(std::string file_name);
std::string extractPathNameFromPath(std::string path);
std::string extractExtFromFileName(std::string file_name);
void splitPrefixAndIndexFromFileName(std::string file_name,
									std::string& prefix,
									std::string& index);
std::string generateNumber(int num,int numberspace);
std::string mergePrefixAndIndex(std::string prefix,
								int num,
								int numberspace);

std::string extractName(const char* path);
std::string extractNameWithoutExt(const char* path);

}
#endif