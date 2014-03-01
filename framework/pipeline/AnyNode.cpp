#include "AnyNode.h"

namespace eagleeye
{
AnyNode::AnyNode(const char* unit_name)
:AnyUnit(unit_name)
{
	m_updating_flag = false;
}

AnyNode::~AnyNode()
{
	std::vector<AnySignal*>::iterator iter,iend(m_output_signals.end());
	for (iter = m_output_signals.begin(); iter != iend; ++iter)
	{
		if ((*iter))
		{
			delete (*iter);
		}
	}
}

void AnyNode::addInputPort(AnySignal* sig)
{
	m_input_signals.push_back(sig);
	modified();
}

void AnyNode::setInputPort(AnySignal* sig,int index)
{
	m_input_signals[index] = sig;
	modified();
}

void AnyNode::removeInputPort(AnySignal* sig)
{
	std::vector<AnySignal*>::iterator iter,iend(m_input_signals.end());
	for (iter = m_input_signals.begin();iter != iend; ++iter)
	{
		if ((*iter) == sig)
		{
			(*iter)=NULL;
		}
	}
	modified();
}
void AnyNode::removeInputPort(int index)
{
	if (index < int(m_input_signals.size()))
	{
		m_input_signals[index]=NULL;
	}
	modified();
}

AnySignal* AnyNode::getInputPort(unsigned int index)
{
	if (index < m_input_signals.size())
	{
		return m_input_signals[index];
	}
	else
	{
		return NULL;
	}
}

const AnySignal* AnyNode::getInputPort(unsigned int index) const 
{
	if (index < m_input_signals.size())
	{
		return m_input_signals[index];
	}
	else
	{
		return NULL;
	}
}

AnySignal* AnyNode::getOutputPort(unsigned int index)
{
	if (index < m_output_signals.size())
	{
		return m_output_signals[index];
	}
	else
	{
		return NULL;
	}
}

const AnySignal* AnyNode::getOutputPort(unsigned int index) const
{
	if (index < m_output_signals.size())
	{
		return m_output_signals[index];
	}
	else
	{
		return NULL;
	}
}

void AnyNode::setOutputPort(AnySignal* sig,int index)
{
	//does this change anything?
	if (index < int(m_output_signals.size()) && sig == m_output_signals[index])
	{
		return;
	}

	//expand array if necessary
	if (index >= int(m_output_signals.size()))
	{
		setNumberOfOutputSignals(index + 1);
	}

	if (m_output_signals[index])
	{
		//delete the old output signal
		m_output_signals[index]->dislinkAnyNode(this,index);
		delete m_output_signals[index];

		m_output_signals[index] = NULL;
	}

	if (sig)
	{
		sig->linkAnyNode(this,index);
	}
	
	//save this sig as output signals
	m_output_signals[index] = sig;

	modified();
}

void AnyNode::setNumberOfOutputSignals(unsigned int outputnum)
{
	if (outputnum != m_output_signals.size())
	{
		std::vector<AnySignal*> output_signals;
		output_signals.resize(outputnum);
		
		std::vector<bool> output_port_states;
		output_port_states.resize(outputnum);

		int old_output_signals_num = m_output_signals.size();

		if (old_output_signals_num < int(outputnum))
		{
			for (int i = 0; i < old_output_signals_num; ++i)
			{
				output_signals[i] = m_output_signals[i];
				output_port_states[i] = m_output_port_state[i];
			}

			for (int i = old_output_signals_num; i < int(outputnum); ++i)
			{
				output_signals[i] = NULL;
				output_port_states[i] = true;
			}
		}
		else
		{
			for (int i = 0; i < int(outputnum); ++i)
			{
				output_signals[i] = m_output_signals[i];
				output_port_states[i] = m_output_port_state[i];
			}

			for (int i = outputnum; i < old_output_signals_num; ++i)
			{
				if (m_output_signals[i])
				{
					m_output_signals[i]->dislinkAnyNode(this,i);
					delete m_output_signals[i];
				}
			}
		}

		m_output_signals = output_signals;
		m_output_port_state = output_port_states;
	}
}
void AnyNode::setNumberOfInputSignals(unsigned int inputnum)
{
	if (inputnum != m_input_signals.size())
	{
		std::vector<AnySignal*> input_signals;
		input_signals.resize(inputnum);
		if (inputnum < m_input_signals.size())
		{
			for (int i = 0; i < int(inputnum); ++i)
			{
				input_signals[i] = m_input_signals[i];
			}
		}
		else
		{
			for (int i = 0; i < int(m_input_signals.size()); ++i)
			{
				input_signals[i] = m_input_signals[i];
			}
			for (int i = int(m_input_signals.size()); i < int(inputnum); ++i)
			{
				input_signals[i] = NULL;
			}
		}

		m_input_signals = input_signals;
	}
}

void AnyNode::resetPipeline()
{
	//call modified(), forcelly
	//Next starting off updateUnitInfo, this node would be update 
	modified();
}

AnySignal* AnyNode::makeOutputSignal()
{
	return new AnySignal;
}

void AnyNode::passonNodeInfo()
{
	std::vector<AnySignal*>::iterator in_iter,in_iend(m_input_signals.end());
	std::vector<AnySignal*>::iterator out_iter,out_iend(m_output_signals.end());

	for (in_iter = m_input_signals.begin();in_iter != in_iend; ++in_iter)
	{
		for (out_iter = m_output_signals.begin(); out_iter != out_iend; ++out_iter)
		{
			(*out_iter)->copyInfo(*in_iter);
		}
	}
}

void AnyNode::start()
{
	//update some necessary info, such as basic format or struct of AnySignal(without content),
	//re-assign update time
	std::vector<AnySignal*>::iterator out_iter,out_iend(m_output_signals.end());
	for (out_iter = m_output_signals.begin(); out_iter != out_iend; ++out_iter)
	{
		(*out_iter)->updateUnitInfo();
	}

	for (out_iter = m_output_signals.begin(); out_iter != out_iend; ++out_iter)
	{
		//complement some concrete task, such as generating some data and so on.
		(*out_iter)->processUnitInfo();
	}
}

void AnyNode::print()
{
	//print input signal info
	std::vector<AnySignal*>::iterator in_iter,in_iend(m_input_signals.end());
	for (in_iter = m_input_signals.begin(); in_iter != in_iend; ++in_iter)
	{
		(*in_iter)->printUnit();
	}
		
	//print this node info
	printUnit();
}

void AnyNode::updateUnitInfo()
{
	unsigned long t1,t2;

	//watch out for loops in the pipeline
	//prevent from trapping into dead loop
	if (m_updating_flag)
	{
		modified();
		return;
	}

	//get the parameter update time of of this Node
	//we now wish to set the PipelineMTime of each output signal
	// to the largest of this AnyNode's MTime, all input signal's PipelineMTime
	// , and all input's MTime. We begin with the MTime of this AnyNode.
	t1 = getMTime();

	//Loop through the inputs
	for (unsigned int idx = 0; idx < m_input_signals.size(); ++idx)
	{
		if (m_input_signals[idx])
		{
			AnySignal* input_signal = m_input_signals[idx];

			//propagate update unit info
			//notify the upper unit update
			m_updating_flag = true;
			input_signal->updateUnitInfo();
			m_updating_flag = false;

			//What is the PipelineTime of this input signal? Compare this with
			// our current computation to find the largest one.
			t2=input_signal->getPipelineTime();

			if (t2 > t1)
			{
				t1 = t2;
			}

			//Pipeline Time of this input signal doesn't include the Time of 
			//this input signal itself.
			t2 = input_signal->getMTime();
			if (t2 > t1)
			{
				t1 = t2;
			}
		}
	}

	//judge whether the current node is need to update
	if (t1 > m_node_info_mtime.getMTime())
	{
		//If the current node is needed to update,
		//we need to re-set the Pipeline Time of all output signals 
		//to force them to update
		for (unsigned int idx = 0;idx < m_output_signals.size(); ++idx)
		{
			AnySignal* output_signal = m_output_signals[idx];
			output_signal->setPipelineTime(t1);
		}

		//start off passing on node info
		//now we can use the struct info of all m_input_signals 
		passonNodeInfo();

		//record the time that updateUnitInfo() was called
		m_node_info_mtime.modified();
	}
}

void AnyNode::processUnitInfo()
{	
	//the upper unit should process unit info firstly.
	std::vector<AnySignal*>::iterator in_iter,in_iend(m_input_signals.end());
	for (in_iter = m_input_signals.begin(); in_iter != in_iend; ++in_iter)
	{
		(*in_iter)->processUnitInfo();
	}

	EAGLEEYE_INFO("execute node (%s) -- (%s)\n",getClassIdentity(),getUnitName());

	if (isNeedProcessed())
	{
		//execute task
		//now we can use all info of m_input_signals
		executeNodeInfo();

		//clear some compute resource
		clearSomething();
	}

	//all info of m_output_signals has been generated
	//we should change their time stamp
	std::vector<AnySignal*>::iterator out_iter,out_iend(m_output_signals.end());
	for (out_iter = m_output_signals.begin(); out_iter != out_iend; ++out_iter)
	{
		(*out_iter)->signalHasBeenUpdate();
	}
}

void AnyNode::getPipelineMonitorPool(std::map<const char*,std::vector<AnyMonitor*>>& pipeline_monitor_pool)
{
	if(pipeline_monitor_pool.find(getClassIdentity()) == pipeline_monitor_pool.end())
	{
		pipeline_monitor_pool[getClassIdentity()] = m_unit_monitor_pool;
	}

	//traverse the whole pipeline
	std::vector<AnySignal*>::iterator signal_iter,signal_iend(m_input_signals.end());
	for (signal_iter = m_input_signals.begin();signal_iter != signal_iend; ++signal_iter)
	{
		if ((*signal_iter))
			(*signal_iter)->getPipelineMonitorPool(pipeline_monitor_pool);
		else
			return;
	}
}

void AnyNode::enableOutputPort(int index)
{
	if (index < int(m_output_port_state.size()))
	{
		m_output_port_state[index] = true;
	}
}
void AnyNode::disableOutputPort(int index)
{
	if (index < int(m_output_port_state.size()))
	{
		m_output_port_state[index] = false;
	}
}

bool AnyNode::selfcheck()
{
	//set default unit name
	if (std::string(getUnitName()) == std::string("AnyNode"))
	{
		setUnitName(getClassIdentity());
	}

	//it configure file exists, loading configure parameters
	//it would run only once
	loadUnitConfig();

	return true;
}

void AnyNode::printUnit()
{
	EAGLEEYE_INFO("node id--( %s )  name--( %s ) \n", getClassIdentity(),getUnitName());
}
}