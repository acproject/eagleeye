#ifndef _ANYSIGNAL_H_
#define _ANYSIGNAL_H_
#include "EagleeyeMacro.h"

#include "AnyUnit.h"
#include <stdio.h>
#include <string>
#include <map>
#include <vector>

namespace eagleeye
{
class AnyNode;
class EAGLEEYE_API AnySignal:public AnyUnit
{
public:
	/**
	 *	@brief define some basic type
	 *	@note you must do these
	 */
	typedef AnySignal							Self;
	typedef AnyUnit								Superclass;

	AnySignal(const char* unit_name = "AnySignal");
	virtual ~AnySignal();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(AnySignal);

	/**
	 *	@brief Set/Get pipeline time controlled by AnyNode
	 */
	unsigned long getPipelineTime();
	void setPipelineTime(unsigned long time);

	/**
	 *	@brief Link/Dislink AnyNonde 
	 */
	void linkAnyNode(AnyNode* node,int index);
	void dislinkAnyNode(AnyNode* node,int index);

	/**
	 *	@brief Copy info from signal
	 *	@note If you want to copy some specific info, you have to
	 *	overload this function. This function would be called by "passonNodeInfo"
	 *	of AnyNode, implicitly.
	 */
	virtual void copyInfo(AnySignal* sig){};
	
	/**
	 *	@brief Deliver the update flow
	 */
	virtual void updateUnitInfo();

	/**
	 *	@brief According to update time, notify the linked node
	 *	to process unit info dynamically.
	 */
	virtual void processUnitInfo();

	/**
	 *	@brief print this signal unit info
	 *	@note this function would be called by AnyNode object
	 */
	virtual void printUnit();

	/**
	 *	@brief one easy explained function, which notify itself
	 *	update.
	 */
	void signalHasBeenUpdate();

	/**
	 *	@brief get monitor pool of the whole pipeline
	 *	@note traverse the whole pipeline
	 */
	void getPipelineMonitorPool(std::map<const char*,std::vector<AnyMonitor*>>& pipeline_monitor_pool);

	/**
	 *	@brief clear signal content
	 */
	virtual void makeempty(){};

private:
	AnySignal(const AnySignal&);
	void operator=(const AnySignal&);

	unsigned long m_pipeline_time;
	AnyNode* m_link_node;
	int m_link_index;
};
}

#endif