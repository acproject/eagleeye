#ifndef _JPGFILE_H_
#define _JPGFILE_H_

#include "CoreMacro.h"
#include <string>
#include "MFile.h"

namespace core
{
struct JPGParam
{
	int width;
	int height;
	int components;
	int row_stride;
	int quality;
	ColorSpace color_space;
};

class JpgFile:public MFile
{
public:
	friend class JpgFileManager;
	CORE_API virtual bool getImageData(void* &data,int &rows,int &cols,
										DataType& datatype,
										ImageFileExtentInfo& exinfo=ImageFileExtentInfo(),
										void* param=NULL);

	CORE_API virtual bool saveImageData(const void* data,const int rows,
										const int cols,const DataType datatype,
										const ImageFileExtentInfo exinfo=ImageFileExtentInfo(),
										void* param=NULL);

	CORE_API virtual bool getImageData(void* &data,int* dims,DataType& datatype,
										ImageFileExtentInfo& exinfo=ImageFileExtentInfo(),
										void* param=NULL);

	CORE_API virtual bool saveImageData(const void* data,const int* dims,
										const DataType datatype,
										const ImageFileExtentInfo exinfo=ImageFileExtentInfo(),
										void* param=NULL);

	CORE_API virtual FileBaseInfo getFileBasicInfo();

	virtual ~JpgFile(void);

private:
	JpgFile(const char* filepath);

	bool readJPEGFile();
	bool writeJPEGFile(const void* data);

	unsigned char* m_data;

	JPGParam m_jpg_param;
};

}

#endif
