#ifndef _PPMFILE_H_
#define _PPMFILE_H_

#include "CoreMacro.h"
#include "MFile.h"
#include <string>

namespace core
{
/**
 *	@brief only support image with RGB pixel
 */
class PPMFile:public MFile
{
public:
	friend class PPMFileManager;

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

	~PPMFile();

private:
	PPMFile(const char* filename);

	bool readPPMFile();
	bool writePPMFile(const void* data);

	int m_image_height;
	int m_image_width;
	unsigned char* m_data;

	DataType m_data_type;
};
}

#endif