#include "ProbabilityDecision.h"
#include "ProbabilityEstimator.h"
#include <vector>
#include <limits>
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
ProbabilityDecision::ProbabilityDecision(ProbabilityEstimator* positive_generator,ProbabilityEstimator* negative_generator,int bins_num)
{
	m_positive_generator = positive_generator;
	m_negative_generator = negative_generator;
	m_bins_num = bins_num;
}
ProbabilityDecision::~ProbabilityDecision()
{

}

void ProbabilityDecision::setBinsNum(int bins_num)
{
	m_bins_num = bins_num;
}

Matrix<float> ProbabilityDecision::decisionMinMisClassificationRate()
{
	float class_min_val,class_max_val;
	findingMinMax(class_min_val,class_max_val);

	float step_val = (class_max_val - class_min_val) / float(m_bins_num);

	std::vector<float> p_temp;

	//class one data region
	std::vector<std::pair<float,float>> regions_record;
	std::pair<float,float> new_region;
	bool flag = false;
	for (int i = 0; i < m_bins_num; ++i)
	{
		float check_sampling = class_min_val + float(i) * step_val;
		float positive_p_val = m_positive_generator->p(check_sampling) * step_val;
		float negative_p_val = m_negative_generator->p(check_sampling) * step_val;
		
		p_temp.push_back(positive_p_val);

		if ((positive_p_val >= negative_p_val) && (flag == false))
		{
			flag = true;
			new_region.first = check_sampling;
		}
		else if(((positive_p_val < negative_p_val) || (i == (m_bins_num - 1))) && (flag == true))
		{
			flag = false;
			new_region.second = check_sampling;
			regions_record.push_back(new_region);
		}
	}

	Matrix<float> ggg(1,p_temp.size(),&p_temp[0],true);
	putToMatlab(ggg,"p_ggg");

	int regions_num = regions_record.size();
	Matrix<float> decision_regions(regions_num,2);
	for (int i = 0; i < regions_num; ++i)
	{
		decision_regions(i,0) = regions_record[i].first;
		decision_regions(i,1) = regions_record[i].second;
	}
	return decision_regions;
}
Matrix<float> ProbabilityDecision::decisionMinExpectedLoss(const Matrix<float>& loss_mat)
{
	float class_min_val,class_max_val;
	findingMinMax(class_min_val,class_max_val);
	float step_val = (class_max_val - class_min_val) / float(m_bins_num);

	//class one data region
	std::vector<std::pair<float,float>> regions_record;
	std::pair<float,float> new_region;
	bool flag = false;
	for (int i = 0; i < m_bins_num; ++i)
	{
		float check_sampling = class_min_val + i * step_val;
		float positive_p_val = m_positive_generator->p(check_sampling) * step_val;
		float negative_p_val = m_negative_generator->p(check_sampling) * step_val;

		float assign_to_possitive_loss = 0.0f;
		assign_to_possitive_loss = positive_p_val * loss_mat(0,0) + negative_p_val * loss_mat(1,0);

		float assign_to_negative_loss = 0.0f;
		assign_to_negative_loss = positive_p_val * loss_mat(0,1) + negative_p_val * loss_mat(1,1);

		if ((assign_to_possitive_loss <= assign_to_negative_loss) &&
			flag == false)
		{
			flag = true;
			new_region.first = check_sampling;
		}
		else if(((assign_to_possitive_loss > assign_to_negative_loss) || (i == (m_bins_num - 1))) && 
			flag == true)
		{
			flag = false;
			new_region.second = check_sampling;
			regions_record.push_back(new_region);
		}
	}

	int regions_num = regions_record.size();
	Matrix<float> decision_regions(regions_num,2);
	for (int i = 0; i < regions_num; ++i)
	{
		decision_regions(i,0) = regions_record[i].first;
		decision_regions(i,1) = regions_record[i].second;
	}
	return decision_regions;
}

Matrix<float> ProbabilityDecision::decisionRejectOption(float reject_probability_threshold)
{
	float class_min_val,class_max_val;
	findingMinMax(class_min_val,class_max_val);

	float step_val = (class_max_val - class_min_val) / float(m_bins_num);
	std::vector<std::pair<float,float>> regions_record;
	std::pair<float,float> new_region;
	bool flag = false;

	for (int i = 0; i < m_bins_num; ++i)
	{
		float check_sampling = class_min_val + i * step_val;
		float positive_p_val = m_positive_generator->p(check_sampling) * step_val;
		float negative_p_val = m_negative_generator->p(check_sampling) * step_val;

		if ((positive_p_val > negative_p_val) && 
			(positive_p_val > reject_probability_threshold) && 
			(flag == false))
		{
			new_region.first = check_sampling;
			flag = true;
		}
		else if(((positive_p_val < negative_p_val) || 
			(positive_p_val < reject_probability_threshold) || (i == (m_bins_num - 1))) && 
			(flag == true))
		{
			new_region.second = check_sampling;
			regions_record.push_back(new_region);
			flag = false;
		}
	}


	int regions_num = regions_record.size();
	Matrix<float> decision_regions(regions_num,2);
	for (int i = 0; i < regions_num; ++i)
	{
		decision_regions(i,0) = regions_record[i].first;
		decision_regions(i,1) = regions_record[i].second;
	}
	return decision_regions;
}

void ProbabilityDecision::findingMinMax(float& class_min_val,float& class_max_val)
{
	Matrix<float> positive_sampling_data = m_positive_generator->getSamplingData();
	float* positive_data = positive_sampling_data.row(0);
	int positive_sampling_num = positive_sampling_data.cols();
	float positive_min_val = std::numeric_limits<float>::max();
	float positive_max_val = std::numeric_limits<float>::min();
	for (int i = 0; i < positive_sampling_num; ++i)
	{
		if (positive_min_val > positive_data[i])
		{
			positive_min_val = positive_data[i];
		}
		if (positive_max_val < positive_data[i])
		{
			positive_max_val = positive_data[i];
		}
	}

	Matrix<float> negative_sampling_data = m_negative_generator->getSamplingData();
	float* negative_data = negative_sampling_data.row(0);
	int negative_sampling_num = negative_sampling_data.cols();
	float negative_min_val = std::numeric_limits<float>::max();
	float negative_max_val = std::numeric_limits<float>::min();

	for (int i = 0; i < negative_sampling_num; ++i)
	{
		if (negative_min_val > negative_data[i])
		{
			negative_min_val = negative_data[i];
		}
		if (negative_max_val < negative_data[i])
		{
			negative_max_val = negative_data[i];
		}
	}

	if (m_bins_num == 0)
	{
		m_bins_num = EAGLEEYE_MAX(positive_sampling_num * 10,negative_sampling_num * 10);
	}

	class_min_val = EAGLEEYE_MIN(positive_min_val,negative_min_val);
	class_max_val = EAGLEEYE_MAX(positive_max_val,negative_max_val);

	float step = (class_max_val - class_min_val) / float(m_bins_num);

	//extent min value and max value
	class_min_val = class_min_val - float(step) * 10.0f;
	class_max_val = class_max_val + float(step) * 10.0f;
}

}