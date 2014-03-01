#include "ObjectDetGrammarTreeTrainer.h"
#include "Print.h"
#include "EagleeyeStr.h"
#include "ImageReadNode.h"
#include "ImageWriteNode.h"
#include "EagleeyeIO.h"
#include "MatrixMath.h"
#include "Learning/ObjectDetGrammarTreeLatentSVM.h"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
ObjectDetGrammarTreeTrainer::ObjectDetGrammarTreeTrainer(ObjectDetGrammarTree* gt_tree)
{
	m_hold_gt_tree = gt_tree;
	
	m_training_samples_num = 0;
	m_training_samples_dim = 0;
	m_latent_dim = 0;
	m_training_model_max_iters = 1;

	m_trainer_samples_info = "_samples_info";
	m_trainer_latent_info = "_latent_info";
	m_weight_info = "_weight_info";

	//in order to make the whole pipeline work, 
	// setting one empty output signal
	setNumberOfOutputSignals(1);
	setOutputPort(makeOutputSignal());
}
ObjectDetGrammarTreeTrainer::~ObjectDetGrammarTreeTrainer()
{

}

void ObjectDetGrammarTreeTrainer::train()
{
	Matrix<float> optimum_weight;
	for (int i = 0; i < m_training_model_max_iters; ++i)
	{
		EAGLEEYE_INFO("iterator %d for training model\n",i);

		//parse all training images
		parseImageWithoutAnnotation();

		//reading data from file
		EagleeyeIO training_samples_io;
		training_samples_io.createReadHandle(m_trainer_folder + m_unit_name + m_trainer_samples_info + m_ext_name,READ_BINARY_MODE);

		Matrix<float> training_features(m_training_samples_num,m_training_samples_dim - 1,0.0f);
		Matrix<int> training_labels(m_training_samples_num,1,int(0));

		int training_samples_count = 0;
		while(training_samples_count < m_training_samples_num)
		{
			Matrix<float> load_samples;
			training_samples_io.read(load_samples);
			int input_samples_num = load_samples.rows();

			training_features(Range(training_samples_count,training_samples_count + input_samples_num),
				Range(0,m_training_samples_dim - 1)).copy(load_samples(Range(0,input_samples_num),Range(1,m_training_samples_dim)));
			for (int m = 0; m < input_samples_num; ++m)
			{
				training_labels(training_samples_count + m,0) = int(load_samples(m,0));
			}

			training_samples_count += input_samples_num;
		}
		training_samples_io.destroyHandle();

		EagleeyeIO latent_variables_io;
		latent_variables_io.createReadHandle(m_trainer_folder + m_unit_name + m_trainer_latent_info + m_ext_name,READ_BINARY_MODE);

		Matrix<int> latent_variables(m_training_samples_num,m_latent_dim,int(0));
		training_samples_count = 0;
		while(training_samples_count < m_training_samples_num)
		{
			Matrix<int> load_latents;
			latent_variables_io.read(load_latents);
			latent_variables(Range(training_samples_count,training_samples_count + load_latents.rows()),
				Range(0,m_latent_dim)).copy(load_latents);

			training_samples_count += load_latents.rows();
		}
		latent_variables_io.destroyHandle();
		
		//get default model coefficient from grammar tree model
		Matrix<float> gt_weight = m_hold_gt_tree->getGrammarTreeWeight();
		GrammarTreeStructureInfo gt_structure_info = m_hold_gt_tree->getGrammarTreeStructureInfo();
		ObjectDetGrammarTreeLatentSVM gt_latent_svm(gt_structure_info);
		gt_latent_svm.setTrainingSamples(training_labels,latent_variables,training_features);
		gt_latent_svm.setIniWeight(gt_weight);
		gt_latent_svm.learn();

		optimum_weight = gt_latent_svm.getOptimumWeight();
		m_hold_gt_tree->saveGrammarTreeWeight(optimum_weight);

		//refresh the whole grammar tree model
		m_hold_gt_tree->initialize();
	}

	//save optimum weight
	EagleeyeIO::write(optimum_weight,m_trainer_folder + m_unit_name + m_weight_info + m_ext_name,WRITE_BINARY_MODE);

	//finding optimum score threshold
	findingOptimumSplitThreshold();
}

bool ObjectDetGrammarTreeTrainer::isNeedProcessed()
{
	if (EagleeyeIO::isexist(m_trainer_folder + m_unit_name + m_weight_info + m_ext_name))
	{
		EAGLEEYE_INFO("training files have been existed, don't need to be trained again\n");

		//load optimum weight from file
		Matrix<float> optimum_weight;
		EagleeyeIO::read(optimum_weight,m_trainer_folder + m_unit_name + m_weight_info + m_ext_name,READ_BINARY_MODE);

		//refresh the whole grammar tree model
		m_hold_gt_tree->saveGrammarTreeWeight(optimum_weight);
		m_hold_gt_tree->initialize();

		return false;
	}
	return true;
}
bool ObjectDetGrammarTreeTrainer::selfcheck()
{
	if (!m_hold_gt_tree)
	{
		EAGLEEYE_ERROR("sorry,please set ObjectDetGrammarTree object \n");
		return false;
	}

	return true;
}

void ObjectDetGrammarTreeTrainer::parseImageWithoutAnnotation()
{
	m_positive_scores.clear();
	m_negative_scores.clear();

	//load samples files 
	std::map<std::string,std::vector<Array<int,4>>> p_samples;
	std::map<std::string,std::vector<Array<int,4>>> n_samples;

	loadObjectDetTrainingSampls(p_samples,n_samples);

	ImageReadNode<ImageSignal<unsigned char>> img_read_node;
	EagleeyeIO training_samples_io;
	training_samples_io.createWriteHandle(m_trainer_folder + m_unit_name + m_trainer_samples_info + m_ext_name,false,WRITE_BINARY_MODE);
	EagleeyeIO latent_variables_io;
	latent_variables_io.createWriteHandle(m_trainer_folder + m_unit_name + m_trainer_latent_info + m_ext_name,false,WRITE_BINARY_MODE);
	
	m_training_samples_num = 0;
	m_training_samples_dim = 0;
	m_latent_dim = 0;

	//parse all positive samples and write to file sequentially
	std::map<std::string,std::vector<Array<int,4>>>::iterator p_iter,p_iend(p_samples.end());
	for (p_iter = p_samples.begin(); p_iter != p_iend; ++p_iter)
	{
		if (!(p_iter->first.empty()) && !(p_iter->second.empty()))
		{
			//display training process
			EAGLEEYE_INFO("parse positive training samples %s \n",p_iter->first.c_str());
			
			//read sample image
			std::string sample_file = p_iter->first;
			img_read_node.setFilePath((m_trainer_folder + sample_file).c_str());
			img_read_node.start();

			Matrix<unsigned char> sample_img = img_read_node.getImage();
			
			std::vector<Array<int,4>>::iterator region_iter,region_iend(p_iter->second.end());
			for (region_iter = p_iter->second.begin(); region_iter != region_iend; ++region_iter)
			{
				Array<int,4> object_region = (*region_iter);
				
				Matrix<float> gt_sample;
				Matrix<int> gt_latent_variables;
				float positive_score = m_hold_gt_tree->parseTrainingData(&sample_img,EAGLEEYE_POSITIVE_SAMPLE,&object_region,gt_sample,gt_latent_variables);

				if (gt_sample.cols() > 0)
				{
					//recored positive score
					m_positive_scores.push_back(positive_score);

					//save valid training samples
					training_samples_io.write(gt_sample);
					latent_variables_io.write(gt_latent_variables);

					m_training_samples_num += gt_sample.rows();
					m_training_samples_dim = gt_sample.cols();
					m_latent_dim = gt_latent_variables.cols();
				}
			}
		}
	}

	//parse all negative samples and write to file sequentially
	std::map<std::string,std::vector<Array<int,4>>>::iterator n_iter,n_end(n_samples.end());
	for (n_iter = n_samples.begin(); n_iter != n_end; ++n_iter)
	{
		if (!(n_iter->first.empty()))
		{
			//display training process
			EAGLEEYE_INFO("parse negative training samples %s \n",n_iter->first.c_str());

			//read sample image
			std::string sample_file = n_iter->first;
			img_read_node.setFilePath((m_trainer_folder + sample_file).c_str());
			img_read_node.start();

			Matrix<unsigned char> sample_img = img_read_node.getImage();

			//not used for negative samples
			Array<int,4> object_region;
			object_region[0] = 0; object_region[1] = sample_img.cols();
			object_region[2] = 0; object_region[3] = sample_img.rows();

			Matrix<float> gt_sample;
			Matrix<int> gt_latent_variables;
			float negative_score = m_hold_gt_tree->parseTrainingData(&sample_img,EAGLEEYE_NEGATIVE_SAMPLE,&object_region,gt_sample,gt_latent_variables);

			if (gt_sample.cols() > 0)
			{
				//record negative score
				m_negative_scores.push_back(negative_score);
				
				//valid training samples
				training_samples_io.write(gt_sample);
				latent_variables_io.write(gt_latent_variables);
				m_training_samples_num += gt_sample.rows();
			}
		}
	}

	training_samples_io.destroyHandle();
	latent_variables_io.destroyHandle();
}
void ObjectDetGrammarTreeTrainer::clearSomething()
{
	//now do nothing
}

void ObjectDetGrammarTreeTrainer::findingOptimumSplitThreshold()
{
	Matrix<float> positive_scores(1,m_positive_scores.size(),&m_positive_scores[0],true);
	Matrix<float> negative_scores(1,m_negative_scores.size(),&m_negative_scores[0],true);

	float kde_param[3] = {100.0f,0.0025f,0.0025f};

	Matrix<float> optimum_threshold = autoProbabilityDecision(positive_scores,negative_scores,GAUSSIAN_KDE_MODEL,true,kde_param);

	m_hold_gt_tree->setScoreThreshold(optimum_threshold(0,0));
}

void ObjectDetGrammarTreeTrainer::setMaxIterators(int max_iters)
{
	m_training_model_max_iters = max_iters;
}

}