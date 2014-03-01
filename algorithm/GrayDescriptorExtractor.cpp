#include "GrayDescriptorExtractor.h"

namespace eagleeye
{
GrayDescriptorExtractor::GrayDescriptorExtractor()
{
	m_width = 10;
	m_height = 10;
	m_ext_mode = CENTER_EXT;
}
GrayDescriptorExtractor::~GrayDescriptorExtractor()
{

}

void GrayDescriptorExtractor::setRegionSize(int width,int height)
{
	m_width = width;
	m_height = height;
}

void GrayDescriptorExtractor::setExtentMode(ExtentMode ext_mode)
{
	m_ext_mode = ext_mode;
}

void GrayDescriptorExtractor::setUnitPara(MemoryBlock param_block)
{
	_Parameter* para = (_Parameter*)param_block.block();
	m_width = para->width;
	m_height = para->height;
	m_ext_mode = ExtentMode(para->ext_mode);
}
void GrayDescriptorExtractor::getUnitPara(MemoryBlock& param_block)
{
	param_block = MemoryBlock(sizeof(_Parameter));
	_Parameter* para = (_Parameter*)param_block.block();
	para->width = m_width;
	para->height = m_height;
	para->ext_mode = (int)m_ext_mode;
}

void GrayDescriptorExtractor::computeImpl(const Matrix<float>& img,std::vector<KeyPoint>& keypoints, Matrix<float>& img_descriptors)
{
	int rows = img.rows();
	int cols = img.cols();
	int keypoints_num = keypoints.size();
	int description_dim = m_width * m_height;
	img_descriptors = Matrix<float>(keypoints_num,description_dim,float(0.0f));

	switch(m_ext_mode)
	{
	case LEFT_UP_EXT:
		{
			int keypoints_num =  keypoints.size();
			for (int i = 0; i < keypoints_num; ++i)
			{
				int x = (int)keypoints[i].pt[0];
				int y = (int)keypoints[i].pt[1];
				float* des_data = img_descriptors.row(i);

				if ((x - m_width) > 0 && (y - m_height) > 0)
				{
					Matrix<float> local_region = img(Range(y - m_height,y),Range(x - m_width,x));
					for (int r = 0; r < m_height; ++r)
					{
						float* local_region_data = local_region.row(r);
						int index = r * m_width;
						for (int c = 0; c < m_width; ++c)
						{
							des_data[index + c] = local_region_data[c];
						}
					}
				}
				else
				{
					//assign empty
					keypoints[i] = KeyPoint();
				}
			}

			break;
		}
	case CENTER_EXT:
		{
			int keypoints_num =  keypoints.size();
			for (int i = 0; i < keypoints_num; ++i)
			{
				int x = (int)keypoints[i].pt[0];
				int y = (int)keypoints[i].pt[1];
				float* des_data = img_descriptors.row(i);

				if ((x - m_width / 2) > 0 && (y - m_height / 2) > 0 &&
					(x + m_width / 2) < cols && (y + m_height / 2) < rows)
				{
					Matrix<float> local_region = img(Range(y - m_height / 2,y - m_height / 2 + m_height),Range(x - m_width / 2,x - m_width / 2 + m_width));
					for (int r = 0; r < m_height; ++r)
					{
						float* local_region_data = local_region.row(r);
						int index = r * m_width;
						for (int c = 0; c < m_width; ++c)
						{
							des_data[index + c] = local_region_data[c];
						}
					}
				}
				else
				{
					//assign empty
					keypoints[i] = KeyPoint();
				}
			}
			break;
		}
	case RIGHT_BOTTOM_EXT:
		{
			int keypoints_num =  keypoints.size();
			for (int i = 0; i < keypoints_num; ++i)
			{
				int x = (int)keypoints[i].pt[0];
				int y = (int)keypoints[i].pt[1];
				float* des_data = img_descriptors.row(i);

				if ((x + m_width) < cols && (y + m_height) < rows)
				{
					Matrix<float> local_region = img(Range(y,y + m_height),Range(x,x + m_width));
					for (int r = 0; r < m_height; ++r)
					{
						float* local_region_data = local_region.row(r);
						int index = r * m_width;
						for (int c = 0; c < m_width; ++c)
						{
							des_data[index + c] = local_region_data[c];
						}
					}
				}
				else
				{
					//assign empty
					keypoints[i] = KeyPoint();
				}
			}
			break;
		}
	}
}

int GrayDescriptorExtractor::descriptorSize()
{
	return m_width * m_height;
}

}