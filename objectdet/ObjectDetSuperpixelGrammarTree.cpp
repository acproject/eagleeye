#include "ObjectDetSuperpixelGrammarTree.h"
namespace eagleeye
{
ObjectDetSuperpixelGrammarTree::ObjectDetSuperpixelGrammarTree(const char* name,const char* model_folder,ObjectDetSymbol* root)
:GrammarTree(name,model_folder,root)
{
	m_root_symbol = root;
}
ObjectDetSuperpixelGrammarTree::~ObjectDetSuperpixelGrammarTree()
{

}

void ObjectDetSuperpixelGrammarTree::parseData(void* data,int width,int height,void* auxiliary_data)
{
	//set image data
	m_gt_root->setModelData(data,EAGLEEYE_UNDEFINED_SAMPLE,auxiliary_data);
	
	//compute score
	void* score_data = m_gt_root->getSymbolScore(SUPERPIXEL_SPACE,INDEPENDENT_SEARCH);

	Matrix<float> score_mat = *((Matrix<float>*)score_data);

	//finding superpixel label with max score
	int num = score_mat.rows();
	int search_length = (score_mat.cols() - SUPERPIXEL_SCORE_OFFSET) / 2;

	for (int i = 0; i < num; ++i)
	{
		float* score_data = score_mat.row(i) + SUPERPIXEL_SCORE_OFFSET;
		float max_score = -EAGLEEYE_FINF;
		float max_score_label = 0;
		for (int label_index = 0; label_index < search_length; ++label_index)
		{
			if (max_score < score_data[label_index * 2 + 1])
			{
				max_score = score_data[label_index * 2 + 1];
				max_score_label = label_index;
			}
		}

		//pass in latent info
		DetSymbolInfo sym_info;
		sym_info.superpixel = *(score_mat.row(i));			//superpixel index
		sym_info.level = *(score_mat.row(i) + 1);			//superpixel level
		sym_info.scale = *(score_mat.row(i) + 2);			//superpixel level scale
		sym_info.label = max_score_label;					//predict label
		sym_info.val = max_score;							//predict score

		m_gt_root->findModelLatentInfo(&sym_info);
	}
}

Matrix<float> ObjectDetSuperpixelGrammarTree::getPredictLabel()
{	
	std::vector<void*> output_info;
	m_gt_root->getModelOutput(output_info);

	Matrix<float> label_img = *((Matrix<float>*)output_info[0]);
	return label_img;
}

void ObjectDetSuperpixelGrammarTree::setRootSymbol(Symbol* root)
{
	m_root_symbol = dynamic_cast<ObjectDetSymbol*>(root);
	GrammarTree::setRootSymbol(root);
}

}