#include "EagleeyeCore.h"
#include "Variable.h"
#include "MatrixMath.h"
#include "InfoTheory.h"
#include "Matlab/MatlabInterface.h"
#include "opencv2/core/core.hpp"
namespace eagleeye
{
void skmeans(const Matrix<float>& data,int k,Matrix<float>& centers,
			 Matrix<int>& centers_count)
{
	int data_num = data.rows();
	int data_dim = data.cols();

	if ((centers.rows() * centers.cols()) != 0)
	{
		//predefined centers
		//make sure that the dimension is same
		assert(data_dim == centers.cols());

		//make sure that the cluster number is same
		assert(k == centers.rows());
	}
	else
	{
		Variable<int> var = Variable<int>::uniform(0,data_num);
		centers = Matrix<float>(k,data_dim);
		for (int i = 0; i < k; ++i)
		{
			float* centers_data = centers.row(i);
			int random_row_index = var.var();
			const float* d = data.row(random_row_index);
			for (int j = 0; j < data_dim; ++j)
			{
				centers_data[j] = d[j];
			}
		}
	}

	if (centers_count.rows() * centers_count.cols() != 0)
	{
		assert(k == centers_count.rows());
	}
	else
	{
		//initialize zeros
		centers_count = Matrix<int>(k,1,int(0));
	}

	int* centers_count_data = centers_count.dataptr();

	for (int data_index = 0; data_index < data_num; ++data_index)
	{
		const float* d = data.row(data_index);

		//find the nearest center
		float nearest_center_dist = 1000000000.0f;
		int nearest_center_index = 0;
		for (int center_index = 0; center_index < k; ++center_index)
		{
			float* center_data = centers.row(center_index);
			float dist = 0.0f;
			for (int dim_index = 0; dim_index < data_dim; ++dim_index)
			{
				dist += (d[dim_index] - center_data[dim_index]) * (d[dim_index] - center_data[dim_index]);
			}
			if (dist < nearest_center_dist)
			{
				nearest_center_dist = dist;
				nearest_center_index = center_index;
			}
		}

		centers_count_data[nearest_center_index]++;
		float* center_data = centers.row(nearest_center_index);
		for (int dim_index = 0; dim_index < data_dim; ++dim_index)
		{
			center_data[dim_index] = center_data[dim_index] + (1.0f / centers_count_data[nearest_center_index]) * 
				(d[dim_index] - center_data[dim_index]);
		}
	}
}

void forgetful_skmeans(const Matrix<float>& data,int k,Matrix<float>& centers,float a)
{
	int data_num = data.rows();
	int data_dim = data.cols();

	if ((centers.rows() * centers.cols()) != 0)
	{
		//predefined centers
		//make sure that the dimension is same
		assert(data_dim == centers.cols());

		//make sure that the cluster number is same
		assert(k == centers.rows());
	}
	else
	{
		Variable<int> var = Variable<int>::uniform(0,data_num);
		centers = Matrix<float>(k,data_dim);
		for (int i = 0; i < k; ++i)
		{
			float* centers_data = centers.row(i);
			int random_row_index = var.var();
			const float* d = data.row(random_row_index);
			for (int j = 0; j < data_dim; ++j)
			{
				centers_data[j] = d[j];
			}
		}
	}

	for (int data_index = 0; data_index < data_num; ++data_index)
	{
		const float* d = data.row(data_index);

		//find the nearest center
		float nearest_center_dist = 1000000000.0f;
		int nearest_center_index = 0;
		for (int center_index = 0; center_index < k; ++center_index)
		{
			float* center_data = centers.row(center_index);
			float dist = 0.0f;
			for (int dim_index = 0; dim_index < data_dim; ++dim_index)
			{
				dist += (d[dim_index] - center_data[dim_index]) * (d[dim_index] - center_data[dim_index]);
			}
			if (dist < nearest_center_dist)
			{
				nearest_center_dist = dist;
				nearest_center_index = center_index;
			}
		}

		float* center_data = centers.row(nearest_center_index);
		for (int dim_index = 0; dim_index < data_dim; ++dim_index)
		{
			center_data[dim_index] = center_data[dim_index] + a * (d[dim_index] - center_data[dim_index]);
		}
	}
}

// 交换整数 a 、b 的值
inline void swap_int(int *a, int *b) 
{
	*a ^= *b;
	*b ^= *a;
	*a ^= *b;
}

// Bresenham's line algorithm
void drawLine(Matrix<ERGB>& img,int x1,int y1,int x2,int y2,ERGB c)
{
	// 参数 c 为颜色值
	int dx = abs(x2 - x1),
		dy = abs(y2 - y1),
		yy = 0;

	if (dx < dy) 
	{
		yy = 1;
		swap_int(&x1, &y1);
		swap_int(&x2, &y2);
		swap_int(&dx, &dy);
	}

	int ix = (x2 - x1) > 0 ? 1 : -1,
		iy = (y2 - y1) > 0 ? 1 : -1,
		cx = x1,
		cy = y1,
		n2dy = dy * 2,
		n2dydx = (dy - dx) * 2,
		d = dy * 2 - dx;

	if (yy) 
	{ // 如果直线与 x 轴的夹角大于 45 度
		while (cx != x2) 
		{
			if (d < 0) 
			{
				d += n2dy;
			} else 
			{
				cy += iy;
				d += n2dydx;
			}
			img.at(cx,cy)=c;
			cx += ix;
		}
	} 
	else 
	{ // 如果直线与 x 轴的夹角小于 45 度
		while (cx != x2) 
		{
			if (d < 0) 
			{
				d += n2dy;
			} else 
			{
				cy += iy;
				d += n2dydx;
			}
			img.at(cy,cx)=c;
			cx += ix;
		}
	}
}

inline void draw_circle_8(Matrix<ERGB>& img, int xc, int yc, int x, int y, ERGB c) 
{
	// 参数 c 为颜色值
	img.at(yc + y,xc + x)=c;
	img.at(yc + y,xc - x)=c;
	img.at(yc - y,xc + x)=c;
	img.at(yc - y,xc - x)=c;
	img.at(yc + x,xc + y)=c;
	img.at(yc + x,xc - y)=c;
	img.at(yc - x,xc + y)=c;
	img.at(yc - x,xc - y)=c;
}
void drawCircle(Matrix<ERGB>& img,int xc,int yc,int r,int fill,ERGB c)
{
	int rows=img.rows();
	int cols=img.cols();
	// (xc, yc) 为圆心，r 为半径
	// fill 为是否填充
	// c 为颜色值

	// 如果圆在图片可见区域外，直接退出
	if (xc + r < 0 || xc - r >= cols ||
		yc + r < 0 || yc - r >= rows) return;

	int x = 0, y = r, yi, d;
	d = 3 - 2 * r;

	if (fill) 
	{
		// 如果填充（画实心圆）
		while (x <= y) 
		{
			for (yi = x; yi <= y; yi ++)
				draw_circle_8(img, xc, yc, x, yi, c);

			if (d < 0) 
			{
				d = d + 4 * x + 6;
			} 
			else 
			{
				d = d + 4 * (x - y) + 10;
				y --;
			}
			x++;
		}
	} 
	else 
	{
		// 如果不填充（画空心圆）
		while (x <= y) 
		{
			draw_circle_8(img, xc, yc, x, y, c);

			if (d < 0) {
				d = d + 4 * x + 6;
			} else {
				d = d + 4 * (x - y) + 10;
				y --;
			}
			x ++;
		}
	}
}

void drawRect(Matrix<ERGB>& img,int x,int y ,int width,int height,ERGB c)
{
	int rows = img.rows();
	int cols = img.cols();
	//draw left_bottom to right_bottom
	ERGB* left_bottom_data = img.row(y + height - 1);
	for (int i = 0; i < width; ++i)
	{
		left_bottom_data[x + i] = c;
	}

	//draw right_top to right_bottom
	for (int i = 0; i < height; ++i)
	{
		img.row(y + i)[x + width - 1] = c;
	}

	//draw left_top to right_top
	ERGB* left_top_data = img.row(y);
	for (int i = 0; i < width; ++i)
	{
		left_top_data[x + i] = c;
	}

	//draw left_top to left_bottom
	for (int i = 0; i < height; ++i)
	{
		img.row(y + i)[x] = c;
	}
}

// Computes a gradient orientation histogram at a specified pixel
float calcOrientationHist( const Matrix<float>& img, const Array<float,2>& pt, int radius,
						  float sigma, float* hist, int n )
{
	int rows = img.rows();
	int cols = img.cols();

	int i, j, k, len = (radius*2+1)*(radius*2+1);

	float expf_scale = -1.f/(2.f * sigma * sigma);

	float* buf = new float[len*4 + n+4];
	float *X = buf, *Y = X + len, *Mag = X, *Ori = Y + len, *W = Ori + len;
	float* temphist = W + len + 2;

	for( i = 0; i < n; i++ )
		temphist[i] = 0.f;

	for( i = -radius, k = 0; i <= radius; i++ )
	{
		int y = int(pt[1]) + i;
		if( y <= 0 || y >= rows - 1 )
			continue;
		for( j = -radius; j <= radius; j++ )
		{
			int x = int(pt[0]) + j;
			if( x <= 0 || x >= cols - 1 )
				continue;

			float dx = (float)(img.at(y, x+1) - img.at(y, x-1));
			float dy = (float)(img.at(y-1, x) - img.at(y+1, x));

			X[k] = dx; Y[k] = dy; W[k] = (i*i + j*j)*expf_scale;
			k++;
		}
	}

	len = k;

	// compute gradient values, orientations and the weights over the pixel neighborhood
	cv::exp(W, W, len);
	cv::fastAtan2(Y, X, Ori, len, true);
	cv::magnitude(X, Y, Mag, len);

	for( k = 0; k < len; k++ )
	{
		int bin = round((n/360.f)*Ori[k]);
		if( bin >= n )
			bin -= n;
		if( bin < 0 )
			bin += n;
		temphist[bin] += W[k]*Mag[k];
	}

	// smooth the histogram
	temphist[-1] = temphist[n-1];
	temphist[-2] = temphist[n-2];
	temphist[n] = temphist[0];
	temphist[n+1] = temphist[1];
	for( i = 0; i < n; i++ )
	{
		hist[i] = (temphist[i-2] + temphist[i+2])*(1.f/16.f) +
			(temphist[i-1] + temphist[i+1])*(4.f/16.f) +
			temphist[i]*(6.f/16.f);
	}

	float maxval = hist[0];
	for( i = 1; i < n; i++ )
		maxval = eagleeye_max(maxval, hist[i]);

	//clear buffer
	delete buf;
	return maxval;
}

float calcMainDirection( const Matrix<float>& img, const Array<float,2>& pt,int radius,float sigma,int n)
{
	float* hist=new float[n];
	float threshold=calcOrientationHist(img,pt,radius,sigma,hist,n);

	threshold = threshold * 0.8f;

	float angle = 0.0f;

	for( int j = 0; j < n; j++ )
	{
		int l = j > 0 ? j - 1 : n - 1;
		int r2 = j < n-1 ? j + 1 : 0;

		if( hist[j] > hist[l]  &&  hist[j] > hist[r2]  &&  hist[j] >= threshold )
		{
			float bin = j + 0.5f * (hist[l]-hist[r2]) / (hist[l] - 2*hist[j] + hist[r2]);
			bin = bin < 0 ? n + bin : bin >= n ? bin - n : bin;
			angle = 360.f - (float)((360.f/n) * bin);
			if(std::abs(angle - 360.f) < eagleeye_eps)
				angle = 0.f;
		}
	}

	delete hist;

	return angle;
}

void drawKeypoints(Matrix<ERGB>& img,std::vector<KeyPoint> keypoints, Matrix<ERGB>& output_img, float ratio)
{
	int rows = img.rows();
	int cols = img.cols();

	output_img = Matrix<ERGB>(rows,cols);
	output_img.copy(img);

	Variable<unsigned char> random_color=Variable<unsigned char>::uniform(0,255);

	int keypoints_num=keypoints.size();
	for (int i = 0; i < keypoints_num; ++i)
	{
		int x = int(keypoints[i].pt[0]);
		int y = int(keypoints[i].pt[1]);
		float size = keypoints[i].size;
		float angle = keypoints[i].angle;

		//draw circle around keypoint
		ERGB c;
		c[0]=random_color.var();
		c[1]=random_color.var();
		c[2]=random_color.var();

		int radius=int(size*ratio);

		//draw keypoint position
		drawCircle(output_img,x,y,radius,0,c);

		//draw main direction
		int end_x = x + int((radius+1) * cos((angle/360.0f)*2*EAGLEEYE_PI));
		int end_y = y + int((radius+1) * sin((angle/360.0f)*2*EAGLEEYE_PI));

		drawLine(output_img,x,y,end_x,end_y,c);
	}
}

Matrix<float> feaatureWhiting(const Matrix<float>& data,void* whiting_param)
{
	Matrix<float> mean_vec;
	Matrix<float> var_vec;

	if (!whiting_param)
	{
		mean_vec = rowmean(data);
		var_vec = rowvar(data);
	}
	else
	{
		WhitingParam feature_whiting_param = *(WhitingParam*)whiting_param;
		mean_vec = feature_whiting_param.mean_vec;
		var_vec = feature_whiting_param.var_vec;
	}

	//finding mean of every dimension
	float* mean_vec_data = mean_vec.row(0);

	//finding variance of every dimension
	float* var_vec_data = var_vec.row(0);

	int rows = data.rows();
	int cols = data.cols();

	Matrix<float> whiting_data(rows,cols,0.0f);

	for (int i = 0; i < rows; ++i)
	{
		const float* data_ptr = data.row(i);
		float* whiting_data_ptr = whiting_data.row(i);
		for(int j = 0; j < cols; ++j)
		{
			if (var_vec_data[j] != 0.0f)
				whiting_data_ptr[j] = (data_ptr[j] - mean_vec_data[j]) / var_vec_data[j];
		}
	}

	return whiting_data;
}

Matrix<float> featureBinary(const Matrix<float>& data)
{
	int rows = data.rows();
	int cols = data.cols();

	Matrix<float> norm_data(rows,cols,0.0f);

	for (int i = 0; i < rows; ++i)
	{
		const float* data_ptr = data.row(i);
		float* norm_data_ptr = norm_data.row(i);
		for (int j = 0; j < cols; ++j)
		{
			if (data_ptr[j] > 0)
				norm_data_ptr[j] = 1.0f;
			else
				norm_data_ptr[j] = 0.0f;
		}
	}

	return norm_data;
}

//performance is very poor
//perhaps, we would optimize it in the future
Matrix<float> featureBinaryBasedOnMIM(const Matrix<float>& data,void* param)
{
	//const int
	const int bins = 100;

	//parameter
	MIMParam* mim_param = (MIMParam*)param;
	if (!mim_param)
		return Matrix<float>();

	//data number
	int data_num = data.rows();

	//feature dimension
	int feature_dim = data.cols();

	if (!mim_param->label_vec.isempty())
	{
		//finding optimum binary thresholds

		//check
		assert(mim_param->label_vec.rows() * mim_param->label_vec.cols() == data_num);

		//some temporary vector
		//feature and binary feature
		Matrix<float> feat(1,data_num);
		float* feat_ptr = feat.row(0);
		
		Matrix<float> binary_feat(1,data_num);
		float* binary_feat_ptr = binary_feat.row(0);

		//threshold vector
		Matrix<float> threshold_vec(1,bins,0.0f);
		float* threshold_vec_ptr = threshold_vec.row(0);

		//reshape label vector
		Matrix<float> label_vec = Matrix<float>(1,data_num,mim_param->label_vec.row(0));

		//search every feature
		for (int feature_index = 0; feature_index < feature_dim; ++feature_index)
		{
			//fill data
			for (int i = 0; i < data_num; ++i)
			{
				feat_ptr[i] = data(i,feature_index);
			}

			float min_val,max_val;
			getMaxMin(feat,max_val,min_val);
			float step_val = (max_val - min_val) / float(bins);
			threshold_vec_ptr[0] = min_val;

			for (int i = 1; i < bins; ++i)
			{
				threshold_vec_ptr[i] = threshold_vec_ptr[i - 1] + step_val;
			}

			//explore all possible thresholds
			float max_mu_val = 0.0f;
			float optimum_threshold_val = 0.0f;
			for (int i = 1; i < bins - 1; ++i) 
			{
				float threshld_val = threshold_vec_ptr[i];
				for (int n = 0; n < data_num; ++n)
				{
					if(feat_ptr[n] > threshld_val)
						binary_feat_ptr[n] = 1.0f;
					else
						binary_feat_ptr[n] = 0.0f;
				}

				//compute mutual information, finding max mutual information
				float mu_val = mi(binary_feat,label_vec);
				if (max_mu_val < mu_val)
				{
					max_mu_val = mu_val;
					optimum_threshold_val = threshld_val;
				}
			}

			//save optimum threshold
			mim_param->thre_vec(feature_index) = optimum_threshold_val;
		}
	}

	if (mim_param->thre_vec.isempty())
		return Matrix<float>();

	float* mim_thre_ptr = mim_param->thre_vec.row(0);
	Matrix<float> norm_feature(data_num,feature_dim,0.0f);
	for (int i = 0; i < data_num; ++i)
	{
		float* norm_feature_ptr = norm_feature.row(i);
		const float* data_ptr = data.row(i);
		for (int feature_index = 0; feature_index < feature_dim; ++feature_index)
		{
			if (data_ptr[feature_index] > mim_thre_ptr[feature_index])
				norm_feature_ptr[feature_index] = 1.0f;
			else
				norm_feature_ptr[feature_index] = 0.0f;
		}
	}

	return norm_feature;
}

Matrix<float> featureNormalize(const Matrix<float>& data,NormMode feat_norm,void* norm_param)
{
	switch(feat_norm)
	{
	case WHITEING:
		{
			return feaatureWhiting(data,norm_param);
		}
	case S_BINARY:
		{
			return featureBinary(data);
		}
	case MIM_BINARY:
		{
			return featureBinaryBasedOnMIM(data,norm_param);
		}
	default:
		{
			return Matrix<float>();
		}
	}
}
}