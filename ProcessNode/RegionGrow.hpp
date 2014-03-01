#include "../Matlab/MatlabInterface.h"

namespace eagleeye
{
	template<class ImageSigT>
	RegionGrow<ImageSigT>::RegionGrow()
	{
		m_region_counter = 0;
		m_neib_num = 4;
		m_min_area = 150;
		m_region_counter_area_limited = 0;

		//set input port number
		setNumberOfInputSignals(1);
		
		//set output port property
		setNumberOfOutputSignals(1);
		setOutputPort(new ImageSignal<unsigned char>(),OUTPUT_PORT_LABEL_IMAGE_DATA);

		EAGLEEYE_MONITOR_VAR_EXT(int,setRegionMinArea,getRegionMinArea,"region_min_area","min=0");
	}

	template<class ImageSigT>
	RegionGrow<ImageSigT>::~RegionGrow()
	{

	}
	
	template<class ImageSigT>
	bool RegionGrow<ImageSigT>::selfcheck()
	{
		//只接受单通道数据
		if( !AtomicTypeTrait<PixelType>::size==1 )
			return false;
		if( !getInputImageSignal() )
			return false;
		if( m_neib_num !=4 && m_neib_num !=8 )
			return false;
		return true;
	}

	template<class ImageSigT>
	void RegionGrow<ImageSigT>::executeNodeInfo()
	{
		ImageSigT* input_img_signal = getInputImageSignal();
		Matrix<PixelType> input_img = input_img_signal->img;

		int rows = input_img.rows();
		int cols = input_img.cols();
		m_label_image = Matrix<float>( rows, cols, (float)0.0 );
		grow( input_img );
	}

	template<class ImageSigT>
	void RegionGrow<ImageSigT>::grow( Matrix<PixelType>& image )
	{
		//复制图像，大于0的值全部赋255
		int rows = image.rows();
		int cols = image.cols();
		Matrix<unsigned char> img_copy = image.transform<unsigned char>();
		for( int i=0; i<rows; ++i )
			for( int j=0; j<cols; ++j )
			{
				if( img_copy(i,j)>0 )
					img_copy(i,j) = 255;
			}
		//在复制图像上做区域增长。不可以并行
		for( int i=0; i<rows; ++i )
			for( int j=0; j<cols; ++j )
			{
				if( img_copy(i,j) !=0 )
				{
					m_region_counter++;
					getBlock( i, j, img_copy );
				}
			}
		//在要输出的区域大小序列里，把面积阈值以下的删掉
		for(std::vector<int>::iterator iter=m_pixel_num_vector.begin(); iter!=m_pixel_num_vector.end(); )
		{
			if( *iter <= m_min_area)
				iter = m_pixel_num_vector.erase(iter);
			else
				iter ++ ;
		}
	}

	template<class ImageSigT>
	void RegionGrow<ImageSigT>::getBlock( int y, int x, Matrix<unsigned char>& image )
	{
		//四邻域或八邻域区域生长
		int i, j, pt_x, pt_y;
		int min_x, max_x, min_y, max_y;
		int m=0, n=0, pos=0;	
		unsigned char pix_value;
		std::vector<int> mask_x;
		std::vector<int> mask_y;
		bool break_flag = 0;

		int width = image.cols();
		int height = image.rows();

		for( i=y; i<height; ++i )
		{
			for( j=x; j<width; ++j )
			{
				pix_value = image(i,j);
				if( pix_value == image(i,j) )
				{
					//找到有值的点，求四连通，凡是连通区内的都算作一个区域。可以改成八连通
					image(i,j) = 0;
					mask_y.push_back(i);
					mask_x.push_back(j);
					min_y = i;
					max_y = i;
					min_x = j;
					max_x = j;
					m_pixel_num_vector.push_back(1);
					m_label_image(i,j) = (float)m_region_counter;
					m = n;
					pos = n;
					n++;
					while( m < n )
					{
						pt_x = mask_x[m];
						pt_y = mask_y[m];
						int index_x[8] = {pt_x,pt_x,(pt_x-1),(pt_x+1),(pt_x-1),(pt_x+1),(pt_x-1),(pt_x+1)};
						int index_y[8] = {(pt_y-1),(pt_y+1),pt_y,pt_y,(pt_y-1),(pt_y-1),(pt_y+1),(pt_y+1)};
						for( int k=0; k<m_neib_num; ++k )
						{
							if( index_x[k]<0 || index_y[k]<0 || index_x[k]>width-1 || index_y[k]>height-1 )
								continue;
							if( image(index_y[k],index_x[k]) == pix_value )
							{
								image(index_y[k],index_x[k]) = 0;
								mask_x.push_back( index_x[k] );
								mask_y.push_back( index_y[k] );
								if( index_x[k]>max_x )
									max_x = index_x[k];
								if( index_x[k]<min_x )
									min_x = index_x[k];
								if( index_y[k]>max_y )
									max_y = index_y[k];
								if( index_y[k]<min_y )
									min_y = index_y[k];
								m_label_image( index_y[k], index_x[k] ) = (float)m_region_counter;
								m_pixel_num_vector[ m_region_counter - 1 ] ++;
								n++;
								if( height*width == n )
								{
									m = n;
									break;
								}
							}
						}
						m++;
					}
					if( (n-pos)>0 && (n-pos)<width*height )
					{
						if( mask_x.size() > (unsigned int)m_min_area )
						{
							//给mask加一个margin，为了之后可能的形态学操作
							int margin = 1;
							RECT region_rect;
							region_rect.left =		EAGLEEYE_MAX( 0, min_x-margin );
							region_rect.top  =		EAGLEEYE_MAX( 0, min_y-margin );
							region_rect.right =		EAGLEEYE_MIN( width-1, max_x+margin );
							region_rect.bottom =	EAGLEEYE_MIN( height-1, max_y+margin );
							m_rects.push_back( region_rect );
							Matrix<unsigned char> mask_img( region_rect.bottom-region_rect.top+1,
															region_rect.right-region_rect.left+1, (unsigned char)0 );
							for( unsigned int k=0; k<mask_x.size(); ++k )
							{
								mask_img( mask_y[k]-region_rect.top, mask_x[k]-region_rect.left ) = (unsigned char)255;
							}
							m_masks.push_back( mask_img );
							m_region_counter_area_limited++;
						}
					}
					break_flag = 1;
					break;	//不继续找不连通，但值一样的区域了
				}
			}
			if( break_flag )
				break;
		}
		mask_x.clear();
		mask_y.clear();
	}
	
	template<class ImageSigT>
	int RegionGrow<ImageSigT>::getBiggestIndex()
	{
		int index = -1;
		int max_pix_num = 0;
		for( unsigned int i=0; i<m_pixel_num_vector.size(); ++i )
		{
			if( m_pixel_num_vector[i]>max_pix_num )
			{
				max_pix_num = m_pixel_num_vector[i];
				index = (int)i;
			}
		}
		return index;
	}

}