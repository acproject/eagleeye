#ifndef _GRIDSUPERPIXELNODE_H_
#define _GRIDSUPERPIXELNODE_H_

#include "EagleeyeMacro.h"
#include "AnyNode.h"
#include "Matrix.h"
#include "SignalFactory.h"
#include <math.h>
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
template<class ImageSigT>
class GridSuperPixelNode:public AnyNode
{
public:
	typedef GridSuperPixelNode						Self;
	typedef AnyNode									Superclass;

	typedef typename ImageSigT::MetaType				PixelType;

	GridSuperPixelNode();
	~GridSuperPixelNode();

	/**
	 *	@brief get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(GridSuperPixelNode);

	/**
	 *	@brief define input and output image signal type
	 */
	EAGLEEYE_INPUT_PORT_TYPE(ImageSigT,			0,		IMAGE_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSignal<int>,	0,		LABEL_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSigT,		1,		SUPERPIXEL_IMAGE_DATA);

	/**
	 *	@brief get image comprised with superpixels
	 *	@note Firstly, you have to update the pipeline. Otherwise,you would
	 *	get anything.
	 */
	Matrix<PixelType> getSuperPixelImage();

	/**
	 *	@brief get label map
	 */
	Matrix<int>	getLabelImage();

	/**
	 *	@brief get superpixel center
	 */
	Matrix<float> getSuperpixelCenter();

	/**
	 *	@brief set/get superpixels number
	 */
	void setSuperPixelsNum(const int num);
	void getSuperPixelsNum(int& num);

	/**
	 *	@brief set/get superpixel size
	 */
	void setSuperPixelSize(const int size);
	void getSuperPixelSize(int& size);

	/**
	 *	@brief execute grid superpixel algorithm
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief make self check
	 *	@note judge whether some preliminary conditions have been satisfied
	 */
	virtual bool selfcheck();

private:
	GridSuperPixelNode(const GridSuperPixelNode&);
	void operator=(const GridSuperPixelNode&);

	Matrix<float> m_superpixel_center;
	Matrix<PixelType> m_superpixel_img;
	Matrix<int> m_superpixel_label;

	int m_superpixel_size;
	int m_superpixel_num;
};
}

#include "GridSuperPixelNode.hpp"
#endif