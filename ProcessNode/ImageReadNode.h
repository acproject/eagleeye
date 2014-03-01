#ifndef _IMAGEREADNODE_H_
#define _IMAGEREADNODE_H_

#include "EagleeyeMacro.h"
#include "FileManager.h"
#include "MFile.h"
#include "EagleeyeCore.h"
#include "TraitCenter.h"
#include "ImageIONode.h"

namespace eagleeye
{
/**
 *	@brief reading image file to any Matrix
 */
template<class ImgSigT>
class ImageReadNode:public ImageIONode<ImgSigT>
{
public:
	typedef ImageReadNode						Self;
	typedef ImageIONode<ImgSigT>				Superclass;

	typedef typename ImgSigT::MetaType			PixelType;

	ImageReadNode();
	virtual ~ImageReadNode();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(ImageReadNode);

	/**
	 *	@brief define output port image signal type
	 */
	EAGLEEYE_OUTPUT_PORT_TYPE(ImgSigT,	0,	IMAGE_DATA);

	/**
	 *	@brief pass image struct info
	 */
	virtual void passonNodeInfo();

	/**
	 *	@brief read image data from file
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief make some self check, such as judge whether support the predefined file type.
	 */
	virtual bool selfcheck();

private:
	ImageReadNode(const ImageReadNode&);
	void operator=(const ImageReadNode&);
};
}

#include "ImageReadNode.hpp"
#endif