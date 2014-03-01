#ifndef _SUPERPIXELSPLITCARGO_H_
#define _SUPERPIXELSPLITCARGO_H_

#include "EagleeyeMacro.h"
#include "AnyNode.h"
#include "Matrix.h"
#include "SignalFactory.h"
#include "BasicMath/MedianFilter1D.h"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
template<class ImageSigT>
class SuperpixelSplitCargo:public AnyNode
{
public:
	typedef SuperpixelSplitCargo					Self;
	typedef AnyNode									Superclass;
	typedef typename ImageSigT::MetaType			PixelType;

	EAGLEEYE_CLASSIDENTITY(SuperpixelSplitCargo);

	EAGLEEYE_INPUT_PORT_TYPE(ImageSigT,0,IMAGE_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSigT,0,DISPLAY_SPLIT_IMAGE);

	SuperpixelSplitCargo();
	~SuperpixelSplitCargo();


	/**
	 *	@brief start splitting process
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief make self check
	 */
	virtual bool selfcheck();

protected:
	void splitToSuperpixel();

private:
	SuperpixelSplitCargo(const SuperpixelSplitCargo&);
	void operator=(const SuperpixelSplitCargo&);

	unsigned char m_gray_threshold;
};
}

#include "SuperpixelSplitCargo.hpp"
#endif