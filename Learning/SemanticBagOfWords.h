#ifndef _SEMANTICBAGOFWORDS_H_
#define _SEMANTICBAGOFWORDS_H_

#include "EagleeyeMacro.h"
#include "Learning/MarginalCRF.h"

namespace eagleeye
{
class EAGLEEYE_API SemanticBagOfWords:public MarginalCRF
{
public:
	SemanticBagOfWords(int words_num,int states_num);
	virtual ~SemanticBagOfWords();

	void semanticLearn(const Matrix<int>& words_state_samples,
		const Matrix<float>& words_fre_samples,
		const Matrix<float>& words_dis_samples,
		const Matrix<float>& words_angle_samples);

	Matrix<float> semanticInfer(const Matrix<int>& words_state_samples,
		const Matrix<float>& words_fre_samples,
		const Matrix<float>& words_dis_samples,
		const Matrix<float>& words_angle_samples);

protected:
	void constructSemanticGraph();

private:
	SemanticBagOfWords(const SemanticBagOfWords&);
	void operator=(const SemanticBagOfWords&);

	int m_words_num;
	int m_pairwise_num;/**< m_words_num * (m_words_num - 1)*/
	int m_words_states;/**< 0,1,2*/
};
}

#endif