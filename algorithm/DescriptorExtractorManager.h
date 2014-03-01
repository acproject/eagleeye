#ifndef _DESCRIPTOREXTRACTOR_H_
#define _DESCRIPTOREXTRACTOR_H_

#include "EagleEyeMacro.h"
#include "DescriptorExtractor.h"

namespace eagleeye
{
class DescriptorExtractorManager
{
public:
	DescriptorExtractorManager();
	~DescriptorExtractorManager();

	static DescriptorExtractor* factory(const char* descriptor_name);
};
}

#endif