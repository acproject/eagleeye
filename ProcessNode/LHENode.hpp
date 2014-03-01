namespace eagleeye
{
template<class ImageSigT>
LHENode<ImageSigT>::LHENode()
{
	m_spatial_sampling = 10.0f;
	m_gray_range_sampling = 0.002f;
	m_hist_limit_ratio = 0.0f;

	m_down_rows = 1;
	m_down_cols = 1;
	m_down_depth = 1;

	m_pdf_grid = NULL;
	m_cdf_grid = NULL;

	m_img_update_time = 0;
	m_roi_region = Array<int,4>(0);

	/**
	 *	@brief set the number of input port
	 */
	setNumberOfInputSignals(1);

	/**
	 *	@brief set output port property
	 */
	setNumberOfOutputSignals(1);
	setOutputPort(makeOutputSignal(),OUTPUT_PORT_LHE_RESULT_DATA);

	//////////////////////////////////////////////////////////////////////////
	//build monitor var
	EAGLEEYE_MONITOR_VAR(float, setSamplingSpatial,getSamplingSpatial,"spatial_sampling");
	EAGLEEYE_MONITOR_VAR(float, setSamplingGrayRange,getSamplingGrayRange,"gray_sampling");
	EAGLEEYE_MONITOR_VAR(float, setHistLimitRatio,getHistLimitRatio,"hist_limit_ratio");
	//////////////////////////////////////////////////////////////////////////
}
template<class ImageSigT>
LHENode<ImageSigT>::~LHENode()
{
	if (m_pdf_grid)
	{
		delete []m_pdf_grid;
	}
	
	if (m_cdf_grid)
	{
		delete []m_cdf_grid;
	}
}

template<class ImageSigT>
void LHENode<ImageSigT>::setSamplingSpatial(float spatial_sampling)
{
	m_spatial_sampling = spatial_sampling;

	modified();
}

template<class ImageSigT>
void LHENode<ImageSigT>::getSamplingSpatial(float& spatial_sampling)
{
	spatial_sampling = m_spatial_sampling;
}

template<class ImageSigT>
void LHENode<ImageSigT>::setSamplingGrayRange(float gray_gray_sampling)
{
	m_gray_range_sampling = gray_gray_sampling;	

	modified();
}

template<class ImageSigT>
void LHENode<ImageSigT>::getSamplingGrayRange(float& gray_gray_sampling)
{
	gray_gray_sampling = m_gray_range_sampling;
}

template<class ImageSigT>
void LHENode<ImageSigT>::setHistLimitRatio(float hist_limit_ratio)
{
	m_hist_limit_ratio = hist_limit_ratio;

	modified();
}

template<class ImageSigT>
void LHENode<ImageSigT>::getHistLimitRatio(float& hist_limit_ratio)
{
	hist_limit_ratio = m_hist_limit_ratio;
}

template<class ImageSigT>
void LHENode<ImageSigT>::buildPDFGrid(const Matrix<float>& img)
{
	int rows = img.rows();
	int cols = img.cols();

	//down sample
	m_down_rows = int(ceil(rows / m_spatial_sampling + 0.5f));
	m_down_cols = int(ceil(cols / m_spatial_sampling + 0.5f));
	m_down_depth = int(ceil(1.0f / m_gray_range_sampling + 0.5f));

	//allocate pdf memory
	if (m_pdf_grid)
	{
		delete []m_pdf_grid;
		m_pdf_grid = NULL;
	}
	m_pdf_grid = new float[m_down_rows * m_down_cols * m_down_depth];
	memset(m_pdf_grid,0,sizeof(float) * m_down_rows * m_down_cols * m_down_depth);

	for (int i = 0; i < rows; ++i)
	{
		const float* img_data = img.row(i);
		for (int j = 0; j < cols; ++j)
		{
			int down_i = int(floor(i / m_spatial_sampling + 0.5f));
			int down_j = int(floor(j / m_spatial_sampling + 0.5f));
			int down_d = int(floor(img_data[j] / m_gray_range_sampling + 0.5f));

			int down_index = down_d * m_down_rows * m_down_cols + down_i * m_down_cols + down_j;
			m_pdf_grid[down_index] = m_pdf_grid[down_index] + 1.0f;
		}
	}

	//transform count to probability
	float* depth_counts = new float[m_down_rows * m_down_cols];
	memset(depth_counts,0,sizeof(float) * m_down_rows * m_down_cols);

	for (int depth = 0; depth < m_down_depth; ++depth)
	{
		int slice_index = depth * m_down_rows * m_down_cols;

		for (int i = 0; i < m_down_rows; ++i)
		{
			int row_index = i * m_down_cols;
			for (int j = 0; j < m_down_cols; ++j)
			{
				depth_counts[row_index + j] += m_pdf_grid[slice_index + row_index +j];
			}
		}
	}


	for (int i = 0; i < m_down_rows; ++i)
	{
		int row_index = i * m_down_cols;
		for (int j = 0; j < m_down_cols; ++j)
		{
			if(depth_counts[row_index + j] == 0.0f)
				depth_counts[row_index + j] = 1.0f;
		}
	}

	for (int depth = 0; depth < m_down_depth; ++depth)
	{
		int slice_index = depth * m_down_rows * m_down_cols;

		for (int i = 0; i < m_down_rows; ++i)
		{
			int row_index = i * m_down_cols;
			for (int j = 0; j < m_down_cols; ++j)
			{
				m_pdf_grid[slice_index + row_index +j] /= depth_counts[row_index + j];
			}
		}
	}

	delete []depth_counts;
}

template<class ImageSigT>
void LHENode<ImageSigT>::lheBuildCDFGrid()
{
	//compute cumulative distribution histogram
	if (m_cdf_grid)
	{
		delete []m_cdf_grid;
	}
	m_cdf_grid = new float[m_down_rows * m_down_cols * m_down_depth];
	memset(m_cdf_grid,0,sizeof(float) * m_down_rows * m_down_cols * m_down_depth);
	memcpy(m_cdf_grid,m_pdf_grid,sizeof(float) * m_down_rows * m_down_cols);

	for (int depth = 1; depth < m_down_depth; ++depth)
	{
		int slice_index = depth * m_down_rows * m_down_cols;
		int up_slice_index = (depth - 1)* m_down_rows * m_down_cols;

		for (int i = 0; i < m_down_rows; ++i)
		{
			int row_index = i * m_down_cols;
			for (int j = 0; j < m_down_cols; ++j)
			{
				m_cdf_grid[slice_index + row_index + j] += m_cdf_grid[up_slice_index + row_index +j] + 
					m_pdf_grid[slice_index + row_index + j];
			}
		}
	}
}

template<class ImageSigT>
void LHENode<ImageSigT>::sliceCDFGrid(const Matrix<float>& img,Matrix<float>& result)
{
	int rows,cols,offset_r,offset_c;
	if (m_roi_region != 0)
	{
		rows = m_roi_region[3] - m_roi_region[1];
		cols = m_roi_region[2] - m_roi_region[0];
		offset_r = m_roi_region[1];
		offset_c = m_roi_region[0];
	}
	else
	{
		rows = img.rows();
		cols = img.cols();
		offset_r = 0;
		offset_c = 0;
	}

	//tri_linear interpolation
	result = Matrix<float>(rows,cols,float(0));

	int slice_offset = m_down_rows * m_down_cols;

	for (int i = 0; i < rows; ++i)
	{
		const float* img_data = img.row(i + offset_r);
		float* result_data = result.row(i);
		for (int j = 0; j < cols; ++j)
		{
			float down_i = (i + offset_r) / m_spatial_sampling + eagleeye_eps;
			int predowni = int(floor(down_i));
			int nexdowni = int(ceil(down_i));
			nexdowni = (nexdowni >= m_down_rows)?(m_down_rows - 1):nexdowni;

			float down_j = (j + offset_c) / m_spatial_sampling + eagleeye_eps;
			int predownj = int(floor(down_j));
			int nexdownj = int(ceil(down_j));
			nexdownj = (nexdownj >= m_down_cols)?(m_down_cols - 1):nexdownj;

			float down_d = img_data[j + offset_c] / m_gray_range_sampling + eagleeye_eps;
			int predownz = int(floor(down_d));
			int nexdownz = int(ceil(down_d));
			nexdownz = (nexdownz >= m_down_depth)?(m_down_depth - 1):nexdownz;


			//i direction interpolation(create 4 interpolated points)
			float alphai = 1 - (down_i - predowni);
			float value1 = alphai * m_cdf_grid[predownz * slice_offset + predowni * m_down_cols + predownj] + 
				(1 - alphai) * m_cdf_grid[predownz * slice_offset + nexdowni * m_down_cols + predownj];

			float value2 = alphai * m_cdf_grid[predownz * slice_offset + predowni * m_down_cols + nexdownj] + 
				(1 - alphai) * m_cdf_grid[predownz * slice_offset + nexdowni * m_down_cols + nexdownj];

			float value3 = alphai * m_cdf_grid[nexdownz * slice_offset + predowni * m_down_cols + nexdownj] + 
				(1 - alphai) * m_cdf_grid[nexdownz * slice_offset + nexdowni * m_down_cols + nexdownj];

			float value4 = alphai * m_cdf_grid[nexdownz * slice_offset + predowni * m_down_cols + predownj] + 
				(1 - alphai) * m_cdf_grid[nexdownz * slice_offset + nexdowni * m_down_cols + predownj];

			//j direction interpolation(create 2 interpolated points)
			float alphaj = 1 - (down_j - predownj);
			float value01 = alphaj * value1 + (1 - alphaj) * value2;
			float value02 = alphaj * value4 + (1 - alphaj) * value3;

			//z direction interpolation(create 1 interpolated points)
			float alphaz = 1 - (down_d - predownz);
			float value000 = alphaz * value01 + (1 - alphaz) * value02;

			result_data[j] = value000;
		}
	}
}

template<class ImageSigT>
void LHENode<ImageSigT>::lheProcess(const Matrix<float>& img, Matrix<float>& result)
{
	if (m_img_update_time < getInputPort()->getMTime())
	{
		buildPDFGrid(img);
		lheBuildCDFGrid();

		m_img_update_time = getInputPort()->getMTime();
	}

	sliceCDFGrid(img,result);
}

template<class ImageSigT>
void LHENode<ImageSigT>::clheBuildCDFGrid()
{
	//modify PDF (Probability Distribution Function)
	int slice_offset = m_down_rows * m_down_cols;

	for (int i = 0; i < m_down_rows; ++i)
	{
		int row_index = i * m_down_cols;
		for (int j = 0; j < m_down_cols; ++j)
		{
			//find max value along gray axis
			float max_val = 0;
			for (int depth_index = 0; depth_index < m_down_depth; ++depth_index)
			{
				if (max_val < m_pdf_grid[depth_index * slice_offset + row_index + j])
				{
					max_val = m_pdf_grid[depth_index * slice_offset + row_index + j];
				}
			}

			float threashold = m_hist_limit_ratio * max_val;

			float extra_val = 0;
			for (int depth_index = 0; depth_index < m_down_depth; ++depth_index)
			{
				if (m_pdf_grid[depth_index * slice_offset + row_index + j] > threashold)
				{
					extra_val += (m_pdf_grid[depth_index * slice_offset + row_index + j] - 
						threashold);
				}
			}

			//we should know that some info is missed
			float ave_extra_val = extra_val / float(m_down_depth);
			for (int depth_index = 0; depth_index < m_down_depth; ++depth_index)
			{
				if (m_pdf_grid[depth_index * slice_offset + row_index + j] > threashold)
				{
					m_pdf_grid[depth_index * slice_offset + row_index + j] = threashold;
				}
				else
				{
					m_pdf_grid[depth_index * slice_offset + row_index + j] += ave_extra_val;
				}
			}
		}
	}


	//compute CDF (Cumulative Distribution Function)
	//compute cumulative distribution histogram
	if (m_cdf_grid)
	{
		delete []m_cdf_grid;
	}
	m_cdf_grid = new float[m_down_rows * m_down_cols * m_down_depth];
	memset(m_cdf_grid,0,sizeof(float) * m_down_rows * m_down_cols * m_down_depth);
	memcpy(m_cdf_grid,m_pdf_grid,sizeof(float) * m_down_rows * m_down_cols);

	for (int depth = 1; depth < m_down_depth; ++depth)
	{
		int slice_index = depth * m_down_rows * m_down_cols;
		int up_slice_index = (depth - 1)* m_down_rows * m_down_cols;

		for (int i = 0; i < m_down_rows; ++i)
		{
			int row_index = i * m_down_cols;
			for (int j = 0; j < m_down_cols; ++j)
			{
				m_cdf_grid[slice_index + row_index + j] += m_cdf_grid[up_slice_index + row_index +j] + 
					m_pdf_grid[slice_index + row_index + j];
			}
		}
	}
}

template<class ImageSigT>
void LHENode<ImageSigT>::setROIRegion(Array<int,4> roi)
{
	if (roi != 0)
	{
		m_roi_region = roi;

		//force time update
		modified();
	}
}

template<class ImageSigT>
void LHENode<ImageSigT>::clheProcess(const Matrix<float>& img,Matrix<float>& result)
{
	if (m_img_update_time < getOutputPort()->getMTime())
	{
		buildPDFGrid(img);
		clheBuildCDFGrid();

		m_img_update_time = getOutputPort()->getMTime();
	}
	sliceCDFGrid(img,result);
}

template<class ImageSigT>
void LHENode<ImageSigT>::executeNodeInfo()
{
	ImageSigT* input_img_sig = getInputImageSignal(INPUT_PORT_IMAGE_DATA);

	Matrix<PixelType> input_img = input_img_sig->img;
	PixelType min_val = AtomicTypeTrait<PixelType>::minval();
	PixelType max_val = AtomicTypeTrait<PixelType>::maxval();

	typedef NormalizeOperation<PixelType,float> FrontNormType;
	Matrix<float> img = input_img.transform(FrontNormType(min_val,max_val,0.0f,1.0f));

	Matrix<float> result;

	if ((m_hist_limit_ratio > 0.0f) && (m_hist_limit_ratio < 1.0f))
	{
		clheProcess(img,result);
	}
	else
	{
		lheProcess(img,result);
	}

	typedef NormalizeOperation<float,PixelType> BackNormType;
	Matrix<PixelType> output_img;
	output_img = result.transform(BackNormType(0.0f,1.0f,min_val,max_val));

	ImageSigT* output_img_sig = getOutputImageSignal(OUTPUT_PORT_LHE_RESULT_DATA);
	if (!output_img_sig)
	{
		EAGLEEYE_ERROR("sorry,output image is not correct\n");
		return;
	}

	output_img_sig->img = output_img;

	if ((m_hist_limit_ratio > 0.0f) && (m_hist_limit_ratio < 1.0f))
	{
		EAGLEEYE_INFO("finish CLHE ...\n");
	}
	else
	{
		EAGLEEYE_INFO("finish LHE ...\n");
	}
	
}

template<class ImageSigT>
bool LHENode<ImageSigT>::selfcheck()
{
	Superclass::selfcheck();

	if (AtomicTypeTrait<PixelType>::size != 1)
	{
		EAGLEEYE_ERROR("sorry,only support image with single channel\n");
		return false;
	}

	if (!getInputImageSignal(INPUT_PORT_IMAGE_DATA))
	{
		EAGLEEYE_ERROR("sorry,image pixel isn't consistent...\n");
		return false;
	}

	return true;
}

}