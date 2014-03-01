namespace eagleeye
{
template<class SrcT,class TransformT>
PerformanceEvaluationNode<SrcT,TransformT>::PerformanceEvaluationNode()
{
	//generate input signal
	setNumberOfInputSignals(1);
	
	//make one invalid output port
	setNumberOfOutputSignals(1);
	setOutputPort(makeOutputSignal(),0);

	m_sta_plan = IMAGE_COMPARE_BASED_PIXEL_STA;
	m_search_folder = "";
	m_ext_str = "";
	m_image_read_node = new ImageReadNode<SrcT>;

	m_cumulative_count = 0;
	m_labels_num = 0;

	m_error_ratio = 0.0f;
}

template<class SrcT,class TransformT>
PerformanceEvaluationNode<SrcT,TransformT>::~PerformanceEvaluationNode()
{
	if (m_image_read_node)
	{
		delete m_image_read_node;
	}
}

template<class SrcT,class TransformT>
void PerformanceEvaluationNode<SrcT,TransformT>::saveStatisticsInfo(const char* file_path)
{
	//save statistic information
	if (m_sta_plan == LABEL_COMPARE_STA)
	{
		EagleeyeIO statistic_info_io;
		statistic_info_io.createWriteHandle(file_path,false);

		statistic_info_io.write(m_miss_ratio,"miss ratio");
		statistic_info_io.write(m_recall_ratio,"recall ratio");
		statistic_info_io.write(m_omission_ratio,"omission ratio");

		statistic_info_io.destroyHandle();

		return;
	}

	//save statistic information
	if (m_sta_plan == LABEL_TOP_HIT_3 || m_sta_plan == LABEL_TOP_HIT_5)
	{
		FILE* fp = NULL;
		fp = fopen(file_path,"w+");
		print_info(fp," error ratio %f\n hit ratio %f",m_error_ratio,1.0f - m_error_ratio);
		fclose(fp);

		return;
	}

	//save statistic information
	if (m_sta_plan == IMAGE_COMPARE_BASED_PIXEL_STA)
	{
		std::vector<int> pixels_num(m_labels_num,0);
		
		for (int predict_index = 0; predict_index < m_labels_num; ++predict_index)
		{
			for (int groundtruth_index = 0; groundtruth_index < m_labels_num; ++groundtruth_index)
			{
				pixels_num[predict_index] += m_statistic_matrix(predict_index,groundtruth_index);
			}
		}

		Matrix<float> probability_mat(m_labels_num,m_labels_num,float(0.0f));
		for (int predict_index = 0; predict_index < m_labels_num; ++predict_index)
		{
			for (int groundtruth_index = 0; groundtruth_index < m_labels_num; ++groundtruth_index)
			{
				probability_mat(predict_index,groundtruth_index) = float(m_statistic_matrix(predict_index,groundtruth_index)) / float(pixels_num[predict_index]);
			}
		}

		EagleeyeIO::write(probability_mat,file_path,WRITE_ASCII_MODE);

		return;
	}
}

template<class SrcT,class TransformT>
void PerformanceEvaluationNode<SrcT,TransformT>::executeNodeInfo()
{
	switch(m_sta_plan)
	{
	case IMAGE_COMPARE_BASED_PIXEL_STA:
		{
			imageCompareBasedPixelStatis();
			break;
		}
	case LABEL_COMPARE_STA:
		{
			labelCompareStatis();
			break;
		}
	case LABEL_TOP_HIT_3:
		{
			labelTopHitStatis(3);
			break;
		}
	case LABEL_TOP_HIT_5:
		{
			labelTopHitStatis(5);
			break;
		}
	}
}

template<class SrcT,class TransformT>
void PerformanceEvaluationNode<SrcT,TransformT>::setStatisticPlan(StatisticPlan sta_plan)
{
	m_sta_plan = sta_plan;
}

template<class SrcT,class TransformT>
Matrix<typename SrcT::MetaType> PerformanceEvaluationNode<SrcT,TransformT>::getGroundTruthImg(std::string file_path)
{
	m_image_read_node->setFilePath(file_path.c_str());
	m_image_read_node->start();

	return m_image_read_node->getImage();
}

template<class SrcT,class TransformT>
float PerformanceEvaluationNode<SrcT,TransformT>::getRecallRatio(int label_index)
{
	if (label_index >= m_labels_num)
	{
		return 0;
	}

	int p_num = 0;
	int det_p_p_num = 0;

	int rows = m_statistic_matrix.rows();
	int cols = m_statistic_matrix.cols();

	//get the number of positive samples
	for (int i = 0; i < rows; ++i)
	{
		int* sta_data = m_statistic_matrix.row(i);
		p_num += sta_data[label_index];
	}

	//get the number of positive samples detected from positive samples
	det_p_p_num = m_statistic_matrix.at(label_index,label_index);

	if (p_num == 0)
	{
		return 0;
	}
	else
	{
		return float(det_p_p_num) / float(p_num);
	}
}

template<class SrcT,class TransformT>
float PerformanceEvaluationNode<SrcT,TransformT>::getMissRatio(int label_index)
{
	if (label_index >= m_labels_num)
	{
		return 0;
	}

	//number of positive samples detected
	int det_p_num = 0;

	//number of positive samples detected from all negative samples
	int det_p_n_num = 0;

	int rows = m_statistic_matrix.rows();
	int cols = m_statistic_matrix.cols();

	//get the number of positive samples detected
	int* sta_data = m_statistic_matrix.row(label_index);
	for (int j = 0; j < cols; ++j)
	{
		det_p_num += sta_data[j];
	}

	//get the number of positive samples detected from all negative samples
	for (int j = 0; j < cols; ++j)
	{
		if (j != label_index)
		{
			det_p_n_num += sta_data[j];
		}
	}

	if (det_p_num == 0)
	{
		return 0;
	}
	else
	{
		return float(det_p_n_num) / float(det_p_num);
	}
}

template<class SrcT,class TransformT>
float PerformanceEvaluationNode<SrcT,TransformT>::getOmissionRatio(int label_index)
{
	if (label_index >= m_labels_num)
	{
		return 0.0f;
	}

	//number of positive samples
	int p_num = 0;

	//number of negative samples detected from all positive samples
	int det_n_p = 0;

	int rows = m_statistic_matrix.rows();
	int cols = m_statistic_matrix.cols();

	//get the number of positive samples
	for (int i = 0; i < rows; ++i)
	{
		int* sta_data = m_statistic_matrix.row(i);
		p_num += sta_data[label_index];
	}

	//get the number of negative samples detected from all positive samples
	for (int i = 0; i < rows; ++i)
	{
		if (i != label_index)
		{
			det_n_p += m_statistic_matrix.at(i,label_index);
		}
	}

	if (p_num == 0)
	{
		return 0;
	}
	else
	{
		return float(det_n_p) / float(p_num);
	}
}

//print to console
template<class SrcT,class TransformT>
void PerformanceEvaluationNode<SrcT,TransformT>::printUnit()
{
	Superclass::printUnit();

	EAGLEEYE_INFO("recall ratio ");
	for (int index = 0; index < m_labels_num; ++index)
	{
		EAGLEEYE_INFO("%f ",m_recall_ratio[index]);
	}
	EAGLEEYE_INFO("\n");

	EAGLEEYE_INFO("miss ratio ");
	for (int index = 0; index < m_labels_num; ++index)
	{
		EAGLEEYE_INFO("%f ",m_miss_ratio[index]);
	}
	EAGLEEYE_INFO("\n");

	EAGLEEYE_INFO("omission ratio ");
	for (int index = 0; index < m_labels_num; ++index)
	{
		EAGLEEYE_INFO("%f ",m_omission_ratio[index]);
	}
	EAGLEEYE_INFO("\n");
}

template<class SrcT,class TransformT>
bool PerformanceEvaluationNode<SrcT,TransformT>::selfcheck()
{
	Superclass::selfcheck();

	switch(m_sta_plan)
	{
	case IMAGE_COMPARE_BASED_PIXEL_STA:
		{
			if(!(TO_IMAGE_SIGNAL(MetaType,getInputPort(INPUT_PORT_CLASSIFY_DATA))))
			{
				return false;
			}

			if (m_search_folder == "")
			{
				EAGLEEYE_ERROR("sorry,search folder is empty, I couldn't find groundtruth \n");
				return false;
			}

			if (m_ext_str == "")
			{
				EAGLEEYE_ERROR("sorry, groundtruth file ext name is empty \n");
				return false;
			}

			break;
		}
	case LABEL_COMPARE_STA:
		{
			if (!(TO_IMAGE_SIGNAL(MetaType,getInputPort(INPUT_PORT_CLASSIFY_DATA))))
			{
				return false;
			}
			break;
		}
	}

	return true;
}

template<class SrcT,class TransformT>
bool PerformanceEvaluationNode<SrcT,TransformT>::imageCompareBasedPixelStatis()
{
	//input port is one predicted image
	SrcT* img_signal = dynamic_cast<SrcT*>(getInputPort(INPUT_PORT_CLASSIFY_DATA));
	Matrix<MetaType> predict_img = img_signal->img;

	//read ground truth image
	std::string ground_truth_file = m_search_folder + std::string(img_signal->ext_info) + m_ext_str;
	Matrix<MetaType> groundtruth_img = getGroundTruthImg(ground_truth_file);
	
	//transform operator
	TransformT transform_op;

	if (!(groundtruth_img.isempty()))
	{
		int rows = predict_img.rows();
		int cols = predict_img.cols();

		for (int i = 0; i < rows; ++i)
		{
			MetaType* predict_img_data = predict_img.row(i);
			MetaType* groundtruth_img_data = groundtruth_img.row(i);
			for (int j = 0; j < cols; ++j)
			{
				int groundtruth_index = int(groundtruth_img_data[j]);
				int predict_index = transform_op(predict_img_data[j]);
				if (groundtruth_index >= 0 && predict_index >= 0)
				{
					m_statistic_matrix(predict_index,groundtruth_index) = 
						m_statistic_matrix(predict_index,groundtruth_index) + 1;
				}
			}
		}

		m_cumulative_count++;
		return true;
	}

	return false;
}

template<class SrcT,class TransformT>
bool PerformanceEvaluationNode<SrcT,TransformT>::labelCompareStatis()
{
	//input port is one matrix with two columns
	Matrix<MetaType> label_mat = TO_IMAGE(MetaType,getInputPort(INPUT_PORT_CLASSIFY_DATA));

	//transform operator
	TransformT transform_op;

	if (!(label_mat.isempty()))
	{
		int rows = label_mat.rows();
		int cols = label_mat.cols();

		Matrix<MetaType> predicted_label_mat = label_mat(Range(0,rows),Range(0,1));
		Matrix<MetaType> ground_truth_label_mat = label_mat(Range(0,rows),Range(1,2));

		for (int i = 0; i < rows; ++i)
		{
			int groundtruth_index = int(ground_truth_label_mat.at(i));
			int predict_index = transform_op(predicted_label_mat.at(i));
			if (groundtruth_index >= 0 && predict_index >= 0)
			{
				m_statistic_matrix(predict_index,groundtruth_index) = 
					m_statistic_matrix(predict_index,groundtruth_index) + 1;
			}
		}

		//update statistic info
		updateMissRecallOmissStatis();

		return true;
	}

	return false;
}

template<class SrcT,class TransformT>
void PerformanceEvaluationNode<SrcT,TransformT>::updateMissRecallOmissStatis()
{
	//update cumulative statistical quantity
	for (int index = 0; index < m_labels_num; ++index)
	{
		float miss_ratio = getMissRatio(index);
		m_miss_ratio[index] = (float(m_cumulative_count) * m_miss_ratio[index] + miss_ratio) / float(m_cumulative_count + 1);

		float recall_ratio = getRecallRatio(index);
		m_recall_ratio[index] = (float(m_cumulative_count) * m_recall_ratio[index] + recall_ratio) / float(m_cumulative_count + 1);

		float omission_ratio=getOmissionRatio(index);
		m_omission_ratio[index] = (float(m_cumulative_count) * m_omission_ratio[index] + omission_ratio) / float(m_cumulative_count + 1);
	}

	m_cumulative_count++;
}

template<class SrcT,class TransformT>
void PerformanceEvaluationNode<SrcT,TransformT>::setLabelsNum(int num)
{
	m_labels_num = num;

	//build statistic matrix
	m_statistic_matrix = Matrix<int>(num,num,0);

	//assign default value
	m_miss_ratio = DynamicArray<float>(num,0.0f);
	m_recall_ratio = DynamicArray<float>(num,0.0f);
	m_omission_ratio = DynamicArray<float>(num,0.0f);
	
	m_cumulative_count = 0;
}

template<class SrcT,class TransformT>
void PerformanceEvaluationNode<SrcT,TransformT>::setHowToSearchGroundTruthImg(char* folder,char* ext_str)
{
	m_search_folder = folder;
	m_ext_str = ext_str;
}

template<class SrcT,class TransformT>
bool PerformanceEvaluationNode<SrcT,TransformT>::labelTopHitStatis(int k_top_hit /* = 3 */)
{
	Matrix<MetaType> classify_data = TO_IMAGE(MetaType,getInputPort(INPUT_PORT_CLASSIFY_DATA));

	//samples number
	int samples_num = classify_data.rows();
	
	Matrix<MetaType> predict_probability = classify_data(Range(0,samples_num),Range(0,classify_data.cols() - 1));
	Matrix<MetaType> ground_truth_label = classify_data(Range(0,samples_num),Range(classify_data.cols() - 1,classify_data.cols()));
	
	//transform operator
	TransformT transform_op;

	int error_num = 0;
	for (int i = 0; i < samples_num; ++i)
	{
		Matrix<MetaType> p = predict_probability(Range(i,i + 1),Range(0,m_labels_num));
		std::vector<unsigned int> order_labels = sort<DescendingSortPredict<MetaType>>(p);

		bool flag = false;
		for (int m = 0; m < k_top_hit; ++m)
		{
			if (transform_op(MetaType(order_labels[m])) == int(ground_truth_label(i)))
			{
				flag = true;
				break;
			}
		}

		if (flag == false)
			error_num++;
	}

	m_error_ratio = float(error_num) / float(samples_num);
	return true;
}

}