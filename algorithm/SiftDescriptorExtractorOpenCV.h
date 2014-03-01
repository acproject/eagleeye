#ifndef _FEATUREDESCRIPTORSIFTOPENCV_H_
#define _FEATUREDESCRIPTORSIFTOPENCV_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "DescriptorExtractor.h"

namespace eagleeye
{
class EAGLEEYE_API SiftDescriptorExtractorOpenCV:public DescriptorExtractor
{
public:
	struct _Parameter
	{
		float edge_threshold;
		float contrast_threshold;
		float sigma;
		int n_octavelayers;
	};

	SiftDescriptorExtractorOpenCV();
	virtual ~SiftDescriptorExtractorOpenCV();

	/**
	 *	@brief set class identity
	 */
	EAGLEEYE_CLASSIDENTITY(SiftDescriptorExtractorOpenCV);

	/**
	 *	@brief get descriptor size
	 */
	virtual int descriptorSize();

	/**
	 *	@brief set some parameters
	 */
	void setExtractorParams(int octave_layers,float contrast_threshold,float edge_threshold,float sigma);

	/**
	 *	@brief set/get parameter block
	 */
	virtual void setUnitPara(MemoryBlock param_block);
	virtual void getUnitPara(MemoryBlock& param_block);

protected:
	/**
	 *	@brief compute every keypoint description on the image
	 */
	virtual void computeImpl(const Matrix<float>& img,std::vector<KeyPoint>& keypoints,
		Matrix<float>& descriptors);

private:
	float m_edge_threshold;
	float m_contrast_threshold;
	float m_sigma;
	
	int m_n_octavelayers;

	int m_des_size;
};
}
#endif