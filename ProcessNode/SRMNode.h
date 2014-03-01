#ifndef _SRMNODE_H_
#define _SRMNODE_H_

#include "AnyNode.h"
#include "MetaOperation.h"
#include "MatrixMath.h"
#include "SignalFactory.h"
#include "Variable.h"

namespace eagleeye
{
template<class ImageSigT>
class SRMNode:public AnyNode
{
public:
	/**
	 *	@brief define some basic type
	 *	@note you must do these
	 */
	typedef SRMNode											Self;
	typedef	AnyNode											Superclass;

	typedef typename ImageSigT::MetaType					PixelType;

	SRMNode();
	virtual ~SRMNode();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(SRMNode);
	
	/**
	 *	@brief define input and output image signal type
	 */
	EAGLEEYE_INPUT_PORT_TYPE(ImageSigT,					0,		IMAGE_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSignal<int>,			0,		LABEL_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSignal<float>,		1,		SEGMENTATION_IMAGE_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSignal<int>,			2,		NUMBER_DATA);

	/**
	 *	@brief set/get some algorithm parameter
	 */
	void setDistributionComplexity(const float f_complex=10.0f);
	void getDistributionComplexity(float& f_complex);

	/**
	 *	@brief Get the segmentation label map
	 *	@note Firstly,you have to run the pipeline update. Otherwise,
	 *	you couldn't get anything
	 */
	Matrix<int> getLabelMap();

	/**
	 *	@brief get the segmentation gray map
	 *	@note Firstly, you have to run the pipeline update. Otherwise,
	 *	you couldn't get anything.
	 */
	Matrix<float> getGrayMap();

	/**
	 *	@brief get pixel number of every segmentation
	 */
	Matrix<int> getPixelNumOfSegmentation();

	/**
	 *	@brief get segmentation centers(x,y)
	 */
	Matrix<float> getSegmentationCenters();

	/**
	 *	@brief get the number of labels
	 *	@note Firstly, you have to run the pipeline update. Otherwise,
	 *	you couldn't get anything.
	 */
	int getLabelNum();

	/**
	 *	@brief set/get parameter block
	 */
	virtual void setUnitPara(MemoryBlock param_block);
	virtual void getUnitPara(MemoryBlock& param_block);

	/**
	 *	@brief execute segmentation algorithm
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief make self check
	 *	@note judge whether some preliminary conditions have been satisfied.
	 */
	virtual bool selfcheck();

	/**
	 *	@brief print node info
	 */
	virtual void printUnit();

protected:
	int initialize(int n_width, int n_height,int n_depth = 1);
	int process2D(unsigned char* puch_src,int* pui_label,unsigned int& ui_region_count,float* pf_average = NULL);

	void srm2D(int* pui_label,float* pf_average = NULL);
	void initRegions2D();

	void addNeighborPair(int n_neighbor_index, int i1, int i2);
	void initNeighbors2D();
	int getRegionIndex(int i);
	bool predicate(int i1, int i2);
	void mergeAllNeighbors2D(); 
	void mergeRegions(int i1, int i2);
	int consolidateRegions();

	/**
	 *	@brief clear some temp memory
	 */
	void recycleGarbage();

private:
	SRMNode(const SRMNode&);
	void operator=(const SRMNode&);

	int m_n_width, m_n_height, m_n_pixel_number, m_n_depth;
	int m_n_img_pixel_number, m_n_region_count;
	unsigned char* m_puch_srcptr;

	float m_fg; // number of different intensity values
	float m_fq; //25; // complexity of the assumed distributions
	float m_f_delta;

	/*
	 * The predicate: is the difference of the averages of the two
	 * regions R and R' smaller than
	 *
	 * g sqrt(1/2Q (1/|R| + 1/|R'|) ln 2/delta)
	 *
	 * Instead of calculating the square root all the time, we calculate
	 * the factor g^2 / 2Q ln 2/delta, and compare
	 *
	 * (<R> - <R'>)^2 < factor (1/|R| + 1/|R'|)
	 *
	 * instead.
	 */
	float m_f_factor, m_f_log_delta;

	/*
	 * For performance reasons, these are held in w * h arrays
	 */
	float*	m_pf_average;
	int*	m_pn_count;
	int*	m_pn_region_index; // if < 0, it is -1 - actual_regionIndex

	/*
	 * The statistical region merging wants to merge regions in a specific
	 * order: for all neighboring pixel pairs, in ascending order of
	 * intensity differences.
	 *
	 * In that order, it is tested if the regions these two pixels belong
	 * to (by construction, the regions must be distinct) should be merged.
	 *
	 * For efficiency, we do it by bucket sorting, because there are only
	 * g+1 possible differences.
	 *
	 * After sorting, for each difference the pixel pair with the largest
	 * index is stored in neighbor_buckets[difference], and every
	 * next_neighbor[index] points to the pixel pair with the same
	 * difference and the next smaller index (or -1 if there is none).
	 *
	 * The pixel pairs are identified by
	 *
	 *     2*(x+(w-1)*y) + direction
	 *
	 * where direction = 0 means "right neighbor", and direction = 1 means
	 * " lower neighbor".  (We do not need "left" or "up", as the order
	 * within the pair is not important.)
	 *
	 * In n dimensions, it must be n * pixel_count, and "direction"
	 * specifies the Cartesian unit vector (axis) determining the neighbor.
	 */
	int* m_pn_next_neighbor;
	int* m_pn_neighbor_bucket;

	Matrix<unsigned char> m_input_img;
	Matrix<int> m_label_map;
	Matrix<float> m_gray_map;
};
}

#include "SRMNode.hpp"
#endif