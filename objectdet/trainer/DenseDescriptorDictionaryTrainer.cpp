#include "DenseDescriptorDictionaryTrainer.h"
#include "ProcessNode/ImageReadNode.h"
#include "DenseFeatureDetector.h"
#include "Learning/dictionary/OnlineDictionary.h"
#include "EagleeyeIO.h"
#include "Matlab/MatlabInterface.h"
#include "MatrixMath.h"

namespace eagleeye
{
DenseDescriptorDictionaryTrainer::DenseDescriptorDictionaryTrainer()
{
	m_dictionary_capacity = 1000;

	m_init_scale = 1;
	m_scale_levels = 1;
	m_scale_mul = 1;
	m_init_xy_step = 50;
	m_init_img_bound = 10;
	m_search_radius = 10;
	m_vary_xy_step_with_scale = false;
	m_vary_img_bound_with_scale = false;
	m_is_needed_main_dir = false;

	m_descriptor = NULL;
}

DenseDescriptorDictionaryTrainer::~DenseDescriptorDictionaryTrainer()
{

}

void DenseDescriptorDictionaryTrainer::train()
{
	//parse training images
	parseImage();

	//write to output signal
	InfoSignal<std::string>* output_signal_info = TO_INFO_SIGNAL(std::string,getOutputPort(OUTPUT_PORT_DIC_FILE_INFO));
	m_trainer_info = m_trainer_name;
	output_signal_info->info = &m_trainer_info;

	EAGLEEYE_INFO("success to build dictionary (%s)\n",m_trainer_name.c_str());
}

void DenseDescriptorDictionaryTrainer::setDenseDetectParams(float init_scale,int scale_levels,float scale_mul,int init_xy_step,
						  int init_img_bound,int search_radius,
						  bool is_needed_main_dir,
						  bool vary_xy_step_with_scale,
						  bool vary_img_bound_with_scale)
{
	m_init_scale = init_scale;
	m_scale_levels = scale_levels;
	m_scale_mul = scale_mul;
	m_init_xy_step = init_xy_step;
	m_init_img_bound = init_img_bound;
	m_search_radius = search_radius;

	m_is_needed_main_dir = is_needed_main_dir;
	m_vary_xy_step_with_scale = vary_xy_step_with_scale;
	m_vary_img_bound_with_scale = vary_img_bound_with_scale;
}

bool DenseDescriptorDictionaryTrainer::selfcheck()
{
	if (!m_descriptor)
	{
		EAGLEEYE_ERROR("sorry, please descriptor extractor");
		return false;
	}

	if (!TO_INFO_SIGNAL(std::string,getInputPort(0)))
	{
		EAGLEEYE_ERROR("sorry,there isn't correct input port");
		return false;
	}

	return true;
}

bool DenseDescriptorDictionaryTrainer::isNeedProcessed()
{
	if(EagleeyeIO::isexist(m_trainer_folder + m_trainer_name + m_ext_name))
	{
		//dictionary has been existed
		EAGLEEYE_INFO("dictionary has been existed \n");

		//write output signal info directly
		InfoSignal<std::string>* output_signal_info = TO_INFO_SIGNAL(std::string,getOutputPort(OUTPUT_PORT_DIC_FILE_INFO));
		m_trainer_info = m_trainer_name;

		output_signal_info->info = &m_trainer_info;
		return false;
	}
	else
	{
		return true;
	}
}

void DenseDescriptorDictionaryTrainer::parseImage()
{
	//online dictionary builder
	OnlineDictionary online_dictionary(m_dictionary_capacity);
	
	std::map<std::string,std::string> samples_and_annotation;
	std::map<std::string,Array<int,4>> samples_regions;
	std::map<std::string,int> samples_labels;
	int labels_num;

	//load samples from file
	loadObjectClassifyTrainingSamples(samples_and_annotation,samples_regions,samples_labels,labels_num);

	ImageReadNode<ImageSignal<float>> img_read;
	ImageReadNode<ImageSignal<unsigned char>> lable_read;
	std::map<std::string,std::string>::iterator iter,iend(samples_and_annotation.end());
	for (iter = samples_and_annotation.begin(); iter != iend; ++iter)
	{
		std::string sample_file_name = iter->first;
		if (!sample_file_name.empty())
		{
			EAGLEEYE_INFO("process %s \n",sample_file_name.c_str());

			img_read.setFilePath((m_trainer_folder + sample_file_name).c_str());
			img_read.start();

			Matrix<float> img_sample = img_read.getImage();		
			Array<int,4> object_region = samples_regions[sample_file_name];

			Matrix<float> local_img_sample = img_sample(Range(object_region[2],object_region[3]),Range(object_region[0],object_region[1]));
			local_img_sample.clone();
			
//			putToMatlab(local_img_sample,"local_img");

			//resize img
			if (m_is_resize_flag)
			{
				local_img_sample = resize(local_img_sample,m_scales[0]);
			}
			
//			putToMatlab(local_img_sample,"resize_local_img");

			//get rid of noise 
			if (m_is_gaussian_flag)
			{
				local_img_sample = gaussFilter(local_img_sample,m_kernel_size[0],m_kernel_size[1]);
			}
//			putToMatlab(local_img_sample,"gaussian_local_img");

			//get dense points
			DenseFeatureDetector dense_feature_det;
			if (m_is_needed_main_dir)
			{
				dense_feature_det.enableCalcMainDir();
			}
			else
			{
				dense_feature_det.disableCalcMainDir();
			}

			dense_feature_det.setDetectorParams(m_init_scale,m_scale_levels,m_scale_mul,m_init_xy_step,m_init_img_bound,
				m_search_radius,m_vary_xy_step_with_scale,m_vary_img_bound_with_scale);

			std::vector<KeyPoint> keypoints;
			dense_feature_det.detect(local_img_sample,keypoints);

			//compute keypoint descriptors
			Matrix<float> keypoints_descripors;
			m_descriptor->compute(local_img_sample,keypoints,keypoints_descripors);

			online_dictionary.trainOnline(keypoints_descripors,m_dictionary,m_dictionary_counts);
		}
	}

	Dictionary::saveDictionary(m_dictionary,(m_trainer_folder + m_trainer_name + m_ext_name).c_str());
}

void DenseDescriptorDictionaryTrainer::clearSomething()
{
	//clear some temporary variables
	m_dictionary = Matrix<float>();
	m_dictionary_counts = Matrix<int>();
}
}