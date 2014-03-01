#include "Learning/MultiArmedBandit.h"
namespace eagleeye
{
MultiArmedBandit::MultiArmedBandit()
{
	m_gammar=0.5f;
	m_slot_machines_num=0;
	m_experts_num=0;
	m_gamble_strategy=EXP3;
}

MultiArmedBandit::~MultiArmedBandit()
{

}

void MultiArmedBandit::setGambleStrategy(GambleStrategyType strategy)
{
	m_gamble_strategy=strategy;
}

void MultiArmedBandit::saveMABModel(const char* model_file)
{
	EagleeyeIO io;
	io.createWriteHandle(std::string(model_file),false,WRITE_BINARY_MODE);
	
	io.write(m_gamble_probability);
	io.write(m_slot_machines_index);

	io.write(m_gammar);
	if (m_gamble_strategy==EXP3)
	{
		io.write(int(0));
		io.write(m_slot_machines_weight);
	}
	else
	{
		io.write(int(1));
		io.write(m_experts_reliability);
	}
	io.destroyHandle();
}

void MultiArmedBandit::loadMABModel(const char* model_file)
{
	EagleeyeIO io;
	io.createReadHandle(std::string(model_file),READ_BINARY_MODE);
	
	io.read(m_gamble_probability);
	io.read(m_slot_machines_index);

	io.read(m_gammar);
	int type_index;
	io.read(type_index);
	if (type_index==0)
	{
		m_gamble_strategy=EXP3;
		io.read(m_slot_machines_weight);
	}
	else
	{
		m_gamble_strategy=EXP4;
		io.read(m_experts_reliability);
	}

	m_slot_machines_num=m_slot_machines_index.size();

	io.destroyHandle();
}

void MultiArmedBandit::setSlotMachinesIndex(DynamicArray<int> slot_machines_index)
{
	m_slot_machines_index=slot_machines_index;
	m_slot_machines_num=m_slot_machines_index.size();
	m_gamble_probability=DynamicArray<float>(m_slot_machines_num,1.0f/float(m_slot_machines_num));
}

void MultiArmedBandit::setSlotMachinesIndex(int num)
{
	m_slot_machines_index = DynamicArray<int>(num);
	for (int i = 0; i < num; ++i)
	{
		m_slot_machines_index[i] = i;
	}
	m_slot_machines_num=m_slot_machines_index.size();
	m_gamble_probability=DynamicArray<float>(m_slot_machines_num,1.0f/float(m_slot_machines_num));
}

void MultiArmedBandit::setExpertsAdvices(Matrix<float> advices)
{
	m_gamble_strategy=EXP4;
	m_experts_advices=advices;
	m_experts_num=m_experts_advices.rows();
}

int MultiArmedBandit::tryAgain()
{
	//draw action i
	Variable<int> gamble_action=Variable<int>::discreteDis(m_slot_machines_index,m_gamble_probability);
	int gamble_one=gamble_action.var();
	return gamble_one;
}

void MultiArmedBandit::setGammar(float gammar)
{
	m_gammar=gammar;
}

void MultiArmedBandit::initialize()
{
	switch(m_gamble_strategy)
	{
	case EXP3:
		{
			m_slot_machines_weight=DynamicArray<float>(m_slot_machines_num,1.0f/m_slot_machines_num);
			break;
		}
	case EXP4:
		{
			m_experts_reliability=DynamicArray<float>(m_experts_num,1.0f/m_experts_num);
			break;
		}
	default:
		{
			EAGLEEYE_ERROR("don't support this kind of gamble strategy");
		}
	}
}
}