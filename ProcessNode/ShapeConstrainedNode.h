#ifndef _SHAPECONFINEDNODE_H_
#define _SHAPECONFINEDNODE_H_

#include "EagleeyeMacro.h"
#include "ImageProcessNode.h"
#include "SignalFactory.h"
#include "RegionAndImageProperty.h"
#include "opencv2/opencv.hpp"

namespace eagleeye
{
template<class ImageSigT>
class ShapeConstrainedNode:public ImageProcessNode<ImageSigT,ImageSigT>
{
public:
	typedef ShapeConstrainedNode							Self;
	typedef ImageProcessNode<ImageSigT,ImageSigT>			Superclass;

	typedef typename ImageSigT::MetaType					PixelType;

	/**
	 *	@brief confined flags
	 */
	enum _Conflags
	{
		_CONSTRAINED_MASK = 0x1f
	};
	static const _Conflags area_constrained = (_Conflags)0x1;
	static const _Conflags w_h_ratio_constrained = (_Conflags)0x2;
	static const _Conflags circle_constrained = (_Conflags)0x4;
	static const _Conflags region_constrained = (_Conflags)0x08;

	ShapeConstrainedNode();
	virtual ~ShapeConstrainedNode();

	/**
	 *	@brief get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(ShapeConstrainedNode);

	/**
	 *	@brief set input and output port
	 */
	EAGLEEYE_INPUT_PORT_TYPE(ImageSigT,0,IMAGE_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSigT,0,SHAPE_CONSTRAINED_IMAGE_DATA);

	/**
	 *	@brief set constrained flags
	 */
	void setConstrainedFlags(_Conflags con_flags);

	/**
	 *	@brief set constrained area, including min area and max area
	 */
	void setConstrainedArea(float min_area,float max_area);

	/**
	 *	@brief set constrained width height ratio, including min ratio and max ratio
	 */
	void setConstrainedWHRatio(float min_ratio,float max_ratio);

	/**
	 *	@brief set constrained circle radius, including min radius and max radius
	 */
	void setConstrainedCircleRadius(float min_radius,float max_radius);

	/**
	 *	@brief set constrained region, including bottom row, top row, left col and right col.
	 */
	void setConstrainedRegion(int bottom_r,int top_r,int left_c, int right_c);

	/**
	 *	@brief implement shape constrained operation
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief make self check
	 *	@note judge whether some preliminary conditions have been satisfied
	 */
	virtual bool selfcheck();

	/**
	 *	@brief do some clear work
	 */
	virtual void clearSomething();

	/**
	 *	@brief print node info
	 */
	virtual void printUnit();

protected:
	void constrainedOperation(Matrix<unsigned char>& img);

private:
	float m_w_h_ratio[2];
	float m_area[2];
	float m_circle_radius[2];
	int m_region[4];

	Matrix<int> m_connected_components_label;
	int m_connected_components_num;
	std::map<int,std::vector<int>> m_connected_components_table;

	_Conflags m_con_flags;
};
}

#include "ShapeConstrainedNode.hpp"
#endif