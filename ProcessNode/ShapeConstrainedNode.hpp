namespace eagleeye
{
template<class ImageSigT>
ShapeConstrainedNode<ImageSigT>::ShapeConstrainedNode()
{
	memset( m_w_h_ratio, 0, sizeof(float)*2 );
	memset( m_area, 0, sizeof(float) * 2 );
	memset( m_circle_radius, 0, sizeof(float)*2 );
	memset( m_region, 0, sizeof(int)*4 );

	m_con_flags = (_Conflags)0x0;
	m_connected_components_num = 0;

	//set input port number
	setNumberOfInputSignals(1);

	//set output port property
	setNumberOfOutputSignals(1);
	setOutputPort(makeOutputSignal(),OUTPUT_PORT_SHAPE_CONSTRAINED_IMAGE_DATA);
}

template<class ImageSigT>
ShapeConstrainedNode<ImageSigT>::~ShapeConstrainedNode()
{

}

template<class ImageSigT>
void ShapeConstrainedNode<ImageSigT>::executeNodeInfo()
{
	ImageSigT* input_immg_signal = getInputImageSignal(INPUT_PORT_IMAGE_DATA);
	if (!input_immg_signal)
	{
		EAGLEEYE_ERROR("sorry, couldn't transform to predefined input image signal \n");
		return;
	}
	Matrix<PixelType> input_img = input_immg_signal->img;
	int rows = input_img.rows();
	int cols = input_img.cols();
	
	Matrix<unsigned char> binary_img = input_img.transform<unsigned char>();
	for (int i = 0; i < rows; ++i)
	{
		unsigned char* binary_img_data = binary_img.row(i);
		for (int j = 0; j < cols; ++j)
		{
			if (binary_img_data[j] > unsigned char(0))
			{
				binary_img_data[j] = 1;
			}
			else
			{
				binary_img_data[j] = 0;
			}
		}
	}

	bwlabel( binary_img, m_connected_components_label, m_connected_components_num, 8 );

	//build connected components table
	for (int i = 0; i < rows; ++i)
	{
		int* components_label_data = m_connected_components_label.row(i);
		for (int j = 0; j < cols; ++j)
		{
			m_connected_components_table[ components_label_data[ j ] ].push_back( i * cols + j );
		}
	}

	//implement constrained operation
	constrainedOperation( binary_img );

	Matrix<PixelType> output_img = binary_img.transform<PixelType>();
	
	ImageSigT* output_img_sig = getOutputImageSignal(OUTPUT_PORT_SHAPE_CONSTRAINED_IMAGE_DATA);
	if ( !output_img_sig )
	{
		EAGLEEYE_ERROR("sorry, couldn't transform to predefined output image signal...\n ");
		return;
	}

	output_img_sig->img = output_img;
}

template<class ImageSigT>
bool ShapeConstrainedNode<ImageSigT>::selfcheck()
{
	Superclass::selfcheck();

	if (!getInputImageSignal())
	{
		EAGLEEYE_ERROR("sorry, image pixel isn't consistent ...\n please be careful... \n");
		return false;
	}

	return true;
}

template<class ImageSigT>
void ShapeConstrainedNode<ImageSigT>::constrainedOperation(Matrix<unsigned char>& img)
{
	float min_ratio = m_w_h_ratio[ 0 ];
	float max_ratio = m_w_h_ratio[ 1 ];

	float min_area = m_area[ 0 ];
	float max_area = m_area[ 1 ];

	float min_circle_radius = m_circle_radius[ 0 ];
	float max_circle_radius = m_circle_radius[ 1 ];

	int rows = img.rows();
	int cols = img.cols();

	//find bounding rectangle
	unsigned char* img_data = img.dataptr();

	for ( int component_index = 1; component_index < m_connected_components_num; ++component_index )
	{
		std::vector<int> pos_vec = m_connected_components_table[ component_index ];

		int left_bottom_r = rows;
		int left_bottom_c = cols;

		int right_top_r = 0;
		int right_top_c = 0;

		// judge whether the component is empty
		if ( pos_vec.size() > 0 )
		{
			//first step
			// process constrained area
			if ( m_con_flags & area_constrained )
			{
				int area = int( pos_vec.size() );
				if ( area < min_area || area > max_area )
				{
					std::vector<int>::iterator iter,iend( pos_vec.end() );
					for (iter = pos_vec.begin(); iter != iend; ++iter)
					{
						img_data[ (*iter) ] = 0;
					}

					m_connected_components_table[ component_index ].clear();
					continue;
				}
			}

			std::vector<int>::iterator iter,iend( pos_vec.end() );
			for ( iter = pos_vec.begin(); iter < iend; ++iter )
			{
				int pos = (*iter);
				int r_index = pos / cols;
				int c_index = pos - r_index * cols;

				if (left_bottom_r>r_index)
				{
					left_bottom_r=r_index;
				}
				if (left_bottom_c>c_index)
				{
					left_bottom_c=c_index;
				}

				if (right_top_r<r_index)
				{
					right_top_r=r_index;
				}
				if (right_top_c<c_index)
				{
					right_top_c=c_index;
				}
			}

			//second step
			//process constrained region
			if ( m_con_flags & region_constrained )
			{
				int constrained_bottom_r = m_region[0];
				int constrained_top_r = m_region[1];
				int constrained_left_c = m_region[2];
				int constrained_right_c = m_region[3];

				if ( (right_top_r < constrained_bottom_r) ||
					(left_bottom_r > constrained_top_r)||
					(right_top_c < constrained_left_c)||
					(left_bottom_c > constrained_right_c))
				{
					std::vector<int>::iterator iter,iend( pos_vec.end() );
					for (iter = pos_vec.begin(); iter != iend; ++iter)
					{
						img_data[ (*iter) ] = 0;
					}

					m_connected_components_table[ component_index ].clear();
					continue;
				}
			}

			Matrix<unsigned char> component_mat(right_top_r-left_bottom_r+1,right_top_c-left_bottom_c+1,unsigned char(0));
			for ( iter = pos_vec.begin(); iter < iend; ++iter )
			{
				int pos = (*iter);
				int temp = pos / cols;

				int r_index = temp - left_bottom_r;
				int c_index = pos - temp * cols - left_bottom_c;

				component_mat(r_index,c_index)=1;
			}

			cv::Mat temp_mat(right_top_r-left_bottom_r+1,right_top_c-left_bottom_c+1,CV_8U,component_mat.dataptr());
			IplImage temp_img=temp_mat;

			CvMemStorage* storage=cvCreateMemStorage();
			CvSeq* seq=NULL;
			int cnt=cvFindContours(&temp_img,storage,&seq,sizeof(CvContour),CV_RETR_EXTERNAL);

			if (seq)
			{
				//third step
				//process constrained wh ratio
				if ( m_con_flags & w_h_ratio_constrained )
				{
					CvRect rect = cvBoundingRect( seq , 1 );
					float ratio = float( rect.width )/float( rect.height );

					if ( ratio < min_ratio || ratio > max_ratio )
					{
						std::vector<int>::iterator iter,iend( pos_vec.end() );
						for ( iter = pos_vec.begin(); iter != iend; ++iter )
						{
							img_data[ (*iter) ] = 0;
						}

						m_connected_components_table[ component_index ].clear();
						continue;
					}
				}

				//fourth step
				//process constrained circle radius
				if ( m_con_flags & circle_constrained )
				{
					CvPoint2D32f circle_center;
					float circle_radius;

					cvMinEnclosingCircle( seq,&circle_center,&circle_radius );

					if ( circle_radius < min_circle_radius || circle_radius > max_circle_radius )
					{
						std::vector<int>::iterator iter,iend( pos_vec.end() );
						for ( iter = pos_vec.begin(); iter != iend; ++iter )
						{
							img_data[ (*iter) ] = 0;
						}

						m_connected_components_table[ component_index ].clear();

						continue;
					}
				}
			}
			else
			{
				std::vector<int>::iterator iter,iend( pos_vec.end() );
				for ( iter = pos_vec.begin(); iter != iend; ++iter )
				{
					img_data[ (*iter) ] = 0;
				}

				m_connected_components_table[ component_index ].clear();
			}

			cvClearMemStorage(storage);
		}
	}
}

template<class ImageSigT>
void ShapeConstrainedNode<ImageSigT>::setConstrainedFlags(typename ShapeConstrainedNode<ImageSigT>::_Conflags con_flags)
{
	m_con_flags=con_flags;
}

template<class ImageSigT>
void ShapeConstrainedNode<ImageSigT>::clearSomething()
{
	m_connected_components_label=Matrix<int>();
}

template<class ImageSigT>
void ShapeConstrainedNode<ImageSigT>::printUnit()
{
	Superclass::printUnit();

	EAGLEEYE_INFO("w/h ratio min - %f , max - %f \n",m_w_h_ratio[ 0 ],m_w_h_ratio[ 1 ]);
	EAGLEEYE_INFO("area min - %f , max - %f \n",m_area[ 0 ],m_area[ 1 ]);
	EAGLEEYE_INFO("enclosing circle radius min - %f , max - %f \n",m_circle_radius[ 0 ], m_circle_radius[ 1 ]);
}

template<class ImageSigT>
void ShapeConstrainedNode<ImageSigT>::setConstrainedArea(float min_area,float max_area)
{
	m_area[0]=min_area;
	m_area[1]=max_area;
}

template<class ImageSigT>
void ShapeConstrainedNode<ImageSigT>::setConstrainedWHRatio(float min_ratio,float max_ratio)
{
	m_w_h_ratio[0]=min_ratio;
	m_w_h_ratio[1]=max_ratio;
}

template<class ImageSigT>
void ShapeConstrainedNode<ImageSigT>::setConstrainedCircleRadius(float min_radius,float max_radius)
{
	m_circle_radius[0]=min_radius;
	m_circle_radius[1]=max_radius;
}

template<class ImageSigT>
void ShapeConstrainedNode<ImageSigT>::setConstrainedRegion(int bottom_r,int top_r,int left_c, int right_c)
{
	m_region[0] = bottom_r;
	m_region[1] = top_r;
	m_region[2] = left_c;
	m_region[3] = right_c;
}

}