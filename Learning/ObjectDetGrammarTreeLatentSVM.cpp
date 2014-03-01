#include "ObjectDetGrammarTreeLatentSVM.h"
#include <math.h>
#include "Print.h"
#include "EagleeyeIO.h"

namespace eagleeye
{
int head_size = 0;
#define FEATURE_LABEL(ex) (((int *)ex)[0])
#define FEATURE_LABEL_DATA(ex) (((int *)(ex)))
#define FEATURE_LATENT_DATA(ex) (((int *)(ex + sizeof(int))))
#define FEATURE_DATA(ex) (((float *)(ex + sizeof(int) * (head_size))))

ObjectDetGrammarTreeLatentSVM::ObjectDetGrammarTreeLatentSVM(GrammarTreeStructureInfo gt_info,float C,float J)
{
	m_gt_info = gt_info;
	m_C = C;
	m_J = J;
	m_mess_samples = NULL;
	m_sorted_samples = NULL;
	m_collapses = NULL;
	m_default_weight_low_bound = -10;
}

ObjectDetGrammarTreeLatentSVM::~ObjectDetGrammarTreeLatentSVM()
{
	if (m_mess_samples)
	{
		//free all training samples
		for (int i = 0; i < m_samples_num; ++i)
		{
			free(m_mess_samples[i]);
		}

		//free m_mess_samples
		free(m_mess_samples);
	}
	if (m_collapses)
		free(m_collapses);

	if (m_sorted_samples)
		free(m_sorted_samples);
}

void ObjectDetGrammarTreeLatentSVM::learn()
{
	//collapse all samples to collapse samples
	makeCollapse();

	//using collapse samples to learn models
	float* w;
	float* lb;
	if (m_weight.isempty())
	{
		m_weight = Matrix<float>(1,m_feature_dim,float(0.0f));
		w = m_weight.row(0);
		Variable<float> random_weight = Variable<float>::uniform(-1.0f,1.0f);
		for (int i = 0; i < m_feature_dim; ++i)
		{
			w[i] = random_weight.var();
		}
	}
	else
		w = m_weight.row(0);

	if (m_low_weight_bounds.isempty())
	{
		m_low_weight_bounds = Matrix<float>(1,m_feature_dim,m_default_weight_low_bound);
		lb = m_low_weight_bounds.row(0);
	}
	else
		lb = m_low_weight_bounds.row(0);

	//generate one uniform variable
	Variable<float> uniform_var = Variable<float>::uniform(0.0f,1.0f);

	//build permutation for learning out-of-order
	std::vector<int> perm;
	perm.resize(m_collapses_num);

	float prev_loss = 1E9;
	bool converged = false;
	int stop_count = 0;
	int iter_count = 0;
	
	//state for small cache
	std::vector<int> hard_indicator;
	hard_indicator.resize(m_collapses_num,INCACHE);

	//enter learning stage
	while(iter_count < MAX_ITER && !converged)
	{
		iter_count++;
		//check whether it has satisfied some "stop" conditions
		if (iter_count % 10 == 0)
		{
			float loss_info[3];
			float lo = computeLoss(loss_info,m_C,m_J,w);
			float delta = 1.0f - (fabs(prev_loss - lo) / lo);

			if (delta >= DELTA_STOP && iter_count >= MIN_ITER)
			{
				stop_count++;
				if (stop_count > STOP_COUNT)
					converged = true;
			}else if(stop_count > 0)
			{
				stop_count = 0;
			}

			prev_loss = lo;

			EAGLEEYE_INFO("%7.2f%% of max # iterations (loss=%.7f; delta=%.5f; stop count=%d)\n",
				100 * float(iter_count) / float(MAX_ITER),lo,EAGLEEYE_MAX(delta,0.0f),STOP_COUNT - stop_count + 1);

			if (converged)
				break;
		}

		//pick random permutation
		for (int i = 0; i < m_collapses_num; ++i)
		{
			perm[i] = i;
		}

		//building out-of-order
		//why we do this? pick randomly
		for (int swapi = 0; swapi < m_collapses_num; ++swapi)
		{
			int swapj = int(floor(uniform_var.var() * (m_collapses_num - 1 - swapi))) + swapi;

			int tmp = perm[swapi];
			perm[swapi] = perm[swapj];
			perm[swapj] = tmp;
		}

		//count number of examples in the small cache
		//the examples in the small cache are hard example
		//hard_example_num records the number of hard example
		//we could use this value to set the learning rate dynamically
		int hard_example_num = 0;
		for (int i = 0; i < m_collapses_num; ++i)
		{
			if (hard_indicator[i] <= INCACHE)
				hard_example_num++;
		}

		int num_updated = 0;
		for (int swapi = 0; swapi < m_collapses_num; ++swapi)
		{
			//select example
			int collapse_index = perm[swapi];

			//skip if example is not in small cache
			//we skip the easy example
			//When we skip one easy example, we would reduce its hard_indicator. 
			// Why do like this? We would leave it some opportunities to become one hard example.
			if (hard_indicator[collapse_index] > INCACHE)
			{
				hard_indicator[collapse_index]--;
				continue;
			}
			
			_Collapse x = m_collapses[collapse_index];
	
			//learning rate
			//I don't know why set learning rate like this.
			float T = EAGLEEYE_MIN(float(MAX_ITER)/2.0f,float(iter_count) + 10000.0f);
			//随着迭代次数的增多，学习速度逐渐变小。随着困难样本数的增多，学习速率会变大
			//两个因素共同作用
			// possessing the idea of "simulated annealing algorithm"
			float rate_x = hard_example_num * m_C / T;

			//compute max over latent placements
			int max_index = -1;
			float max_val = -EAGLEEYE_FINF;
			for (int m = 0; m < x.num; ++m)
			{
				float val = exScore(x.seq[m],w);
				if (val > max_val)
				{
					max_val = val;
					max_index = m;
				}
			}

			char* ptr = x.seq[max_index];
			int label = FEATURE_LABEL(ptr);
			//see (7.19) in Pattern Recognition and Machine Learning
			if (float(label) * max_val < 1.0f)
			{
				//hard example
				//find
				num_updated++;
				hard_indicator[collapse_index] = 0;
				float* feat_data = FEATURE_DATA(ptr);
				
				int units_num = m_gt_info.units_offset.size();
				std::vector<int> units_size = m_gt_info.units_size;
				std::vector<int> units_offset = m_gt_info.units_offset;
				std::vector<float> units_learnmult = m_gt_info.units_learnmult;

				for (int unit_index = 0; unit_index < units_num; ++unit_index)
				{
					float mult = (label > 0 ? m_J : -1.0f) * rate_x * units_learnmult[unit_index];
					int offset = units_offset[unit_index];

					for (int k = 0; k < units_size[unit_index]; ++k)
					{
						w[offset + k] += mult * feat_data[offset + k];
					}
				}
			}
			else
			{
				//easy example
				if (hard_indicator[collapse_index] == INCACHE)
				{
					Variable<float> random_increase = Variable<float>::uniform(0.0f,1.0f);
					hard_indicator[collapse_index] = MINWAIT + int(random_increase.var() * 50.0f);
				}
				else
					hard_indicator[collapse_index]++;
			}
			
			//periodically regularize the model
			if (iter_count % REGFREQ == 0)
			{
				//apply lowerbounds
				int units_num = m_gt_info.units_offset.size();
				std::vector<int> units_size = m_gt_info.units_size;
				std::vector<int> units_offset = m_gt_info.units_offset;
				std::vector<float> units_regmult = m_gt_info.units_regmult;
				std::vector<float> units_learnmult = m_gt_info.units_learnmult;

				std::vector<int> subtrees_size = m_gt_info.subtrees_size;
				std::vector<int> subtrees_offset = m_gt_info.subtrees_offset;
				int subtrees_num = subtrees_size.size();

				for (int unit_index = 0; unit_index < units_num; ++unit_index)
				{
					int offset = units_offset[unit_index];
					for (int k = 0; k < units_size[unit_index]; ++k)
					{
						w[offset + k] = EAGLEEYE_MAX(w[offset + k],lb[offset + k]);
					}
				}

				float rate_r = 1.0f / T;

#if FULL_L2
				//update model
				for (unit_index = 0; unit_index < units_num; ++unit_index)
				{
					float mult = rate_r * units_regmult[unit_index] * units_learnmult[unit_index];
					mult = pow((1 - mult),REGFREQ);
					int offset = units_offset[unit_index];
					for (int k = 0; k < units_size[unit_index]; ++k)
					{
						w[k] = mult * w[k];
					}
				}
#else
				//assume simple mixture model
				int max_subtree_index = 0;
				float max_subtree_val = 0.0f;
				for (int subtree_index = 0; subtree_index < subtrees_num; ++subtree_index)
				{
					float val = 0.0f;
					int s_offset = subtrees_offset[subtree_index];
					int s_size = subtrees_size[subtree_index];
					for (int s = 0; s < s_size; ++s)
					{
						int unit_index = s_offset + s;
						int offset = units_offset[unit_index];
						float unit_val = 0.0f;
						for (int k = 0; k < units_size[unit_index]; ++k)
						{
							unit_val += w[offset + k] * w[offset + k] * units_regmult[unit_index];
						}

						val += unit_val;
					}
					if (val > max_subtree_val)
					{
						max_subtree_val = val;
						max_subtree_index = subtree_index;
					}
				}

				int max_subtree_offset = subtrees_offset[max_subtree_index];
				int max_subtree_size = subtrees_size[max_subtree_index];
				for (int s = 0; s < max_subtree_size; ++s)
				{
					int unit_index = max_subtree_offset + s;
					int offset = units_offset[unit_index];
					float mult = rate_r * units_regmult[unit_index] * units_learnmult[unit_index];
					mult = pow((1 - mult),REGFREQ);
					for (int k = 0; k < units_size[unit_index]; ++k)
					{
						w[offset + k] = mult * w[offset + k];
					}
				}

#endif
			}
		}
	}

	if (converged)
	{
		EAGLEEYE_INFO("\nTermination criteria reached after %d iterations.\n",iter_count);
	}
	else
	{
		EAGLEEYE_INFO("\nMax iteration count reached.\n",iter_count);
	}
}

int comp(const void* a, const void* b)
{
	//sort by latent data
	int c = memcmp(a,b,head_size * sizeof(int));
	return c;
}

void ObjectDetGrammarTreeLatentSVM::makeCollapse()
{
	m_sorted_samples = (char**) malloc(m_samples_num * sizeof(char*));
	memcpy(m_sorted_samples,m_mess_samples,m_samples_num * sizeof(char*));
	qsort(m_sorted_samples,m_samples_num,sizeof(char*),comp);
	
	m_collapses_num = 1;
	char** check_ptr = m_sorted_samples;
	for (int i = 1; i < m_samples_num; ++i)
	{
		if (memcmp(check_ptr[0],m_sorted_samples[i],head_size * sizeof(int)) != 0)
		{
			m_collapses_num++;
			check_ptr[0] = m_sorted_samples[i];
		}
	}

	m_collapses = (_Collapse*)malloc(sizeof(_Collapse) * m_collapses_num);
	m_collapses[0].seq = m_sorted_samples;
	m_collapses[0].num = 1;
	int collapsed_index = 0;
	for (int j = 1; j < m_samples_num; ++j)
	{
		if (memcmp(m_collapses[collapsed_index].seq[0],m_sorted_samples[j],head_size * sizeof(int)) == 0)
		{
			m_collapses[collapsed_index].num++;
		}
		else
		{
			collapsed_index++;
			m_collapses[collapsed_index].seq = &(m_sorted_samples[j]);
			m_collapses[collapsed_index].num = 1;
		}
	}
}

float ObjectDetGrammarTreeLatentSVM::computeLoss(float out[3],float C,float J,float* w)
{
	float loss = 0.0f;

	std::vector<int> units_size = m_gt_info.units_size;
	std::vector<int> units_offset = m_gt_info.units_offset;
	std::vector<float> units_regmult = m_gt_info.units_regmult;
	int units_num = units_size.size();

	std::vector<int> subtrees_size = m_gt_info.subtrees_size;
	std::vector<int> subtrees_offset = m_gt_info.subtrees_offset;
	int subtrees_num = subtrees_size.size();

	//compute regularization item
#if FULL_L2
	//compute ||w||^2
	for (int unit_index = 0; unit_index < units_num; ++unit_index)
	{
		int offset = units_offset[unit_index];
		for (int k = 0; k < units_size[unit_index]; ++k)
		{
			loss += w[offset + k] * w[offset + k] * units_regmult[unit_index];
		}
	}
#else
	for (int subtree_index = 0; subtree_index < subtrees_num; ++subtree_index)
	{
		float val = 0.0f;
		int s_offset = subtrees_offset[subtree_index];
		int s_size = subtrees_size[subtree_index];
		for (int s = 0; s < s_size; ++s)
		{
			int unit_index = s_offset + s;
			int offset = units_offset[unit_index];
			float unit_val = 0.0f;
			for (int k = 0; k < units_size[unit_index]; ++k)
			{
				unit_val += w[offset + k] * w[offset + k] * units_regmult[unit_index];
			}

			val += unit_val;
		}
		if (val > loss)
		{
			loss = val;
		}
	}
#endif
	loss *= 0.5f;
	out[2] = loss;

	//compute square sum error
	//compute loss from the training data
	for (int s_label = 0; s_label <= 1; ++s_label)
	{
		//which label subset to look at: -1 or 1
		int subset = (s_label * 2) - 1;
		float subset_loss = 0.0f;
		for (int i = 0; i < m_collapses_num; ++i)
		{
			_Collapse x = m_collapses[i];
			
			//only consider examples in the target subset
			char* ptr = x.seq[0];
			if (FEATURE_LABEL(ptr) != subset)
				continue;

			//compute max over latent placements
			int max_index = -1;
			float max_val = -EAGLEEYE_FINF;
			for (int index = 0; index < x.num; ++index)
			{
				float val = exScore(x.seq[index],w);
				if (val > max_val)
				{
					max_val = val;
					max_index = index;
				}
			}

			//compute loss on max
			ptr = x.seq[max_index];
			int ptr_label = FEATURE_LABEL(ptr);
			float mult = C * (ptr_label == 1 ? J : 1);
			subset_loss += mult * EAGLEEYE_MAX(0.0f,1.0f - float(ptr_label) * max_val);
		}

		loss += subset_loss;
		out[s_label] = subset_loss;
	}

	return loss;
}

float ObjectDetGrammarTreeLatentSVM::exScore(const char* ex,float* w)
{
	float val = 0.0f;
	float* feat_data = FEATURE_DATA(ex);
	int units_num = m_gt_info.units_offset.size();
	std::vector<int> units_size = m_gt_info.units_size;
	std::vector<int> units_offset = m_gt_info.units_offset;

	for (int unit_index = 0; unit_index < units_num; ++unit_index)
	{
		float unit_val = 0.0f;
		for (int k = 0; k < units_size[unit_index]; ++k)
		{
			unit_val += w[units_offset[unit_index] + k] * feat_data[units_offset[unit_index] + k];
		}
		val += unit_val;
	}

	return val;
}

void ObjectDetGrammarTreeLatentSVM::setTrainingSamples(const Matrix<int>& labels,const Matrix<int>& latent_variables,const Matrix<float>& features)
{
	m_samples_num = labels.rows();
	m_len = 1 * sizeof(int) + latent_variables.cols() * sizeof(int) + features.cols() * sizeof(float);
	head_size = 1 + latent_variables.cols();
	m_mess_samples = (char**)malloc(m_samples_num * sizeof(char*));
	for (int i = 0; i < m_samples_num; ++i)
	{
		m_mess_samples[i] = (char*)malloc(m_len);
		//write label
		memcpy(FEATURE_LABEL_DATA(m_mess_samples[i]),labels.row(i),sizeof(int));
		//write latent
		memcpy(FEATURE_LATENT_DATA(m_mess_samples[i]),latent_variables.row(i),sizeof(int) * latent_variables.cols());
		//write feature
		memcpy(FEATURE_DATA(m_mess_samples[i]),features.row(i),sizeof(float) * features.cols());
	}

	m_feature_dim = features.cols();
	m_latent_variables_num = latent_variables.cols();
}
void ObjectDetGrammarTreeLatentSVM::loadTrainingSamples(const char* latent_variables_file,const char* samples_file)
{
	Matrix<int> latent_variables; 
	EagleeyeIO::read(latent_variables,latent_variables_file,READ_BINARY_MODE);

	Matrix<float> samples;
	EagleeyeIO::read(samples,samples_file,READ_BINARY_MODE);

	Matrix<int> samples_label = samples(Range(0,samples.rows()),Range(0,1)).transform<int>();
	Matrix<float> samples_feature = samples(Range(0,samples.rows()),Range(1,samples.cols()));

	setTrainingSamples(samples_label,latent_variables,samples_feature);
}

void ObjectDetGrammarTreeLatentSVM::loadWeights(const char* weight_file)
{
	EagleeyeIO::read(m_weight,weight_file,READ_BINARY_MODE);
}

void ObjectDetGrammarTreeLatentSVM::saveWeights(const char* weight_file)
{
	EagleeyeIO::write(m_weight,weight_file,WRITE_BINARY_MODE);
}

void ObjectDetGrammarTreeLatentSVM::setIniWeight(Matrix<float> ini_weight)
{
	m_weight = ini_weight;
	m_weight.clone();
}

Matrix<float> ObjectDetGrammarTreeLatentSVM::getOptimumWeight()
{
	return m_weight;
}

}