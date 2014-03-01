#include "SymbolTrainer.h"

namespace eagleeye
{
std::string SymbolTrainer::m_ext_name = ".voc";

SymbolTrainer::SymbolTrainer()
{
	m_trainer_name = "Trainer";

	//invalid label
	m_invalid_label = -1;

	//resize image
	m_is_resize_flag = false;
	m_scales[0] = 0.5f;
	m_scales[1] = 0.5f;

	//gaussian smooth
	m_is_gaussian_flag = false;
	m_kernel_size[0] = 3;
	m_kernel_size[1] = 3;
	
	m_fixed_offset[0] = 0;
	m_fixed_offset[1] = 0;

	m_img_bound[0] = 0;
	m_img_bound[1] = 0;

	//whiting feature
	m_whiting_flag = false;

	/**
	 *	@brief set input port number
	 */
	setNumberOfInputSignals(1);
}

SymbolTrainer::~SymbolTrainer()
{

}

void SymbolTrainer::startTrain()
{
	start();
}

void SymbolTrainer::executeNodeInfo()
{
	train();
}

void SymbolTrainer::passonNodeInfo()
{
	if (getInputPort(INPUT_PORT_PARSE_FILE_INFO))
	{
		m_parse_file_name = TO_INFO(std::string,getInputPort(INPUT_PORT_PARSE_FILE_INFO));
		AnyNode::passonNodeInfo();
	}
}

void SymbolTrainer::setParseFile(std::string file_name)
{
	m_parse_file_name = file_name;
}

void SymbolTrainer::setTrainerFolder(std::string folder)
{
	m_trainer_folder = folder;
}

void SymbolTrainer::setTrainerName(std::string name)
{
	m_trainer_name = name;
}

AnySignal* SymbolTrainer::makeOutputSignal()
{
	return new InfoSignal<std::string>;
}

void SymbolTrainer::setInvalidLabel(int invalid_label)
{
	m_invalid_label = invalid_label;
}

void SymbolTrainer::setExtName(const char* ext_name)
{
	m_ext_name = ext_name;
}
const char* SymbolTrainer::getExtName()
{
	return m_ext_name.c_str();
}

void SymbolTrainer::setResizeFlag(bool flag,float x_scale,float y_scale)
{
	m_is_resize_flag = flag;
	m_scales[0] = x_scale;
	m_scales[1] = y_scale;
}

void SymbolTrainer::setGaussianProcessFlag(bool flag,int kernel_x,int kernel_y)
{
	m_is_gaussian_flag = flag;
	m_kernel_size[0] = kernel_x;
	m_kernel_size[1] = kernel_y;
}

void SymbolTrainer::loadObjectDetTrainingSampls(
	std::map<std::string,std::vector<Array<int,4>>>& p_samples, 
	std::map<std::string,std::vector<Array<int,4>>>& n_samples)
{
	std::string parse_file_path = m_trainer_folder + m_parse_file_name;
	analyzeSimpleParseFile(parse_file_path.c_str(),p_samples,n_samples);
}

void SymbolTrainer::loadObjectDetTrainingSampls(std::map<std::string,std::vector<Array<int,4>>>& samples)
{
	std::string parse_file_path = m_trainer_folder + m_parse_file_name;
	analyzeSimpleParseFile(parse_file_path.c_str(),samples);
}

bool SymbolTrainer::loadObjectClassifyTrainingSamples(std::map<std::string,std::string>& samples_and_annotation,
									   std::map<std::string,Array<int,4>>& samples_regions,
									   std::map<std::string,int>& samples_labels,
									   int& labels_num)
{
	std::string parse_file_path = m_trainer_folder + m_parse_file_name;

	std::vector<TrainingSampleInfo> s_info_set;
	analyzeParseFile(parse_file_path.c_str(),s_info_set);

	std::vector<TrainingSampleInfo>::iterator iter,iend(s_info_set.end());
	for (iter = s_info_set.begin(); iter != iend; ++iter)
	{
		TrainingSampleInfo s_info = (*iter);
		if (s_info.t_address == TrainingSampleInfo::unkown_flag ||
			s_info.t_a_address == TrainingSampleInfo::unkown_flag)
		{
			return false;
		}

		samples_and_annotation[s_info.t_address] = s_info.t_a_address;
		samples_regions[s_info.t_address] = s_info.object_regions[0];
		samples_labels[s_info.t_address] = s_info.object_labels[0];
		labels_num = s_info.total_labels;
	}

	return true;
}

void SymbolTrainer::setWhitingFlag(bool flag)
{
	m_whiting_flag = flag;
}

void SymbolTrainer::setFixedOffset(int x,int y)
{
	m_fixed_offset[0] = x;
	m_fixed_offset[1] = y;
}

void SymbolTrainer::setImageBounds(int bound_width,int bound_height)
{
	m_img_bound[0] = bound_width;
	m_img_bound[1] = bound_height;
}

}