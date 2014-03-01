#ifndef _VERTICALSPLITNODE_H_
#define _VERTICALSPLITNODE_H_

#include "EagleeyeMacro.h"
#include "AnyNode.h"
#include "MatrixMath.h"
#include "Matrix.h"
#include "SignalFactory.h"
#include <vector>
#include "Matlab/MatlabInterface.h"
#include "ProcessNode/CLAHENode.h"
#include "BasicMath/MedianFilter1D.h"
#include "EagleeyeCore.h"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
enum SplitDir
{
	HORIZONTAL_SPLIT = 0,
	VERTICAL_SPLIT
};

template<class ImageSigT>
class VerticalHorizontalSplitNode:public AnyNode
{
public:
	typedef VerticalHorizontalSplitNode				Self;
	typedef AnyNode									Superclass;

	typedef typename ImageSigT::MetaType			PixelType;

	VerticalHorizontalSplitNode();
	~VerticalHorizontalSplitNode();
	
	/**
	 *	@brief get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(VerticalHorizontalSplitNode);

	/**
	 *	@brief set input port
	 */
	EAGLEEYE_INPUT_PORT_TYPE(ImageSigT,0,IMAGE_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSignal<ERGB>,0,DISPLAY_SPLIT_IMAGE);

	/**
	 *	@brief get small image piece after splitting
	 *	@note you have to run the whole pipeline
	 */
	Matrix<PixelType> getImage(int index);

	/**
	 *	@brief set/get split direction
	 */
	void setSplitDir(const SplitDir split_dir);
	void getSplitDir(SplitDir& split_dir);
	void setSplitDir(const int split_dir);
	void getSplitDir(int& split_dir);
	
	/**
	 *	@brief set/get split number
	 */
	void setSplitNumber(const int split_num);
	void getSplitNumber(int& split_num);

	/**
	 *	@brief set/get small region limit
	 */
	void setSmallRegionLimit(const int small_region_limit);
	void getSmallRegionLimit(int& small_region_limit);

	/**
	 *	@brief set/get median fitler size
	 */
	void setMedianFilterSize(const int window_size);
	void getMedianFilterSize(int& window_size);

	/**
	 *	@brief execute splitting algorithm
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief check whether some preliminary conditions have been satisfied
	 */
	virtual bool selfcheck();

	/**
	 *	@brief print some unit information
	 */
	virtual void printUnit();

protected:
	/**
	 *	@brief split image along horizontal direction automatically
	 *	@note it would call horizontalSplitForcely() and horizontalMergeForcely()
	 */
	void autoVerticalSplit(const Matrix<PixelType>& split_img);
	void verticalSplitForcely(std::vector<std::pair<int,int>>& small_pieces,int target_pieces_num);
	void verticalMergeForcely(std::vector<std::pair<int,int>>& small_pieces,int target_pieces_num);
	bool verticalSmallRegionMerge(std::vector<std::pair<int,int>>& split_pieces);

	/**
	 *	@brief split image along vertical direction automatically
	 */
	void autoHorizontalSplit(const Matrix<PixelType>& img);
	
	void findingOptimumThrehold(std::vector<float> project_his);

private:
	VerticalHorizontalSplitNode(const VerticalHorizontalSplitNode&);
	void operator=(const VerticalHorizontalSplitNode&);

	SplitDir m_split_dir;

	int m_predefined_split_num;
	float m_optimum_split_threshold;
	int m_small_region_limit;
	int m_median_filter_size;
	int m_neighbor_size;
	std::vector<std::pair<int,int>> m_separate_region;
	Matrix<PixelType> m_img;
};
}

#include "VerticalHorizontalSplitNode.hpp"
#endif