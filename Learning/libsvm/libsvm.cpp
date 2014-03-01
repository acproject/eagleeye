#include "Learning/libsvm/LibSVM.h"
#include "Learning/libsvm/svm.h"
#include "Print.h"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
// svm arguments
struct svm_parameter param;		// set by parse_command_line
struct svm_problem prob;		// set by read_problem
struct svm_node *x_space = NULL;
int cross_validation;
int nr_fold;
struct svm_model* libsvm_model = NULL;

void exit_with_help()
{
	EAGLEEYE_INFO("the core module of LibSVM exits");

	/*
	EAGLEEYE_INFO(
		"Usage: model = svmtrain(training_label_vector, training_instance_matrix, 'libsvm_options');\n"
		"libsvm_options:\n"
		"-s svm_type : set type of SVM (default 0)\n"
		"	0 -- C-SVC		(multi-class classification)\n"
		"	1 -- nu-SVC		(multi-class classification)\n"
		"	2 -- one-class SVM\n"
		"	3 -- epsilon-SVR	(regression)\n"
		"	4 -- nu-SVR		(regression)\n"
		"-t kernel_type : set type of kernel function (default 2)\n"
		"	0 -- linear: u'*v\n"
		"	1 -- polynomial: (gamma*u'*v + coef0)^degree\n"
		"	2 -- radial basis function: exp(-gamma*|u-v|^2)\n"
		"	3 -- sigmoid: tanh(gamma*u'*v + coef0)\n"
		"	4 -- precomputed kernel (kernel values in training_instance_matrix)\n"
		"-d degree : set degree in kernel function (default 3)\n"
		"-g gamma : set gamma in kernel function (default 1/num_features)\n"
		"-r coef0 : set coef0 in kernel function (default 0)\n"
		"-c cost : set the parameter C of C-SVC, epsilon-SVR, and nu-SVR (default 1)\n"
		"-n nu : set the parameter nu of nu-SVC, one-class SVM, and nu-SVR (default 0.5)\n"
		"-p epsilon : set the epsilon in loss function of epsilon-SVR (default 0.1)\n"
		"-m cachesize : set cache memory size in MB (default 100)\n"
		"-e epsilon : set tolerance of termination criterion (default 0.001)\n"
		"-h shrinking : whether to use the shrinking heuristics, 0 or 1 (default 1)\n"
		"-b probability_estimates : whether to train a SVC or SVR model for probability estimates, 0 or 1 (default 0)\n"
		"-wi weight : set the parameter C of class i to weight*C, for C-SVC (default 1)\n"
		"-v n : n-fold cross validation mode\n"
		"-q : quiet mode (no outputs)\n"
		);*/
}

double do_cross_validation()
{
	int i;
	int total_correct = 0;
	double total_error = 0;
	double sumv = 0, sumy = 0, sumvv = 0, sumyy = 0, sumvy = 0;
	double *target = (double*)malloc(sizeof(double)*prob.l);
	double retval = 0.0;

	svm_cross_validation(&prob,&param,nr_fold,target);
	if(param.svm_type == EPSILON_SVR ||
		param.svm_type == NU_SVR)
	{
		for(i=0;i<prob.l;i++)
		{
			double y = prob.y[i];
			double v = target[i];
			total_error += (v-y)*(v-y);
			sumv += v;
			sumy += y;
			sumvv += v*v;
			sumyy += y*y;
			sumvy += v*y;
		}
		EAGLEEYE_INFO("Cross Validation Mean squared error = %g\n",total_error/prob.l);
		EAGLEEYE_INFO("Cross Validation Squared correlation coefficient = %g\n",
			((prob.l*sumvy-sumv*sumy)*(prob.l*sumvy-sumv*sumy))/
			((prob.l*sumvv-sumv*sumv)*(prob.l*sumyy-sumy*sumy))
			);
		retval = total_error/prob.l;
	}
	else
	{
		for(i=0;i<prob.l;i++)
			if(target[i] == prob.y[i])
				++total_correct;
		EAGLEEYE_INFO("Cross Validation Accuracy = %g%%\n",100.0*total_correct/prob.l);
		retval = 100.0*total_correct/prob.l;
	}
	free(target);
	return retval;
}

int parse_command_line(const char* cmd_str)
{
	int i, argc = 1;
	char cmd[CMD_LEN];
	char *argv[CMD_LEN/2];
	
	const char* temp=cmd_str;
	int cmd_str_count=0;
	while((*temp)!='\0')
	{
		temp++;
		cmd_str_count++;
	}
	
	if (cmd_str_count>0&&(cmd_str_count+1)<CMD_LEN)
	{
		memcpy(cmd,cmd_str,sizeof(char)*cmd_str_count);
		cmd[cmd_str_count]='\0';
	}

	// default values
	param.svm_type = C_SVC;
	param.kernel_type = RBF;
	param.degree = 3;
	param.gamma = 0;	// 1/num_features
	param.coef0 = 0;
	param.nu = 0.5;
	param.cache_size = 100;
	param.C = 1;
	param.eps = 1e-3;
	param.p = 0.1;
	param.shrinking = 1;
	param.probability = 0;
	param.nr_weight = 0;
	param.weight_label = NULL;
	param.weight = NULL;
	cross_validation = 0;

	if((argv[argc] = strtok(cmd, " ")) != NULL)
		while((argv[++argc] = strtok(NULL, " ")) != NULL);

	// parse options
	for(i=1;i<argc;i++)
	{
		if(argv[i][0] != '-') break;
		++i;
		if(i>=argc && argv[i-1][1] != 'q')	// since option -q has no parameter
			return 1;
		switch(argv[i-1][1])
		{
		case 's':
			param.svm_type = atoi(argv[i]);
			break;
		case 't':
			param.kernel_type = atoi(argv[i]);
			break;
		case 'd':
			param.degree = atoi(argv[i]);
			break;
		case 'g':
			param.gamma = atof(argv[i]);
			break;
		case 'r':
			param.coef0 = atof(argv[i]);
			break;
		case 'n':
			param.nu = atof(argv[i]);
			break;
		case 'm':
			param.cache_size = atof(argv[i]);
			break;
		case 'c':
			param.C = atof(argv[i]);
			break;
		case 'e':
			param.eps = atof(argv[i]);
			break;
		case 'p':
			param.p = atof(argv[i]);
			break;
		case 'h':
			param.shrinking = atoi(argv[i]);
			break;
		case 'b':
			param.probability = atoi(argv[i]);
			break;
		case 'q':
			i--;
			break;
		case 'v':
			cross_validation = 1;
			nr_fold = atoi(argv[i]);
			if(nr_fold < 2)
			{
				EAGLEEYE_INFO("n-fold cross validation: n must >= 2\n");
				return 1;
			}
			break;
		case 'w':
			++param.nr_weight;
			param.weight_label = (int *)realloc(param.weight_label,sizeof(int)*param.nr_weight);
			param.weight = (double *)realloc(param.weight,sizeof(double)*param.nr_weight);
			param.weight_label[param.nr_weight-1] = atoi(&argv[i-1][2]);
			param.weight[param.nr_weight-1] = atof(argv[i]);
			break;
		default:
			EAGLEEYE_INFO("Unknown option -%c\n",argv[i-1][1]);
			return 1;
		}
	}

	svm_set_print_string_function(NULL);
	return 0;
}

// read in a problem (in svmlight format)
int read_problem_dense(Matrix<float> labels_mat,Matrix<float> samples_mat)
{
	int i, j, k;
	int elements, max_index, sc, label_vector_row_num;

	prob.x = NULL;
	prob.y = NULL;
	x_space = NULL;
	float* labels=labels_mat.dataptr();
	float* samples=samples_mat.dataptr();
	sc = samples_mat.cols();//the dimension of feature

	elements = 0;
	// the number of instance
	prob.l=samples_mat.rows();
	label_vector_row_num = labels_mat.rows();

	if(label_vector_row_num!=prob.l)
	{
		EAGLEEYE_INFO("length of label vector does not match # of instances.\n");
		return -1;
	}

	if(param.kernel_type == PRECOMPUTED)
		elements = prob.l * (sc + 1);
	else
	{
		for(i = 0; i < prob.l; i++)
		{
			for(k = 0; k < sc; k++)
				if(samples[i*sc+k] != 0)
					elements++;
			// count the '-1' element
			elements++;
		}
	}

	prob.y = (double*)malloc(sizeof(double)*prob.l);
	prob.x = (struct svm_node **)malloc(sizeof(struct svm_node *)*prob.l);
	x_space = (struct svm_node*)malloc(sizeof(struct svm_node)*elements);

	max_index = sc;
	j = 0;
	for(i = 0; i < prob.l; i++)
	{
		prob.x[i] = &x_space[j];
		prob.y[i] = labels[i];

		for(k = 0; k < sc; k++)
		{
			if(param.kernel_type == PRECOMPUTED || samples[i*sc+k] != 0)
			{
				x_space[j].index = k + 1;
				x_space[j].value = samples[i*sc+k];
				j++;
			}
		}
		x_space[j++].index = -1;
	}

	if(param.gamma == 0 && max_index > 0)
		param.gamma = 1.0/max_index;

	if(param.kernel_type == PRECOMPUTED)
		for(i=0;i<prob.l;i++)
		{
			if((int)prob.x[i][0].value <= 0 || (int)prob.x[i][0].value > max_index)
			{
				EAGLEEYE_INFO("wrong input format: sample_serial_number out of range\n");
				return -1;
			}
		}

		return 0;
}

void svmtrain(Matrix<float> labels_mat,Matrix<float> samples_mat,const char* cmd_str)
{
	const char *error_msg;

	// fix random seed to have same results for each run
	// (for cross validation and probability estimation)
	srand(1);

	if(parse_command_line(cmd_str))
	{
		exit_with_help();
		svm_destroy_param(&param);
		return;
	}

	int err;
	err = read_problem_dense(labels_mat, samples_mat);

	// svmtrain's original code
	error_msg = svm_check_parameter(&prob, &param);

	if(err || error_msg)
	{
		if (error_msg != NULL)
			EAGLEEYE_INFO("Error: %s\n", error_msg);
		svm_destroy_param(&param);
		free(prob.y);
		free(prob.x);
		free(x_space);
		return;
	}

	//train svm model
	libsvm_model = svm_train(&prob, &param);
}

//////////////////////////////////////////////////////////////////////////
libsvm::libsvm()
{
	m_svm_parameter_str = "";	
	m_kernel_type = LIBSVM_LINEAR;
	m_svm_type = LIBSVM_C_SVC;
	m_predict_paobability = false;

	m_samples_num = 0;
	m_feature_dim = 0;
}

libsvm::~libsvm()
{
	destroyAllResource();
}

bool libsvm::learn()
{
	//run selfcheck
	if (!selfcheck())
	{
		EAGLEEYE_INFO("couldn't start learning!!!\n");
	}

	//destroy all old resource
	destroyAllResource();

	//it would save libsvm_model as one global variable.
	svmtrain(m_samples_label,m_samples_feature,m_svm_parameter_str.c_str());

	return true;
}

void libsvm::predict(const Matrix<float> samples,Matrix<int>& predict_label,Matrix<float>& predict_estimate)
{
	//check whether libsvm_model is valid
	if (!libsvm_model)
	{
		EAGLEEYE_INFO("please load libsvm model firstly...\n");
		return;
	}
	int samples_num = samples.rows();
	int samples_dim = samples.cols();

	int svm_type = svm_get_svm_type(libsvm_model);
	int nr_class = svm_get_nr_class(libsvm_model);

	//temporary variable
	double* prob_estimates = NULL;
	if (m_predict_paobability)
	{
		prob_estimates = (double *)malloc(nr_class * sizeof(double));
	}

	Matrix<double> ptr_predict_label(samples_num,1);
	Matrix<double> ptr_prob_estimates;
	Matrix<double> ptr_dec_values;

	double* ptr_data = NULL;

	if (m_predict_paobability)
	{
		// prob estimates are ptr_data
		if (svm_type == C_SVC || svm_type == NU_SVC)
		{
			ptr_data = (double*)malloc(samples_num * nr_class * sizeof(double));
			ptr_prob_estimates = Matrix<double>::mapfrom(samples_num,nr_class,ptr_data);
		}
	}
	else
	{
		// decision values are in ptr_data
		if(svm_type == ONE_CLASS ||
			svm_type == EPSILON_SVR ||
			svm_type == NU_SVR ||
			nr_class == 1) // if only one class in training data, decision values are still returned.
		{
			ptr_data = (double*)malloc(samples_num * 1 * sizeof(double));
			ptr_dec_values = Matrix<double>::mapfrom(samples_num,1,ptr_data);
		}
		else
		{
			ptr_data = (double*)malloc(samples_num * nr_class*(nr_class-1)/2 * sizeof(double));
			ptr_dec_values = Matrix<double>::mapfrom(samples_num,nr_class*(nr_class-1)/2,ptr_data);
		}
	}

	//construct sample
	svm_node* sample_x = (struct svm_node*)malloc((samples_dim+1)*sizeof(struct svm_node) );
	for (int sample_index = 0; sample_index < samples_num; ++sample_index)
	{
		double predict_label;
		
		//fill info
		const float* samples_data = samples.row(sample_index);
		for (int i = 0; i < samples_dim; ++i)
		{
			sample_x[i].index = i + 1;
			sample_x[i].value = samples_data[i];
		}
		sample_x[samples_dim].index = -1;

		if (m_predict_paobability)
		{
			if(svm_type == C_SVC || svm_type == NU_SVC)
			{
				predict_label = svm_predict_probability(libsvm_model, sample_x, prob_estimates);
				ptr_predict_label[sample_index] = predict_label;
				for(int i = 0; i < nr_class; i++)
					ptr_prob_estimates[sample_index * nr_class + i] = prob_estimates[i];
			} 
			else 
			{
				predict_label = svm_predict(libsvm_model,sample_x);
				ptr_predict_label[sample_index] = predict_label;
			}
		}
		else
		{
			if(svm_type == ONE_CLASS ||
				svm_type == EPSILON_SVR ||
				svm_type == NU_SVR)
			{
				double res;
				predict_label = svm_predict_values(libsvm_model, sample_x, &res);
				ptr_dec_values[sample_index] = res;
			}
			else
			{
				double *dec_values = (double *) malloc(sizeof(double) * nr_class*(nr_class-1)/2);
				predict_label = svm_predict_values(libsvm_model, sample_x, dec_values);
				if(nr_class == 1) 
					ptr_dec_values[sample_index] = 1;
				else
				{
					int multi_num = (nr_class*(nr_class-1))/2;
					for(int i = 0; i < multi_num; i++)
						ptr_dec_values[sample_index * multi_num + i] = dec_values[i];
				}
				free(dec_values);
			}
			ptr_predict_label[sample_index] = predict_label;
		}
	}

	predict_label = ptr_predict_label.transform<int>();
	predict_estimate = ptr_prob_estimates.transform<float>();

	free(sample_x);
	if (ptr_data)
		free(ptr_data);
	if (prob_estimates)
		free(prob_estimates);
}

void libsvm::setKernelType(KernelType ker)
{
	m_kernel_type = ker;
}

void libsvm::setSVMType(SVMType svm_type)
{
	m_svm_type = svm_type;
}

bool libsvm::selfcheck()
{	
	//set svm type
	switch(m_svm_type)
	{
	case LIBSVM_C_SVC:
		{
			m_svm_parameter_str = m_svm_parameter_str + "-s 0";
			break;
		}
	case LIBSVM_NU_SVC:
		{
			m_svm_parameter_str = m_svm_parameter_str + "-s 1";
			break;
		}
	case LIBSVM_ONE_CLASS:
		{
			m_svm_parameter_str = m_svm_parameter_str + "-s 2";
			break;
		}
	default:
		{
			m_svm_parameter_str = m_svm_parameter_str + "-s 2";
		}
	}

	m_svm_parameter_str = m_svm_parameter_str + " ";

	//set SVM kernel type
	switch(m_kernel_type)
	{
	case LIBSVM_LINEAR:
		{
			m_svm_parameter_str = m_svm_parameter_str + "-t 0";
			break;
		}
	case LIBSVM_POLY:
		{
			m_svm_parameter_str = m_svm_parameter_str + "-t 1";
			break;
		}
	case LIBSVM_RBF:
		{
			m_svm_parameter_str = m_svm_parameter_str + "-t 2";
			break;
		}
	case LIBSVM_SIGMOD:
		{
			m_svm_parameter_str = m_svm_parameter_str + "-t 3";
			break;
		}
	default:
		{
			m_svm_parameter_str = m_svm_parameter_str + "-t 0";
		}
	}
	
	//set the flag about whether output confidence
	if (m_predict_paobability)
	{
		m_svm_parameter_str = m_svm_parameter_str + " -b 1";
	}

	return true;
}

void libsvm::setSamplesMat(Matrix<float> samples_label,Matrix<float> samples_feature)
{
	//pass in label and features data
	m_samples_label = samples_label;
	m_samples_label.clone();

	m_samples_feature = samples_feature;
	m_samples_feature.clone();

	//check
	assert(m_samples_feature.rows() == m_samples_label.rows());
	assert(samples_label.cols() == 1);;

	m_samples_num = m_samples_feature.rows();
	m_feature_dim = m_samples_feature.cols();
}

void libsvm::saveSVMModel(const char* model_file)
{
	if (libsvm_model)
	{
		EAGLEEYE_INFO("save libsvm model\n");
		svm_save_model(model_file,libsvm_model);
	}
	else
		EAGLEEYE_ERROR("sorry,libsvm model is empty\n");
}

void libsvm::readSVMModel(const char* model_file)
{
	if (libsvm_model)
	{
		destroyAllResource();
	}
	libsvm_model = svm_load_model(model_file);

	switch(libsvm_model->param.svm_type)
	{
	case C_SVC:
		{
			m_svm_type = LIBSVM_C_SVC;
			break;
		}
	case NU_SVC:
		{
			m_svm_type = LIBSVM_NU_SVC;
			break;
		}
	case ONE_CLASS:
		{
			m_svm_type = LIBSVM_ONE_CLASS;
			break;
		}
	default:
		{
			EAGLEEYE_ERROR("not support... \n");
			return;
		}
	}

	switch(libsvm_model->param.kernel_type)
	{
	case LINEAR:
		{
			m_kernel_type = LIBSVM_LINEAR;
			break;
		}
	case POLY:
		{
			m_kernel_type = LIBSVM_POLY;
			break;
		}
	case RBF:
		{
			m_kernel_type = LIBSVM_RBF;
			break;
		}
	case SIGMOID:
		{
			m_kernel_type = LIBSVM_SIGMOD;
			break;
		}
	default:
		{
			EAGLEEYE_ERROR("not support...\n");
			return;
		}
	}

	if(libsvm_model->probA != NULL && libsvm_model->probB != NULL)
	{
		m_predict_paobability = true;
	}
	else
	{
		m_predict_paobability = false;
	}
}

void libsvm::destroyAllResource()
{
	if (libsvm_model)
	{
		svm_free_and_destroy_model(&libsvm_model);
		svm_destroy_param(&param);
		
		if (prob.y)
		{
			free(prob.y);
			prob.y = NULL;
		}
		
		if (prob.x)
		{
			free(prob.x);
			prob.x = NULL;
		}
		
		if (x_space)
		{
			free(x_space);
			x_space = NULL;
		}
		
		libsvm_model = NULL;
	}
}

int libsvm::getClassNum()
{
	return libsvm_model->nr_class;
}

void libsvm::setProbabilityEstimate(bool flag)
{
	m_predict_paobability = flag;
}

}
