#ifndef _IMGFILE_H_
#define _IMGFILE_H_

#include "CoreMacro.h"
#include <string>
#include "MFile.h"

namespace core
{
struct IMGParam
{
	typedef unsigned short int WORD;
	typedef unsigned long int DWORD;

	WORD version;
	WORD x_length;
	WORD y_length;
	WORD z_length;
	WORD t_length;
	WORD max_pixel;
	WORD min_pixel;
	WORD pixel_type;
	WORD pixel_order;
	WORD reserve;
	DWORD time_total_second;
	WORD time_year;
	WORD time_month;
	WORD time_date;
	WORD time_hour;
	WORD time_minute;
	WORD time_second;
};

class ImgFile:public MFile
{
	typedef unsigned short int WORD;
	typedef unsigned long int DWORD;
public:
	friend class ImgFileManager;

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
	
	virtual ~ImgFile(void);

private:
	ImgFile(const char* filepath);
	bool readFile(void* &data);
	bool writeFile(const void* data=NULL,const int rows=0,const int cols=0);

	WORD m_version;
	WORD m_x_length;
	WORD m_y_length;
	WORD m_z_length;
	WORD m_t_length;
	WORD m_max_pixel;
	WORD m_min_pixel;
	WORD m_pixel_type;
	WORD m_pixel_order;
	WORD m_reserve;
	DWORD m_time_total_second;
	WORD m_time_year;
	WORD m_time_month;
	WORD m_time_date;
	WORD m_time_hour;
	WORD m_time_minute;
	WORD m_time_second;

	unsigned char* m_img_data;

	DataType m_datatype;
};
}

#endif
