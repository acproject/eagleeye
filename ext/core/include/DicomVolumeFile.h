#ifndef _DICOMVOLUMEFILE_H_
#define _DICOMVOLUMEFILE_H_

#include "CoreMacro.h"
#include "Core.h"
#include "MFile.h"

namespace core
{
class DicomVolumeFile:public MFile
{
public:
	friend class DicomVolumeFileManager;
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

	CORE_API virtual bool getImageData(void* &data,int* dims,DataType& datatype,
										ImageFileExtentInfo& exinfo=ImageFileExtentInfo(),
										void* param=NULL);

	CORE_API virtual bool saveImageData(const void* data,const int* dims,
										const DataType datatype,
										const ImageFileExtentInfo exinfo=ImageFileExtentInfo(),
										void* param=NULL);


	CORE_API virtual FileBaseInfo getFileBasicInfo();
	
	virtual ~DicomVolumeFile(void);

private:
	DicomVolumeFile(const char* filepath);
	void splitBackSlashStrToFloat(char* str,float& ele1,float& ele2);
	void determinDataType(int pixeldepth);
	
	int m_width;
	int m_height;
	int m_slice_num;
	DataType m_datatype;
	float m_spacing[3];
	unsigned char* m_volume_data;
	int m_volume_data_byte_num;
	int m_slice_data_byte_num;
	unsigned long m_width_length;

};
}

#endif