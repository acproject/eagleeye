namespace eagleeye
{
template<class ImageSigT>
CLAHENode<ImageSigT>::CLAHENode()
{
	m_histogram_bins = 255;
	m_clip_limit = 1.6f/400.0f;

	m_r_block = 1;
	m_c_block = 1;

	//generate output signal
	setNumberOfOutputSignals(1);
	setOutputPort(makeOutputSignal(),0);

	//generate input signal
	setNumberOfInputSignals(1);

	//////////////////////////////////////////////////////////////////////////
	//build monitor var
	EAGLEEYE_MONITOR_VAR(int,setHisBins,getHisBins,"his_bins");
	EAGLEEYE_MONITOR_VAR(float,setClipLimit,getClipLimit,"clip_limit");
	EAGLEEYE_MONITOR_VAR(int,setRBlocks,getRBlocks,"row_blocks");
	EAGLEEYE_MONITOR_VAR(int,setCBlocks,getCBlocks,"col_blocks");

	//////////////////////////////////////////////////////////////////////////
}
template<class ImageSigT>
CLAHENode<ImageSigT>::~CLAHENode()
{

}

template<class ImageSigT>
void CLAHENode<ImageSigT>::executeNodeInfo()
{
	ImageSigT* input_img_sig = getInputImageSignal(INPUT_PORT_IMAGE_DATA);

	Matrix<PixelType> input_img = input_img_sig->img;
	PixelType min_val,max_val;
	getMaxMin(input_img,max_val,min_val);
	typedef NormalizeOperation<PixelType,float> FrontNormType;
	Matrix<float> img = input_img.transform(FrontNormType(min_val,max_val,0.0f,1.0f));

	cv::Mat cvimg(input_img.rows(),input_img.cols(),CV_32F,img.dataptr());
	clahe(cvimg,m_c_block,m_r_block,m_histogram_bins,m_clip_limit);
	
	typedef NormalizeOperation<float,PixelType> BackNormType;
	Matrix<PixelType> output_img = Matrix<float>(cvimg.rows,cvimg.cols,cvimg.data,false).transform(BackNormType(0.0f,1.0f,min_val,max_val));

	ImageSigT* output_img_sig = getOutputImageSignal(OUTPUT_PORT_CLAHE_IMAGE_DATA);	
	output_img_sig->img = output_img;

	EAGLEEYE_INFO("finish CLAHE\n");
}

template<class ImageSigT>
bool CLAHENode<ImageSigT>::selfcheck()
{
	Superclass::selfcheck();

	if (AtomicTypeTrait<PixelType>::size != 1)
	{
		EAGLEEYE_ERROR("sorry, only support image with single channel\n");
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
void CLAHENode<ImageSigT>::setHisBins(const int bins_num)
{
	m_histogram_bins = bins_num;

	//force time update
	modified();
}
template<class ImageSigT>
void CLAHENode<ImageSigT>::getHisBins(int& bins_num)
{
	bins_num = m_histogram_bins;
}

template<class ImageSigT>
void CLAHENode<ImageSigT>::setClipLimit(const float clip_lim)
{
	m_clip_limit = clip_lim;

	modified();
}
template<class ImageSigT>
void CLAHENode<ImageSigT>::getClipLimit(float& clip_lim)
{
	clip_lim = m_clip_limit;
}

template<class ImageSigT>
void CLAHENode<ImageSigT>::setRBlocks(const int blocks_num)
{
	m_r_block = blocks_num;

	modified();
}
template<class ImageSigT>
void CLAHENode<ImageSigT>::getRBlocks(int& blocks_num)
{
	blocks_num = m_r_block;
}

template<class ImageSigT>
void CLAHENode<ImageSigT>::setCBlocks(const int blocks_num)
{
	m_c_block = blocks_num;

	modified();
}
template<class ImageSigT>
void CLAHENode<ImageSigT>::getCBlocks(int& blocks_num)
{
	blocks_num = m_c_block;
} 

template<class ImageSigT>
void CLAHENode<ImageSigT>::clipHistogram(cv:: Mat& hist ,int ul_clip_limit)
{
	int bins = hist. rows;
	int ul_nr_excess = 0;//the number of exceeding the limit(ulClipLimit)

	for (int i = 0; i < bins ; ++i)
	{
		float* hv = hist. ptr<float >(i);
		if (hv [0] > ul_clip_limit)
		{
			ul_nr_excess += (int )(hv[0] - ul_clip_limit);
		}
	}

	//Second part: clip histogram and redistribute excess pixels in each bin
	int ul_bin_incr = ul_nr_excess / bins;
	int ul_upper = ul_clip_limit - ul_bin_incr;

	for (int i = 0; i < bins; ++i)
	{
		float* hv = hist. ptr<float>(i);
		if (hv [0] > ul_clip_limit)
		{
			hv[0] = (float)ul_clip_limit;
		}
		else
		{
			if (hv [0] > ul_upper)
			{
				ul_nr_excess -= (int)(hv[0] - ul_upper);
				hv[0] = (float)ul_clip_limit;
			}
			else
			{
				ul_nr_excess -= ul_bin_incr ;
				hv[0] += ul_bin_incr ;
			}
		}
	}

	//Redistrubte the remaining excess
	int curindex = 0;
	while(ul_nr_excess )
	{
		while(ul_nr_excess && curindex < bins)
		{
			int ul_step_size = bins / ul_nr_excess;
			if (ul_step_size < 1) ul_step_size = 1;

			for (int i = curindex;i < bins&& ul_nr_excess;i = i + ul_step_size)
			{
				float* hv = hist. ptr<float >(i);
				if (hv [0] < ul_clip_limit)
				{
					hv[0]++;
					ul_nr_excess--;
				}
			}

			curindex++;
		}
	}
}

//compute accumulate histogram
template<class ImageSigT>
void CLAHENode<ImageSigT>::mapHistogram(cv:: Mat& hist ,float pixmin,float pixmax, unsigned int ul_nr_of_pixels)
{
	unsigned long ul_sum = 0;
	float fscale = (pixmax - pixmin) / ul_nr_of_pixels ;
	unsigned int bins= hist.rows ;

	for (unsigned int i = 0; i < bins; ++i)
	{
		float* hv = hist.ptr<float>(i);
		ul_sum += unsigned long(hv [0]);
		hv[0] = pixmin + fscale * ul_sum;
		if (hv [0] > pixmax)
			hv[0] = pixmax ;
	}
}

template<class ImageSigT>
void CLAHENode<ImageSigT>::interpolateCLAHEImage(cv:: Mat& sub_image , 
	const cv::Mat & histLU, const cv ::Mat& histRU, 
	const cv ::Mat& histLB,const cv:: Mat& histRB, 
	float pixmin, float pixmax )
{
	int on_cols = sub_image. cols;
	int on_rows = sub_image. rows;

	int bins = histLU. rows;
	float bin_size = (pixmax - pixmin) / bins ;
	int ui_num = on_rows * on_cols;
	int ui_x_coef ,ui_y_coef;
	int ui_x_inv_coef ,ui_y_inv_coef;

	for (int i = 0; i < on_rows; ++i)
	{
		float* subdata = sub_image. ptr<float >(i);
		for (int j = 0; j < on_cols; ++j)
		{
			int index = int(floor(( subdata[j] - pixmin) / bin_size + 0.5f));
			index = (index >= bins) ? (bins - 1):index ;
			ui_y_coef = i ;                            
			ui_x_coef = j ;
			ui_y_inv_coef = on_rows - i;        
			ui_x_inv_coef = on_cols - j;

			const float * histLUV = histLU.ptr<float>( index);
			const float * histRUV = histRU.ptr<float>( index);
			const float * histLBV = histLB.ptr<float>( index);
			const float * histRBV = histRB.ptr<float>( index);

			subdata[j] = (ui_y_inv_coef * ( ui_x_inv_coef * histLUV [0] + ui_x_coef * histRUV[0]) + 
				ui_y_coef * (ui_x_inv_coef * histLBV[0] + ui_x_coef * histRBV[0])) / float(ui_num);
		}
	}

}

template<class ImageSigT>
void CLAHENode<ImageSigT>::clahe(cv:: Mat& image , 
	unsigned int ui_nr_x ,unsigned int ui_nr_y , 
	unsigned int ui_nr_bins,float f_clip_limit)
{
	unsigned int ui_x_size, ui_y_size;
	unsigned int ul_clip_limit;
	std::vector<cv:: Mat> histarray ;

	int on_rows ,on_cols;
	on_rows = image.rows;
	on_cols = image.cols;

	//split the matrix
	//ui_x_size and ui_y_size are the width and height of every sub region
	//sub_pixels_num is the total pixel number in every sub region
	ui_x_size = on_cols / ui_nr_x;
	ui_y_size = on_rows / ui_nr_y;
	int sub_pixels_num = ui_x_size * ui_y_size;

	histarray.resize (ui_nr_x * ui_nr_y);
	int histsize [] = {ui_nr_bins};
	float pmax = 0;
	float pmin = 10000000;

	//find maxvalue and minvalue in imagedata
	for (int i = 0; i < on_rows ; ++i)
	{
		float* imagedata =image. ptr<float >(i);
		for (int j=0; j<on_cols ;++j)
		{
			if (pmax <imagedata[j])
				pmax=imagedata [j];
			if (pmin >imagedata[j])
				pmin=imagedata [j];
		}
	}

	float hranges [] = {pmin, pmax};
	const float * ranges[] = {hranges};

	int channels [] = {0};
	//compute actual cliplimit
	int min_clip_limit = (ui_x_size * ui_y_size) / ui_nr_bins ;
	min_clip_limit = (min_clip_limit < 1) ? 1 : min_clip_limit;

	//the clip number
	ul_clip_limit = unsigned int(min_clip_limit + f_clip_limit * ( ui_x_size * ui_y_size - min_clip_limit));

	//compute the histogram on every region and compute accumulate histogram
	//we should notice that the accumulate histogram is just the transformation function.
	//this accumulate histogram is stored in "histarray"
	int histindex = 0;
	for (unsigned int i = 0; i < ui_nr_y; ++i)
	{
		for (unsigned int j = 0; j < ui_nr_x; ++j)
		{
			cv::Mat subimage = image(cv:: Range(i * ui_y_size,(i + 1) * ui_y_size ),cv:: Range(j * ui_x_size,(j + 1) * ui_x_size ));

			cv::calcHist (&subimage,1, channels,cv ::Mat(), histarray[histindex],1,histsize, ranges);

			clipHistogram(histarray [histindex], ul_clip_limit);
			mapHistogram(histarray [histindex], pmin,pmax ,sub_pixels_num);
			histindex++;
		}
	}

	int ui_yu ,ui_yb, ui_xl,ui_xr ;
	int ui_subx ,ui_suby;
	int pr1 = 0;
	int pr2 = 0;
	int pc1 = 0;
	int pc2 = 0;
	for (unsigned int ui_y = 0; ui_y <= ui_nr_y; ++ui_y)
	{
		if (ui_y == 0)                                     //special case :top row
		{
			ui_yu = ui_yb = 0;
			ui_suby = ui_y_size / 2;
		}
		else
		{
			if (ui_y == ui_nr_y)                       //special case: bottom row
			{
				ui_yu = ui_yb = ui_nr_y - 1;
				ui_suby = ui_y_size / 2;
			}
			else
			{
				ui_yu = ui_y - 1;
				ui_yb = ui_yu + 1;
				ui_suby = ui_y_size ;
			}
		}

		if (ui_y != ui_nr_y)
		{
			pr1 = pr2;
			pr2 = pr1 + ui_suby;
		}
		else
		{
			pr1 = pr2;
			pr2 = on_rows;
		}


		pc1 = 0;pc2 = 0;
		for (unsigned int ui_x = 0; ui_x <= ui_nr_x; ++ui_x)
		{
			if (ui_x == 0)                             //special case: left column
			{
				ui_xl = ui_xr = 0;
				ui_subx = ui_x_size / 2;
			}
			else
			{
				if (ui_x == ui_nr_x)
				{
					ui_xl = ui_xr = ui_nr_x - 1;
					ui_subx = ui_x_size / 2;
				}
				else
				{
					ui_xl = ui_x - 1;
					ui_xr = ui_xl + 1;
					ui_subx = ui_x_size ;
				}                               
			}

			cv::Mat & histLU = histarray[ui_yu * ui_nr_x + ui_xl];
			cv::Mat & histRU = histarray[ui_yu * ui_nr_x + ui_xr];
			cv::Mat & histLB = histarray[ui_yb * ui_nr_x + ui_xl];
			cv::Mat & histRB = histarray[ui_yb * ui_nr_x + ui_xr];

			if (ui_x != ui_nr_x)
			{
				pc1 = pc2 ;
				pc2 = pc1 + ui_subx;
			}
			else
			{
				pc1 = pc2 ;
				pc2 = on_cols ;
			}

			cv::Mat subimage = image(cv ::Range( pr1,pr2 ),cv:: Range(pc1 ,pc2));
			interpolateCLAHEImage(subimage ,histLU, histRU,histLB ,histRB, pmin,pmax );
		}
	}
}
}