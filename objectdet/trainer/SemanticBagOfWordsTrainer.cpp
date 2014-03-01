#include "SemanticBagOfWordsTrainer.h"
#include "Learning/AuxiliaryFunctions.h"
#include "Matlab/MatlabInterface.h"
namespace eagleeye
{
SemanticBagOfWordsTrainer::SemanticBagOfWordsTrainer()
{
	m_switch_flag = false;
	m_cross_validation_mode = K_5_FOLDER;
	m_disturb_order_flag = false;

	m_class_num = 0;
	m_words_num = 0;
	m_clique_num = 0;
}
SemanticBagOfWordsTrainer::~SemanticBagOfWordsTrainer()
{

}

void SemanticBagOfWordsTrainer::setCrossValidation(bool switch_flag,bool disturb_order /* = false */,CrossValidationMode mode /* = K_10_FOLDER */)
{
	m_switch_flag = switch_flag;
	m_disturb_order_flag = disturb_order;
	m_cross_validation_mode = mode;
}

bool SemanticBagOfWordsTrainer::isNeedProcessed()
{
	if (EagleeyeIO::isexist(m_trainer_folder + m_trainer_name + m_ext_name))
	{
		//check whether classification parameters file has been built
		EAGLEEYE_INFO("classifier model have existed \n");
		
		//write classifier model info
		InfoSignal<std::string>* output_signal_calssifier_info = 
			TO_INFO_SIGNAL(std::string,getOutputPort(OUTPUT_PORT_CLASSIFIER_INFO));

		m_trainer_info = m_trainer_name;
		output_signal_calssifier_info->info = &m_trainer_info;

		return false;
	}
	else
		return true;
}
bool SemanticBagOfWordsTrainer::selfcheck()
{
	return true;
}

void SemanticBagOfWordsTrainer::train()
{
	//get samples file
	std::string training_samples_file = TO_INFO(std::string,getInputPort(INPUT_PORT_SAMPLES_INFO));

	Matrix<float> samples_label,samples_representation;
	{
		Matrix<float> training_samples;
		EagleeyeIO::read(training_samples,m_trainer_folder + training_samples_file + m_ext_name,READ_BINARY_MODE);
		int samples_num = training_samples.rows();

		samples_label = training_samples(Range(0,samples_num),Range(0,1));
		samples_label.clone();
		samples_representation = training_samples(Range(0,samples_num),Range(1,training_samples.cols()));
		samples_representation.clone();
	}

	EagleeyeIO semantic_models_o;
	semantic_models_o.createWriteHandle(m_trainer_folder + m_trainer_name + m_ext_name,false,WRITE_BINARY_MODE);
	//write class number
	semantic_models_o.write(m_class_num);
	
	for (int class_index = 0; class_index < 2; ++class_index)
	{
		EAGLEEYE_INFO("process model %d\n",class_index);

		Matrix<float> resampling_label = samples_label;
		resampling_label.clone();
		int samples_num = resampling_label.rows();
		//reassign positive and negative samples
		for (int i = 0; i < samples_num; ++i)
		{
			if (int(resampling_label(i)) == class_index)
				resampling_label(i) = 0.0f;
			else
				resampling_label(i) = 1.0f;
		}

		Matrix<float> resampling_samples = samples_representation;
		resampling_samples.clone();
		resampling(resampling_samples,resampling_label);

		int resamples_num = resampling_label.rows();

		//reconstruct sample feature
		Matrix<float> words_frequency = resampling_samples(Range(0,resamples_num),Range(0,m_words_num));
		Matrix<float> words_pair_dis = resampling_samples(Range(0,resamples_num),Range(m_words_num,m_words_num + m_clique_num));
		Matrix<float> words_pair_angle = resampling_samples(Range(0,resamples_num),Range(m_words_num + m_clique_num,m_words_num + 2 * m_clique_num));

		Matrix<int> target_states(resamples_num,m_words_num);
		for (int sample_index = 0; sample_index < resamples_num; ++sample_index)
		{
			if (int(resampling_label(sample_index)) == 0)
			{
				//positive samples
				for (int w_index = 0; w_index < m_words_num; ++w_index)
				{
					if (words_frequency(sample_index,w_index) > 0.0001f)
						target_states(sample_index,w_index) = 1;
					else
						target_states(sample_index,w_index) = 0;
				}
			}
			else
			{
				//negative samples
				for (int w_index = 0; w_index < m_words_num; ++w_index)
				{
					if (words_frequency(sample_index,w_index) > 0.0001f)
						target_states(sample_index,w_index) = 2;
					else
						target_states(sample_index,w_index) = 0;
				}
			}
		}

/*		putToMatlab(target_states,"target");*/

		//start training
		SemanticBagOfWords* semantic_model = new SemanticBagOfWords(m_words_num,3);
		semantic_model->semanticLearn(target_states,words_frequency,words_pair_dis,words_pair_angle);

		Matrix<float> unary_coe,clique_coe;
		semantic_model->getModelParam(unary_coe,clique_coe);

		semantic_models_o.write(unary_coe);
		semantic_models_o.write(clique_coe);

		delete semantic_model;
	}

	semantic_models_o.destroyHandle();
	
	//write classifier model info
	InfoSignal<std::string>* output_signal_calssifier_info = 
		TO_INFO_SIGNAL(std::string,getOutputPort(OUTPUT_PORT_CLASSIFIER_INFO));

	m_trainer_info = m_trainer_name;
	output_signal_calssifier_info->info = &m_trainer_info;

	//evaluate performance
}

void SemanticBagOfWordsTrainer::setParameters(int class_num,int words_num,int clique_num)
{
	m_class_num = class_num;
	m_words_num = words_num;
	m_clique_num = clique_num;
}
}