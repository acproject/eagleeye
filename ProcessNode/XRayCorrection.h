#ifndef _BACKGROUNDPEELOFF_H_
#define _BACKGROUNDPEELOFF_H_

#include "EagleeyeMacro.h"
#include "ImageProcessNode.h"

namespace eagleeye
{
/**
 *	@brief This algorithm module is used to process X-Ray image
 */
class EAGLEEYE_API XRayCorrection
	:public ImageProcessNode<ImageSignal<float>,ImageSignal<float>>
{
public:
	/**
	*	@brief define some basic type
	*	@note you must do these
	*/
	typedef XRayCorrection												Self;
	typedef ImageProcessNode<ImageSignal<float>,ImageSignal<float>>		Superclass;

	XRayCorrection();
	virtual ~XRayCorrection();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(XRayCorrection);

	/**
	 *	@brief set input and output port
	 */
	EAGLEEYE_INPUT_PORT_TYPE(ImageSignal<float>,	0,	IMAGE_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSignal<float>,	0,	CORRECTED_IMAGE_DATA);

	/**
	 *	@brief set the image threshold
	 */
	void clearPixelsThreshold(float low_threshold,float high_threshold);

	/**
	 *	@brief set air start column and end column
	 */
	void setAirColumn(int air_s_c,int air_e_c);

	/**
	 *	@brief set monitor start row and end row
	 */
	void setMonitorRow(int monitor_s_r,int monitor_e_r);

	/**
	 *	@brief set background image
	 */
	void setBackgroundImg(Matrix<float> background_img);

	/**
	 *	@brief set air image
	 */
	void setAirImg(Matrix<float> air_img);

	/**
	 *	@brief disable/enable basic correction process
	 */
	void enableBasicCorrection();
	void disableBasicCorrection();

	/**
	 *	@brief process node info
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief make some self check
	 */
	virtual bool selfcheck();

protected:
	/**
	 *	@brief first step
	 *	@detail subtract background, then divide air
	 */
	void airbackgroundCorrection(Matrix<float>& img);

	/**
	 *	@brief second step
	 *	@detail divide monitor data
	 */
	void monitorCorrection(Matrix<float>& img);

	/**
	 *	@brief third step
	 *	@detail clear all pixels in the predefined threshold
	 */
	void clearPixels(Matrix<float>& img);

private:
	XRayCorrection(const XRayCorrection&);
	void operator=(const XRayCorrection&);

	float m_invalid_low_threshold;
	float m_invalid_high_threshold;

	int m_air_s_c;
	int m_air_e_c;
	int m_monitor_s_r;
	int m_monitor_e_r;

	Matrix<float> m_air_img;
	Matrix<float> m_background_img;

	bool m_basic_correction_flag;
};
}

#endif