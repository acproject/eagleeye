#include "MatrixMath.h"
#include "MetaOperation.h"

#include "../Matlab/MatlabInterface.h"
namespace eagleeye
{
	template<class ImageSigT>
	ImageDestripe<ImageSigT>::ImageDestripe()
	{
		m_gray_threshold = 200;
		m_search_range = 50;
		m_nth_biggest = 30;

		setNumberOfInputSignals(1);

		setNumberOfOutputSignals(1);
		setOutputPort(new ImageSignal<float>,0);
	}
	
	template<class ImageSigT>
	ImageDestripe<ImageSigT>::~ImageDestripe()
	{

	}

	template<class ImageSigT>
	bool ImageDestripe<ImageSigT>::selfcheck()
	{
		//只接受单通道数据
		if( !AtomicTypeTrait<PixelType>::size==1 )
			return false;
		if( !getInputImageSignal() )
			return false;
		return true;
	}

	template<class ImageSigT>
	void ImageDestripe<ImageSigT>::executeNodeInfo()
	{
		typedef NormalizeOperation<PixelType,float> NormType;
		
		ImageSigT* input_img_signal = getInputImageSignal();
		Matrix<PixelType> input_img = input_img_signal->img;
		Matrix<float> input_img32, input_img32_transpose, output_img32;
		int rows = input_img.rows();
		int cols = input_img.cols();
		PixelType min_value;
		PixelType max_value;
		getMaxMin( input_img, max_value, min_value );

		//1.先变成[0,255]的uchar,得到原始和
		input_img32 = input_img.transform(NormType(min_value,max_value,0.0f,255.0f));
		input_img32_transpose = input_img32.t();
		//2.去横条纹和纵条纹
		horizontalDestripe( input_img32 );
		horizontalDestripe( input_img32_transpose );
		//3.得到输出图像
		output_img32 = input_img32_transpose.t();
		output_img32 = maxMatrix(output_img32,input_img32);
		for( int i=0; i<rows; ++i )
			for( int j=0; j<cols; ++j )
			{
				if( output_img32(i,j)>255.0f )
					output_img32(i,j) = 255.0f;
			}
		//4.转换为输出类型
		ImageSignal<float>* output_img_signal = getOutputImageSignal();
		output_img_signal->img = output_img32;

		return;
	}

	template<class ImageSigT>
	void ImageDestripe<ImageSigT>::horizontalDestripe( Matrix<float>& reference_img )
	{
		int width = reference_img.cols();
		int height = reference_img.rows();
		int range = EAGLEEYE_MIN(m_search_range,width-1);
		Matrix<float> sub_reference_img = reference_img(Range(0,height),
														Range(0, range ) );
		sub_reference_img.clone();
		Matrix<float> sub_img_left =sub_reference_img;

		Matrix<float> sub_reference_img2 = reference_img(Range(0,height),
										  Range(width-range-1, width-1 ) );
		sub_reference_img2.clone();
		Matrix<float> sub_img_right = sub_reference_img2;

		for( unsigned int i=0; i<reference_img.rows(); ++i )
		{
			float* img_row_pointer = sub_img_left.row(i);
			////每一行在一定范围内搜索较大的值，左边和右边一起找，那个大算哪个
			nth_element( img_row_pointer, img_row_pointer+m_nth_biggest-1,
				img_row_pointer+range-1,std::greater<float>() );
			float big_air_value_left = *(img_row_pointer+m_nth_biggest-1);

			img_row_pointer = sub_img_right.row(i);
			nth_element( img_row_pointer, img_row_pointer+m_nth_biggest-1,
				img_row_pointer+range-1,std::greater<float>() );
			float big_air_value_right = *(img_row_pointer+m_nth_biggest-1 );

			float big_air_value = EAGLEEYE_MAX( big_air_value_left, big_air_value_right );
			if( 0 == big_air_value )
				big_air_value = 1.0f;
			for( unsigned int j=0; j<reference_img.cols(); ++j )
			{
				//这一行用这个值归一化
				reference_img(i,j) = reference_img(i,j)/big_air_value*255.0f;
			}
		}
		//putToMatlab( reference_img, "why" );
		return;
	}
}