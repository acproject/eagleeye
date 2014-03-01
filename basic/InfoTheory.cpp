#include "InfoTheory.h"
#include "EagleeyeCore.h"

namespace eagleeye
{
float entropy(const Matrix<float>& proba)
{
	//check
	assert(proba.rows() == 1);

	int disnum = proba.cols();
	const float* proba_ptr = proba.row(0);

	float entropy_val = 0.0f;
	for (int i = 0; i < disnum; ++i)
	{
		entropy_val += proba_ptr[i] * log(proba_ptr[i]);
	}
	entropy_val = -entropy_val / float(EAGLEEYE_LOG2);

	return entropy_val;
}

int normalizeObjectStates(const Matrix<float>& input_vec,Matrix<int>& output_vec)
{
	//check
	assert(input_vec.rows() == 1);

	int min_val = 0;
	int max_val = 0;
	int current_val = 0; 
	int i = 0;
	
	//the number of discrete data 
	int dis_num = input_vec.cols();
	if (output_vec.isempty())
		output_vec = Matrix<int>(1,dis_num,int(0));

	const float* input_vec_data = input_vec.row(0);
	int* output_vec_data = output_vec.row(0);

	min_val = (int)floor(input_vec_data[0]);
	max_val = (int)floor(input_vec_data[0]);
	for (i = 0; i < dis_num; ++i)
	{
		current_val = (int)floor(input_vec_data[i]);
		output_vec_data[i] = current_val;

		if (current_val < min_val)
			min_val = current_val;
		else if(current_val > max_val)
			max_val = current_val;
	}

	for (i = 0; i < dis_num; ++i)
		output_vec_data[i] = output_vec_data[i] - min_val;

	max_val = (max_val - min_val) + 1;

	return max_val;
}

int mergeObjectStates(const Matrix<float>& first_vec,const Matrix<float>& second_vec,Matrix<float>& output_vec)
{
	//check
	assert(first_vec.rows() == 1);
	assert(second_vec.rows() == 1);
	assert(first_vec.cols() == second_vec.cols());

	//number
	int dis_num = first_vec.cols();

	int first_num_states,second_num_states;
	Matrix<int> first_norm_vec;
	first_num_states = normalizeObjectStates(first_vec,first_norm_vec);
	int* first_norm_vec_ptr = first_norm_vec.row(0);

	Matrix<int> second_norm_vec;
	second_num_states = normalizeObjectStates(second_vec,second_norm_vec);
	int* second_norm_vec_ptr = second_norm_vec.row(0);
	
	if (output_vec.isempty())
		output_vec = Matrix<float>(1,dis_num,0.0f);
	float* output_vec_ptr = output_vec.row(0);

	Matrix<int> state_map(second_num_states,first_num_states,int(0));
	int* state_map_ptr = state_map.row(0);
	int state_count = 1;
	int cur_index = 0;
	for (int i = 0; i < dis_num; ++i)
	{	
		cur_index = first_norm_vec_ptr[i] + (second_norm_vec_ptr[i] * first_num_states);
		if (state_map_ptr[cur_index] == 0)
		{
			state_map_ptr[cur_index] = state_count;
			state_count++;
		}
	
		output_vec_ptr[i] = float(state_map_ptr[cur_index]);
	}

	return state_count;
}

JointProbabilityInfo calculateJointProbability(const Matrix<float>& first_vec,const Matrix<float>& second_vec)
{
	//check
	assert(first_vec.rows() == 1);
	assert(second_vec.rows() == 1);
	assert(first_vec.cols() == second_vec.cols());
	int dis_num = first_vec.cols();

	Matrix<int> first_norm_vec;
	int first_num_states = normalizeObjectStates(first_vec,first_norm_vec);
	Matrix<int> first_states_counts_vec(1,first_num_states,int(0));
	int* first_norm_vec_ptr = first_norm_vec.row(0);
	int* first_states_counts_vec_ptr = first_states_counts_vec.row(0);

	Matrix<int> second_norm_vec;
	int second_num_states = normalizeObjectStates(second_vec,second_norm_vec);
	Matrix<int> second_states_counts_vec(1,second_num_states,int(0));
	int* second_norm_vec_ptr = second_norm_vec.row(0);
	int* second_states_counts_vec_ptr = second_states_counts_vec.row(0);

	int joint_num_states = first_num_states * second_num_states;
	Matrix<int> joint_states_counts_mat(second_num_states,first_num_states,int(0));
	int* joint_states_counts_mat_ptr = joint_states_counts_mat.dataptr();

	//optimized for number of FP operations
	for (int i = 0; i < dis_num; ++i)
	{
		first_states_counts_vec_ptr[first_norm_vec_ptr[i]] += 1;
		second_states_counts_vec_ptr[second_norm_vec_ptr[i]] += 1;
		joint_states_counts_mat_ptr[second_norm_vec_ptr[i] * first_num_states + first_norm_vec_ptr[i]] += 1;
	}

	//compute probability
	Matrix<float> first_proba(1,first_num_states,0.0f);
	float* first_proba_ptr = first_proba.row(0);
	for (int i = 0; i < first_num_states; ++i)
	{
		first_proba_ptr[i] = float(first_states_counts_vec_ptr[i]) / float(dis_num);
	}

	Matrix<float> second_proba(1,second_num_states,0.0f);
	float* second_proba_ptr = second_proba.row(0);
	for (int i = 0; i < second_num_states; ++i)
	{
		second_proba_ptr[i] = float(second_states_counts_vec_ptr[i]) / float(dis_num);
	}

	Matrix<float> joint_proba(second_num_states,first_num_states,0.0f);
	float* joint_proba_ptr = joint_proba.dataptr();
	for (int i = 0; i < joint_num_states; ++i)
	{
		joint_proba_ptr[i] = float(joint_states_counts_mat_ptr[i]) / float(dis_num);
	}

	JointProbabilityInfo j_ps;
	j_ps.first_proba_vec = first_proba;
	j_ps.second_proba_vec = second_proba;
	j_ps.joint_proba_mat = joint_proba;

	return j_ps;
}

Matrix<float> calculateProbability(const Matrix<float>& data_vec)
{
	//check
	assert(data_vec.rows() == 1);
	int dis_num = data_vec.cols();

	Matrix<int> norm_data_vec;
	int num_states = normalizeObjectStates(data_vec,norm_data_vec);
	Matrix<int> state_counts_vec(1,num_states,int(0));
	int* state_counts_vec_ptr = state_counts_vec.row(0);
	int* norm_data_vec_ptr = norm_data_vec.row(0);

	//optimized for number of FP operation
	for (int i = 0;i < dis_num; ++i)
	{
		state_counts_vec_ptr[norm_data_vec_ptr[i]] += 1;
	}

	Matrix<float> state_proba_vec(1,num_states,0.0f);
	float* state_proba_vec_ptr = state_proba_vec.row(0);
	for (int i = 0; i < num_states; ++i)
	{
		state_proba_vec_ptr[i] = float(state_counts_vec_ptr[i]) / float(dis_num);
	}

	return state_proba_vec;
}

float calculateEntropy(const Matrix<float>& data_vec)
{
	float entropy_val = 0.0f;
	float temp_val = 0.0f;
	
	Matrix<float> p_info = calculateProbability(data_vec);
	float* p_info_ptr = p_info.row(0);
	int states_num = p_info.cols();

	/*H(X) = - sum p(x) log p(x)*/
	for (int i = 0; i < states_num; ++i)
	{
		temp_val = p_info_ptr[i];

		if (temp_val > 0)
		{
			entropy_val -= temp_val * log(temp_val);
		}
	}

	entropy_val /= float(EAGLEEYE_LOG2);

	return entropy_val;
}

float calculateJointEntropy(const Matrix<float>& first_vec,const Matrix<float>& second_vec)
{
	float joint_entropy_val = 0.0f;
	float temp_val = 0.0f;
	JointProbabilityInfo joint_p_info = calculateJointProbability(first_vec,second_vec);
	int num_joint_states = joint_p_info.joint_proba_mat.rows() * joint_p_info.joint_proba_mat.cols();
	float* joint_p_ptr = joint_p_info.joint_proba_mat.row(0);

	/*H(XY) = - sumx sumy p(xy) log p(xy)*/
	for (int i = 0; i < num_joint_states; ++i)
	{
		temp_val = joint_p_ptr[i];
		if (temp_val > 0.0f)
		{
			joint_entropy_val -= temp_val * log(temp_val);
		}
	}

	joint_entropy_val /= float(EAGLEEYE_LOG2);

	return joint_entropy_val;
}

float calculateConditionalEntorpy(const Matrix<float>& data_vec,const Matrix<float>& condition_vec)
{
	/*
     ** Conditional entropy
     ** H(X|Y) = - sumx sumy p(xy) log p(xy)/p(y)
     */
	float cond_entropy = 0.0f;
	float joint_val = 0.0f;
	float cond_val = 0.0f;
	
	JointProbabilityInfo joint_p_info = calculateJointProbability(data_vec,condition_vec);
	int joint_num_states = joint_p_info.joint_proba_mat.rows() * joint_p_info.joint_proba_mat.cols();
	int first_num_states = joint_p_info.first_proba_vec.cols();
	float* joint_proba_ptr = joint_p_info.joint_proba_mat.row(0);
	float* cond_proba_ptr = joint_p_info.second_proba_vec.row(0);

	/*H(X|Y) = - sumx sumy p(xy) log p(xy)/p(y)*/
	/* to index by numFirstStates use modulus of i
	** to index by numSecondStates use integer division of i by numFirstStates
	*/
	for (int i = 0; i < joint_num_states; ++i)
	{
		joint_val = joint_proba_ptr[i];
		cond_val = cond_proba_ptr[i / first_num_states];

		if ((joint_val > 0) && (cond_val > 0))
		{
			cond_entropy -= joint_val * log(joint_val / cond_val);
		}
	}

	cond_entropy /= float(EAGLEEYE_LOG2);

	return cond_entropy;
}

float mi(const Matrix<float>& data_vec,const Matrix<float>& target_vec)
{
	float mutual_info = 0.0f;
	int first_index,second_index;
	
	JointProbabilityInfo joint_info = calculateJointProbability(data_vec,target_vec);

	/*
	** I(X;Y) = sum sum p(xy) * log (p(xy)/p(x)p(y))
	*/
	float* first_proba_ptr = joint_info.first_proba_vec.row(0);
	float* second_proba_ptr = joint_info.second_proba_vec.row(0);
	float* joint_proba_ptr = joint_info.joint_proba_mat.row(0);

	int num_joint_states = joint_info.joint_proba_mat.rows() * joint_info.joint_proba_mat.cols();
	int num_first_states = joint_info.first_proba_vec.cols();
	for (int i = 0; i < num_joint_states; ++i)
	{
		first_index = i % num_first_states;
		second_index = i / num_first_states;

		if ((joint_proba_ptr[i] > 0) &&
			(first_proba_ptr[first_index] > 0) &&
			(second_proba_ptr[second_index] > 0))
		{
			mutual_info += 
				joint_proba_ptr[i] * log(joint_proba_ptr[i] / first_proba_ptr[first_index] / second_proba_ptr[second_index]);
		}
	}

	mutual_info /= float(EAGLEEYE_LOG2);

	return mutual_info;
}

float cmi(const Matrix<float>& data_vec,const Matrix<float>& target_vec,const Matrix<float>& condition_vec)
{
	float mul_info = 0.0f;
	
	/* I(X;Y|Z) = H(X|Z) - H(X|YZ) */
	Matrix<float> merge_yz;
	mergeObjectStates(target_vec,condition_vec,merge_yz);

	float first_cond_entropy = calculateConditionalEntorpy(data_vec,condition_vec);
	float second_cond_entropy = calculateConditionalEntorpy(data_vec,merge_yz);

	mul_info = first_cond_entropy - second_cond_entropy;

	return mul_info;
}

}