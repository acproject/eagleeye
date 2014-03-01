#include "HOGFeatureFunctions.h"
#include "cuda/MatrixMath.cuh"
#include <math.h>
#include <omp.h>

namespace eagleeye
{
//////////////////////////////////////////////////////////////////////////
// unit vectors used to compute gradient orientation
// 
float uu[9] = {1.0000f, 
	0.9397f, 
	0.7660f, 
	0.500f, 
	0.1736f, 
	-0.1736f, 
	-0.5000f, 
	-0.7660f, 
	-0.9397f};
float vv[9] = {0.0000f, 
	0.3420f, 
	0.6428f, 
	0.8660f, 
	0.9848f, 
	0.9848f, 
	0.8660f, 
	0.6428f, 
	0.3420f};

//////////////////////////////////////////////////////////////////////////

Matrix<HOG32Vec> generateHOG32Features(const Matrix<float>& img,int sbin)
{
	// memory for caching orientation histograms & their norms
	int dims[2];
	dims[0] = img.cols();
	dims[1] = img.rows();

	int blocks[2];
	blocks[0] = (int)round((float)dims[0] / (float)sbin);
	blocks[1] = (int)round((float)dims[1] / (float)sbin);

	float *hist = (float *)malloc(blocks[0] * blocks[1] * 18 * sizeof(float));
	memset(hist,0,blocks[0] * blocks[1] * 18 * sizeof(float));
	float *norm = (float *)malloc(blocks[0] * blocks[1] * sizeof(float));
	memset(norm,0,blocks[0] * blocks[1] * sizeof(float));

	// memory for HOG features
	int out[3];
	out[0] = eagleeye_max(blocks[0] - 2, 0);
	out[1] = eagleeye_max(blocks[1] - 2, 0);
	out[2] = 27 + 4 + 1;

	Matrix<HOG32Vec> feat_m(out[1],out[0]);
	feat_m.setzeros();
	float* feat = feat_m.dataptr()->data;

	int visible[2];
	visible[0] = blocks[0] * sbin;
	visible[1] = blocks[1] * sbin;

	for (int r = 1; r < visible[1] - 1; r++) 
	{
		for (int c = 1; c < visible[0] - 1; c++) 
		{
			// first color channel
// 				const float *s=img.anyptr((((eagleeye_min(r, dims[1]-2))*dims[0]) +(eagleeye_min(c, dims[0]-2))));
/*				float dx = *(s+1) - *(s-1);*/
/*				float dy = *(s+dims[0]) - *(s-dims[0]);*/
			int used_r=eagleeye_min(r, dims[1]-2);
			int used_c=eagleeye_min(c, dims[0]-2);
			float dx=img.at(used_r,used_c+1)-img.at(used_r,used_c-1);
			float dy=img.at(used_r+1,c)-img.at(used_r-1,c);
			float v = dx*dx + dy*dy;

			// snap to one of 18 orientations
			float best_dot = 0;
			int best_o = 0;
			for (int o = 0; o < 9; o++) 
			{
				//float dot = uu[o]*dx + vv[o]*dy;
				float dot=uu[o]*dy+vv[o]*dx;
				if (dot > best_dot) 
				{
					best_dot = dot;
					best_o = o;
				} else if (-dot > best_dot) 
				{
					best_dot = -dot;
					best_o = o+9;
				}
			}

			// add to 4 histograms around pixel using linear interpolation
			float xp = ((float)r+0.5f)/(float)sbin - 0.5f;
			float yp = ((float)c+0.5f)/(float)sbin - 0.5f;
			int ixp = (int)floor(xp);
			int iyp = (int)floor(yp);
			float vx0 = xp-ixp;
			float vy0 = yp-iyp;
			float vx1 = 1.0f-vx0;
			float vy1 = 1.0f-vy0;
			v = sqrt(v);

			if (ixp >= 0 && iyp >= 0) 
			{
				*(hist + (ixp*blocks[0] + iyp)*18+best_o) += 
					vx1*vy1*v;
			}

			if (ixp+1 < blocks[1] && iyp >= 0) 
			{
				*(hist + ((ixp+1)*blocks[0] + iyp)*18 + best_o) += 
					vx0*vy1*v;
			}

			if (ixp >= 0 && iyp+1 < blocks[0]) 
			{
				*(hist + (ixp*blocks[0] + (iyp+1))*18 + best_o) += 
					vx1*vy0*v;
			}

			if (ixp+1 < blocks[1] && iyp+1 < blocks[0]) 
			{
				*(hist + ((ixp+1)*blocks[0] + (iyp+1))*18 + best_o) += 
					vx0*vy0*v;
			}
		}
	}

	// compute energy in each block by summing over orientations
	for (int o=0;o<9;++o)
	{
		for (int i=0;i<blocks[1];++i)
		{
			for (int j=0;j<blocks[0];++j)
			{
				float* src1=hist+(i*blocks[0]+j)*18+o;
				float* src2=src1+9;

				float* dst=norm+i*blocks[0]+j;
				(*dst)+=(*src1 + *src2)*(*src1 + *src2);
			}
		}
	}


	// compute features
	for (int r = 0; r < out[1]; r++) 
	{
		for (int c = 0; c < out[0]; c++) 
		{
			float *dst = feat + (r*out[0] + c)*out[2];
			float *src, *p, n1, n2, n3, n4;

			p = norm + (r+1)*blocks[0] + c+1;
			n1 = 1.0f / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + float(eagleeye_eps));
			p = norm + (r+1)*blocks[0] + c;
			n2 = 1.0f / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + float(eagleeye_eps));
			p = norm + r*blocks[0] + c+1;
			n3 = 1.0f / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + float(eagleeye_eps));
			p = norm + r*blocks[0] + c;      
			n4 = 1.0f / sqrt(*p + *(p+1) + *(p+blocks[0]) + *(p+blocks[0]+1) + float(eagleeye_eps));

			float t1 = 0;
			float t2 = 0;
			float t3 = 0;
			float t4 = 0;

			// contrast-sensitive features
			src = hist + ((r+1)*blocks[0] + (c+1))*18;

			for (int o = 0; o < 18; o++) 
			{
				float h1 = eagleeye_min((*src) * n1, 0.2f);
				float h2 = eagleeye_min((*src) * n2, 0.2f);
				float h3 = eagleeye_min((*src) * n3, 0.2f);
				float h4 = eagleeye_min((*src) * n4, 0.2f);
				*dst = 0.5f * (h1 + h2 + h3 + h4);
				t1 += h1;
				t2 += h2;
				t3 += h3;
				t4 += h4;

				dst +=1;
				src +=1;
			}

			// contrast-insensitive features
			src = hist + ((r+1)*blocks[0] + (c+1))*18;
			for (int o = 0; o < 9; o++) 
			{
				float sum = *src + *(src + 9);
				float h1 = eagleeye_min(sum * n1, 0.2f);
				float h2 = eagleeye_min(sum * n2, 0.2f);
				float h3 = eagleeye_min(sum * n3, 0.2f);
				float h4 = eagleeye_min(sum * n4, 0.2f);
				*dst = 0.5f * (h1 + h2 + h3 + h4);

				dst +=1;
				src +=1;
			}

			// texture features
			*dst = t1*0.2357f;
			dst += 1;
			*dst = t2*0.2357f;
			dst += 1;
			*dst = t3*0.2357f;
			dst += 1;
			*dst = t4*0.2357f;

			// truncation feature
			dst += 1;
			*dst = 0;
		}
	}

	free(hist);
	free(norm);
	return feat_m;
}

HOG32Pyramid generateHOG32Pyramid(const Matrix<float>& img,int interval,int sbin,int padx/* =0 */,int pady/* =0 */)
{
	float sc=pow(2,1.0f/float(interval));
	// 	int rows_num=img.rows();
	// 	int cols_num=img.cols();
	// 	int min_num=min(rows_num,cols_num);
	// 
	// 	int max_scale=1+floor(log(float(min_num)/float(5*sbin))/log(sc));
	// 	int pyramid_levels=max_scale+interval;

	//we should extend some pad

	int pyramid_levels=interval*2;
	HOG32Pyramid hog_pyramid;
	hog_pyramid.create(pyramid_levels);
	hog_pyramid.padx=padx;
	hog_pyramid.pady=pady;
	hog_pyramid.interval=interval;

	for (int i=0;i<interval;++i)
	{
		float scaled_value=1.0f/pow(sc,i);
		Matrix<float> scaled_img=resize(img,scaled_value);

		//"first" 2x interval
		Matrix<HOG32Vec> feat=generateHOG32Features(scaled_img,sbin/2);
		hog_pyramid[i]=feat;
		hog_pyramid.scales(i)=scaled_value*2;

		//"second" 2x interval
		feat=generateHOG32Features(scaled_img,sbin);
		hog_pyramid[i+interval]=feat;
		hog_pyramid.scales(i+interval)=scaled_value;
	}

	return hog_pyramid;
}

Matrix<float> convWithHOG32Features(const Matrix<HOG32Vec>& hog_feat,const Matrix<HOG32Vec>& hog_filter)
{
	//feature size
	int feat_rows=hog_feat.rows();
	int feat_cols=hog_feat.cols();

	//filter size
	int filter_rows=hog_filter.rows();
	int filter_cols=hog_filter.cols();

	Array<int,2> pad_size;
	pad_size[0]=filter_rows;
	pad_size[1]=filter_cols;

	//ext hog feature
	Matrix<HOG32Vec> ext_hog_feat=padArray(hog_feat,pad_size,HOG32Vec(0),Post);
	int ext_hog_feat_rows=int(ext_hog_feat.rows());
	int ext_hog_feat_cols=int(ext_hog_feat.cols());
	const HOG32Vec* ext_feat_data=ext_hog_feat.dataptr();

	//hog filter
	Matrix<HOG32Vec> used_hog_filter=hog_filter;
	used_hog_filter.clone();
	const HOG32Vec* hog_filter_data=used_hog_filter.dataptr();

	//ext result matrix
	Matrix<float> dst(feat_rows,feat_cols);
	float* dst_data=dst.dataptr();

	//traverse the whole image
	//This may be too slow
	//This design is very poor. There exists some waste.
	int total_indexs=feat_rows*feat_cols;
	int inner_total_indexs=filter_rows*filter_cols;

	omp_set_num_threads(16);
#pragma omp parallel for
	for (int index=0;index<total_indexs;++index)
	{
		int r_index=int(index/feat_cols);
		int c_index=int(index-r_index*feat_cols);

		float val=0;

		for (int inner_index=0;inner_index<inner_total_indexs;++inner_index)
		{
			int rp=int(inner_index/filter_cols);
			int cp=int(inner_index-rp*filter_cols);

			const float* ext_feat_d=(ext_feat_data+((r_index+rp)*ext_hog_feat_cols+(c_index+cp)))->data;
			const float* filter_d=(hog_filter_data+inner_index)->data;

			val+=ext_feat_d[0]*filter_d[0];
			val+=ext_feat_d[1]*filter_d[1];
			val+=ext_feat_d[2]*filter_d[2];
			val+=ext_feat_d[3]*filter_d[3];
			val+=ext_feat_d[4]*filter_d[4];
			val+=ext_feat_d[5]*filter_d[5];
			val+=ext_feat_d[6]*filter_d[6];
			val+=ext_feat_d[7]*filter_d[7];
			val+=ext_feat_d[8]*filter_d[8];
			val+=ext_feat_d[9]*filter_d[9];

			val+=ext_feat_d[10]*filter_d[10];
			val+=ext_feat_d[11]*filter_d[11];
			val+=ext_feat_d[12]*filter_d[12];
			val+=ext_feat_d[13]*filter_d[13];
			val+=ext_feat_d[14]*filter_d[14];
			val+=ext_feat_d[15]*filter_d[15];
			val+=ext_feat_d[16]*filter_d[16];
			val+=ext_feat_d[17]*filter_d[17];
			val+=ext_feat_d[18]*filter_d[18];
			val+=ext_feat_d[19]*filter_d[19];


			val+=ext_feat_d[20]*filter_d[20];
			val+=ext_feat_d[21]*filter_d[21];
			val+=ext_feat_d[22]*filter_d[22];
			val+=ext_feat_d[23]*filter_d[23];
			val+=ext_feat_d[24]*filter_d[24];
			val+=ext_feat_d[25]*filter_d[25];
			val+=ext_feat_d[26]*filter_d[26];
			val+=ext_feat_d[27]*filter_d[27];
			val+=ext_feat_d[28]*filter_d[28];
			val+=ext_feat_d[29]*filter_d[29];

			val+=ext_feat_d[30]*filter_d[30];
			val+=ext_feat_d[31]*filter_d[31];
		}

		dst_data[index]=val;
	}

	return dst;
}

bool overlapDetect(int bbox[4],int detector_win[2],int detector_range[2],float overlap,float scale/* =1 */)
{
	int range_rows=detector_range[1];
	int range_cols=detector_range[0];

	int win_rows=detector_win[1];
	int win_cols=detector_win[0];

	int box_left_bottom_r=bbox[2];
	int box_left_bottom_c=bbox[0];

	int box_right_top_r=bbox[3];
	int box_right_top_c=bbox[1];

	for (int i=0;i<range_rows;++i)
	{
		for (int j=0;j<range_cols;++j)
		{
			//Under the original image coordinate
			int left_bottom_r=int(floor(i*scale));
			int left_bottom_c=int(floor(j*scale));

			int right_top_r=left_bottom_r+int(floor(win_rows*scale));
			int right_top_c=left_bottom_c+int(floor(win_cols*scale));

			//intersection of the detector with bbox
			int intersection_left_bottom_r=eagleeye_max(left_bottom_r,box_left_bottom_r);
			int intersection_left_bottom_c=eagleeye_max(left_bottom_c,box_left_bottom_c);

			int intersection_right_top_r=eagleeye_min(right_top_r,box_right_top_r);
			int intersection_right_top_c=eagleeye_min(right_top_c,box_right_top_c);

			//compute width and height of every intersection box
			int inter_width=intersection_right_top_c-intersection_left_bottom_c+1;
			int inter_height=intersection_right_top_r-intersection_left_bottom_r+1;

			//the area of the intersection box
			int inter_area=inter_width*inter_height;

			//a = area of (possibly clipped) detection windows
			int a=(right_top_c-left_bottom_c+1)*(right_top_r-left_bottom_r+1);

			//b = area of bbox
			int b=(box_right_top_c-box_left_bottom_c+1)*(box_right_top_r-box_left_bottom_r+1);

			//intersection over union overlap
			float value=float(inter_area)/float(a+b-inter_area);

			if (value>overlap)
			{
				return true;
			}
		}
	}

	return false;
}

std::vector<int> getValidLevels(const HOG32Pyramid& pyramid,const int sbin,int bbox[4],int detector_win[2],float overlap)
{
	int pyramid_levels_num=pyramid.levels();

	std::vector<int> valid_levels;

	for (int i=0;i<pyramid_levels_num;++i)
	{
		int rows=pyramid[i].rows();
		int cols=pyramid[i].cols();

		int detector_range[2]={cols,rows};
		float used_scale=float(sbin)/pyramid.scales(i);


		if (overlapDetect(bbox,detector_win,detector_range,overlap,used_scale))
		{
			valid_levels.push_back(i);
		}
	}

	return valid_levels;
}

std::vector<int> getValidLevels(const HOG32Pyramid& pyramid,
	const int sbin,
	int start_level,int end_level,
	int bbox[4],int detector_win[2],float overlap)
{
	std::vector<int> valid_levels;
	for (int i=start_level;i<=end_level;++i)
	{
		int rows=pyramid[i].rows();
		int cols=pyramid[i].cols();

		int detector_range[2]={cols,rows};
		float used_scale=float(sbin)/pyramid.scales(i);

		if (overlapDetect(bbox,detector_win,detector_range,overlap,used_scale))
		{
			valid_levels.push_back(i);
		}
	}

	return valid_levels;
}
}
