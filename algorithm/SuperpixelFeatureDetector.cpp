#include "SuperpixelFeatureDetector.h"
#include "MatrixMath.h"

namespace eagleeye
{
SuperpixelFeatureDetector::SuperpixelFeatureDetector()
{
	m_bound_width = 0;
	m_bound_height = 0;
	m_superpixel_num = 0;
	m_minimum_superpixel_area = 200;
	m_exclude_index = -1;
	m_sampling_num = 0;
}
SuperpixelFeatureDetector::~SuperpixelFeatureDetector()
{

}

void SuperpixelFeatureDetector::detect(const Matrix<float>& img,std::vector<KeyPoint>& keypoints)
{
	//image size
	int rows = img.rows();
	int cols = img.cols();

	//computing gradient
	//we would sample based on gradient magnitude
	Matrix<float> gradient_img = computeGradientMag(img);
	//get rid of gradient magnitude at image edges
	gradient_img(Range(0,m_bound_height),Range(0,(unsigned int)gradient_img.cols())).setval(0.0f);
	gradient_img(Range(gradient_img.rows() - m_bound_height,gradient_img.rows()),Range(0,gradient_img.cols())).setval(0.0f);
	gradient_img(Range(0,gradient_img.rows()),Range(0,m_bound_width)).setval(0.0f);
	gradient_img(Range(0,gradient_img.rows()),Range(gradient_img.cols() - m_bound_width,gradient_img.cols())).setval(0.0f);

	std::vector<DynamicArray<int>> discrete_data_in_superpixel(m_superpixel_num);
	std::vector<DynamicArray<float>> distribution_in_superpixel(m_superpixel_num);
	std::vector<float> cumulative_distribution(m_superpixel_num,0.0f);
	std::vector<int> count(m_superpixel_num,0);

	//computing pixel number in every superpixel
	std::vector<int> pixel_num_in_superpixel(m_superpixel_num,0);
	for (int i = 0; i < rows; ++i)
	{
		int* index_data = m_superpixel_img.row(i);
		for (int j = 0; j < cols; ++j)
		{
			pixel_num_in_superpixel[index_data[j]] += 1;
		}
	}

	//build dynamic array
	for (int index = 0; index < m_superpixel_num; ++index)
	{
		discrete_data_in_superpixel[index] = DynamicArray<int>(pixel_num_in_superpixel[index]);
		distribution_in_superpixel[index] = DynamicArray<float>(pixel_num_in_superpixel[index]);
	}

	for (int i = 0; i < rows; ++i)
	{
		int* index_data = m_superpixel_img.row(i);
		float* gradient_img_data = gradient_img.row(i);
		for (int j = 0; j < cols; ++j)
		{
			int index = index_data[j];
			discrete_data_in_superpixel[index][count[index]] = i * cols + j;
			distribution_in_superpixel[index][count[index]] = gradient_img_data[j];
			cumulative_distribution[index] = cumulative_distribution[index] + gradient_img_data[j];

			count[index] = count[index] + 1;
		}
	}

	for (int index = 0; index < m_superpixel_num; ++index)
	{
		if (cumulative_distribution[index] != 0.0f)
		{
			for (int m = 0; m < count[index]; ++m)
			{
				distribution_in_superpixel[index][m] /= cumulative_distribution[index];
			}
		}
	}

	//get samples from every superpixel
	//it's very slow,like a snail
	for (int index = 0; index < m_superpixel_num; ++index)
	{
		if (pixel_num_in_superpixel[index] > m_minimum_superpixel_area && index != m_exclude_index)
		{
			DynamicArray<int> discrete_data = discrete_data_in_superpixel[index];
			DynamicArray<float> distribution = distribution_in_superpixel[index];
			Variable<int> local_var = Variable<int>::discreteDis(discrete_data,distribution);

			int sampling_total = pixel_num_in_superpixel[index] / 2;
			if (m_sampling_num != 0)
			{
				sampling_total = m_sampling_num;
			}
			
			int sampling_count = 0;
			while(sampling_count < sampling_total)
			{
				int pos_index = local_var.var();
				int pos_r = pos_index / cols; 
				int pos_c = pos_index - (pos_index / cols) * cols;

				KeyPoint region_point;
				region_point.pt[0] = float(pos_c);
				region_point.pt[1] = float(pos_r);
				keypoints.push_back(region_point);

				sampling_count++;
			}
		}
	}
}

void SuperpixelFeatureDetector::setSuperpixelImage(Matrix<int> superpixel_img,int superpixel_num,int exclude_index)
{
	m_superpixel_img = superpixel_img;
	m_superpixel_num = superpixel_num;
	m_exclude_index = exclude_index;
}

void SuperpixelFeatureDetector::setImageBounds(int bound_width,int bound_height)
{
	m_bound_width = bound_width;
	m_bound_height = bound_height;
}

void SuperpixelFeatureDetector::setSamplingNum(int sampling_num /* = 1 */)
{
	m_sampling_num = sampling_num;
}

}