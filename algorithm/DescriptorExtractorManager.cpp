#include "DescriptorExtractorManager.h"

namespace eagleeye
{
DescriptorExtractorManager::DescriptorExtractorManager()
{

}
DescriptorExtractorManager::~DescriptorExtractorManager()
{

}
DescriptorExtractor* DescriptorExtractorManager::factory(const char* descriptor_name)
{
	return NULL;
}
}