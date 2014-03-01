#include "ExtractFeatureTrainer.h"
#include "EagleeyeIO.h"
#include "ProcessNode/ImageReadNode.h"
#include "MatrixMath.h"
#include "EagleeyeCore.h"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
ExtractFeatureTrainer::ExtractFeatureTrainer()
{
	m_hold_symbol = NULL;

	//set the number of output port
	setNumberOfOutputSignals(2);
	setOutputPort(makeOutputSignal(),OUTPUT_PORT_SAMPLES_INFO);
	setOutputPort(makeOutputSignal(),OUTPUT_PORT_SAMPLES_WHITING_INFO);
}
ExtractFeatureTrainer::~ExtractFeatureTrainer()
{

}

void ExtractFeatureTrainer::setHoldSymbol(Symbol* hold_symbol)
{
	m_hold_symbol = hold_symbol;
}

void ExtractFeatureTrainer::train()
{
	//parse image
	parseImageInSuperpixelSpace();

	Matrix<float> whiting_mat;
	if (m_whiting_flag)
	{
		//whiting feature data
		//computing mean vector and variance vector
		Matrix<float> mean_vec = rowmean(m_samples_presentation);
		Matrix<float> var_vec = rowvar(m_samples_presentation);

		//executing whiting transform
		WhitingParam whiting_param;
		whiting_param.mean_vec = mean_vec;
		whiting_param.var_vec = var_vec;
		m_samples_presentation = featureNormalize(m_samples_presentation,WHITEING,&whiting_param);
		whiting_mat = Matrix<float>(2,mean_vec.cols());
		whiting_mat(Range(0,1),Range(0,whiting_mat.cols())).copy(mean_vec);
		whiting_mat(Range(1,2),Range(0,whiting_mat.cols())).copy(var_vec);
	}

	//save whiting matrix
	if (!whiting_mat.isempty())
	{
		EagleeyeIO::write(whiting_mat,m_trainer_folder + m_trainer_name + "_whiting" + m_ext_name,WRITE_BINARY_MODE);
	}

	Matrix<float> samples_mat(m_samples_presentation.rows(),m_samples_presentation.cols() + 1);
	samples_mat(Range(0,samples_mat.rows()),Range(0,1)).copy(m_samples_label);
	samples_mat(Range(0,samples_mat.rows()),Range(1,samples_mat.cols())).copy(m_samples_presentation);

	//save feature samples and feature label data
	EagleeyeIO::write(samples_mat,m_trainer_folder + m_trainer_name + "_samples" + m_ext_name,WRITE_BINARY_MODE);

	//write samples info
	InfoSignal<std::string>* output_signal_samples_info = TO_INFO_SIGNAL(std::string,getOutputPort(OUTPUT_PORT_SAMPLES_INFO));
	m_trainer_samples_info = m_trainer_name + "_samples";
	output_signal_samples_info->info = &m_trainer_samples_info;

	//write samples whiting info
	InfoSignal<std::string>* output_signal_whiting_info = TO_INFO_SIGNAL(std::string,getOutputPort(OUTPUT_PORT_SAMPLES_WHITING_INFO));
	m_trainer_whiting_info = m_trainer_name + "_whiting";
	output_signal_whiting_info->info = &m_trainer_whiting_info;
}
bool ExtractFeatureTrainer::isNeedProcessed()
{
	if (EagleeyeIO::isexist(m_trainer_folder + m_trainer_name + "_samples" + m_ext_name))
	{
		//check whether classification parameters file has been built
		EAGLEEYE_INFO("samples info have been existed \n");

		//write samples info
		InfoSignal<std::string>* output_signal_samples_info = TO_INFO_SIGNAL(std::string,getOutputPort(OUTPUT_PORT_SAMPLES_INFO));
		m_trainer_samples_info = m_trainer_name + "_samples";
		output_signal_samples_info->info = &m_trainer_samples_info;

		//write samples whiting info
		InfoSignal<std::string>* output_signal_whiting_info = TO_INFO_SIGNAL(std::string,getOutputPort(OUTPUT_PORT_SAMPLES_WHITING_INFO));
		m_trainer_whiting_info = m_trainer_name + "_whiting";
		output_signal_whiting_info->info = &m_trainer_whiting_info;

		return false;
	}
	else
	{
		return true;
	}
}
bool ExtractFeatureTrainer::selfcheck()
{
	if (!m_hold_symbol)
	{
		EAGLEEYE_ERROR("sorry,symbol is NULL \n");
		return false;
	}

	if (!TO_INFO_SIGNAL(std::string,getInputPort(INPUT_PORT_PARSE_FILE_INFO)))
	{
		EAGLEEYE_ERROR("sorry, don't set a proper input port (parse file)\n");
		return false;
	}

	return true;
}
void ExtractFeatureTrainer::parseImageInSuperpixelSpace()
{
	//load samples files and annotation files
	std::map<std::string,std::string> samples_and_annotation;
	std::map<std::string,Array<int,4>> samples_regions;
	std::map<std::string,int> invalid_labels;
	int labels_num;
	loadObjectClassifyTrainingSamples(samples_and_annotation,samples_regions,invalid_labels,labels_num);

	//store description matrix temporarily
	std::map<std::string,std::vector<Matrix<float>>> description_store_of_every_img;

	int feature_dim = 0;

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
			
			//generate superpixel
			ImageSignal<unsigned char> img_sig(local_sample_img);			
			m_superpixel_generator->setInputPort(&img_sig);
			m_superpixel_generator->start();

			Matrix<int> superpixel_img = TO_IMAGE(int,m_superpixel_generator->getOutputPort(0));
			Matrix<int> superpixel_num = TO_IMAGE(int,m_superpixel_generator->getOutputPort(2));

			AuxiliaryInfoInSuperpixelSpace auxiliary_info;
			auxiliary_info.annotation_label = local_annotation_img;
			auxiliary_info.superpixel_pyr.create(1);
			auxiliary_info.superpixel_pyr[0] = superpixel_img;
			auxiliary_info.superpixel_num_pyr.resize(1);
			auxiliary_info.superpixel_num_pyr[0] = superpixel_num(0);

			m_hold_symbol->setUnitData(&local_sample_img,EAGLEEYE_POSITIVE_SAMPLE,&auxiliary_info);

			Matrix<float> unit_feature = m_hold_symbol->getUnitFeature(SUPERPIXEL_SPACE,INDEPENDENT_SEARCH);
			Matrix<float> sample_representation = unit_feature(Range(0,unit_feature.rows()),Range(SUPERPIXEL_FEATURE_DATA_OFFSET,unit_feature.cols()));
			Matrix<float> sample_label = unit_feature(Range(0,unit_feature.rows()),Range(SUPERPIXEL_FEATURE_DATA_OFFSET - 1,SUPERPIXEL_FEATURE_DATA_OFFSET));
			int feature_num = sample_representation.rows();
			feature_dim = sample_representation.cols();

			//get the description number of every label
			std::vector<int> description_num_of_every_label;
			description_num_of_every_label.resize(labels_num,int(0));
			for (int index = 0; index < feature_num; ++index)
			{
				int label = sample_label[index];
				if (label != m_invalid_label)
				{
					description_num_of_every_label[label]++;
				}
			}

			for (int index = 0; index < labels_num; ++index)
			{
				if(description_num_of_every_label[index] > 0)
				{
					description_store_of_every_img[f_iter->first][index] = Matrix<float>(description_num_of_every_label[index],sample_representation.cols());
				}
			}

			std::vector<int> count_record;
			count_record.resize(labels_num,int(0));
			for (int index = 0; index < feature_num; ++index)
			{			
				int label = sample_label[index];
				if (label != m_invalid_label)
				{
					float* temp_pos_ptr = description_store_of_every_img[f_iter->first][label].row(count_record[label]);
					float* pos_ptr = sample_representation.row(index);
					memcpy(temp_pos_ptr,pos_ptr,sizeof(float) * feature_dim);

					count_record[label]++;
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
		pos_offset[i] += (samples_num_of_every_label[i - 1] + pos_offset[i - 1]);
	}

	//initialize matrix space
	m_samples_presentation = Matrix<float>(total_samples_num,feature_dim);
	m_samples_label = Matrix<float>(total_samples_num,1);

	std::vector<int> samples_count_of_every_label;
	samples_count_of_every_label.resize(labels_num,int(0));

	//fill value
	for (d_iter = description_store_of_every_img.begin(); d_iter != d_iend; ++d_iter)
	{
		std::vector<Matrix<float>> temp_samples_of_every_label = d_iter->second;
		for (int i = 0; i < labels_num; ++i)
		{
			float* description_pos_ptr = m_samples_presentation.row(pos_offset[i] + samples_count_of_every_label[i]);
			float* pos_ptr = temp_samples_of_every_label[i].row(0);
			memcpy(description_pos_ptr,pos_ptr,sizeof(float) * temp_samples_of_every_label[i].rows() * feature_dim);

			samples_count_of_every_label[i] += temp_samples_of_every_label[i].rows();
		}
	}

	for (int i = 0; i < labels_num; ++i)
	{
		if (samples_count_of_every_label[i] > 0)
		{
			if (i != labels_num - 1)
			{
				for (int offset_i = pos_offset[i]; offset_i < pos_offset[i + 1]; ++offset_i)
				{
					m_samples_label[offset_i] = float(i);
				}
			}
			else
			{
				for (int offset_i = pos_offset[i]; offset_i < total_samples_num; ++offset_i)
				{
					m_samples_label[offset_i] = float(i);
				}
			}
		}
	}
}

void ExtractFeatureTrainer::clearSomething()
{
	//clear all temporary variables
	m_samples_presentation = Matrix<float>();
	m_samples_label = Matrix<float>();
}

void ExtractFeatureTrainer::setSuperpixelGenerator(AnyNode* superpixel_generator)
{
	m_superpixel_generator = superpixel_generator;
}

}