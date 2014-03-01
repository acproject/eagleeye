#include "MatrixMath.h"
#include <map>
#include <omp.h>
#include "Eigen/Eigen"
#include "opencv2/opencv.hpp"
#include "ProbabilityEstimator.h"
#include "ProbabilityDecision.h"
#include <math.h>
#include "EagleeyeCore.h"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
void fft1D(const Complex<double>* const t_data,Complex<double>* const f_data,unsigned int r)
{
	unsigned int count = 1<<r;

	Complex<double>* W = new Complex<double>[count/2];
	Complex<double>* X1 = new Complex<double>[count];
	Complex<double>* X2 = new Complex<double>[count];

	//compute the weight
	double angle;
	for (unsigned int i = 0; i < count / 2; ++i)
	{
		angle = -(double(i) * EAGLEEYE_PI * 2.0f / count);
		W[i] = Complex<double>(cos(angle),sin(angle));
	}

	//write value of t_data to X1
	memcpy(X1,t_data,sizeof(Complex<double>) * count);

	//use butterfly computation to compute FFT
	unsigned int bf_size;
	unsigned int p;
	Complex<double>* X;

	for (unsigned int k = 0; k < r; ++k)
	{
		for (unsigned int j = 0; j < unsigned int(1<<k); ++j)
		{
			bf_size = 1<<(r-k);
			for (unsigned int i = 0; i < bf_size / 2; ++i)
			{
				p = j * bf_size;
				X2[i + p] = X1[i + p] + X1[i + p + bf_size / 2];
				X2[i + p + bf_size / 2] = (X1[i + p] - X1[i + p + bf_size / 2]) * W[i * (1<<k)];
			}
		}

		X = X1;
		X1 = X2;
		X2 = X;
	}

	//sort again
	for (unsigned int j = 0; j < count; ++j)
	{
		p = 0;
		for (unsigned int i = 0; i < r; ++i)
		{
			if (j & (1<<i))
			{
				p += 1<<(r - i - 1);
			}
		}

		f_data[j] = X1[p];
	}

	delete []X1;
	delete []X2;
	delete []W;
}

void ifft1D(const Complex<double>* const f_data,Complex<double>* const t_data,unsigned int r)
{
	unsigned int count = 1<<r;

	//allocate some space
	Complex<double>* X = new Complex<double>[count];

	//write the f_data to X
	memcpy(X,f_data,sizeof(Complex<double>)*count);

	//compute conjugate
	for (unsigned int i = 0; i < count; ++i)
	{
		X[i] = Complex<double>(X[i].rd,-X[i].id);
	}

	fft1D(X,t_data,r);

	//compute conjugate in space field

	for (unsigned int i = 0; i < count; ++i)
	{
		t_data[i] = Complex<double>(t_data[i].rd / count,-t_data[i].id / count);
	}

	//free space
	delete []X;
}


Matrix<Complex<double>> ifft2D(Matrix<Complex<double>> m,unsigned int row_r,unsigned int col_r)
{
	unsigned int requested_row = unsigned int(pow(float(2),int(row_r)));
	unsigned int requested_col = unsigned int(pow(float(2),int(col_r)));
	Matrix<Complex<double>> frequency_data(requested_row,requested_col);
	frequency_data.setzeros();
	Complex<double>* frequency_data_ptr = frequency_data.dataptr();

	Matrix<Complex<double>> space_data(requested_row,requested_col);
	space_data.setzeros();
	Complex<double>* space_data_ptr = space_data.dataptr();


	//write data of m to space_data
	unsigned int min_row = EAGLEEYE_MIN(requested_row,m.rows());
	unsigned int min_col = EAGLEEYE_MIN(requested_col,m.cols());	

	for (unsigned int i = 0; i < min_row; ++i)
	{
		Complex<double>* r_ptr = m.row(i);
		Complex<double>* f_r_ptr = frequency_data_ptr + (i * requested_col);

		for (unsigned int j = 0; j < min_col; ++j)
		{
			f_r_ptr[j] = r_ptr[j];
		}
	}

	//implement ifft along the y direction(row)
	for (unsigned int i = 0; i < requested_row; ++i)
	{
		ifft1D(frequency_data_ptr + i * requested_col,space_data_ptr + i * requested_col,col_r);
	}

	for (unsigned int i = 0; i < requested_row; ++i)
	{
		for (unsigned int j = 0; j < requested_col; ++j)
		{
			frequency_data_ptr[i + requested_row * j] = space_data_ptr[j + requested_col * i];
		}
	}

	//implement ifft along the x direction(col)
	for (unsigned int i = 0; i < requested_col; ++i)
	{
		ifft1D(frequency_data_ptr + i * requested_row,space_data_ptr + i * requested_row,row_r);
	}
	
	Matrix<Complex<double>> result(requested_row,requested_col);
	for (unsigned int i = 0; i < requested_row; ++i)
	{
		Complex<double>* result_data_ptr = result.row(i);

		for (unsigned int j = 0; j < requested_col; ++j)
		{
			result_data_ptr[j] = space_data_ptr[j * requested_row + i];
		}
	}

	return result;
}

Matrix<float> fspecial(SpecialType ker,int kernel_size[2],int var_num,...)
{
	switch(ker)
	{
	case EAGLEEYE_AVERAGE:
		{
			//locate the start position of variable parameter list
			va_list arg_p;
			va_start(arg_p,var_num);

			double predefined_ave = 1;
			if (var_num > 0)
			{
				predefined_ave = va_arg(arg_p,double);
			}

			va_end(arg_p);
			
			int elements_num = kernel_size[0] * kernel_size[1];
			float element_val = float(predefined_ave) / float(elements_num);
			Matrix<float> special_kernel(kernel_size[1],kernel_size[0],element_val);

			return special_kernel;
		}
	case EAGLEEYE_GAUSSIAN:
		{
			//locate the start position of variable parameter list
			va_list arg_p;
			va_start(arg_p,var_num);

			//predefined_val[0]---- variance
			double val;
			if (var_num > 0)
			{
				val = va_arg(arg_p,double);
			}
			
			va_end(arg_p);

			float predefined_var = float(2.0 * val * val);
			
			Matrix<float> gauss_ker(kernel_size[1],kernel_size[0],float(0));
			int kernel_center_r = kernel_size[1] / 2;
			int kernel_center_c = kernel_size[0] / 2;

			float sum_val = 0.0f;
			for (int i = 0; i < kernel_size[1]; ++i)
			{
				for (int j = 0; j < kernel_size[0]; ++j)
				{
					float dis_r = float(i - kernel_center_r);
					float dis_c = float(j - kernel_center_c);

					gauss_ker(i,j) = exp(-(dis_r * dis_r + dis_c * dis_c) / predefined_var);
					sum_val += gauss_ker(i,j);
				}
			}

			//normalize to 1
			for (int i = 0; i < kernel_size[1]; ++i)
			{
				for (int j = 0; j < kernel_size[0]; ++j)
				{
					gauss_ker(i,j) = gauss_ker(i,j) / sum_val;
				}
			}

			return gauss_ker;
		}
	case EAGLEEYE_DISK:
		{
			return Matrix<float>();
		}
	case EAGLEEYE_LAPLACIAN:
		{
			//locate the start position of variable parameter list
			va_list arg_p;
			va_start(arg_p,var_num);

			double alpha = 0.2;
			
			if (var_num > 0)
			{
				alpha = va_arg(arg_p,double);
			}
			
			va_end(arg_p);
			
			float f_alpha = float(alpha);

			Matrix<float> lap(3,3,float(0));
			lap(0,0) = f_alpha / 4.0f;
			lap(0,1) = (1.0f - f_alpha) / 4.0f;
			lap(0,2) = f_alpha / 4.0f;
			lap(1,0) = (1.0f - f_alpha) / 4.0f;
			lap(1,1) = -1.0f;
			lap(1,2) = (1.0f - f_alpha) / 4.0f;
			lap(2,0) = f_alpha / 4.0f;
			lap(2,1) = (1.0f - f_alpha) / 4.0f;
			lap(2,2) = f_alpha / 4.0f;

			for (int i = 0; i < 3; ++i)
			{
				for (int j = 0; j < 3; ++j)
				{
					lap(i,j) = lap(i,j) * (4.0f / (f_alpha + 1.0f));
				}
			}

			return lap;
		}
	case EAGLEEYE_LOG:
		{
			//locate the start position of variable parameter list
			va_list arg_p;
			va_start(arg_p,var_num);

			//predefined_val[0]---- variance
			double val = 1.0;
			if (var_num > 0)
			{
				val = va_arg(arg_p,double);
			}

			va_end(arg_p);

			float predefined_var = float(2.0 * val * val);

			Matrix<float> log_ker(kernel_size[1],kernel_size[0],float(0));
			int kernel_center_r = kernel_size[1] / 2;
			int kernel_center_c = kernel_size[0] / 2;

			float sum_val = 0.0f;
			for (int i = 0; i < kernel_size[1]; ++i)
			{
				for (int j = 0; j < kernel_size[0]; ++j)
				{
					float dis_r = float(i - kernel_center_r);
					float dis_c = float(j - kernel_center_c);

					log_ker(i,j) = exp(-(dis_r * dis_r + dis_c * dis_c) / predefined_var);
					sum_val += log_ker(i,j);
				}
			}

			float temp = float(pow(val,6));
			temp = sum_val * 2.0f * EAGLEEYE_PI * temp;

			for (int i = 0; i < kernel_size[1]; ++i)
			{
				for (int j = 0; j < kernel_size[0]; ++j)
				{
					float dis_r = float(i - kernel_center_r);
					float dis_c = float(j - kernel_center_c);

					log_ker(i,j) = log_ker(i,j) * (dis_r * dis_r + dis_c * dis_c - predefined_var) / temp;
				}
			}

			return log_ker;
		}
	case EAGLEEYE_MOTION:
		{
			return Matrix<float>();
		}
	case EAGLEEYE_PREWITT:
		{
			//return the 3-by-3 filter h (shown below) that emphasizes
			//horizontal edges by approximating a vertical gradient.
			Matrix<float> prewitt(3,3,float(0.0f));
			/*[ 1  1  1 
				0  0  0 
				-1 -1 -1 ]*/
			prewitt(0,0) = 1.0f; prewitt(0,1) = 1.0f; prewitt(0,2) = 1.0f;
			prewitt(1,0) = 0.0f; prewitt(1,1) = 0.0f; prewitt(1,2) = 0.0f;
			prewitt(2,0) = -1.0f; prewitt(2,1) = -1.0f; prewitt(2,2) = -1.0f;

			return prewitt;
		}
	case  EAGLEEYE_SOBEL:
		{
			//returns a 3-by-3 filter h (shown below) that emphasizes
			//horizontal edges using the smoothing effect by approximating a vertical
			//gradient.
			Matrix<float> sobel_ker(3,3,float(0.0f));
			/*	1  2  1 
				0  0  0 
				-1 -2 -1*/
			sobel_ker(0,0) = 1.0f; sobel_ker(0,1) = 2.0f; sobel_ker(0,2) = 1.0f;
			sobel_ker(1,0) = 0.0f; sobel_ker(1,1) = 0.0f; sobel_ker(1,2) = 0.0f;
			sobel_ker(2,0) = -1.0f; sobel_ker(2,1) = -2.0f; sobel_ker(2,2) = -1.0f;
			return sobel_ker;
		}
	case EAGLEEYE_UNSHARP:
		{
			return Matrix<float>();
		}
	}


	return Matrix<float>();
}

void responseMax(const Matrix<float>& img,const Matrix<float>& kernel,float& max_res,int& r_index,int& c_index)
{
	int rows = img.rows();
	int cols = img.cols();

	Matrix<float> response = conv2DInSpace(img,kernel);
	r_index = 0;
	c_index = 0;
	max_res = AtomicTypeTrait<float>::minval();

	for (int i = 0; i < rows; ++i)
	{
		float* response_data = response.row(i);
		for (int j = 0; j < cols; ++j)
		{
			if (response_data[j] > max_res)
			{
				max_res = response_data[j];
				r_index = i;
				c_index = j;
			}
		}
	}
}

void meshgrid(int x_start,int x_end, int y_start, int y_end, 
			  Matrix<int>& x, Matrix<int>& y)
{
	x = Matrix<int>(x_end - x_start, y_end - y_start);
	y = Matrix<int>(x_end - x_start, y_end - y_start);

	for (int i = x_start; i < x_end; ++i)
	{
		for (int j = y_start; j < y_end; ++j)
		{
			x(i - x_start, j - y_start) = j;
			y(i - x_start, j - y_start) = (x_end -1) - (i - x_start);
		}
	}
}

Matrix<float> shiftPad(const Matrix<float>& img,float shift_x,float shift_y)
{
	int rows = img.rows();
	int cols = img.cols();

	Matrix<float> shift_img(rows,cols,float(0.0f));
	float* shift_img_data = shift_img.dataptr();

	Matrix<float> img_proxy(rows,cols);
	img_proxy.copy(img);
	float* img_proxy_data = img_proxy.dataptr();

	omp_set_num_threads(16);
#pragma omp parallel for
	for (int i = 0; i < rows; ++i)
	{
		int index = i * cols;
		for (int j = 0; j < cols; ++j)
		{
			int up_index,down_index,left_index,right_index;
			left_index = int(floor(j + shift_x));
			right_index = int(ceil(j + shift_x));
			up_index = int(floor(i + shift_y));
			down_index = int(ceil(i + shift_y));

			left_index = (left_index > 0) ? left_index: 0;
			left_index = (left_index < cols) ? left_index: (cols - 1);
			
			right_index = (right_index > 0) ? right_index: 0;
			right_index = (right_index < cols) ? right_index: (cols - 1);

			up_index = (up_index > 0) ? up_index: 0;
			up_index = (up_index < rows) ? up_index: (rows - 1);
			
			down_index = (down_index > 0) ? down_index: 0;
			down_index = (down_index < rows) ? down_index: (rows - 1);

			int up_left_bias,down_left_bias,up_right_bias,down_right_bias;
			up_left_bias = up_index * cols + left_index;
			up_right_bias = up_index * cols + right_index;
			down_left_bias = down_index * cols + left_index;
			down_right_bias = down_index * cols + right_index;

			float f1,f2,fp;
			if (right_index != left_index)
			{
				float f1_part1 = float(right_index - (j + shift_x)) / float(right_index - left_index) * img_proxy_data[down_left_bias];
				float f1_part2 = float((j + shift_x) - left_index) / float(right_index - left_index) * img_proxy_data[down_right_bias];
				f1 = f1_part1 + f1_part2;

				float f2_part1 = float(right_index - (j + shift_x)) / float(right_index - left_index) * img_proxy_data[up_left_bias];
				float f2_part2 = float((j + shift_x) - left_index) / float(right_index - left_index) * img_proxy_data[up_right_bias];
				f2 = f2_part1 + f2_part2;
			}
			else
			{
				f1 = img_proxy_data[down_left_bias];
				f2 = img_proxy_data[up_left_bias];
			}

			if (up_index != down_index)
			{
				float f_part1 = float(up_index - (i + shift_y)) / float(up_index - down_index) * f1;
				float f_part2 = float((i + shift_y) - down_index) / float(up_index - down_index) * f2;
				fp = f_part1 + f_part2;
			}
			else
			{
				fp = f1;
			}

			shift_img_data[index+j] = fp;
		}
	}

	return shift_img;
}

void maptoOrder(Matrix<int>& mess_to_order,int& num)
{
	int rows = mess_to_order.rows();
	int cols = mess_to_order.cols();			
	int n_labels = 0;
	num = 0;

	std::vector<int> labels_map(rows * cols, -1);
	
	std::map<int,int> order_map;
	for (int i = 0; i < rows; ++i)
	{
		int* mess_to_order_data = mess_to_order.row(i);
		for (int j = 0; j < cols; ++j)
		{
			int mess_label = mess_to_order_data[j];
			std::map<int,int>::iterator check_iter = order_map.find(mess_label);
			if (check_iter == order_map.end())
			{
				order_map[mess_label] = n_labels;
				n_labels++;
			}

			mess_to_order_data[j] = order_map[mess_label];
		}
	}

	num = n_labels;
}

inline float interp_linear(const float* xdata,const float* ydata,const float xi,int cols)
{
	if (xi <= xdata[0])	return ydata[0];
	if (xi >= xdata[cols-1]) return ydata[cols-1];

	int col_j = 0;
	for (int j = 0; j < cols-1; ++j)
	{
		if (xi >= xdata[j] && xi <= xdata[j+1])
		{
			col_j = j;
			break;
		}
	}

	return (ydata[col_j] - ydata[col_j+1]) * (xi - xdata[col_j]) / 
		(xdata[col_j] - xdata[col_j+1]) + ydata[col_j];
}

inline float interp_linear_inverse(const float* xdata,const float* ydata,const float xi,int cols)
{
	if(xi >= xdata[0]) return ydata[0];
	if(xi <= xdata[cols - 1]) return ydata[cols - 1];

	int col_j = 0;
	for (int j = 0; j < cols - 1; ++j)
	{
		if (xi <= xdata[j] && xi >= xdata[j + 1])
		{
			col_j = j;
			break;
		}
	}

	return (ydata[col_j] - ydata[col_j + 1]) * (xi - xdata[col_j]) / 
		(xdata[col_j] - xdata[col_j + 1]) + ydata[col_j];
}

void interp(const Matrix<float>& grid_index_vec,const Matrix<float>& grid_val_vec, 
			const Matrix<float>& index_vec,Matrix<float>& val_vec,InterpMethod interp_type,
			float a, float b)
{
	assert(grid_index_vec.rows() == 1 && grid_val_vec.rows() == 1);
	assert(index_vec.rows() == 1);
	assert(grid_index_vec.cols() ==  grid_val_vec.cols());

	int grid_cols = grid_val_vec.cols();
	val_vec = Matrix<float>(1,index_vec.cols(),float(0.0f));

	switch(interp_type)
	{
	case LINEAR_INTERPOLATION:
		{
			const float* x_data = grid_index_vec.row(0);
			const float* y_data = grid_val_vec.row(0);
			if (x_data[0] < x_data[1])
			{
				const float* xi_data = index_vec.row(0);
				int xi_cols = index_vec.cols();

				float* yi_data = val_vec.row(0);		
				for (int i = 0; i < xi_cols; ++i)
				{
					yi_data[i] = interp_linear(x_data,y_data,xi_data[i],grid_cols);
				}

			}
			else
			{
				const float* xi_data = index_vec.row(0);
				int xi_cols = index_vec.cols();

				float* yi_data = val_vec.row(0);		
				for (int i = 0; i < xi_cols; ++i)
				{
					yi_data[i] = interp_linear_inverse(x_data,y_data,xi_data[i],grid_cols);
				}
			}

			break;
		}
	case SPLINE_INTERPOLATION:
		{
			int grid_num = grid_index_vec.cols();

			//构建三弯矩方程组
// 			cv::Mat T(grid_num,grid_num,CV_32F,cv::Scalar(0));
// 			cv::Mat D(grid_num,1,CV_32F,cv::Scalar(0));
			Eigen::MatrixXf T(grid_num,grid_num);
			T.setZero();
			Eigen::VectorXf D(grid_num);
			D.setZero();

			float* h = new float[grid_num - 1];
			float* f = new float[grid_num - 1];
			memset(h,0,sizeof(float) * (grid_num - 1));
			memset(f,0,sizeof(float) * (grid_num - 1));

			for (int i = 1; i < grid_num; ++i)
			{
				h[i-1] = grid_index_vec.at(i) - grid_index_vec.at(i - 1);
				f[i-1]=(grid_val_vec.at(i) - grid_val_vec.at(i - 1)) / h[i - 1];
			}

			float* mu = new float[grid_num];
			float* lamda = new float[grid_num];
			float* d = new float[grid_num];

			//使用第一边界条件
			lamda[0] = 1.0f;
			d[0] = 6.0f * (f[0] - a) / h[0];
			mu[grid_num - 1] = 1.0f;
			d[grid_num - 1] = 6.0f * (b - f[grid_num - 2]) / h[grid_num - 2];

			for (int i = 1; i < grid_num - 1; ++i)
			{
				mu[i] = h[i - 1] / (h[i - 1] + h[i]);
				lamda[i] = 1 - mu[i];
				d[i] = 6.0f * (f[i] - f[i - 1]) * (1.0f / (h[i - 1] + h[i]));
			}

			for (int i = 1; i < grid_num - 1; ++i)
			{
				T(i,i-1) = mu[i];
				T(i,i) = 2.0f;
				T(i,i+1) = lamda[i];

				D(i) = d[i];
			}

			T(0,0) = 2.0f;
			T(0,1) = lamda[0];

			T(grid_num - 1,grid_num - 2) = mu[grid_num - 1];
			T(grid_num - 1,grid_num - 1) = 2;

			D(0) = d[0];
			D(grid_num - 1) = d[grid_num - 1];

			Eigen::VectorXf M = T.colPivHouseholderQr().solve(D);

			int xi_cols = index_vec.cols();
			for (int i = 0; i < xi_cols; ++i)
			{
				float index_val = index_vec.at(i);

				if (index_val < grid_index_vec.at(0))
				{
					val_vec.at(i) = grid_val_vec.at(0);
				}
				if (index_val > grid_index_vec.at(grid_index_vec.cols() - 1))
				{
					val_vec.at(i) = grid_val_vec.at(grid_index_vec.cols() - 1);
				}

				int index = 0;
				for (int g = 0; g < int(grid_index_vec.cols() - 1); ++g)
				{
					if (index_val >= grid_index_vec.at(g) && index_val <= grid_index_vec.at(g + 1))
					{
						index = g;
						break;
					}
				}

				val_vec.at(i) = M(index) * pow((grid_index_vec.at(index + 1) - index_val),3) / (6.0f * h[index]) + 
					M(index + 1) * (pow(index_val - grid_index_vec.at(index),3) / (6.0f * h[index])) + 
					(grid_val_vec.at(index) - M(index) * pow(h[index],2) / 6.0f) * (grid_index_vec.at(index + 1) - index_val) / h[index] + 
					(grid_val_vec.at(index + 1) - M(index + 1) * pow(h[index],2) / 6.0f) * ((index_val - grid_index_vec(index)) / h[index]);
			}

			delete []mu;
			delete []lamda;
			delete []d;

			delete []h;
			delete []f;

			break;
		}
	}
}

void qdmax(const Matrix<float>& x,const Matrix<float>& y,float& xm,float& ym)
{
	Eigen::Matrix3f eigen_v;
	for (int i = 0; i < 3; ++i)
	{
		float x_val = x.at(i);
		eigen_v(i,0) = 1;
		eigen_v(i,1) = x_val;
		eigen_v(i,2) = x_val * x_val;
	}

	Eigen::Vector3f eigen_y;
	eigen_y(0,0) = y.at(0); eigen_y(1,0) = y.at(1); eigen_y(2,0) = y.at(2);

	Eigen::Vector3f A = eigen_v.inverse() * eigen_y;
	
	xm = -A(1,0) / A(2,0) / 2.0f;
	
	Eigen::Matrix<float,1,3> d;
	d(0,0) = 1;
	d(0,1) = xm;
	d(0,2) = xm * xm;
	ym = d * A;
}

void convolution2D(Matrix<float>& src,Matrix<float>& kernel,Matrix<float>& response)
{
	//finding optimum dft size
	cv::Size dft_size;
	dft_size.width = cv::getOptimalDFTSize(src.cols() + kernel.cols() - 1);
	dft_size.height = cv::getOptimalDFTSize(src.rows() + kernel.rows() - 1);

	//kernel size
	int kernel_size_rows = kernel.rows();
	int kernel_size_cols = kernel.cols();
	
	//forward DFT for src
	cv::Mat cv_src(src.rows(),src.cols(),CV_32F,src.dataptr());
	cv::Mat cv_fre_src(dft_size,CV_32F,cv::Scalar::all(0));
	cv::Mat cv_roi_fre_src(cv_fre_src,cv::Rect(kernel_size_cols / 2, kernel_size_rows / 2,cv_src.cols,cv_src.rows));
	cv_src.copyTo(cv_roi_fre_src);
	cv::dft(cv_fre_src,cv_fre_src,0,cv_src.rows + kernel_size_rows / 2);

	//forward DFT for kernel
	cv::Mat cv_kernel(kernel.rows(),kernel.cols(),CV_32F,kernel.dataptr());
	cv::Mat cv_fre_kernel(dft_size,CV_32F,cv::Scalar::all(0));
	cv::Mat cv_roi_fre_kernel(cv_fre_kernel,cv::Rect(0,0,kernel_size_cols,kernel_size_rows));
	cv_kernel.copyTo(cv_roi_fre_kernel);
	cv::dft(cv_fre_kernel,cv_fre_kernel,0,kernel_size_rows);

	//multiply src frequency and kernel frequency
	cv::Mat cv_fre_response(dft_size,CV_32F,cv::Scalar::all(0));
	cv::mulSpectrums(cv_fre_src,cv_fre_kernel,cv_fre_response,0,true);

	//backward DFT for response frequency
	cv::dft(cv_fre_response,cv_fre_response,cv::DFT_INVERSE | cv::DFT_SCALE,cv_src.rows + kernel_size_rows - 1);
	
	Matrix<float> fre_response(cv_fre_response.rows,cv_fre_response.cols,cv_fre_response.data);
	response = fre_response(Range(0,cv_src.rows),Range(0,cv_src.cols));
	response.clone();
}
void convolution2DBank(Matrix<float>& src,std::vector<Matrix<float>>& kernel_bank,std::vector<Matrix<float>>& response_bank)
{
	//kernel number
	int kernel_bank_size = kernel_bank.size();

	//kernel size
	int kernel_size_rows = kernel_bank[0].rows();
	int kernel_size_cols = kernel_bank[0].cols();
	
	//create response bank
	response_bank.resize(kernel_bank_size);

	//find optimal DFT size
	cv::Size cv_dft_size;
	cv_dft_size.width = cv::getOptimalDFTSize(src.cols() + kernel_size_cols - 1);
	cv_dft_size.height = cv::getOptimalDFTSize(src.rows() + kernel_size_rows - 1);

	//forward DFT for src
	cv::Mat cv_src(src.rows(),src.cols(),CV_32F,src.dataptr());
	cv::Mat cv_fre_src(cv_dft_size,CV_32F,cv::Scalar::all(0));
	cv::Mat cv_roi_fre_src(cv_fre_src,cv::Rect(kernel_size_cols / 2, kernel_size_rows / 2,cv_src.cols,cv_src.rows));
	cv_src.copyTo(cv_roi_fre_src);

	cv::dft(cv_fre_src,cv_fre_src,0,cv_src.rows + kernel_size_rows / 2);

#pragma omp parallel for
	for (int i = 0; i < kernel_bank_size; ++i)
	{
		//forward DFT for kernel
		cv::Mat cv_kernel(kernel_bank[i].rows(),kernel_bank[i].cols(),CV_32F,kernel_bank[i].dataptr());
		cv::Mat cv_fre_kernel(cv_dft_size,CV_32F,cv::Scalar::all(0));
		cv::Mat cv_roi_fre_kernel(cv_fre_kernel,cv::Rect(0,0,kernel_size_cols,kernel_size_rows));
		cv_kernel.copyTo(cv_roi_fre_kernel);
		cv::dft(cv_fre_kernel,cv_fre_kernel,0,kernel_size_rows);
		
		//multiply src frequency and kernel frequency
		cv::Mat cv_fre_response(cv_dft_size,CV_32F,cv::Scalar::all(0));
		cv::mulSpectrums(cv_fre_src,cv_fre_kernel,cv_fre_response,0,true);
		
		//backward DFT for response
		cv::dft(cv_fre_response,cv_fre_response,cv::DFT_INVERSE | cv::DFT_SCALE,cv_src.rows + kernel_size_rows - 1);

		Matrix<float> fre_response(cv_fre_response.rows,cv_fre_response.cols,cv_fre_response.data);
		Matrix<float> roi_fre_response = fre_response(Range(0,cv_src.rows),Range(0,cv_src.cols));
		roi_fre_response.clone();

		response_bank[i] = roi_fre_response;
	}
}

Matrix<float> singleGaussianDecision(const Matrix<float>& positive_samples,
							 const Matrix<float>& negative_samples,
							 bool optimum_decision_flag,void* para)
{
	float* decision_param = (float*)para;

	SingleGaussianEstimator positive_gaussian_esitimator;
	positive_gaussian_esitimator.building(positive_samples);
	SingleGaussianEstimator negative_gaussian_estimator;
	negative_gaussian_estimator.building(negative_samples);
	
	ProbabilityDecision p_decision(&positive_gaussian_esitimator,&negative_gaussian_estimator,int(decision_param[0]));
	if (optimum_decision_flag)
	{
		Matrix<float> positive_decision_region = p_decision.decisionMinMisClassificationRate();
		return positive_decision_region;
	}
	else
	{
		Matrix<float> positive_decision_region = p_decision.decisionRejectOption(decision_param[1]);
		return positive_decision_region;
	}

	return Matrix<float>();
}
Matrix<float> gaussianKDEDecision(const Matrix<float>& positive_samples,
			   const Matrix<float>& negative_samples,
			   bool optimum_decision_flag,void* para)
{
	float* kde_param = (float*)para;
	GaussianKDE positive_gaussian_kde(kde_param[1]);
	positive_gaussian_kde.building(positive_samples);
	GaussianKDE negative_gaussian_kde(kde_param[2]);
	negative_gaussian_kde.building(negative_samples);

	ProbabilityDecision p_decision(&positive_gaussian_kde,&negative_gaussian_kde,int(kde_param[0]));
	if (optimum_decision_flag)
	{
		Matrix<float> positive_decision_region = p_decision.decisionMinMisClassificationRate();
		return positive_decision_region;
	}
	else
	{
		float reject_probability_threshold = kde_param[3];
		Matrix<float> positive_decision_region = p_decision.decisionRejectOption(reject_probability_threshold);
		return positive_decision_region;
	}

	return Matrix<float>();
}

Matrix<float> autoProbabilityDecision(const Matrix<float>& positive_samples,
					   const Matrix<float>& negative_samples, 
					   ProbabilityEstimatorModel p_m,
					   bool optimum_decision_flag,void* para)
{
	switch(p_m)
	{
	case SINGLE_GAUSSIAN_MODEL:
		{
			return singleGaussianDecision(positive_samples,negative_samples,optimum_decision_flag,para);
		}
	case GAUSSIAN_KDE_MODEL:
		{
			return gaussianKDEDecision(positive_samples,negative_samples,optimum_decision_flag,para);
		}
	}

	return Matrix<float>();
}

Matrix<unsigned char> canny(Matrix<unsigned char> img,double threshold1,double threshold2,int aperture_size,bool l2gradient)
{
	int rows = img.rows();
	int cols = img.cols();
	Matrix<unsigned char> edges(rows,cols);
	cv::Mat cv_img(rows,cols,CV_8U,img.dataptr());
	cv::Mat cv_edges(rows,cols,CV_8U,edges.dataptr());

	cv::Canny(cv::InputArray(cv_img),cv::OutputArray(cv_edges),threshold1,threshold2,aperture_size,l2gradient);

	return edges;
}

float kldiv(const Matrix<float>& p1,const Matrix<float>& p2,KLMode kl_mode /* = DEFAULT_KL */)
{
	//check
	assert(p1.rows() == 1 && p2.rows() == 1);
	assert(p1.cols() == p2.cols());

	int dis_num = p1.cols();
	const float* p1_ptr = p1.row(0);
	const float* p2_ptr = p2.row(0);

	switch(kl_mode)
	{
	case SYM_KL:
		{
//			KL1 = sum(pVect1 .* (log2(pVect1)-log2(pVect2)));
//			KL2 = sum(pVect2 .* (log2(pVect2)-log2(pVect1)));
//			KL = (KL1+KL2)/2;
			float kl_1 = 0.0f;
			float kl_2 = 0.0f;
			for (int i = 0; i < dis_num; ++i)
			{
				kl_1 += p1_ptr[i] * (log(p1_ptr[i]) - log(p2_ptr[i]));
				kl_2 += p2_ptr[i] * (log(p2_ptr[i]) - log(p1_ptr[i]));
			}
			float kl_val = (kl_1 + kl_2) / 2.0f / float(EAGLEEYE_LOG2);
			return kl_val;
		}
	case JS_KL:
		{
//			logQvect = log2((pVect2+pVect1)/2);
//			KL = .5 * (sum(pVect1.*(log2(pVect1)-logQvect)) + ...
//				sum(pVect2.*(log2(pVect2)-logQvect)));
			
			float kl_val = 0.0f;
			float log_q = 0.0f;
			for (int i = 0; i < dis_num; ++i)
			{
				log_q = log((p1_ptr[i] + p2_ptr[i]) / 2.0f);
				kl_val += (log(p1_ptr[i]) - log_q)* p1_ptr[i] + 
					(log(p2_ptr[i]) - log_q) * p2_ptr[i];
			}
			kl_val = kl_val / float(EAGLEEYE_LOG2) * 0.5f;
			return kl_val;
		}
	default:
		{
			float kl_val = 0.0f;
			for (int i = 0; i < dis_num; ++i)
			{
				kl_val += p1_ptr[i] * (log(p1_ptr[i]) - log(p2_ptr[i]));
			}
			kl_val /= float(EAGLEEYE_LOG2);
			return kl_val;
		}
	}
}
}
