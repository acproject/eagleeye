#ifndef _OBJECTDETGRAMMARTREE_H_
#define _OBJECTDETGRAMMARTREE_H_

#include "EagleeyeMacro.h"

#include "GrammarTree.h"
#include <string>
#include "GrammarUnit.h"
#include "ObjectDetSymbol.h"
#include "ObjectDetNonterminalSymbol.h"
#include "DataPyramid.h"
#include "ProcessNode/ShapeConstrainedNode.h"

namespace eagleeye
{
struct ObjectRegion
{
	int left_bottom_r;
	int left_bottom_c;
	int right_top_r;
	int right_top_c;
};

/**
 *	@brief ObjectDet Grammar Tree(search in PIXEL_SPACE by OPTIMUM_SEARCH mode). \n
 *	this tree uses linear svm classifier
 *	@detail support search space(PIXEL_SPACE,RECT_WINDOW_SPACE and SUPERPIXEL_SPACE) and
 *	search mode(OPTIMUM_MODE,INDEPENDENT_MODE and CONDITIONAL_MODE). 'Grammar Tree' weight 
 *	only support this group 'PIXEL_SPACE - OPTIMUM_MODE and RECT_WINDOW_SPACE - OPTIMUM_MODE'.
 */
class EAGLEEYE_API ObjectDetGrammarTree:public GrammarTree
{
	friend class ObjectDetGrammarTreeTrainer;
public:
	/**
	 *	@brief define some basic type
	 *	@note you must do these
	 */
	typedef ObjectDetGrammarTree							Self;
	typedef GrammarTree										Superclass;

	ObjectDetGrammarTree(const char* name,const char* model_folder,ObjectDetNonterminalSymbol* root = NULL);
	virtual ~ObjectDetGrammarTree();
	
	/**
	 *	@brief (first)initialize the whole grammar tree
	 *	@note 4 steps \n
	 *	(1) load grammar tree structure \n
	 *	(2) load grammar tree weight (not all grammar tree possess weight file)\n
	 *	(3) analyze grammar tree structure info \n
	 *	(4) all nodes in this tree initialize
	 */
	virtual void initialize();

	/**
	 *	@brief get object regions
	 */
	void getObjectRegions(std::vector<ObjectRegion>& regions);

	/**
	 *	@brief parse data
	 */
	virtual void parseData(void* data,int width,int height,void* auxiliary_data);

	/**
	 *	@brief set score threshold
	 */
	void setScoreThreshold(float threshold);
	void getScoreThreshold(float& threshold);

	/**
	 *	@brief set the root of this Grammar Tree
	 */
	virtual void setRootSymbol(Symbol* root);

	/**
	 *	@brief get score pyramid
	 */
	ScorePyramid getScorePyramid();

	/**
	 *	@brief get predict label image
	 */
	Matrix<unsigned char> getPredictMaskImage();

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
	 */
	void enableConstrainedArea(int min_constrained_area = 20);
	void disableConstrainedArea();

	/**
	 *	@brief if enable constrained rectangle, we would check the bounding box 
	 *	of every connected region. If the width height ratio of bounding box is
	 *	not between min_constrained_wh_ratio and max_constrained_wh_ratio, we 
	 *	would remove this region.
	 */
	void enableConstrainedRec(float min_constrained_wh_ratio = 0.0f,float max_constrained_wh_ratio = 1.0f);
	void disableConstrainedRec();

	/**
	 *	@brief train object detection grammar tree
	 */
	virtual void learn(const char* parse_file);

protected:
	/**
	 *	@brief using for training grammar tree model
	 */
	float parseTrainingData(void* data,SampleState sample_state,
							void* auxiliary_data,
							Matrix<float>& gt_sample,
							Matrix<int>& gt_latent_variables);


private:
	ObjectDetNonterminalSymbol* m_det_root_symbol;
	int m_negative_samples;

	std::vector<ObjectRegion> m_output_regions;
	float m_score_threshold;

	ScorePyramid m_score_pyr;
	int m_min_constrained_area;
	float m_constrained_wh_ratio[2];

	bool m_structure_filter_flag;
	bool m_constrained_area_flag;
	bool m_constrained_rec_flag;

	int m_detect_img_width;
	int m_detect_img_height;
};
}

#endif