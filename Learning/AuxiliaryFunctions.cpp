#include "AuxiliaryFunctions.h"
#include "Variable.h"

namespace eagleeye
{
int getColor(const unsigned char* c)
{
	return c[0] + 256*c[1] + 256 * 256 * c[2];
}
void putColor(unsigned char* c,int cc)
{
	c[0] = cc&0xff; c[1] = (cc>>8)&0xff; c[2] = (cc>>16)&0xff;
}

void resampling(Matrix<float>& samples_representation,Matrix<float>& samples_label)
{
	int samples_num = samples_representation.rows();

	//extent original training samples
	std::map<int,int> label_statistic;
	int count = 0;
	for (int i = 0; i < samples_num; ++i)
	{
		if (label_statistic.find(int(samples_label[i])) == label_statistic.end())
		{
			label_statistic[int(samples_label[i])] = count;
			count++;
		}
	}
	int label_num = label_statistic.size();

	std::vector<int> sample_num_of_every_class(label_num,0);
	std::vector<int> sample_offset_of_every_class(label_num,0);

	for (int i = 0; i < samples_num; ++i)
	{
		int label_index = label_statistic[int(samples_label[i])];
		sample_num_of_every_class[label_index] += 1;
	}
	for (int i = 1; i < label_num; ++i)
	{
		sample_offset_of_every_class[i] = sample_offset_of_every_class[i - 1] + sample_num_of_every_class[i - 1];
	}

	//finding max num of some class
	int max_sample_num_of_some_class = 0;
	for (int i = 0; i < label_num; ++i)
	{
		if (max_sample_num_of_some_class < sample_num_of_every_class[i])
		{
			max_sample_num_of_some_class = sample_num_of_every_class[i];
		}
	}

	Matrix<float> ext_samples_representation(max_sample_num_of_some_class * label_num,samples_representation.cols());
	Matrix<float> ext_samples_label(max_sample_num_of_some_class * label_num,1);
	for (int i = 0; i < label_num; ++i)
	{
		//fill feature data
		float* ext_samples_representation_data = ext_samples_representation.row(max_sample_num_of_some_class * i);
		float* samples_representation_data = samples_representation.row(sample_offset_of_every_class[i]);
		memcpy(ext_samples_representation_data,samples_representation_data,sizeof(float) * sample_num_of_every_class[i] * samples_representation.cols());

		Variable<int> uniform_var = Variable<int>::uniform(0,sample_num_of_every_class[i] - 1);
		for (int m = 0; m < (max_sample_num_of_some_class - sample_num_of_every_class[i]); ++m)
		{
			float* others_ext_samples_representation_data = ext_samples_representation.row(max_sample_num_of_some_class * i + sample_num_of_every_class[i] + m);
			float* others_samples_representation_data = samples_representation.row(sample_offset_of_every_class[i] + uniform_var.var());
			memcpy(others_ext_samples_representation_data,others_samples_representation_data,sizeof(float) * samples_representation.cols());
		}

		//fill label data
		int ground_truth_label = int(samples_label[sample_offset_of_every_class[i]]);
		for (int m = 0; m < max_sample_num_of_some_class; ++m)
		{
			ext_samples_label[max_sample_num_of_some_class * i + m] = float(ground_truth_label);
		}
	}

	samples_representation = ext_samples_representation;
	samples_label = ext_samples_label;
}
}