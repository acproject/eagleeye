namespace eagleeye
{
template<typename T>
Matrix<Complex<double>> fft2D(Matrix<T> m,unsigned int row_r,unsigned int col_r)
{
	unsigned int requested_row=unsigned int(pow(float(2),int(row_r)));
	unsigned int requested_col=unsigned int(pow(float(2),int(col_r)));
	Matrix<Complex<double>> frequency_data(requested_row,requested_col);
	frequency_data.setzeros();
	Complex<double>* frequency_data_ptr=frequency_data.dataptr();

	Matrix<Complex<double>> space_data(requested_row,requested_col);
	space_data.setzeros();
	Complex<double>* space_data_ptr=space_data.dataptr();

	
	//write data of m to space_data
	unsigned int min_row=EAGLEEYE_MIN(requested_row,m.rows());
	unsigned int min_col=EAGLEEYE_MIN(requested_col,m.cols());	
	
	for (unsigned int i=0;i<min_row;++i)
	{
		T* r_ptr=m.row(i);
		Complex<double>* s_r_ptr=space_data_ptr+(i*requested_col);

		for (unsigned int j=0;j<min_col;++j)
		{
			s_r_ptr[j]=r_ptr[j];
		}
	}
	
	//implement fft along the y direction(row)
	for (unsigned int i=0;i<requested_row;++i)
	{
		fft1D(space_data_ptr+i*requested_col,frequency_data_ptr+i*requested_col,col_r);
	}

	for (unsigned int i=0;i<requested_row;++i)
	{
		for (unsigned int j=0;j<requested_col;++j)
		{
			space_data_ptr[i+requested_row*j]=frequency_data_ptr[j+requested_col*i];
		}
	}

	//implement fft along the x direction(col)
	for (unsigned int i=0;i<requested_col;++i)
	{
		fft1D(space_data_ptr+i*requested_row,frequency_data_ptr+i*requested_row,row_r);
	}
	
	Matrix<Complex<double>> result(requested_row,requested_col);
	for (unsigned int i=0;i<requested_row;++i)
	{
		Complex<double>* result_data_ptr=result.row(i);
		
		for (unsigned int j=0;j<requested_col;++j)
		{
			result_data_ptr[j]=frequency_data_ptr[j*requested_row+i];
		}
	}

	return result;
}

template<typename T>
Matrix<float> matchTemplate(const Matrix<T> match_target,const Matrix<T> match_template)
{
	unsigned int target_rows=match_target.rows();
	unsigned int target_cols=match_target.cols();

	unsigned int template_rows=match_template.rows();
	unsigned int template_cols=match_template.cols();

	//compute R1 term
	unsigned int c=1;
	unsigned int r=1;
	unsigned int target_cp=0;
	unsigned int target_rp=0;
	while (c*2<=target_cols)
	{
		c=c*2;
		target_cp++;
	}
	c=c*2;
	target_cp++;

	while(r*2<=target_rows)
	{
		r=r*2;
		target_rp++;
	}
	r=r*2;
	target_rp++;

	Matrix<float> mean_match_template_matrix(template_rows,template_cols);
	
	//remove the mean value from match_template image
	float mean_match_template=mean(match_template);
	for (unsigned int i=0;i<template_rows;++i)
	{
		const T* data_ptr=match_template.row(i);
		float* mean_data_ptr=mean_match_template_matrix.row(i);

		for (unsigned int j=0;j<template_cols;++j)
		{
			mean_data_ptr[j]=data_ptr[j]-mean_match_template;
		}
	}


	Matrix<Complex<double>> match_target_frequency=fft2D(match_target,target_rp,target_cp);
	Matrix<Complex<double>> match_template_frequency=fft2D(mean_match_template_matrix,target_rp,target_cp);
	
	Matrix<Complex<double>> match_template_frequency_conjugate(r,c);
	//compute the conjugate matrix of match_template_frequency
	for (unsigned int i=0;i<r;++i)
	{
		Complex<double>* data_ptr=match_template_frequency.row(i);
		Complex<double>* conjugate_data_ptr=match_template_frequency_conjugate.row(i);

		for (unsigned int j=0;j<c;++j)
		{
			conjugate_data_ptr[j]=data_ptr[j].conjugate();
		}	
	}


	Matrix<Complex<double>> mediate_result=match_target_frequency.dot(match_template_frequency_conjugate);
	Matrix<Complex<double>> result=ifft2D(mediate_result,target_rp,target_cp);
	
	Matrix<float> R1(target_rows,target_cols);
	for (unsigned int i=0;i<target_rows;++i)
	{
		float* R1_data_ptr=R1.row(i);
		Complex<double>* data_ptr=result.row(i);

		for (unsigned int j=0;j<target_cols;++j)
		{
			R1_data_ptr[j]=data_ptr[j].magnitude();
		}
	}


	Matrix<float> sumintegral(target_rows,target_cols);
	Matrix<float> sqsumintegral(target_rows,target_cols);
	integralImage(match_target,sumintegral,sqsumintegral);

	//compute R2 term
	Matrix<float> R2(target_rows,target_cols);
	for (unsigned int i=0;i<target_rows;++i)
	{
		float* R2_data_ptr=R2.row(i);
		for (unsigned int j=0;j<target_cols;++j)
		{
			unsigned int extend_r=EAGLEEYE_MIN((i+template_rows),(target_rows-1));
			unsigned int extend_c=EAGLEEYE_MIN((j+template_cols),(target_cols-1));
			
			R2_data_ptr[j]=sumintegral.at(extend_r,extend_c)-
				sumintegral.at(i,extend_c)-
				sumintegral.at(extend_r,j)+
				sumintegral.at(i,j);
			
		}
	}

	//compute R3 term
	Matrix<float> R3(target_rows,target_cols);
	for (unsigned int i=0;i<target_rows;++i)
	{
		float* R3_data_ptr=R3.row(i);
		for (unsigned int j=0;j<target_cols;++j)
		{
			unsigned int extend_r=EAGLEEYE_MIN((i+template_rows),(target_rows-1));
			unsigned int extend_c=EAGLEEYE_MIN((j+template_cols),(target_cols-1));

			R3_data_ptr[j]=sqsumintegral.at(extend_r,extend_c)-
				sqsumintegral.at(i,extend_c)-
				sqsumintegral.at(extend_r,j)+
				sqsumintegral.at(i,j);
		}
	}

	//compute relevant coefficient
	float template_mean_value=mean(match_template);

	float template_sqsum=0;
	for (unsigned int i=0;i<template_rows;++i)
	{
		const T* template_data_ptr=match_template.row(i);			
		for (unsigned int j=0;j<template_cols;++j)
		{
			template_sqsum+=(template_data_ptr[j]-template_mean_value)*(template_data_ptr[j]-template_mean_value);
		}
	}
	float term3=sqrt(template_sqsum);

	float term1,term2;

	Matrix<float> relevant_coefficient(target_rows,target_cols);

	if (term3==0)
	{
		//it illustrates that every pixel in the match template image is equal
		for (unsigned int i=0;i<target_rows;++i)
		{
			float* relevant_coe_data_ptr=relevant_coefficient.row(i);
			float* R2_data_ptr=R2.row(i);
			float* R3_data_ptr=R3.row(i);

			for (unsigned int j=0;j<target_cols;++j)
			{
				term2=sqrt(R3_data_ptr[j]-R2_data_ptr[j]*R2_data_ptr[j]/(template_rows*template_cols));

				if (term2==0)
				{
					relevant_coe_data_ptr[j]=1;
				}
				else
				{
					relevant_coe_data_ptr[j]=0;
				}
			}
			
		}
	}
	else
	{
		for (unsigned int i=0;i<target_rows;++i)
		{
			float* relevant_coe_data_ptr=relevant_coefficient.row(i);

			float* R1_data_ptr=R1.row(i);
			float* R2_data_ptr=R2.row(i);
			float* R3_data_ptr=R3.row(i);

			for (unsigned int j=0;j<target_cols;++j)
			{
				term1=R1_data_ptr[j];

				term2=sqrt(R3_data_ptr[j]-R2_data_ptr[j]*R2_data_ptr[j]/(template_rows*template_cols));

				if (term2!=0)
				{
					relevant_coe_data_ptr[j]=term1/(term2*term3);
				}
				else
				{
					if (term1==0)
					{
						relevant_coe_data_ptr[j]=1;
					}
					else
					{
						relevant_coe_data_ptr[j]=0;
					}

				}
			}
		}
	}

	return relevant_coefficient;

}

template<class T>
Matrix<float> matchTemplateSQDIFF(const Matrix<T> match_target,const Matrix<T> match_template)
{
	unsigned int target_rows=match_target.rows();
	unsigned int target_cols=match_target.cols();

	unsigned int template_rows=match_template.rows();
	unsigned int template_cols=match_template.cols();

	Matrix<float>term1=conv2DInSpace(match_target,match_template);

	//compute term2
	//sum(f^2(x,y))
	Matrix<float> sumintegral(target_rows,target_cols);
	Matrix<float> sqsumintegral(target_rows,target_cols);
	integralImage(match_target,sumintegral,sqsumintegral);
	Matrix<float> term2(target_rows,target_cols);
	for (unsigned int i=0;i<target_rows;++i)
	{
		float* term2_data_ptr=term2.row(i);
		for (unsigned int j=0;j<target_cols;++j)
		{
			unsigned int extend_r=EAGLEEYE_MIN((i+template_rows),(target_rows-1));
			unsigned int extend_c=EAGLEEYE_MIN((j+template_cols),(target_cols-1));

			term2_data_ptr[j]=sqsumintegral.at(extend_r,extend_c)-
				sqsumintegral.at(i,extend_c)-
				sqsumintegral.at(extend_r,j)+
				sqsumintegral.at(i,j);
		}
	}

	//compute term3 
	//sum(t^2(x-u,y-v))
	float term3=0;
	for (unsigned int i=0;i<template_rows;++i)
	{
		const T* template_data_ptr=match_template.row(i);			
		for (unsigned int j=0;j<template_cols;++j)
		{
			term3+=template_data_ptr[j]*template_data_ptr[j];
		}
	}

	Matrix<float> relevant_coefficient(target_rows,target_cols);
	for (unsigned int i=0;i<target_rows;++i)
	{
		float* relevant_data_ptr=relevant_coefficient.row(i);
		float* term1_data_ptr=term1.row(i);
		float* term2_data_ptr=term2.row(i);

		for (unsigned int j=0;j<target_cols;++j)
		{
			relevant_data_ptr[j]=term2_data_ptr[j]-2*term1_data_ptr[j]+term3;
		}
	}

	return relevant_coefficient;

}

template<typename T>
Matrix<float> matchTemplateSQDIFF(const Matrix<T> match_target,
	const Matrix<T> match_template,
	unsigned int row_interval,
	unsigned int col_interval)
{
	unsigned int target_rows=match_target.rows();
	unsigned int target_cols=match_target.cols();

	unsigned int template_rows=match_template.rows();
	unsigned int template_cols=match_template.cols();

	unsigned int sample_row_rows=target_rows/row_interval;
	unsigned int sample_col_cols=target_cols/col_interval;

	//compute term2 at predefined position
	//sum(f^2(x,y))
	Matrix<float> term2(sample_row_rows,sample_col_cols);

	for (unsigned int i=0;i<sample_row_rows;++i)
	{
		float* term2_data_ptr=term2.row(i);

		for (unsigned int j=0;j<sample_col_cols;++j)
		{
			float sum_value=0;

			unsigned int extern_r=EAGLEEYE_MIN(i*row_interval+template_rows,target_rows);
			unsigned int extern_c=EAGLEEYE_MIN(j*col_interval+template_cols,target_cols);

			for (unsigned int sub_i=i*row_interval;sub_i<extern_r;++sub_i)
			{
				const T* target_data_ptr=match_target.row(sub_i);

				for (unsigned int sub_j=j*col_interval;sub_j<extern_c;++sub_j)
				{
					sum_value+=target_data_ptr[sub_j]*target_data_ptr[sub_j];
				}
			}

			term2_data_ptr[j]=sum_value;
		}
	}

	//convolution at predefined position
	Matrix<float> term1(sample_row_rows,sample_col_cols);

	for (unsigned int i=0;i<sample_row_rows;++i)
	{
		float* term1_data_ptr=term1.row(i);
		for (unsigned int j=0;j<sample_col_cols;++j)
		{
			float sum_value=0;

			unsigned int extern_r=EAGLEEYE_MIN(i*row_interval+template_rows,target_rows);
			unsigned int extern_c=EAGLEEYE_MIN(j*col_interval+template_cols,target_cols);

			for (unsigned int sub_i=i*row_interval;sub_i<extern_r;++sub_i)
			{
				const T* target_data_ptr=match_target.row(sub_i);
				const T* template_data_ptr=match_template.row(sub_i-i*row_interval);

				for (unsigned int sub_j=j*col_interval;sub_j<extern_c;++sub_j)
				{
					sum_value+=target_data_ptr[sub_j]*template_data_ptr[sub_j-j*col_interval];
				}
			}

			term1_data_ptr[j]=sum_value;
		}
	}


	//compute term3 
	//sum(t^2(x-u,y-v))
	float term3=0;
	for (unsigned int i=0;i<template_rows;++i)
	{
		const T* template_data_ptr=match_template.row(i);			
		for (unsigned int j=0;j<template_cols;++j)
		{
			term3+=template_data_ptr[j]*template_data_ptr[j];
		}
	}


	Matrix<float> relevant_coefficient(sample_row_rows,sample_col_cols);
	for (unsigned int i=0;i<sample_row_rows;++i)
	{
		float* relevant_data_ptr=relevant_coefficient.row(i);
		float* term1_data_ptr=term1.row(i);
		float* term2_data_ptr=term2.row(i);

		for (unsigned int j=0;j<sample_col_cols;++j)
		{
			relevant_data_ptr[j]=term2_data_ptr[j]-2*term1_data_ptr[j]+term3;
		}
	}

	return relevant_coefficient;
}

template<class CompareT>
std::vector<unsigned int> sort(const Matrix<typename CompareT::ElementType>& data)
{
	typedef typename CompareT::ElementType		ElementType;

	assert(data.rows() == 1 || data.cols() == 1);

	unsigned int r_num = data.rows();
	unsigned int c_num = data.cols();

	std::vector<SortElement<ElementType>> temp_vector(r_num * c_num);
	unsigned int index = 0;
	for (unsigned int i = 0; i < r_num; ++i)
	{
		const ElementType* d_ptr = data.row(i);
		for (unsigned int j = 0; j < c_num; ++j)
		{
			temp_vector[index] = SortElement<ElementType>(d_ptr[j],index);
			index++;
		}
	}

	//sort
	CompareT compare_op;
	std::sort(temp_vector.begin(),temp_vector.end(),compare_op);

	std::vector<unsigned int> result_index(r_num * c_num);
	index = 0;
	std::vector<SortElement<ElementType>>::iterator iter,iend(temp_vector.end());
	for (iter = temp_vector.begin(); iter != iend; ++iter)
	{
		result_index[index] = ((*iter).index);
		index++;
	}

	return result_index;
}

template<typename InterpolationType,typename T>
Matrix<T> upSample(const Matrix<T> src,unsigned int row_r,unsigned int col_r)
{
	unsigned int row_interval=1<<row_r;
	unsigned int col_interval=1<<col_r;

	float row_delta=1.0f/row_interval;
	float col_delta=1.0f/col_interval;

	unsigned int rows=src.rows();
	unsigned int cols=src.cols();

	unsigned int up_rows=row_interval*rows;
	unsigned int up_cols=col_interval*cols;

	Matrix<T> up_matrix(up_rows,up_cols);

	InterpolationType interpolate(src);		

	for (unsigned int i=0;i<up_rows;++i)
	{
		T* up_ptr=up_matrix.row(i);

		for (unsigned int j=0;j<up_cols;++j)
		{
			up_ptr[j]=interpolate(i*row_delta,j*col_delta);
		}
	}

	return up_matrix;
}

template<typename T>
Matrix<T> downSample(const Matrix<T> src,unsigned int row_interval,unsigned int col_interval)
{
	unsigned int row_n = src.rows();
	unsigned int col_n = src.cols();

	unsigned int down_row_n = row_n / row_interval;
	unsigned int down_col_n = col_n / col_interval;

	Matrix<T> down_image(down_row_n,down_col_n);

	for (unsigned int i = 0; i < down_row_n; ++i)
	{
		//the row pointer of src
		const T* src_ptr = src.row(i * row_interval);
		//the row pointer of down_image
		T* down_ptr = down_image.row(i);

		for (unsigned int j = 0; j < down_col_n; ++j)
		{
			down_ptr[j] = src_ptr[j * col_interval];
		}
	}

	return down_image;
}

template<typename T>
Matrix<T> upInterpolate(const Matrix<T> src, unsigned int row_interval /* = 1 */, unsigned int col_interval /* = 1 */)
{
	unsigned int rows = src.rows();
	unsigned int cols = src.cols();

	unsigned int up_rows = rows * row_interval;
	unsigned int up_cols = cols * col_interval;

	Matrix<T> up_image(up_rows,up_cols);
	for (unsigned int i = 0; i < up_rows; ++i)
	{
		//the row pointer of src
		const T* src_ptr = src.row(i / row_interval);
		//the row pointer of up_image
		T* up_ptr = up_image.row(i);

		for (unsigned int j = 0; j < up_cols; ++j)
		{
			up_ptr[j] = src_ptr[j / col_interval];
		}
	}

	return up_image;
}

template<typename T>
std::vector<VecF> computeDirectionDescription(const Matrix<T> image,
	std::vector<PointPos> pos_set,
	unsigned int region_width,unsigned int region_height)
{
	Matrix<Gradient> gradient_image;
	computeGradient(image,gradient_image);

	unsigned int row_num=image.rows();
	unsigned int col_num=image.cols();

	//This matrix store the angle(0,2pi)
	Matrix<float> gradinet_angle_horizontal_image(row_num,col_num);
	
	//This matrix store the weight
	Matrix<float> gradient_weight_image(row_num,col_num);

	//compute gradient angle along the horizontal direction on every 
	// position of the gradient image
	for (unsigned int i=0;i<row_num;++i)
	{
		Gradient* gradient_ptr=gradient_image.row(i);

		float* gradient_angle_ptr=gradinet_angle_horizontal_image.row(i);

		float* gradient_weight_ptr=gradient_weight_image.row(i);
		
		for (unsigned int j=0;j<col_num;++j)
		{
			//compute angle 

			//the first quadrant
			if (gradient_ptr[j].x>=0&&gradient_ptr[j].y>=0)
			{
				gradient_angle_ptr[j]=atan(gradient_ptr[j].y/(gradient_ptr[j].x+eagleeye_eps));
			}

			//the second quadrant
			if (gradient_ptr[j].x<=0&&gradient_ptr[j].y>0)
			{
				gradient_angle_ptr[j]=atan(gradient_ptr[j].y/(gradient_ptr[j].x+eagleeye_eps))+EAGLEEYE_PI;
			}

			//the third quadrant
			if(gradient_ptr[j].x<=0&&gradient_ptr[j].y<0)
			{
				gradient_angle_ptr[j]=atan(gradient_ptr[j].y/(gradient_ptr[j].x+eagleeye_eps))+EAGLEEYE_PI;
			}

			//the fourth quadrant
			if (gradient_ptr[j].x>=0&&gradient_ptr[j].y<0)
			{
				gradient_angle_ptr[j]=atan(gradient_ptr[j].y/(gradient_ptr[j].x+eagleeye_eps))+2.0f*EAGLEEYE_PI;
			}
			
			//compute weight
			gradient_weight_ptr[j]=gradient_ptr[j].magnitude;
		}
	}
	
	std::vector<VecF> feature_set;
	
	unsigned int half_width=region_width/2;
	unsigned int half_height=region_height/2;
	region_width=half_width*2;
	region_height=half_height*2;

	std::vector<PointPos>::iterator iter,iend(pos_set.end());
	for (iter=pos_set.begin();iter!+iend;++iter)
	{
		PointPos current_pos=(*iter);
		//Using the nearest point
		unsigned int row_index=unsigned int(current_pos.y);
		unsigned int col_index=unsigned int(current_pos.x);
		
		//Find the neighbor region (region_width*region_height)around the current point
		unsigned int neighbor_row_start_index=row_index-half_width;
		neighbor_row_start_index=EAGLEEYE_MAX(neighbor_row_start_index,0);
		neighbor_row_start_index=EAGLEEYE_MIN(neighbor_col_start_index,row_num);

		unsigned int neighbor_row_end_index=row_index+half_width;
		neighbor_row_end_index=EAGLEEYE_MAX(neighbor_row_end_index,0);
		neighbor_row_end_index=EAGLEEYE_MIN(neighbor_row_end_index,row_num);

		unsigned int neighbor_col_start_index=col_index-half_height;
		neighbor_col_start_index=EAGLEEYE_MAX(neighbor_col_start_index,0);
		neighbor_col_start_index=EAGLEEYE_MIN(neighbor_col_start_index,col_num);

		unsigned int neighbor_col_end_index=col_num+half_height;
		neighbor_col_end_index=EAGLEEYE_MAX(neighbor_col_end_index,0);
		neighbor_col_end_index=EAGLEEYE_MIN(neighbor_col_end_index,col_num);
		
		
		//Get four subregions
		//left top
		Matrix<float> sub_left_top=
			gradinet_angle_horizontal_image(Range(neighbor_row_start_index,row_index),
			Range(neighbor_col_start_index,col_index));

		Matrix<float> sub_left_top_weight=
			gradient_weight_image(Range(neighbor_row_start_index,row_index),
			Range(neighbor_col_start_index,col_index));

		int left_top_histogram[8];
		computeHistogram<float,8>(sub_left_top,gradient_weight_image,0,6.28,left_top_histogram);

		//right top
		Matrix<float> sub_right_top=
			gradinet_angle_horizontal_image(Range(neighbor_row_start_index,row_index),
			Range(col_index,neighbor_col_end_index));
		
		Matrix<float> sub_right_top_weight=
			gradient_weight_image(Range(neighbor_row_start_index,row_index),
			Range(col_index,neighbor_col_end_index));

		int right_top_histogram[8];
		computeHistogram<float,8>(sub_right_top,sub_right_top_weight,0,6.28,right_top_histogram);

		//left bottom
		Matrix<float> sub_left_bottom=
			gradinet_angle_horizontal_image(Range(row_index,neighbor_row_end_index),
			Range(neighbor_col_start_index,col_index));

		Matrix<float> sub_left_bottom_weight=
			gradient_weight_image(Range(row_index,neighbor_row_end_index),
			Range(neighbor_col_start_index,col_index));

		int left_bottom_histogram[8];
		computeHistogram<float,8>(sub_left_bottom,sub_left_bottom_weight,0,6.28,left_bottom_histogram);

		//right bottom
		Matrix<float> sub_right_bottom=
			gradinet_angle_horizontal_image(Range(row_index,neighbor_row_end_index),
			Range(col_index,neighbor_col_end_index));

		Matrix<float> sub_right_bottom_weight=
			gradient_weight_image(Range(row_index,neighbor_row_end_index),
			Range(col_index,neighbor_col_end_index));

		int right_bottom_histogram[8];
		computeHistogram<float,8>(sub_right_bottom,0,6.28,right_bottom_histogram);


		FeatureDescriptionType feature(1,32);
		//left top
		for (unsigned int i=0;i<8;++i)
		{
			feature.at(i)=left_top_histogram[i];
		}

		//right top
		for (unsigned int i=0;i<8;++i)
		{
			feature.at(i+8)=right_top_histogram[i];
		}

		//left bottom
		for (unsigned int i=0;i<8;++i)
		{
			feature.at(i+16)=left_bottom_histogram[i];
		}

		//right bottom
		for (unsigned int i=0;i<8;++i)
		{
			feature.at(i+24)=right_bottom_histogram[i];
		}

		feature_set.push_back(feature);
	}

	return feature_set;
}

template<typename T>
Matrix<Gradient> computeGradient(const Matrix<T> src_image)
{
	unsigned int src_rows = src_image.rows();
	unsigned int src_cols = src_image.cols();

	Matrix<Gradient> gradient_image = Matrix<Gradient>(src_rows,src_cols);
	
	for (unsigned int i = 1; i < src_rows; ++i)
	{
		const T* src_up_ptr = src_image.row(i - 1);
		const T* src_ptr = src_image.row(i);

		Gradient* gradient_ptr = gradient_image.row(i);

		for (unsigned int j = 1; j < src_cols; ++j)
		{
			float horizontal_gradient = float(src_ptr[j] - src_ptr[j - 1]);
			float vertial_gradient = float(src_ptr[j] - src_up_ptr[j]);
			
			gradient_ptr[j].x = horizontal_gradient;
			gradient_ptr[j].y = vertial_gradient;
			gradient_ptr[j].magnitude = sqrt(horizontal_gradient * horizontal_gradient + 
				vertial_gradient*vertial_gradient);
		}
	}

	//fill in the first row data by the second row data
	Gradient* first_row_data = gradient_image.row(0);
	Gradient* second_row_data = gradient_image.row(1);
	memcpy(first_row_data,second_row_data,sizeof(Gradient) * src_cols);

	//fill in the first col data by the second col data
	for (unsigned int i = 0; i < src_rows; ++i)
	{
		Gradient* gradient_ptr = gradient_image.row(i);
		gradient_ptr[0] = gradient_ptr[1];
	}

	return gradient_image;
}

template<typename T>
Matrix<float> computeGradientMag(const Matrix<T>& img)
{
	int rows = img.rows();
	int cols = img.cols();	

	Matrix<float> gradient_map(rows,cols,0.0f);

	float dx2,dy2;
	float left_x_val,right_x_val;
	float up_y_val,down_y_val;
	for (int i = 1; i < rows - 1; ++i)
	{
		float* gradient_map_data = gradient_map.row(i);
		for (int j = 1; j < cols - 1; ++j)
		{
			dx2 = 0; dy2 = 0;
			for (int p = 0; p < AtomicTypeTrait<T>::size; ++p)
			{
				float val = float(OperateTrait<T>::unit(img(i,j),p));

				left_x_val = float(OperateTrait<T>::unit(img(i,j - 1),p));
				right_x_val = float(OperateTrait<T>::unit(img(i,j + 1),p));
				dx2 += (left_x_val + right_x_val - 2.0f * val) * (left_x_val + right_x_val - 2.0f * val) * 0.25f;

				up_y_val = float(OperateTrait<T>::unit(img(i - 1,j),p));
				down_y_val = float(OperateTrait<T>::unit(img(i + 1,j),p));
				dy2 += (up_y_val + down_y_val - 2.0f * val) * (up_y_val + down_y_val - 2.0f * val) * 0.25f;
			}

			gradient_map_data[j] = float(sqrt(dx2 + dy2));
		}
	}

	return gradient_map;
}

template<typename T,unsigned int BINS_NUM>
void computeHistogram(const Matrix<T>& image, float minvalue,float maxvalue,int histogram[BINS_NUM])
{
	memset(histogram,0,sizeof(int) * BINS_NUM);

	unsigned int row = image.rows();
	unsigned int col = image.cols();

	int n_bins = BINS_NUM;

	float singlestep = (maxvalue - minvalue) / n_bins;

	for (unsigned int i = 0; i < row; ++i)
	{
		const T* row_ptr = image.row(i);
		int index;
		for (unsigned int j = 0; j < col; ++j)
		{
			index = int(floor((float(row_ptr[j]) - minvalue) / singlestep - eagleeye_eps));
			if (index >= n_bins)
			{
				index = n_bins - 1;
			}
			if (index < 0)
			{
				index = 0;
			}

			histogram[index]++;
		}
	}
}

template<typename T,unsigned int BINS_NUM>
void computeHistogram(const Matrix<T>& image,const Matrix<float>& weight,
	float minvalue,float maxvalue,float histogram[BINS_NUM])
{
	memset(histogram,0,sizeof(float) * BINS_NUM);

	unsigned int row = image.rows();
	unsigned int col = image.cols();

	int n_bins = BINS_NUM;

	float singlestep = (maxvalue - minvalue) / n_bins;

	for (unsigned int i = 0; i < row; ++i)
	{
		const T* row_ptr = image.row(i);
		const float* weight_ptr = weight.row(i);

		int index;
		for (unsigned int j = 0; j < col; ++j)
		{
			index = int(floor((float(row_ptr[j]) - minvalue) / singlestep - eagleeye_eps));
			if (index >= n_bins)
			{
				index = n_bins - 1;
			}
			if (index < 0)
			{
				index = 0;
			}

			histogram[index] += weight_ptr[j];
		}
	}
}

template<typename T>
std::vector<float> computeInfoQuantity(const Matrix<T> image,std::vector<PointPos> pos_set,
	unsigned int region_width,unsigned int region_height)
{
	unsigned int sampling_num = pos_set.size();
	unsigned int row_num = image.rows();
	unsigned int col_num = image.cols();
	
	Matrix<Gradient> gradient_image;
	computeGradient(image,gradient_image);

	//This matrix store the angle(0,2pi)
	Matrix<float> gradinet_angle_horizontal_image(row_num,col_num);

	//This matrix store the weight
	Matrix<float> gradient_weight_image(row_num,col_num);

	//compute mean magnitude
	float mean_gradient_magnitude = 0;
	float magnitude_threshold;
	for (unsigned int i = 0; i < row_num; ++i)
	{
		Gradient* gradient_ptr = gradient_image.row(i);
		for (unsigned int j = 0; j < col_num; ++j)
		{
			mean_gradient_magnitude += gradient_ptr[j].magnitude;
		}
	}

	mean_gradient_magnitude /= (row_num * col_num);
	magnitude_threshold = mean_gradient_magnitude * 0.5f;

	
	//compute gradient angle along the horizontal direction on every 
	// position of the gradient image
	for (unsigned int i = 0; i < row_num; ++i)
	{
		Gradient* gradient_ptr = gradient_image.row(i);

		float* gradient_angle_ptr = gradinet_angle_horizontal_image.row(i);

		float* gradient_weight_ptr = gradient_weight_image.row(i);

		for (unsigned int j = 0; j < col_num; ++j)
		{
			//compute angle 

			//the first quadrant
			if (gradient_ptr[j].x >= 0 && gradient_ptr[j].y >= 0)
			{
				gradient_angle_ptr[j] = atan(gradient_ptr[j].y / (gradient_ptr[j].x + eagleeye_eps));
			}

			//the second quadrant
			if (gradient_ptr[j].x < 0 && gradient_ptr[j].y >= 0)
			{
				gradient_angle_ptr[j] = atan(gradient_ptr[j].y / (gradient_ptr[j].x - eagleeye_eps)) + EAGLEEYE_PI;
			}

			//the third quadrant
			if(gradient_ptr[j].x <= 0 && gradient_ptr[j].y <= 0)
			{
				gradient_angle_ptr[j] = atan(gradient_ptr[j].y / (gradient_ptr[j].x - eagleeye_eps)) + EAGLEEYE_PI;
			}

			//the fourth quadrant
			if (gradient_ptr[j].x > 0 && gradient_ptr[j].y <= 0)
			{
				gradient_angle_ptr[j] = atan(gradient_ptr[j].y / (gradient_ptr[j].x + eagleeye_eps)) + 2.0f * EAGLEEYE_PI;
			}

			//compute weight
			gradient_weight_ptr[j] =
 				(gradient_ptr[j].magnitude > magnitude_threshold) ? gradient_ptr[j].magnitude : 0;
		}
	}
	

	unsigned int half_width = region_width / 2;
	unsigned int half_height = region_height / 2;
	region_width = half_width * 2;
	region_height = half_height * 2;
	
	std::vector<float> info;

	float histogram[8];
	Matrix<float> his_m(1,8);

	std::vector<PointPos>::iterator iter,iend(pos_set.end());
	for (iter = pos_set.begin();iter != iend; ++iter)
	{
		PointPos current_pos = (*iter);
		//Using the nearest point
		unsigned int row_index = unsigned int(current_pos.y);
		unsigned int col_index = unsigned int(current_pos.x);

		//Find the neighbor region (region_width*region_height)around the current point
		int neighbor_row_start_index = row_index - half_width;
		neighbor_row_start_index = EAGLEEYE_MAX(neighbor_row_start_index,0);
		neighbor_row_start_index = EAGLEEYE_MIN(neighbor_row_start_index,row_num);

		int neighbor_row_end_index = row_index + half_width;
		neighbor_row_end_index = EAGLEEYE_MAX(neighbor_row_end_index,0);
		neighbor_row_end_index = EAGLEEYE_MIN(neighbor_row_end_index,row_num);

		int neighbor_col_start_index = col_index - half_height;
		neighbor_col_start_index = EAGLEEYE_MAX(neighbor_col_start_index,0);
		neighbor_col_start_index = EAGLEEYE_MIN(neighbor_col_start_index,col_num);

		int neighbor_col_end_index = col_index + half_height;
		neighbor_col_end_index = EAGLEEYE_MAX(neighbor_col_end_index,0);
		neighbor_col_end_index = EAGLEEYE_MIN(neighbor_col_end_index,col_num);

		Matrix<float> sub_gradient_angle_image = gradinet_angle_horizontal_image(Range(neighbor_row_start_index,neighbor_row_end_index),
			Range(neighbor_col_start_index,neighbor_col_end_index));
		
		Matrix<float> sub_weight_image = gradient_weight_image(Range(neighbor_row_start_index,neighbor_row_end_index),
			Range(neighbor_col_start_index,neighbor_col_end_index));

		computeHistogram<float,8>(sub_gradient_angle_image,sub_weight_image,0,6.28f,histogram);
		
		memcpy(his_m.dataptr(),histogram,sizeof(float)*8);

		info.push_back(infoEntropy<float,8>(histogram));
	}
	
	return info;
}

template<typename T>
Matrix<float> computeInfoQuantity(const Matrix<T> image,
	unsigned int row_interval,unsigned int col_interval, 
	unsigned int region_width,unsigned int region_height)
{
	unsigned int row_num=image.rows();
	unsigned int col_num=image.cols();

	unsigned int sample_row_rows=row_num/row_interval;
	unsigned int sample_col_cols=col_num/col_interval;

	Matrix<float> info_matrix(sample_row_rows,sample_col_cols);

	std::vector<PointPos> pos_set;
	for (unsigned int i=0;i<sample_row_rows;++i)
	{
		for (unsigned int j=0;j<sample_col_cols;++j)
		{
			PointPos pos;
			pos.x=j*col_interval;
			pos.y=i*row_interval;

			pos_set.push_back(pos);
		}
	}

	std::vector<float> info=computeInfoQuantity(image,pos_set,region_width,region_height);

	for (unsigned int i=0;i<sample_row_rows;++i)
	{
		float* info_data_ptr=info_matrix.row(i);
		for (unsigned int j=0;j<sample_col_cols;++j)
		{
			info_data_ptr[j]=info[i*sample_col_cols+j];
		}
	}

	return info_matrix;
}

template<typename T>
float computeEuropeanDistance(const Matrix<T>& src_m,const Matrix<T>& target_m)
{
	assert(src_m.rows()==target_m.rows());
	assert(src_m.cols()==target_m.cols());

	unsigned int src_row_num=src_m.rows();
	unsigned int src_col_num=src_m.cols();

	float distance=0;

	for (unsigned int i=0;i<src_row_num;++i)
	{
		const T* src_ptr=src_m.row(i);
		const T* target_ptr=target_m.row(i);

		for (unsigned int j=0;j<src_col_num;++j)
		{
			float d=float(src_ptr[j]-target_ptr[j]);
			distance+=d*d;
		}
	}

	return sqrt(distance)/(src_row_num*src_col_num);
}

template<typename T>
float variance(const Matrix<T> m)
{
	unsigned int row_num=m.rows();
	unsigned int col_num=m.cols();


	float mean_value=mean(m);
	float var_value=0;

	for (unsigned int i=0;i<row_num;++i)
	{
		const T* data_ptr=m.row(i);
		for (unsigned int j=0;j<col_num;++j)
		{
			var_value+=(data_ptr[j]-mean_value)*(data_ptr[j]-mean_value);
		}
	}

	var_value=sqrt(var_value/(row_num*col_num-1));

	return var_value;
}

template<typename T>
float mean(const Matrix<T> m)
{
	unsigned int rows = m.rows();
	unsigned int cols = m.cols();
	unsigned int total_num = rows * cols;

	float meanvalue = 0.0f;

	for (unsigned int i = 0; i < rows; ++i)
	{
		const T* data_ptr = m.row(i);
		for (unsigned int j = 0; j < cols; ++j)
		{
			meanvalue += float(data_ptr[j]) / float(total_num);
		}
	}

	return meanvalue;
}

template<typename T>
Matrix<float> rowmean(const Matrix<T> m)
{
	unsigned int rows = m.rows();
	unsigned int cols = m.cols();

	Matrix<float> meanvalue(1,cols);
	meanvalue.setzeros();

	float* mean_data_ptr = meanvalue.row(0);

	for (unsigned int i = 0; i < rows; ++i)
	{
		const T* data_ptr = m.row(i);
		for (unsigned int j = 0; j < cols; ++j)
		{
			mean_data_ptr[j] += float(data_ptr[j]) / float(rows);
		}
	}

	return meanvalue;
}

template<typename T>
Matrix<float> colmean(const Matrix<T> m)
{
	unsigned int rows = m.rows();
	unsigned int cols = m.cols();

	Matrix<float> meanvalue(rows,1);
	meanvalue.setzeros();
	
	for (unsigned int i = 0; i < rows; ++i)
	{
		const T* data_ptr = m.row(i);
		float* mean_data_ptr = meanvalue.row(i);

		for (unsigned int j = 0; j < cols; ++j)
		{
			mean_data_ptr[0] += float(data_ptr[j]) / float(cols);
		}
	}

	return meanvalue;
}

template<typename T>
Matrix<float> rowvar(const Matrix<T>& data)
{
	int rows = int(data.rows());
	int cols = int(data.cols());

	Matrix<float> f_data = data.transform<float>();
	
	//mean vector
	Matrix<float> mean_vec = rowmean(f_data);
	float* mean_vec_data = mean_vec.row(0);

	//variance vector
	Matrix<float> var_vec(1,cols,float(0.0f));
	float* var_vec_data = var_vec.row(0);

	for (int i = 0; i < rows; ++i)
	{
		float* f_data_data = f_data.row(i);
		for (int j = 0; j < cols; ++j)
		{
			var_vec_data[j] += (f_data_data[j] - mean_vec_data[j]) * (f_data_data[j] - mean_vec_data[j]) / float(rows - 1);
		}
	}

	for(int i = 0; i < cols; ++i)
	{
		var_vec_data[i] = sqrt(var_vec_data[i]);
	}

	return var_vec;
}

template<typename T>
Matrix<float> colvar(const Matrix<T>& data)
{
	int rows = data.rows();
	int cols = data.cols();

	Matrix<float> f_data = data.transform<float>();
	
	//mean vector
	Matrix<float> mean_vec = colmean(f_data);
	float* mean_vec_data = mean_vec.dataptr();

	//variance vector
	Matrix<float> var_vec(rows,1,0.0f);
	float* var_vec_data = var_vec.dataptr();

	for (int i = 0; i < rows; ++i)
	{
		float* f_data_data = f_data.row(i);
		for (int j = 0; j < cols; ++j)
		{
			var_vec_data[i] += (f_data_data[j] - mean_vec_data[i]) * (f_data_data[j] - mean_vec_data[i]) / float(cols - 1);
		}
	}

	for(int i = 0; i < rows; ++i)
	{
		var_vec_data[i] = sqrt(var_vec_data[i]);
	}

	return var_vec;
}

template<typename T>
Matrix<T> rot180(const Matrix<T> image)
{
	unsigned int row_num=image.rows();
	unsigned int col_num=image.cols();
	
	Matrix<T> rotated_image(row_num,col_num);

	for (unsigned int i=0;i<row_num;++i)
	{
		const T* image_data_ptr=image.row(i);
		T* rotated_data_ptr=rotated_image.row(i);

		for (unsigned int j=0;j<col_num;++j)
		{
			rotated_data_ptr[(row_num-i-1)*col_num+(col_num-j-1)]=
				image_data_ptr[i*col_num+j];
		}
	}

	return rotated_image;
}

template<typename T>
void integralImage(const Matrix<T> image, 
	Matrix<float>& sum, 
	Matrix<float>& sqsum/* =Matrix<float> */)
{
	unsigned int rows=image.rows();
	unsigned int cols=image.cols();

	//compute sum integral image
	memset(sum.dataptr(),0,sizeof(float)*rows*cols);

	for (unsigned int i=0;i<rows;++i)
	{
		const T* data_ptr=image.row(i);
		float* sum_data_ptr=sum.row(i);

		float cii=0;

		if (i==0)
		{
			for (unsigned int j=0;j<cols;++j)
			{
				if (j==0)
				{
					cii=data_ptr[j];
				}
				else
				{
					cii=cii+data_ptr[j];
				}

				sum_data_ptr[j]=cii;
			}
		}
		else
		{
			float* sum_up_data_ptr=sum.row(i-1);
			for (unsigned int j=0;j<cols;++j)
			{
				if (j==0)
				{
					cii=data_ptr[j];
				}
				else
				{
					cii=cii+data_ptr[j];
				}

				sum_data_ptr[j]=sum_up_data_ptr[j]+cii;
			}
		}

	}

	//compute square sum integral image 
	if (sqsum.dataptr()!=NULL)
	{
		memset(sqsum.dataptr(),0,sizeof(float)*rows*cols);

		Matrix<float> sqimage(rows,cols);
		for (unsigned int i=0;i<rows;++i)
		{
			float* sq_data_ptr=sqimage.row(i);
			const T* data_ptr=image.row(i);

			for (unsigned int j=0;j<cols;++j)
			{
				sq_data_ptr[j]=float(data_ptr[j]*data_ptr[j]);
			}
		}

		for (unsigned int i=0;i<rows;++i)
		{
			T* data_ptr=sqimage.row(i);
			float* sum_data_ptr=sqsum.row(i);

			float cii=0;

			if (i==0)
			{
				for (unsigned int j=0;j<cols;++j)
				{
					if (j==0)
					{
						cii=data_ptr[j];
					}
					else
					{
						cii=cii+data_ptr[j];
					}

					sum_data_ptr[j]=cii;
				}
			}
			else
			{
				float* sum_up_data_ptr=sqsum.row(i-1);
				for (unsigned int j=0;j<cols;++j)
				{
					if (j==0)
					{
						cii=data_ptr[j];
					}
					else
					{
						cii=cii+data_ptr[j];
					}

					sum_data_ptr[j]=sum_up_data_ptr[j]+cii;
				}
			}
		}
	}
}

template<typename T>
void getMaxMin(const Matrix<T>& m,T& maxvalue,T& minvalue)
{
	unsigned int rows=m.rows();
	unsigned int cols=m.cols();
	
	maxvalue=AtomicTypeTrait<T>::minval();
	minvalue=AtomicTypeTrait<T>::maxval();

	for (unsigned int i=0;i<rows;++i)
	{
		const T* data_ptr=m.row(i);
		for (unsigned int j=0;j<cols;++j)
		{
			if (data_ptr[j]>maxvalue)
			{
				maxvalue=data_ptr[j];
			}
			if (data_ptr[j]<minvalue)
			{
				minvalue=data_ptr[j];
			}
		}
	}
}

template<typename T>
Matrix<float> conv2DInSpace(const Matrix<T>& img,const Matrix<T>& kernel)
{
	unsigned int rows = img.rows();
	unsigned int cols = img.cols();
	
	unsigned int template_rows = kernel.rows();
	unsigned int template_cols = kernel.cols();

	Matrix<float> result(rows,cols);
	memset(result.dataptr(),0,sizeof(float) * rows * cols);


	for (unsigned int i = 0; i < rows; ++i)
	{
		unsigned int row_index = i * cols;
		float* r_data_ptr = result.row(i);

		for (unsigned int j = 0; j < cols; ++j)
		{
			unsigned int extend_r = EAGLEEYE_MIN(i + template_rows,rows);
			unsigned int extend_c = EAGLEEYE_MIN(j + template_cols,cols);

			for (unsigned int sub_i = i; sub_i < extend_r; ++sub_i)
			{
				const T* img_data_ptr = img.row(sub_i);
				const T* kernel_data_ptr = kernel.row(sub_i-i);

				for (unsigned int sub_j = j; sub_j < extend_c; ++sub_j)
				{
					r_data_ptr[j] += img_data_ptr[sub_j] * kernel_data_ptr[sub_j - j];
				}
			}
		}
	}

	return result;
}

template<typename T, unsigned int BINS_NUM>
float infoEntropy(T histogram[BINS_NUM])
{
	float sum_count = 0.0f;
	float probability = 0.0f;
	float h_info = 0.0f;
	for (int i = 0; i < BINS_NUM; ++i)
	{
		sum_count += histogram[i];
	}

	for (int i = 0; i < BINS_NUM; ++i)
	{
		if (histogram[i] != 0)
		{
			probability = float(histogram[i]) / sum_count;
			h_info += (-probability * log(probability));
		}
	}

	return h_info;
}

template<typename T>
float computeSSIM(const Matrix<T> target_image,const Matrix<T> detect_image,
	float max_value,float min_value)
{
	assert(target_image.rows() == detect_image.rows());
	assert(target_image.cols() == detect_image.cols());

	unsigned int target_rows = target_image.rows();
	unsigned int target_cols = target_image.cols();
	
	//compute luminance component
	float target_image_mean_value = mean(target_image);
	float detect_image_mean_value = mean(detect_image);

	float SSIM_l;
	float K1 = 0.00001f;
	float C1 = (max_value - min_value) * K1;
	SSIM_l = (2.0f * target_image_mean_value * detect_image_mean_value + C1) / 
		(target_image_mean_value * target_image_mean_value + 
		detect_image_mean_value * detect_image_mean_value + C1);

	//compute contrast component
	float target_image_var_value = variance(target_image);
	float detect_image_var_value = variance(detect_image);
	float SSIM_c;
	float K2 = 0.00001f;
	float C2 = (max_value - min_value) * K2;
	SSIM_c = (2.0f * target_image_var_value * detect_image_var_value + C2) / 
		(target_image_var_value * target_image_var_value + 
		detect_image_var_value * detect_image_var_value + C2);

	//compute structure component
	float delta_xy = 0.0f;
	for (unsigned int i = 0; i < target_rows; ++i)
	{
		const T* target_data_ptr = target_image.row(i);
		const T* detect_data_ptr = detect_image.row(i);

		for (unsigned int j = 0; j < target_cols; ++j)
		{
			delta_xy += (target_data_ptr[j] - target_image_mean_value) * 
				(detect_data_ptr[j] - detect_image_mean_value);
		}
	}

	delta_xy /= float(target_rows * target_cols - 1);

	float SSIM_s;
	float K3 = 0.00001f;
	float C3 = (max_value - min_value) * K3;
	SSIM_s = (delta_xy + C3) / (target_image_var_value * detect_image_var_value + C3);

	float SSIM = /*SSIM_l*SSIM_c**/abs(SSIM_s);

	return SSIM;
}

template<typename T>
Matrix<float> matchTemplateSSIM(const Matrix<T> match_target,
	const Matrix<T> match_template,
	unsigned int row_interval,
	unsigned int col_interval)
{
	float max_value,min_value;
	getMaxMin(match_target,max_value,min_value);

	unsigned int target_rows = match_target.rows();
	unsigned int target_cols = match_target.cols();

	unsigned int template_rows = match_template.rows();
	unsigned int template_cols = match_template.cols();

	unsigned int sample_rows = target_rows / row_interval;
	unsigned int sample_cols = target_cols / col_interval;

	Matrix<float> similarity(sample_rows,sample_cols);

	for (unsigned int i = 0; i < sample_rows; ++i)
	{
		float* similarity_data_ptr = similarity.row(i);

		for (unsigned int j = 0; j < sample_cols; ++j)
		{
			int extend_r = EAGLEEYE_MIN(i * row_interval + template_rows,target_rows);
			int extend_c = EAGLEEYE_MIN(j * col_interval + template_cols,target_cols);
			
			if (((extend_r - i * row_interval) < template_rows)||
				((extend_c - j * col_interval) < template_cols))
			{
				similarity_data_ptr[j] = 0.0f;
			}
			else
			{
				Matrix<T> sub_target_matrix = match_target(Range(i * row_interval,extend_r),Range(j * col_interval,extend_c));
				Matrix<T> sub_template_matrix = match_template(Range(0,extend_r - i * row_interval),
					Range(0,extend_c - j * col_interval));

				similarity_data_ptr[j] = computeSSIM(sub_target_matrix,sub_template_matrix,max_value,min_value);
			}

		}
	}

	return similarity;
}

template<typename T>
Matrix<T> gaussFilter(const Matrix<T>& img,const unsigned int kernel_width,const unsigned int kernel_height)
{
	float kernel_center_r = kernel_height / 2.0f;
	float kernel_center_c = kernel_width / 2.0f;
	
	float temp = 1.0f / sqrt(EAGLEEYE_PI * 2.0f);

	//construct kernel
	float weight_total = 0.0f;
	Matrix<float> kernel(kernel_height,kernel_width);
	for (unsigned int i = 0; i < kernel_height; ++i)
	{
		float* kernel_data = kernel.row(i);

		for (unsigned int j = 0; j < kernel_width; ++j)
		{
			kernel_data[j] = 
				temp * exp(-sqrt((i - kernel_center_r) * (i - kernel_center_r) + 
				(j - kernel_center_c) * (j - kernel_center_c)) / 2.0f);			

			weight_total += kernel_data[j];
		}
	}

	for (unsigned int i = 0; i < kernel_height; ++i)
	{
		float* kernel_data = kernel.row(i);

		for (unsigned int j = 0; j < kernel_width; ++j)
		{
			kernel_data[j] = kernel_data[j] / weight_total;
		}
	}

	Matrix<float> img_f = img.transform<float>();

	//convolution
	Matrix<float> response;
	convolution2D(img_f,kernel,response);
	return response.transform<T>();
}

template<typename T>
Matrix<T> maxMatrix(const Matrix<T>& left,const Matrix<T>& right)
{
	assert(left.rows() == right.rows());
	assert(left.cols() == right.cols());

	int rows = left.rows();
	int cols = left.cols();

	Matrix<T> max_m(rows,cols);

	for (int i = 0; i < rows; ++i)
	{
		const T* left_data = left.row(i);
		const T* right_data = right.row(i);
		T* max_m_data = max_m.row(i);

		for (int j = 0; j < cols; ++j)
		{
			max_m_data[j] = (left_data[j] > right_data[j]) ? left_data[j] : right_data[j];
		}
	}

	return max_m;
}

template<typename T>
Matrix<T> minMatrix(const Matrix<T>& left,const Matrix<T>& right)
{
	assert(left.rows() == right.rows());
	assert(left.cols() == right.cols());

	int rows = left.rows();
	int cols = left.cols();

	Matrix<T> min_m(rows,cols);

	for (int i = 0; i < rows; ++i)
	{
		const T* left_data = left.row(i);
		const T* right_data = right.row(i);
		T* max_m_data = min_m.row(i);

		for (int j = 0; j < cols; ++j)
		{
			max_m_data[j] = (left_data[j] < right_data[j]) ? left_data[j] : right_data[j];
		}
	}

	return min_m;
}

template<typename T>
Matrix<T> summat(const Matrix<T>& srcmat,Order d)
{
	int src_rows = srcmat.rows();
	int src_cols = srcmat.cols();

	switch(d)
	{
	case ROW:
		{
			Matrix<T> target(src_rows,1,T(0));

			for (int r = 0; r < src_rows; ++r)
			{
				const T* src_data = srcmat.row(r);
				T* target_data = target.row(r);
				for (int c = 0; c < src_cols; ++c)
				{
					target_data[0] += src_data[c];
				}
			}

			return target;
		}
	case COL:
		{
			Matrix<T> target(1,src_cols,T(0));
			T* target_data = target.row(0);

			for (int r = 0; r < src_rows; ++r)
			{
				const T* src_data = srcmat.row(r);

				for (int c = 0; c < src_cols; ++c)
				{
					target_data[c] += src_data[c];
				}
			}

			return target;
		}
	}

	return Matrix<T>();
}

template<typename T>
Matrix<T> maxmat(const Matrix<T>& srcmat,Order d,Matrix<int>& max_index)
{
	int rows=srcmat.rows();
	int cols=srcmat.cols();

	switch(d)
	{
	case ROW:
		{
			Matrix<T> max_mat(rows,1,AtomicTypeTrait<T>::minval());
			max_index=Matrix<int>(rows,1,-1);

			for (int i=0;i<rows;++i)
			{
				T* srcmat_data=srcmat.row(i);
				T* max_mat_data=max_mat.row(i);
				int* max_index_data=max_index.row(i);

				for (int j=0;j<cols;++j)
				{
					if (srcmat_data[j]>max_mat_data[0])
					{
						max_mat_data[0]=srcmat_data[j];
						max_index_data[0]=j;
					}
				}
			}
			return max_mat;
							
		}
	case COL:
		{
			Matrix<T> max_mat(1,cols,AtomicTypeTrait<T>::minval());
			max_index=Matrix<int>(1,cols,-1);
			T* max_mat_data=max_mat.row(0);
			int* max_index_data=max_index.row(0);

			for (int i=0;i<rows;++i)
			{
				T* srcmat_data=srcmat.row(i);

				for (int j=0;j<cols;++j)
				{
					if (srcmat_data[j]>max_mat_data[j])
					{
						max_mat_data[j]=srcmat_data[j];
						max_index_data[j]=i;
					}
				}
			}
			return max_mat;
		}
	}

	return Matrix<T>();
}

template<typename T>
Matrix<T> minmat(const Matrix<T>& srcmat,Order d,Matrix<int>& min_index)
{
	int rows=srcmat.rows();
	int cols=srcmat.cols();

	switch(d)
	{
	case ROW:
		{
			Matrix<T> min_mat(rows,1,AtomicTypeTrait<T>::maxval());
			min_index=Matrix<int>(rows,1,-1);

			for (int i=0;i<rows;++i)
			{
				T* srcmat_data=srcmat.row(i);
				T* min_mat_data=min_mat.row(i);
				int* min_index_data=min_index.row(i);

				for (int j=0;j<cols;++j)
				{
					if (srcmat_data[j]<min_mat_data[0])
					{
						min_mat_data[0]=srcmat_data[j];
						min_index_data[0]=j;
					}
				}
			}
			return min_mat;

		}
	case COL:
		{
			Matrix<T> min_mat(1,cols,AtomicTypeTrait<T>::maxval());
			min_index=Matrix<int>(1,cols,-1);
			T* min_mat_data=min_mat.row(0);
			int* min_index_data=min_index.row(0);

			for (int i=0;i<rows;++i)
			{
				T* srcmat_data=srcmat.row(i);

				for (int j=0;j<cols;++j)
				{
					if (srcmat_data[j]<min_mat_data[j])
					{
						min_mat_data[j]=srcmat_data[j];
						min_index_data[j]=i;
					}
				}
			}
			return min_mat;
		}
	}

	return Matrix<T>();
}

template<typename T>
T sum(const Matrix<T>& srcmat)
{
	int src_rows=srcmat.rows();
	int src_cols=srcmat.cols();
	T val(0);

	for (int i=0;i<src_rows;++i)
	{
		const T* srcmat_data=srcmat.row(i);
		for (int j=0;j<src_cols;++j)
		{
			val=val+srcmat_data[j];
		}
	}

	val=val/T(src_rows*src_cols);
	return val;
}

template<typename T>
void findHotSpot(const Matrix<T>& big_mat,int sub_size[2], int sub_nums,
	std::vector<Matrix<T>>& sub_mats,
	std::vector<std::pair<int,int>>& sub_offsets)
{
	Matrix<float> kernel_mask=fspecial(EAGLEEYE_AVERAGE,sub_size);

	int big_rows=big_mat.rows();
	int big_cols=big_mat.cols();

	Matrix<T> square_mat(big_rows,big_cols);
	AtomicTypeTrait<T>::AtomicType p_val=0;

	for (int i=0;i<big_rows;++i)
	{
		T* square_mat_data=square_mat.row(i);
		const T* big_mat_data=big_mat.row(i);

		for (int j=0;j<big_cols;++j)
		{
			for (int p=0;p<AtomicTypeTrait<T>::size;++p)
			{
				p_val=OperateTrait<T>::unit(big_mat_data[j],p);
				if (p_val>0)
				{
					OperateTrait<T>::unit(square_mat_data[j],p)=p_val*p_val;
				}
				else
				{
					OperateTrait<T>::unit(square_mat_data[j],p)=0;
				}
			}
		}
	}

	//build energy matrix
	Matrix<float> energy(big_rows,big_cols,0.0f);
	for (int i =0;i<big_rows;++i)
	{
		T* square_mat_data=square_mat.row(i);
		float* energy_data=energy.row(i);

		for (int j=0;j<big_cols;++j)
		{
			for (int p=0;p<AtomicTypeTrait<T>::size;++p)
			{
				energy_data[j]+=OperateTrait<T>::unit(square_mat_data[j],p);
			}
		}
	}

	//we should guarantee energy is not changed 
	Matrix<float> used_energy=energy;
	used_energy.clone();

	//initial parts placement based on greedy location
	for (int i=0;i<sub_nums;++i)
	{
		int max_res_r_index,max_res_c_index;
		float max_res;
		responseMax(used_energy,kernel_mask,max_res,max_res_r_index,max_res_c_index);

		max_res_r_index=EAGLEEYE_MIN(max_res_r_index,(big_rows-sub_size[1]));
		max_res_c_index=EAGLEEYE_MIN(max_res_c_index,(big_cols-sub_size[0]));

		Matrix<T> sub_mat=big_mat(Range(max_res_r_index,max_res_r_index+sub_size[1]),Range(max_res_c_index,max_res_c_index+sub_size[0]));			
		sub_mat.clone();

		//keep all elements greater than zero
		float norm_val=0;
		for (int sub_i=0;sub_i<sub_size[1];++sub_i)
		{
			T* sub_mat_data=sub_mat.row(sub_i);
			for (int sub_j=0;sub_j<sub_size[0];++sub_j)
			{
				for (int sub_p=0;sub_p<AtomicTypeTrait<T>::size;++sub_p)
				{
					p_val=OperateTrait<T>::unit(sub_mat_data[sub_j],sub_p);
					norm_val+=p_val*p_val;
// 						if (p_val<0)
// 						{
// 							OperateTrait<T>::unit(sub_mat_data[sub_j],sub_p)=0;
// 						}
// 						else
// 						{
// 							norm_val+=p_val*p_val;
// 						}
				}
			}
		}
	
		norm_val=sqrt(norm_val);
		for (int sub_i=0;sub_i<sub_size[1];++sub_i)
		{
			T* sub_mat_data=sub_mat.row(sub_i);
			for (int sub_j=0;sub_j<sub_size[0];++sub_j)
			{
				sub_mat_data[sub_j]=sub_mat_data[sub_j]/norm_val;
			}
		}

		sub_mats.push_back(sub_mat);
		sub_offsets.push_back(std::make_pair<int,int>(max_res_c_index,max_res_r_index));

		used_energy(Range(max_res_r_index,max_res_r_index+sub_size[1]),Range(max_res_c_index,max_res_c_index+sub_size[0]))=0;
	}


	//greedy process
	//remove a part at random and look for the best place to put it
	//continue until no more progress can be made (or maxiters)
	int maxiter=1000;
	int retries=10;
	float best_cover=AtomicTypeTrait<float>::minval();
	std::vector<Matrix<T>> temp_sub_mats;
	std::vector<std::pair<int,int>> temp_sub_offsets;
	Variable<float> uniform_var=Variable<float>::uniform(0.0f,1.0f);

	for (int i=0;i<retries;++i)
	{
		temp_sub_mats=sub_mats;
		temp_sub_offsets=sub_offsets;
		std::vector<int> progress(sub_nums,1);

		for (int k=0;k<maxiter;++k)
		{
			int sum_num=0;
			for (int p_index=0;p_index<sub_nums;++p_index)
			{
				sum_num+=progress[p_index];
			}
			if (sum_num==0)
			{
				break;
			}

			//copy energy from energy matrix
			used_energy.copy(energy);
			
			int p=floor(sub_nums*uniform_var.var());

			for (int p_index=0;p_index<sub_nums;++p_index)
			{
				if (p_index!=p)
				{
					int x_offset=temp_sub_offsets[p_index].first;
					int y_offset=temp_sub_offsets[p_index].second;
					used_energy(Range(y_offset,y_offset+sub_size[1]),Range(x_offset,x_offset+sub_size[0]))=0;
				}
			}

			int max_res_r_index,max_res_c_index;
			float res_max;
			responseMax(used_energy,kernel_mask,res_max,max_res_r_index,max_res_c_index);
			max_res_r_index=EAGLEEYE_MIN(max_res_r_index,(big_rows-sub_size[1]));
			max_res_c_index=EAGLEEYE_MIN(max_res_c_index,(big_cols-sub_size[0]));

			if (max_res_c_index==temp_sub_offsets[p].first&&
				max_res_r_index==temp_sub_offsets[p].second)
			{
				progress[p]=0;
				continue;
			}
			
			progress[p]=1;

			temp_sub_mats[p].copy(big_mat(Range(max_res_r_index,max_res_r_index+sub_size[1]),Range(max_res_c_index,max_res_c_index+sub_size[0])));
			temp_sub_offsets[p].first=max_res_c_index;
			temp_sub_offsets[p].second=max_res_r_index;
		}

		float covered=0;
		for (int p_index=0;p_index<sub_nums;++p_index)
		{
			int r_index=temp_sub_offsets[p_index].second;
			int c_index=temp_sub_offsets[p_index].first;
			covered+=sum(energy(Range(r_index,r_index+sub_size[1]),Range(c_index,c_index+sub_size[0])));
		}

		if (covered>best_cover)
		{
			best_cover=covered;
			sub_mats=temp_sub_mats;
			sub_offsets=temp_sub_offsets;
		}
	}
}


template<typename T>
Matrix<T> resize(const Matrix<T>& img,float scale,InterpMethod interp_method)
{
	int rows = img.rows();
	int cols = img.cols();

	int after_rows = int(ceil(rows * scale));
	int after_cols = int(ceil(cols * scale));

	return resize(img,after_rows,after_cols,interp_method);
}

template<typename T>
Matrix<T> resize(const Matrix<T>& img,int after_r,int after_c,InterpMethod interp_method)
{
	int rows = img.rows();
	int cols = img.cols();
	Matrix<T> after_img(after_r,after_c);
	
	switch(interp_method)
	{
	case BILINEAR_INTERPOLATION:
		{
			for (int i = 0; i < after_r; ++i)
			{
				T* after_img_data = after_img.row(i);
				for (int j = 0; j < after_c; ++j)
				{
					float r_f = (float(i) / float(after_r)) * float(rows);
					float c_f = (float(j) / float(after_c)) * float(cols);

					int r = int(floor(r_f));
					int c = int(floor(c_f));

					float u = r_f - r;
					float v = c_f - c;

					int first_r_index = eagleeye_min(r,(rows - 1));
					int first_c_index = eagleeye_min(c,(cols - 1));
					T first_point = img.at(first_r_index,first_c_index);

					int second_r_index = first_r_index;
					int second_c_index = eagleeye_min((c + 1),(cols - 1));
					T second_point = img.at(second_r_index,second_c_index);

					int third_r_index = eagleeye_min((r + 1),(rows - 1));
					int third_c_index = first_c_index;
					T third_point = img.at(third_r_index,third_c_index);

					int fourth_r_index = third_r_index;
					int fourth_c_index = second_c_index;
					T fourth_point = img.at(fourth_r_index,fourth_c_index);

					float first_weight = (1 - u) * (1 - v);
					float second_weight = (1 - u) * v;
					float third_weight = u * (1 - v);
					float fourth_weight = u * v;

					after_img_data[j] = T(first_point * first_weight + 
						second_point * second_weight + 
						third_point * third_weight + 
						fourth_point * fourth_weight);
				}
			}
			break;
		}
	case NEAREST_NEIGHBOR_INTERPOLATION:
		{
			for (int i = 0; i < after_r; ++i)
			{
				T* after_img_data = after_img.row(i);
				for (int j = 0; j < after_c; ++j)
				{
					float r_f = (float(i) / float(after_r)) * float(rows);
					float c_f = (float(j) / float(after_c)) * float(cols);

					int r = int(floor(r_f));
					int c = int(floor(c_f));

					after_img_data[j] = img(r,c);
				}
			}
			break;
		}
	}

	return after_img;
}

template<class ArrayPos,class ArrayVal>
Matrix<ArrayVal> gKernelFilter(const Matrix<ArrayPos>& pos_mat,const Matrix<ArrayVal>& val_mat)
{
	//basic check 
	assert(sizeof(AtomicTypeTrait<ArrayPos>::AtomicType) == sizeof(float));
	assert(sizeof(AtomicTypeTrait<ArrayVal>::AtomicType) == sizeof(float));
	assert(pos_mat.rows() * pos_mat.cols() == val_mat.rows() * val_mat.cols());

	//gaussian filter in feature space
	Matrix<ArrayPos> used_pos_mat = pos_mat;
	if (!(used_pos_mat.isfull()))
		used_pos_mat.clone();

	Matrix<ArrayVal> used_val_mat = val_mat;
	if (!(used_val_mat.isfull()))
		used_val_mat.clone();

	const float* pos_mat_data = (float*)(used_pos_mat.dataptr());
	const float* val_mat_data = (float*)(used_val_mat.dataptr());

	//feature dimension
	int featurn_dim = AtomicTypeTrait<ArrayPos>::size;
	
	//grid dimension
	int rows = used_val_mat.rows();
	int cols = used_val_mat.cols();

	Matrix<ArrayVal> filter_result(rows,cols);
	float* filter_result_data = (float*)filter_result.dataptr();

	int total = rows * cols;

	Permutohedral lattice;
	lattice.init(pos_mat_data,featurn_dim,total);
	float* norm_data = new float[total];
	for (int i = 0; i < total; ++i)
	{
		norm_data[i] = 1.0f;
	}
	lattice.compute(norm_data,norm_data,1);
	lattice.compute(filter_result_data,val_mat_data,AtomicTypeTrait<ArrayVal>::size);
	
	for (int i = 0, k = 0; i < total; ++i)
	{
		for (int j = 0; j < AtomicTypeTrait<ArrayVal>::size; ++j, ++k)
		{
			filter_result_data[k] = filter_result_data[k] / norm_data[i];
		}
	}

	delete []norm_data;
	return filter_result;
}

template<class T>
Matrix<T> randmat(int rows,int cols,Variable<T> var)
{
	Matrix<T> random_mat(rows,cols,T(0));
	for (int i=0;i<rows;++i)
	{
		T* mat_data=random_mat.row(i);
		for (int j=0;j<cols;++j)
		{
			mat_data[j]=var.var();
		}
	}

	return random_mat;
}

template<class T>
void autoBWSplit(const Matrix<T>& img,float& split_threshold,float& foreground_mean,
				 float& background_mean,
				 ThresholdMethod t_method,
				 bool auto_fore_background_judge,
				 float his_limit_ratio)
{
	//compute histogram
	int rows = img.rows();
	int cols = img.cols();
	int total_num = rows * cols;

	//find max and min of img matrix
	T max_val,min_val;
	getMaxMin(img,max_val,min_val);

	//compute histogram
	const int hist_nbins = 256;
	float step = (float(max_val) - float(min_val)) / float(hist_nbins);
	
	float loc_data[hist_nbins];

	loc_data[0] = float(min_val + min_val + step) / 2.0f;
	for (int i = 1; i < hist_nbins; ++i)
	{
		loc_data[i] = loc_data[i - 1] + step;
	}
	
	//histogram based on counts
	float hist_data[hist_nbins];
	memset(hist_data,0,sizeof(float) * hist_nbins);

	for (int i = 0; i <rows; ++i)
	{
		const T* img_data = img.row(i);
		int index;
		for (int j = 0; j < cols; ++j)
		{
			index = int(floor(float(img_data[j] - min_val) / step + eagleeye_eps));
			index = (index < hist_nbins) ? index: (hist_nbins - 1);
			index = (index > 0) ? index: 0;

			hist_data[index] = hist_data[index] + 1;
		}
	}

	if (his_limit_ratio != 0.0f)
	{
		//we would limit histogram
		//we would clip some too tall bins
		int clip_num = 0;
		//finding max counts of histogram
		int max_bin_num = 0;
		for (int i = 0; i < hist_nbins; ++i)
		{
			if (max_bin_num < hist_data[i])
			{
				max_bin_num = int(hist_data[i]);
			}
		}

		int clip_threshold = int(max_bin_num * his_limit_ratio);
		
		//clip this histogram
		for (int i = 0; i < hist_nbins; ++i)
		{
			if (hist_data[i] > clip_threshold)
			{
				clip_num += (int(hist_data[i]) - clip_threshold);
				hist_data[i] = float(clip_threshold);
			}
		}

		total_num = total_num - clip_num;
	}

	//transform histogram to probability space
	for (int i = 0; i < hist_nbins; ++i)
	{
		hist_data[i] /= float(total_num);
	}

	//analyze histogram
	//assign some default value
	split_threshold = 0;
	foreground_mean = 0;
	background_mean = 0;

	int threshold_gray_index = 0;
	float threshold_region_mean_t0 = 0;
	float threshold_region_mean_t1 = 0;

	switch(t_method)
	{
	case OSTU:
		{
			float yita = 0;
			float mu_global = 0;
			for (int i = 0; i < hist_nbins; ++i)
			{
				mu_global += loc_data[i] * hist_data[i];
			}

			for (int i = 0; i < hist_nbins - 1; ++i)
			{
				float p0 = 0;
				float p1 = 0;
				for (int j0 = 0; j0 <= i; ++j0)
				{
					p0 += hist_data[j0];
				}

				p1 = 1 - p0;

				float mut = 0;
				for (int j0 = 0; j0 <= i; ++j0)
				{
					mut += loc_data[j0] * hist_data[j0];
				}

				float mu0 = mut / p0;						//see in 10.3-6(Gonzalez)
				float mu1 = (mu_global - mut) /(1 - p0);	//see in 10.3-10(Gonzalez)

				float between_variance = p0 * p1 * (mu0 - mu1) * (mu0 - mu1);	//see in 10.3-15(Gonzalez)

				if (between_variance > yita)
				{
					yita = between_variance;
					threshold_gray_index = i;
					threshold_region_mean_t0 = mu0;
					threshold_region_mean_t1 = mu1;
				}
			}
			break;
		}
	case ENTROPIC_ANALYSIS:
		{
			float entroy_max = 0;

			for (int i = 0; i < hist_nbins - 1; ++i)
			{
				float p0 = 0;
				float p1 = 0;
				for (int j0 = 0; j0 <= i; ++j0)
				{
					p0 += hist_data[j0];
				}

				p1 = 1 - p0;

				float entroy_0 = 0;
				float entroy_1 = 0;
				for (int j0 = 0; j0 <= i; ++j0)
				{
					if (hist_data[j0] != 0 && p0 != 0)
					{
						float temp = hist_data[j0] / p0;
						entroy_0 += (-temp * log(temp));
					}
				}

				for (int j0 = i + 1; j0 < hist_nbins; ++j0)
				{
					if (hist_data[j0] != 0 && p1 != 0)
					{
						float temp = hist_data[j0] / p1;
						entroy_1 += (-temp * log(temp));
					}
				}

				if (entroy_max < (entroy_0 + entroy_1))
				{
					entroy_max = entroy_0 + entroy_1;
					threshold_gray_index = i;
				}
			}

			//find mean value of every region
			float mu0 = 0;
			float mu1 = 0;
			for (int j0 = 0; j0 <= threshold_gray_index; ++j0)
			{
				mu0 += loc_data[j0] * hist_data[j0];
			}

			for (int j0 = threshold_gray_index + 1; j0 < hist_nbins; ++j0)
			{
				mu1 += loc_data[j0] * hist_data[j0];
			}

			threshold_region_mean_t0 = mu0;
			threshold_region_mean_t1 = mu1;

			break;
		}
	}

	if (auto_fore_background_judge)
	{
		//distinguish foreground and background 
		//we think the within-class variance of background is smaller
		float delta_0 = 0;
		float delta_1 = 0;
		for (int j = 0; j <= threshold_gray_index; ++j)
		{
			delta_0 += (loc_data[j] - threshold_region_mean_t0) * (loc_data[j] - threshold_region_mean_t0) * hist_data[j];
		}

		for (int j = threshold_gray_index +1; j < hist_nbins; ++j)
		{
			delta_1 += (loc_data[j] - threshold_region_mean_t1) * (loc_data[j] - threshold_region_mean_t1) * hist_data[j];
		}

		if (delta_0 < delta_1)
		{
			//left region 0 is background
			foreground_mean = threshold_region_mean_t0;
			background_mean = threshold_region_mean_t1;
		}
		else
		{
			//right region 1 is background
			foreground_mean = threshold_region_mean_t1;
			background_mean = threshold_region_mean_t0;
		}
	}
	else
	{
		foreground_mean = threshold_region_mean_t0;
		background_mean = threshold_region_mean_t1;
	}

	split_threshold = loc_data[threshold_gray_index];
}

template<typename T>
void squeezeRegion(const Matrix<T>& img, T squeezed_label,int squeezed_size,Matrix<T>& squeezed_img)
{
	int rows = img.rows();
	int cols = img.cols();
	int total = cols * rows;

	if (squeezed_img.isempty())
	{
		squeezed_img = Matrix<T>(rows,cols,T(0));
	}
	squeezed_img.copy(img);

	//from left to right
	for (int i = 0; i < rows; ++i)
	{
		const T* img_data = img.row(i);
		T* squeeze_img_data = squeezed_img.row(i);
		for (int j = 1; j < cols - 1; ++j)
		{
			T current_data = img_data[j];

			if (img_data[j] != squeezed_label && 
				img_data[j - 1] == squeezed_label)
			{
				
				for (int check_index = 1 ; check_index <= squeezed_size; ++check_index)
				{
					if ((j - check_index) < 0)
						break;
					else
						squeeze_img_data[j - check_index] = current_data;
				}	
			}
			if (img_data[j] != squeezed_label && 
				img_data[j + 1] == squeezed_label)
			{
				for (int check_index = 1 ; check_index <= squeezed_size; ++check_index)
				{
					if ((j + check_index) >= cols)
						break;
					else
						squeeze_img_data[j + check_index] = current_data;
				}	
			}

		}
	}

	//from up to down
	for (int i = 1; i < rows - 1; ++i)
	{
		for (int j = 0; j < cols; ++j)
		{
			T current_data = img(i,j);

			if ((current_data != squeezed_label) &&
				(img(i - 1,j) == squeezed_label))
			{
				for (int check_index = 1 ; check_index <= squeezed_size; ++check_index)
				{
					if ((i - check_index) < 0)
						break;
					else
						squeezed_img(i - check_index,j) = current_data;
				}	
			}

			if ((current_data != squeezed_label) && 
				(img(i + 1,j) == squeezed_label))
			{
				for (int check_index = 1 ; check_index <= squeezed_size; ++check_index)
				{
					if ((i + check_index) >= rows)
						break;
					else
						squeezed_img(i + check_index,j) = current_data;
				}	
			}
		}
	}
}

}
