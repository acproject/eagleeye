#include "ObjectDetSymbol.h"
#include "MatrixMath.h"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
DetSymbolInfo ObjectDetSymbol::m_symbol_info;
ObjectDetSymbol::ObjectDetSymbol(const char* name,SymbolType type)
	:Symbol(name,type)
{
	m_anchor_r = 0;
	m_anchor_c = 0;
	m_anchor_level = 0;

	m_symbol_info.x = 0;
	m_symbol_info.y = 0;
	m_symbol_info.level = 0;
	m_symbol_info.ds = 0;
	m_symbol_info.val = 0;

	m_invalid_label = 255;
}

ObjectDetSymbol::~ObjectDetSymbol()
{

}

void ObjectDetSymbol::setAnchor(int anchor_r,int anchor_c,int anchor_level)
{
	m_anchor_r = anchor_r;
	m_anchor_c = anchor_c;
	m_anchor_level = anchor_level;
}
void ObjectDetSymbol::getAnchor(int& anchor_r,int& anchor_c,int& anchor_level)
{
	anchor_r = m_anchor_r;
	anchor_c = m_anchor_c;
	anchor_level = m_anchor_level;
}

Matrix<unsigned char> ObjectDetSymbol::getSuperpixelCentersLabel(const Matrix<int>& superpixel_img,int superpixel_num,const Matrix<unsigned char>& label_annotation)
{
	Matrix<unsigned char> superpixel_centers_label(superpixel_num,1,int(m_invalid_label));
	std::vector<int> count_record(superpixel_num,0);

	int rows = superpixel_img.rows();
	int cols = superpixel_img.cols();

	std::vector<std::map<unsigned char,int>> superpixel_statistics;
	superpixel_statistics.resize(superpixel_num);

	//gain the pixel number of every segmentation
	for (int i = 0; i < rows; ++i)
	{
		const int* superpixel_img_data = superpixel_img.row(i);
		for (int j = 0; j < cols; ++j)
		{
			count_record[superpixel_img_data[j]] = count_record[superpixel_img_data[j]] + 1;

			unsigned char label = label_annotation.at(i,j);
			superpixel_statistics[superpixel_img_data[j]][label] = 
				superpixel_statistics[superpixel_img_data[j]][label] + 1;
		}
	}

	for (int index = 0; index < superpixel_num; ++index)
	{
		std::map<unsigned char,int> my = superpixel_statistics[index];
		std::map<unsigned char,int>::iterator iter,iend(my.end());
		int max_num = 0;
		for (iter = my.begin(); iter != iend; ++iter)
		{
			if (max_num < iter->second)
			{
				max_num = iter->second;
				superpixel_centers_label(index,0) = iter->first;
			}
		}
	}
	return superpixel_centers_label;
}

//using for detecting
void ObjectDetSymbol::squeezeInvalidSuperpixel(const Matrix<int>& superpixel_img,
												const int superpixel_num, 
												const Matrix<unsigned char>& img, 
												const unsigned char predict_gray_threshold, 
												Matrix<int>& after_superpixel_img, 
												int& after_superpixel_num, 
												Matrix<int>& after_pnum_in_superpixel, 
												Matrix<unsigned char>& predict_superpixel_label)
{
	//image size
	int rows = img.rows();
	int cols = img.cols();

	//valid and invalid pixel
	//pixels number of every superpixel
	Matrix<int> pnum_in_superpixel(superpixel_num,1,int(0));
	Matrix<int> invalid_pnum_in_superpixel(superpixel_num,1,int(0));
	for (int i = 0; i < rows; ++i)
	{
		const int* index_data = superpixel_img.row(i);
		const unsigned char* img_data = img.row(i);
		for (int j = 0; j < cols; ++j)
		{
			pnum_in_superpixel[index_data[j]] = pnum_in_superpixel[index_data[j]] + 1;

			if (img_data[j] > predict_gray_threshold)
			{
				invalid_pnum_in_superpixel[index_data[j]] = invalid_pnum_in_superpixel[index_data[j]] + 1;
			}
		}
	}

	//finding invalid superpixel coarsely
	std::vector<bool> invalid_superpixel_flag(superpixel_num,false);
	for (int i = 0; i < rows; ++i)
	{
		const int* index_data = superpixel_img.row(i);
		for (int j = 0; j <cols; ++j)
		{
			if(invalid_pnum_in_superpixel[index_data[j]] > pnum_in_superpixel[index_data[j]] / 2.0f)
				invalid_superpixel_flag[index_data[j]] = true;
		}
	}

	//squeeze all invalid superpixel
	Matrix<int> temp_mat = Matrix<int>(rows,cols);
	temp_mat.copy(superpixel_img);
	for (int i = 0; i < superpixel_num; ++i)
	{
		if (invalid_superpixel_flag[i] == true)
		{
			//'i' is invalid superpixel
			//it's very slow
			squeezeRegion(temp_mat,int(i),10,after_superpixel_img);
			temp_mat.copy(after_superpixel_img);
		}
	}

	putToMatlab(after_superpixel_img,"after");

	if (after_superpixel_img.isempty())
	{
		after_superpixel_img = temp_mat;
	}

	//superpixel number isn't changed
	after_superpixel_num = superpixel_num;

	//we don't predict any info
	predict_superpixel_label = Matrix<unsigned char>(after_superpixel_num,1,unsigned char(0));
	for (int i = 0; i < after_superpixel_num; ++i)
	{
		predict_superpixel_label[i] = 0;
	}
}

//using for training
void ObjectDetSymbol::squeezeInvalidSuperpixel(const Matrix<int>& superpixel_img, 
												const int superpixel_num,
												const Matrix<unsigned char>& label_annotation, 
												Matrix<int>& after_superpixel_img,
												int& after_superpixel_num, 
												Matrix<int>& after_pnum_in_superpixel, 
												Matrix<unsigned char>& after_superpixel_label)
{
	//image size
	int rows = superpixel_img.rows();
	int cols = superpixel_img.cols();

	//get superpixel label of every superpixel
	Matrix<unsigned char> superpixel_center_label = getSuperpixelCentersLabel(superpixel_img,superpixel_num,label_annotation);

	//finding invalid superpixel
	std::vector<bool> invalid_superpixel_index(superpixel_num,false);
	for (int i = 0; i < superpixel_num; ++i)
	{
		if (superpixel_center_label[i] == m_invalid_label)
			invalid_superpixel_index[i] = true;
	}

	Matrix<int> temp_mat = Matrix<int>(rows,cols);
	temp_mat.copy(superpixel_img);
	//squeeze all invalid superpixel
	for (int i = 0; i < superpixel_num; ++i)
	{
		if (invalid_superpixel_index[i] == true)
		{
			//'i' is invalid superpixel
			//it's very slow
			squeezeRegion(temp_mat,int(i),10,after_superpixel_img);
			temp_mat.copy(after_superpixel_img);
		}
	}

	//after_superpixel_img = superpixel_img
	if (after_superpixel_img.isempty())
	{
		after_superpixel_img = temp_mat;
	}

	//superpixel number isn't changed
	after_superpixel_num = superpixel_num;

	//finding superpixel label of every superpixel
	after_superpixel_label = getSuperpixelCentersLabel(after_superpixel_img,after_superpixel_num,label_annotation);

	//get the pixel number of every segmentation
	after_pnum_in_superpixel = Matrix<int>(after_superpixel_num,1,int(0));
	for (int i = 0; i < rows; ++i)
	{
		int* index_data = after_superpixel_img.row(i);
		for (int j = 0; j < cols; ++j)
		{
			after_pnum_in_superpixel[index_data[j]] = after_pnum_in_superpixel[index_data[j]] + 1;
		}
	}
}

}
