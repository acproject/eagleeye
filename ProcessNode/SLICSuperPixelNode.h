#ifndef _SLICSUPERPIXELNODE_H_
#define _SLICSUPERPIXELNODE_H_

#include "EagleeyeMacro.h"
#include "AnyNode.h"
#include "MatrixAuxiliary.h"
#include "MetaOperation.h"
#include "MatrixMath.h"
#include "ColorSpace.h"

namespace eagleeye
{
/**
 *	@brief generate superpixel (over segmentation)
 *	@detail This code implements the superpixel method described in:\n
 *	Radhakrishna Achanta, Appu Shaji, Kevin Smith, Aurelien Lucchi, Pascal Fua, and Sabine Susstrunk,\n
 *  "SLIC Superpixels"
 *  @note there are some small modification. Now, it is valid for image with single or three channels 
 */
template<class ImageSigT>
class SLICSuperPixelNode:public AnyNode
{
public:
	/**
	 *	@brief define some basic type
	 *	@note you must do these
	 */
	typedef SLICSuperPixelNode								Self;
	typedef AnyNode											Superclass;

	typedef typename ImageSigT::MetaType					PixelType;

	SLICSuperPixelNode();
	virtual ~SLICSuperPixelNode();
	
	EAGLEEYE_CLASSIDENTITY(SLICSuperPixelNode);

	/**
	 *	@brief define input and output image signal type
	 */
	EAGLEEYE_INPUT_PORT_TYPE(ImageSigT,					0,		IMAGE_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSignal<int>,			0,		LABEL_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSignal<PixelType>,	1,		SUPERPIXEL_IMAGE_DATA);

	/**
	 *	@brief get image comprised with superpixels
	 *	@note Firstly, you have to upate the pipeline. Otherwise, you couldn't 
	 *	get anything.
	 */
	Matrix<PixelType> getSuperPixelImage();

	/**
	 *	@brief get label map
	 */
	Matrix<int> getLableImage();
	
	/**
	 *	@brief get superpixel center
	 */
	Matrix<float> getSuperpixelCenter();

	/**
	 *	@brief set/get superpixels number
	 */
	void setSuperPixelsNum(const int num);
	void getSuperPixelsNum(int& num);
	int getSuperPixelsNum();

	/**
	 *	@brief set/get superpixel size
	 */
	void setSuperPixelSize(const int size);
	void getSuperPixelSize(int& size);
	
	/**
	 *	@brief set/get compactness
	 */
	void setCompactness(const float compactness);
	void getCompactness(float& compactness);

	/**
	 *	@brief execute SLIC algorithm
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief make self check
	 *	@note judge whether some preliminary conditions have been satisfied.
	 */
	virtual bool selfcheck();
	
protected:
	/**
	 *	@brief superpixel segmentation for a given step size 
	 *	(superpixel size~=step*step)
	 *	@note the input image is RGB or gray\n
	 *	compactness value depends on the input pixels values. For instance, if the
	 *	input is grayscale with values ranging from 0~100, then a compactness value
	 *	of 20.0 would give good results. A greater value will make the superpixels more
	 *	compact while a smaller value would make them more uneven.
	 */
	template<class SpecialT>
	void doSuperpixelSegmentation(const Matrix<SpecialT>& gray_img)
	{
		//transform to 0~1
		PixelType max_val,min_val;
		getMaxMin(gray_img,max_val,min_val);
		Matrix<float> normalied_gray_img=
			gray_img.transform<NormalizeOperation<PixelType,float>>(NormalizeOperation<PixelType,float>(min_val,max_val,0.0f,1.0f));

		if (m_k_superpixles != 0)
		{
			m_size_superpixel=int(gray_img.rows()*gray_img.cols()/m_k_superpixles);
		}

		//------------------------------------------------
		const int step = int(sqrt(float(m_size_superpixel))+0.5);
		//------------------------------------------------
		std::vector<float> kseedsgray(0);
		std::vector<float> kseedsx(0);
		std::vector<float> kseedsy(0);

		//--------------------------------------------------
		int width=normalied_gray_img.cols();
		int height=normalied_gray_img.rows();
		int sz = width*height;
		//klabels.resize( sz, -1 );
		//--------------------------------------------------
		m_label_img=Matrix<int>(height,width,(int)0);
		int* klabels=m_label_img.dataptr();

		for( int s = 0; s < sz; s++ ) klabels[s] = -1;

		//get lab xy seeds
		getGrayXYSeeds(kseedsgray,kseedsx,kseedsy,step,normalied_gray_img);

		//generate superpixel by using seeds
		performSuperpixelSLIC(kseedsgray,kseedsx,kseedsy,m_label_img,step,normalied_gray_img,m_compactness);
		m_label_num=kseedsgray.size();
		

		//remove some small regions
		Matrix<int> update_label;
		enforceLabelConnectivity(m_label_img,int(float(width*height)/float(step*step)),update_label,m_label_num);

		m_label_img=update_label;

		m_superpixel_img=
			normalied_gray_img.transform<NormalizeOperation<float,PixelType>>(NormalizeOperation<float,PixelType>(0.0f,1.0f,min_val,max_val));
	}
	template<>
	void doSuperpixelSegmentation<ERGB>(const Matrix<ERGB>& rgb_img)
	{
		if (m_k_superpixles != 0)
		{
			m_size_superpixel=int(rgb_img.rows()*rgb_img.cols()/m_k_superpixles);
		}

		//------------------------------------------------
		const int step = sqrt(float(m_size_superpixel))+0.5;
		//------------------------------------------------
		std::vector<float> kseedsl(0);
		std::vector<float> kseedsa(0);
		std::vector<float> kseedsb(0);
		std::vector<float> kseedsx(0);
		std::vector<float> kseedsy(0);

		//--------------------------------------------------
		int width=rgb_img.cols();
		int height=rgb_img.rows();
		int sz = width*height;
		//klabels.resize( sz, -1 );
		//--------------------------------------------------
		m_label_img=Matrix<int>(height,width,(int)0);
		int* klabels=m_label_img.dataptr();

		for( int s = 0; s < sz; s++ ) klabels[s] = -1;

		//transform from rgb space to lab space
		Matrix<Array<float,3>> lab=rgb2lab(rgb_img);

		Matrix<float> edge_mag;
		if (m_perturb_flag)
		{
			edge_mag=computeGradientMag(lab);
		}

		//get lab xy seeds
		getLABXYSeeds(kseedsl,kseedsa,kseedsb,kseedsx,kseedsy,step,edge_mag,lab,m_perturb_flag);

		performSuperpixelSLIC(kseedsl,kseedsa,kseedsb,kseedsx,kseedsy,m_label_img,step,edge_mag,lab,m_compactness);
		m_label_num=kseedsl.size();

		Matrix<int> update_label;
		enforceLabelConnectivity(m_label_img,int(float(width*height)/float(step*step)),update_label,m_label_num);

		m_label_img=update_label;
	}

	/**
	 *	@brief main SLIC algorithm for generating superpixels
	 */
	void performSuperpixelSLIC(std::vector<float>& kseedsl,
		std::vector<float>& kseedsa,
		std::vector<float>& kseedsb,
		std::vector<float>& kseedsx,
		std::vector<float>& kseedsy,
		Matrix<int>& labels,
		const int step,
		const Matrix<float>& edge_mag,
		const Matrix<Array<float,3>>& lab,
		const float compactness);

	/**
	 *	@brief pick seeds for superpixels when step size of superpixels is given
	 */
	void getLABXYSeeds(std::vector<float>& kseedsl,
		std::vector<float>& kseedsa,
		std::vector<float>& kseedsb,
		std::vector<float>& kseedsx,
		std::vector<float>& kseedsy,
		const int step,
		const Matrix<float>& edge_mag,
		const Matrix<Array<float,3>>& lab,
		const bool perturb_seeds_flag);

	/**
	 *	@brief perturb seed by using edge gradient magnitude
	 */
	void perturbLABSeeds(const Matrix<Array<float,3>>& lab,
		const Matrix<float>& edge_mag,
		std::vector<float>& kseedsl,
		std::vector<float>& kseedsa,
		std::vector<float>& kseedsb,
		std::vector<float>& kseedsx,
		std::vector<float>& kseedsy);

	/**
	 *	@brief main SLIC algorithm for generating superpixels
	 *	@note for image with single channel
	 */
	void performSuperpixelSLIC(std::vector<float>& kseedsgray,
		std::vector<float>& kseedsx,
		std::vector<float>& kseedsy,
		Matrix<int>& labels,
		const int step,
		const Matrix<float>& gray,
		const float compactness);

	/**
	 *	@brief get GrayXY seeds
	 *	@note for image with single channel
	 */
	void getGrayXYSeeds(std::vector<float>& kseedsgray,
		std::vector<float>& kseedsx,
		std::vector<float>& kseedsy,
		const int step,
		const Matrix<float>& gray);

private:
	SLICSuperPixelNode(const SLICSuperPixelNode&);
	void operator=(const SLICSuperPixelNode&);

	float m_compactness;	/**< one parameter used in SLIC*/
	int m_k_superpixles;	/**< required number of superpixels*/
	int m_size_superpixel;	/**< required size of one supersize*/

	bool m_perturb_flag;

	Matrix<int> m_label_img;
	Matrix<PixelType> m_superpixel_img;
	int m_label_num;
	Matrix<float> m_superpixel_center;
};
}

#include "SLICSuperPixelNode.hpp"
#endif