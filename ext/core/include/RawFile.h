#ifndef _RAWFILE_H_
#define _RAWFILE_H_

#include "CoreMacro.h"
#include "Core.h"
#include "MFile.h"

namespace core
{
class RawFile:public MFile
{
	friend class RawFileManager;
public:
	CORE_API virtual bool getVolumeData(void* &data,int* dims,float* spacing,
										DataType& datatype,int slicestart=0,
										int slicenum=0,
										ImageFileExtentInfo& exinfo=ImageFileExtentInfo(),
										void* param=NULL);

	CORE_API virtual bool saveVolumeData(const void* data,const int* dims,
										const float* spacing=NULL,
										const DataType datatype=CORE_NOT_DEFINED,
										ImageFileExtentInfo exinfo=ImageFileExtentInfo(),
										void* param=NULL);


	CORE_API virtual bool getImageData(void* &data,int &rows,int &cols,
										DataType& datatype,
										ImageFileExtentInfo& exinfo=ImageFileExtentInfo(),
										void* param=NULL);

	CORE_API virtual bool saveImageData(const void* data,const int rows,
										const int cols,
										const DataType datatype,
										const ImageFileExtentInfo exinfo=ImageFileExtentInfo(),
										void* param=NULL);

	CORE_API virtual bool getImageData(void* &data,int* dims,
										DataType& datatype,
										ImageFileExtentInfo& exinfo=ImageFileExtentInfo(),
										void* param=NULL);

	CORE_API virtual bool saveImageData(const void* data,const int* dims,
										const DataType datatype,
										const ImageFileExtentInfo exinfo=ImageFileExtentInfo(),
										void* param=NULL);

	virtual ~RawFile(void);

	CORE_API virtual FileBaseInfo getFileBasicInfo();

private:
	RawFile(const char* pathname);

	unsigned char* m_data;

};
}
#endif