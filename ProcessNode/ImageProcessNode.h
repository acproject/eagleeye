#ifndef _IMAGEPROCESSNODE_H_
#define _IMAGEPROCESSNODE_H_

#include "EagleeyeMacro.h"
#include "AnyNode.h"
#include "SignalFactory.h"

namespace eagleeye
{
template<class SrcT,class TargetT>
class ImageProcessNode:public AnyNode
{
public:
	/**
	 *	@brief define some basic type
	 *	@note you must do these
	 */
	typedef ImageProcessNode							Self;
	typedef AnyNode										Superclass;

	typedef typename SrcT::MetaType						InputPixelType;
	typedef typename TargetT::MetaType					OutputPixelType;


	ImageProcessNode();
	virtual ~ImageProcessNode();

	/**
	 *	@brief get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(ImageProcessNode);

	/**
	 *	@brief get output image
	 */
	Matrix<OutputPixelType> getOutputImage(unsigned int index=0);

	/**
	 *	@brief get input image signal
	 */
	SrcT* getInputImageSignal(unsigned int index=0);
	const SrcT* getInputImageSignal(unsigned int index=0) const;

	/**
	 *	@brief get output image signal
	 */
	TargetT* getOutputImageSignal(unsigned int index=0);
	const TargetT* getOutputImageSignal(unsigned int index=0) const;

protected:
	/**
	 *	@brief make one output signal
	 */
	virtual AnySignal* makeOutputSignal();

private:
	ImageProcessNode(const ImageProcessNode&);
	void operator=(const ImageProcessNode&);
};
}

#include "ImageProcessNode.hpp"
#endif