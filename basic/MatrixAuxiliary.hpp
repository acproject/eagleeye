
namespace eagleeye
{
	template<typename T>
	Matrix<T> padArray(const Matrix<T>& src,const Array<int,2>& ext,T padval,PadPattern pat/* =PrePost */)
	{
		int src_mat_rows=src.rows();
		int src_mat_cols=src.cols();

		switch(pat)
		{
		case Pre:
			{
				int ext_rows=ext[0];
				int ext_cols=ext[1];

				int dst_mat_rows=src_mat_rows+ext_rows;
				int dst_mat_cols=src_mat_cols+ext_cols;

				Matrix<T> dst_mat(dst_mat_rows,dst_mat_cols,padval);
				Matrix<T> sub_dst_mat=dst_mat(Range(ext_rows,dst_mat_rows),Range(ext_cols,dst_mat_cols));
				sub_dst_mat.copy(src);

				return dst_mat;
			}
		case Post:
			{
				int ext_rows=ext[0];
				int ext_cols=ext[1];

				int dst_mat_rows=src_mat_rows+ext_rows;
				int dst_mat_cols=src_mat_cols+ext_cols;

				Matrix<T> dst_mat(dst_mat_rows,dst_mat_cols,padval);
				Matrix<T> sub_dst_mat=dst_mat(Range(0,src_mat_rows),Range(0,src_mat_cols));
				sub_dst_mat.copy(src);

				return dst_mat;
			}
		case PrePost:
			{
				int ext_rows=ext[0];
				int ext_cols=ext[1];

				int dst_mat_rows=src_mat_rows+2*ext_rows;
				int dst_mat_cols=src_mat_cols+2*ext_cols;

				Matrix<T> dst_mat(dst_mat_rows,dst_mat_cols,padval);
				Matrix<T> sub_dst_mat=dst_mat(Range(ext_rows,src_mat_rows+ext_rows),Range(ext_cols,src_mat_cols+ext_cols));
				sub_dst_mat.copy(src);

				return dst_mat;
			}
		default:
			{
				return Matrix<T>();
			}
		}
	}

	template<typename T>
	void normalize(Matrix<T>& m,T minvalue,T maxvalue)
	{
		//element size is equal to one
		assert(AtomicTypeTrait<T>::size==1);
		unsigned int row=m.rows();
		unsigned int col=m.cols();

		//find max and min element
		if (minvalue==0&&maxvalue==0)
		{
			//find min and max
			minvalue=AtomicTypeTrait<T>::maxval();
			maxvalue=AtomicTypeTrait<T>::minval();
			for (int i=0;i<row;++i)
			{
				T* mdata=m.row(i);
				for (int j=0;j<col;++j)
				{
					if (mdata[j]>maxvalue)
					{
						maxvalue=mdata[j];
					}

					if (mdata[j]<minvalue)
					{
						minvalue=mdata[j];
					}
				}
			}
		}

		for (unsigned int i=0;i<row;++i)
		{
			T* mdata=m.row(i);
			for (unsigned int j=0;j<col;++j)
			{
				mdata[j]=(mdata[j]-minvalue)/(maxvalue-minvalue);
				mdata[j]=EAGLEEYE_MAX(mdata[j],0);
				mdata[j]=EAGLEEYE_MIN(mdata[j],1);
			}
		}
	}

	template<typename T>
	void normalize(DynamicArray<T>& arr,T minvalue,T maxvalue)
	{
		//element size must be equal to 1
		assert(AtomicTypeTrait<T>::size==1);

		T* arr_data=arr.dataptr();
		int arr_size=arr.size();

		//find max and min element
		if (maxvalue==0&&minvalue==0)
		{
			minvalue=AtomicTypeTrait<T>::maxval();
			maxvalue=AtomicTypeTrait<T>::minval();
			for (int i=0;i<arr_size;++i)
			{
				if (arr_data[i]>maxvalue)
				{
					maxvalue=arr_data[i];
				}

				if (arr_data[i]<minvalue)
				{
					minvalue=arr_data[i];
				}
			}
		}

		for (int i=0;i<arr_size;++i)
		{
			arr_data[i]=(arr_data[i]-minvalue)/(maxvalue-minvalue);
			arr_data[i]=EAGLEEYE_MAX(arr_data[i],0);
			arr_data[i]=EAGLEEYE_MIN(arr_data[i],1);
		}

	}

	template<class T>
	Matrix<T> averageImageWithLabel(const Matrix<int>& label,const Matrix<T>& img)
	{
		assert(label.rows()==img.rows());
		assert(label.cols()==img.cols());

		int rows=img.rows();
		int cols=img.cols();

		std::map<int,Array<double,AtomicTypeTrait<T>::size>> ave_map;
		std::map<int,int> ave_num;
		for (int i=0;i<rows;++i)
		{
			const int* label_data=label.row(i);
			const T* img_data=img.row(i);
			for (int j=0;j<cols;++j)
			{
				if (ave_map.find(label_data[j])==ave_map.end())
				{
					ave_num[label_data[j]]=0;

					for (int p=0;p<AtomicTypeTrait<T>::size;++p)
					{
						ave_map[label_data[j]][p]=double(OperateTrait<T>::unit(img_data[j],p));
					}
					
				}
				else
				{
					ave_num[label_data[j]]++;

					for (int p=0;p<AtomicTypeTrait<T>::size;++p)
					{
						ave_map[label_data[j]][p]+=double(OperateTrait<T>::unit(img_data[j],p));
					}
				}
			}
		}

		std::map<int,Array<double,AtomicTypeTrait<T>::size>>::iterator iter,iend(ave_map.end());
		for (iter=ave_map.begin();iter!=ave_map.end();++iter)
		{
			iter->second=iter->second/ave_num[iter->first];
		}

		Matrix<T> ave_img(rows,cols,T(0));
		for (int i=0;i<rows;++i)
		{
			const int* label_data=label.row(i);
			T* ave_img_data=ave_img.row(i);
			for (int j=0;j<cols;++j)
			{
				for (int p=0;p<AtomicTypeTrait<T>::size;++p)
				{
					OperateTrait<T>::unit(ave_img_data[j],p)=AtomicTypeTrait<T>::AtomicType(ave_map[label_data[j]][p]);
				}
			}
		}

		return ave_img;
	}

	template<class T>
	void splitsToMultiLayers(const Matrix<T>& multi_layers_matrix,
		std::vector<Matrix<typename AtomicTypeTrait<T>::AtomicType>>& single_layer_matrixs)
	{
		int rows=multi_layers_matrix.rows();
		int cols=multi_layers_matrix.cols();

		for (int layer_index=0;layer_index<AtomicTypeTrait<T>::size;++layer_index)
		{
			Matrix<AtomicTypeTrait<T>::AtomicType> single_mat(rows,cols);
		
			for (int i=0;i<rows;++i)
			{
				AtomicTypeTrait<T>::AtomicType* single_mat_data=single_mat.row(i);
				const T* multi_mat_data=multi_layers_matrix.row(i);
				
				for (int j=0;j<cols;++j)
				{
					single_mat_data[j]=OperateTrait<T>::unit(multi_mat_data[j],layer_index);
				}
			}

			single_layer_matrixs.push_back(single_mat);
		}
	}
}
