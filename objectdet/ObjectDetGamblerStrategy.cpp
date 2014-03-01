#include "ObjectDetGamblerStrategy.h"
#include "ProcessNode/ImageReadNode.h"
#include "LinkRule.h"
#include "EagleeyeIO.h"
#include <math.h>
#include "ModelTrainerIO.h"

namespace eagleeye
{
ObjectDetGamblerStrategy::ObjectDetGamblerStrategy(const char* name)
:ObjectDetSymbol(name,NON_TERMINAL)
{
	m_gambler = new MultiArmedBandit;
	m_gambler->setGambleStrategy(MultiArmedBandit::EXP3);
	m_gambler->setGammar(0.2f);
	m_gambler_select = 0;

	m_gambling_reward_mode = PIXEL_BASED_GAMBLING_REWARD;
}
ObjectDetGamblerStrategy::~ObjectDetGamblerStrategy()
{
	if (m_gambler)
		delete m_gambler;
}

void* ObjectDetGamblerStrategy::getSymbolScore(SearchSpace pos_index,SearchMode select_mode,Matrix<float> pos_mat /* = Matrix<float>() */)
{
	if (select_mode != OPTIMUM_SEARCH)
		return NULL;

	if (!m_data_update_flag)
		return &m_score_pyr;

	//gambling 
	m_gambler_select = m_gambler->tryAgain();
	m_score_pyr=*((ScorePyramid*)(m_rule_pool[m_gambler_select]->getParseScore(pos_index,select_mode,pos_mat)));

	//disable data update flag
	m_data_update_flag = false;

	return &m_score_pyr;
}

void ObjectDetGamblerStrategy::findModelLatentInfo(void* info)
{
	DetSymbolInfo def_info = *((DetSymbolInfo*)info);
	def_info.val = m_score_pyr[def_info.level].at(def_info.y,def_info.x);
	
	for (int i = 0; i < int(m_rule_pool.size()); ++i)
	{
		if (i == m_gambler_select)
			m_rule_pool[m_gambler_select]->findModelLatentInfo(&def_info);
		else
			m_rule_pool[i]->disableSubTree();
	}
}

void ObjectDetGamblerStrategy::initializeUnit()
{
	if (EagleeyeIO::isexist(m_model_folder + m_unit_name + "_gamble_info.dat"))
	{
		m_gambler->loadMABModel((m_model_folder + m_unit_name + "_gamble_info.dat").c_str());
		m_gambler->initialize();
	}
}

void ObjectDetGamblerStrategy::setGamblerGammar(float gammar)
{
	m_gambler->setGammar(gammar);
}

class GamblerRewardBasedPixelIndex
{
public:
	GamblerRewardBasedPixelIndex(std::vector<LinkRule*> rule_pools,SampleState sample_state)
	{
		m_rule_pools = rule_pools;
		m_sample_state = sample_state;
	}
	float operator()(int index)
	{
		float reward = 0.0f;
		ScorePyramid parse_score_pyr = *((ScorePyramid*)(m_rule_pools[index]->getParseScore(PIXEL_SPACE,INDEPENDENT_SEARCH)));
		float max_score = -EAGLEEYE_FINF;
		for (int level_index = parse_score_pyr.interval; level_index < parse_score_pyr.levels(); ++level_index)
		{
			Matrix<float> score = parse_score_pyr[level_index];
			int rows = score.rows();
			int cols = score.cols();
			for (int i = 0; i < rows; ++i)
			{
				float* score_data = score.row(i);
				for (int j = 0; j < cols; ++j)
				{
					if (max_score < score_data[j])
					{
						max_score = score_data[j];
					}
				}
			}
		}

		switch(m_sample_state)
		{
		case EAGLEEYE_POSITIVE_SAMPLE:
			{
				//for positive sample
				reward = 1.0f / (1.0f + exp(-max_score));
				break;
			}
		case EAGLEEYE_NEGATIVE_SAMPLE:
			{
				//for negative sample
				reward = 1.0f - 1.0f / (1.0f + exp(-max_score));
				break;
			}
		}

		return reward;
	}
private:
	std::vector<LinkRule*> m_rule_pools;
	SampleState m_sample_state;
};

void ObjectDetGamblerStrategy::learnUnit(const char* parse_file)
{
	//write code here directly
	if (!EagleeyeIO::isexist(m_model_folder + m_unit_name + "_gamble_info.dat"))
	{
		//set how many machines in front of gambler
		m_gambler->setSlotMachinesIndex(m_rule_pool.size());
		m_gambler->initialize();

		//training
		std::string parse_file_path = m_model_folder + parse_file;
		std::map<std::string,std::vector<Array<int,4>>> p_samples;
		std::map<std::string,std::vector<Array<int,4>>> n_samples;
		analyzeSimpleParseFile(parse_file_path.c_str(),p_samples,n_samples);

		ImageReadNode<ImageSignal<unsigned char>> img_read_node;

		//parse all positive samples
		std::map<std::string,std::vector<Array<int,4>>>::iterator p_iter,p_iend(p_samples.end());
		for (p_iter = p_samples.begin(); p_iter != p_iend; ++p_iter)
		{
			if (!(p_iter->first.empty()) && !(p_iter->second.empty()))
			{
				//display training process
				EAGLEEYE_INFO("training samples %s \n",p_iter->first.c_str());

				//read sample image
				std::string sample_file = p_iter->first;
				img_read_node.setFilePath((m_model_folder + sample_file).c_str());
				img_read_node.start();

				Matrix<unsigned char> sample_img = img_read_node.getImage();

				std::vector<Array<int,4>>::iterator region_iter,region_iend(p_iter->second.end());
				for (region_iter = p_iter->second.begin(); region_iter != region_iend; ++region_iter)
				{
					Array<int,4> object_region = (*region_iter);
					//using every link rule to parse this training sample
					for (int rule_index = 0; rule_index < int(m_rule_pool.size()); ++rule_index)
					{
						//first step set data
						m_rule_pool[rule_index]->setModelData(&sample_img,EAGLEEYE_POSITIVE_SAMPLE,&object_region);
						
						switch(m_gambling_reward_mode)
						{
						case PIXEL_BASED_GAMBLING_REWARD:
							{
								//second step parse
								m_rule_pool[rule_index]->getParseScore(PIXEL_SPACE,INDEPENDENT_SEARCH);
								break;
							}
						default:
							{
								//don't support
								EAGLEEYE_ERROR("don't support gambling reward\n");
								break;
							}
						}
					}

					//training gambler
					m_gambler->tryAgain<GamblerRewardBasedPixelIndex>(GamblerRewardBasedPixelIndex(m_rule_pool,EAGLEEYE_POSITIVE_SAMPLE));
				}
			}
		}

		//parse all negative samples 
		std::map<std::string,std::vector<Array<int,4>>>::iterator n_iter,n_end(n_samples.end());
		for (n_iter = n_samples.begin(); n_iter != n_end; ++n_iter)
		{
			if (!(n_iter->first.empty()))
			{
				//display training process
				EAGLEEYE_INFO("training samples %s \n",n_iter->first.c_str());

				//read sample image
				std::string sample_file = n_iter->first;
				img_read_node.setFilePath((m_model_folder + sample_file).c_str());
				img_read_node.start();

				Matrix<unsigned char> sample_img = img_read_node.getImage();

				//not used for negative samples
				Array<int,4> object_region;
				object_region[0] = 0; object_region[1] = sample_img.cols();
				object_region[2] = 0; object_region[3] = sample_img.rows();

				//using every link rule to parse this training sample
				for (int rule_index = 0; rule_index < int(m_rule_pool.size()); ++rule_index)
				{
					//first step set data
					m_rule_pool[rule_index]->setModelData(&sample_img,EAGLEEYE_NEGATIVE_SAMPLE,&object_region);

					switch(m_gambling_reward_mode)
					{
					case PIXEL_BASED_GAMBLING_REWARD:
						{
							//second step parse
							m_rule_pool[rule_index]->getParseScore(PIXEL_SPACE,INDEPENDENT_SEARCH);
							break;
						}
					default:
						{
							//don't support
							EAGLEEYE_ERROR("don't support gambling reward\n");
							break;
						}
					}
				}

				//training gambler
				m_gambler->tryAgain<GamblerRewardBasedPixelIndex>(GamblerRewardBasedPixelIndex(m_rule_pool,EAGLEEYE_NEGATIVE_SAMPLE));
			}
		}

		m_gambler->saveMABModel((m_model_folder + m_unit_name + "_gamble_info.dat").c_str());
	}

	m_gambler->loadMABModel((m_model_folder + m_unit_name + "_gamble_info.dat").c_str());
}
}