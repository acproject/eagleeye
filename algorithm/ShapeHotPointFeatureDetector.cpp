#include "ShapeHotPointFeatureDetector.h"
#include "MatrixMath.h"

namespace eagleeye
{
ShapeHotPointFeatureDetector::ShapeHotPointFeatureDetector()
{
	m_jitter_num = 10;
	m_jitter_switch = false;
	m_jitter_degree = 10.0f;
	m_canny_low_threshold = 50.0;
	m_canny_high_threshold = 70.0;

	m_img_bound_width = 0;
	m_img_bound_height = 0;

	m_sampling_limit_ratio = 0.5f;
}
ShapeHotPointFeatureDetector::~ShapeHotPointFeatureDetector()
{

}

void ShapeHotPointFeatureDetector::detect(const Matrix<float>& img,std::vector<KeyPoint>& keypoints)
{
	//image size
	int rows = img.rows();
	int cols = img.cols();

	bool is_exclude = false;
	if (!m_exclude_region.isempty())
		is_exclude = true;

	//extract edge from local image
	Matrix<unsigned char> edges = canny(img.transform<unsigned char>(),m_canny_low_threshold,m_canny_high_threshold);

	//edges points number
	int edge_points_num = 0;
	for (int i = 0; i < rows; ++i)
	{
		unsigned char* edges_data = edges.row(i);
		for (int j = 0; j < cols; ++j)
		{
			if (edges_data[j] == 255)
				edge_points_num++;			
		}
	}

	//we shouldn't sample too much
	if (edge_points_num * m_jitter_num > int(rows * cols * m_sampling_limit_ratio))
		m_jitter_num = int(float(rows * cols * m_sampling_limit_ratio) / float(edge_points_num));

	for (int i = 0; i < rows; ++i)
	{
		unsigned char* edges_data = edges.row(i);

		for (int j = 0; j < cols; ++j)
		{
			//exclude points in 'exclude region'
			if (is_exclude)
				if (m_exclude_region(i,j) == 0)
					continue;

			if (edges_data[j] == 255 && (i > m_img_bound_height) && (i < (rows - m_img_bound_height)) &&
				(j > m_img_bound_width) && (j < (cols - m_img_bound_width)))
			{
				KeyPoint p;
				p.pt[0] = float(j);	//x
				p.pt[1] = float(i);	//y

				//sampling points at edges
				keypoints.push_back(p);

				//sampling points around edges
				if (m_jitter_switch)
				{
					Variable<float> x_gaussian_random = Variable<float>::gaussian(double(j),double(m_jitter_degree));	//x
					Variable<float> y_gaussian_random = Variable<float>::gaussian(double(i),double(m_jitter_degree));	//y

					//jitter
					int jitter_count = 0;
					while(jitter_count < m_jitter_num)
					{
						p.pt[0] = x_gaussian_random.var();
						p.pt[1] = y_gaussian_random.var();
						
						//limit position
						p.pt[0] = EAGLEEYE_MAX(0.0f,p.pt[0]);
						p.pt[0] = EAGLEEYE_MIN(p.pt[0],float(cols - 1));
						p.pt[1] = EAGLEEYE_MAX(0.0f,p.pt[1]);
						p.pt[1] = EAGLEEYE_MIN(p.pt[1],float(rows - 1));

						//exclude points in 'exclude regions'
						if (is_exclude)
							if(m_exclude_region(int(p.pt[1]),int(p.pt[0])) == 0)
								continue;

						if ((p.pt[0] > m_img_bound_width) && (p.pt[0] < (cols - m_img_bound_width)) && 
							(p.pt[1] > m_img_bound_height) && (p.pt[1] < (rows - m_img_bound_height)))
						{
							keypoints.push_back(p);
						}

						jitter_count++;
					}
				}
			}
		}
	}

}

void ShapeHotPointFeatureDetector::setImageBounds(int bound_width,int bound_height)
{
	m_img_bound_width = bound_width;
	m_img_bound_height = bound_height;
}

void ShapeHotPointFeatureDetector::setCannyThreshold(double low_threshold,double high_threshold)
{
	m_canny_low_threshold = low_threshold;
	m_canny_high_threshold = high_threshold;
}

void ShapeHotPointFeatureDetector::setRandomJitterSwitch(bool flag,float jitter_degree,int jitter_num /* = 10 */)
{
	m_jitter_switch = flag;
	m_jitter_degree = jitter_degree;
	m_jitter_num = jitter_num;
}

void ShapeHotPointFeatureDetector::setSamplingLimitRatio(float ratio)
{
	m_sampling_limit_ratio = ratio;
}

}