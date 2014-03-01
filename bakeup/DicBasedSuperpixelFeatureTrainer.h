#ifndef _DICTIONARYBASEDCLASSIFICATION_H_
#define _DICTIONARYBASEDCLAsSIFICATION_H_

#include "EagleEyeMacro.h"
#include "SymbolTrainer.h"
#include "SignalFactory.h"
#include "DescriptorExtractor.h"
#include "BOWDescriptorExtractorOpenCV.h"

namespace eagleeye
{
class EAGLEEYE_API DicBasedSuperpixelFeatureTrainer:public SymbolTrainer
{
public:
	typedef DicBasedSuperpixelFeatureTrainer					Self;
	typedef SymbolTrainer									Superclass;

	DicBasedSuperpixelFeatureTrainer();
	virtual ~DicBasedSuperpixelFeatureTrainer();
	
	/**
	 *	@brief get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(DicBasedSuperpixelFeatureTrainer);

	/**
	 *	@brief define input and output info signal type
	 */
	EAGLEEYE_INPUT_PORT_TYPE(InfoSignal<std::string>,0,PARSE_FILE_INFO);
	EAGLEEYE_INPUT_PORT_TYPE(InfoSignal<std::string>, 1, DIC_FILE_INFO);
	EAGLEEYE_OUTPUT_PORT_TYPE(InfoSignal<std::string>, 0, SAMPLES_FEATURE_INFO);
	EAGLEEYE_OUTPUT_PORT_TYPE(InfoSignal<std::string>,1,SAMPLES_FEATURE_LABEL_INFO);

	/**
	 *	@brief the concrete code is implemented in this function
	 */
	virtual void train();

// 	/**
// 	 *	@brief set descriptor to be used to analyze sample
// 	 */
// 	void setFeatureDescriptor(BOWDescriptorExtractorOpenCV* descriptor);
// 	
	/**
	 *	@brief set superpixel size
	 */
	void setSuperpixelComplexity(float superpixel_complexity);

	/**
	 *	@brief set sampling number of every superpixel
	 */
	void setSamplingNumOfEverySuperpixel(int num);

	/**
	 *	@brief set minimum area of every superpixel
	 */
	void setSuperpixelAreaLimit(int min_area);

protected:
	/**
	 *	@brief check whether it needs to be processed
	 *	@note if the model parameters file have been existed, it wouldn't
	 *	execute again. It would walk into the next node directly.
	 */
	virtual bool isNeedProcessed();
	
	/**
	 *	@brief check whether some preliminary conditions have been
	 *	satisfied.
	 */
	virtual bool selfcheck();

	/**
	 *	@brief using parse file format two
	 */
	virtual void parseImageAnnotationTrainingSamples();

	/**
	 *	@brief after processing this node, we should clear some 
	 *	temporary variables
	 */
	virtual void clearSomething();
	
	/**
	 *	@brief get the label of every superpixel
	 */
	Matrix<unsigned char> getSuperpixelCentersLabel(const Matrix<int>& superpixel_index,int superpixel_num,const Matrix<unsigned char>& label_annotation);
	
	/**
	 *	@brief using for training
	 */
	void adjustSuperpixel(const Matrix<int>& superpixel,
		const int superpixel_num,
		const Matrix<unsigned char>& label_annotation,
		Matrix<int>& after_superpixel,int& after_superpixel_num,
		Matrix<int>& after_pixel_num_of_every_superpixel,
		Matrix<unsigned char>& after_superpixel_center_label);

private:
	BOWDescriptorExtractorOpenCV* m_bow_descriptor;

	float m_superpixel_complexity;
	Matrix<float> m_samples_description;
	Matrix<float> m_samples_label;
	int m_sampling_num_of_every_superpixel;
	int m_superpixel_area_limit;

	std::string m_trainer_label_info;
	std::string m_trainer_feature_info;
};
}


#endif