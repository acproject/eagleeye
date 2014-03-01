#ifndef _FILTERNODE_H_
#define _FILTERNODE_H_

#include "EagleeyeMacro.h"
#include "ImageProcessNode.h"
#include "MatrixMath.h"
#include "Matrix.h"

namespace eagleeye
{
enum FilterType
{
	BILATERAL_FILTER = 0,
	NONLOCALMEAN_FILTER,
	TRILATERAL_FILTER,
	CURVATURE_FILTER,
	GAUSSIAN_FILTER
};

template<class ImageSigT>
class FilterNode:public ImageProcessNode<ImageSigT,ImageSigT>
{
public:
	/**
	 *	@brief define some basic type
	 *	@note you must do these
	 */
	typedef FilterNode											Self;
	typedef ImageProcessNode<ImageSigT,ImageSigT>				Superclass;

	typedef typename ImageSigT::MetaType						PixelType;

	FilterNode();
	virtual ~FilterNode();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(FilterNode);
	
	/**
	 *	@brief set input and output port property
	 */
	EAGLEEYE_INPUT_PORT_TYPE(ImageSigT,0,IMAGE_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSigT,0,FILTER_RESULT_DATA);

	/**
	 *	@brief set filter type
	 */
	void setFilterType(FilterType filter_type);
	void setFilterType(const int index);
	void getFilterType(int& index);

	/**
	 *	@brief set/get space distance deviation
	 */
	void setFilterDisDev(const float dis_dev);
	void getFilterDisDev(float& dis_dev);

	/**
	 *	@brief set/get color deviation (for bilateral filter)
	 */
	void setFilterColorDev(const float color_dev);
	void getFilterColorDev(float& color_dev);

	void setFilterGradientDev(const float gradient_dev);
	void getFilterGradientDev(float& gradient_dev);

	/**
	 *	@brief pass some node info
	 *	@note Here, we don't fill up some code for this function.
	 */
	virtual void passonNodeInfo();

	/**
	 *	@brief execute filter algorithm
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief make self check
	 *	@note judge whether some preliminary conditions have been
	 *	satisfied.
	 */
	virtual bool selfcheck();

protected:
	/**
	 *	@brief bilateral filter algorithm 
	 */
	void bilateralFilter();

	/**
	 *	@brief nonlocal filter algorithm
	 */
	void nonLocalMeanFilter();

	/**
	 *	@brief filter algorithm based on gradient 
	 */
	void trilateralFilter();

	/**
	 *	@brief filter algorithm based on curvature
	 */
	void curvatureFilter();

	/**
	 *	@brief gaussian filter algorithm
	 */
	void gaussianFilter();

private:
	FilterNode(const FilterNode&);
	void operator=(const FilterNode&);

	float m_color_dev;
	float m_dis_dev;
	float m_gradient_dev;
	
	FilterType m_filter_type;/**< filter type*/
};
}

#include "FilterNode.hpp"
#endif