#include "AutoMakeObjectDetTerminalSymbols.h"
#include "ModelTrainerIO.h"

namespace eagleeye
{
AutoMakeObjectDetTerminalSymbolsTrainer::AutoMakeObjectDetTerminalSymbolsTrainer()
{
	m_objectdet_terminal_syms = NULL;
	m_objectdet_terminal_sym_num = 1;
}
AutoMakeObjectDetTerminalSymbolsTrainer::~AutoMakeObjectDetTerminalSymbolsTrainer()
{
	if (m_objectdet_terminal_syms)
	{
		delete m_objectdet_terminal_syms;
	}
}
void AutoMakeObjectDetTerminalSymbolsTrainer::setObjectDetTerminalSymbolsNum(int num)
{
	m_objectdet_terminal_sym_num = num;
}
ObjectDetTerminalSymbol* AutoMakeObjectDetTerminalSymbolsTrainer::getObjectDetTerminalSymbol(int index)
{
	return m_objectdet_terminal_syms[index];
}

void AutoMakeObjectDetTerminalSymbolsTrainer::learn(const char* model_folder,const char* parse_file)
{
	//generate some ObjectDetTerminalSymbols with proper detect window
	std::string parse_file_path = std::string(model_folder) + parse_file;
	std::map<std::string,std::vector<Array<int,4>>> p_samples;
	analyzeSimpleParseFile(parse_file_path.c_str(),p_samples);

	autoSplit(p_samples);

	if (!m_objectdet_terminal_syms)
	{
		m_objectdet_terminal_syms = new ObjectDetTerminalSymbol*[m_objectdet_terminal_sym_num];
	}

	for (int i = 0; i < m_objectdet_terminal_sym_num; ++i)
	{
		char buf[100];
		std::string id_name = std::string(itoa(m_objectdet_wins[i].first,buf,10)) + "-" + std::string(itoa(m_objectdet_wins[i].second,buf,10));
		m_objectdet_terminal_syms[i] = new ObjectDetTerminalSymbol(id_name.c_str());
		m_objectdet_terminal_syms[i]->setImageWin(m_objectdet_wins[i].first,m_objectdet_wins[i].second);

		float step = (m_max_ratio - m_min_ratio) / m_objectdet_terminal_sym_num;
		float sym_accepted_min_ratio = m_min_ratio + i * step;
		float sym_accepted_max_ratio = m_min_ratio + (i + 1) * step;
		m_objectdet_terminal_syms[i]->setAcceptedHeightWidthRatio(sym_accepted_min_ratio,sym_accepted_max_ratio);
	}
}

void AutoMakeObjectDetTerminalSymbolsTrainer::autoSplit(std::map<std::string,std::vector<Array<int,4>>> samples)
{
	m_min_ratio = 100000.0f;
	m_max_ratio = 0.0f;

	std::map<std::string,std::vector<Array<int,4>>>::iterator iter,iend(samples.end());
	for (iter = samples.begin(); iter != iend; ++iter)
	{
		std::vector<Array<int,4>> object_regions = iter->second;
		int regins_num = object_regions.size();
		for (int i = 0; i < regins_num; ++i)
		{
			float object_width = float(object_regions[i][1] - object_regions[i][0]);
			float object_height = float(object_regions[i][3] - object_regions[i][2]);

			float ratio = object_height / object_width;
			if (m_min_ratio > ratio)
			{
				m_min_ratio = ratio;
			}
			if (m_max_ratio < ratio)
			{
				m_max_ratio = ratio;
			}
		}
	}

	float region_interval = (m_max_ratio - m_min_ratio) / float(m_objectdet_terminal_sym_num) + 0.001f;

	std::vector<std::vector<std::pair<int,int>>> all_winds_store(m_objectdet_terminal_sym_num);
	for (iter = samples.begin(); iter != iend; ++iter)
	{
		std::vector<Array<int,4>> object_regions = iter->second;
		int regions_num = object_regions.size();

		for (int i = 0; i < regions_num; ++i)
		{
			float object_width = float(object_regions[i][1] - object_regions[i][0]);
			float object_height = float(object_regions[i][3] - object_regions[i][2]);

			float ratio = object_height / object_width;
			int component_index = int(floor((ratio - m_min_ratio) / region_interval));
			
			all_winds_store[component_index].push_back(std::make_pair<int,int>(int(object_width),int(object_height)));
		}
	}

	m_objectdet_wins.resize(m_objectdet_terminal_sym_num);
	for (int i = 0; i < m_objectdet_terminal_sym_num; ++i)
	{
		float width_mean = 0.0f;
		float height_mean = 0.0f;
		int wins_num = all_winds_store[i].size();
		for (int m = 0; m < int(all_winds_store[i].size()); ++m)
		{
			width_mean += float(all_winds_store[i][m].first) / float(wins_num);
			height_mean += float(all_winds_store[i][m].second) / float(wins_num);
		}
		m_objectdet_wins[i].first = int(width_mean);
		m_objectdet_wins[i].second = int(height_mean);
	}
}
}