#ifndef _GRAYDESCRIPTOREXTRACTOR_H_
#define _GRAYDESCRIPTOREXTRACTOR_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "DescriptorExtractor.h"
#include <vector>
#include "MemoryBlock.h"
namespace eagleeye
{
/**
 *	@brief using gray in predefined region as feature description
 *	@note don't support multi-scale
 */
class EAGLEEYE_API GrayDescriptorExtractor:public DescriptorExtractor
{
	enum ExtentMode
	{
		LEFT_UP_EXT,
		CENTER_EXT,
		RIGHT_BOTTOM_EXT
	};
	struct _Parameter
	{
		int width;
		int height;
		int ext_mode;
	};


	GrayDescriptorExtractor();
	virtual ~GrayDescriptorExtractor();

	/**
	 *	@brief set class identity
	 */
	EAGLEEYE_CLASSIDENTITY(GrayDescriptorExtractor);

	/**
	 *	@brief set region size
	 */
	void setRegionSize(int width,int height);

	/**
	 *	@brief set extent mode
	 */
	void setExtentMode(ExtentMode ext_mode);

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
	virtual void computeImpl(const Matrix<float>& img,std::vector<KeyPoint>& keypoints, Matrix<float>& img_descriptors);

private:
	int m_width;
	int m_height;
	
	ExtentMode m_ext_mode;
};
}
#endif