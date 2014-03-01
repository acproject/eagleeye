#ifndef _LHENODE_H_
#define _LHENODE_H_

#include "EagleeyeMacro.h"
#include "MatrixMath.h"
#include "ImageProcessNode.h"
#include "SignalFactory.h"
#include "MetaOperation.h"

namespace eagleeye
{
template<class ImageSigT>
class LHENode:public ImageProcessNode<ImageSigT,ImageSigT>
{
public:
	typedef LHENode											Self;
	typedef ImageProcessNode<ImageSigT,ImageSigT>			Superclass;

	typedef typename ImageSigT::MetaType					PixelType;

	LHENode();
	virtual ~LHENode();

	/**
	 *	@brief define class identity
	 */
	EAGLEEYE_CLASSIDENTITY(LHENode);

	/**
	 *	@brief set input and output port
	 */
	EAGLEEYE_INPUT_PORT_TYPE(ImageSigT,0,IMAGE_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSigT,0,LHE_RESULT_DATA);

	/**
	 *	@brief set/get sampling in space
	 */
	void setSamplingSpatial(float spatial_sampling);
	void getSamplingSpatial(float& spatial_sampling);

	/**
	 *	@brief set/get sampling in gray dimension
	 */
	void setSamplingGrayRange(float gray_gray_sampling);
	void getSamplingGrayRange(float& gray_gray_sampling);

	/**
	 *	@brief set/get histogram limit ratio
	 */
	void setHistLimitRatio(float limit_ratio);
	void getHistLimitRatio(float& limit_ratio);

	/**
	 *	@brief set roi region
	 */
	void setROIRegion(Array<int,4> roi);

	/**
	 *	@brief execute lhe or clhe histogram equalization
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief make self check
	 *	@note judge whether some preliminary conditions have been satisfied
	 */
	virtual bool selfcheck();

protected:
	/**
	 *	@brief Local Histogram Equalization using the Bilateral Grid
	 */
	void lheProcess(const Matrix<float>& img, Matrix<float>& result);

	/**
	 *	@brief Contrast Limiting Local Histogram Equalization using the Bilateral Grid
	 */
	void clheProcess(const Matrix<float>& img,Matrix<float>& result);

	/**
	 *	@brief computing cumulative distribution function for LHE
	 */
	void lheBuildCDFGrid();

	/**
	 *	@brief computing cumulative distribution function for CLHE
	 */
	void clheBuildCDFGrid();
	
	/**
	 *	@brief computing probability distribution function
	 *	@note this function is shared by LHE and CLHE
	 */
	void buildPDFGrid(const Matrix<float>& img);

	/**
	 *	@brief completing histogram equalization idea by tri-linear interpolation
	 *	@note this function is shared by LHE and CLHE
	 */
	void sliceCDFGrid(const Matrix<float>& img,Matrix<float>& result);

private:
	LHENode(const LHENode&);
	void operator=(const LHENode&);

	float m_spatial_sampling;
	float m_gray_range_sampling;
	float m_hist_limit_ratio;

	float* m_pdf_grid;						/**< probability distribution function*/
	float* m_cdf_grid;						/**< cumulative distribution function*/
	
	int m_down_rows;
	int m_down_cols;
	int m_down_depth;

	Array<int,4> m_roi_region;				/**< x1,y1,x2,y2*/

	unsigned long m_img_update_time;		/**< record image update time*/
};
}

#include "LHENode.hpp"
#endif