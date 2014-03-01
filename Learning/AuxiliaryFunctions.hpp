namespace eagleeye
{
template<class T>
int getColor(const T p)
{
	int index = 0;
	int scale = 255;
	int s = 1;
	for (int i = 0; i < AtomicTypeTrait<T>::size; ++i)
	{
		unsigned char meta_color = (unsigned char)(OperateTrait<T>::unit(p,i));
		index += s * meta_color;
		s = s * scale;
	}

	return index;
}

template<class T>
void putColor(T& p,int cc)
{
	for (int i = 0; i < AtomicTypeTrait<T>::size; ++i)
	{
		OperateTrait<T>::unit(p,i) = cc&0xff;
		cc = cc>>8;
	}
}

template<class T>
void analyzeAnnotationImg(const Matrix<T>& img,int labels_num,
						  Matrix<float>& energy,
						  std::vector<int>& colors_map,
						  float gt_prob)
{
	const float u_energy = -log( 1.0f / float( labels_num ) );
	const float n_energy = -log( ( 1.0f - gt_prob ) / ( labels_num - 1 ) );
	const float p_energy = -log( gt_prob );

	int rows = int( img.rows() );
	int cols = int( img.cols() );
	int total_pix_num = rows * cols;

	int n_colors = 0;
	colors_map.resize( 255 );

	energy = Matrix<float>( total_pix_num , labels_num );
	
	const T* img_data = img.dataptr();
	for( int k = 0; k < total_pix_num; k++ )
	{
		// Map the color to a label
		int c = getColor( img_data[k] );
		int i;
		for( i = 0; i < n_colors && c != colors_map[ i ]; i++ );
		if (c && i == n_colors)
		{
			if (i < labels_num)
				colors_map[ n_colors++ ] = c;
			else
				c = 0;
		}

		// Set the energy
		float* probability_data = energy.row( k );
		if ( c )
		{
			for( int j = 0; j < labels_num; j++ )
				probability_data[ j ] = n_energy;
			probability_data[ i ] = p_energy;
		}
		else
		{
			for( int j = 0; j < labels_num; j++ )
				probability_data[ j ] = u_energy;
		}
	}
}

template<class T>
void shuffling(std::vector<T>& data,ShuffleMode shuffle_mode /* = IN_PLACE */)
{
	switch(shuffle_mode)
	{
	case IN_PLACE:
		{
			int n = data.size();
			int i = n - 1;
			int j = 0;
			while(i > 0)
			{
				j = rand() % (i + 1);

				//swap j and i
				T tmp = data[i];
				data[i] = data[j];
				data[j] = tmp;

				i--;
			}
			break;
		}
	}
}

}