#ifndef _OBJECTCLASSIFYNODE_H_
#define _OBJECTCLASSIFYNODE_H_

#include "EagleeyeMacro.h"
#include "ObjectDetSuperpixelGrammarTree.h"
#include "AnyNode.h"
#include "SignalFactory.h"
#include "ProcessNode/SRMNode.h"
#include <string>

namespace eagleeye
{
enum OutputMode
{
	MAX_RESPONSE_OUTPUT,
	TOP_HIT_3_OUTPUT,
	TOP_HIT_5_OUTPUT
};

template<class ImageSigT>
class ObjectDetSuperpixelNode:public AnyNode
{
public:
	typedef ObjectDetSuperpixelNode							Self;
	typedef AnyNode										Superclass;

	typedef typename ImageSigT::MetaType				PixelType;

	ObjectDetSuperpixelNode();
	~ObjectDetSuperpixelNode();

	/**
	 *	@brief get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(ObjectDetSuperpixelNode);

	/**
	 *	@brief set input and output port
	 */
	EAGLEEYE_INPUT_PORT_TYPE(ImageSigT,						0,			IMAGE_DATA);
	EAGLEEYE_INPUT_PORT_TYPE(ImageSignal<int>,				1,			SUPERPIXEL_IMAGE_DATA);
	EAGLEEYE_INPUT_PORT_TYPE(ImageSignal<int>,				2,			SUPERPIXEL_NUM_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSignal<unsigned char>,	0,			MAX_RESPONSE_LABEL_DATA);

	/**
	 *	@brief set/get object classify tree name 
	 */
	void setGrammarTreeName(const std::string tree_name);
	void getGrammarTreeName(std::string& tree_name);

	/**
	 *	@brief set/get folder holding model
	 */
	void setModelFolder(const std::string model_folder);
	void getModelFolder(std::string& model_folder);

	/**
	 *	@brief get invalid label of classify image
	 */
	unsigned char getInvalidLabel();

	/**
	 *	@brief set output mode
	 */
	void setOutputMode(OutputMode mode);

	/**
	 *	@brief this function would be called automatically
	 *	@note In this function, we should implement some prepare work,
	 *	such as load model, set parameters or other things.
	 *	But, we couldn't use input data in this function
	 */
	virtual void passonNodeInfo();

	/**
	 *	@brief detect the input image and get predicated classify label
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief check whether this algorithm node has possessed all preliminary parameters
	 */
	virtual bool selfcheck();

protected:
	/**
	 *	@brief for MAX_RESPONSE_OUTPUT
	 */
	void outputMaxResponseLabel(const Matrix<int>& superpixel_img,const Matrix<float>& predict_label);

private:
	ObjectDetSuperpixelNode(const ObjectDetSuperpixelNode&);
	void operator=(const ObjectDetSuperpixelNode&);

	ObjectDetSuperpixelGrammarTree* m_gt_detector;

	std::string m_gt_name;
	std::string m_gt_model_folder;
	
	OutputMode m_output_mode;

	bool m_model_update_flag;
	AuxiliaryInfoInSuperpixelSpace m_auxiliary_info;
};
}

#include "ObjectDetSuperpixelNode.hpp"
#endif