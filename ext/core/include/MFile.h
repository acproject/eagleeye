#ifndef _MFILE_H_
#define _MFILE_H_

#include "CoreMacro.h"
#include "Core.h"
#include <string>

namespace core
{
struct FileBaseInfo
{
	FileDescription file_data_description;
};

struct ImageFileExtentInfo 
{
	ImageFileExtentInfo()
	{
		memset(pixel_spacing,0,sizeof(float)*3);
		memset(image_dim,0,sizeof(int)*3);
		image_reading_state=SINGLE_READING;
		image_num=1;
		image_size_bits=0;
		perpixel_bits=1;
	};

	~ImageFileExtentInfo(){};

	int image_dim[3];
	float pixel_spacing[3];
	ImageReadingState image_reading_state;
	unsigned int image_num;
	unsigned long perpixel_bits;
	DataType pixel_type;
	unsigned long image_size_bits;/**< the number of bytes*/
};

struct MeshFileExtentInfo
{
	MeshFileExtentInfo(){};
	~MeshFileExtentInfo(){};
};

class MFile
{
	friend class MFileManager;
public:
	CORE_API virtual bool getVolumeData(void* &data,int* dims,float* spacing,
										DataType& datatype,int slicestart=0,
										int slicenum=0,
										ImageFileExtentInfo& exinfo=ImageFileExtentInfo(),
										void* param=NULL)
	{
		return false;
	}

	CORE_API virtual bool saveVolumeData(const void* data,const int* dims,
										const float* spacing=NULL,
										const DataType datatype=CORE_NOT_DEFINED,
										ImageFileExtentInfo exinfo=ImageFileExtentInfo(),
										void* param=NULL)
	{
		return false;
	}

	CORE_API virtual bool getImageData(void* &data,int &rows,int &cols,
										DataType& datatype,
										ImageFileExtentInfo& exinfo=ImageFileExtentInfo(),
										void* param=NULL)
	{
		return false;
	}

	CORE_API virtual bool saveImageData(const void* data,const int rows,
										const int cols,const DataType datatype,
										const ImageFileExtentInfo exinfo=ImageFileExtentInfo(),
										void* param=NULL)
	{
		return false;
	}

	CORE_API virtual bool getImageData(void* &data,int* dims,DataType& datatype,
										ImageFileExtentInfo& exinfo=ImageFileExtentInfo(),
										void* param=NULL)
	{
		return false;
	}

	CORE_API virtual bool saveImageData(const void* data,const int* dims,
										const DataType datatype,
										const ImageFileExtentInfo exinfo=ImageFileExtentInfo(),
										void* param=NULL)
	{
		return false;
	}

	CORE_API virtual bool getMeshData(void* &mesh,
										MeshFileExtentInfo& exinfo,
										void* param=NULL)
	{
		return false;
	}

	CORE_API virtual bool saveMeshData(const void* mesh,
										MeshFileExtentInfo exinfo,
										void* param=NULL)
	{
		return false;
	}

	CORE_API virtual FileDescription getFileDataDescription()
	{
		return m_file_data_description;
	}

	CORE_API void setFileExtentName(std::string fileextentname);
	CORE_API void setFileName(std::string filename);

	CORE_API std::string getFileExtentName();
	CORE_API std::string getFileName();
	CORE_API std::string getFilePath();
	CORE_API std::string getFileNameWithoutExt();

	CORE_API virtual FileBaseInfo getFileBasicInfo();

	virtual ~MFile(void);

protected:
	std::string m_path_name;
	std::string m_file_extent_name;
	std::string m_file_name;
	std::string m_file_name_without_ext;
	FileDescription m_file_data_description;

	MFile(const char* filepath);
};

}

#endif
