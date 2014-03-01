#include "UnitManager.h"
#include "SiftDescriptorExtractorOpenCV.h"
#include "TextonDescriptorExtractor.h"
#include "ProcessNode/SRMNode.h"
#include "ObjectDetTerminalSymbol.h"
#include "ObjectDetNonterminalSymbol.h"
#include "ObjectDetStructureRule.h"
#include "ObjectDetDeformationRule.h"
#include "ObjectDetSuperpixelSymbol.h"
#include "TopHitSymbol.h"
#include "DummyLinkRule.h"

namespace eagleeye
{
UnitManager::UnitManager()
{

}
UnitManager::~UnitManager()
{

}
AnyUnit* UnitManager::factory(const char* unit_id,const char* ext_info)
{
	std::string unit_id_str = unit_id;
	std::string ext_info_str = ext_info;

	if (unit_id_str == "SiftDescriptorExtractorOpenCV")
	{
		return new SiftDescriptorExtractorOpenCV;
	}
	if (unit_id_str == "TextonDescriptorExtractor")
	{
		return new TextonDescriptorExtractor;
	}
	if (unit_id_str == "SRMNode" && ext_info_str == "unsigned char")
	{
		return new SRMNode<ImageSignal<unsigned char>>;
	}

	if (unit_id_str == "ObjectDetTerminalSymbol")
	{
		return new ObjectDetTerminalSymbol(ext_info);
	}

	if (unit_id_str == "ObjectDetNonterminalSymbol")
	{
		return new ObjectDetNonterminalSymbol(ext_info);
	}

	if (unit_id_str == "ObjectDetStructureRule")
	{
		return new ObjectDetStructureRule(ext_info);
	}

	if (unit_id_str == "ObjectDetDeformationRule")
	{
		return new ObjectDetDeformationRule(ext_info);
	}

	if (unit_id_str == "ObjectDetSuperpixelSymbol")
	{
		return new ObjectDetSuperpixelSymbol(ext_info);
	}

	if (unit_id_str == "TopHitSymbol")
	{
		return new TopHitSymbol(ext_info);
	}
	
	if (unit_id_str == "DummyLinkRule")
	{
		return new DummyLinkRule(ext_info);
	}

	return NULL;
}
}