#include "CrossValidation.h"
#include "Print.h"
#include "Variable.h"
#include "Learning/AuxiliaryFunctions.h"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
CrossValidation::CrossValidation(DummyLearner* dummy_learner,const Matrix<float>& samples,const Matrix<float>& labels)
{
	m_mode = K_10_FOLDER;
	m_samples = samples;
	m_labels = labels;
	m_dummy_learner = dummy_learner;

	m_optimum_error = EAGLEEYE_FINF;	//inf
	m_maximum_error = 0.0f;
	m_average_error = 0.0f;
	m_optimum_index = 0;

	m_disturb_order_flag = false;
}
CrossValidation::~CrossValidation()
{

}

void CrossValidation::setCrossValidationMode(CrossValidationMode mode)
{
	m_mode = mode;
}

void CrossValidation::startCrossValidation(const char* folder_path)
{
	switch(m_mode)
	{
	case K_10_FOLDER:
		{
			k10folder(folder_path);
			break;
		}
	case K_5_FOLDER:
		{
			k5folder(folder_path);
			break;
		}
	case LEAST_ONE_OUT:
		{
			leastOneOut(folder_path);
			break;
		}
	}
}

void CrossValidation::k10folder(const char* folder_path)
{
	//split samples to 10 folder
	std::vector<Matrix<float>> folder_10_samples(10);		//sample matrix	in every folder
	std::vector<Matrix<float>> folder_10_labels(10);		//label matrix in every folder
	std::vector<int> folder_10_num(10);						//sample number in every folder
	std::vector<std::vector<int>> folder_10_index(10);		//sample index in every folder
	
	int samples_num = m_samples.rows();	//total sample number
	int samples_dim = m_samples.cols();	//sample dimension

	//finding sample index with different label
	std::map<int,std::vector<int>> labels_map;
	for (int i = 0; i < samples_num; ++i)
	{
		int label = int(m_labels(i,0));
		labels_map[label].push_back(i);
	}

	//total labels number
	int total_labels_num = labels_map.size();

	//disturb sample order in different label by using shuffling algorithm
	if (m_disturb_order_flag)
	{
		for (int label_index = 0; label_index < total_labels_num; ++label_index)
		{
			std::vector<int> order_index_vec = labels_map[label_index];

			//shuffling 
			shuffling(order_index_vec);

			//now,it's unordered
			labels_map[label_index] = order_index_vec;
		}
	}

	//finding sample index in every folder
	for (int label_index = 0; label_index < total_labels_num; ++label_index)
	{
		int num_in_label = labels_map[label_index].size();
		std::vector<int> samples_index_in_label = labels_map[label_index];

		//10 folder
		for (int i = 0; i < 10; ++i)
		{
			int select_index = i;
			while(select_index < num_in_label)
			{
				folder_10_index[i].push_back(samples_index_in_label[select_index]);
				select_index += 10;
			}
		}
	}

	//compute number in every folder
	for (int i = 0; i < 10; ++i)
	{
		folder_10_num[i] = folder_10_index[i].size();
	}

	//copy data to folder
	for (int i = 0; i < 10; ++i)
	{
		int samples_num_in_folder = folder_10_index[i].size();
		folder_10_samples[i] = Matrix<float>(samples_num_in_folder,samples_dim);
		folder_10_labels[i] = Matrix<float>(samples_num_in_folder,1);

		int count = 0;
		while(count < samples_num_in_folder)
		{
			int sample_index = folder_10_index[i][count];

			folder_10_samples[i](Range(count,count + 1),Range(0,samples_dim)).copy(m_samples(Range(sample_index,sample_index + 1),Range(0,samples_dim)));
			folder_10_labels[i](count) = m_labels(sample_index,0);
			count++;
		}
	}

	//training
	for (int pick_index = 0; pick_index < 10; ++pick_index)
	{
		//finding samples number
		int num = 0;
		for (int i = 0; i < 10; ++i)
			if (i != pick_index)
				num += folder_10_num[i];

		Matrix<float> training_samples(num,samples_dim);
		Matrix<float> training_labels(num,1);
		
		//using 9 folders to learn
		int count = 0;
		for (int i = 0; i < 10; ++i)
		{
			if (i != pick_index)
			{
				training_samples(Range(count,count + folder_10_num[i]),Range(0,samples_dim)).copy(folder_10_samples[i]);
				training_labels(Range(count,count + folder_10_num[i]),Range(0,1)).copy(folder_10_labels[i]);

				count += folder_10_num[i];
			}
		}

		//check
		if (m_dummy_learner == NULL)
		{
			EAGLEEYE_ERROR("learner is NULL\n");
			return;
		}

		//learning
		m_dummy_learner->learn(training_samples,training_labels);

		//evaluation
		float error = m_dummy_learner->evalution(folder_10_samples[pick_index],folder_10_labels[pick_index]);

		m_average_error += error;
		if (m_optimum_error > error)
		{
			m_optimum_error = error;
			m_optimum_index = pick_index;
		}

		if (m_maximum_error < error)
			m_maximum_error = error;

		if (folder_path)
		{
			char str[100];
			std::string model_name = itoa(pick_index,str,10);
			m_dummy_learner->save((std::string(folder_path) + model_name).c_str());
		}
	}

	m_average_error /= 10.0f;

	//write file
	FILE* fp = fopen((std::string(folder_path) + "cross_validation.dat").c_str(),"wt+");
	print_info(fp," optimum error %f\n average error %f\n max error %f\n optimum model index %d",m_optimum_error,m_average_error,m_maximum_error,m_optimum_index);
	fclose(fp);
}

void CrossValidation::k5folder(const char* folder_path /* = NULL */)
{
	//split samples to 5 folder
	std::vector<Matrix<float>> folder_5_samples(5);			//sample matrix	in every folder
	std::vector<Matrix<float>> folder_5_labels(5);			//label matrix in every folder
	std::vector<int> folder_5_num(5);						//sample number in every folder
	std::vector<std::vector<int>> folder_5_index(5);		//sample index in every folder

	int samples_num = m_samples.rows();	//total sample number
	int samples_dim = m_samples.cols();	//sample dimension

	//finding sample index with different label
	std::map<int,std::vector<int>> labels_map;
	for (int i = 0; i < samples_num; ++i)
	{
		int label = int(m_labels(i,0));
		labels_map[label].push_back(i);
	}

	//total labels number
	int total_labels_num = labels_map.size();

	//disturb sample order in different label by using shuffling algorithm
	if (m_disturb_order_flag)
	{
		for (int label_index = 0; label_index < total_labels_num; ++label_index)
		{
			std::vector<int> order_index_vec = labels_map[label_index];
		
			//shuffling
			shuffling(order_index_vec);

			//now, it's unordered
			labels_map[label_index] = order_index_vec;
		}
	}

	//finding sample index in every folder
	for (int label_index = 0; label_index < total_labels_num; ++label_index)
	{
		int num_in_label = labels_map[label_index].size();
		std::vector<int> samples_index_in_label = labels_map[label_index];

		//5 folder
		for (int i = 0; i < 5; ++i)
		{
			int select_index = i;
			while(select_index < num_in_label)
			{
				folder_5_index[i].push_back(samples_index_in_label[select_index]);
				select_index += 5;
			}
		}
	}

	//compute number in every folder
	for (int i = 0; i < 5; ++i)
	{
		folder_5_num[i] = folder_5_index[i].size();
	}

	//copy data to folder
	for (int i = 0; i < 5; ++i)
	{
		int samples_num_in_folder = folder_5_index[i].size();
		folder_5_samples[i] = Matrix<float>(samples_num_in_folder,samples_dim);
		folder_5_labels[i] = Matrix<float>(samples_num_in_folder,1);

		int count = 0;
		while(count < samples_num_in_folder)
		{
			int sample_index = folder_5_index[i][count];

			folder_5_samples[i](Range(count,count + 1),Range(0,samples_dim)).copy(m_samples(Range(sample_index,sample_index + 1),Range(0,samples_dim)));
			folder_5_labels[i](count) = m_labels(sample_index,0);
			count++;
		}
	}

	//training
	for (int pick_index = 0; pick_index < 5; ++pick_index)
	{
		//finding samples number
		int num = 0;
		for (int i = 0; i < 5; ++i)
			if (i != pick_index)
				num += folder_5_num[i];

		Matrix<float> training_samples(num,samples_dim);
		Matrix<float> training_labels(num,1);

		//using 4 folders to learn
		int count = 0;
		for (int i = 0; i < 5; ++i)
		{
			if (i != pick_index)
			{
				training_samples(Range(count,count + folder_5_num[i]),Range(0,samples_dim)).copy(folder_5_samples[i]);
				training_labels(Range(count,count + folder_5_num[i]),Range(0,1)).copy(folder_5_labels[i]);

				count += folder_5_num[i];
			}
		}

		//check
		if (m_dummy_learner == NULL)
		{
			EAGLEEYE_ERROR("learner is NULL\n");
			return;
		}

		//learning
		m_dummy_learner->learn(training_samples,training_labels);

		//evaluation
		float error = m_dummy_learner->evalution(folder_5_samples[pick_index],folder_5_labels[pick_index]);

		m_average_error += error;
		if (m_optimum_error > error)
		{
			m_optimum_error = error;
			m_optimum_index = pick_index;
		}

		if (m_maximum_error < error)
			m_maximum_error = error;

		if (folder_path)
		{
			char str[100];
			std::string model_name = itoa(pick_index,str,10);
			m_dummy_learner->save((std::string(folder_path) + model_name).c_str());
		}
	}

	m_average_error /= 5.0f;

	//write file
	FILE* fp = fopen((std::string(folder_path) + "cross_validation.dat").c_str(),"wt+");
	print_info(fp," optimum error %f\n average error %f\n max error %f\n optimum model index %d",m_optimum_error,m_average_error,m_maximum_error,m_optimum_index);
	fclose(fp);
}

void CrossValidation::leastOneOut(const char* model_path)
{

}

int CrossValidation::getOptimumModelIndex()
{
	return m_optimum_index;
}

void CrossValidation::disturbOrder(bool flag)
{
	m_disturb_order_flag = flag;
}

}