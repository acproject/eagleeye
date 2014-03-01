#ifndef _SHAPEDICTIONARYTRAINER_H_
#define _SHAPEDICTIONARYTRAINER_H_

#include "EagleeyeMacro.h"
#include "SignalFactory.h"
#include "DictionaryTrainer.h"
#include "Array.h"
#include "Matrix.h"
#include <string>

namespace eagleeye
{
class EAGLEEYE_API ShapeDictionaryTrainer:public DictionaryTrainer
{
public:
	ShapeDictionaryTrainer();
	virtual ~ShapeDictionaryTrainer();

	/**
	 *	@brief define class identity
	 */
	EAGLEEYE_CLASSIDENTITY(ShapeDictionaryTrainer);

	/**
	 *	@brief set random jitter switch
	 *	@note if it's true, we would sample points around edges. Otherwise,
	 *	it could only sample points at edges.
	 */
	void setRandomJitterSwitch(bool flag,float jitter_degree,int jitter_num = 10);

	/**
	 *	@brief set canny threshold
	 */
	void setCannyThreshold(double low_threshold,double high_threshold);

	/**
	 *	@brief set maximum iterator number when training online dictionary
	 */
	void setMaxIteratorsNum(int num);

	/**
	 *	@brief the concrete code is implemented in this function
	 */
	virtual void train();

	/**
	 *	@brief check whether some preliminary conditions are satisfied
	 */
	virtual bool selfcheck();

protected:
	/**
	 *	@brief if the dictionary file has been existed, it wouldn't run
	 */
	virtual bool isNeedProcessed();

	/**
	 *	@brief after processing this node, we should clear some temporary variables
	 */
	virtual void clearSomething();

	/**
	 *	@brief parse all images
	 */
	void parseImages();

private:
	Matrix<float> m_dictionary;
	Matrix<int> m_dictionary_counts;

	int m_jitter_num;								/**< the number of sampling points around edges*/
	bool m_random_jitter_switch;					/**< jitter switch*/
	float m_jitter_degree;							/**< gaussian variance*/
	
	double m_canny_low_threshold;
	double m_canny_high_threshold;

	int m_max_iterators_num;
};
}

#endif