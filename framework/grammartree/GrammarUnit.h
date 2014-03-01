#ifndef _UINTBASE_H_
#define _UINTBASE_H_

#include "EagleeyeMacro.h"

#include "Matrix.h"
#include "EagleeyeIO.h"
#include "MemoryBlock.h"
#include <string>
#include <map>
#include <vector>
#include "AnyUnit.h"

namespace eagleeye
{
struct UnitStructureInfo 
{
	std::string unit_name;			//unit name
	int unit_size;					//coefficient array length used in unit
	float unit_regmult;				//unit regmult for learning
	float unit_learnmult;			//unit learnmult for learning
};

struct UnitFeatureInfo
{
	std::string unit_name;			//unit name
	Matrix<float> unit_feature;		//unit feature
};

struct UnitWeightInfo
{
	std::string unit_name;			//unit name
	Matrix<float> unit_weight;		//unit weight
};

enum SearchSpace
{
	PIXEL_SPACE,
	SUPERPIXEL_SPACE,
	RECT_WINDOW_SPACE
};

enum SearchMode
{
	INDEPENDENT_SEARCH,
	CONDITIONAL_SEARCH,
	OPTIMUM_SEARCH
};

enum GrammarUnitState
{
	GRAMMAR_UNIT_ACTIVE,
	GRAMMAR_UNIT_PASSIVE
};

struct GrammarTreeStructureInfo
{
	std::vector<int> units_size;
	std::vector<int> subtrees_size;
	std::vector<int> subtrees_offset;
	std::vector<float> units_regmult;
	std::vector<float> units_learnmult;
	std::vector<int> units_offset;
	std::vector<std::string> units_name;
};

class EAGLEEYE_API GrammarUnit:public AnyUnit
{
public:
	/**
	 *	@brief define some basic type
	 */
	typedef GrammarUnit							Self;
	typedef AnyUnit								Superclass;

	GrammarUnit(const char* name="");
	virtual ~GrammarUnit();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(GrammarUnit);

	/**
	 *	@brief set model folder
	 */
	static void setModelFolder(const char* model_folder);
	static std::string getModelFolder();

	/**
	 *	@brief get/set learn rate
	 *	@note used in "Learning Framework"
	 */
	void setUnitLearnMult(float learn_mult);
	float getUnitLearnMult();

	/**
	 *	@brief get/set regularization coefficient
	 *	@note used in "Learning Framework"
	 */
	void setUnitRegMult(float reg_mult);
	float getUnitRegMult();

	/**
	 *	@brief set/get symbol weight vector
	 */
	virtual Matrix<float> getUnitWeight(){return Matrix<float>();};
	virtual void setUnitWeight(const Matrix<float>& weight){};

	/**
	 * @brief get feature vector
	 */
	virtual Matrix<float> getUnitFeature(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat = Matrix<float>()){return Matrix<float>();};

	/**
	 *	@brief (core)Traverse the model and initialize every unit
	 *	@note first step
	 */
	virtual void initialize() = 0;

	/**
	 *	@brief (core)Traverse the whole model and call setUnitData implicitly
	 *	@note second step
	 */
	virtual void setModelData(void* data,SampleState sample_state,void* auxiliary_data = NULL) = 0;

	/**
	 *	@brief (core)Traverse the whole model and find the latent info by using prior-info.
	 *	@note fourth step. Sometimes, some key info in this unit is latent and
	 *	couldn't be observed directly. In this case, we have to find them
	 *	by some prior-info or some standards.
	 */
	virtual void findModelLatentInfo(void* info){};

	/**
	 *	@brief (core)Traverse the whole model and collect all outputs
	 *	@note fifth step.
	 */
	virtual void getModelOutput(std::vector<void*>& output_info) = 0;

	/**
	 *	@brief (core)learn all unknown info of this model
	 */
	virtual void learn(const char* parse_file) = 0;

	/**
	 *	@brief Traverse the whole model and call saveUnitStructure respectively.
	 */
	virtual void saveModelStructure(EagleeyeIO& io) = 0;

	/**
	 *	@brief Traverse the whole model, every unit would select 
	 *	its own unit weight from weight_map indexed by unit name
	 */
	virtual void assignModelWeight(const std::map<std::string, Matrix<float>>& weight_map) = 0;

	/**
	 *	@brief Traverse the whole model, every unit would select
	 *	its unit parameter from para_map indexed by unit name
	 */
	virtual void assignModelPara(const std::map<std::string,MemoryBlock>& para_map) = 0;

	/**
	 *	@brief Traverse the whole model to collect Unit Configure Info
	 */
	virtual void collectModelStructureInfo(std::vector<UnitStructureInfo>& info) = 0;

	/**
	 *	@brief Traverse the whole model to collect unit feature
	 *	@note These info would be sent to "Learning Framework"
	 */
	virtual void collectModelFeatureInfo(std::vector<UnitFeatureInfo>& info,SearchSpace search_space) = 0;

	/**
	 *	@brief Traverse the whole model to collect unit Weight
	 *	@note these info would be sent to "Learning Framework"
	 */
	virtual void collectModelWeightInfo(std::vector<UnitWeightInfo>& weight_info) = 0;

	/**
	 *	@brief Traverse the whole model, every unit would destroy its resource
	 */
	virtual void destroytModelRes() = 0;

	/**
	 *	@brief Traverse the whole model and clear the link between units
	 */
	virtual void disassembleModel() = 0;

	/**
	 *	@brief Traverse the whole model to set every unit state
	 */
	virtual void setModelState(GrammarUnitState state) = 0;

	/**
	 *	@brief if unit_name is me, it would return myself
	 */
	GrammarUnit* isme(std::string unit_name){return NULL;}

	/**
	 *	@brief set the unit data
	 *	@note the subclass should implement this class
	 */
	virtual void setUnitData(void* data,SampleState sample_state,void* auxiliary_data=NULL){};

	/**
	 *	@brief set/get class number
	 */
	void setClassNum(int class_num){m_class_num = class_num;};
	int getClassNum(){return m_class_num;};

protected:
	/**
	 *	@brief Check whether all preliminary conditions of this unit have been
	 *	satisfied.
	 */
	virtual bool selfcheck(){return true;}

	/**
	 *	@brief Initialize this unit.
	 *	@note This function would be called implicitly.\n
	 *	The subclass should implement this function
	 */
	virtual void initializeUnit(){};

	/**
	 *	@brief Save coefficients of this unit
	 *	@note These two functions would be called implicitly.\n
	 *	The subclass should implement this function
	 */
	virtual void saveUnitStructure(EagleeyeIO& io){};

	/**
	 *	@brief learning unknown info of this unit
	 */
	virtual void learnUnit(const char* samples_file){};
	virtual void learnUnit(const Matrix<float>& samples){};

	/**
	 *	@brief set this unit state. GRAMMAR_UNIT_PASSIVE or GRAMMAR_UNIT_ACTIVE.
	 *	@note Perhaps, in some cases, we only want to use only a few units.
	 */
	void setUnitState(GrammarUnitState state);
	GrammarUnitState getUnitState();

	static SampleState m_sample_state;		/**< the sample type, EAGLEEYE_POSITIVE, EAGLEEYE_NEGATIVE and Undefined*/

	float m_unit_learn_mult;				/**< Learning rate used in "Learning Framework"*/
	float m_unit_reg_mult;					/**< Regularization used in "Learning Framework"*/

	bool m_data_update_flag;				/**< data update flag*/

	char* m_class_identity;
	static std::string m_model_folder;
	int m_class_num;

private:
	GrammarUnitState m_unit_state;			/**< the unit state*/
};

inline GrammarTreeStructureInfo wraptoDummyGrammarTree(GrammarUnit* unit)
{
	GrammarTreeStructureInfo gt_info;
	gt_info.units_size.push_back(unit->getUnitWeight().cols());
	gt_info.subtrees_size.push_back(1);
	gt_info.subtrees_offset.push_back(0);
	gt_info.units_regmult.push_back(unit->getUnitRegMult());
	gt_info.units_learnmult.push_back(unit->getUnitLearnMult());
	gt_info.units_offset.push_back(0);
	gt_info.units_name.push_back(unit->getUnitName());

	return gt_info;
}
}

#endif