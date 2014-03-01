#include "SystemFunctions.h"

namespace eagleeye
{
SYSTEMTIME getFileModifiedTime(const eagleeye_string& filepath)
{
	HANDLE hfile;
	WIN32_FIND_DATA wfd;
	SYSTEMTIME sys_t;
	memset(&sys_t,0,sizeof(SYSTEMTIME));
	memset(&wfd,0,sizeof(wfd));

	if ((hfile=FindFirstFile(filepath.c_str(),&wfd))==INVALID_HANDLE_VALUE)
	{
		EAGLEEYE_ERROR("couldn't find file \n");
		return sys_t;
	}

	FILETIME file_write_time=wfd.ftLastWriteTime;
	FileTimeToLocalFileTime(&file_write_time,&file_write_time);
	FileTimeToSystemTime(&file_write_time,&sys_t);

	return sys_t;
}

SYSTEMTIME getCurrentTime()
{
	SYSTEMTIME sys_t; 
	GetLocalTime( &sys_t);
	return sys_t;
}

#ifdef _SYSTEM_VERSION_INFO_
bool getModuleVersion(std::string& info)
{
	HANDLE hmodule=GetModuleHandle(NULL);
	return getModuleVersion(hmodule,info);
}

bool getModuleVersion(HANDLE hmodule,std::string& info)
{
	WORD buffer[4];
	info.clear();

	if (getModuleVersion(hmodule, buffer))
	{
		char str2[64];
		for (int i = 0; i < 4; i++)
		{
			int buffer_num=buffer[i];
			itoa(buffer_num, str2, 10);
			info += str2;

			if (i !=  3)
			{
				info += ".";
			}
		}

		return true;
	}

	return false;
}

bool getModuleVersion(HANDLE hmodule,WORD* buffer)
{
	TCHAR file_path_name[MAX_PATH];
	VS_FIXEDFILEINFO *p_vi;
	DWORD dw_handle;

	if (GetModuleFileName((HMODULE)hmodule, file_path_name, MAX_PATH))
	{
		int size = GetFileVersionInfoSize(file_path_name, &dw_handle);

		if (size > 0) 
		{
			BYTE* data=new BYTE[size];
			if (GetFileVersionInfo(file_path_name, dw_handle, size, data)) 
			{
				if (VerQueryValue(data, _T("\\"), (LPVOID *)&p_vi, (PUINT)&size)) 
				{
					buffer[0] = HIWORD(p_vi->dwFileVersionMS);
					buffer[1] = LOWORD(p_vi->dwFileVersionMS);
					buffer[2] = HIWORD(p_vi->dwFileVersionLS);
					buffer[3] = LOWORD(p_vi->dwFileVersionLS);

					delete data;
					return true;
				}
			}

			delete data;
		}
	}

	return false;
}
#endif

}