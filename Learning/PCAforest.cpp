#include "PCAforest.h"
#include "Variable.h"
#include "Print.h"
#include "EagleeyeIO.h"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
PCAforest::PCAforest(int leaf_num_of_every_tree)
{
	m_leaf_num_of_every_tree = leaf_num_of_every_tree;
	m_pca_leaf_contri = 0.98f;
	m_pca_leaf_type = RANDOM_PCA_LEAF;
	m_pca_min_leaf_size = 5;
	m_pca_max_leaf_size = 100;
	m_class_num = 0;
}

PCAforest::~PCAforest()
{
	destroyAllTrees();
}

void PCAforest::generateRandomPCA()
{
	int feat_dim = m_samples_mat.cols();

	Variable<int> random_pca_leaf_size = Variable<int>::uniform(m_pca_min_leaf_size,m_pca_max_leaf_size);
	Variable<int> random_pca_feat_pick = Variable<int>::uniform(0,feat_dim);
	
	m_pca_trees.resize(m_class_num);
	
	for (int class_index = 0; class_index < m_class_num; ++class_index)
	{
		m_pca_trees[class_index].min_pca_leaf_size = m_pca_min_leaf_size;
		m_pca_trees[class_index].max_pca_leaf_size = m_pca_max_leaf_size;
		m_pca_trees[class_index].pca_leaf_contri = m_pca_leaf_contri;
		m_pca_trees[class_index].pca_leaf_type = m_pca_leaf_type;
		m_pca_trees[class_index].pca_leafs.resize(m_leaf_num_of_every_tree);
		m_pca_trees[class_index].class_label = class_index;
		m_pca_trees[class_index].confidence = 1.0f;

		for (int i = 0; i < m_leaf_num_of_every_tree; ++i)
		{
			int random_pca_dim = random_pca_leaf_size.var();
			m_pca_trees[class_index].pca_leafs[i].class_label = class_index;
			m_pca_trees[class_index].pca_leafs[i].size = random_pca_dim;
			m_pca_trees[class_index].pca_leafs[i].index = new int[random_pca_dim];

			for (int m = 0; m < random_pca_dim; ++m)
			{
				m_pca_trees[class_index].pca_leafs[i].index[m] = random_pca_feat_pick.var();
			}
		}
	}
}

void PCAforest::generateGridPCA()
{
	int feat_dim = m_samples_mat.cols();
	int aver_leaf_feat_dim = feat_dim / m_leaf_num_of_every_tree;

	m_pca_trees.resize(m_class_num);

	for (int class_index = 0; class_index < m_class_num; ++class_index)
	{
		m_pca_trees[class_index].min_pca_leaf_size = aver_leaf_feat_dim;
		m_pca_trees[class_index].max_pca_leaf_size = aver_leaf_feat_dim;
		m_pca_trees[class_index].class_label = class_index;
		m_pca_trees[class_index].pca_leaf_type = m_pca_leaf_type;
		m_pca_trees[class_index].pca_leaf_contri = m_pca_leaf_contri;
		m_pca_trees[class_index].confidence = 1.0f;

		for (int leaf_index = 0; leaf_index < m_leaf_num_of_every_tree; ++leaf_index)
		{
			int leaf_feat_dim = aver_leaf_feat_dim;
			if (leaf_index == m_leaf_num_of_every_tree - 1)
			{
				leaf_feat_dim = feat_dim - aver_leaf_feat_dim * (m_leaf_num_of_every_tree - 1);
			}
			m_pca_trees[class_index].pca_leafs[leaf_index].class_label = class_index;
			m_pca_trees[class_index].pca_leafs[leaf_index].size = leaf_feat_dim;
			m_pca_trees[class_index].pca_leafs[leaf_index].index = new int[leaf_feat_dim];

			for (int m = 0; m < leaf_feat_dim; ++m)
			{
				m_pca_trees[class_index].pca_leafs[leaf_index].index[m] = aver_leaf_feat_dim * leaf_index + m;
			}
		}
	}
}

void PCAforest::generateVarianceRandomPCA()
{
	int rows = m_samples_mat.rows();
	int cols = m_samples_mat.cols();
	int feat_dim = cols;

	Matrix<float> variance_mat(1,cols,float(0.0f));
	
	//compute variance
	float total_variance = 0;
	for (int j = 0; j < cols; ++j)
	{
		variance_mat(j) = variance(m_samples_mat(Range(0,rows),Range(j,j+1)));
		total_variance += variance_mat(j);
	}
	
	//construct discrete distribution by variance
	DynamicArray<int> discrete_loc(cols);
	DynamicArray<float> distribution(cols);
	for (int i = 0; i < cols; ++i)
	{
		discrete_loc(i) = i;
		distribution(i) = variance_mat(i) / total_variance;
	}

	Variable<int> random_pca_leaf_size = Variable<int>::uniform(m_pca_min_leaf_size,m_pca_max_leaf_size);
	Variable<int> random_pca_feat_pick = Variable<int>::discreteDis(discrete_loc,distribution);

	m_pca_trees.resize(m_class_num);

	for (int class_index = 0; class_index < m_class_num; ++class_index)
	{
		m_pca_trees[class_index].min_pca_leaf_size = m_pca_min_leaf_size;
		m_pca_trees[class_index].max_pca_leaf_size = m_pca_max_leaf_size;
		m_pca_trees[class_index].pca_leaf_contri = m_pca_leaf_contri;
		m_pca_trees[class_index].pca_leaf_type = m_pca_leaf_type;
		m_pca_trees[class_index].pca_leafs.resize(m_leaf_num_of_every_tree);
		m_pca_trees[class_index].class_label = class_index;
		m_pca_trees[class_index].confidence = 1.0f;

		for (int i = 0; i < m_leaf_num_of_every_tree; ++i)
		{
			int random_pca_dim = random_pca_leaf_size.var();
			m_pca_trees[class_index].pca_leafs[i].class_label = class_index;
			m_pca_trees[class_index].pca_leafs[i].size = random_pca_dim;
			m_pca_trees[class_index].pca_leafs[i].index = new int[random_pca_dim];

			for (int m = 0; m < random_pca_dim; ++m)
			{
				m_pca_trees[class_index].pca_leafs[i].index[m] = random_pca_feat_pick.var();
			}
		}
	}
}

void PCAforest::train()
{
	//generate PCA leafs basic info
	switch(m_pca_leaf_type)
	{
	case GRID_PCA_LEAF:
		{
			generateGridPCA();
			break;
		}
	case RANDOM_PCA_LEAF:
		{
			generateRandomPCA();
			break;
		}
	case VARIANCE_RANDOM_PCA_LEAF:
		{
			generateVarianceRandomPCA();
			break;
		}
	default:
		{
			generateRandomPCA();
			break;
		}
	}

	//train reconstruct PCA forest
	trainReconstructForest();
	
	//train distinctive PCA forest
	trainDistinctiveForest();
}

void PCAforest::setTrainingSamples(Matrix<float> samples_label,Matrix<float> samples_mat)
{
	int samples_num = samples_label.rows();
	for (int i = 0; i < samples_num; ++i)
	{
		if (m_inner_label_map.find(int(samples_label(i))) == m_inner_label_map.end())
		{
			int index = m_inner_label_map.size();
			m_inner_label_map[int(samples_label(i))] = index;
			m_outer_label_map[index] = int(samples_label(i));
		}
	}

	//reassign label
	//get samples number of every class
	m_class_num = m_inner_label_map.size();
	m_offset_every_class.resize(m_class_num,0);
	m_num_every_class.resize(m_class_num,0);
	for (int i = 0; i < samples_num; ++i)
	{
		m_num_every_class[m_inner_label_map[int(samples_label(i))]] += 1;
	}

	for (int i = 1; i < m_class_num; ++i)
	{
		m_offset_every_class[i] = m_num_every_class[i - 1] + m_offset_every_class[i - 1];
	}

	m_samples_label = Matrix<float>(samples_num,1);
	m_samples_mat = Matrix<float>(samples_num,samples_mat.cols());

	std::vector<int> count_every_class;
	count_every_class.resize(m_class_num,0);
	for (int i = 0; i < samples_num; ++i)
	{
		int inner_class_label =  m_inner_label_map[int(samples_label(i))];
		m_samples_label(i) = float(inner_class_label);
		float* ptr_offset = m_samples_mat.row(m_offset_every_class[inner_class_label] + count_every_class[inner_class_label]);
		memcpy(ptr_offset,samples_mat.row(i),sizeof(float) * samples_mat.cols());

		count_every_class[inner_class_label] = count_every_class[inner_class_label] + 1;
	}
}

void PCAforest::vote(const Matrix<float>& data,Matrix<int>& class_label,Matrix<float>& vote_score)
{
	int data_num = data.rows();
	int feat_dim = data.cols();

	class_label = Matrix<int>(data_num,1,int(0));
	vote_score = Matrix<float>(data_num,m_class_num,float(0.0f));;
	
	int pca_tree_index = 0;
	std::vector<_PCATree>::iterator pca_tree_iter,pca_tree_iend(m_pca_trees.end());
	for (pca_tree_iter = m_pca_trees.begin(); pca_tree_iter != pca_tree_iend; ++pca_tree_iter)
	{
		//class label of this tree
		int pca_tree_class_label = (*pca_tree_iter).class_label;
		_PCATree pca_tree = (*pca_tree_iter);

		EAGLEEYE_INFO("class label %d,PCA tree %d has leafs number %d\n",pca_tree_class_label,pca_tree_index,(*pca_tree_iter).pca_leafs.size());
		
		Matrix<float> tree_vote_score;
		voteByTree(pca_tree,data,tree_vote_score);
		
		for (int data_index = 0; data_index < data_num; ++data_index)
		{
			float* vote_score_data = vote_score.row(data_index);
			float* tree_vote_score_data = tree_vote_score.row(data_index);
			vote_score_data[pca_tree_class_label] = tree_vote_score_data[pca_tree_class_label] * pca_tree.confidence;
		}

		//tree index add one
		pca_tree_index++;
	}

	//judge
	for (int data_index = 0; data_index < data_num; ++data_index)
	{
		float max_vote_num = 0.0f;
		int max_vote_index = 0;
		float* class_vote_data = vote_score.row(data_index);
		for (int class_index = 0; class_index< m_class_num; ++class_index)
		{
			if (max_vote_num< class_vote_data[class_index])
			{
				max_vote_num = class_vote_data[class_index];
				max_vote_index = class_index;
			}
		}

		class_label(data_index) = max_vote_index;
	}
}

void PCAforest::savePCAforestModel(const char* model_file)
{
	EagleeyeIO pca_forest_model_io;
	pca_forest_model_io.createWriteHandle(model_file,false,WRITE_BINARY_MODE);
	
	//write class number
	pca_forest_model_io.write(m_class_num);

	//write PCA trees
	std::vector<_PCATree>::iterator tree_iter,tree_iend(m_pca_trees.end());
	for (tree_iter = m_pca_trees.begin(); tree_iter != tree_iend; ++tree_iter)
	{
		std::vector<_PCALeaf> pca_leafs = (*tree_iter).pca_leafs;

		//write PCA leafs number of this tree
		pca_forest_model_io.write(pca_leafs.size());
		
		//write PCA leafs of this tree
		std::vector<_PCALeaf>::iterator leaf_iter,leaf_iend(pca_leafs.end());
		for (leaf_iter = pca_leafs.begin(); leaf_iter != leaf_iend; ++leaf_iter)
		{
			_PCALeaf leaf = *leaf_iter;
			pca_forest_model_io.write(leaf.class_label);
			pca_forest_model_io.write(leaf.size);
			pca_forest_model_io.write(leaf.index,sizeof(int) * leaf.size);
			pca_forest_model_io.write(leaf.mean_vec);
			pca_forest_model_io.write(leaf.pc);
			pca_forest_model_io.write(leaf.max_err);
			pca_forest_model_io.write(leaf.leaf_distinctive);
		}

		//write other info of this tree
		pca_forest_model_io.write((*tree_iter).pca_leaf_contri);
		pca_forest_model_io.write((int)(*tree_iter).pca_leaf_type);
		pca_forest_model_io.write((*tree_iter).min_pca_leaf_size);
		pca_forest_model_io.write((*tree_iter).max_pca_leaf_size);
		pca_forest_model_io.write((*tree_iter).class_label);
		pca_forest_model_io.write((*tree_iter).confidence);
	}

	//write inner label map
	std::map<int,int>::iterator inner_iter,inner_iend(m_inner_label_map.end());
	for (inner_iter = m_inner_label_map.begin(); inner_iter != inner_iend; ++inner_iter)
	{
		pca_forest_model_io.write(inner_iter->first);
		pca_forest_model_io.write(inner_iter->second);
	}

	//write outer label map
	std::map<int,int>::iterator outer_iter,outer_iend(m_outer_label_map.end());
	for (outer_iter = m_outer_label_map.begin(); outer_iter != outer_iend; ++outer_iter)
	{
		pca_forest_model_io.write(outer_iter->first);
		pca_forest_model_io.write(outer_iter->second);
	}

	pca_forest_model_io.destroyHandle();
}

void PCAforest::readPCAforestModel(const char* model_file)
{
	//destroy all leafs
	destroyAllTrees();

	//load leafs from file
	EagleeyeIO pca_forest_model_io;
	pca_forest_model_io.createReadHandle(model_file,READ_BINARY_MODE);

	//read class number
	pca_forest_model_io.read(m_class_num);
	
	//read PCA trees
	m_pca_trees.resize(m_class_num);

	std::vector<_PCATree>::iterator tree_iter,tree_iend(m_pca_trees.end());
	for (tree_iter = m_pca_trees.begin(); tree_iter != tree_iend; ++tree_iter)
	{
		std::vector<_PCALeaf> pca_leafs;

		//read PCA leafs number of this tree
		int leaf_num;
		pca_forest_model_io.read(leaf_num);
		pca_leafs.resize(leaf_num);

		//read PCA leafs of this tree
		for (int leaf_index = 0; leaf_index < leaf_num; ++leaf_index)
		{
			pca_forest_model_io.read(pca_leafs[leaf_index].class_label);
			pca_forest_model_io.read(pca_leafs[leaf_index].size);

			int index_size;
			void* index_ptr;
			pca_forest_model_io.read(index_ptr,index_size);
			pca_leafs[leaf_index].index = new int[pca_leafs[leaf_index].size];
			memcpy(pca_leafs[leaf_index].index,index_ptr,sizeof(int) * pca_leafs[leaf_index].size);

			pca_forest_model_io.read(pca_leafs[leaf_index].mean_vec);
			pca_forest_model_io.read(pca_leafs[leaf_index].pc);
			pca_forest_model_io.read(pca_leafs[leaf_index].max_err);
			pca_forest_model_io.read(pca_leafs[leaf_index].leaf_distinctive);
		}
		
		(*tree_iter).pca_leafs = pca_leafs;

		//read other information of this tree
		pca_forest_model_io.read((*tree_iter).pca_leaf_contri);
		int mode;
		pca_forest_model_io.read(mode);
		(*tree_iter).pca_leaf_type = (PCALeafType)mode;
		pca_forest_model_io.read((*tree_iter).min_pca_leaf_size);
		pca_forest_model_io.read((*tree_iter).max_pca_leaf_size);
		pca_forest_model_io.read((*tree_iter).class_label);
		pca_forest_model_io.read((*tree_iter).confidence);
	}

	//read inner label map
	for (int inner_index = 0; inner_index < m_class_num; ++inner_index)
	{
		int left_label,right_label;
		pca_forest_model_io.read(left_label);
		pca_forest_model_io.read(right_label);
		m_inner_label_map[left_label] = right_label;
	}

	//read outer label map
	for (int outer_index = 0; outer_index < m_class_num; ++outer_index)
	{
		int left_label,right_label;
		pca_forest_model_io.read(left_label);
		pca_forest_model_io.read(right_label);
		m_outer_label_map[left_label] = right_label;
	}

	pca_forest_model_io.destroyHandle();
}

void PCAforest::configurePCALeaf(PCALeafType pca_leaf_grow_mode,float leaf_contribution,int min_leaf_size,int max_leaf_size)
{
	m_pca_leaf_type = pca_leaf_grow_mode;
	m_pca_leaf_contri = leaf_contribution;
	m_pca_min_leaf_size = min_leaf_size;
	m_pca_max_leaf_size = max_leaf_size;
}

void PCAforest::destroyAllTrees()
{
	std::vector<_PCATree>::iterator tree_iter,tree_iend(m_pca_trees.end());
	for(tree_iter = m_pca_trees.begin(); tree_iter != tree_iend; ++tree_iter)
	{
		std::vector<_PCALeaf>::iterator leaf_iter,leaf_iend((*tree_iter).pca_leafs.end());
		for (leaf_iter = (*tree_iter).pca_leafs.begin(); leaf_iter != leaf_iend; ++leaf_iter)
		{
			delete [](*leaf_iter).index;
		}
	}

	m_pca_trees.clear();
}

Matrix<int> PCAforest::transformToInnerLabel(const Matrix<int>& labels)
{
	//labels must be column matrix
	assert(labels.cols() == 1);

	int rows = labels.rows();
	Matrix<int> inner_labels(rows,1);

	for (int i = 0; i < rows; ++i)
	{
		inner_labels(i) = m_inner_label_map[labels(i)];	
	}

	return inner_labels;
}

Matrix<int> PCAforest::transformToOuterLable(const Matrix<int>& labels)
{
	//labels must be column matrix
	assert(labels.cols() == 1);

	int rows = labels.rows();
	Matrix<int> outer_labels(rows,1);

	for (int i = 0; i < rows; ++i)
	{
		outer_labels(i) = m_outer_label_map[labels(i)];
	}

	return outer_labels;
}

std::map<int,int> PCAforest::getInnerLabelMap()
{
	return m_inner_label_map;
}

std::map<int,int> PCAforest::getOuterLabelMap()
{
	return m_outer_label_map;
}

void PCAforest::voteByTree(const _PCATree& tree,const Matrix<float>& data,Matrix<float>& tree_vote_score)
{
	int data_num = data.rows();
	int feat_dim = data.cols();

	tree_vote_score = Matrix<float>(data_num,m_class_num,float(0.0f));

	//class label of this tree
	int tree_class_label = tree.class_label;

	std::vector<_PCALeaf> tree_pca_leafs = tree.pca_leafs;
	std::vector<_PCALeaf>::iterator pca_leaf_iter,pca_leaf_iend(tree_pca_leafs.end());
	for (pca_leaf_iter = tree_pca_leafs.begin(); pca_leaf_iter != pca_leaf_iend; ++pca_leaf_iter)
	{
		_PCALeaf pca_leaf = *pca_leaf_iter;

		//////////////////////////////////////////////////////////////////////////
		int pca_size = pca_leaf.size;
		int* pca_index = pca_leaf.index;
		Matrix<float> pca_mean_vec = pca_leaf.mean_vec;
		Matrix<float> pca_principal_component = pca_leaf.pc;
		float pca_reconstruct_max_err = pca_leaf.max_err;
		//////////////////////////////////////////////////////////////////////////

		Matrix<float> sub_data(data_num,pca_size);
		for (int data_index = 0; data_index < data_num; ++data_index)
		{
			const float* data_data = data.row(data_index);
			float* sub_data_data = sub_data.row(data_index);
			for (int f_index = 0; f_index < pca_size; ++f_index)
			{
				sub_data_data[f_index] = data_data[pca_index[f_index]];
			}
		}

		Matrix<float> pca_decompose_coe = PCA::decompose(sub_data,pca_mean_vec,pca_principal_component);
		Matrix<float> pca_reconstruct_data = PCA::reconstruct(pca_decompose_coe,pca_mean_vec,pca_principal_component);
		Matrix<float> pca_reconstruct_err = PCA::reconstructErr(sub_data,pca_reconstruct_data);

		for (int data_index = 0; data_index < data_num; ++data_index)
		{
			float* tree_vote_score_data = tree_vote_score.row(data_index);
			if (pca_reconstruct_err(data_index) <= pca_reconstruct_max_err)
			{
				tree_vote_score_data[tree_class_label] += 1.0f;
			}
		}
	}
}

void PCAforest::trainReconstructForest()
{
	EAGLEEYE_INFO("training reconstruct PCA forest");

	int pca_tree_index = 0;
	std::vector<_PCATree>::iterator pca_tree_iter,pca_tree_iend(m_pca_trees.end());
	for (pca_tree_iter = m_pca_trees.begin(); pca_tree_iter != pca_tree_iend; ++pca_tree_iter)
	{
		//class label of this tree
		int tree_class_label = (*pca_tree_iter).class_label;

		EAGLEEYE_INFO("class label %d,PCA tree %d has leafs number %d\n",tree_class_label,pca_tree_index,(*pca_tree_iter).pca_leafs.size());

		int pca_leaf_index = 0;
		std::vector<_PCALeaf>::iterator pca_leaf_iter,pca_leaf_iend((*pca_tree_iter).pca_leafs.end());
		for (pca_leaf_iter = (*pca_tree_iter).pca_leafs.begin(); pca_leaf_iter != pca_leaf_iend; ++pca_leaf_iter)
		{
			//extract sub matrix for PCA
			_PCALeaf pca_leaf = *pca_leaf_iter;

			EAGLEEYE_INFO("PCA leaf %d of tree %d,leaf size %d \n",pca_leaf_index,pca_tree_index,pca_leaf.size);

			Matrix<float> sub_feat_mat(m_num_every_class[tree_class_label],pca_leaf.size);

			for (int sample_index_of_class = 0; sample_index_of_class < m_num_every_class[tree_class_label]; ++sample_index_of_class)
			{
				float* sub_feat_mat_data = sub_feat_mat.row(sample_index_of_class);
				float* samples_mat_data = m_samples_mat.row(m_offset_every_class[tree_class_label] + sample_index_of_class);
				for (int f_index = 0; f_index < pca_leaf.size; ++f_index)
				{
					sub_feat_mat_data[f_index] = samples_mat_data[pca_leaf.index[f_index]];
				}
			}

			//build PCA
			PCA pca_analyze(m_pca_leaf_contri);

			EAGLEEYE_INFO("pca computing... \n");

			print_elapsed_time();
			pca_analyze.compute(sub_feat_mat);

			Matrix<float> mean_vec = pca_analyze.pcaMeanVec();
			Matrix<float> principal_component = pca_analyze.pcaComponents();

			//reconstruct original data and compute reconstruct error
			Matrix<float> decompose_coef = pca_analyze.decompose(sub_feat_mat,mean_vec,principal_component);
			Matrix<float> reconstruct_sub_feat_mat = pca_analyze.reconstruct(decompose_coef,mean_vec,principal_component);
			print_elapsed_time();

			float max_err = 0;
			int rows = sub_feat_mat.rows();
			int cols = sub_feat_mat.cols();
			for (int index = 0; index < rows; ++index)
			{
				float* sub_feat_mat_data = sub_feat_mat.row(index);
				float* reconstruct_sub_feat_mat_data = reconstruct_sub_feat_mat.row(index);
				float err = 0;
				for (int feat_index = 0; feat_index < cols; ++feat_index)
				{
					err += abs(sub_feat_mat_data[feat_index] - reconstruct_sub_feat_mat_data[feat_index]);
				}

				if (max_err < err)
				{
					max_err = err;
				}
			}

			(*pca_leaf_iter).mean_vec = mean_vec;
			(*pca_leaf_iter).pc = principal_component;
			(*pca_leaf_iter).max_err = max_err;

			//leaf index add one
			pca_leaf_index++;
		}

		//tree index add one
		pca_tree_index++;
	}
}

void PCAforest::trainDistinctiveForest()
{
	EAGLEEYE_INFO("training distinctive PCA forest");
}

Matrix<float> PCAforest::reconstruct(const Matrix<float>& data,int class_label)
{
	_PCATree my_tree = m_pca_trees[class_label];

	int data_num = data.rows();
	int feat_dim = data.cols();

	Matrix<float> reconstruct_data = Matrix<float>(data_num,feat_dim,float(0.0f));

	std::vector<_PCALeaf> pca_leafs = my_tree.pca_leafs;
	int pca_leafs_num = pca_leafs.size();
	for (int leaf_index = 0; leaf_index < pca_leafs_num; ++leaf_index)
	{
		_PCALeaf pca_leaf = pca_leafs[leaf_index];

		//////////////////////////////////////////////////////////////////////////
		int pca_size = pca_leaf.size;
		int* pca_index = pca_leaf.index;
		Matrix<float> pca_mean_vec = pca_leaf.mean_vec;
		Matrix<float> pca_principal_component = pca_leaf.pc;
		//////////////////////////////////////////////////////////////////////////

		//extract sub data from data
		Matrix<float> sub_data(data_num,pca_size);
		for (int data_index = 0; data_index < data_num; ++data_index)
		{
			const float* data_data = data.row(data_index);
			float* sub_data_data = sub_data.row(data_index);
			for (int f_index = 0; f_index < pca_size; ++f_index)
			{
				sub_data_data[f_index] = data_data[pca_index[f_index]];
			}
		}

		Matrix<float> pca_decompose_coe = PCA::decompose(sub_data,pca_mean_vec,pca_principal_component);
		Matrix<float> pca_reconstruct_data = PCA::reconstruct(pca_decompose_coe,pca_mean_vec,pca_principal_component);
		
		//fill sub data to data
		for (int data_index = 0; data_index < data_num; ++data_index)
		{
			float* reconstruct_data_data = reconstruct_data.row(data_index);
			float* sub_data_data = sub_data.row(data_index);
			for (int f_index = 0; f_index < pca_size; ++f_index)
			{
				reconstruct_data_data[pca_index[f_index]] += sub_data_data[f_index];
			}
		}
	}

	std::vector<int> count_statistic(feat_dim,0);
	for (int leaf_index = 0; leaf_index < pca_leafs_num; ++leaf_index)
	{
		_PCALeaf pca_leaf = pca_leafs[leaf_index];

		//////////////////////////////////////////////////////////////////////////
		int pca_size = pca_leaf.size;
		int* pca_index = pca_leaf.index;
		//////////////////////////////////////////////////////////////////////////
		
		for (int f_index = 0; f_index < pca_size; ++f_index)
		{
			count_statistic[pca_index[f_index]] += 1;
		}
	}

	//adjust reconstruct data
	for (int data_index = 0; data_index < data_num; ++data_index)
	{
		float* reconstruct_data_data = reconstruct_data.row(data_index);
		for (int f_index = 0; f_index < feat_dim; ++f_index)
		{
			if (count_statistic[f_index] != 0)
			{
				reconstruct_data_data[f_index] /=float(count_statistic[f_index]);
			}
		}
	}

	return reconstruct_data;
}

}