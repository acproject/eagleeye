namespace eagleeye
{
template<class ImageSigT>
VerticalHorizontalSplitNode<ImageSigT>::VerticalHorizontalSplitNode()
{
	m_split_dir = VERTICAL_SPLIT;
	m_predefined_split_num = 0;
	m_optimum_split_threshold = 0.2f;

	m_small_region_limit = 5;
	m_median_filter_size = 21;
	m_neighbor_size = 100;

	//set input port number
	setNumberOfInputSignals(1);

	//set output port property
	setNumberOfOutputSignals(1);
	setOutputPort(new ImageSignal<ERGB>,0);

/*	disableOutputPort(0);*/

	//////////////////////////////////////////////////////////////////////////
	//monitor
	EAGLEEYE_MONITOR_VAR_EXT(int,setSplitDir,getSplitDir,"split_dir","min=0 max=1");
	EAGLEEYE_MONITOR_VAR_EXT(int,setSplitNumber,getSplitNumber,"split_num","min=0");
	EAGLEEYE_MONITOR_VAR(int,setSmallRegionLimit,getSmallRegionLimit,"small_region_limit");
	EAGLEEYE_MONITOR_VAR_EXT(int,setMedianFilterSize,getMedianFilterSize,"median_filter_size","min=3");
}

template<class ImageSigT>
VerticalHorizontalSplitNode<ImageSigT>::~VerticalHorizontalSplitNode()
{

}

template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::executeNodeInfo()
{
	Matrix<PixelType> input_img = TO_IMAGE(PixelType,getInputPort(0));
	m_img = input_img;

	switch(m_split_dir)
	{
	case VERTICAL_SPLIT:
		{
			autoVerticalSplit(input_img);
			break;
		}
	case HORIZONTAL_SPLIT:
		{
			autoHorizontalSplit(input_img);
			break;
		}
	}

	if (m_output_port_state[OUTPUT_PORT_DISPLAY_SPLIT_IMAGE])
	{
		int width = input_img.cols();
		int height = input_img.rows();

		ImageSignal<ERGB>* output_img_sig = TO_IMAGE_SIGNAL(ERGB,getOutputPort(0));
		Matrix<ERGB> output_img = input_img.transform<ERGB>();
		output_img.clone();
		int separate_region_num = m_separate_region.size();
		ERGB red_color;
		red_color[0] = 255; red_color[1] = 0; red_color[2] = 0;
		for (int i = 0; i < separate_region_num; ++i)
		{
			if (m_separate_region[i].second != width)
			{
				drawLine(output_img,m_separate_region[i].second,0,m_separate_region[i].second,height - 1,red_color);
			}
		}

		output_img_sig->img = output_img;
	}
}

template<class ImageSigT>
bool VerticalHorizontalSplitNode<ImageSigT>::selfcheck()
{
	if (!TO_IMAGE_SIGNAL(PixelType,getInputPort(0)))
	{
		EAGLEEYE_ERROR("input port is not correct...\n");
		return false;
	}

	return true;
}

template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::printUnit()
{
	//do nothing
}

template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::autoVerticalSplit(const Matrix<PixelType>& split_img)
{
	typedef AverageOperations<PixelType,float> AverageOpT;
	Matrix<float> img = split_img.transform(AverageOpT());
/*	putToMatlab(img,"img");*/

	int rows = img.rows();
	int cols = img.cols();

	m_neighbor_size = cols / 10;

	float foreground_mean,background_mean;
	float bw_threshold = 0.0f;
	
	//we don't want to using foreground_mean and background_mean directly
	autoBWSplit(img,bw_threshold,foreground_mean,background_mean,OSTU);

	//gray of foreground region is less than average
	//bw process
	Matrix<unsigned char> bw_img(rows,cols);
	for (int i = 0;i < rows; ++i)
	{
		float* img_data = img.row(i);
		unsigned char* bw_data = bw_img.row(i);
		for (int j = 0; j < cols; ++j)
		{
			if (img_data[j] < bw_threshold)
				bw_data[j] = 1;
			else
				bw_data[j] = 0;
		}
	}
	
/*	putToMatlab(bw_img,"bw_img");*/

	//project to bottom line
	std::vector<float> project_his_vec(cols,float(0));
	for (int i = 0; i < rows; ++i)
	{
		unsigned char* bw_data = bw_img.row(i);
		for (int j = 0; j < cols; ++j)
		{
			project_his_vec[j] += float(bw_data[j]) / float(rows);
		}
	}
	
	MedianFilter1D<float> median_filter(m_median_filter_size);
	std::vector<float> smooth_project_his_vec = median_filter.execute(project_his_vec);
	
	putToMatlab(Matrix<float>(1,smooth_project_his_vec.size(),&smooth_project_his_vec[0],false),"project_his");
	
	//finding optimum split threshold
	findingOptimumThrehold(smooth_project_his_vec);

	//split to small pieces
	//finding optimum split position
	//first, finding split position coarsely
	std::vector<int> raw_end_split_pos;
	std::vector<int> raw_start_split_pos;
	bool flag = false;

// 	for (int i = 3; i < cols - 3; ++i)
// 	{
// 		if (((smooth_project_his_vec[i - 3] >= m_optimum_split_threshold) &&
// 			(smooth_project_his_vec[i-2] >= m_optimum_split_threshold) &&
// 			(smooth_project_his_vec[i-1] >= m_optimum_split_threshold) &&
// 			(smooth_project_his_vec[i + 1] <= m_optimum_split_threshold)) || 
// 			((smooth_project_his_vec[i - 1] >= m_optimum_split_threshold) &&
// 			(smooth_project_his_vec[i + 1] <= m_optimum_split_threshold) &&
// 			(smooth_project_his_vec[i + 2] <= m_optimum_split_threshold) &&
// 			(smooth_project_his_vec[i + 3] <= m_optimum_split_threshold)))
// 		{
// 			//record split position coarsely
// 			if (flag == true)
// 			{
// 				flag = false;
// 				raw_start_split_pos.push_back(i);
// 				continue;
// 			}
// 		}
// 
// 		if (((smooth_project_his_vec[i - 3] <= m_optimum_split_threshold) &&
// 			(smooth_project_his_vec[i - 2] <= m_optimum_split_threshold) &&
// 			(smooth_project_his_vec[i - 1] <= m_optimum_split_threshold) &&
// 			(smooth_project_his_vec[i + 1] >= m_optimum_split_threshold)) || 
// 			((smooth_project_his_vec[i - 1] <= m_optimum_split_threshold) &&
// 			(smooth_project_his_vec[i + 1] >= m_optimum_split_threshold) &&
// 			(smooth_project_his_vec[i + 2] >= m_optimum_split_threshold) &&
// 			(smooth_project_his_vec[i + 3] >= m_optimum_split_threshold)))
// 		{
// 			//record split position coarsely
// 			if (flag == false)
// 			{
// 				flag = true;
// 				raw_end_split_pos.push_back(i);
// 
// 				continue;
// 			}
// 		}
// 	}
	
	//finding split pair
	std::vector<std::pair<int,int>> raw_split_pair_set;
	for (int i = 0; i < int(raw_start_split_pos.size()); ++i)
	{
		for (int j = 0; j < raw_end_split_pos.size(); ++j)
		{
			if (raw_start_split_pos[i] < raw_end_split_pos[j])
			{
				raw_split_pair_set.push_back(std::make_pair<int,int>(raw_start_split_pos[i],raw_end_split_pos[j]));
				break;
			}
		}
	}

	//second, split to some separate region block
	int left_start_col = 0;
	int right_end_col = 0;
	int raw_split_pair_num = raw_split_pair_set.size();
	for (int i = 0; i < raw_split_pair_set.size(); ++i)
	{
		std::pair<int,int> split_pair = raw_split_pair_set[i];
		if (i == 0)
		{
			//first
			left_start_col = 0;
			right_end_col = (split_pair.first + split_pair.second) / 2;
		}
		else
		{
			left_start_col = right_end_col;
			right_end_col = (split_pair.first + split_pair.second) / 2;
		}

		m_separate_region.push_back(std::make_pair<int,int>(left_start_col,right_end_col));
	}

	//third, merge some small regions
	while(verticalSmallRegionMerge(m_separate_region));

	if (m_predefined_split_num != 0)
	{
		//fourth, merge or split to the predefined split number
		int predicted_split_num = m_separate_region.size();
		if (predicted_split_num < m_predefined_split_num)
		{
			//split further
			while(int(m_separate_region.size()) < m_predefined_split_num)
			{
				verticalSplitForcely(m_separate_region,m_predefined_split_num);
			}
		}
		else if(predicted_split_num > m_predefined_split_num)
		{
			//merge further
			while(int(m_separate_region.size()) > m_predefined_split_num)
			{
				verticalMergeForcely(m_separate_region,m_predefined_split_num);
			}
		}
	}
}

template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::autoHorizontalSplit(const Matrix<PixelType>& img)
{
	
}

template<class ImageSigT>
Matrix<typename ImageSigT::MetaType> VerticalHorizontalSplitNode<ImageSigT>::getImage(int index)
{
	if (index < m_separate_region.size())
	{
		return m_img(Range(0,m_img.rows()),Range(m_separate_region[index].first,m_separate_region[index].second));
	}
	else
		return Matrix<PixelType>();
}

template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::setSplitNumber(const int split_num)
{
	m_predefined_split_num = split_num;
}
template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::getSplitNumber(int& split_num)
{
	split_num = m_predefined_split_num;
}

template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::verticalSplitForcely(std::vector<std::pair<int,int>>& small_pieces,int target_pieces_num)
{
	int max_width, max_index;
	max_width = 0; max_index = 0;
	int current_pieces_num = small_pieces.size();

	if (current_pieces_num < target_pieces_num)
	{
		//finding region block with max width
		for (int i = 0; i < current_pieces_num; ++i)
		{
			if (int(small_pieces[i].second - small_pieces[i].first) > max_width)
			{
				max_width = small_pieces[i].second - small_pieces[i].first;
				max_index = i;
			}
		}

		std::pair<int,int> selected_piece = small_pieces[max_index];

		std::vector<std::pair<int,int>> temp_small_pieces = small_pieces;
		small_pieces.clear();
		small_pieces.resize(current_pieces_num + 1);

		for (int i = 0; i < max_index; ++i)
		{
			small_pieces[i] = temp_small_pieces[i];
		}
		small_pieces[max_index] = std::make_pair<int,int>(selected_piece.first , (selected_piece.first + selected_piece.second) / 2);
		small_pieces[max_index + 1] = std::make_pair<int,int>((selected_piece.first + selected_piece.second) / 2,selected_piece.second);
		for (int i = max_index + 2; i < current_pieces_num + 1; ++i)
		{
			small_pieces[i] = temp_small_pieces[i-1];
		}
	}
}

template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::verticalMergeForcely(std::vector<std::pair<int,int>>& small_pieces,int target_pieces_num)
{
	int small_pieces_num = small_pieces.size();
	if (small_pieces_num > target_pieces_num)
	{
		//finding one pair regions with small width
		int min_width,min_index;
		min_width = AtomicTypeTrait<int>::maxval(); min_index = 0;
		for (int i = 0; i < small_pieces_num - 1; ++i)
		{
			if (int((small_pieces[i].second - small_pieces[i].first) + (small_pieces[i+1].second - small_pieces[i+1].first)) < min_width)
			{
				min_width = (small_pieces[i].second - small_pieces[i].first) + (small_pieces[i+1].second - small_pieces[i+1].first);
				min_index = i;
			}
		}

		std::vector<std::pair<int,int>> temp_small_pieces = small_pieces;
		
		small_pieces.clear();
		small_pieces.resize(small_pieces_num - 1);
		for (int i = 0; i < min_index; ++i)
		{
			small_pieces[i] = temp_small_pieces[i];
		}

		small_pieces[min_index] = std::pair<int,int>(temp_small_pieces[min_index].first,temp_small_pieces[min_index+1].second);

		for (int i = min_index + 1; i < small_pieces_num - 1; ++i)
		{
			small_pieces[i] = temp_small_pieces[i + 1];
		}
	}
}

template<class ImageSigT>
bool VerticalHorizontalSplitNode<ImageSigT>::verticalSmallRegionMerge(std::vector<std::pair<int,int>>& split_pieces)
{
	int split_pieces_num = split_pieces.size();
	if (split_pieces_num > 1)
	{
		bool flag = false;

		std::vector<std::pair<int,int>> new_split_pieces;
		int i = 0;
		while(i < split_pieces_num)
		{
			if (i == split_pieces_num - 1)
			{
				//check the last region
				if (int(split_pieces[i].second - split_pieces[i].first) < m_small_region_limit)
				{
					new_split_pieces[new_split_pieces.size() - 1].first = new_split_pieces[new_split_pieces.size() - 1].first;
					new_split_pieces[new_split_pieces.size() - 1].second = split_pieces[i].second;

					flag = true;
				}
				else
				{
					new_split_pieces.push_back(split_pieces[i]);
				}	

				i = i + 1;
			}
			else
			{
				if (int(split_pieces[i].second - split_pieces[i].first) < m_small_region_limit)
				{
					new_split_pieces.push_back(std::make_pair<int,int>(split_pieces[i].first,split_pieces[i+1].second));
					i = i + 2;

					flag = true;
				}
				else
				{
					new_split_pieces.push_back(split_pieces[i]);
					i = i + 1;
				}
			}
		}		

		split_pieces = new_split_pieces;

		return flag;
	}
	else
		return false;
}

template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::setSplitDir(const SplitDir split_dir)
{
	m_split_dir = split_dir;
}
template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::getSplitDir(SplitDir& split_dir)
{
	split_dir = m_split_dir;
}
template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::setSplitDir(const int split_dir)
{
	if (split_dir >= 0 && split_dir <= 1)
	{
		m_split_dir = SplitDir(split_dir);
	}
}
template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::getSplitDir(int& split_dir)
{
	split_dir = int(m_split_dir);
}

template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::setSmallRegionLimit(const int small_region_limit)
{
	m_small_region_limit = small_region_limit;
}
template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::getSmallRegionLimit(int& small_region_limit)
{
	small_region_limit = m_small_region_limit;
}
template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::setMedianFilterSize(const int window_size)
{
	if (window_size % 2 == 0)
		m_median_filter_size = window_size + 1;
	else
		m_median_filter_size = window_size;
}
template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::getMedianFilterSize(int& window_size)
{
	window_size = m_median_filter_size;
}

template<class ImageSigT>
void VerticalHorizontalSplitNode<ImageSigT>::findingOptimumThrehold(std::vector<float> project_his)
{
	//finding local minimum value in neighbor
	int max_num = project_his.size() / m_neighbor_size - 1;
	std::vector<float> local_min_set;
	std::vector<float> local_max_set;
	for (int i = 0; i < max_num; ++i)
	{
		float local_min_val = 100000000.0f;
		float local_max_val = 0.0f;
		for (int j = i * m_neighbor_size; j < (i + 1) * m_neighbor_size; ++j)
		{
			if (local_min_val > project_his[j])
			{
				local_min_val = project_his[j];
			}
			if (local_max_val < project_his[j])
			{
				local_max_val = project_his[j];
			}
		}
		local_min_set.push_back(local_min_val);
		local_max_set.push_back(local_max_val);
	}

	//finding max val in all local min value
	float max_in_local_min_set = 0;
	for (int i = 0; i < local_min_set.size(); ++i)
	{
		if (max_in_local_min_set < local_min_set[i])
		{
			max_in_local_min_set = local_min_set[i];
		}
	}
	
	//finding median in all local max value
	std::sort(local_max_set.begin(),local_max_set.end());
	float median_in_local_max_set = local_max_set[int(local_max_set.size() / 2)];
	
	m_optimum_split_threshold = max_in_local_min_set + (EAGLEEYE_MAX(((median_in_local_max_set - max_in_local_min_set) * 0.3f),0.0f));
}
}