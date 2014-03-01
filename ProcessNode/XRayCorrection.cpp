#include "XRayCorrection.h"
#include "MatrixMath.h"

namespace eagleeye
{
XRayCorrection::XRayCorrection()
{
	m_invalid_low_threshold = 0;
	m_invalid_high_threshold = 0;

	m_air_s_c = 0;
	m_air_e_c = 0;
	m_monitor_s_r = 0;
	m_monitor_e_r = 0;

	m_basic_correction_flag = false;

	//set input port number
	setNumberOfInputSignals(1);

	//set output port property
	setNumberOfOutputSignals(1);
	setOutputPort(makeOutputSignal(),OUTPUT_PORT_CORRECTED_IMAGE_DATA);
}

XRayCorrection::~XRayCorrection()
{
	
}

void XRayCorrection::executeNodeInfo()
{
	//get input image
	ImageSignal<float>* img_signal = getInputImageSignal(0);
	if ( !img_signal )
	{
		EAGLEEYE_ERROR("There is no input image\n");
		return;
	}

	Matrix<float> input_img = img_signal->img;

	Matrix<float> output_img( input_img.rows(), input_img.cols() );
	output_img.copy( input_img );

	//////////////////////////////////////////////////////////////////////////
	if ( m_basic_correction_flag )
	{
		airbackgroundCorrection( output_img );
		monitorCorrection( output_img );
	}

	clearPixels( output_img );
	//////////////////////////////////////////////////////////////////////////
		

	ImageSignal<float>* output_img_signal = getOutputImageSignal( 0 );
	output_img_signal->img = output_img;

	EAGLEEYE_INFO("XRay correction is completed\n");
}

void XRayCorrection::clearPixelsThreshold(float low_threshold,float high_threshold)
{
	m_invalid_low_threshold = low_threshold;
	m_invalid_high_threshold = high_threshold;

	//update the time
	modified();
}

void XRayCorrection::setAirColumn( int air_s_c, int air_e_c )
{
	m_air_s_c = air_s_c;
	m_air_e_c = air_e_c;
}

void XRayCorrection::setMonitorRow( int monitor_s_r, int monitor_e_r )
{
	m_monitor_s_r = monitor_s_r;
	m_monitor_e_r = monitor_e_r;
}

void XRayCorrection::airbackgroundCorrection( Matrix<float>& img )
{
	if ( !m_air_img.dataptr() )
	{
		m_air_img = img( Range( 0, img.rows() ), Range( m_air_s_c, m_air_e_c ) );
		m_air_img.clone();
	}

	Matrix<float> air_mean = colmean( m_air_img );

	int rows = img.rows();
	int cols = img.cols();

	//subtract background data
	Matrix<float> background_mean = colmean( m_background_img );
	for ( int i = 0; i < rows; ++i )
	{
		float* background_mean_data = background_mean.row( i );
		float* air_img_data = air_mean.row( i );
		float* img_data = img.row( i );

		air_img_data[ 0 ] = air_img_data[ 0 ] - background_mean_data[ 0 ];

		for ( int j = 0; j < cols; ++j )
		{
			img_data[ j ] = img_data[ j ] - background_mean_data[ 0 ];
		}
	}
	
	//divide air data
	for ( int i = 0; i < rows; ++i)
	{
		float* air_mean_data = air_mean.row( i );
		float* img_data = img.row( i );
		for (int j = 0; j < cols; ++j )
		{
			if ( air_mean_data[ 0 ] != 0 )
			{
				img_data[ j ] = img_data[ j ] / air_mean_data[ 0 ];
			}
		}
	}
}

void XRayCorrection::monitorCorrection( Matrix<float>& img )
{
	if ( m_monitor_e_r > m_monitor_s_r )
	{
		Matrix<float> monitor_region = img( Range( m_monitor_s_r, m_monitor_e_r ), Range( 0, img.cols() ) );
		Matrix<float> monitor_mean = rowmean( monitor_region );

		int rows = img.rows();
		int cols = img.cols();
		float* monitor_mean_data = monitor_mean.row( 0 );

		for ( int i = 0; i < rows; ++i )
		{
			float* img_data = img.row( i );
			for ( int j = 0; j < cols; ++j )
			{
				if ( monitor_mean_data[ j ] != 0 )
				{
					img_data[ j ] = img_data[ j ] / monitor_mean_data[ j ];
				}
			}
		}
	}
}

void XRayCorrection::clearPixels( Matrix<float>& img )
{
	if ( m_invalid_high_threshold > m_invalid_low_threshold )
	{
		int rows = img.rows();
		int cols = img.cols();

		for ( int i = 0; i < rows; ++i )
		{
			float* img_data = img.row( i );
			for ( int j = 0; j < cols; ++j )
			{
				if ( img_data[ j ] > m_invalid_low_threshold && img_data[ j ] < m_invalid_high_threshold )
				{
					img_data[ j ] = 0;
				}
			}
		}
	}
}

void XRayCorrection::setAirImg( Matrix<float> air_img )
{
	m_air_img = air_img;
}

void XRayCorrection::setBackgroundImg( Matrix<float> background_img )
{
	m_background_img = background_img;
}

bool XRayCorrection::selfcheck()
{	
	Superclass::selfcheck();

	ImageSignal<float>* input_sig = getInputImageSignal();
	if ( !input_sig )
	{
		EAGLEEYE_ERROR( "sorry, input image pixel isn't consistent... \n please be careful... \n" );
		return false;
	}

	if ( m_basic_correction_flag )
	{
		if ( !m_background_img.dataptr() )
		{
			EAGLEEYE_ERROR( "background image couldn't be empty\n" );
			return false;
		}

		if ( ( m_air_s_c == m_air_e_c ) && ( !m_air_img.dataptr() ) )
		{
			EAGLEEYE_ERROR( "air data must be defined\n" );
			return false;
		}

		if ( m_monitor_s_r == m_monitor_e_r )
		{
			EAGLEEYE_ERROR( "monitor data must be defined\n" );
			return false;
		}
	}

	return true;
}
}