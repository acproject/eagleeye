#ifndef _SIGNALFACTORY_H_
#define _SIGNALFACTORY_H_

#include "EagleeyeMacro.h"

#include "AnySignal.h"
#include "Matrix.h"
#include "TraitCenter.h"
#include "Print.h"
#include <string>

namespace eagleeye
{
//////////////////////////////////////////////////////////////////////////
class EAGLEEYE_API BaseImageSignal:public AnySignal
{
public:
	/**
	 *	@brief define some basic type
	 *	@note you must do these
	 */
	typedef BaseImageSignal						Self;
	typedef AnySignal							Superclass;

	BaseImageSignal(EagleeyePixel p_type,int cha,char* info = "")
		:AnySignal("ImgSig"),pixel_type(p_type),
		channels(cha),
		ext_info(info){}
	virtual ~BaseImageSignal(){};

	/**
	 *	@brief copy info
	 */
	virtual void copyInfo(AnySignal* sig)
	{
		BaseImageSignal* from_sig = dynamic_cast<BaseImageSignal*>(sig);
		if (from_sig)
		{
			ext_info = from_sig->ext_info;
			needed_size = from_sig->needed_size;
		}
	}

	/**
	 *	@brief get image data
	 */
	virtual void getEagleEyeData(void*& eagleeye_data,
		int& eagleeye_rows,int& eagleeye_cols,
		int& eagleeye_offset_r,int& eagleeye_offset_c,
		int& eagleeye_width) = 0;

	const EagleeyePixel pixel_type;
	const int channels;
	std::string ext_info;
	Array<int,2> needed_size;
};

//////////////////////////////////////////////////////////////////////////
template<class T>
class ImageSignal:public BaseImageSignal
{
public:
	/**
	 *	@brief define some basic type
	 *	@note you must do these
	 */
	typedef ImageSignal				Self;
	typedef BaseImageSignal			Superclass;

	/**
	 *	@brief meta type hold by Image Signal
	 */
	typedef T						MetaType;

	ImageSignal(Matrix<T> m=Matrix<T>(),char* name="",char* info="");
	virtual ~ImageSignal(){};

	/**
	 *	@brief copy info
	 */
	virtual void copyInfo(AnySignal* sig);

	/**
	 *	@brief print image signal info
	 */
	virtual void printUnit();

	/**
	 *	@brief clear image signal content
	 */
	virtual void makeempty();

	/**
	 *	@brief create one image
	 *	@note most of time, we should use this function to load one image,\n
	 *	it would force updating 'data time'
	 */
	virtual void createImage(Matrix<T> m,char* name="",char* info="");

	/**
	 *	@brief get image
	 *	@note most of time, we should use this function to get image
	 */
	Matrix<T> getImage();

	/**
	 *	@brief get image data
	 *	@param eagleeye_data hold by Matrix
	 *	@param eagleeye_rows matrix row
	 *	@param eagleeye_cols matrix col
	 *	@param eagleeye_offset_r row offset
	 *	@param eagleeye_offset_c col offset
	 *	@param eagleeye_width matrix width
	 */
	virtual void getEagleEyeData(void*& eagleeye_data,
		int& eagleeye_rows,int& eagleeye_cols,
		int& eagleeye_offset_r,int& eagleeye_offset_c,
		int& eagleeye_width);

	Matrix<T> img;
	std::string img_name;
};

//////////////////////////////////////////////////////////////////////////
template<class InfoT>
class InfoSignal:public AnySignal
{
public:
	/**
	 *	@brief define some basic type
	 *	@note you must do these
	 */
	typedef InfoSignal							Self;
	typedef AnySignal							Superclass;

	typedef InfoT								MetaType;

	InfoSignal(InfoT* sth_info=NULL):info(sth_info){};
	virtual ~InfoSignal(){};

	/**
	 *	@brief copy info
	 */
	virtual void copyInfo(AnySignal* sig);

	/**
	 *	@brief print InfoSignal info
	 */
	virtual void printUnit();

	/**
	 *	@brief clear Info signal content
	 */
	virtual void makeempty();

	InfoT* info;
};

#define TO_IMAGE_SIGNAL(TYPE,S) \
	dynamic_cast<eagleeye::ImageSignal<TYPE>*>(S)
#define TO_IMAGE(TYPE,S) \
	(dynamic_cast<eagleeye::ImageSignal<TYPE>*>(S))->img

#define TO_INFO_SIGNAL(TYPE,S) \
	dynamic_cast<eagleeye::InfoSignal<TYPE>*>(S)
#define TO_INFO(TYPE,S) \
	*((dynamic_cast<eagleeye::InfoSignal<TYPE>*>(S))->info)


}

#include "SignalFactory.hpp"
#endif