namespace eagleeye
{
template<class ImageSigT>
GridSuperPixelNode<ImageSigT>::GridSuperPixelNode()
{
	m_superpixel_size = 0;
	m_superpixel_num = 100;

	//set input signal number
	setNumberOfInputSignals(1);

	//set output port signal
	setNumberOfOutputSignals(2);
	setOutputPort(new ImageSignal<int>,0);
	setOutputPort(new ImageSignal<PixelType>,1);

	OUTPUT_PORT_LABEL_DATA_STATE = 1;
	OUTPUT_PORT_SUPERPIXEL_IMAGE_DATA_STATE = 0;

	//////////////////////////////////////////////////////////////////////////
	//add monitor variable
	EAGLEEYE_MONITOR_VAR(int,setSuperPixelsNum,getSuperPixelsNum,"superpixel_number");
	EAGLEEYE_MONITOR_VAR(int,setSuperPixelSize,getSuperPixelSize,"superpixel_size"); 
	//////////////////////////////////////////////////////////////////////////
}

template<class ImageSigT>
GridSuperPixelNode<ImageSigT>::~GridSuperPixelNode()
{

}

template<class ImageSigT>
Matrix<typename ImageSigT::MetaType> GridSuperPixelNode<ImageSigT>::getSuperPixelImage()
{
	return m_superpixel_img;
}

template<class ImageSigT>
Matrix<int> GridSuperPixelNode<ImageSigT>::getLabelImage()
{
	return m_superpixel_label;
}

template<class ImageSigT>
Matrix<float> GridSuperPixelNode<ImageSigT>::getSuperpixelCenter()
{
	return m_superpixel_center;
}

template<class ImageSigT>
void GridSuperPixelNode<ImageSigT>::setSuperPixelsNum(const int num)
{
	m_superpixel_num = num;
}

template<class ImageSigT>
void GridSuperPixelNode<ImageSigT>::getSuperPixelsNum(int& num)
{
	num = m_superpixel_num;
}

template<class ImageSigT>
void GridSuperPixelNode<ImageSigT>::setSuperPixelSize(const int size)
{
	m_superpixel_size = size;
}

template<class ImageSigT>
void GridSuperPixelNode<ImageSigT>::getSuperPixelSize(int& size)
{
	size = m_superpixel_size;
}

template<class ImageSigT>
void GridSuperPixelNode<ImageSigT>::executeNodeInfo()
{
	//get input image
	Matrix<PixelType> input_img = TO_IMAGE(PixelType,getInputPort(INPUT_PORT_IMAGE_DATA));

	int rows = input_img.rows();
	int cols = input_img.cols();
	
	if (m_superpixel_size <= 1)
	{
		m_superpixel_size = EAGLEEYE_MAX(int(rows * cols / m_superpixel_num),1);
	}

	int block_edge = int(sqrt(double(m_superpixel_size)));
	m_superpixel_num = int(ceil((float(rows) / float(block_edge)))) * int(ceil((float(cols) / float(block_edge))));

	m_superpixel_label = Matrix<int>(rows,cols);
	m_superpixel_center = Matrix<float>(m_superpixel_num,2);

	int label_count = 0;
	for (int i = 0; i < rows; i = i + block_edge)
	{
		for (int j = 0; j < cols; j = j + block_edge)
		{
			int row_end = EAGLEEYE_MIN(i + block_edge,rows);
			int col_end = EAGLEEYE_MIN(j + block_edge,cols);
			Matrix<int> sub_superpixel_label = m_superpixel_label(Range(i,row_end),Range(j,col_end));			
			
			sub_superpixel_label = label_count;
			m_superpixel_center(label_count,0) = (j + col_end) / 2.0f;
			m_superpixel_center(label_count,1) = (i + row_end) / 2.0f;

			label_count++;
		}
	}

	if (OUTPUT_PORT_SUPERPIXEL_IMAGE_DATA_STATE)
	{
		//output superpixel image
		m_superpixel_img = averageImageWithLabel(m_superpixel_label,input_img);
	}

	ImageSignal<int>* superpixel_label_output_sig = TO_IMAGE_SIGNAL(int,getOutputPort(OUTPUT_PORT_LABEL_DATA));
	superpixel_label_output_sig->img = m_superpixel_label;

	ImageSignal<PixelType>* superpixel_img_output_sig = TO_IMAGE_SIGNAL(PixelType,getOutputPort(OUTPUT_PORT_SUPERPIXEL_IMAGE_DATA));
	superpixel_img_output_sig->img = m_superpixel_img;
}

template<class ImageSigT>
bool GridSuperPixelNode<ImageSigT>::selfcheck()
{
	if (m_superpixel_size <= 1 && m_superpixel_num <= 0)
	{
		EAGLEEYE_ERROR("sorry, superpixel size couldn't be less than zero\n");
		return false;
	}

	if (!(TO_IMAGE_SIGNAL(PixelType,getInputPort())))
	{
		EAGLEEYE_ERROR("sorry, input port isn't proper\n");
		return false;
	}

	return true;
}

}