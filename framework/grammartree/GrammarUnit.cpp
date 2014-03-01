#include "GrammarUnit.h"
#include "EagleeyeIO.h"

namespace eagleeye
{
std::string GrammarUnit::m_model_folder = "";
SampleState GrammarUnit::m_sample_state = EAGLEEYE_UNDEFINED_SAMPLE;

GrammarUnit::GrammarUnit(const char* name)
{
	m_unit_name = name;
	m_sample_state = EAGLEEYE_UNDEFINED_SAMPLE;

	m_unit_learn_mult = 0.2f;
	m_unit_reg_mult = 1.0f;

	m_unit_state = GRAMMAR_UNIT_ACTIVE;
	m_data_update_flag = true;

	m_class_num = 2;
}

GrammarUnit::~GrammarUnit()
{

}

void GrammarUnit::setUnitLearnMult(float learn_speed)
{
	m_unit_learn_mult = learn_speed;
}

float GrammarUnit::getUnitLearnMult()
{
	return m_unit_learn_mult;
}

void GrammarUnit::setUnitRegMult(float re)
{
	m_unit_reg_mult = re;
}

float GrammarUnit::getUnitRegMult()
{
	return m_unit_reg_mult;
}

void GrammarUnit::setUnitState(GrammarUnitState state)
{
	m_unit_state = state;
}

GrammarUnitState GrammarUnit::getUnitState()
{
	return m_unit_state;
}
void GrammarUnit::setModelFolder(const char* model_folder)
{
	m_model_folder = model_folder;
}
std::string GrammarUnit::getModelFolder()
{
	return m_model_folder;
}

}
