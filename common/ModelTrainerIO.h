#ifndef _MODELTRAINERIO_H_
#define _MODELTRAINERIO_H_

#include "EagleeyeMacro.h"

#include "Array.h"
#include <string>
#include <vector>
#include <map>
//////////////////////////////////////////////////////////////////////////
// P filename x1 x2 y1 y2
// N filename x1 x2 y1 y2
// P filename x1 x2 y1 y2
// P filename x1 x2 y1 y2
//////////////////////////////////////////////////////////////////////////

namespace eagleeye
{
class EAGLEEYE_API TrainingSampleInfo
{
public:
	TrainingSampleInfo();
	~TrainingSampleInfo(){};

	std::string t_address;
	std::string t_a_address;
	std::string t_x_address;
	std::vector<Array<int,4>> object_regions;
	std::vector<int> object_labels;
	int total_labels;

	static std::string unkown_flag;
};

/**
 *	@brief parse training samples file and get positive samples and
 *	negative samples
 */
EAGLEEYE_API bool analyzeSimpleParseFile(const char* file_path,
	std::map<std::string,std::vector<Array<int,4>>>& p_samples,
	std::map<std::string,std::vector<Array<int,4>>>& n_samples);

/**
 *	@brief parse training samples file and get positive samples
 */
EAGLEEYE_API bool analyzeSimpleParseFile(const char* file_path, 
	std::map<std::string,std::vector<Array<int,4>>>& p_samples);

/**
 *	@brief save positive samples and negative samples
 */
EAGLEEYE_API bool saveSimpleParseFile(const char* file_path,
	std::map<std::string,std::vector<Array<int,4>>> p_samples,
	std::map<std::string,std::vector<Array<int,4>>> n_samples);

/**
 *	@brief read/write training samples
 */
EAGLEEYE_API bool analyzeParseFile(const char* file_path,std::vector<TrainingSampleInfo>& training_sample_info);
EAGLEEYE_API bool saveParseFile(const char* file_path,std::vector<TrainingSampleInfo> training_sample_info);
}
#endif