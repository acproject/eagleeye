#include "TextonDescriptorExtractor.h"
#include "MatrixMath.h"
#include <math.h>
#include "cuda/MatrixMath.cuh"
#include "Print.h"
#include "opencv2/opencv.hpp"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
TextonDescriptorExtractor::TextonDescriptorExtractor()
{
	m_filter_bank_type = LM_FILTER_BANK;
	m_filter_size = 49;
}

TextonDescriptorExtractor::~TextonDescriptorExtractor()
{

}

void TextonDescriptorExtractor::computeImpl(const Matrix<float>& img,std::vector<KeyPoint>& keypoints, 
											Matrix<float>& img_descriptors)
{
	switch(m_filter_bank_type)
	{
	case LM_FILTER_BANK:
		{
			generateLMFilterBank();
			break;
		}
	case S_FILTER_BANK:
		{
			generateSFilterBank();
			break;
		}
	case MR_FILTER_BANK:
		{
			generateMRFilterBank();
			break;
		}
	}
	
	//kernel number
	int kernel_nums = m_filter_bank.size();

	//filter response
	std::vector<Matrix<float>> response;
	response.resize(kernel_nums);

#ifdef _USE_CUDA_
	float** kernels = new float*[kernel_nums];
	for (int i = 0; i < kernel_nums; ++i)
	{
		kernels[i] = m_filter_bank[i].dataptr();
	}

	float** results = new float*[kernel_nums];
	for (int i = 0; i < kernel_nums; ++i)
	{
		response[i] = Matrix<float>(img.rows(),img.cols());
		results[i] = response[i].dataptr();
	}

	convolution2DBankCu(img.dataptr(),img.rows(),img.cols(),kernels,kernel_nums,m_filter_size,m_filter_size,results);

	//release memory space
	delete []kernels;
	delete []results;

#else
	Matrix<float> img_proxy = img;
	convolution2DBank(img_proxy,m_filter_bank,response);
#endif

	//generate description for every keypoint
	int keypoints_num = keypoints.size();
	img_descriptors = Matrix<float>(keypoints_num,kernel_nums);
	for (int i = 0; i < keypoints_num; ++i)
	{
		float* img_descriptors_data = img_descriptors.row(i);
		for (int kernel_index = 0; kernel_index < kernel_nums; ++kernel_index)
		{
			Matrix<float> res = response[kernel_index];
			img_descriptors_data[kernel_index] = res(int(keypoints[i].pt[1]),int(keypoints[i].pt[0]));
		}
	}
}

void TextonDescriptorExtractor::setFilterBankType(_FilterBankType filter_bank_type)
{
	m_filter_bank_type = filter_bank_type;
}

Matrix<float> TextonDescriptorExtractor::makeFilter(const Matrix<float>& pts,float scale, int phase_x, int phase_y, int sup)
{
	Matrix<float> gx = gauss1d(pts(Range(0,1),Range(0,pts.cols())),3.0f * scale, 0, phase_x);
	Matrix<float> gy = gauss1d(pts(Range(1,2),Range(0,pts.cols())), scale, 0, phase_y);	
	Matrix<float> g_dot = gx.dot(gy);

	Matrix<float> f = g_dot.reshape(sup,sup,COL);	
	normalizeFilter(f);

	return f;
}

void TextonDescriptorExtractor::generateLMFilterBank()
{
	m_filter_bank.resize(48);

	int sup = m_filter_size;	//support of the largest filter (must be odd)
	float scalex[3] = {1.0f,2.0f,3.0f};	//sigma_{x} for the oriented filters
	scalex[0] = pow(sqrt(2.0f), scalex[0]);
	scalex[1] = pow(sqrt(2.0f), scalex[1]);
	scalex[2] = pow(sqrt(2.0f), scalex[2]);

	int norient = 6;	//number of orientations

	int nrotinv = 12;
	int nbar = 3 * norient;
	int nedge = 3 * norient;

	int nf = nbar + nedge + nrotinv;

	Matrix<float> orgpts(2,sup * sup,float(0));

	int hsup = (sup - 1) / 2;

	Matrix<int> xx;
	Matrix<int> yy;
	meshgrid(-hsup,hsup + 1,-hsup,hsup + 1,xx,yy);
	xx = xx.t();
	yy = yy.t();

	float* orgpts_row_first = orgpts.row(0);
	float* orgpts_row_second = orgpts.row(1);
	for (int i = 0; i < sup * sup; ++i)
	{
		orgpts_row_first[i] = float(xx(i));
		orgpts_row_second[i] = float(yy(i));
	}

	int count = 0;
	for (int scale_index = 0; scale_index < 3; ++scale_index)
	{
		for(int orient_index = 0; orient_index < norient; ++orient_index)
		{
			float angle = float(EAGLEEYE_PI) * float(orient_index) / float(norient);
			float c = cos(angle);
			float s = sin(angle);
			
			Matrix<float> rotate_mat(2,2);
			rotate_mat(0,0) = c;
			rotate_mat(0,1) = -s;
			rotate_mat(1,0) = s;
			rotate_mat(1,1) = c;

			Matrix<float> rot_pts = rotate_mat * orgpts;
			m_filter_bank[count] = makeFilter(rot_pts,scalex[scale_index],0,1,sup);
			m_filter_bank[count + nedge] = makeFilter(rot_pts,scalex[scale_index],0,2,sup);

			count++;
		}
	}

	count = nbar + nedge;
	float scales[4];

	scales[0] = pow(sqrt(2.0f),1);
	scales[1] = pow(sqrt(2.0f),2);
	scales[2] = pow(sqrt(2.0f),3);
	scales[3] = pow(sqrt(2.0f),4);

	for (int i = 0; i < 4; ++i)
	{
		int kernel_size[2]={sup,sup};
		Matrix<float> gaussian_filter = fspecial(EAGLEEYE_GAUSSIAN,kernel_size,1,double(scales[i]));
		normalizeFilter(gaussian_filter);
		m_filter_bank[count] = gaussian_filter;
		
		Matrix<float> log_filter_1 = fspecial(EAGLEEYE_LOG,kernel_size,1,double(scales[i]));
		normalizeFilter(log_filter_1);
		m_filter_bank[count + 1] = log_filter_1;

		Matrix<float> log_filter_2 = fspecial(EAGLEEYE_LOG,kernel_size,1,double(scales[i]*3.0f));
		normalizeFilter(log_filter_2);
		m_filter_bank[count + 2] = log_filter_2;

		count = count +3;
	}
}

Matrix<float> TextonDescriptorExtractor::makeFilter(int sup,float sigma_val,int tau)
{
	int hsup = (sup - 1) / 2;
	Matrix<int> x,y;
	meshgrid(-hsup,hsup+1,-hsup,hsup+1,x,y);
	Matrix<float> r(sup,sup);

	for (int i = 0; i < sup; ++i)
	{
		float* r_data = r.row(i);
		int* x_data = x.row(i);
		int* y_data = y.row(i);
		for (int j = 0; j < sup; ++j)
		{
			r_data[j] = sqrt(float(x_data[j] * x_data[j]) + float(y_data[j] * y_data[j]));
			r_data[j] = cos(r_data[j] * (float(EAGLEEYE_PI * tau) / float(sigma_val))) * 
				exp(-(r_data[j] * r_data[j])/(2 * sigma_val * sigma_val));
		}
	}

	normalizeFilter(r);

	return r;
}

void TextonDescriptorExtractor::generateSFilterBank()
{
	m_filter_bank.resize(13);

	int sup = m_filter_size;

	m_filter_bank[0] = makeFilter(sup,2,1);
	m_filter_bank[1] = makeFilter(sup,4,1);
	m_filter_bank[2] = makeFilter(sup,4,2);
	m_filter_bank[3] = makeFilter(sup,6,1);
	m_filter_bank[4] = makeFilter(sup,6,2);
	m_filter_bank[5] = makeFilter(sup,6,3);
	m_filter_bank[6] = makeFilter(sup,8,1);
	m_filter_bank[7] = makeFilter(sup,8,2);
	m_filter_bank[8] = makeFilter(sup,8,3);
	m_filter_bank[9] = makeFilter(sup,10,1);
	m_filter_bank[10] = makeFilter(sup,10,2);
	m_filter_bank[11] = makeFilter(sup,10,3);
	m_filter_bank[12] = makeFilter(sup,10,4);
}

void TextonDescriptorExtractor::generateMRFilterBank()
{
	m_filter_bank.resize(38);

	int sup = m_filter_size;	//support of the largest filter (must be odd)
	float scalex[3] = {1.0f,2.0f,4.0f};	//sigma_{x} for the oriented filters

	int norient = 6;	//number of orientations

	int nrotinv = 2;
	int nbar = 3 * norient;
	int nedge = 3 * norient;

	int nf = nbar + nedge + nrotinv;

	Matrix<float> orgpts(2,sup * sup,float(0));

	int hsup = (sup - 1) / 2;

	Matrix<int> xx;
	Matrix<int> yy;
	meshgrid(-hsup,hsup+1,-hsup,hsup+1,xx,yy);
	xx=xx.t();
	yy=yy.t();

	float* orgpts_row_first = orgpts.row(0);
	float* orgpts_row_second = orgpts.row(1);
	for (int i = 0; i < sup * sup; ++i)
	{
		orgpts_row_first[i] = float(xx(i));
		orgpts_row_second[i] = float(yy(i));
	}

	int count = 0;
	for (int scale_index = 0; scale_index < 3; ++scale_index)
	{
		for(int orient_index = 0; orient_index < norient; ++orient_index)
		{
			float angle = float(EAGLEEYE_PI) * float(orient_index) / float(norient);
			float c = cos(angle);
			float s = sin(angle);

			Matrix<float> rotate_mat(2,2);
			rotate_mat(0,0) = c;
			rotate_mat(0,1) = -s;
			rotate_mat(1,0) = s;
			rotate_mat(1,1) = c;

			Matrix<float> rot_pts = rotate_mat * orgpts;
			m_filter_bank[count] = makeFilter(rot_pts,scalex[scale_index],0,1,sup);
			m_filter_bank[count + nedge] = makeFilter(rot_pts,scalex[scale_index],0,2,sup);

			count++;
		}
	}

	int kernel_size[2]={sup,sup};
	Matrix<float> gaussian_filter = fspecial(EAGLEEYE_GAUSSIAN,kernel_size,1,double(10.0));
	normalizeFilter(gaussian_filter);
	m_filter_bank[nbar + nedge] = gaussian_filter;

	Matrix<float> log_filter = fspecial(EAGLEEYE_LOG,kernel_size,1,double(10.0));
	normalizeFilter(log_filter);
	m_filter_bank[nbar + nedge + 1] = log_filter;
}

void TextonDescriptorExtractor::setFilterSize(int size)
{
	if (size % 2)
	{
		m_filter_size = size;
	}
	else
	{
		m_filter_size = size + 1;
	}
}

void TextonDescriptorExtractor::normalizeFilter(Matrix<float>& filter)
{
	int rows = filter.rows();
	int cols = filter.cols();

	float mean_val = mean(filter);
	float sum_val = 0;

	for (int i = 0;i < rows; ++i)
	{
		float* filter_data=filter.row(i);
		for (int j = 0; j< cols; ++j)
		{
			filter_data[j] = filter_data[j] - mean_val;
			sum_val += abs(filter_data[j]);
		}
	}

	for (int i = 0;i < rows; ++i)
	{
		float* filter_data = filter.row(i);
		for (int j = 0; j < cols; ++j)
		{
			filter_data[j] = filter_data[j] / sum_val;
		}
	}
}

Matrix<float> TextonDescriptorExtractor::gauss1d(const Matrix<float>& x, float sigma_val, float mean_val, int ord)
{
	int rows = x.rows();
	int cols = x.cols();

	Matrix<float> g(rows,cols);
	
	float variance = sigma_val * sigma_val;
	float denom = 2.0f * variance;
	for (int i = 0; i < rows; ++i)
	{
		const float* x_data = x.row(i);
		float* g_data = g.row(i);
		for (int j = 0; j<cols; ++j)
		{
			g_data[j] = exp(-x_data[j] * x_data[j] / denom) / sqrt(EAGLEEYE_PI * denom);

			switch(ord)
			{
			case 1:
				g_data[j] = -g_data[j] * (x_data[j] / variance);
				break;
			case 2:
				g_data[j] = g_data[j] * ((x_data[j] * x_data[j] - variance) / (variance * variance));
				break;
			}
		}
	}

	return g;
}

int TextonDescriptorExtractor::descriptorSize()
{
	switch(m_filter_bank_type)
	{
	case LM_FILTER_BANK:
		{
			return 48;
		}
	case S_FILTER_BANK:
		{
			return 13;
		}
	case MR_FILTER_BANK:
		{
			return 38;
		}
	}

	return 0;
}

void TextonDescriptorExtractor::setUnitPara(MemoryBlock param_block)
{
	_Parameter* me_param_block = (_Parameter*)(param_block.block());
	setFilterBankType(me_param_block->m_filter_bank_type);
	setFilterSize(me_param_block->m_filter_size);
}
void TextonDescriptorExtractor::getUnitPara(MemoryBlock& param_block)
{
	param_block = MemoryBlock(sizeof(_Parameter));
	_Parameter* me_param_block = (_Parameter*)(param_block.block());
	me_param_block->m_filter_bank_type = m_filter_bank_type;
	me_param_block->m_filter_size = m_filter_size;
}

}