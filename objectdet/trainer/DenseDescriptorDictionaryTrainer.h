#ifndef _DENSESIFTDICTIONARYTRAINER_H_
#define _DENSESIFTDICTIONARYTRAINER_H_

#include "EagleeyeMacro.h"
#include "SignalFactory.h"
#include "DescriptorExtractor.h"
#include "DictionaryTrainer.h"
#include "Array.h"
#include "Matrix.h"
#include <string>

namespace eagleeye
{
class EAGLEEYE_API DenseDescriptorDictionaryTrainer:public DictionaryTrainer
{
public:
	DenseDescriptorDictionaryTrainer();
	virtual ~DenseDescriptorDictionaryTrainer();

	/**
	 *	@brief define class identity
	 */
	EAGLEEYE_CLASSIDENTITY(DenseDescriptorDictionaryTrainer);

	/**
	 *	@brief set dense feature detect parameters
	 */
	void setDenseDetectParams(float init_scale = 1,int scale_levels = 1,float scale_mul = 1,int init_xy_step = 10,
		int init_img_bound = 10,int search_radius = 10,bool is_needed_main_dir = false,
		bool vary_xy_step_with_scale = false,
		bool vary_img_bound_with_scale = false);


	/**
	 *	@brief the concrete code is implemented in this function.
	 */
	virtual void train();

	/**
	 *	@brief check whether some preliminary conditions are satisfied
	 */
	virtual bool selfcheck();

protected:
	/**
	 *	@brief if dictionary file has been existed, it wouldn't run "train()" again.
	 */
	virtual bool isNeedProcessed();

	/**
	 *	@brief parse all images
	 */
	void parseImage();

	/**
	 *	@brief after processing this node, we should clear some temporary
	 *	variables
	 */
	virtual void clearSomething();

private:
	float m_init_scale;
	int m_scale_levels;
	float m_scale_mul;
	int m_init_xy_step;
	int m_init_img_bound;
	bool m_vary_xy_step_with_scale;
	bool m_vary_img_bound_with_scale;
	int m_search_radius;
	bool m_is_needed_main_dir;

	Matrix<float> m_dictionary;			/**< dictionary matrix*/
	Matrix<int> m_dictionary_counts;	
};
}

#endif