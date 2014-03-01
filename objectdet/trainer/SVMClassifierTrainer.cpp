#include "SVMClassifierTrainer.h"
#include "Variable.h"
#include "MatrixMath.h"
#include "EagleeyeIO.h"
#include "ProcessNode/PerformanceEvaluationNode.h"
#include "EagleeyeFile.h"
#include "Learning/AuxiliaryFunctions.h"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
class libsvmLearner:public DummyLearner
{
public:
	libsvmLearner(libsvm* lib_svm,int k_top_hit = 1):m_lib_svm(lib_svm),m_k_top_hit(k_top_hit){};
	virtual ~libsvmLearner(){};

	virtual void learn(const Matrix<float>& samples,const Matrix<float>& labels)
	{
		m_lib_svm->setSamplesMat(labels,samples);
		m_lib_svm->learn();
	}
	virtual float evalution(const Matrix<float>& validation_samples,const Matrix<float>& labels)
	{
		Matrix<int> predict_labels;
		Matrix<float> predict_estimates;
		m_lib_svm->predict(validation_samples,predict_labels,predict_estimates);
		int labels_num = m_lib_svm->getClassNum();

		if (predict_estimates.isempty())
		{
			//compare predict labels and labels directly
			int num = validation_samples.rows();
			int error_num = 0;
			for (int i = 0; i < num; ++i)
			{
				if (predict_labels[i] != unsigned int(labels(i)))
					error_num++;
			}

			return float(error_num) / float(num);
		}
		else
		{
			//using top hit strategy
			int num = validation_samples.rows();
			int error_num = 0;

			for (int i = 0; i < num; ++i)
			{
				Matrix<float> predict_probability = predict_estimates(Range(i,i + 1),Range(0,labels_num));
				std::vector<unsigned int> order_labels = sort<DescendingSortPredict<float>>(predict_probability);
				
				bool flag = false;
				for (int m = 0; m < m_k_top_hit; ++m)
				{
					if (order_labels[m] == unsigned int(labels(i)))
					{
						flag = true;
						break;
					}
				}

				if (flag == false)
					error_num++;
			}

			return float(error_num) / float(num);
		}

	}
	virtual void save(const char* model_path)
	{
		m_lib_svm->saveSVMModel(model_path);
	}

private:
	libsvm* m_lib_svm;
	int m_k_top_hit;
};

SVMClassifierTrainer::SVMClassifierTrainer()
{
	m_kernel_type = libsvm::LIBSVM_RBF;
	m_svm_type = libsvm::LIBSVM_C_SVC;

	m_switch_flag = false;
	m_resample_flag = false;
	m_cross_validation_mode = K_10_FOLDER;
	m_disturb_order_flag = false;

	m_top_hit_mode = false;
	m_k_top_hit = 3;
}

SVMClassifierTrainer::~SVMClassifierTrainer()
{

}

void SVMClassifierTrainer::train()
{
	//get samples file
	std::string samples_file = TO_INFO(std::string,getInputPort(INPUT_PORT_SAMPLES_INFO));
	
	//read samples from file
	Matrix<float> samples_label,samples_representation;
	{
		Matrix<float> samples;
		EagleeyeIO::read(samples,m_trainer_folder + samples_file + m_ext_name,READ_BINARY_MODE);
		int samples_num = samples.rows();

		samples_label = samples(Range(0,samples_num),Range(0,1));
		samples_label.clone();
		samples_representation = samples(Range(0,samples_num),Range(1,samples.cols()));
		samples_representation.clone();
	}

	//resampling
	if (m_resample_flag)
	{
		resampling(samples_representation,samples_label);
	}

	//using libsvm
	libsvm* lib_svm = new libsvm;
	lib_svm->setKernelType(m_kernel_type);
	lib_svm->setSVMType(m_svm_type);

	//judge whether enable top hit strategy
	if (m_top_hit_mode == true)
	{
		lib_svm->setProbabilityEstimate(true);
	}

	if (m_switch_flag)
	{
		EAGLEEYE_INFO("start cross validation\n");
		libsvmLearner* libsvm_learner = new libsvmLearner(lib_svm,m_k_top_hit);

		//use cross validation
		CrossValidation cross_validation(libsvm_learner,samples_representation,samples_label);
		cross_validation.setCrossValidationMode(m_cross_validation_mode);
		cross_validation.disturbOrder(m_disturb_order_flag);
		cross_validation.startCrossValidation(m_trainer_folder.c_str());

		int optimum_model_index = cross_validation.getOptimumModelIndex();

		//copy model file
		char str[100];
		EAGLEEYE_CopyFile((m_trainer_folder + itoa(optimum_model_index,str,10)).c_str(),(m_trainer_folder + m_trainer_name + m_ext_name).c_str());
		delete libsvm_learner;
	}
	else
	{
		//using libsvm directly
		//don't use cross validation
		EAGLEEYE_INFO("start training libsvm\n");
		lib_svm->setSamplesMat(samples_label,samples_representation);
		lib_svm->learn();

		EAGLEEYE_INFO("save libsvm\n");
		lib_svm->saveSVMModel((m_trainer_folder + m_trainer_name + m_ext_name).c_str());
	}

	delete lib_svm;

	//analyze optimum libsvm performance
	EAGLEEYE_INFO("analyze libsvm performance in training samples\n");
	libsvm* optimum_libsvm = new libsvm;
	optimum_libsvm->readSVMModel((m_trainer_folder + m_trainer_name + m_ext_name).c_str());
	Matrix<int> predict_label;
	Matrix<float> predict_estimates; 
	optimum_libsvm->predict(samples_representation,predict_label,predict_estimates);

	//labels number
	int labels_num = optimum_libsvm->getClassNum();

	//sample number
	int samples_num = samples_representation.rows();

	if (m_top_hit_mode)
	{
		StatisticPlan statis_plan;
		if (m_k_top_hit == 3)
		{
			//top hit 3
			statis_plan = LABEL_TOP_HIT_3;
		}
		else
		{
			//top hit 5
			statis_plan = LABEL_TOP_HIT_5;
		}

		Matrix<float> predict_probability_and_labels(samples_num,labels_num + 1);
		Matrix<float> predict_probability_mat = predict_probability_and_labels(Range(0,samples_num),Range(0,labels_num));
		predict_probability_mat.copy(predict_estimates);

		Matrix<float> labels_mat = predict_probability_and_labels(Range(0,samples_num),Range(labels_num,labels_num + 1));
		labels_mat.copy(samples_label);

		PerformanceEvaluationNode<ImageSignal<float>> p_eva_node;
		ImageSignal<float> classify_data_sig(predict_probability_and_labels);
		p_eva_node.setInputPort(&classify_data_sig);
		p_eva_node.setStatisticPlan(statis_plan);
		p_eva_node.setLabelsNum(labels_num);
		p_eva_node.start();

		p_eva_node.saveStatisticsInfo((m_trainer_folder + "performance_analyze.dat").c_str());
	}
	else
	{
		Matrix<int> predict_and_groundtruth_labels(predict_label.rows(),2);
		predict_and_groundtruth_labels(Range(0,predict_label.rows()),Range(0,1)).copy(predict_label);
		predict_and_groundtruth_labels(Range(0,predict_label.rows()),Range(1,2)).copy(samples_label.transform<int>());

		PerformanceEvaluationNode<ImageSignal<int>> p_eva_node;
		ImageSignal<int> classify_data_sig(predict_and_groundtruth_labels);
		p_eva_node.setInputPort(&classify_data_sig);
		p_eva_node.setStatisticPlan(LABEL_COMPARE_STA);
		p_eva_node.setLabelsNum(labels_num);
		p_eva_node.start();

		p_eva_node.saveStatisticsInfo((m_trainer_folder + "performance_analyze.dat").c_str());
	}

	delete optimum_libsvm;

	//write classifier model info
	InfoSignal<std::string>* output_signal_calssifier_info = 
		TO_INFO_SIGNAL(std::string,getOutputPort(OUTPUT_PORT_CLASSIFIER_INFO));

	m_trainer_info = m_trainer_name;
	output_signal_calssifier_info->info = &m_trainer_info;
}

bool SVMClassifierTrainer::isNeedProcessed()
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
	{
		return true;
	}
}

bool SVMClassifierTrainer::selfcheck()
{
	if (!TO_INFO_SIGNAL(std::string,getInputPort(INPUT_PORT_SAMPLES_INFO)))
	{
		EAGLEEYE_ERROR("sorry,samples feature input port is empty \n");
		return false;
	}

	return true;
}

void SVMClassifierTrainer::setlibsvmKernelType(const libsvm::KernelType kernel_type)
{
	m_kernel_type = kernel_type;
}
void SVMClassifierTrainer::setlibsvmKernelType(const int kernel_type)
{
	m_kernel_type = (libsvm::KernelType)kernel_type;
}
void SVMClassifierTrainer::getlibsvmKernelType(int& kernel_type)
{
	kernel_type = int(m_kernel_type);
}

void SVMClassifierTrainer::setlibsvmType(const libsvm::SVMType svm_type)
{
	m_svm_type = svm_type;
}
void SVMClassifierTrainer::setlibsvmType(const int svm_type)
{
	m_svm_type = (libsvm::SVMType)svm_type;
}
void SVMClassifierTrainer::getlibsvmType(int& svm_type)
{
	svm_type = (int)m_svm_type;
}

void SVMClassifierTrainer::setCrossValidation(bool switch_flag,bool disturb_order,CrossValidationMode mode /* = K_10_FOLDER */)
{
	m_switch_flag = switch_flag;
	m_disturb_order_flag = disturb_order;
	m_cross_validation_mode = mode;
}

void SVMClassifierTrainer::setResamplingSamples(bool flag)
{
	m_resample_flag = flag;
}

void SVMClassifierTrainer::setTopHitStrategy(bool flag,int k_top_hit /* = 3 */)
{
	m_top_hit_mode = flag;
	m_k_top_hit = k_top_hit;
}

}