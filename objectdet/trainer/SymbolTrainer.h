#ifndef _MODELTRAINER_H_
#define _MODELTRAINER_H_

#include "EagleeyeMacro.h"
#include "AnyNode.h"
#include "SharedData.h"
#include "GrammarTree.h"
#include "ModelTrainerIO.h"
#include "SignalFactory.h"
namespace eagleeye
{
class EAGLEEYE_API SymbolTrainer:public AnyNode
{
public:
	SymbolTrainer();
	virtual ~SymbolTrainer();

	/**
	 *	@brief define class identity
	 */
	EAGLEEYE_CLASSIDENTITY(SymbolTrainer);

	/**
	 *	@brief set input port
	 */
	EAGLEEYE_INPUT_PORT_TYPE(InfoSignal<std::string>,0,PARSE_FILE_INFO);

	/**
	 *	@brief set training data file name
	 */
	void setParseFile(std::string file_name);

	/**
	 *	@brief set trainer folder
	 */
	void setTrainerFolder(std::string folder);

	/**
	 *	@brief set trainer name
	 */
	void setTrainerName(std::string name);

	/**
	 *	@brief load training samples file
	 */
	void loadObjectDetTrainingSampls(
		std::map<std::string,std::vector<Array<int,4>>>& p_samples,
		std::map<std::string,std::vector<Array<int,4>>>& n_samples);

	void loadObjectDetTrainingSampls(std::map<std::string,std::vector<Array<int,4>>>& p_samples);

	bool loadObjectClassifyTrainingSamples(std::map<std::string,std::string>& samples_and_annotation,
		std::map<std::string,Array<int,4>>& samples_regions,
		std::map<std::string,int>& samples_labels,
		int& labels_num);

	/**
	 *	@brief set invalid label
	 */
	void setInvalidLabel(int invalid_label);

	/**
	 *	@brief set x and y scale
	 */
	void setResizeFlag(bool flag,float x_scale = 1.0f,float y_scale = 1.0f);

	/**
	 *	@brief set gaussian process flag
	 */
	void setGaussianProcessFlag(bool flag,int kernel_x = 3.0f,int kernel_y = 3.0f);

	/**
	 *	@brief set fixed offset
	 */
	void setFixedOffset(int x,int y);

	/**
	 *	@brief set image bound
	 *	@note when computing some important data, we should skip image bounds.
	 */
	void setImageBounds(int bound_width,int bound_height);

	/**
	 *	@brief set whiting flag
	 */
	void setWhitingFlag(bool flag);

	/**
	 *	@brief set/get extent name of trainer file
	 */
	static void setExtName(const char* ext_name);
	static const char* getExtName();

	/**
	 *	@brief we could specify one custom member to start training.
	 *	@note It only call "start".
	 */
	void startTrain();

	/**
	 *	@brief construct one model
	 */
	virtual void passonNodeInfo();

	/**
	 *	@brief process node info.
	 */
	virtual void executeNodeInfo();

protected:
	/**
	 *	@brief It's only one custom member to complement training
	 *	@note We have implemented this function in the subclass
	 */
	virtual void train() = 0;

	/**
	 *	@brief make one output signal
	 *	@note std::string signal
	 */
	virtual AnySignal* makeOutputSignal();

	std::string m_parse_file_name;				/**< parse file name*/
	std::string m_trainer_folder;				/**< trainer info would be saved in this folder*/
	std::string m_trainer_name;					/**< trainer name*/
	std::string m_trainer_info;					/**< trainer info*/
	
	int m_invalid_label;						/**< invalid label*/
	
	static std::string m_ext_name;				/**< extent name*/
	
	bool m_is_resize_flag;						/**< whether resize image*/
	Array<float,2> m_scales;					/**< resize scale*/

	bool m_is_gaussian_flag;					/**< whether smooth image*/
	Array<int,2> m_kernel_size;					/**< guassian kernel size*/

	Array<int,2> m_fixed_offset;				/**< some fixe offset*/	
	Array<int,2> m_img_bound;					/**< image bound*/

	bool m_whiting_flag;						/**< whether whiting features*/

private:
	SymbolTrainer(const SymbolTrainer&);
	void operator=(const SymbolTrainer&);
};
}

#endif