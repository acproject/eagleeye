#include "ShapeDictionaryTrainer.h"
#include "ProcessNode/ImageReadNode.h"
#include "MatrixMath.h"
#include "Learning/dictionary/OnlineDictionary.h"
#include "Learning/dictionary/BOWDictionaryOpenCV.h"
#include "Matlab/MatlabInterface.h"
#include "Variable.h"

namespace eagleeye
{
ShapeDictionaryTrainer::ShapeDictionaryTrainer()
{
	m_random_jitter_switch = false;
	m_jitter_num = 10;
	m_jitter_degree = 10.0f;

	m_canny_low_threshold = 20.0;
	m_canny_high_threshold = 40.0;

	m_max_iterators_num = 1;
}

ShapeDictionaryTrainer::~ShapeDictionaryTrainer()
{

}

void ShapeDictionaryTrainer::train()
{
	//parse training images
	parseImages();

	//write to output signal
	InfoSignal<std::string>* output_signal_info = TO_INFO_SIGNAL(std::string,getOutputPort(OUTPUT_PORT_DIC_FILE_INFO));
	m_trainer_info = m_trainer_name;
	output_signal_info->info = &m_trainer_info;

	EAGLEEYE_INFO("success to build dictionary (%s)\n",m_trainer_name.c_str());
}

bool ShapeDictionaryTrainer::selfcheck()
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

bool ShapeDictionaryTrainer::isNeedProcessed()
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

void ShapeDictionaryTrainer::parseImages()
{
	OnlineDictionary online_dictionary(m_dictionary_capacity);

	std::map<std::string,std::string> samples_and_annotation;
	std::map<std::string,Array<int,4>> samples_regions;
	std::map<std::string,int> samples_labels;
	int labels_num;

	//load samples from file
	loadObjectClassifyTrainingSamples(samples_and_annotation,samples_regions,samples_labels,labels_num);

	int iterator_count = 0;
	while(iterator_count < m_max_iterators_num)
	{
		//clear m_dictioanry_counts
		m_dictionary_counts.setzeros();
		
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

				int rows = local_img_sample.rows();
				int cols = local_img_sample.cols();

				//resize img
				if (m_is_resize_flag)
				{
					local_img_sample = resize(local_img_sample,m_scales[0]);
				}

				//get rid of noise 
				if (m_is_gaussian_flag)
				{
					local_img_sample = gaussFilter(local_img_sample,m_kernel_size[0],m_kernel_size[1]);
				}

				//extract edge from local image
				Matrix<unsigned char> edges = canny(local_img_sample.transform<unsigned char>(),m_canny_low_threshold,m_canny_high_threshold);

				std::vector<KeyPoint> keypoints;
				for (int i = 0; i < rows; ++i)
				{
					unsigned char* edges_data = edges.row(i);
					for (int j = 0; j < cols; ++j)
					{
						if (edges_data[j] == 255 && (i > m_img_bound[1]) && (i < (rows - m_img_bound[1])) &&
							(j > m_img_bound[0]) && (j < (cols - m_img_bound[0])))
						{
							KeyPoint p;
							p.pt[0] = float(j);	//x
							p.pt[1] = float(i);	//y

							Variable<float> x_gaussian_random = Variable<float>::gaussian(double(j),double(m_jitter_degree));	//x
							Variable<float> y_gaussian_random = Variable<float>::gaussian(double(i),double(m_jitter_degree));	//y

							//sampling points at edges
							keypoints.push_back(p);

							//sampling points around edges
							if (m_random_jitter_switch)
							{
								//jitter
								int jitter_count = 0;
								while(jitter_count < m_jitter_num)
								{
									p.pt[0] = x_gaussian_random.var();
									p.pt[1] = y_gaussian_random.var();
									if ((p.pt[0] > m_img_bound[0]) && (p.pt[0] < (cols - m_img_bound[0])) && 
										(p.pt[1] > m_img_bound[1]) && (p.pt[1] < (rows - m_img_bound[1])))
									{
										keypoints.push_back(p);
									}

									jitter_count++;
								}
							}
						}
					}
				}

				//compute keypoint descriptors
				Matrix<float> keypoints_descripors;
				m_descriptor->compute(local_img_sample,keypoints,keypoints_descripors);

				online_dictionary.trainOnline(keypoints_descripors,m_dictionary,m_dictionary_counts);
			}
		}

		iterator_count++;
	}

	Dictionary::saveDictionary(m_dictionary,(m_trainer_folder + m_trainer_name + m_ext_name).c_str());
}

void ShapeDictionaryTrainer::clearSomething()
{
	//clear some temporary variables
	m_dictionary = Matrix<float>();
	m_dictionary_counts = Matrix<int>();
}

void ShapeDictionaryTrainer::setRandomJitterSwitch(bool flag,float jitter_degree,int jitter_num)
{
	m_random_jitter_switch = flag;
	m_jitter_degree = jitter_degree;
	m_jitter_num = jitter_num;
}

void ShapeDictionaryTrainer::setCannyThreshold(double low_threshold,double high_threshold)
{
	m_canny_low_threshold = low_threshold;
	m_canny_high_threshold = high_threshold;
}

void ShapeDictionaryTrainer::setMaxIteratorsNum(int num)
{
	m_max_iterators_num = num;
}
}