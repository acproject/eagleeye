namespace eagleeye
{
template<class ImageSigT>
ObjectDetSuperpixelNode<ImageSigT>::ObjectDetSuperpixelNode()
{
	m_gt_detector = NULL;
	m_model_update_flag = true;
	
	m_output_mode = MAX_RESPONSE_OUTPUT;

	m_auxiliary_info.superpixel_pyr.create(1);
	m_auxiliary_info.superpixel_num_pyr.resize(1);
	//set input port number
	setNumberOfInputSignals(3);

	//set output port property
	setNumberOfOutputSignals(1);
	setOutputPort(new ImageSignal<unsigned char>,OUTPUT_PORT_MAX_RESPONSE_LABEL_DATA);

	//////////////////////////////////////////////////////////////////////////
	EAGLEEYE_MONITOR_VAR(std::string,setGrammarTreeName,getGrammarTreeName,"classify_tree_name");
	EAGLEEYE_MONITOR_VAR(std::string,setModelFolder,getModelFolder,"model_folder");
}
template<class ImageSigT>
ObjectDetSuperpixelNode<ImageSigT>::~ObjectDetSuperpixelNode()
{
	if (m_gt_detector)
	{
		delete m_gt_detector;
	}
}

template<class ImageSigT>
void ObjectDetSuperpixelNode<ImageSigT>::passonNodeInfo()
{
	if (m_model_update_flag)
	{
		if (m_gt_detector)
		{
			delete m_gt_detector;
		}
		m_gt_detector = new ObjectDetSuperpixelGrammarTree(m_gt_name.c_str(),m_gt_model_folder.c_str());
		m_gt_detector->initialize();
		
		m_model_update_flag = false;
	}

	Superclass::passonNodeInfo();
}

template<class ImageSigT>
void ObjectDetSuperpixelNode<ImageSigT>::executeNodeInfo()
{
	Matrix<PixelType> input_img = TO_IMAGE(PixelType,getInputPort(INPUT_PORT_IMAGE_DATA));
	Matrix<int>	superpixel_img = TO_IMAGE(int,getInputPort(INPUT_PORT_SUPERPIXEL_IMAGE_DATA));
	Matrix<int> superpixel_num = TO_IMAGE(int,getInputPort(INPUT_PORT_SUPERPIXEL_NUM_DATA));
	
	putToMatlab(input_img,"input_img");
	putToMatlab(superpixel_img,"superpixel_img");

	m_auxiliary_info.superpixel_pyr[0] = superpixel_img;
	m_auxiliary_info.superpixel_num_pyr[0] = superpixel_num(0);

	Matrix<unsigned char> object_img = input_img.transform<unsigned char>();
	m_gt_detector->parseData(&object_img,object_img.cols(),object_img.rows(),&m_auxiliary_info);
	Matrix<float> predict_label = m_gt_detector->getPredictLabel();

	switch(m_output_mode)
	{
	case MAX_RESPONSE_OUTPUT:
		{
			outputMaxResponseLabel(superpixel_img,predict_label);
			break;
		}
	}
}

template<class ImageSigT>
bool ObjectDetSuperpixelNode<ImageSigT>::selfcheck()
{
	Superclass::selfcheck();

	if (!TO_IMAGE_SIGNAL(PixelType,getInputPort(INPUT_PORT_IMAGE_DATA)))
	{
		EAGLEEYE_ERROR("sorry,data type is not consistent...\n");
		return false;
	}

	return true;
}

template<class ImageSigT>
void ObjectDetSuperpixelNode<ImageSigT>::setGrammarTreeName(const std::string tree_name)
{
	m_gt_name = tree_name;
	m_model_update_flag = true;
}
template<class ImageSigT>
void ObjectDetSuperpixelNode<ImageSigT>::getGrammarTreeName(std::string& tree_name)
{
	tree_name = m_gt_name;
}

template<class ImageSigT>
void ObjectDetSuperpixelNode<ImageSigT>::setModelFolder(const std::string model_folder)
{
	m_gt_model_folder = model_folder;
	m_model_update_flag = true;
}
template<class ImageSigT>
void ObjectDetSuperpixelNode<ImageSigT>::getModelFolder(std::string& model_folder)
{
	model_folder = m_gt_model_folder;
}

template<class ImageSigT>
unsigned char ObjectDetSuperpixelNode<ImageSigT>::getInvalidLabel()
{
	return m_gt_detector->getInvalidLabel();
}

template<class ImageSigT>
void ObjectDetSuperpixelNode<ImageSigT>::setOutputMode(OutputMode mode)
{
	m_output_mode = mode;
}

template<class ImageSigT>
void ObjectDetSuperpixelNode<ImageSigT>::outputMaxResponseLabel(const Matrix<int>& superpixel_img,const Matrix<float>& predict_label)
{
	int rows = superpixel_img.rows();
	int cols = superpixel_img.cols();
	Matrix<unsigned char> max_response_label(rows,cols);

	for (int i = 0; i < rows; ++i)
	{
		unsigned char* det_result_data = max_response_label.row(i);
		const int* superpixel_index = superpixel_img.row(i);
		for (int j = 0; j < cols; ++j)
		{
			det_result_data[j] = unsigned char(predict_label(superpixel_index[j],SUPERPIXEL_SCORE_OFFSET));
		}
	}

	ImageSignal<unsigned char>* output_img_sig = TO_IMAGE_SIGNAL(unsigned char,getOutputPort(OUTPUT_PORT_MAX_RESPONSE_LABEL_DATA));
	output_img_sig->img = max_response_label;
}

}