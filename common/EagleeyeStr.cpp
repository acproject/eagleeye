#include "EagleeyeStr.h"
#include <vector>

namespace eagleeye
{
std::string wchar2ansi(const std::wstring& str)
{
	return wchar2ansi(str.c_str());
}

std::string wchar2ansi(std::string& str)
{
	std::string t(str.c_str());
	return t;
}

std::string wchar2ansi(LPCWSTR str)
{
	int n_len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);

	if (n_len<= 0) return std::string("");

	char* str_dst = new char[n_len+1];
	if (NULL == str_dst) return std::string("");

	memset(str_dst,0,sizeof(char)*(n_len+1));
	WideCharToMultiByte(CP_ACP, 0, str, -1, str_dst, n_len, NULL, NULL);

	std::string result(str_dst);
	delete [] str_dst;

	return result;
}

std::wstring ansi2wchar(const std::string& str)
{
	return ansi2wchar(str.c_str());
}

std::wstring ansi2wchar(const std::wstring& str)
{
	std::wstring t(str.c_str());
	return t;
}

std::wstring ansi2wchar(LPCSTR str)
{
	int unicode_len  = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)str, -1, 0, 0);
	if(unicode_len  <= 0) return NULL;

	wchar_t *str_dst = new wchar_t[unicode_len +1];
	if( NULL == str_dst) return NULL;
	memset(str_dst,0,(unicode_len+1)*sizeof(wchar_t));  

	MultiByteToWideChar(CP_ACP, 0,(LPCSTR)str, -1, str_dst, unicode_len );
	str_dst[unicode_len ] = 0;

	std::wstring result(str_dst);
	delete []str_dst;

	return result;
}

eagleeye_string extractPath(TCHAR* pathname,int len)
{
	//find the end of pathname
	int len_end = 0;
	TCHAR* temp_path = pathname;
	while((*temp_path) != _T('\0') && len_end < len)
	{
		len_end++;
		temp_path++;
	}

	//find / from the end to the start
	while(((*temp_path)!=_T('\\'))&&((*temp_path)!=_T('/')))
	{
		(*temp_path) = _T('\0');
		temp_path--;
	}

	eagleeye_string str(pathname);
	return str;
}

Matrix<float> splitstr(const char* str)
{
	std::vector<float> numbers;
	while((*str) != '\0')
	{
		char num[100];
		int numindex = 0;
		while((((*str) != ' ') && ((*str) != ',') && ((*str) != ';') && ((*str) != '\t')) && (*str) != '\0')
		{
			num[numindex] = (*str);
			str++;
			numindex++;
		}
		num[numindex] = '\0';
		if (numindex != 0)
		{
			//×ª»»³ÉÊý×Ö
			float f_num = float(atof(num));
			numbers.push_back(f_num);
		}
		if ((*str) == '\0')
			break;
		else
			str++;
	}

	Matrix<float> number_mat(1,numbers.size());
	memcpy(number_mat.dataptr(),&numbers[0],sizeof(float) * numbers.size());
	
	return number_mat;
}

std::string extractName(const char* path)
{
	//find the end of pathname
	int len_end = 0;
	while(path[len_end] != '\0')
	{
		len_end++;
	}
	len_end = len_end - 1;

	//find / from the end to the start
	while((path[len_end] != '\\') && (path[len_end] != '/'))
	{
		len_end--;
	}

	std::string str(path + len_end + 1);
	return str;
}

std::string extractNameWithoutExt(const char* path)
{
	std::string str_path = path;
	std::string::size_type file_name_start_pos = str_path.rfind("\\");
	if (file_name_start_pos == std::string::npos)
	{
		file_name_start_pos = str_path.rfind("/");
		if (file_name_start_pos == std::string::npos)
		{
			return std::string();
		}
	}
	
	std::string::size_type file_name_end_pos = str_path.rfind(".");
	return str_path.substr(file_name_start_pos + 1,(file_name_end_pos - file_name_start_pos - 1));
}

std::vector<std::string> splitpath(const char* path)
{
	std::string str_path = path;
	std::string sub_str_path;

	std::string::size_type slash_start_pos = 0;
	std::string::size_type slash_end_pos = 0;
	std::vector<std::string> split_str;
	
	slash_start_pos = EAGLEEYE_MIN(str_path.find_first_of("\\"),str_path.find_first_of("/"));
	if (slash_start_pos == std::string::npos)
	{
		return split_str;
	}

	sub_str_path = str_path.substr(slash_start_pos + 1);

	while(EAGLEEYE_MIN(sub_str_path.find_first_of("\\"),sub_str_path.find_first_of("/")) != std::string::npos)
	{
		slash_end_pos = EAGLEEYE_MIN(sub_str_path.find_first_of("\\"),sub_str_path.find_first_of("/"));
		std::string key_str = sub_str_path.substr(0,slash_end_pos);
		split_str.push_back(key_str);

		sub_str_path = sub_str_path.substr(slash_end_pos + 1);
	}

	return split_str;
}

std::string extractPath(const char* path)
{
	//find the end of pathname
	std::string path_str = path;
	
	int str_length = path_str.length();
	int check_pos = str_length - 1;
	while((path_str.at(check_pos) != '\\') && (path_str.at(check_pos) != '/'))
	{
		check_pos--;
	}

	std::string str = path_str.substr(0,check_pos + 1);
	return str;
}

void copystr(char* target,const char* src,int max_len /* = EAGLEEYE_MAX_STR */)
{
	for (int i = 0;i < max_len; ++i)
	{
		if (src[i] != '\0')
			target[i] = src[i];
		else
		{
			target[i] = '\0';
			return;
		}
	}
}
}

