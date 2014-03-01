#ifndef _RECOGNIZATIONSTATISTICSNODE_H_
#define _RECOGNIZATIONSTATISTICSNODE_H_

#include "EagleeyeMacro.h"
#include "AnyNode.h"
#include <string>
#include "ImageReadNode.h"
#include "MatrixMath.h"
#include "Print.h"

namespace eagleeye
{
enum StatisticPlan
{
	IMAGE_COMPARE_BASED_PIXEL_STA,
	IMAGE_COMPARE_BASED_REGION_STA,
	LABEL_COMPARE_STA,
	LABEL_TOP_HIT_5,
	LABEL_TOP_HIT_3,
	MULTI_OBJECTS_STA,
	SINGLE_OBJECT_STA
};

template<typename T>
struct DefaultTransform
{
	int operator()(T label)
	{
		return (int)label;
	}
};

template<class SrcT,class TransformT = DefaultTransform<typename SrcT::MetaType>>
class PerformanceEvaluationNode:public AnyNode
{
public:
	/**
	 *	@brief define some basic type
	 *	@note you must do these
	 */
	typedef PerformanceEvaluationNode						Self;
	typedef AnyNode										Superclass;

	typedef typename SrcT::MetaType						MetaType;

	PerformanceEvaluationNode();
	virtual ~PerformanceEvaluationNode();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(PerformanceEvaluationNode);
	
	/**
	 *	@brief define input port image signal type
	 */
	EAGLEEYE_INPUT_PORT_TYPE(SrcT,0,CLASSIFY_DATA);

	/**
	 *	@brief save statistics info
	 */
	void saveStatisticsInfo(const char* file_path);

	/**
	 *	@brief get some statistic quantity
	 *	@detail recall factor	=	det_p_p	/ p			\n
	 *	omission detection		=	det_n_p / p			\n
	 *	miss detection			=	det_p_n	/ det_p		\n
	 *	det_p					=	det_p_n + det_p_p	\n
	 */
	float getRecallRatio(int label_index = 0);
	float getOmissionRatio(int label_index = 0);
	float getMissRatio(int label_index = 0);

	/**
	 *	@brief set statistic object
	 */
	void setStatisticPlan(StatisticPlan sta_object);

	/**
	 *	@brief set how to search groundtruth image
	 */
	void setHowToSearchGroundTruthImg(char* folder,char* ext_str);

	/**
	 *	@brief set object variety
	 */
	void setLabelsNum(int num);

	/**
	 *	@brief process node info
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief print some key info possessed by this node
	 *	@detail recall ratio, omission ratio and miss ratio
	 */
	virtual void printUnit();

protected:
	/**
	 *	@brief get ground truth image
	 */
	Matrix<MetaType> getGroundTruthImg(std::string file_path);

	/**
	 *	@brief implement self check
	 *	@note 
	 */
	virtual bool selfcheck();

	/**
	 *	@brief compute predict image and groundtruth image
	 */
	bool imageCompareBasedPixelStatis();

	/**
	 *	@brief compare predict compare label and groundtruth label
	 */
	bool labelCompareStatis();

	/**
	 *	@brief top hit statis
	 */
	bool labelTopHitStatis(int k_top_hit = 3);

	/**
	 *	@brief update statistics
	 */
	void updateMissRecallOmissStatis();

private:
	PerformanceEvaluationNode(const PerformanceEvaluationNode&);
	void operator=(const PerformanceEvaluationNode&);

	StatisticPlan m_sta_plan;								/**< statistic object*/

	std::string m_search_folder;							/**< search groundtruth image*/
	std::string m_ext_str;									/**< search groundtruth image*/
	
	Matrix<int> m_statistic_matrix;							/**< predict -- groundtruth(for MISS_RECALL_OMISS)*/
	ImageReadNode<SrcT>* m_image_read_node;

	DynamicArray<float> m_miss_ratio;						/**< cumulative statistical quantity*/
	DynamicArray<float> m_recall_ratio;						/**< cumulative statistical quantity*/
	DynamicArray<float> m_omission_ratio;					/**< cumulative statistical quantity*/
	int m_cumulative_count;
	
	float m_error_ratio;

	int m_labels_num;
};
}

#include "PerformanceEvaluationNode.hpp"
#endif