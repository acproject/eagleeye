#ifndef _RANDOMWALKNODE_H_
#define _RANDOMWALKNODE_H_

#include "ImageProcessNode.h"
namespace eagleeye
{
template<class ImageSigT>
class RandomWalkNode:public ImageProcessNode<ImageSigT,ImageSigT>
{
public:
	/**
	 *	@brief define some basic type
	 *	@note you must do these
	 */
	typedef RandomWalkNode								Self;
	typedef ImageProcessNode<ImageSigT,ImageSigT>		Superclass;
	
	typedef typename ImageSigT::MetaType				PixelType;

	RandomWalkNode();
	~RandomWalkNode();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(RandomWalkNode);
};
}

#include "RandomWalkNode.hpp"
#endif