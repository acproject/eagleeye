namespace eagleeye
{
template<class SlotMachine>
int MultiArmedBandit::tryAgain(SlotMachine slot_machine)
{
	switch(m_gamble_strategy)
	{
	case EXP3:
		{
			return runExp3<SlotMachine>(slot_machine);
		}
	case EXP4:
		{
			return runExp4<SlotMachine>(slot_machine);
		}
	}

	return -1;
}

template<class SlotMachine>
int MultiArmedBandit::runExp3(SlotMachine slot_machine)
{
	//step 2 and 3 in paper
	//draw action i
	Variable<int> gamble_action=Variable<int>::discreteDis(m_slot_machines_index,m_gamble_probability);
	int gamble_one=gamble_action.var();

	//gamble reward must be normalized to [0,1]
	float gamble_reward=slot_machine(gamble_one);

	if (gamble_reward>1||gamble_reward<0)
	{
		EAGLEEYE_ERROR("gamble reward must be normalize to [0,1]");
		return -1;
	}

	//step 4 in paper
	for (int machine_index=0;machine_index<m_slot_machines_num;++machine_index)
	{
		if (machine_index==gamble_one)
		{
			m_slot_machines_weight[machine_index]=m_slot_machines_weight[machine_index]*
				exp(m_gammar*gamble_reward/m_gamble_probability[gamble_one]/m_slot_machines_num);
		}
	}

	float weight_total=0;
	for (int machine_index=0;machine_index<m_slot_machines_num;++machine_index)
	{
		weight_total+=m_slot_machines_weight[machine_index];
	}

	//step 1 in paper
	//update gamble model
	float probability_total=0;
	for (int machine_index=0;machine_index<m_slot_machines_num;++machine_index)
	{
		m_slot_machines_weight[machine_index]=m_slot_machines_weight[machine_index]/weight_total;

		m_gamble_probability[machine_index]=
			(1-m_gammar)*m_slot_machines_weight[machine_index]+m_gammar/m_slot_machines_num;
		probability_total+=m_gamble_probability[machine_index];
	}

	//normalize gamble probability
	for (int machine_index=0;machine_index<m_slot_machines_num;++machine_index)
	{
		m_gamble_probability[machine_index]=m_gamble_probability[machine_index]/probability_total;
	}
	return gamble_one;
}

template<class SlotMachine>
int MultiArmedBandit::runExp4(SlotMachine slot_machine)
{
	//step 3 and 4 in paper
	//draw action i
	Variable<int> gamble_action=Variable<int>::discreteDis(m_slot_machines_index,m_gamble_probability);
	int gamble_one=gamble_action.var();

	//gamble reward must be normalize to [0,1]
	float gamble_reward=slot_machine(gamble_one);
	if (gamble_reward>1||gamble_reward<0)
	{
		EAGLEEYE_ERROR("gamble reward must be normalize to [0,1]");
		return -1;
	}

	gamble_reward=gamble_reward/m_gamble_probability(gamble_one);

	//step 5 in paper
	Matrix<float> gamble_reward_vec(m_slot_machines_num,1,0.0f);
	gamble_reward_vec.at(gamble_one,0)=gamble_reward;	

	//step 6 in paper
	for (int expert_index=0;expert_index<m_experts_num;++expert_index)
	{
		Matrix<float> product=m_experts_advices(Range(expert_index,expert_index+1),Range(0,m_slot_machines_num))*gamble_reward_vec;
		float expert_confidence_update=product(0,0);

		for (int machine_index=0;machine_index<m_slot_machines_num;++machine_index)
		{
			m_experts_reliability(expert_index)=m_experts_reliability(expert_index)*exp(m_gammar*expert_confidence_update/m_slot_machines_num);
		}
	}


	//step 2 in paper
	//update gamble model
	float conficence_total=0;
	for(int expert_index=0;expert_index<m_experts_num;++expert_index)
	{
		conficence_total+=m_experts_reliability(expert_index);
	}

	//normalize experts reliability
	for (int expert_index=0;expert_index<m_experts_num;++expert_index)
	{
		m_experts_reliability(expert_index)=m_experts_reliability(expert_index)/conficence_total;
	}
	conficence_total=1.0f;

	float probability_total=0;
	for (int machine_index=0;machine_index<m_slot_machines_num;++machine_index)
	{
		float val=0;
		for (int expert_index=0;expert_index<m_experts_num;++expert_index)
		{
			val+=m_experts_reliability(expert_index)*m_experts_advices(expert_index,machine_index)/conficence_total;
		}

		m_gamble_probability(machine_index)=(1-m_gammar)*val+m_gammar/m_slot_machines_num;

		probability_total+=m_gamble_probability(machine_index);
	}

	//normalize gamble_probability
	for (int machine_index=0;machine_index<m_slot_machines_num;++machine_index)
	{
		m_gamble_probability(machine_index)=m_gamble_probability(machine_index)/probability_total;
	}

	return gamble_one;
}
}