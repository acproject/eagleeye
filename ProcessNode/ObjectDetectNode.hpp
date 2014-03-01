namespace eagleeye
{
template<class ImageSigT>
ObjectDetectNode<ImageSigT>::ObjectDetectNode()
{
	m_gt_name = "";
	m_gt_model_folder = "";
	m_score_threshold = 0;
	m_gt_detector = NULL;
	m_last_model_update_time = 0;

	m_constrained_rec_flag = false;
	m_constrained_area_flag = false;
	m_structure_filter_flag = false;
	m_min_constrained_area = 0;
	m_constrained_wh_ratio[0] = 0;
	m_constrained_wh_ratio[1] = 0;

	//set input port number
	setNumberOfInputSignals(1);

	//set output port property
	setNumberOfOutputSignals(2);
	setOutputPort(new ImageSignal<unsigned char>,OUTPUT_PORT_MASK_DATA);
	setOutputPort(new ImageSignal<ERGB>,OUTPUT_PORT_IMAGE_WITH_BOX);
	enableOutputPort(OUTPUT_PORT_MASK_DATA);
	disableOutputPort(OUTPUT_PORT_IMAGE_WITH_BOX);
}

template<class ImageSigT>
ObjectDetectNode<ImageSigT>::~ObjectDetectNode()
{
	if ( m_gt_detector )
	{
		delete m_gt_detector;
	}
}

template<class ImageSigT>
void ObjectDetectNode<ImageSigT>::setGrammarTreeModel(char* model_name,char* model_folder)
{
	m_gt_name = model_name;
	m_gt_model_folder = model_folder;
	
	m_model_timestamp.modified();

	//modify this node update time
	modified();
}

template<class ImageSigT>
void ObjectDetectNode<ImageSigT>::passonNodeInfo()
{
	//call the superclass function
	Superclass::passonNodeInfo();

	if ( m_last_model_update_time < m_model_timestamp.getMTime() )
	{
		if ( m_gt_detector )
		{
			delete m_gt_detector;
		}
		m_gt_detector = new ObjectDetGrammarTree( m_gt_name.c_str(),m_gt_model_folder.c_str());

		//initialize grammar tree
		m_gt_detector->initialize();

		//enable some additional operations
		if ( m_structure_filter_flag )
		{
			m_gt_detector->enableStructureFilter();
		}
		else
		{
			m_gt_detector->disableStructureFilter();
		}

		if ( m_constrained_area_flag )
		{
			m_gt_detector->enableConstrainedArea( m_min_constrained_area );
		}
		else
		{
			m_gt_detector->disableConstrainedArea();
		}

		if ( m_constrained_rec_flag )
		{
			m_gt_detector->enableConstrainedRec( m_constrained_wh_ratio[ 0 ], m_constrained_wh_ratio[ 1 ] );
		}
		else
		{
			m_gt_detector->disableConstrainedRec();
		}

		m_last_model_update_time = m_model_timestamp.getMTime();
	}
}

template<class ImageSigT>
void ObjectDetectNode<ImageSigT>::executeNodeInfo( )
{
	Matrix<PixelType> input_img = TO_IMAGE(PixelType,getInputPort(0));

	Matrix<unsigned char> detect_img = input_img.transform<unsigned char>();
	m_gt_detector->parseData( &detect_img,detect_img.cols(),detect_img.rows() );

	ImageSignal<unsigned char>* output_img_sig = TO_IMAGE_SIGNAL(unsigned char,getOutputPort(OUTPUT_PORT_MASK_DATA));
	output_img_sig->img = m_gt_detector->getPredictMaskImage();

	if (m_output_port_state[OUTPUT_PORT_IMAGE_WITH_BOX])
	{
		//merge image with predicting object region
		Matrix<ERGB> rgb_img = detect_img.transform<ERGB>();
		std::vector<ObjectRegion> predict_object_regions;
		m_gt_detector->getObjectRegions(predict_object_regions);

		ERGB red_color;
		red_color[0] = 255;red_color[1] = 0; red_color[2] = 0;
		int regions_num = predict_object_regions.size();
		for (int region_index = 0; region_index < regions_num; ++region_index)
		{
			ObjectRegion region = predict_object_regions[region_index];
			drawRect(rgb_img,region.left_bottom_c,region.left_bottom_r,(region.right_top_c - region.left_bottom_c),(region.right_top_r - region.left_bottom_r),red_color);
		}

		ImageSignal<ERGB>* output_fusion_img = TO_IMAGE_SIGNAL(ERGB,getOutputPort(OUTPUT_PORT_IMAGE_WITH_BOX));
		output_fusion_img->img = rgb_img;
	}
}

template<class ImageSigT>
void ObjectDetectNode<ImageSigT>::getObjectRegions( std::vector<ObjectRegion>& object_regions )
{
	m_gt_detector->getObjectRegions( object_regions );
}

template<class ImageSigT>
void ObjectDetectNode<ImageSigT>::getPredictMaskImg( Matrix<unsigned char>& img )
{
	img = m_gt_detector->getPredictMaskImage( );
}

template<class ImageSigT>
bool ObjectDetectNode<ImageSigT>::selfcheck()
{
	Superclass::selfcheck();

	ImageSigT* input_img_sig = TO_IMAGE_SIGNAL(PixelType,getInputPort());

	if ( !input_img_sig )
	{
		EAGLEEYE_ERROR( "sorry, image pixel isn't consistent ...\n please be careful... \n" );
		return false;
	}

	if ( m_gt_name == "" )
	{
		EAGLEEYE_ERROR( "sorry, grammar tree name is empty... \n" );
		return false;
	}

	if ( m_gt_model_folder == "" )
	{
		EAGLEEYE_ERROR( "sorry, grammar tree model folder is empty... \n" );
		return false;
	}

	return true;
}

template<class ImageSigT>
void ObjectDetectNode<ImageSigT>::enableStructureFilter( )
{
	m_structure_filter_flag = true;
}
template<class ImageSigT>
void ObjectDetectNode<ImageSigT>::disableStructureFilter()
{
	m_structure_filter_flag = false;
}

template<class ImageSigT>
void ObjectDetectNode<ImageSigT>::enableConstrainedArea(int min_constrained_area /* = 20 */)
{
	m_constrained_area_flag = true;
	m_min_constrained_area = min_constrained_area;
}
template<class ImageSigT>
void ObjectDetectNode<ImageSigT>::disableConstrainedArea()
{
	m_constrained_area_flag = false;
}

template<class ImageSigT>
void ObjectDetectNode<ImageSigT>::enableConstrainedRec(float min_constrained_wh_ratio /* = 0.0f */,float max_constrained_wh_ratio /* = 1.0f */)
{
	m_constrained_rec_flag = true;
	m_constrained_wh_ratio[0] = min_constrained_wh_ratio;
	m_constrained_wh_ratio[1] = max_constrained_wh_ratio;
}
template<class ImageSigT>
void ObjectDetectNode<ImageSigT>::disableConstrainedRec()
{
	m_constrained_rec_flag = false;
}
}