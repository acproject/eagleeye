#ifndef _ANYNODE_H_
#define _ANYNODE_H_

#include "EagleeyeMacro.h"

#include "AnyUnit.h"
#include "AnySignal.h"
#include "EagleeyeTimeStamp.h"
#include "Print.h"
#include <map>
#include <vector>

namespace eagleeye
{
class EAGLEEYE_API AnyNode:public AnyUnit
{
public:
	/**
	 *	@brief define some basic type
	 *	@note you must do these
	 */
	typedef AnyNode								Self;
	typedef AnyUnit								Superclass;

	AnyNode(const char* unit_name = "AnyNode");
	virtual ~AnyNode();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(AnyNode);

	/**
	 *	@brief start pipeline mechanism
	 */
	void start();

	/**
	 *	@brief print pipeline info
	 */
	void print();

	/**
	 *	@brief some functions about input signals
	 */
	virtual void addInputPort(AnySignal* sig);
	virtual void setInputPort(AnySignal* sig,int index=0);
	virtual void removeInputPort(AnySignal* sig);
	virtual void removeInputPort(int index);

	AnySignal* getInputPort(unsigned int index=0);
	const AnySignal* getInputPort(unsigned int index=0) const;

	/**
	 *	@brief get output signal
	 */
	AnySignal* getOutputPort(unsigned int index=0);
	const AnySignal* getOutputPort(unsigned int index=0) const;
	
	void setOutputPort(AnySignal* sig,int index=0);
	
	/**
	 *	@brief set/get the number of output signals and input signals
	 */
	void setNumberOfOutputSignals(unsigned int outputnum);
	int getNumberOfOutputSignals(){return int(m_output_signals.size());};
	void setNumberOfInputSignals(unsigned int inputnum);
	int getNumberOfInputSignals(){return int(m_input_signals.size());};
	
	/**
	 *	@brief enable/disable output port
	 */
	void enableOutputPort(int index);
	void disableOutputPort(int index);

	/**
	 *	@brief Help passing on some info hold by signal, such as
	 *	struct info, size or other things.
	 *	@note If you want to achieve some complex functions.
	 *	The subclass should implement this function.
	 */
	virtual void passonNodeInfo();

	/**
	 *	@brief execute some concrete task
	 *	@note if necessary, the subclass should overload this
	 *	function.
	 */
	virtual void executeNodeInfo(){};

	/**
	 *	@brief reset the pipeline
	 *	@note Force the whole pipeline to update
	 */
	virtual void resetPipeline();

	/**
	 *	@brief update unit info, which lead to update pipeline
	 */
	virtual void updateUnitInfo();

	/**
	 *	@brief complete some concrete task, such as generating 
	 *	some data
	 */
	virtual void processUnitInfo();

	/**
	 *	@brief make some self check
	 *	@note check whether some preliminary parameters have been set. This 
	 *	function would be called before "updateUnitInfo()" automatically.
	 *	Please notice that if "selfcheck" is failed, the pipeline would be
	 *	stop.
	 */
	virtual bool selfcheck();

	/**
	 *	@brief According to some outside info, get the flag whether it \n
	 *	needs to be processed.
	 *	@note Sometimes, some outside info would influence the node's behavior.
	 */
	virtual bool isNeedProcessed(){return true;};

	/**
	 *	@brief get monitor pool of the whole pipeline
	 *	@note traverse the whole pipeline
	 */
	void getPipelineMonitorPool(std::map<const char*,std::vector<AnyMonitor*>>& pipeline_monitor_pool);

	/**
	 *	@brief print this node info
	 */
	virtual void printUnit();

protected:
	/**
	 *	@brief make one output signal
	 *	@note If you want a special signal, you have to
	 *	overload this function in the subclass
	 */
	virtual AnySignal* makeOutputSignal();

	/**
	 *	@brief After processing unit info, perhaps we would clear some
	 *	compute resource
	 */
	virtual void clearSomething(){};

	std::vector<AnySignal*> m_input_signals;
	std::vector<AnySignal*> m_output_signals;
	std::vector<bool> m_output_port_state;

	EagleeyeTimeStamp m_node_info_mtime;
	bool m_updating_flag;

private:
	AnyNode(const AnyNode&);
	void operator=(const AnyNode&);
};
}


#endif