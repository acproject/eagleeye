#ifndef _OBJECTDETSEMANTICSUPERPIXEL_H_
#define _OBJECTDETSEMANTICSUPERPIXEL_H_

#include "EagleeyeMacro.h"
#include "ObjectDetSymbol.h"
#include "Matrix.h"
#include "Trainer/ShapeDictionaryTrainer.h"
#include "SemanticBOWDescriptorExtractor.h"
#include "Learning/SemanticBagOfWords.h"
#include "Trainer/SemanticBagOfWordsTrainer.h"
#include "Learning/dictionary/Dictionary.h"
#include "Trainer/ExtractFeatureTrainer.h"
#include "ShapeHotPointFeatureDetector.h"
#include <string>
#include <vector>

namespace eagleeye
{
class EAGLEEYE_API ObjectDetSemanticSuperpixel:public ObjectDetSymbol
{
public:
	ObjectDetSemanticSuperpixel(const char* name);
	virtual ~ObjectDetSemanticSuperpixel();

	/**
	 *	@brief get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(ObjectDetSemanticSuperpixel);

	/**
	 *	@brief set descriptor extractor
	 */
	void setDescriptorExtractor(DescriptorExtractor* feature_extractor);

	/**
	 *	@brief we assign -EAGLEEYE_INF for any pixel larger than gray_threshold
	 */
	void setGrayThreshold(float gray_threshold);

	/**
	 *	@brief set gaussian kernel size for smoothing
	 */
	void setGaussianKernelSize(int kernel_size);

	/**
	 *	@brief set symbol trainer
	 */
	void setSymbolTrainer(DictionaryTrainer* dic_trainer,
		SemanticBagOfWordsTrainer* semantic_bagofwords_trainer,
		ExtractFeatureTrainer* ef_trainer);

	/**
	 *	@brief Get this symbol score(probability score)
	 *	@note derived from superclass
	 */
	virtual void* getSymbolScore(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat = Matrix<float>());

	/**
	 *	@brief get symbol feature description
	 *	@note this is semantic feature description
	 */
	virtual Matrix<float> getUnitFeature(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat = Matrix<float>());

	/**
	 *	@brief set this unit data
	 *	@note This function would be called by Grammar Tree implicitly
	 *	derived from superclass
	 */
	virtual void  setUnitData(void* data, SampleState sample_state,void* auxiliary_data);

	/**
	 *	@brief Set/Get this unit parameter
	 *	@note use this two function to set or get all parameter info of this symbol.\n
	 *	grammar tree would call them to save or read model file automatically.
	 */
	virtual void getUnitPara(MemoryBlock& param_block);
	virtual void setUnitPara(MemoryBlock param_block);

	/**
	 *	@brief learning all unknown info of this symbol
	 */
	virtual void learnUnit(const char* parse_file);

	/**
	 *	@brief get output result
	 */
	virtual void getSymbolOutput(std::vector<void*>& output_info);

	/**
	 *	@brief finding some latent info(superpixel label)
	 */
	virtual void findModelLatentInfo(void* info);

protected:
	void loadDictionary(std::string dictionary_file);
	void loadSemanticModels(std::string semantic_model_file);

private:
	Matrix<unsigned char> m_detect_img;
	float m_gray_threshold;									/**< gray threshold*/
	int m_kernel_size;										/**< gaussian kernel size*/

	SemanticBOWDescriptorExtractor* m_semantic_extractor;	/**< semantic feature extractor*/
	DescriptorExtractor* m_feature_extractor;				/**< extract feature from image*/

	DictionaryTrainer* m_dic_trainer;						/**< dictionary trainer*/
	ExtractFeatureTrainer* m_ef_trainer;
	SemanticBagOfWordsTrainer* m_semantic_bow_trainer;
	std::string m_dictionary_file;							/**< dictionary file*/
	std::string m_semantic_model_file;

	Matrix<float> m_dic;									/**< dictionary matrix*/
	
	std::vector<SemanticBagOfWords*> m_semantic_infer;		/**< semantic infer*/

	std::map<int,int> m_superpixel_index_map;				/**< map table between superpixel index and feature index*/
	std::map<int,int> m_superpixel_label_map;				/**< superpixel -> label*/
	Matrix<float> m_output_info;							/**< output info*/

	Matrix<unsigned char> m_annotation_label;				/**< annotation image for training(auxiliary info)*/
	Matrix<int> m_superpixel_img;							/**< superpixel index image(auxiliary info)*/
	int m_superpixel_num;									/**< superpixel number(auxiliary info)*/

	Matrix<float> m_superpixel_score;
	int m_words_num;
	int m_clique_num;
};
}

#endif