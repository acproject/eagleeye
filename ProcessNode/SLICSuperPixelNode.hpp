namespace eagleeye
{
template<class ImageSigT>
SLICSuperPixelNode<ImageSigT>::SLICSuperPixelNode()
{
	m_compactness = 20.0f;
	m_k_superpixles = 0;
	m_size_superpixel = 100;
	bool m_perturb_flag = false;
	m_label_num = 0;

	//set output port
	setNumberOfOutputSignals(2);
	setOutputPort(new ImageSignal<int>,0);
	setOutputPort(new ImageSignal<PixelType>,1);

	//generate input signal
	setNumberOfInputSignals(1);

	//close this output port dy default
	disableOutputPort(OUTPUT_PORT_SUPERPIXEL_IMAGE_DATA);

	//////////////////////////////////////////////////////////////////////////
	//add monitor variable
	EAGLEEYE_MONITOR_VAR(int,setSuperPixelsNum,getSuperPixelsNum,"superpixel_number");
	EAGLEEYE_MONITOR_VAR(int,setSuperPixelSize,getSuperPixelSize,"superpixel_size");
	EAGLEEYE_MONITOR_VAR(float,setCompactness,getCompactness,"compactness");
	//////////////////////////////////////////////////////////////////////////
}

template<class ImageSigT>
SLICSuperPixelNode<ImageSigT>::~SLICSuperPixelNode()
{

}

template<class ImageSigT>
Matrix<typename ImageSigT::MetaType> SLICSuperPixelNode<ImageSigT>::getSuperPixelImage()
{
	return m_superpixel_img;
}

template<class ImageSigT>
Matrix<int> SLICSuperPixelNode<ImageSigT>::getLableImage()
{
	return m_label_img;
}

template<class ImageSigT>
Matrix<float> SLICSuperPixelNode<ImageSigT>::getSuperpixelCenter()
{
	return m_superpixel_center;
}

template<class ImageSigT>
void SLICSuperPixelNode<ImageSigT>::executeNodeInfo()
{
	ImageSigT* input_img_sig = dynamic_cast<ImageSigT*>(getInputPort());
	if (!input_img_sig)
	{
		EAGLEEYE_ERROR("sorry,input image is not correct\n");
		return;
	}
	Matrix<PixelType> input_img = input_img_sig->img;

	doSuperpixelSegmentation(input_img);

	ImageSignal<int>* output_label_sig = dynamic_cast<ImageSignal<int>*>(getOutputPort(0));
	output_label_sig->img = m_label_img;

	if (m_output_port_state[OUTPUT_PORT_SUPERPIXEL_IMAGE_DATA])
	{
		//generate superpixel image
		m_superpixel_img = averageImageWithLabel(m_label_img,input_img);

		ImageSignal<PixelType>* superpixel_img_sig = dynamic_cast<ImageSignal<PixelType>*>(getOutputPort(1));
		superpixel_img_sig->img = m_superpixel_img;
	}
}

template<class ImageSigT>
bool SLICSuperPixelNode<ImageSigT>::selfcheck()
{
	Superclass::selfcheck();

	if(AtomicTypeTrait<PixelType>::size != 1 && AtomicTypeTrait<PixelType>::size != 3)
	{
		EAGLEEYE_ERROR("sorry,SLIC superpixel module only support image with single or three channels\n");
		return false;
	}

	if ((AtomicTypeTrait<PixelType>::size == 3) && (AtomicTypeTrait<PixelType>::pixel_type != EAGLEEYE_RGB))
	{
		EAGLEEYE_ERROR("sorry,output must be RGB");
		return false;
	}

	return true;
}

template<class ImageSigT>
void SLICSuperPixelNode<ImageSigT>::perturbLABSeeds(const Matrix<Array<float,3>>& lab,
													   const Matrix<float>& edge_mag, 													   
													   std::vector<float>& kseedsl, 
													   std::vector<float>& kseedsa, 
													   std::vector<float>& kseedsb, 
													   std::vector<float>& kseedsx, 
													   std::vector<float>& kseedsy)
{
	int width = edge_mag.cols();
	int height = edge_mag.rows();
	const float* gradient_map_data = edge_mag.dataptr();
	const Array<float,3>* lab_data = lab.dataptr();

	const int dx8[8] = {-1, -1,  0,  1, 1, 1, 0, -1};
	const int dy8[8] = { 0, -1, -1, -1, 0, 1, 1,  1};

	int numseeds = kseedsl.size();

	for( int n = 0; n < numseeds; n++ )
	{
		int ox = kseedsx[n];//original x
		int oy = kseedsy[n];//original y
		int oind = oy*width + ox;

		int storeind = oind;
		for( int i = 0; i < 8; i++ )
		{
			int nx = ox+dx8[i];//new x
			int ny = oy+dy8[i];//new y

			if( nx >= 0 && nx < width && ny >= 0 && ny < height)
			{
				int nind = ny*width + nx;
				if( gradient_map_data[nind] < gradient_map_data[storeind])
				{
					storeind = nind;
				}
			}
		}
		if(storeind != oind)
		{
			kseedsx[n] = storeind%width;
			kseedsy[n] = storeind/width;
			kseedsl[n] = lab_data[storeind][0];
			kseedsa[n] = lab_data[storeind][1];
			kseedsb[n] = lab_data[storeind][2];
		}
	}
}

template<class ImageSigT>
void SLICSuperPixelNode<ImageSigT>::getLABXYSeeds(std::vector<float>& kseedsl, 
													 std::vector<float>& kseedsa, 
													 std::vector<float>& kseedsb, 
													 std::vector<float>& kseedsx, 
													 std::vector<float>& kseedsy, 
													 const int step, 
													 const Matrix<float>& edge_mag, 
													 const Matrix<Array<float,3>>& lab, 
													 const bool perturb_seeds_flag)
{
	int width = edge_mag.cols();
	int height = edge_mag.rows();

	const bool hexgrid = false;
	int numseeds(0);
	int n(0);

	int xstrips = (0.5+float(width)/float(step));
	int ystrips = (0.5+float(height)/float(step));

	int xerr = width  - step*xstrips;if(xerr < 0){xstrips--;xerr = width - step*xstrips;}
	int yerr = height - step*ystrips;if(yerr < 0){ystrips--;yerr = height- step*ystrips;}

	float xerrperstrip = float(xerr)/float(xstrips);
	float yerrperstrip = float(yerr)/float(ystrips);

	int xoff = step/2;
	int yoff = step/2;
	//-------------------------
	numseeds = xstrips*ystrips;
	//-------------------------
	kseedsl.resize(numseeds);
	kseedsa.resize(numseeds);
	kseedsb.resize(numseeds);
	kseedsx.resize(numseeds);
	kseedsy.resize(numseeds);

	const Array<float,3>* lab_data=lab.dataptr();

	for( int y = 0; y < ystrips; y++ )
	{
		int ye = y*yerrperstrip;
		for( int x = 0; x < xstrips; x++ )
		{
			int xe = x*xerrperstrip;
			int seedx = (x*step+xoff+xe);
			if(hexgrid){ seedx = x*step+(xoff<<(y&0x1))+xe; seedx = eagleeye_min(width-1,seedx); }//for hex grid sampling
			int seedy = (y*step+yoff+ye);
			int i = seedy*width + seedx;

			kseedsl[n] = lab_data[i][0];
			kseedsa[n] = lab_data[i][1];
			kseedsb[n] = lab_data[i][2];
			kseedsx[n] = seedx;
			kseedsy[n] = seedy;
			n++;
		}
	}

	if(perturb_seeds_flag)
	{
		perturbLABSeeds(lab,edge_mag,kseedsl, kseedsa, kseedsb, kseedsx, kseedsy);
	}
}

template<class ImageSigT>
void SLICSuperPixelNode<ImageSigT>::performSuperpixelSLIC(std::vector<float>& kseedsl, 
															 std::vector<float>& kseedsa, 
															 std::vector<float>& kseedsb, 
															 std::vector<float>& kseedsx, 
															 std::vector<float>& kseedsy, 
															 Matrix<int>& labels,
															 const int step, 
															 const Matrix<float>& edge_mag, 
															 const Matrix<Array<float,3>>& lab,
															 const float compactness)
{
	int width=edge_mag.cols();
	int height=edge_mag.rows();

	int sz = width*height;
	const int numk = kseedsl.size();
	//----------------
	int offset = step;
	//if(STEP < 8) offset = STEP*1.5;//to prevent a crash due to a very small step size
	//----------------

	std::vector<float> clustersize(numk, 0);
	std::vector<float> inv(numk, 0);//to store 1/clustersize[k] values

	std::vector<float> sigmal(numk, 0);
	std::vector<float> sigmaa(numk, 0);
	std::vector<float> sigmab(numk, 0);
	std::vector<float> sigmax(numk, 0);
	std::vector<float> sigmay(numk, 0);
	std::vector<float> distvec(sz, FLT_MAX);

	float invwt = 1.0/((step/compactness)*(step/compactness));

	int x1, y1, x2, y2;
	float l, a, b;
	float dist;
	float distxy;

	const Array<float,3>* lab_data=lab.dataptr();
	int* labels_data=labels.dataptr();

	for( int itr = 0; itr < 10; itr++ )
	{
		distvec.assign(sz, FLT_MAX);
		for( int n = 0; n < numk; n++ )
		{
			y1 = eagleeye_max(0.0,			kseedsy[n]-offset);
			y2 = eagleeye_min((float)height,	kseedsy[n]+offset);
			x1 = eagleeye_max(0.0,			kseedsx[n]-offset);
			x2 = eagleeye_min((float)width,	kseedsx[n]+offset);


			for( int y = y1; y < y2; y++ )
			{
				for( int x = x1; x < x2; x++ )
				{
					int i = y*width + x;

					l = lab_data[i][0];
					a = lab_data[i][1];
					b = lab_data[i][2];

					dist =			(l - kseedsl[n])*(l - kseedsl[n]) +
						(a - kseedsa[n])*(a - kseedsa[n]) +
						(b - kseedsb[n])*(b - kseedsb[n]);

					distxy =		(x - kseedsx[n])*(x - kseedsx[n]) +
						(y - kseedsy[n])*(y - kseedsy[n]);

					//------------------------------------------------------------------------
					dist += distxy*invwt;//dist = sqrt(dist) + sqrt(distxy*invwt);//this is more exact
					//------------------------------------------------------------------------
					if( dist < distvec[i] )
					{
						distvec[i] = dist;
						labels_data[i]  = n;
					}
				}
			}
		}
		//-----------------------------------------------------------------
		// Recalculate the centroid and store in the seed values
		//-----------------------------------------------------------------
		//instead of reassigning memory on each iteration, just reset.

		sigmal.assign(numk, 0);
		sigmaa.assign(numk, 0);
		sigmab.assign(numk, 0);
		sigmax.assign(numk, 0);
		sigmay.assign(numk, 0);
		clustersize.assign(numk, 0);
		//------------------------------------
		//edgesum.assign(numk, 0);
		//------------------------------------

		{int ind(0);
		for( int r = 0; r < height; r++ )
		{
			for( int c = 0; c < width; c++ )
			{
				sigmal[labels_data[ind]] += lab_data[ind][0];
				sigmaa[labels_data[ind]] += lab_data[ind][1];
				sigmab[labels_data[ind]] += lab_data[ind][2];
				sigmax[labels_data[ind]] += c;
				sigmay[labels_data[ind]] += r;
				//------------------------------------
				//edgesum[klabels[ind]] += edgemag[ind];
				//------------------------------------
				clustersize[labels_data[ind]] += 1.0;
				ind++;
			}
		}}

		{for( int k = 0; k < numk; k++ )
		{
			if( clustersize[k] <= 0 ) clustersize[k] = 1;
			inv[k] = 1.0/clustersize[k];//computing inverse now to multiply, than divide later
		}}

		{for( int k = 0; k < numk; k++ )
		{
			kseedsl[k] = sigmal[k]*inv[k];
			kseedsa[k] = sigmaa[k]*inv[k];
			kseedsb[k] = sigmab[k]*inv[k];
			kseedsx[k] = sigmax[k]*inv[k];
			kseedsy[k] = sigmay[k]*inv[k];
			//------------------------------------
			//edgesum[k] *= inv[k];
			//------------------------------------
		}}
	}
}

//////////////////////////////////////////////////////////////////////////
//for image with single channel
template<class ImageSigT>
void SLICSuperPixelNode<ImageSigT>::getGrayXYSeeds(std::vector<float>& kseedsgray, 
												   std::vector<float>& kseedsx, 
												   std::vector<float>& kseedsy, 
												   const int step, 
												   const Matrix<float>& gray)
{
	int width=gray.cols();
	int height=gray.rows();

	const bool hexgrid = false;
	int numseeds(0);
	int n(0);

	int xstrips = int(0.5+float(width)/float(step));
	int ystrips = int(0.5+float(height)/float(step));

	int xerr = width  - step*xstrips;if(xerr < 0){xstrips--;xerr = width - step*xstrips;}
	int yerr = height - step*ystrips;if(yerr < 0){ystrips--;yerr = height- step*ystrips;}

	float xerrperstrip = float(xerr)/float(xstrips);
	float yerrperstrip = float(yerr)/float(ystrips);

	int xoff = step/2;
	int yoff = step/2;
	//-------------------------
	numseeds = xstrips * ystrips;
	//-------------------------
	kseedsgray.resize(numseeds);
	kseedsx.resize(numseeds);
	kseedsy.resize(numseeds);

	const float* gray_data = gray.dataptr();

	for( int y = 0; y < ystrips; y++ )
	{
		int ye = int(y * yerrperstrip);
		for( int x = 0; x < xstrips; x++ )
		{
			int xe = int(x * xerrperstrip);
			int seedx = (x * step + xoff+xe);
			if(hexgrid){ seedx = x * step + (xoff<<(y & 0x1)) + xe; seedx = eagleeye_min(width-1,seedx); }//for hex grid sampling
			int seedy = (y * step + yoff + ye);
			int i = seedy * width + seedx;

			kseedsgray[n] = gray_data[i];
			kseedsx[n] = float(seedx);
			kseedsy[n] = float(seedy);
			n++;
		}
	}
}

template<class ImageSigT>
void SLICSuperPixelNode<ImageSigT>::performSuperpixelSLIC(std::vector<float>& kseedsgray,
						   std::vector<float>& kseedsx,
						   std::vector<float>& kseedsy,
						   Matrix<int>& labels,
						   const int step,
						   const Matrix<float>& gray,
						   const float compactness)
{
	int width=gray.cols();
	int height=gray.rows();

	int sz = width*height;
	const int numk = kseedsgray.size();
	//----------------
	int offset = step;
	//if(STEP < 8) offset = STEP*1.5;//to prevent a crash due to a very small step size
	//----------------

	std::vector<float> clustersize(numk, 0);
	std::vector<float> inv(numk, 0);//to store 1/clustersize[k] values

	std::vector<float> sigmagray(numk, 0);
	std::vector<float> sigmax(numk, 0);
	std::vector<float> sigmay(numk, 0);
	std::vector<float> distvec(sz, FLT_MAX);

	float invwt = float(1.0f/((step/compactness)*(step/compactness)));

	int x1, y1, x2, y2;
	float val;
	float dist;
	float distxy;

	const float* gray_data=gray.dataptr();
	int* labels_data=labels.dataptr();

	for( int itr = 0; itr < 10; itr++ )
	{
		distvec.assign(sz, FLT_MAX);

		for( int n = 0; n < numk; n++ )
		{
			y1 = int(eagleeye_max(0.0,			kseedsy[n]-offset));
			y2 = int(eagleeye_min((float)height,	kseedsy[n]+offset));
			x1 = int(eagleeye_max(0.0,			kseedsx[n]-offset));
			x2 = int(eagleeye_min((float)width,	kseedsx[n]+offset));

			for( int y = y1; y < y2; y++ )
			{
				for( int x = x1; x < x2; x++ )
				{
					int i = y*width + x;

					val=gray_data[i];

					dist=(val-kseedsgray[n])*(val-kseedsgray[n]);

					distxy =		(x - kseedsx[n])*(x - kseedsx[n]) +
						(y - kseedsy[n])*(y - kseedsy[n]);

					//------------------------------------------------------------------------
					dist += distxy*invwt;//dist = sqrt(dist) + sqrt(distxy*invwt);//this is more exact
					//------------------------------------------------------------------------
					if( dist < distvec[i] )
					{
						distvec[i] = dist;
						labels_data[i]  = n;
					}
				}
			}
		}
		//-----------------------------------------------------------------
		// Recalculate the centroid and store in the seed values
		//-----------------------------------------------------------------
		//instead of reassigning memory on each iteration, just reset.

		sigmagray.assign(numk, 0);
		sigmax.assign(numk, 0);
		sigmay.assign(numk, 0);
		clustersize.assign(numk, 0);
		//------------------------------------
		//edgesum.assign(numk, 0);
		//------------------------------------

		{int ind(0);
		for( int r = 0; r < height; r++ )
		{
			for( int c = 0; c < width; c++ )
			{
				sigmagray[labels_data[ind]] += gray_data[ind];
				sigmax[labels_data[ind]] += c;
				sigmay[labels_data[ind]] += r;
				//------------------------------------
				//edgesum[klabels[ind]] += edgemag[ind];
				//------------------------------------
				clustersize[labels_data[ind]] += 1.0;
				ind++;
			}
		}}

		{for( int k = 0; k < numk; k++ )
		{
			if( clustersize[k] <= 0 ) clustersize[k] = 1;
			inv[k] = 1.0f/clustersize[k];//computing inverse now to multiply, than divide later
		}}

		{for( int k = 0; k < numk; k++ )
		{
			kseedsgray[k] = sigmagray[k]*inv[k];
			kseedsx[k] = sigmax[k]*inv[k];
			kseedsy[k] = sigmay[k]*inv[k];
			//------------------------------------
			//edgesum[k] *= inv[k];
			//------------------------------------
		}}
	}

	//save seeds position
	m_superpixel_center = Matrix<float>(kseedsx.size(),2);
	for (int k = 0; k < numk; ++k)
	{
		m_superpixel_center(k,0) = kseedsx[k];
		m_superpixel_center(k,1) = kseedsy[k];
	}
}

template<class ImageSigT>
void SLICSuperPixelNode<ImageSigT>::setSuperPixelsNum(const int num)
{
	m_k_superpixles=num;

	//force time update
	modified();
}

template<class ImageSigT>
void SLICSuperPixelNode<ImageSigT>::getSuperPixelsNum(int& num)
{
	num=m_k_superpixles;
}

template<class ImageSigT>
int SLICSuperPixelNode<ImageSigT>::getSuperPixelsNum()
{
	return m_k_superpixles;
}

template<class ImageSigT>
void SLICSuperPixelNode<ImageSigT>::setSuperPixelSize(const int size)
{
	m_size_superpixel=size;

	//force time update
	modified();
}

template<class ImageSigT>
void SLICSuperPixelNode<ImageSigT>::getSuperPixelSize(int& size)
{
	size=m_size_superpixel;
}

template<class ImageSigT>
void SLICSuperPixelNode<ImageSigT>::setCompactness(const float compactness)
{
	m_compactness=compactness;

	//force time update
	modified();
}

template<class ImageSigT>
void SLICSuperPixelNode<ImageSigT>::getCompactness(float& compactness)
{
	compactness=m_compactness;
}

}