#include "../Matlab/MatlabInterface.h"
#include "ImageDestripe.h"
#include "RegionGrow.h"
#include "BasicMath/MedianFilter1D.h"
#include "BasicMath/VectorMath.h"

namespace eagleeye
{
template<class ImageSigT>
CarriageMask<ImageSigT>::CarriageMask()
{
	m_gray_threshold = 200;
	m_scale_profile = 0.5f;
	m_scale_lightscan = 0.5f*0.15f;

	//设置2个输出端口
	setNumberOfOutputSignals( 3 );
	//端口0: float的局部车厢图像
	setOutputPort( new ImageSignal<unsigned char>, 0 );
	//端口1: unsigned char的货物的mask
	setOutputPort( new ImageSignal<unsigned char>, 1 );
	//端口2: debug图像
	setOutputPort( new ImageSignal<Array<unsigned char,3>>, 2 );
	//设置输入端口
	setNumberOfInputSignals( 1 );

	m_roi.top=0; m_roi.left=0; m_roi.bottom=0; m_roi.right=0;
}

template<class ImageSigT>
CarriageMask<ImageSigT>::~CarriageMask()
{

}

template<class ImageSigT>
bool CarriageMask<ImageSigT>::selfcheck()
{
	//只接受单通道数据
	ImageSigT* input_img_sig=dynamic_cast<ImageSigT*>(getInputPort(0));

	if( !AtomicTypeTrait<PixelType>::size==1 )
		return false;
	if( !input_img_sig )
		return false;
	return true;
}

template<class ImageSigT>
void CarriageMask<ImageSigT>::calcOutputImage( Matrix<unsigned char>& org, Matrix<unsigned char>& img8,
	Matrix<unsigned char>& mask, Matrix<Array<unsigned char,3>>& debug )
{
	//输出
	img8 = org( Range(m_roi.top,m_roi.bottom),
		Range(m_roi.left,m_roi.right) );
	img8.clone();

	mask = img8;			
	mask.clone();

	for( unsigned int i=0; i<mask.rows(); ++i )
		for( unsigned int j=0; j<mask.cols(); ++j )
		{
			if( mask(i,j)>m_gray_threshold )//|| mask(i,j)<50 
				mask(i,j) = 0;
			else
				mask(i,j) = 255;
		}

		cv::Mat opencv_output_mask( mask.rows(), mask.cols(), CV_8UC1, mask.dataptr() );
		cv::erode( opencv_output_mask, opencv_output_mask, cv::Mat(), cv::Point(-1,-1), 5 );	//5

		debug = org.transform<Array<unsigned char,3>>();
		Matrix<Array<unsigned char,3>> debug_image_sub_region 
			= debug( Range(m_roi.top,m_roi.bottom),Range(m_roi.left,m_roi.right) );
		for( unsigned int i=0; i<debug_image_sub_region.rows(); i++ )
			for( unsigned int j=0; j<debug_image_sub_region.cols(); j++ )
			{
				if( mask(i,j)>0)
					debug_image_sub_region(i,j)[0] = 255;
			}
			//debug_image.flipud();
}

template<class ImageSigT>
void CarriageMask<ImageSigT>::executeNodeInfo()
{
	//1.先去条纹
	ImageDestripe<ImageSigT> img_destripe_proc;
	img_destripe_proc.setInputPort( dynamic_cast<ImageSigT*>(getInputPort(0)) );
	img_destripe_proc.start();
	Matrix<float> destriped_img32 = img_destripe_proc.getOutputImage( 0 );
	//destriped_img32.flipud();
	Matrix<unsigned char> destriped_img8 = destriped_img32.transform<unsigned char>();		

	int height = destriped_img32.rows();
	int width = destriped_img32.cols();

	ImageSignal<unsigned char>* gray_image_signal = dynamic_cast<ImageSignal<unsigned char>*>( getOutputPort(0) );
	ImageSignal<unsigned char>* mask_image_signal = dynamic_cast<ImageSignal<unsigned char>*>( getOutputPort(1) );
	ImageSignal<Array<unsigned char,3>>* debug_image_signal = dynamic_cast<ImageSignal<Array<unsigned char,3>>*>( getOutputPort(2) );

	Matrix<unsigned char> output_img8;
	Matrix<unsigned char> output_mask;
	Matrix<Array<unsigned char,3>> debug_image;

	bool have_roi = checkROI( height, width );
	if( have_roi )
	{
		//不用检测车箱位置，用输入的roi		
		calcOutputImage( destriped_img8, output_img8, output_mask, debug_image );
		gray_image_signal->img = output_img8;
		mask_image_signal->img = output_mask;
		debug_image_signal->img = debug_image;
	}
	else
	{
		//Matrix<unsigned char> img_profile = resize( destriped_img8, m_scale_profile );
		Matrix<unsigned char> img_lightscan = resize( destriped_img8, m_scale_lightscan );		

		//2.把图像缩小到光幕图像大小，二值化，区域增长得到最大区域子图像
		Matrix<unsigned char> binary_lightscan( img_lightscan.rows(), img_lightscan.cols(), (unsigned char)0 );
		for( unsigned int i=0; i<img_lightscan.rows(); ++i )
			for( unsigned int j=0; j<img_lightscan.cols(); ++j )
			{
				if( img_lightscan(i,j) < m_gray_threshold )
					binary_lightscan(i,j) = 255;
			}

		ImageSignal<unsigned char>* local_image_signal = new ImageSignal<unsigned char>;
		local_image_signal->img = binary_lightscan;	

		RegionGrow<ImageSignal<unsigned char>> region_grow_proc;
		region_grow_proc.setInputPort( local_image_signal );
		region_grow_proc.start();

		int index = region_grow_proc.getBiggestIndex();
		if( index>=0 )	
		{
			Matrix<unsigned char> mask = region_grow_proc.getMask( index );
			RECT car_rect = region_grow_proc.getRect( index );

			//3.拿轮廓
			getProfile( mask );
			//putToMatlab( m_profile_high, "h" );
			//4.求车头与车箱分割位置
			int cut_left=0,cut_right=0;
			int sts = getCutLoc(cut_left,cut_right);
			if( cut_left==cut_right || sts<0 )
				return;

			//5.得到车的位置.由于图像是反的，top和bottom意义也反过来。top是车底。
			RECT goods_rect;
			int margin = 5;
			goods_rect.left = (int)( (car_rect.left+cut_left+margin)/m_scale_lightscan );
			goods_rect.right = (int)( (car_rect.left+cut_right-margin)/m_scale_lightscan );
			goods_rect.bottom = (int)( (max_val( m_profile_high, cut_left )-margin) / m_scale_lightscan );
			goods_rect.top = (int)( ( median( m_profile_low, cut_left ) + margin + 20 )/m_scale_lightscan );
			m_roi = goods_rect;
			if( m_roi.bottom<=m_roi.top || m_roi.right<=m_roi.left )
			{
				m_roi.top = 0; m_roi.bottom=0; m_roi.left = 0; m_roi.right=0;
			}
			else
			{
				calcOutputImage( destriped_img8, output_img8, output_mask, debug_image );
				gray_image_signal->img = output_img8;
				mask_image_signal->img = output_mask;
				debug_image_signal->img = debug_image;
			}

		}
		delete local_image_signal;
	}		
	return;
}

template<class ImageSigT>
bool CarriageMask<ImageSigT>::checkROI( int h, int w )
{
	bool have_roi = false;
	RECT r0; r0.bottom=0; r0.right=0; r0.top=0; r0.left=0;
	RECT& r = m_roi;
	if( r.right>w )
		r.right = w;
	if( r.bottom>h )
		r.bottom = h;
	if( r.left>=r.right || r.left<0 )
		r = r0;
	if( r.top>=r.bottom || r.top<0 )
		r = r0;
	if( r.right-r.left>100 && r.bottom-r.top>100 )
		have_roi = true;

	return have_roi;
}
template<class ImageSigT>
void CarriageMask<ImageSigT>::getProfile(Matrix<unsigned char>& image )
{
	int height = image.rows();
	int width = image.cols();
	//m_profile_low = std::vector<int>(width,0);
	//m_profile_high = std::vector<int>(width,0);
	for( int i=0; i<width; ++i )
	{
		//求每一列轮廓的最高点和最低点
		int high_value = 0;
		int low_vlaue = height-1;
		bool col_has_car = false;
		for( int j=0; j<height; ++j )
		{
			if( image(j,i)>0 )
			{
				col_has_car = true;
				if( j>high_value )
					high_value = j;
				if( j<low_vlaue )
					low_vlaue = j;
			}
		}
		if( !col_has_car )
		{
			m_profile_high.push_back( 0 );
			m_profile_low.push_back( 0 );
		}
		else
		{
			m_profile_high.push_back( high_value );
			m_profile_low.push_back( low_vlaue );
		}
	}
	
	MedianFilter1D<int> median_filter;
	median_filter.setWindowSize( 5 );
	m_profile_high = median_filter.execute( m_profile_high, true );
	m_profile_low = median_filter.execute( m_profile_low, true );
}

template<class ImageSigT>
int CarriageMask<ImageSigT>::getCutLoc( int& cut_left, int& cut_right )
{
	//int& cut_loc = cut_left;
	//int length = (int)m_profile_high.size();
	//
	//int diff_distance = 4;
	//int thresh = 11;

	//int diff_val = thresh;
	//int start_index = 30;//原55
	//int safe_index = 130;
	//float mean_front = 0;
	//if( length>50 )
	//	mean_front = mean( m_profile_high, 25, 50 );

	//if( mean_front<50.0f )
	//{
	//	start_index = start_index + 40;
	//	safe_index = safe_index + 40;
	//}
	//int end_index = EAGLEEYE_MIN( safe_index, (int)((float)m_profile_high.size()*0.6f));
	//for( int i=start_index; i<end_index; ++i )
	//{
	//	int tmp = m_profile_high[i]-m_profile_high[i-diff_distance];
	//	if( abs(tmp)>=abs(diff_val) )
	//	{
	//		diff_val = tmp;
	//		cut_loc = i;
	//	}
	//}
	//if( 0 == cut_loc )
	//{
	//	for( int i=safe_index; i<end_index; ++i )
	//	{
	//		if( abs(m_profile_high[i]-m_profile_high[i-diff_distance]) > diff_val )
	//			cut_loc = i;
	//	}
	//}

	int sts = 0;
	int max_value=10, max_loc=-1;
	int diff_distance = 4;
	int thresh = 11;
	int l = m_profile_high.size();
	for( unsigned int i=0; i<m_profile_high.size(); i++ )
	{
		if( m_profile_high[i]>max_value )
		{
			max_value = m_profile_high[i];
			max_loc = i;
		}
	}
	if( max_loc>0 )
	{
		for( int i=max_loc; i>0; i-- )
		{
			if( i-diff_distance>=0 )
			{
				if( abs(m_profile_high[i]-m_profile_high[i-diff_distance])<thresh )
					continue;	
				else
				{
					cut_left = i;
					break;
				}
			}
			else
			{
				cut_left = i;
				break;
			}
		}
		for( int i=max_loc; i<l; i++ )
		{
			if( i+diff_distance<l )
			{
				if( abs(m_profile_high[i]-m_profile_high[i+diff_distance])<thresh )
					continue;
				else
				{
					cut_right = i;
					break;
				}
			}
			else
			{
				cut_right = i;
				break;
			}
		}
	}
	else
		sts = -1;

	if( cut_right-cut_left<20 )
		sts = -1;
	//putToMatlab( m_profile_high, "l" );
	return sts;
}

}