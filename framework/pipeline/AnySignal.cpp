#include "AnySignal.h"
#include "AnyNode.h"

namespace eagleeye
{
AnySignal::AnySignal(const char* unit_name)
:AnyUnit(unit_name)
{
	m_pipeline_time=0;
	m_link_index=-1;
	m_link_node=NULL;
}
AnySignal::~AnySignal()
{

}

unsigned long AnySignal::getPipelineTime()
{
	return m_pipeline_time;
}

void AnySignal::setPipelineTime(unsigned long time)
{
	m_pipeline_time=time;
}

void AnySignal::linkAnyNode(AnyNode* node,int index)
{
	m_link_node=node;
	m_link_index=index;
}
void AnySignal::dislinkAnyNode(AnyNode* node,int index)
{
	if (m_link_node==node&&m_link_index==index)
	{
		m_link_node=NULL;
		m_link_index=-1;
	}
}

void AnySignal::updateUnitInfo()
{
	//if selfcheck return false, it illustrates that
	//there is no necessary to update this node.
	if (m_link_node)
	{
		if (!m_link_node->selfcheck())
		{
			EAGLEEYE_ERROR("%s couldn't accomplish self check...\n",m_link_node->getClassIdentity());
			system("pause");
		}

		m_link_node->updateUnitInfo();
	}
}

void AnySignal::processUnitInfo()
{
	if (getMTime()<m_pipeline_time && m_link_node)
	{
		m_link_node->processUnitInfo();
	}
}

void AnySignal::signalHasBeenUpdate()
{
	modified();
}

void AnySignal::getPipelineMonitorPool(std::map<const char*,std::vector<AnyMonitor*>>& pipeline_monitor_pool)
{
	if (m_link_node)
	{
		m_link_node->getPipelineMonitorPool(pipeline_monitor_pool);
	}
}

void AnySignal::printUnit()
{
	if (m_link_node)
	{
		m_link_node->print();
		EAGLEEYE_INFO("signal (%s) -- (%s) \n",getClassIdentity(),getUnitName());
		EAGLEEYE_INFO("link node (%s) -- (%s) \n",m_link_node->getClassIdentity(),m_link_node->getUnitName());
	}
}
}
