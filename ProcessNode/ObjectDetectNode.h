#ifndef _GRAMMARTREEDETECTNODE_H_
#define _GRAMMARTREEDETECTNODE_H_

#include "EagleeyeMacro.h"
#include <string>
#include "ObjectDetGrammarTree.h"
#include "SignalFactory.h"
#include "AnyNode.h"
#include "Array.h"
#include "EagleeyeCore.h"

namespace eagleeye
{
template<class ImageSigT>
class ObjectDetectNode:public AnyNode
{
public:
	typedef ObjectDetectNode													Self;
	typedef AnyNode																Superclass;

	typedef typename ImageSigT::MetaType										PixelType;

	ObjectDetectNode();
	virtual ~ObjectDetectNode();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(ObjectDetectNode);

	/**
	 *	@brief set input and output port
	 */
	EAGLEEYE_INPUT_PORT_TYPE(ImageSigT,0,IMAGE_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSignal<unsigned char>,0,MASK_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSignal<ERGB>,1,IMAGE_WITH_BOX);

	/**
	 *	@brief if enable structure filter, we would convolve the score
	 *	image with 3*3 kernel
	 *	@note only for detecting texture target
	 */
	void enableStructureFilter();
	void disableStructureFilter();

	/**
	 *	@brief if enable constrained area, we would check every 
	 *	connected region. If the area of connected region is less than
	 *	min_constrained_area, we would remove this region.
	 *	@note only for detecting texture target, such as cigarette
	 */
	void enableConstrainedArea(int min_constrained_area = 20);
	void disableConstrainedArea();

	/**
	 *	@brief if enable constrained rectangle, we would check the bounding box 
	 *	of every connected region. If the width height ratio of bounding box is
	 *	not between min_constrained_wh_ratio and max_constrained_wh_ratio, we 
	 *	would remove this region.
	 *	@note only for detecting texture target, such as cigarette
	 */
	void enableConstrainedRec(float min_constrained_wh_ratio = 0.0f,float max_constrained_wh_ratio = 1.0f);
	void disableConstrainedRec();

	/**
	 *	@brief set grammar tree model name and model folder
	 */
	void setGrammarTreeModel(char* model_name,char* model_folder);

	/**
	 *	@brief get object region output
	 */
	void getObjectRegions(std::vector<ObjectRegion>& object_regions);

	/**
	 *	@brief get predict mask image
	 */
	void getPredictMaskImg(Matrix<unsigned char>& img);

	/**
	 *	@brief this function would be called automatically
	 *	@note In this function, we should implement some prepare
	 *	work, such as load model, set parameters or other things. 
	 *	But, we couldn't use input data in this function.
	 */
	virtual void passonNodeInfo();

	/**
	 *	@brief detect the input image and get predicated mask image
	 *	@note This function would be called automatically. In this 
	 *	function, we should run algorithm to process image.
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief detect whether this algorithm node has possessed all 
	 *	preliminary parameters.
	 */
	virtual bool selfcheck();

private:
	ObjectDetectNode(const ObjectDetectNode&);
	void operator=(const ObjectDetectNode&);

	std::string m_gt_name;
	std::string m_gt_model_folder;
	float m_score_threshold;
	ObjectDetGrammarTree* m_gt_detector;

	EagleeyeTimeStamp m_model_timestamp;
	unsigned long m_last_model_update_time;

	bool m_structure_filter_flag;
	bool m_constrained_area_flag;
	bool m_constrained_rec_flag;

	int m_min_constrained_area;
	float m_constrained_wh_ratio[2];
};
}

#include "ObjectDetectNode.hpp"
#endif