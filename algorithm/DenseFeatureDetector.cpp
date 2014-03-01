#include "DenseFeatureDetector.h"
#include "EagleeyeCore.h"

namespace eagleeye
{
DenseFeatureDetector::DenseFeatureDetector()
{
	m_init_scale = 1.0f;
	m_scale_levels = 1;
	m_scale_mul = 1.0f;
	m_init_xy_step = 10;
	m_init_img_bound = 10;
	m_search_radius = 10;
	m_vary_xy_step_with_scale = false;
	m_vary_img_bound_with_scale = false;

	m_calc_main_dir_flag = true;
}
DenseFeatureDetector::~DenseFeatureDetector()
{

}

void DenseFeatureDetector::detect(const Matrix<float>& img,std::vector<KeyPoint>& keypoints)
{
	int rows = img.rows();
	int cols = img.cols();

	float cur_scale = m_init_scale;
	int cur_step = m_init_xy_step;
	int cur_bound = m_init_img_bound;

	//find keypoints number
	int keypoints_num = 0;
	for( int cur_level = 0; cur_level < m_scale_levels; ++cur_level )
	{
		for( int x = cur_bound; x < cols - cur_bound; x += cur_step )
		{
			for( int y = cur_bound; y < rows - cur_bound; y += cur_step )
			{
				keypoints_num++;
			}
		}

		cur_scale = cur_scale * m_scale_mul;
		if( m_vary_xy_step_with_scale ) cur_step = static_cast<int>( cur_step * m_scale_mul + 0.5f );
		if( m_vary_img_bound_with_scale ) cur_bound = static_cast<int>( cur_bound * m_scale_mul + 0.5f );
	}

	keypoints.resize(keypoints_num);

	//record keypoints
	int keypoint_index  = 0;
	for( int cur_level = 0; cur_level < m_scale_levels; ++cur_level )
	{
		for( int x = cur_bound; x < cols - cur_bound; x += cur_step )
		{
			for( int y = cur_bound; y < rows - cur_bound; y += cur_step )
			{
				KeyPoint kp;
				kp.pt[0] = float(x);
				kp.pt[1] = float(y);				
				kp.size = cur_scale * 2;
				
				float angle = 0.0f;
				if (m_calc_main_dir_flag)
				{
					angle = calcMainDirection(img,kp.pt,m_search_radius,1.6f,32);
				}
				kp.angle = angle;

				keypoints[keypoint_index] = kp;
				keypoint_index++;
			}
		}

		cur_scale = cur_scale * m_scale_mul;
		if( m_vary_xy_step_with_scale ) cur_step = static_cast<int>( cur_step * m_scale_mul + 0.5f );
		if( m_vary_img_bound_with_scale ) cur_bound = static_cast<int>( cur_bound * m_scale_mul + 0.5f );
	}
}

void DenseFeatureDetector::detect(int rows,int cols,std::vector<KeyPoint>& keypoints)
{
	float cur_scale = m_init_scale;
	int cur_step = m_init_xy_step;
	int cur_bound = m_init_img_bound;
	for( int cur_level = 0; cur_level < m_scale_levels; cur_level++ )
	{
		for( int x = cur_bound; x < cols - cur_bound; x += cur_step )
		{
			for( int y = cur_bound; y < rows - cur_bound; y += cur_step )
			{
				KeyPoint kp;
				kp.pt[0] = float(x);
				kp.pt[1] = float(y);
				kp.size = cur_scale *2;
				keypoints.push_back( kp );
			}
		}

		cur_scale = cur_scale * m_scale_mul;
		if( m_vary_xy_step_with_scale ) cur_step = static_cast<int>( cur_step * m_scale_mul + 0.5f );
		if( m_vary_img_bound_with_scale ) cur_bound = static_cast<int>( cur_bound * m_scale_mul + 0.5f );
	}

}

void DenseFeatureDetector::setDetectorParams(float init_scale,int scale_levels,
											 float scale_mul,int init_xy_step, 
											 int init_img_bound,
											 int search_radius,
											 bool vary_xy_step_with_scale,
											 bool vary_img_bound_with_scale)
{
	m_init_scale = init_scale;
	m_scale_levels = scale_levels;
	m_scale_mul = scale_mul;
	m_init_xy_step = init_xy_step;
	m_init_img_bound = init_img_bound;
	m_search_radius = search_radius;
	m_vary_xy_step_with_scale = vary_xy_step_with_scale;
	m_vary_img_bound_with_scale = vary_img_bound_with_scale;
}
}