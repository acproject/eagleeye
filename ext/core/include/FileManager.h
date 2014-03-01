#ifndef _FILEMANAGER_H_
#define _FILEMANAGER_H_

#include "CoreMacro.h"
#include "MFile.h"
#include <stdarg.h>
#include <vector>
#include <string>
#include <map>
#include "boost/smart_ptr/shared_ptr.hpp"

namespace core
{
class FileManager
{
public:
	typedef CoreSmartPointer<MFile>							FileHandle;
	typedef DataType										FileDataType;
	typedef std::map<std::string,const FileManager*>		FileManagerMap;

	static CORE_API FileHandle FileFactory(const char* filename);
	static CORE_API void getFileList(const char* dir,const char *filespec,std::vector<std::string>& filelist);
	static CORE_API void getAllFileList(const char* dir,const char* filespec,std::vector<std::string>& filelist);
	static CORE_API bool isSupport(const char* extname);
	static CORE_API int getSupportTypeNum();
	static CORE_API void getSupportType(int index,std::string& ext);

protected:
	virtual FileHandle buildFileHandle(const std::string& filename) const{return FileHandle();};
	
	FileManager();
	~FileManager(void);	
	static FileManagerMap m_dynamic_registry;

private:
	static CORE_API std::string parseFileName(const std::string& filename);
};

#define CORE_FILE_MANAGER(FileClass,ext) \
	class FileClass##Manager:public FileManager \
	{ \
	public: \
		typedef FileManager::FileHandle				FileHandle; \
	private: \
		virtual FileHandle buildFileHandle(const std::string& filename) const \
		{ \
			return FileHandle(new FileClass(filename.c_str())); \
		} \
		FileClass##Manager() \
		{ \
			m_dynamic_registry.insert(std::make_pair(std::string(ext),this)); \
		} \
		~FileClass##Manager(){}; \
		static const FileClass##Manager m_static_register_filemanager_type; \
	}; \
	const FileClass##Manager FileClass##Manager::m_static_register_filemanager_type;
}

#endif
