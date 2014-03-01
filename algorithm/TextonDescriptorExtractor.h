#ifndef _TEXTONDESCRIPTOREXTRACTOR_H_
#define _TEXTONDESCRIPTOREXTRACTOR_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "DescriptorExtractor.h"
#include <vector>

namespace eagleeye
{
class EAGLEEYE_API TextonDescriptorExtractor:public DescriptorExtractor
{
public:
	enum _FilterBankType
	{
		LM_FILTER_BANK,
		S_FILTER_BANK,
		MR_FILTER_BANK
	};

	struct _Parameter
	{
		_FilterBankType m_filter_bank_type;
		int m_filter_size;
	};

	TextonDescriptorExtractor();
	virtual ~TextonDescriptorExtractor();

	/**
	 *	@brief set class identity
	 */
	EAGLEEYE_CLASSIDENTITY(TextonDescriptorExtractor);

	/**
	 *	@brief set filter bank type
	 */
	void setFilterBankType(_FilterBankType filter_bank_type);

	/**
	 *	@brief set the filter size
	 */
	void setFilterSize(int size);
	
	/**
	 *	@brief get this descriptor size
	 */
	virtual int descriptorSize();

	/**
	 *	@brief set/get parameter block
	 */
	virtual void setUnitPara(MemoryBlock param_block);
	virtual void getUnitPara(MemoryBlock& param_block);

protected:
	virtual void computeImpl(const Matrix<float>& img,std::vector<KeyPoint>& keypoints,
		Matrix<float>& img_descriptors);
	
	/**
	 *	@brief generate LM filter bank (The Leung-Malik(LM) Filter Bank)
	 *	@note dimension = 48
	 */
	void generateLMFilterBank();
	Matrix<float> makeFilter(const Matrix<float>& pts,float scale, int phase_x, int phase_y, int sup);
	
	/**
	 *	@brief generate S filter bank(The Schmid Filter Bank)
	 *	@note dimension = 13
	 */
	void generateSFilterBank();
	Matrix<float> makeFilter(int sup,float sigma_val,int tau);

	/**
	 *	@brief generate RFS filter bank(The Maximum Response(MR) Filter Banks)
	 *	@note dimension = 38
	 */
	void generateMRFilterBank();

	/**
	 *	@brief normalize filter
	 */
	void normalizeFilter(Matrix<float>& filter);

	/**
	 *	@brief compute gaussian derivatives of order 0 <= ord < 3 evaluated at x.
	 */
	Matrix<float> gauss1d(const Matrix<float>& x, float sigma_val, float mean_val, int ord);

private:
	_FilterBankType m_filter_bank_type;
	int m_filter_size;

	std::vector<Matrix<float>> m_filter_bank;
};
}

#endif