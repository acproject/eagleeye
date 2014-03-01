#ifndef _IMAGEIONODE_H_
#define _IMAGEIONODE_H_

#include "EagleeyeMacro.h"
#include "FileManager.h"
#include "MFile.h"
#include "AnyNode.h"
#include "SignalFactory.h"

namespace eagleeye
{
enum ChanelsOrder
{
	CHANELS_SWITCH_R_AND_B,
	CHANELS_NO_CHANGE
};

template<class ImgSigT>
class ImageIONode:public AnyNode
{
public:
	typedef ImageIONode							Self;
	typedef AnyNode								Superclass;

	typedef typename ImgSigT::MetaType			PixelType;

	ImageIONode();
	virtual ~ImageIONode();

	/**
	 *	@brief get class identity	
	 */	 
	 EAGLEEYE_CLASSIDENTITY(ImageIONode);

	/**
	 *	@brief set/get the file path
	 */
	void setFilePath(const char* file_path);
	std::string getFilePath();

	/**
	 *	@brief get image
	 */
	Matrix<PixelType> getImage();

	void switchChanelsOrder(ChanelsOrder c_order);

protected:
	std::string m_file_path;
	ChanelsOrder m_channels_order;

	/**
	 *	@brief make one output image signal
	 */
	virtual AnySignal* makeOutputSignal();

	/**
	 *	@brief switch pixel channels
	 */
	template<class T>
	void switchPixelChannels(Matrix<T>& img){};
	template<>
	void switchPixelChannels(Matrix<ERGB>& img)
	{
		switch(m_channels_order)
		{
		case CHANELS_SWITCH_R_AND_B:
			{
				img = img.transform(SwitchOperations<ERGB,ERGB,0,2>());
				return;
			}
		}
	}
	template<>
	void switchPixelChannels(Matrix<ERGBA>& img)
	{
		switch(m_channels_order)
		{
		case CHANELS_SWITCH_R_AND_B:
			{
				img = img.transform(SwitchOperations<ERGBA,ERGBA,0,2>());
				return;
			}
		}
	}

private:
	ImageIONode(const ImageIONode&);
	void operator=(const ImageIONode&);
};
}

#include "ImageIONode.hpp"
#endif