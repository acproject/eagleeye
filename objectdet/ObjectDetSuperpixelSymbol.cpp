#include "ObjectDetSuperpixelSymbol.h"
#include "Learning/dictionary/Dictionary.h"
#include "MatrixMath.h"
#include "ProcessNode/ImageWriteNode.h"
#include "Trainer/ExtractFeatureTrainer.h"
#include "SignalFactory.h"
#include "UnitManager.h"
#include "EagleeyeStr.h"
#include "Matlab/MatlabInterface.h"
#include "ShapeHotPointFeatureDetector.h"
#include "SuperpixelFeatureDetector.h"
#include <map>

namespace eagleeye
{
ObjectDetSuperpixelSymbol::ObjectDetSuperpixelSymbol(const char* name)
	:ObjectDetSymbol(name,TERMINAL)
{
	m_gray_threshold = 180;	//gray with larger than threshold is invalid
	m_sampling_num_in_superpixel = 10000;				//sampling number in every superpixel

	m_bow_descriptor = new BOWDescriptorExtractorOpenCV();
	m_descriptor = NULL;

	m_extract_feature_trainer = NULL;
	m_dic_trainer = NULL;
	m_libsvm_trainer = NULL;

	m_dic_capacity = 0;
	m_superpixel_num = 0;
	m_gauss_kernel_size = 3;
}
ObjectDetSuperpixelSymbol::~ObjectDetSuperpixelSymbol()
{
	if (m_bow_descriptor)
		delete m_bow_descriptor;

	if (m_descriptor)
		delete m_descriptor;

	//we don't destroy others
}

void* ObjectDetSuperpixelSymbol::getSymbolScore(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat/* not used*/)
{
	//only support compute score in superpixel search space
	if (search_space != SUPERPIXEL_SPACE || search_mode != INDEPENDENT_SEARCH)
	{
		EAGLEEYE_ERROR("only support search space 'SUPERPIXEL SPACE' and search mode 'INDEPENDENT_SEARCH'\n");
		return NULL;
	}

	//if data isn't updated, it wouldn't compute score again
	if (!m_data_update_flag)
		return &m_superpixel_score;

	//get unit feature
	Matrix<float> unit_features = getUnitFeature(search_space,INDEPENDENT_SEARCH);
	Matrix<float> superpixel_description = unit_features(Range(0,unit_features.rows()),Range(SUPERPIXEL_FEATURE_DATA_OFFSET,unit_features.cols()));

	//computing score in superpixel search space
	//feature whiting
	WhitingParam feature_whiting_param;
	feature_whiting_param.mean_vec = m_whiting_data(Range(0,1),Range(0,m_dic_capacity));
	feature_whiting_param.var_vec = m_whiting_data(Range(1,2),Range(0,m_dic_capacity));
	superpixel_description = featureNormalize(superpixel_description,WHITEING,&feature_whiting_param);

	//using libsvm to classify
	Matrix<int> predict_label;
	Matrix<float> predict_probability;
	m_libsvm.predict(superpixel_description,predict_label,predict_probability);

	m_superpixel_score = Matrix<float>(m_superpixel_num,SUPERPIXEL_SCORE_OFFSET + m_class_num * 2,float(-EAGLEEYE_FINF));

	if (predict_probability.isempty())
	{
		//don't use probability
		for (int superpixel_index = 0; superpixel_index < m_superpixel_num; ++superpixel_index)
		{
			m_superpixel_score(superpixel_index,0) = float(superpixel_index);			//superpixel index
			m_superpixel_score(superpixel_index,1) = float(0);							//level index in pyramid
			m_superpixel_score(superpixel_index,2) = float(1);							//scale in pyramid
			int map_index = m_superpixel_index_map[superpixel_index];					//

			if (map_index != -1)
			{
				//this superpixel could be predicted by libsvm
				float l = float(predict_label[map_index]);
				for(int i = 0; i < m_class_num; ++i)
				{
					if (i == l)
					{
						m_superpixel_score(superpixel_index,SUPERPIXEL_SCORE_OFFSET + i * 2) = float(i);
						m_superpixel_score(superpixel_index,SUPERPIXEL_SCORE_OFFSET + i * 2 + 1) = 1.0f;
					}
					else
					{
						m_superpixel_score(superpixel_index,SUPERPIXEL_SCORE_OFFSET + i * 2) = float(i);
						m_superpixel_score(superpixel_index,SUPERPIXEL_SCORE_OFFSET + i * 2 + 1) = 0.0f;
					}
				}
			}
			else
			{
				//this superpixel couldn't be predicted by libsvm
				//assign -INF
				m_superpixel_score(superpixel_index,0) = float(superpixel_index);
				m_superpixel_score(superpixel_index,1) = 0.0f;
				m_superpixel_score(superpixel_index,2) = 1.0f;

				for (int i = 0; i < m_class_num; ++i)
				{
					m_superpixel_score(superpixel_index,SUPERPIXEL_SCORE_OFFSET + i * 2) = float(i);
					m_superpixel_score(superpixel_index,SUPERPIXEL_SCORE_OFFSET + i * 2 + 1) = -EAGLEEYE_FINF;
				}
			}
		}
	}
	else
	{
		//use probability
		for (int superpixel_index = 0; superpixel_index < m_superpixel_num; ++superpixel_index)
		{
			m_superpixel_score(superpixel_index,0) = float(superpixel_index);			//superpixel index
			m_superpixel_score(superpixel_index,1) = float(0);							//level index in pyramid
			m_superpixel_score(superpixel_index,2) = float(1);							//scale in pyramid
			int map_index = m_superpixel_index_map[superpixel_index];					//

			if (map_index != -1)
			{
				//this superpixel could be predicted by libsvm
				for (int i = 0; i < m_class_num; ++i)
				{
					m_superpixel_score(superpixel_index,SUPERPIXEL_SCORE_OFFSET + i * 2) = float(i);
					m_superpixel_score(superpixel_index,SUPERPIXEL_SCORE_OFFSET + i * 2 + 1) = predict_probability(map_index,i);
				}
			}
			else
			{
				//this superpixel couldn't be predicted by libsvm
				//assign -INF
				m_superpixel_score(superpixel_index,0) = float(superpixel_index);
				m_superpixel_score(superpixel_index,1) = 0.0f;
				m_superpixel_score(superpixel_index,2) = 1.0f;

				for (int i = 0; i < m_class_num; ++i)
				{
					m_superpixel_score(superpixel_index,SUPERPIXEL_SCORE_OFFSET + i * 2) = float(i);
					m_superpixel_score(superpixel_index,SUPERPIXEL_SCORE_OFFSET + i * 2 + 1) = -EAGLEEYE_FINF;
				}
			}
		}
	}
	
	return &m_superpixel_score;
}

void ObjectDetSuperpixelSymbol::findModelLatentInfo(void* info)
{
	DetSymbolInfo latent_info = *((DetSymbolInfo*)info);
	m_superpixel_label_map[latent_info.superpixel] = latent_info.label;
}

void ObjectDetSuperpixelSymbol::setUnitData(void* data, SampleState sample_state,void* auxiliary_data)
{
	Matrix<unsigned char> img = *((Matrix<unsigned char>*)data);
	if (img.dataptr() != m_detect_img.dataptr())
	{
		m_detect_img = img;
	}

	m_sample_state = sample_state;

	//auxiliary info struct
	AuxiliaryInfoInSuperpixelSpace* info = (AuxiliaryInfoInSuperpixelSpace*)auxiliary_data;
	m_superpixel_num = info->superpixel_num_pyr[0];
	m_superpixel_img = info->superpixel_pyr[0];

	//when training, we need annotation label
	if (m_sample_state != EAGLEEYE_UNDEFINED_SAMPLE)
		m_annotation_label = info->annotation_label;
}

//help struct
struct _Parameter
{
	char dictionary_file[EAGLEEYE_MAX_PATH];
	char svm_module_file[EAGLEEYE_MAX_PATH];
	float gray_threshold;
	int superpixel_area_limit;
	int invalid_label;
	int gauss_kernel_size;
	char feature_descriptor_block[EAGLEEYE_MAX_BLOCK];
	int class_num;
};

void ObjectDetSuperpixelSymbol::setUnitPara(MemoryBlock param_block)
{
	_Parameter* symbol_param = (_Parameter*)param_block.block();
	m_dictionary_file = symbol_param->dictionary_file;
	m_classifier_file = symbol_param->svm_module_file;
	m_gray_threshold = symbol_param->gray_threshold;
	m_invalid_label = symbol_param->invalid_label;
	m_gauss_kernel_size = (float)symbol_param->gauss_kernel_size;
	m_class_num = symbol_param->class_num;
	setFeatureDescriptorBlock(symbol_param->feature_descriptor_block);

	//load dictionary
	loadDictionary(m_dictionary_file);

	//load svm model
	loadSVMModel(m_classifier_file);

	//extra memory
	m_whiting_data = Matrix<float>(2,m_dic_capacity);
	char* extra_ptr = ((char*)param_block.block()) + sizeof(_Parameter);
	memcpy(m_whiting_data.dataptr(),extra_ptr,sizeof(float) * 2 * m_dic_capacity);
}

void ObjectDetSuperpixelSymbol::getUnitPara(MemoryBlock& param_block)
{
	param_block = MemoryBlock(sizeof(_Parameter) + 2 * m_dic_capacity * sizeof(float));	//_Parameter + whiting parameter
	_Parameter* symbol_param = (_Parameter*)param_block.block();
	copystr(symbol_param->dictionary_file,m_dictionary_file.c_str());
	copystr(symbol_param->svm_module_file,m_classifier_file.c_str());
	symbol_param->gray_threshold = m_gray_threshold;
	symbol_param->invalid_label = m_invalid_label;
	symbol_param->gauss_kernel_size = (int)m_gauss_kernel_size;
	symbol_param->class_num = m_class_num;

	//descriptor block
	MemoryBlock descriptor_block;
	m_descriptor->wrapToUnitBlock(descriptor_block);
	memcpy(symbol_param->feature_descriptor_block,descriptor_block.block(),descriptor_block.blockSize());

	//extra memory
	char* extra_ptr = ((char*)param_block.block()) + sizeof(_Parameter);
	memcpy(extra_ptr,m_whiting_data.dataptr(),sizeof(float) * 2 * m_dic_capacity);
}

void ObjectDetSuperpixelSymbol::setDescriptorExtractor(DescriptorExtractor* descriptor)
{
	m_descriptor = descriptor;
	m_bow_descriptor->setDescriptorExtractor(m_descriptor);
}

void ObjectDetSuperpixelSymbol::learnUnit(const char* parse_file)
{
	std::string parse_file_str = parse_file;
	InfoSignal<std::string> parse_file_info;
	parse_file_info.info = &parse_file_str;

	//dictionary trainer
	m_dic_trainer->setInputPort(&parse_file_info);
	m_dic_trainer->setTrainerFolder(m_model_folder);
	m_dic_trainer->setTrainerName(m_unit_name + "_dic");
	m_dic_trainer->setInvalidLabel(m_invalid_label);
	m_dic_trainer->setDescriptorExtractor(m_descriptor);
	m_dic_trainer->setGaussianProcessFlag(true,(int)m_gauss_kernel_size,(int)m_gauss_kernel_size);
	m_dic_trainer->startTrain();

	//dictionary file
	m_dictionary_file = TO_INFO(std::string,m_dic_trainer->getOutputPort(DictionaryTrainer::OUTPUT_PORT_DIC_FILE_INFO)) + 
		m_dic_trainer->getExtName();
	loadDictionary(m_dictionary_file);

	m_extract_feature_trainer->setInputPort(&parse_file_info);
	m_extract_feature_trainer->setTrainerFolder(m_model_folder);
	m_extract_feature_trainer->setInvalidLabel(m_invalid_label);
	m_extract_feature_trainer->setTrainerName(m_unit_name + "_feature");
	m_extract_feature_trainer->setWhitingFlag(true);
	m_extract_feature_trainer->setHoldSymbol(this);
	m_extract_feature_trainer->startTrain();


	//feature whiting file
	std::string whiting_file = TO_INFO(std::string,m_extract_feature_trainer->getOutputPort(ExtractFeatureTrainer::OUTPUT_PORT_SAMPLES_WHITING_INFO)) + 
		m_extract_feature_trainer->getExtName();

	//load whiting parameters
	EagleeyeIO::read(m_whiting_data,m_model_folder + whiting_file,READ_BINARY_MODE);

	//classifier trainer
	m_libsvm_trainer->setInputPort(m_extract_feature_trainer->getOutputPort(ExtractFeatureTrainer::OUTPUT_PORT_SAMPLES_INFO),
		ClassifierTrainer::INPUT_PORT_SAMPLES_INFO);
	m_libsvm_trainer->setTrainerFolder(m_model_folder);
	m_libsvm_trainer->setTrainerName(m_unit_name + "_classifier");
	m_libsvm_trainer->startTrain();

	//classifier file
	m_classifier_file = TO_INFO(std::string,m_libsvm_trainer->getOutputPort()) + 
		m_libsvm_trainer->getExtName();

	//load svm model
	loadSVMModel(m_classifier_file);
}

void ObjectDetSuperpixelSymbol::setFeatureDescriptorBlock(char* data_block)
{
	if (m_descriptor)
	{
		delete m_descriptor;
	}

	//get feature descriptor name
	char name[EAGLEEYE_MAX_NAME];
	memcpy(name,data_block,sizeof(char) * EAGLEEYE_MAX_NAME);
	m_descriptor = dynamic_cast<DescriptorExtractor*>(UnitManager::factory(name));

	//get parameter configuration
	MemoryBlock param_block(EAGLEEYE_MAX_BLOCK - EAGLEEYE_MAX_NAME);
	memcpy(param_block.block(),data_block + EAGLEEYE_MAX_NAME,sizeof(char) * (EAGLEEYE_MAX_BLOCK - EAGLEEYE_MAX_NAME));
	m_descriptor->setUnitPara(param_block);

	m_bow_descriptor->setDescriptorExtractor(m_descriptor);
}

void ObjectDetSuperpixelSymbol::setGrayThreshold(float gray_threshold)
{
	m_gray_threshold = gray_threshold;
}

void ObjectDetSuperpixelSymbol::setSamplingNumInSuperpixel(int num)
{
	m_sampling_num_in_superpixel = num;
}

void ObjectDetSuperpixelSymbol::setGaussianKernelSize(int kernel_size)
{
	m_gauss_kernel_size = (float)kernel_size;
}

Matrix<float> ObjectDetSuperpixelSymbol::getUnitFeature(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat)
{
	if (search_space != SUPERPIXEL_SPACE || search_mode != INDEPENDENT_SEARCH)
		return Matrix<float>();

	//get rid of noise
	m_detect_img  = gaussFilter(m_detect_img,(unsigned int)m_gauss_kernel_size,(unsigned int)m_gauss_kernel_size);

	//image size
	int rows = m_detect_img.rows();
	int cols = m_detect_img.cols();

	Matrix<unsigned char> superpixel_label;

/*	putToMatlab(m_detect_img,"det");*/

	//squeeze invalid superpixel
	//extracting feature way is different at training and detecting stages
	switch(m_sample_state)
	{
	case EAGLEEYE_POSITIVE_SAMPLE:
	case EAGLEEYE_NEGATIVE_SAMPLE:
		{
			Matrix<int> after_superpixel_index;
			int after_superpixel_num;
			Matrix<int> after_pixel_num_in_superpixel;

			//training and learning
			squeezeInvalidSuperpixel(m_superpixel_img,
				m_superpixel_num,
				m_annotation_label,
				after_superpixel_index,
				after_superpixel_num,
				after_pixel_num_in_superpixel,
				superpixel_label);

			m_superpixel_img = after_superpixel_index;
			m_superpixel_num = after_superpixel_num;
			break;
		}
	case EAGLEEYE_UNDEFINED_SAMPLE:
		{ 
			//adjusting superpixel
			Matrix<int> after_superpixel_index;
			int after_superpixel_num;
			Matrix<int> after_pixel_num_of_every_superpixel;

			squeezeInvalidSuperpixel(m_superpixel_img,
				m_superpixel_num,
				m_detect_img,
				(unsigned char)m_gray_threshold,
				after_superpixel_index,
				after_superpixel_num,
				after_pixel_num_of_every_superpixel,
				superpixel_label);

			m_superpixel_num = after_superpixel_num;
			m_superpixel_img = after_superpixel_index;
			break;
		}
	}
	
	//finding exclude region
	Matrix<unsigned char> exclude_region(rows,cols); 
	for (int i = 0; i < rows; ++i)
	{
		unsigned char* exclude_data = exclude_region.row(i);
		int* superpixel_img_data = m_superpixel_img.row(i);
		for (int j = 0; j < cols; ++j)
		{
			if (superpixel_label(superpixel_img_data[j]) == m_invalid_label)
				exclude_data[j] = 0;
			else
				exclude_data[j] = 1;
		}
	}

	ShapeHotPointFeatureDetector shape_hot_detect;
	shape_hot_detect.setCannyThreshold(50.0,70.0);
	shape_hot_detect.setRandomJitterSwitch(true,10.0f,30);
	shape_hot_detect.setImageBounds(10,10);
	shape_hot_detect.setExcludeRegion(exclude_region);
	std::vector<KeyPoint> keypoints;
	shape_hot_detect.detect(m_detect_img.transform<float>(),keypoints);

// 	Matrix<unsigned char> test_img(rows,cols);
// 	test_img.copy(m_detect_img);
// 
// 	std::vector<KeyPoint>::iterator iter,iend(keypoints.end());
// 	for (iter = keypoints.begin(); iter != iend; ++iter)
// 	{
// 		int r = (*iter).pt[1];
// 		int c = (*iter).pt[0];
// 		test_img(r,c) = 255;
// 	}
// 	putToMatlab(test_img,"test_img");

	//extract superpixel representation
	Matrix<float> superpixel_description;
	std::vector<bool> superpixel_flag;
	m_bow_descriptor->compute(m_detect_img.transform<float>(),m_superpixel_img,m_superpixel_num,keypoints,superpixel_description,superpixel_flag);

// 	putToMatlab(superpixel_description);
// 
// 	putToMatlab(m_superpixel_img,"sup_img");

	//get rid of some invalid superpixel representation
	//get valid superpixel number
	int valid_superpixel_num = 0;
	for (int i = 0; i < int(superpixel_flag.size()); ++i)
	{
		if (superpixel_flag[i])
		{
			m_superpixel_index_map[i] = valid_superpixel_num;
			valid_superpixel_num++;
		}
		else
		{
			m_superpixel_index_map[i] = -1;
		}
	}

	Matrix<float> unit_feature(valid_superpixel_num,superpixel_description.cols() + SUPERPIXEL_FEATURE_HEAD_SIZE,0.0f);
	int valid_superpixel_count = 0;

	for (int i = 0; i < int(superpixel_flag.size()); ++i)
	{
		if (superpixel_flag[i])
		{
			float* data = unit_feature.row(valid_superpixel_count);
			float* feature_data = data + SUPERPIXEL_FEATURE_DATA_OFFSET; 
			memcpy(feature_data,superpixel_description.row(i),sizeof(float) * superpixel_description.cols());
			
			SUPERPIXEL_FEATURE_LABEL(data) = float(superpixel_label[i]);		//superpixel label (3)
			SUPERPIXEL_FEATURE_INDEX(data) = float(i);							//superpixel index (0)
			data[1] = 0.0f;														//pyramid level (1)
			data[2] = 1.0f;														//pyramid scale (2)
			valid_superpixel_count++;
		}
	}

	return unit_feature;
}

void ObjectDetSuperpixelSymbol::getSymbolOutput(std::vector<void*>& output_info)
{
	m_output_info = Matrix<float>(m_superpixel_num,SUPERPIXEL_SCORE_OFFSET + 2);
	for (int index = 0; index < m_superpixel_num; ++index)
	{
		m_output_info(index,0) = float(index);		//superpixel index
		m_output_info(index,1) = 0.0f;				//superpixel level
		m_output_info(index,2) = 1.0f;				//superpixel level scale
		
		if (m_superpixel_label_map[index] != -1)
		{
			m_output_info(index,3) = float(m_superpixel_label_map[index]);
			m_output_info(index,4) = 1.0f;
		}
		else
		{
			m_output_info(index,3) = float(m_invalid_label);
			m_output_info(index,4) = -EAGLEEYE_FINF;
		}
	}

	output_info.push_back(&m_output_info);
}

void ObjectDetSuperpixelSymbol::loadDictionary(std::string dictionary_file)
{
	if (dictionary_file.empty())
		return;
	
	//load dictionary data
	Matrix<float> dic = Dictionary::loadDictionary((m_model_folder + dictionary_file).c_str());
	m_bow_descriptor->setVocabulary(dic);

	//feature dimension
	m_dic_capacity = dic.rows();
}
void ObjectDetSuperpixelSymbol::loadSVMModel(std::string svm_model_file)
{
	if (svm_model_file.empty())
		return;

	//load libsvm model
	m_libsvm.readSVMModel((m_model_folder + svm_model_file).c_str());

	//class number
	m_class_num = m_libsvm.getClassNum();
}

//set symbol trainer
void ObjectDetSuperpixelSymbol::setSymbolTrainer(DictionaryTrainer* dic_trainer,
												 SVMClassifierTrainer* libsvm_trainer,
												 ExtractFeatureTrainer* extract_feature_trainer)
{
	m_dic_trainer = dic_trainer;
	m_libsvm_trainer = libsvm_trainer;
	m_extract_feature_trainer = extract_feature_trainer;
}
}