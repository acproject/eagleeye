#ifndef _MESHFILE_H_
#define _MESHFILE_H_

#include "MFile.h"
#include "CoreMacro.h"
#include "Core.h"
#include <string>

namespace core
{
	class MeshFile:public MFile
	{
	public:
		friend class MeshFileManager;

		CORE_API virtual bool getMeshData(void* &mesh,
										MeshFileExtentInfo& exinfo,
										void* param=NULL);
		CORE_API virtual bool saveMeshData(const void* mesh,
										MeshFileExtentInfo exinfo,
										void* param=NULL);
		CORE_API ~MeshFile(void);

	private:
		CORE_API bool readFile(void*& mesh_data);
		CORE_API bool writeFile(const void* mesh_data);
		CORE_API MeshFile(const char* filepath);
	};
}

#endif
