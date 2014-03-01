#ifndef _PCAFOREST_H_
#define _PCAFOREST_H_

#include "EagleeyeMacro.h"
#include "Learning/PCA.h"
#include "Matrix.h"
#include "MatrixMath.h"
#include <vector>
#include <map>

namespace eagleeye
{
enum PCALeafType
{
	GRID_PCA_LEAF,
	RANDOM_PCA_LEAF,
	VARIANCE_RANDOM_PCA_LEAF
};

class EAGLEEYE_API PCAforest
{
public:
	struct _PCALeaf
	{
		int class_label;
		int size;
		int* index;
		Matrix<float> mean_vec;
		Matrix<float> pc;
		float max_err;
		float leaf_distinctive;
	};

	struct _PCATree 
	{
		std::vector<_PCALeaf> pca_leafs;
		float pca_leaf_contri;
		PCALeafType pca_leaf_type;
		int min_pca_leaf_size;
		int max_pca_leaf_size;
		int class_label;
		float confidence;
	};
	
	PCAforest(int leaf_num_of_every_tree = 1);
	~PCAforest();
	
	/**
	 *	@brief train PCA forest
	 */
	void train();

	/**
	 *	@brief using PCA forest to reconstruct data
	 */
	Matrix<float> reconstruct(const Matrix<float>& data,int class_label);

	/**
	 *	@brief using PCA forest to vote
	 */
	void vote(const Matrix<float>& data,Matrix<int>& class_label,Matrix<float>& vote_score);
	
	/**
	 *	@brief configure PCA leaf
	 */
	void configurePCALeaf(PCALeafType pca_leaf_grow_mode,float leaf_contribution,int min_leaf_size,int max_leaf_size);

	/**
	 *	@brief set training samples label and training samples
	 */
	void setTrainingSamples(Matrix<float> samples_label,Matrix<float> samples_mat);
	
	/**
	 *	@brief save or read PCA forest model
	 */
	void savePCAforestModel(const char* model_file);
	void readPCAforestModel(const char* model_file);
	
	/**
	 *	@brief transform to inner label or outer label
	 */
	Matrix<int> transformToInnerLabel(const Matrix<int>& labels);
	Matrix<int> transformToOuterLable(const Matrix<int>& labels);

	std::map<int,int> getInnerLabelMap();
	std::map<int,int> getOuterLabelMap();

protected:
	void generateGridPCA();
	void generateRandomPCA();
	void generateVarianceRandomPCA();
	void destroyAllTrees();
	
	void voteByTree(const _PCATree& tree,const Matrix<float>& data,Matrix<float>& tree_vote_score);

	/**
	 *	@brief construct "Reconstruct" forest
	 */
	void trainReconstructForest();

	/**
	 *	@brief construct "Distinctive" forest
	 */
	void trainDistinctiveForest();

private:
	std::vector<_PCATree> m_pca_trees;		/**< PCA trees*/

	int m_leaf_num_of_every_tree;			/**< leaf number of every tree*/

	std::map<int,int> m_inner_label_map;
	std::map<int,int> m_outer_label_map;	

	std::vector<int> m_offset_every_class;	/**< position offset*/
	std::vector<int> m_num_every_class;		/**< samples number of every class*/

	Matrix<float> m_samples_mat;			/**< training samples*/
	Matrix<float> m_samples_label;			/**< groundtruth label of training samples*/

	int m_class_num;						/**< class number or trees number*/
	
	float m_pca_leaf_contri;				/**< contribution of every leaf*/
	PCALeafType m_pca_leaf_type;			/**< leaf growing mode*/
	int m_pca_min_leaf_size;				/**< min size of every leaf*/
	int m_pca_max_leaf_size;				/**< max size of every leaf*/
};

class EAGLEEYE_API SuperPCAforest
{
	
};

}

#endif