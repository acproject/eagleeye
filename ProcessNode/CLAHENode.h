#ifndef _CLAHENODE_H_
#define _CLAHENODE_H_

#include "EagleeyeMacro.h"
#include "ImageProcessNode.h"
#include "opencv2/opencv.hpp"

namespace eagleeye
{
template<class ImageSigT>
class CLAHENode:public ImageProcessNode<ImageSigT,ImageSigT>
{
public:
	/**
	 *	@brief define some basic type
	 *	@note you must do these
	 */
	typedef CLAHENode											Self;
	typedef ImageProcessNode<ImageSigT,ImageSigT>				Superclass;

	typedef typename ImageSigT::MetaType						PixelType;

	CLAHENode();
	virtual ~CLAHENode();

	/**
	 *	@brief get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(CLAHENode);

	/**
	 *	@brief set input and output port
	 */
	EAGLEEYE_INPUT_PORT_TYPE(ImageSigT,0,IMAGE_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSigT,0,CLAHE_IMAGE_DATA);
	
	/**
	 *	@brief set/get the bins number of histogram
	 */
	void setHisBins(const int bins_num);
	void getHisBins(int& bins_num);

	/**
	 *	@brief set/get slope clip limit
	 */
	void setClipLimit(const float clip_lim);
	void getClipLimit(float& clip_lim);

	/**
	 *	@brief set/get grids number
	 */
	void setRBlocks(const int blocks_num);
	void getRBlocks(int& blocks_num);

	void setCBlocks(const int blocks_num);
	void getCBlocks(int& blocks_num);

	/**
	 *	@brief execute histogram equalization
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief make self check 
	 *	@note judge whether some preliminary conditions have been satisfied.
	 */
	virtual bool selfcheck();

protected:
/**
*	@brief	According to the ul_clip_limit, clipping the histogram.\n
*	The idea how to distribute excess pixels uniformly is very clever
*/
void clipHistogram (cv:: Mat& hist ,int ul_clip_limit);

/**
 *	@brief find the transformation function(That just\n
 *	is the accumulate histogram)
 */
void mapHistogram (cv:: Mat& hist ,float pixmin,float pixmax, unsigned int ul_nr_of_pixels);

/**
 *	@brief Interpolate gray-level to get CLAHE image
 */
void interpolateCLAHEImage (cv:: Mat& sub_image ,
							const cv::Mat & histLU, const cv ::Mat& histRU,
							const cv ::Mat& histLB,const cv:: Mat& histRB,
							float pixmin, float pixmax );
void clahe (cv:: Mat& image ,
							unsigned int ui_nr_x ,unsigned int ui_nr_y ,
							unsigned int ui_nr_bins,float f_clip_limit);

private:
	CLAHENode(const CLAHENode&);
	void operator=(const CLAHENode&);

	cv::Mat m_input_cvimg;
	int m_histogram_bins;
	float m_clip_limit;
	int m_r_block;
	int m_c_block;
};
}

#include "CLAHENode.hpp"
#endif