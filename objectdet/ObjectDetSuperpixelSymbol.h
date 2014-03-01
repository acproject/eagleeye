#ifndef _OBJECTDETSUPERPIXELSYMBOL_H_
#define _OBJECTDETSUPERPIXELSYMBOL_H_

#include "EagleeyeMacro.h"
#include "ObjectDetSymbol.h"
#include "Matrix.h"
#include "DataPyramid.h"
#include "BOWDescriptorExtractorOpenCV.h"
#include "Learning/libsvm/LibSVM.h"
#include "Trainer/DictionaryTrainer.h"
#include "Trainer/SVMClassifierTrainer.h"
#include "Trainer/ExtractFeatureTrainer.h"

namespace eagleeye
{
/**
 *	@brief find class label of every superpixel
 *	@note output Matrix<float>
 */
class EAGLEEYE_API ObjectDetSuperpixelSymbol:public ObjectDetSymbol
{
public:
	ObjectDetSuperpixelSymbol(const char* name);
	virtual ~ObjectDetSuperpixelSymbol();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(ObjectDetSuperpixelSymbol);
	
	/**
	 *	@brief set descriptor extractor
	 */
	void setDescriptorExtractor(DescriptorExtractor* descriptor);

	/**
	 *	@brief we assign -EAGLEEYE_INF for any pixel lower than gray_threshold
	 */
	void setGrayThreshold(float gray_threshold);

	/**
	 *	@brief set gaussian kernel size for smoothing
	 */
	void setGaussianKernelSize(int kernel_size);

	/**
	 *	@brief set sampling number in every superpixel
	 */
	void setSamplingNumInSuperpixel(int num);

	/**
	 *	@brief set symbol trainer
	 */
	void setSymbolTrainer(DictionaryTrainer* dic_trainer,
		SVMClassifierTrainer* libsvm_trainer,
		ExtractFeatureTrainer* extract_feature_trainer = NULL);

	/**
	 *	@brief Get this symbol score(probability score)
	 *	@note derived from superclass
	 */
	virtual void* getSymbolScore(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat = Matrix<float>());

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
	 *	@brief get symbol feature representation
	 *	@note using for training derived from superclass
	 */
	virtual Matrix<float> getUnitFeature(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat = Matrix<float>());

	/**
	 *	@brief get output result
	 */
	virtual void getSymbolOutput(std::vector<void*>& output_info);

	/**
	 *	@brief finding some latent info(superpixel label)
	 */
	virtual void findModelLatentInfo(void* info);

protected:
	void setFeatureDescriptorBlock(char* data_block);
	void loadDictionary(std::string dictionary_file);
	void loadSVMModel(std::string svm_model_file);

private:
	Matrix<float> m_superpixel_score;						/**< superpixel score matrix*/
	Matrix<unsigned char> m_detect_img;						/**< detected image*/

	BOWDescriptorExtractorOpenCV* m_bow_descriptor;			/**< construct bag of words*/
	DescriptorExtractor* m_descriptor;						/**< extract feature description*/
	
	libsvm m_libsvm;										/**< libsvm classifier*/
	
	float m_gray_threshold;									/**< judge invalid region coarsely*/
	int m_sampling_num_in_superpixel;						/**< the sampling number of every superpixel*/

	DictionaryTrainer* m_dic_trainer;						/**< dictionary trainer*/
	ExtractFeatureTrainer* m_extract_feature_trainer;		/**< extract trainer*/
	SVMClassifierTrainer* m_libsvm_trainer;					/**< svm classifier trainer*/
	std::string m_dictionary_file;							/**< dictionary file*/
	std::string m_classifier_file;							/**< libsvm model file*/

	std::map<int,int> m_superpixel_index_map;				/**< map table between superpixel index and feature index*/
	std::map<int,int> m_superpixel_label_map;				/**< superpixel -> label*/
	Matrix<float> m_output_info;							/**< output info*/

	float m_gauss_kernel_size;								/**< smoothing parameter*/
	Matrix<float> m_whiting_data;							/**< whiting data(mean;var)*/

	Matrix<unsigned char> m_annotation_label;				/**< annotation image for training(auxiliary info)*/
	Matrix<int> m_superpixel_img;							/**< superpixel index image(auxiliary info)*/
	int m_superpixel_num;									/**< superpixel number(auxiliary info)*/

	int m_dic_capacity;
};
}
#endif