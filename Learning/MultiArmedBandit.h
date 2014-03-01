#ifndef _MULTIARMEDBANDIT_H_
#define _MULTIARMEDBANDIT_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "DynamicArray.h"
#include "Variable.h"
#include "EagleeyeIO.h"
#include "Print.h"

namespace eagleeye
{
/**
 *	@brief slot machine gamble.
 *	@detail This is a selecting strategy. Sometimes, facing amounts of solutions,
 *	we only need to pick one to solve problem. How to select is key. This selecting model
 *	comes from gamble. In the multi-armed bandit problem, a gambler must decide which arm
 *	of K non-identical slot machines to play in a sequence of trails so as to maximize
 *	his reward.
 *	@note reference: Peter Auer The non-stochastic multi-armed bandit problem
 *	@par example:
 *	@code
 *	(1) EXP3 strategy
 *		MultiArmedBandit gamble_strategy;
 *		gamble_strategy.setGambleStrategy(EXP3);
 *		DynamicArray<int> slot_machine_index(2);
 *		slot_machine_index[0]=0;slot_machine_index[1]=1;
 *		gamble_strategy.setSlotMachinesIndex(slot_machine_index);
 *		gamble_strategy.setGammar(0.2);
 *		gamble_strategy.initialize();
 *		//for training
 *		int gamble_one=gamble_strategy.tryAgain(machine);
 *		//for test
 *		int gamble_one=gamble_strategy.trayAgain();
 *	(2) EXP4 strategy 
 *		MultiArmedBandit gamble_strategy;
 *		gamble_strategy.setGambleStrategy(EXP4);
 *		DynamicArray<int> slot_machine_index(2);
 *		slot_machine_index[0]=0;slot_machine_index[1]=1;
 *		gamble_strategy.setSlotMachinesIndex(slot_machine_index);
 *		gamble_strategy.setGammar(0.2);
 *		gamble_strategy.initialize();
 *		gamble_strategy.setExpertsAdvices(Matrix<float> ...);
 *		//for training
 *		gamble_strategy.tryAgain(machine);
 *		//for test
 *		gamble_strategy.tryAgain();
 *	@endcode
 */
class EAGLEEYE_API MultiArmedBandit
{
public:
	enum GambleStrategyType
	{
		EXP3,
		EXP4
	};

	MultiArmedBandit();
	virtual ~MultiArmedBandit();

	/**
	 *	@brief set solution type
	 *	@note if all data come from model file, we shouldn't call this
	 *	function.
	 */
	void setGambleStrategy(GambleStrategyType mab_type);

	/**
	 *	@brief set slot machines
	 *	@note if all data come from model file, we shouldn't call this
	 *	function.
	 */
	void setSlotMachinesIndex(DynamicArray<int> slot_machines_index);
	
	/**
	 *	@brief set slot machines
	 */
	void setSlotMachinesIndex(int num);

	/**
	 *	@brief set experts advices(matrix  experts*machines) for exp4
	 */
	void setExpertsAdvices(Matrix<float> advices);

	/**
	 *	@brief set parameter gammar manually
	 *	@note If all data come from model file, we shouldn't call this function.
	 */
	void setGammar(float gammar);

	/**
	 *	@brief try gamble. These two functions are used in training and test respectively.
	 */
	template<class SlotMachine>
	int tryAgain(SlotMachine slot_machine);
	int tryAgain();

	/**
	 *	@brief initialize
	 *	@note we should guarantee this function is only called once. If
	 *	all data come from model file, we shouldn't call this function.
	 */
	void initialize();

	/**
	 *	@brief Read/Save MAB model
	 */
	void saveMABModel(const char* model_file);
	void loadMABModel(const char* model_file);

protected:
	/**
	 *	@brief using exp3 gamble strategy
	 *	@note Peter Auer The non-stochastic multi-armed bandit problem
	 */
	template<class SlotMachine>
	int runExp3(SlotMachine slot_machine);

	/**
	 *	@brief using exp4 gamble strategy
	 *	@note Peter Auter The non-stochastic multi-armed bandit problem
	 */
	template<class SlotMachine>
	int runExp4(SlotMachine slot_machine);

private:
	GambleStrategyType m_gamble_strategy;		/**< gamble strategy(algorithm type)*/

	float m_gammar;								/**< the only one parameter in this model*/
	
	DynamicArray<float> m_gamble_probability;	/**< use this probability to gamble*/
	int m_slot_machines_num;					/**< the number of machines*/
	DynamicArray<int> m_slot_machines_index;	/**< the index of machines*/

	DynamicArray<float> m_slot_machines_weight;	/**< used in exp3 strategy*/

	int m_experts_num;							/**< used in exp4 strategy*/
	Matrix<float> m_experts_advices;			/**< used in exp4 strategy*/
	DynamicArray<float> m_experts_reliability;	/**< used in exp4 strategy*/
};
}

#include "MultiArmedBandit.hpp"
#endif