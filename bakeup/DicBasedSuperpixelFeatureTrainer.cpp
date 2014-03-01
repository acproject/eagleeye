#include "DicBasedSuperpixelFeatureTrainer.h"
#include "ImageReadNode.h"
#include "Learning/dictionary/Dictionary.h"
#include "SLICSuperPixelNode.h"
#include "EagleeyeIO.h"
#include "Matlab/MatlabInterface.h"
#include "ProcessNode/GridSuperPixelNode.h"
#include "ProcessNode/SRMNode.h"

namespace eagleeye
{
DicBasedSuperpixelFeatureTrainer::DicBasedSuperpixelFeatureTrainer()
{
	m_bow_descriptor = NULL;
	
	m_superpixel_complexity = 50.0f;
	m_sampling_num_of_every_superpixel = 2000;
	m_superpixel_area_limit = 40000;

	//set input port number
	setNumberOfInputSignals(2);

	//set the number of output port
	setNumberOfOutputSignals(2);
	setOutputPort(makeOutputSignal(),OUTPUT_PORT_SAMPLES_FEATURE_INFO);
	setOutputPort(makeOutputSignal(),OUTPUT_PORT_SAMPLES_FEATURE_LABEL_INFO);
}
DicBasedSuperpixelFeatureTrainer::~DicBasedSuperpixelFeatureTrainer()
{

}

void DicBasedSuperpixelFeatureTrainer::train()
{
	//generate classification model
	parseImageAnnotationTrainingSamples();

	//save feature samples and feature label data
 	IO::write(m_samples_description,m_trainer_folder + m_trainer_name + "_features" + m_ext_name,WRITE_BINARY_MODE);
 	IO::write(m_samples_label,m_trainer_folder + m_trainer_name + "_label" + m_ext_name,WRITE_BINARY_MODE);
 
	//write label info
	InfoSignal<std::string>* output_signal_label_info = TO_INFO_SIGNAL(std::string,getOutputPort(OUTPUT_PORT_SAMPLES_FEATURE_LABEL_INFO));
	m_trainer_label_info = m_trainer_name + "_label";
	output_signal_label_info->info = &m_trainer_label_info;

	//write feature info
	InfoSignal<std::string>* output_signal_feature_info = TO_INFO_SIGNAL(std::string,getOutputPort(OUTPUT_PORT_SAMPLES_FEATURE_INFO));
	m_trainer_feature_info = m_trainer_name + "_features";
	output_signal_feature_info->info = &m_trainer_feature_info;
}

void DicBasedSuperpixelFeatureTrainer::parseImageAnnotationTrainingSamples()
{
	//get parse file from port 0
	setParseFile(TO_INFO(std::string,getInputPort(INPUT_PORT_PARSE_FILE_INFO)) + m_ext_name);

	//get dictionary file from port 1
	std::string dic_file_name = TO_INFO(std::string,getInputPort(INPUT_PORT_DIC_FILE_INFO)) + m_ext_name;

	//load dictionary data from file
	Matrix<float> dic = Dictionary::loadDictionary((m_trainer_folder + dic_file_name).c_str());
	m_bow_descriptor->setVocabulary(dic);

	//load samples files and annotation files
	std::map<std::string,std::string> samples_and_annotation;
	std::map<std::string,Array<int,4>> samples_regions;
	std::map<std::string,int> samples_label;
	int labels_num;
	loadSamplesFile(samples_and_annotation,samples_regions,samples_label,labels_num);

	//store description matrix temporarily
	std::map<std::string,std::vector<Matrix<float>>> description_store_of_every_img;

	//read image node
	ImageReadNode<ImageSignal<unsigned char>> img_read_node;

	//read annotation image node(segmentation)
	ImageReadNode<ImageSignal<unsigned char>> annotation_read_node;

	std::map<std::string,std::string>::iterator f_iter,f_iend(samples_and_annotation.end());
	for (f_iter = samples_and_annotation.begin(); f_iter != f_iend; ++f_iter)
	{
		if (!(f_iter->first.empty()) && !(f_iter->second.empty()))
		{
			//initialize this temporary variable
			description_store_of_every_img[f_iter->first].resize(labels_num);

			//read sample image
			std::string sample_file_name = f_iter->first;
			img_read_node.setFilePath((m_trainer_folder + sample_file_name).c_str());
			img_read_node.start();
			Matrix<unsigned char> sample_img = img_read_node.getImage();

			Array<int,4> object_region = samples_regions[sample_file_name];

			Matrix<unsigned char> local_sample_img = sample_img(Range(object_region[2],object_region[3]),Range(object_region[0],object_region[1]));
			local_sample_img.clone();

			//read sample annotation image
			std::string annotation_file_name = f_iter->second;
			annotation_read_node.setFilePath((m_trainer_folder + annotation_file_name).c_str());		
			annotation_read_node.start();
			Matrix<unsigned char> annotation_img = annotation_read_node.getImage();

			Matrix<unsigned char> local_annotation_img = annotation_img(Range(object_region[2],object_region[3]),Range(object_region[0],object_region[1]));
			local_annotation_img.clone();

// 			putToMatlab(local_sample_img,"local_sample_img");
// 			putToMatlab(local_annotation_img,"local_sample_anno");

			//resize img
			if (m_is_resize_process)
			{
				local_sample_img = resize(local_sample_img,m_scales[0],BILINEAR_INTERPOLATION);
				local_annotation_img = resize(local_annotation_img,m_scales[0],NEAREST_NEIGHBOR_INTERPOLATION);
			}

			//get rid of noise
			if (m_is_gaussian_process)
			{
				local_sample_img = gaussFilter(local_sample_img,m_kernel_size[0],m_kernel_size[1]);
			}

			//get superpixel description
			ImageSignal<unsigned char> img_sig(local_sample_img);
			
			//SRM superpixel node
			SRMNode<ImageSignal<unsigned char>> srm_superpixel_node;
			srm_superpixel_node.setInputPort(&img_sig);
			srm_superpixel_node.setDistributionComplexity(m_superpixel_complexity);
			srm_superpixel_node.enableOutputPort_LABEL_DATA();
			srm_superpixel_node.disableOutputPort_SEGMENTATION_IMAGE_DATA();
			srm_superpixel_node.start();

			Matrix<int> superpixel_index_img = srm_superpixel_node.getLabelMap();
			int superpixel_num = srm_superpixel_node.getLabelNum();

/*			putToMatlab(superpixel_index_img,"su_index_img");*/

			Matrix<int> after_superpixel;
			int after_superpixel_num;
			Matrix<int> after_pixel_num_of_every_superpixel;
			Matrix<unsigned char> after_superpixel_center_label;

			adjustSuperpixel(superpixel_index_img,superpixel_num,local_annotation_img,
				after_superpixel,after_superpixel_num,after_pixel_num_of_every_superpixel,after_superpixel_center_label);
			
			superpixel_index_img = after_superpixel;
			superpixel_num = after_superpixel_num;
			Matrix<unsigned char> superpixel_center_label = after_superpixel_center_label;
			Matrix<int> pixel_num_of_superpixel = after_pixel_num_of_every_superpixel;

// 			//using annotation as superpixel image
// 			int superpixel_num = 0;
// 			Matrix<int> superpixel_index_img = local_annotation_img;
// 			superpixel_index_img.clone();
// 			maptoOrder(superpixel_index_img,superpixel_num);
// //			putToMatlab(superpixel_index_img,"index_img");

//  			//get keyopints on this local image region
// 			//using random samples of every region as keypoints
// 			std::vector<std::vector<std::pair<int,int>>> region_coordinate_record(superpixel_num);
// 			int region_rows = superpixel_index_img.rows();
// 			int region_cols = superpixel_index_img.cols();
// 			for (int i = 0; i < region_rows; ++i)
// 			{
// 				int* region_index_val = superpixel_index_img.row(i);
// 				for (int j = 0; j < region_cols; ++j)
// 				{
// 					std::pair<int,int> pixel_coordinate;
// 					pixel_coordinate.first = i;
// 					pixel_coordinate.second = j;
// 
// 					region_coordinate_record[region_index_val[j]].push_back(pixel_coordinate);
// 				}
// 			}
// 			
// 			//generate random samples for every region
// 			std::vector<KeyPoint> keypoints;
// 			for (int i = 0; i < superpixel_num; ++i)
// 			{
// 				int region_pixel_num = region_coordinate_record[i].size();
// 				//area of region must be larger than m_superpixel_area_limit
// 				if ( region_pixel_num> m_superpixel_area_limit)
// 				{
// 					Variable<int> region_sample_random = Variable<int>::uniform(0,region_pixel_num - 1);
// 					
// 					int sample_count = 0;
// 					while(sample_count < m_sampling_num_of_every_superpixel)
// 					{
// 						int sample_index = region_sample_random.var();
// 						KeyPoint region_point;
// 						region_point.pt[0] = float(region_coordinate_record[i][sample_index].second);
// 						region_point.pt[1] = float(region_coordinate_record[i][sample_index].first);
// 
// 						keypoints.push_back(region_point);
// 
// 						sample_count++;
// 					}
// 				}
// 			}

			Matrix<float> local_img_gradient_mag = computeGradientMag(local_sample_img);
			//get rid of some edges gradient magnitude
			local_img_gradient_mag(Range(0,3),Range(0,local_img_gradient_mag.cols())).setval(0.0f);
			local_img_gradient_mag(Range(local_img_gradient_mag.rows()-3,local_img_gradient_mag.rows()),Range(0,local_img_gradient_mag.cols())).setval(0.0f);
			local_img_gradient_mag(Range(0,local_img_gradient_mag.rows()),Range(0,3)).setval(0.0f);
			local_img_gradient_mag(Range(0,local_img_gradient_mag.rows()),Range(local_img_gradient_mag.cols()-3,local_img_gradient_mag.cols())).setval(0.0f);
			
// 			putToMatlab(local_img_gradient_mag,"gradient_mag");
// 			putToMatlab(superpixel_index_img,"superpixel_img");
			
			//perhaps we should using some function to control gradient magnitude
			
 			//get samples from every superpixel except for invalid superpixel
			std::vector<DynamicArray<int>> discrete_data_of_superpixel(superpixel_num);
			std::vector<DynamicArray<float>> distribution_of_superpixel(superpixel_num);
			std::vector<int> my_count(superpixel_num,0);
			std::vector<float> cumulative_weight(superpixel_num,0.0f);

			//build dynamic array
			for (int superpixel_index = 0; superpixel_index < superpixel_num; ++superpixel_index)
			{
				discrete_data_of_superpixel[superpixel_index] = DynamicArray<int>(pixel_num_of_superpixel[superpixel_index]);
				distribution_of_superpixel[superpixel_index] = DynamicArray<float>(pixel_num_of_superpixel[superpixel_index]);
			}

			int rows = local_sample_img.rows();
			int cols = local_sample_img.cols();
			for (int i = 0; i < rows; ++i)
			{
				int* superpixel_index_img_data = superpixel_index_img.row(i);
				float* local_img_gradient_mag_data = local_img_gradient_mag.row(i);
				for (int j = 0; j < cols; ++j)
				{
					int superpixel_index = superpixel_index_img_data[j];
					discrete_data_of_superpixel[superpixel_index][my_count[superpixel_index]] = i * cols + j;
					distribution_of_superpixel[superpixel_index][my_count[superpixel_index]] = local_img_gradient_mag_data[j];
					cumulative_weight[superpixel_index] = cumulative_weight[superpixel_index] + local_img_gradient_mag_data[j];
				
					my_count[superpixel_index] = my_count[superpixel_index] + 1;
				}
			}

			for (int superpixel_index = 0; superpixel_index < superpixel_num; ++superpixel_index)
			{
				if (cumulative_weight[superpixel_index] == 0.0f)
				{
					superpixel_center_label[superpixel_index] = m_invalid_label;
				}
				else
				{
					for (int m = 0; m < my_count[superpixel_index]; ++m)
					{
						distribution_of_superpixel[superpixel_index][m] /= cumulative_weight[superpixel_index];
					}
				}
			}

			//get samples from every superpixel
			std::vector<KeyPoint> keypoints;
			for (int superpixel_index = 0; superpixel_index < superpixel_num; ++superpixel_index)
			{
				if (superpixel_center_label[superpixel_index] != unsigned char(m_invalid_label))
				{
					DynamicArray<int> discrete_data = discrete_data_of_superpixel[superpixel_index];
					DynamicArray<float> distribution = distribution_of_superpixel[superpixel_index];
					Variable<int> local_var = Variable<int>::discreteDis(discrete_data,distribution);

					int sampling_count = 0;
					int sampling_total = pixel_num_of_superpixel[superpixel_index] / 2;
	
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

			Matrix<float> superpixel_description;
			std::vector<bool> superpixel_flag;
 			m_bow_descriptor->compute(local_sample_img.transform<float>(),superpixel_index_img,superpixel_num,keypoints,superpixel_description,superpixel_flag);

			//get the description number of every label
			std::vector<int> des_num_of_every_label;
			des_num_of_every_label.resize(labels_num,int(0));
			for (int superpixel_index = 0; superpixel_index < superpixel_num; ++superpixel_index)
			{

				int superpixel_label_index = superpixel_center_label.at(superpixel_index,0);
				if (superpixel_label_index != m_invalid_label && superpixel_flag[superpixel_index])
				{
					des_num_of_every_label[superpixel_label_index - 1]++;
				}
			}

			for (int index = 0; index < labels_num; ++index)
			{
				if(des_num_of_every_label[index] > 0)
				{
					description_store_of_every_img[f_iter->first][index] = Matrix<float>(des_num_of_every_label[index],m_bow_descriptor->descriptorSize());
				}
			}

			std::vector<int> count_record;
			count_record.resize(labels_num,int(0));
			for (int superpixel_index = 0; superpixel_index < superpixel_num; ++superpixel_index)
			{			
				int superpixel_label_index = superpixel_center_label(superpixel_index,0);
				if (superpixel_label_index != m_invalid_label && superpixel_flag[superpixel_index])
				{
					float* temp_pos_ptr = description_store_of_every_img[f_iter->first][superpixel_label_index - 1].row(count_record[superpixel_label_index - 1]);
					float* pos_ptr = superpixel_description.row(superpixel_index);
					memcpy(temp_pos_ptr,pos_ptr,sizeof(float) * m_bow_descriptor->descriptorSize());

					count_record[superpixel_label_index - 1]++;
				}
			}
		}
	}

	//merge all samples description into one matrix
	std::vector<int> samples_num_of_every_label;
	samples_num_of_every_label.resize(labels_num,int(0));
	int total_samples_num = 0;

	std::map<std::string,std::vector<Matrix<float>>>::iterator d_iter,d_iend(description_store_of_every_img.end());
	for (d_iter = description_store_of_every_img.begin(); d_iter != d_iend; ++d_iter)
	{
		std::vector<Matrix<float>> temp_samples_of_every_label = d_iter->second;
		for (int i = 0; i < labels_num; ++i)
		{
			samples_num_of_every_label[i] += temp_samples_of_every_label[i].rows();
			total_samples_num += temp_samples_of_every_label[i].rows();
		}
	}
	
	std::vector<int> pos_offset;
	pos_offset.resize(labels_num,int(0));
	for (int i = 1; i < labels_num; ++i)
	{
		pos_offset[i] += (samples_num_of_every_label[i-1] + pos_offset[i-1]);
	}

	//initialize matrix space
	m_samples_description = Matrix<float>(total_samples_num,m_bow_descriptor->descriptorSize());
	m_samples_label = Matrix<float>(total_samples_num,1);
	
	std::vector<int> samples_count_of_every_label;
	samples_count_of_every_label.resize(labels_num,int(0));

	//fill value
	for (d_iter = description_store_of_every_img.begin(); d_iter != d_iend; ++d_iter)
	{
		std::vector<Matrix<float>> temp_samples_of_every_label = d_iter->second;
		for (int i = 0; i < labels_num; ++i)
		{
			float* description_pos_ptr = m_samples_description.row(pos_offset[i] + samples_count_of_every_label[i]);
			float* pos_ptr = temp_samples_of_every_label[i].row(0);
			memcpy(description_pos_ptr,pos_ptr,sizeof(float)*temp_samples_of_every_label[i].rows()*m_bow_descriptor->descriptorSize());

			samples_count_of_every_label[i] += temp_samples_of_every_label[i].rows();
		}
	}

	for (int i = 0; i < labels_num - 1; ++i)
	{
		for (int offset_i = pos_offset[i]; offset_i < pos_offset[i+1]; ++offset_i)
		{
			m_samples_label.at(offset_i,0) = float(i);
		}
	}

	for (int i = pos_offset[labels_num - 1]; i < total_samples_num; ++i)
	{
		m_samples_label.at(i,0) = float(labels_num - 1);
	}
}

bool DicBasedSuperpixelFeatureTrainer::isNeedProcessed()
{
	if (IO::isexist(m_trainer_folder + m_trainer_name + "_label" + m_ext_name) && 
		IO::isexist(m_trainer_folder + m_trainer_name + "_features" + m_ext_name))
	{
		//check whether classification parameters file has been built
		EAGLEEYE_INFO("features and labels files have been existed \n");

		//write label info
		InfoSignal<std::string>* output_signal_label_info = TO_INFO_SIGNAL(std::string,getOutputPort(OUTPUT_PORT_SAMPLES_FEATURE_LABEL_INFO));
		m_trainer_label_info = m_trainer_name + "_label";
		output_signal_label_info->info = &m_trainer_label_info;

		//write feature info
		InfoSignal<std::string>* output_signal_feature_info = TO_INFO_SIGNAL(std::string,getOutputPort(OUTPUT_PORT_SAMPLES_FEATURE_INFO));
		m_trainer_feature_info = m_trainer_name + "_features";
		output_signal_feature_info->info = &m_trainer_feature_info;

		return false;
	}
	else
	{
		return true;
	}
}

bool DicBasedSuperpixelFeatureTrainer::selfcheck()
{
	if (!m_bow_descriptor)
	{
		EAGLEEYE_ERROR("sorry, please define descriptor\n");
		return false;
	}

	if (!TO_INFO_SIGNAL(std::string,getInputPort(INPUT_PORT_PARSE_FILE_INFO)))
	{
		EAGLEEYE_ERROR("sorry, don't set a proper input port (parse file)\n");
		return false;
	}

	if (!TO_INFO_SIGNAL(std::string,getInputPort(INPUT_PORT_DIC_FILE_INFO)))
	{
		EAGLEEYE_ERROR("sorry, don't set a proper input port (dictionary file)");
		return false;
	}

	return true;
}

// void DicBasedSuperpixelFeatureTrainer::setFeatureDescriptor(DescriptorExtractor* descriptor)
// {
// 	m_bow_descriptor = descriptor;
// }

void DicBasedSuperpixelFeatureTrainer::setSuperpixelComplexity(float superpixel_complexity)
{
	m_superpixel_complexity = superpixel_complexity;
}

void DicBasedSuperpixelFeatureTrainer::clearSomething()
{
	//clear all temporary variables
	m_samples_description = Matrix<float>();
	m_samples_label = Matrix<float>();
}

Matrix<unsigned char> DicBasedSuperpixelFeatureTrainer::getSuperpixelCentersLabel(const Matrix<int>& superpixel_label,int superpixel_num,const Matrix<unsigned char>& label_annotation)
{
	Matrix<unsigned char> segmentation_centers_label(superpixel_num,1,int(m_invalid_label));
	std::vector<int> count_record(superpixel_num,0);

	int rows = superpixel_label.rows();
	int cols = superpixel_label.cols();

	std::vector<std::map<unsigned char,int>> superpixel_statistics;
	superpixel_statistics.resize(superpixel_num);

	//gain the pixel number of every segmentation
	for (int i = 0; i < rows; ++i)
	{
		const int* row_data = superpixel_label.row(i);
		for (int j = 0; j < cols; ++j)
		{
			count_record[row_data[j]] = count_record[row_data[j]] + 1;

			unsigned char label = label_annotation.at(i,j);
			superpixel_statistics[row_data[j]][label] = superpixel_statistics[row_data[j]][label] + 1;
		}
	}

	for (int superpixel_index = 0; superpixel_index < superpixel_num; ++superpixel_index)
	{
		std::map<unsigned char,int> my = superpixel_statistics[superpixel_index];
		std::map<unsigned char,int>::iterator iter,iend(my.end());
		int max_num = 0;
		for (iter = my.begin(); iter != iend; ++iter)
		{
			if (max_num < iter->second)
			{
				max_num = iter->second;
				segmentation_centers_label(superpixel_index,0) = iter->first;
			}
		}
	}
	return segmentation_centers_label;
}

void DicBasedSuperpixelFeatureTrainer::setSamplingNumOfEverySuperpixel(int num)
{
	m_sampling_num_of_every_superpixel = num;
}

void DicBasedSuperpixelFeatureTrainer::setSuperpixelAreaLimit(int min_area)
{
	m_superpixel_area_limit = min_area;
}

void DicBasedSuperpixelFeatureTrainer::adjustSuperpixel(const Matrix<int>& superpixel,
														const int superpixel_num,
														const Matrix<unsigned char>& label_annotation,
														Matrix<int>& after_superpixel,int& after_superpixel_num,
														Matrix<int>& after_pixel_num_of_every_superpixel,
														Matrix<unsigned char>& after_superpixel_center_label)
{
	int rows = superpixel.rows();
	int cols = superpixel.cols();
	
	after_superpixel = Matrix<int>(rows,cols,int(0));

	//get superpixel label of every superpixel
	Matrix<unsigned char> superpixel_center_label = getSuperpixelCentersLabel(superpixel,superpixel_num,label_annotation);

	//merge all invalid superpixel
	for (int i = 0; i < rows; ++i)
	{
		const int* superpixel_data = superpixel.row(i);
		int* after_superpixel_data = after_superpixel.row(i);
		for (int j = 0; j <cols; ++j)
		{
			if(superpixel_center_label[superpixel_data[j]] == m_invalid_label)
				after_superpixel_data[j] = -1;
			else
				after_superpixel_data[j] = superpixel_data[j];
		}
	}

	//force valid superpixel to squeeze invalid superpixel
 	after_superpixel = squeezeRegion(after_superpixel,int(-1),10);

	//reassign superpixel index
	maptoOrder(after_superpixel,after_superpixel_num);

	//finding superpixel label of every superpixel
	after_superpixel_center_label = getSuperpixelCentersLabel(after_superpixel,after_superpixel_num,label_annotation);

	//gain the pixel number of every segmentation
	after_pixel_num_of_every_superpixel = Matrix<int>(after_superpixel_num,1,int(0));
	for (int i = 0; i < rows; ++i)
	{
		int* row_data = after_superpixel.row(i);
		for (int j = 0; j < cols; ++j)
		{
			after_pixel_num_of_every_superpixel[row_data[j]] = after_pixel_num_of_every_superpixel[row_data[j]] + 1;
		}
	}
}

}