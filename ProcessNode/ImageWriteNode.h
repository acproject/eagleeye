#ifndef _IMAGEWRITENODE_H_
#define _IMAGEWRITENODE_H_

#include "EagleeyeMacro.h"
#include "FileManager.h"
#include "MFile.h"
#include "TraitCenter.h"
#include "MetaOperation.h"
#include "ImageIONode.h"

namespace eagleeye
{
template<class ImgSigT>
class ImageWriteNode:public ImageIONode<ImgSigT>
{
public:
	typedef ImageWriteNode						Self;
	typedef ImageIONode<ImgSigT>				Superclass;

	typedef typename ImgSigT::MetaType			PixelType;

	ImageWriteNode();
	~ImageWriteNode();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(ImageWriteNode);

	/**
	 *	@brief define input port image signal type
	 */
	EAGLEEYE_INPUT_PORT_TYPE(ImgSigT,	0,	IMAGE_DATA);

	/**
	 *	@brief update image write node info
	 */
	virtual void passonNodeInfo();

	/**
	 *	@brief write image to file
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief make some self check, such as judge whether support
	 *	the predefined file type, some parameters are prepared.
	 */
	virtual bool selfcheck();

private:
	ImageWriteNode(const ImageWriteNode&);
	void operator=(const ImageWriteNode&);
};
}

#include "ImageWriteNode.hpp"
#endif