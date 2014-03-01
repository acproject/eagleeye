#ifndef _CARRIAGEMASK_H_
#define _CARRIAGEMASK_H_

#include "EagleeyeMacro.h"
#include "AnyNode.h"
#include "Matrix.h"
#include "MatrixMath.h"
#include "Array.h"
#include "SignalFactory.h"
#include "opencv2/opencv.hpp"
namespace eagleeye
{
template<class ImageSigT>
class CarriageMask : public AnyNode
{
public:
	/**
	/* @brief define some basic type
	/* @note must typedef Self/Superclass
	*/
	typedef CarriageMask						Self;
	typedef AnyNode								SuperClass;
	typedef typename ImageSigT::MetaType		PixelType;
	
	/** @brief define input and output image signal type */
	EAGLEEYE_INPUT_PORT_TYPE( ImageSigT, 0, IMAGE_DATA );
	EAGLEEYE_OUTPUT_PORT_TYPE( ImageSignal<unsigned char>, 0,ROI_IMAGE_DATA );
	EAGLEEYE_OUTPUT_PORT_TYPE( ImageSignal<unsigned char>, 1,ROI_MASK_DATA );
	EAGLEEYE_OUTPUT_PORT_TYPE( ImageSignal<ERGB>, 2,COLOR_IMAGE_DATA );

	CarriageMask();
	virtual ~CarriageMask();

	/**
	/* @brief Get class identity
	*/
	EAGLEEYE_CLASSIDENTITY( CarriageMask );

	/**
	/* @brief set&get parameters
	*/
	
	/**
	/* @brief Mask Car/Cargo, get mask image of the carriage
	/* @note calculate as unsigned char
	*/
	virtual bool selfcheck();
	virtual void executeNodeInfo();

	RECT getROIRect(){ return m_roi; };
	void setROIRect(RECT r){ m_roi = r; };

private:
	unsigned char m_gray_threshold;
	float m_scale_profile;
	float m_scale_lightscan;

	std::vector<int> m_profile_high;
	std::vector<int> m_profile_low;

	//得到轮廓
	void getProfile(Matrix<unsigned char>& image);
	//从“光幕图像”计算分割位置
	int getCutLoc( int& cut_left, int& cut_right);

	RECT m_roi;
	bool checkROI(int h, int w);
	void calcOutputImage( Matrix<unsigned char>& org, Matrix<unsigned char>& img8,
		Matrix<unsigned char>& mask, Matrix<Array<unsigned char,3>>& debug );
};
}

#include "CarriageMask.hpp"
#endif