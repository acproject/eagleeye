#ifndef _TRANSFORMANDSCALE_H_
#define _TRANSFORMANDSCALE_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "ImageProcessNode.h"

namespace eagleeye
{
enum ControlType
{
	THRESHOLD_CLIP_CONTROL,
	THRESHOLD_BINARY_CONTROL,
	NORMALIZE_CLIP_CONTROL,
	TYPE_CONTROL
};

template<class SrcImageSigT,class TargetImageSigT>
class TransformAndScaleNode:public ImageProcessNode<SrcImageSigT,TargetImageSigT>
{
public:
	TransformAndScaleNode();
	virtual ~TransformAndScaleNode();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(TransformAndScaleNode);

	/**
	 *	@brief set input and output port
	 */
	EAGLEEYE_INPUT_PORT_TYPE(SrcImageSigT,		0,		IMAGE_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(TargetImageSigT,	0,		TRANSFORMED_IMAGE_DATA);

	/**
	 *	@brief set the way of operating input image
	 */
	void setControlType(ControlType control_type);

	/**
	 *	@brief set the threshold value (used in THRESHOLD_BINARY_CONTROL)
	 */
	void setBinaryThreshold(float threshold);

	/**
	 *	@brief set the threshold value (used in THRESHOLD_CLIP_CONTROL)
	 */
	void setClipThreshold(float min_threshold,float max_threshold);

	/**
	 *	@brief set min and max value (used in NORMALIZE_CLIP_CONTROL)
	 */
	void setClipNormalizeMinMax(float min_val,float max_val);

	/**
	 *	@brief execute transform and scale operation
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief make self check
	 */
	virtual bool selfcheck();

	/**
	 *	@brief print some basic info of this node
	 */
	virtual void printUnit();

protected:
	void clipImage(const Matrix<InputPixelType>& input_img,  Matrix<OutputPixelType>& output_img);
	void binaryImage(const Matrix<InputPixelType>& input_img, Matrix<OutputPixelType>& output_img);
	void clipNormalizeImage(const Matrix<InputPixelType>& input_img, Matrix<OutputPixelType>& output_img);

private:
	float m_binary_threshold;
	float m_clip_threshold[2];
	float m_clip_normalize_min_value;
	float m_clip_normalize_max_value;

	ControlType m_control_type;

};
}
#include "TransformAndScaleNode.hpp"

#endif