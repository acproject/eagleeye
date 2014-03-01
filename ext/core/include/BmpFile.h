#ifndef _BMPFILE_H_
#define _BMPFILE_H_

#include "CoreMacro.h"
#include <string>
#include <Windows.h>
#include "MFile.h"

namespace core
{
struct ColorPlate
{
	unsigned char rgb_blue;
	unsigned char rgb_green;
	unsigned char rgb_red;
	unsigned char rgb_reserved;
};

class BmpFile:public MFile
{
public:
	friend class BmpFileManager;

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

	~BmpFile();

private:
	BmpFile(const char* filepath);

	bool readBMPFile();
	bool writeBMPFile(const void* data);

	int m_image_height;
	int m_image_width;
	int m_line_bytes;
	
	unsigned char* m_bmp_data;
	ColorPlate* m_color_table;

	DataType m_data_type;

	BITMAPINFOHEADER m_bmp_param;
};
}
#endif
