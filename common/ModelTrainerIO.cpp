#include "ModelTrainerIO.h"
#include "Print.h"
#include <fstream>

namespace eagleeye
{
std::string TrainingSampleInfo::unkown_flag = "unkown";
TrainingSampleInfo::TrainingSampleInfo()
{
	t_address = unkown_flag;
	t_a_address = unkown_flag;
	t_x_address = unkown_flag;
	total_labels = 0;
}

bool analyzeSimpleParseFile(const char* file_path,
	std::map<std::string,std::vector<Array<int,4>>>& p_samples,
	std::map<std::string,std::vector<Array<int,4>>>& n_samples)
{
	std::ifstream samples_file(file_path);
	if (!samples_file)
	{
		EAGLEEYE_ERROR("couldn't parse training samples file\n");
		return false;
	}
	
	while(samples_file.peek()!=EOF)
	{
		std::string label_name;
		samples_file>>label_name;
		
		if (samples_file.peek()==EOF)
		{
			//find there is no data to read, we have to exit
			break;
		}

		std::string file_name;
		samples_file>>file_name;

		Array<int,4> pos;
		samples_file>>pos[0];
		samples_file>>pos[1];
		samples_file>>pos[2];
		samples_file>>pos[3];

		if (label_name == "P")
		{
			p_samples[file_name].push_back(pos);
		}
		else
		{
			n_samples[file_name].push_back(pos);
		}
	}
	
	samples_file.close();

	EAGLEEYE_INFO("success to parse training samples file\n");
	return true;
}

bool analyzeSimpleParseFile(const char* file_path, 
	std::map<std::string,std::vector<Array<int,4>>>& p_samples)
{
	std::ifstream in_file_stream(file_path);
	if (!in_file_stream)
	{
		EAGLEEYE_ERROR("couldn't parse training samples file\n");
		return false;
	}

	while(in_file_stream.peek()!=EOF)
	{
		std::string label_name;
		in_file_stream>>label_name;
		if (in_file_stream.peek() == EOF)
		{
			//find there is no data to read, we have to exit
			break;
		}

		if (label_name == "P")
		{
			std::string file_name;
			in_file_stream>>file_name;

			Array<int,4> pos;
			in_file_stream>>pos[0];
			in_file_stream>>pos[1];
			in_file_stream>>pos[2];
			in_file_stream>>pos[3];

			p_samples[file_name].push_back(pos);
		}
		else
		{
			std::string empty_str;
			getline(in_file_stream,empty_str);
		}
	}

	in_file_stream.close();

	EAGLEEYE_INFO("success to parse training samples file\n");
	return true;
}

bool saveSimpleParseFile(const char* file_path,
	std::map<std::string,std::vector<Array<int,4>>> p_samples,
	std::map<std::string,std::vector<Array<int,4>>> n_samples)
{
	std::ofstream samples_file(file_path);
	if (!samples_file)
	{
		EAGLEEYE_ERROR("couldn't open training samples file\n");
		return false;
	}

	std::map<std::string,std::vector<Array<int,4>>>::iterator p_iter,p_iend(p_samples.end());
	for (p_iter = p_samples.begin(); p_iter != p_iend; ++p_iter)
	{
		int pos_num = p_iter->second.size();
		for (int i = 0; i < pos_num; ++i)
		{
			//save label
			samples_file<<"P"<<'\t';

			//save filename
			samples_file<<p_iter->first<<'\t';

			//save pos
			samples_file<<p_iter->second[i][0]<<'\t';
			samples_file<<p_iter->second[i][1]<<'\t';
			samples_file<<p_iter->second[i][2]<<'\t';
			samples_file<<p_iter->second[i][3]<<'\n';
		}
	}

	std::map<std::string,std::vector<Array<int,4>>>::iterator n_iter,n_iend(n_samples.end());
	for (n_iter = n_samples.begin(); n_iter != n_iend; ++n_iter)
	{
		int pos_num = n_iter->second.size();
		for (int i = 0; i < pos_num; ++i)
		{
			//save label
			samples_file<<"N"<<'\t';

			//save filename
			samples_file<<n_iter->first<<'\t';

			//save pos
			samples_file<<n_iter->second[i][0]<<'\t';
			samples_file<<n_iter->second[i][1]<<'\t';
			samples_file<<n_iter->second[i][2]<<'\t';
			samples_file<<n_iter->second[i][3]<<'\n';
		}
	}

	samples_file.close();

	EAGLEEYE_INFO("success to save training samples file\n");
	return true;
}

bool analyzeParseFile(const char* file_path,std::vector<TrainingSampleInfo>& training_sample_info)
{
	std::ifstream i_samples_file(file_path);
	if (!i_samples_file)
	{
		EAGLEEYE_INFO("couldn't parse training samples file\n");
		return false;
	}

	while(i_samples_file.peek()!=EOF)
	{
		//read samples file name
		std::string training_sample_addess;
		i_samples_file>>training_sample_addess;

		if (i_samples_file.peek()==EOF)
		{
			//find there is no data to read, we have to exit
			break;
		}

		//read annotation file name
		std::string training_sample_annotation_address;
		i_samples_file>>training_sample_annotation_address;

		//read xml
		std::string training_sample_xml_address;
		i_samples_file>>training_sample_xml_address;

		std::vector<Array<int,4>> object_regions;
		std::vector<int> object_labels;
		int regions_num;
		i_samples_file>>regions_num;
		for (int i = 0; i < regions_num; ++i)
		{
			int object_label;
			i_samples_file>>object_label;
			object_labels.push_back(object_label);

			Array<int,4> object_region;
			i_samples_file>>object_region[0];
			i_samples_file>>object_region[1];
			i_samples_file>>object_region[2];
			i_samples_file>>object_region[3];
			object_regions.push_back(object_region);
		}

		int total_labels;
		i_samples_file>>total_labels;

		TrainingSampleInfo s_info;
		s_info.t_address = training_sample_addess;
		s_info.t_a_address = training_sample_annotation_address;
		s_info.t_x_address = training_sample_xml_address;
		s_info.object_regions = object_regions;
		s_info.object_labels = object_labels;
		s_info.total_labels = total_labels;

		training_sample_info.push_back(s_info);
	}

	i_samples_file.close();
	EAGLEEYE_INFO("success to parse training samples file\n");
	return true;
}

bool saveParseFile(const char* file_path,std::vector<TrainingSampleInfo> training_sample_info)
{
	std::ofstream o_samples_file(file_path);
	if (!o_samples_file)
	{
		EAGLEEYE_INFO("couldn't open training samples file\n");
		return false;
	}

	std::vector<TrainingSampleInfo>::iterator iter,iend(training_sample_info.end());
	for (iter = training_sample_info.begin(); iter != iend; ++iter)
	{
		TrainingSampleInfo s_info = (*iter);
		o_samples_file<<s_info.t_address<<'\t';
		o_samples_file<<s_info.t_a_address<<'\t';
		o_samples_file<<s_info.t_x_address<<'\t';
		
		int num = s_info.object_regions.size();
		o_samples_file<<num<<'\t';

		for (int i = 0; i < num; ++i)
		{
			o_samples_file<<s_info.object_labels[i]<<'\t';

			o_samples_file<<s_info.object_regions[i][0]<<'\t'
				<<s_info.object_regions[i][1]<<'\t'
				<<s_info.object_regions[i][2]<<'\t'
				<<s_info.object_regions[i][3]<<'\t';
		}

		o_samples_file<<s_info.total_labels;

		o_samples_file<<'\n';
	}
	
	o_samples_file.close();
	return true;
}

}
