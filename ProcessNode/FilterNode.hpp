namespace eagleeye
{
template<class ImageSigT>
FilterNode<ImageSigT>::FilterNode()
{
	m_color_dev = 20;
	m_dis_dev = 0.5;
	m_gradient_dev = 20;

	m_filter_type=BILATERAL_FILTER;

	//set input port number
	setNumberOfInputSignals(1);
	
	//set output port property
	setNumberOfOutputSignals(1);
	setOutputPort(makeOutputSignal(),OUTPUT_PORT_FILTER_RESULT_DATA);

	//////////////////////////////////////////////////////////////////////////
	//build monitor var
	EAGLEEYE_MONITOR_VAR(int,setFilterType,getFilterType,"filter_type");
	EAGLEEYE_MONITOR_VAR(float,setFilterColorDev,getFilterColorDev,"color_dev");
	EAGLEEYE_MONITOR_VAR(float, setFilterDisDev,getFilterDisDev,"dis_dev");
	EAGLEEYE_MONITOR_VAR(float,setFilterGradientDev,getFilterGradientDev,"gradient_dev");
	//////////////////////////////////////////////////////////////////////////
}

template<class ImageSigT>
FilterNode<ImageSigT>::~FilterNode()
{

}


template<class ImageSigT>
void FilterNode<ImageSigT>::passonNodeInfo()
{

}

template<class ImageSigT>
void FilterNode<ImageSigT>::executeNodeInfo()
{
	switch(m_filter_type)
	{
	case BILATERAL_FILTER:
		{
			bilateralFilter();
			EAGLEEYE_INFO("finish bilateral filter\n");
			break;
		}
	case NONLOCALMEAN_FILTER:
		{
			nonLocalMeanFilter();
			EAGLEEYE_INFO("finish nonlocal mean filter\n");
			break;
		}
	case TRILATERAL_FILTER:
		{
			trilateralFilter();
			EAGLEEYE_INFO("finish gradient filter\n");
			break;
		}
	case CURVATURE_FILTER:
		{
			curvatureFilter();
			EAGLEEYE_INFO("finish curvature filter\n");
			break;
		}
	case GAUSSIAN_FILTER:
		{
			gaussianFilter();
			EAGLEEYE_INFO("finish gaussian filter\n");
			break;
		}
	default:
		{
			EAGLEEYE_ERROR("sorry, don't support your defined filter type\n");
			return;
		}
	}
}

template<class ImageSigT>
bool FilterNode<ImageSigT>::selfcheck()
{
	Superclass::selfcheck();

	//only process image with channel =1 or 3
	if (!(AtomicTypeTrait<PixelType>::size == 1 || AtomicTypeTrait<PixelType>::size == 3))
	{
		EAGLEEYE_ERROR("sorry, image channel is not supported\n");
		EAGLEEYE_ERROR("image channel must be 1 or 3\n");
		return false;
	}

	if (!getInputImageSignal())
	{
		EAGLEEYE_ERROR("sorry, image pixel isn't consistent...\n please be careful...\n");
		return false;
	}

	return true;
}

template<class ImageSigT>
void FilterNode<ImageSigT>::setFilterType(FilterType filter_type)
{
	m_filter_type = filter_type;

	//force time update
	modified();
}

template<class ImageSigT>
void FilterNode<ImageSigT>::setFilterType(const int index)
{
	m_filter_type = (FilterType)index;

	//force time update
	modified();
}

template<class ImageSigT>
void FilterNode<ImageSigT>::getFilterType(int& index)
{
	index = (int)m_filter_type;
}

template<class ImageSigT>
void FilterNode<ImageSigT>::bilateralFilter()
{
	ImageSigT* input_img_signal = getInputImageSignal(INPUT_PORT_IMAGE_DATA);
	Matrix<PixelType> input_img = input_img_signal->img;

	int rows = input_img.rows();
	int cols = input_img.cols();

	//bilateral filter
	Matrix<Array<float,2+AtomicTypeTrait<PixelType>::size>> pos_mat(rows,cols);
	Matrix<Array<float,AtomicTypeTrait<PixelType>::size>> img_mat(rows,cols);

	for (int i = 0; i < rows; ++i)
	{
		Array<float,2+AtomicTypeTrait<PixelType>::size>* pos_mat_data = pos_mat.row(i);
		Array<float,AtomicTypeTrait<PixelType>::size>* img_mat_data = img_mat.row(i);

		PixelType* input_img_data = input_img.row(i);
		for (int j = 0; j < cols; ++j)
		{
			pos_mat_data[j][0] = float(j) / m_dis_dev;
			pos_mat_data[j][1] = float(i) / m_dis_dev;

			for (int p_d = 0; p_d < AtomicTypeTrait<PixelType>::size; ++p_d)
			{
				pos_mat_data[j][2 + p_d] = float(OperateTrait<PixelType>::unit(input_img_data[j],p_d)) / m_color_dev;
				img_mat_data[j][p_d] = float(OperateTrait<PixelType>::unit(input_img_data[j],p_d));
			}
		}
	}

	//filter_result
	Matrix<Array<float,AtomicTypeTrait<PixelType>::size>> filter_result = gKernelFilter(pos_mat,img_mat);

	//get output image signal
	ImageSigT* output_img_signal = getOutputImageSignal(OUTPUT_PORT_FILTER_RESULT_DATA);
	Matrix<PixelType> result_img(rows,cols);

	for (int i = 0; i < rows; ++i)
	{
		Array<float,AtomicTypeTrait<PixelType>::size>* filter_result_data = filter_result.row(i);
		PixelType* result_img_data = result_img.row(i);

		for (int j = 0; j < cols; ++j)
		{
			for (int p_d = 0; p_d < AtomicTypeTrait<PixelType>::size; ++p_d)
			{
				OperateTrait<PixelType>::unit(result_img_data[j],p_d) = (AtomicTypeTrait<PixelType>::AtomicType)filter_result_data[j][p_d];
			}
		}
	}

	output_img_signal->img = result_img;
}

template<class ImageSigT>
void FilterNode<ImageSigT>::nonLocalMeanFilter()
{
	ImageSigT* input_img_signal = getInputImageSignal(INPUT_PORT_IMAGE_DATA);
	Matrix<PixelType> input_img = input_img_signal->img;

	int rows = input_img.rows();
	int cols = input_img.cols();

	//bilateral filter
	Matrix<Array<float,2 + AtomicTypeTrait<PixelType>::size * 9>> pos_mat(rows,cols);
	Matrix<Array<float,AtomicTypeTrait<PixelType>::size>> img_mat(rows,cols);

	for (int i = 0; i < rows; ++i)
	{
		Array<float,2 + AtomicTypeTrait<PixelType>::size * 9>* pos_mat_data = pos_mat.row(i);
		Array<float,AtomicTypeTrait<PixelType>::size>* img_mat_data = img_mat.row(i);

		for (int j = 0; j < cols; ++j)
		{
			//fill position data
			pos_mat_data[j][0] = float(j) / m_dis_dev;
			pos_mat_data[j][1] = float(i) / m_dis_dev;

			for (int r_i = 0, k = 0; r_i < 3; ++r_i)
			{
				int index_r = EAGLEEYE_MIN((i + r_i),(rows - 1));
				for (int c_j = 0; c_j < 3; ++c_j, ++k)
				{
					int index_c = EAGLEEYE_MIN((j + c_j),(cols - 1));

					for (int p_d = 0; p_d < AtomicTypeTrait<PixelType>::size; ++p_d)
					{
						pos_mat_data[j][2 + k * AtomicTypeTrait<PixelType>::size + p_d] = float(OperateTrait<PixelType>::unit(input_img.at(index_r,index_c),p_d)) / m_color_dev;
					}
				}
			}

			//fill image data
			PixelType* input_img_data = input_img.row(i);
			for (int p_d = 0; p_d < AtomicTypeTrait<PixelType>::size; ++p_d)
			{
				img_mat_data[j][p_d] = float(OperateTrait<PixelType>::unit(input_img_data[j],p_d));
			}
		}
	}


	//filter_result
	Matrix<Array<float,AtomicTypeTrait<PixelType>::size>> filter_result = gKernelFilter(pos_mat,img_mat);

	//get output image signal
	ImageSigT* output_img_signal = getOutputImageSignal(OUTPUT_PORT_FILTER_RESULT_DATA);
	Matrix<PixelType> result_img(rows,cols);

	for (int i = 0; i < rows; ++i)
	{
		Array<float,AtomicTypeTrait<PixelType>::size>* filter_result_data = filter_result.row(i);
		PixelType* result_img_data = result_img.row(i);

		for (int j = 0; j < cols; ++j)
		{
			for (int p_d = 0; p_d < AtomicTypeTrait<PixelType>::size; ++p_d)
			{
				OperateTrait<PixelType>::unit(result_img_data[j],p_d) = (AtomicTypeTrait<PixelType>::AtomicType)filter_result_data[j][p_d];
			}
		}
	}

	output_img_signal->img = result_img;
}

template<class ImageSigT>
void FilterNode<ImageSigT>::gaussianFilter()
{
	ImageSigT* input_img_signal = getInputImageSignal();
	Matrix<PixelType> input_img = input_img_signal->img;

	int rows = input_img.rows();
	int cols = input_img.cols();

	//gaussian filter
	Matrix<Array<float,2>> pos_mat(rows,cols);
	Matrix<Array<float,AtomicTypeTrait<PixelType>::size>> img_mat(rows,cols);

	for (int i = 0; i < rows; ++i)
	{
		Array<float,2>* pos_mat_data = pos_mat.row(i);
		Array<float,AtomicTypeTrait<PixelType>::size>* img_mat_data = img_mat.row(i);

		PixelType* input_img_data = input_img.row(i);
		for (int j = 0; j < cols; ++j)
		{
			pos_mat_data[j][0] = float(j) / m_dis_dev;
			pos_mat_data[j][1] = float(i) / m_dis_dev;
	
			for (int p_d = 0; p_d < AtomicTypeTrait<PixelType>::size; ++p_d)
			{
				img_mat_data[j][p_d] = float(OperateTrait<PixelType>::unit(input_img_data[j],p_d));
			}
		}
	}



	//filter_result
	Matrix<Array<float,AtomicTypeTrait<PixelType>::size>> filter_result = gKernelFilter(pos_mat,img_mat);

	//get output image signal
	ImageSigT* output_img_signal = getOutputImageSignal();
	Matrix<PixelType> result_img(rows,cols);

	for (int i = 0; i < rows; ++i)
	{
		Array<float,AtomicTypeTrait<PixelType>::size>* filter_result_data = filter_result.row(i);
		PixelType* result_img_data = result_img.row(i);

		for (int j = 0; j < cols; ++j)
		{
			for (int p_d = 0; p_d < AtomicTypeTrait<PixelType>::size; ++p_d)
			{
				OperateTrait<PixelType>::unit(result_img_data[j],p_d) = (AtomicTypeTrait<PixelType>::AtomicType)filter_result_data[j][p_d];
			}
		}
	}

	output_img_signal->img = result_img;
}

template<class ImageSigT>
void FilterNode<ImageSigT>::trilateralFilter()
{
	ImageSigT* input_img_signal = getInputImageSignal();
	Matrix<PixelType> input_img = input_img_signal->img;

	Matrix<Gradient> gradient_img;
	gradient_img = computeGradient(input_img);

	int rows = input_img.rows();
	int cols = input_img.cols();

	//bilateral filter
	Matrix<Array<float,2 + 1 + AtomicTypeTrait<PixelType>::size>> pos_mat(rows,cols);
	Matrix<Array<float,AtomicTypeTrait<PixelType>::size>> img_mat(rows,cols);

	for (int i = 0; i < rows; ++i)
	{
		Array<float,2 + 1 + AtomicTypeTrait<PixelType>::size>* pos_mat_data = pos_mat.row(i);
		Array<float,AtomicTypeTrait<PixelType>::size>* img_mat_data = img_mat.row(i);
		Gradient* gradient_img_data = gradient_img.row(i);

		PixelType* input_img_data = input_img.row(i);
		for (int j = 0; j < cols; ++j)
		{
			pos_mat_data[j][0] = float(j) / m_dis_dev;
			pos_mat_data[j][1] = float(i) / m_dis_dev;
			if (gradient_img_data[j].x != 0)
			{
				pos_mat_data[j][2] = abs(gradient_img_data[j].y / gradient_img_data[j].x) / m_gradient_dev;
			}
			else
			{
				pos_mat_data[j][2] = 10000;
			}
			
			for (int p_d = 0; p_d < AtomicTypeTrait<PixelType>::size; ++p_d)
			{
				pos_mat_data[j][3 + p_d] = float(OperateTrait<PixelType>::unit(input_img_data[j],p_d))/m_color_dev;
				img_mat_data[j][p_d] = float(OperateTrait<PixelType>::unit(input_img_data[j],p_d));
			}
		}
	}

	//filter_result
	Matrix<Array<float,AtomicTypeTrait<PixelType>::size>> filter_result = gKernelFilter(pos_mat,img_mat);

	//get output image signal
	ImageSigT* output_img_signal = getOutputImageSignal();
	Matrix<PixelType> result_img(rows,cols);

	for (int i = 0; i < rows; ++i)
	{
		Array<float,AtomicTypeTrait<PixelType>::size>* filter_result_data = filter_result.row(i);
		PixelType* result_img_data = result_img.row(i);

		for (int j = 0; j < cols; ++j)
		{
			for (int p_d = 0; p_d < AtomicTypeTrait<PixelType>::size; ++p_d)
			{
				OperateTrait<PixelType>::unit(result_img_data[j],p_d) = (AtomicTypeTrait<PixelType>::AtomicType)filter_result_data[j][p_d];
			}
		}
	}

	output_img_signal->img = result_img;
}

template<class ImageSigT>
void FilterNode<ImageSigT>::curvatureFilter()
{

}

template<class ImageSigT>
void FilterNode<ImageSigT>::setFilterColorDev(const float color_dev)
{
	m_color_dev = color_dev;

	//force time update
	modified();
}

template<class ImageSigT>
void FilterNode<ImageSigT>::getFilterColorDev(float& color_dev)
{
	color_dev = m_color_dev;
}

template<class ImageSigT>
void FilterNode<ImageSigT>::setFilterDisDev(const float dis_dev)
{
	m_dis_dev = dis_dev;

	//force time update
	modified();
}

template<class ImageSigT>
void FilterNode<ImageSigT>::getFilterDisDev(float& dis_dev)
{
	dis_dev = m_dis_dev;
}

template<class ImageSigT>
void FilterNode<ImageSigT>::setFilterGradientDev(const float gradient_dev)
{
	m_gradient_dev = gradient_dev;

	//force time update
	modified();
}

template<class ImageSigT>
void FilterNode<ImageSigT>::getFilterGradientDev(float& gradient_dev)
{
	gradient_dev = m_gradient_dev;
}

}