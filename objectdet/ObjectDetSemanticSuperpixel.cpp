#include "ObjectDetSemanticSuperpixel.h"
#include "Learning/SemanticBagOfWords.h"
#include "MatrixMath.h"

namespace eagleeye
{
ObjectDetSemanticSuperpixel::ObjectDetSemanticSuperpixel(const char* name):ObjectDetSymbol(name)
{
	m_semantic_extractor = new SemanticBOWDescriptorExtractor;
	m_gray_threshold = 180.0f;
	m_kernel_size = 3;
	m_feature_extractor = NULL;
	m_dic_trainer = NULL;
	m_ef_trainer = NULL;
	m_semantic_bow_trainer = NULL;

	m_superpixel_num = 0;
	m_words_num = 0;
}
ObjectDetSemanticSuperpixel::~ObjectDetSemanticSuperpixel()
{
	if (m_semantic_extractor)
		delete m_semantic_extractor;

	std::vector<SemanticBagOfWords*>::iterator iter,iend(m_semantic_infer.end());
	for (iter = m_semantic_infer.begin(); iter != iend; ++iter)
	{
		delete (*iter);
	}
}

void ObjectDetSemanticSuperpixel::setDescriptorExtractor(DescriptorExtractor* feature_extractor)
{
	m_feature_extractor = feature_extractor;
	m_semantic_extractor->setDescriptorExtractor(m_feature_extractor);
}

void ObjectDetSemanticSuperpixel::setGrayThreshold(float gray_threshold)
{
	m_gray_threshold = gray_threshold;
}
void ObjectDetSemanticSuperpixel::setGaussianKernelSize(int kernel_size)
{
	m_kernel_size = kernel_size;
}
void* ObjectDetSemanticSuperpixel::getSymbolScore(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat /* = Matrix<float>() */)
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
	Matrix<float> semantic_des = unit_features(Range(0,unit_features.rows()),Range(SUPERPIXEL_FEATURE_DATA_OFFSET,unit_features.cols()));

	//feed to SemanticBOWDescriptorExtractor
	int sample_num = semantic_des.rows();
	Matrix<float> words_frequency = semantic_des(Range(0,sample_num),Range(0,m_words_num));
	Matrix<float> words_pair_dis = semantic_des(Range(0,sample_num),Range(m_words_num,m_clique_num));
	Matrix<float> words_pair_angle = semantic_des(Range(0,sample_num),Range(m_words_num + m_clique_num,m_words_num + 2 * m_clique_num));

	Matrix<float> words_states(sample_num,m_words_num);
	for (int s = 0; s < sample_num; ++s)
	{
		float* words_frequency_ptr = words_frequency.row(s);
		float* words_states_ptr = words_states.row(s);
		for (int w_index = 0; w_index < m_words_num; ++w_index)
		{
			if (words_frequency_ptr[w_index] > 0.1f)
				words_states_ptr[w_index] = 1.0f;
			else
				words_states_ptr[w_index] = 0.0f;
		}
	}

	int semantic_num = m_semantic_infer.size();
	for (int infer_index = 0; infer_index < semantic_num; ++infer_index)
	{
		Matrix<float> probability = m_semantic_infer[infer_index]->semanticInfer(words_states.transform<int>(),words_frequency,words_pair_dis,words_pair_angle);
		for (int s_index = 0; s_index < m_superpixel_num; ++s_index)
		{
			m_superpixel_score(s_index,0) = float(s_index);
			m_superpixel_score(s_index,1) = float(0);
			m_superpixel_score(s_index,2) = float(1);

			int map_index = m_superpixel_index_map[s_index];
			if (map_index != -1)
			{
				m_superpixel_score(s_index,SUPERPIXEL_SCORE_OFFSET + infer_index * 2) = float(infer_index);
				m_superpixel_score(s_index,SUPERPIXEL_SCORE_OFFSET + infer_index * 2 + 1) = probability(map_index);
			}
			else
			{
				m_superpixel_score(s_index,SUPERPIXEL_SCORE_OFFSET + infer_index * 2) = float(infer_index);
				m_superpixel_score(s_index,SUPERPIXEL_SCORE_OFFSET + infer_index * 2 + 1) = -EAGLEEYE_FINF;
			}
		}
	}

	return &m_superpixel_score;
}

Matrix<float> ObjectDetSemanticSuperpixel::getUnitFeature(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat)
{
	if (search_space != SUPERPIXEL_SPACE || search_mode != INDEPENDENT_SEARCH)
		return Matrix<float>();

	//get rid of noise
	m_detect_img = gaussFilter(m_detect_img,m_kernel_size,m_kernel_size);

	//image size
	int rows = m_detect_img.rows();
	int cols = m_detect_img.cols();

	//squeeze to invalid superpixel
	Matrix<unsigned char> superpixel_label;
	switch(m_sample_state)
	{
	case EAGLEEYE_POSITIVE_SAMPLE:
	case EAGLEEYE_NEGATIVE_SAMPLE:
		{
			Matrix<int> after_superpixel_img;
			int after_superpixel_num;
			Matrix<int> after_pixel_num_in_superpixel;

			//training and learning
			squeezeInvalidSuperpixel(m_superpixel_img,
				m_superpixel_num,
				m_annotation_label,
				after_superpixel_img,
				after_superpixel_num,
				after_pixel_num_in_superpixel,
				superpixel_label);

			m_superpixel_img = after_superpixel_img;
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

	//finding keypoints,which describes shape information
	ShapeHotPointFeatureDetector shape_hot_detect;
	shape_hot_detect.setCannyThreshold(50.0,70.0);
	shape_hot_detect.setRandomJitterSwitch(true,10.0f,30);
	shape_hot_detect.setImageBounds(10,10);
	shape_hot_detect.setExcludeRegion(exclude_region);
	std::vector<KeyPoint> keypoints;
	shape_hot_detect.detect(m_detect_img.transform<float>(),keypoints);

	//extract superpixel representation
	Matrix<float> semantic_s_des;
	std::vector<bool> superpixel_flag;
	m_semantic_extractor->compute(m_detect_img.transform<float>(),m_superpixel_img,m_superpixel_num,keypoints,semantic_s_des,superpixel_flag);

	//get rid of some invalid superpixel representation
	int valid_superpixel_num = 0;
	for (int i = 0; i < int(superpixel_flag.size()); ++i)
	{
		if (superpixel_flag[i])
		{
			m_superpixel_index_map[i] = valid_superpixel_num;
			valid_superpixel_num++;
		}
		else
			m_superpixel_index_map[i] = -1;
	}

	Matrix<float> unit_feature(valid_superpixel_num,semantic_s_des.cols() + SUPERPIXEL_FEATURE_HEAD_SIZE,0.0f);
	int valid_superpixel_count = 0;

	for (int i = 0; i < int(superpixel_flag.size()); ++i)
	{
		if (superpixel_flag[i])
		{
			float* data = unit_feature.row(valid_superpixel_count);
			float* feature_data = data + SUPERPIXEL_FEATURE_DATA_OFFSET; 
			memcpy(feature_data,semantic_s_des.row(i),sizeof(float) * semantic_s_des.cols());

			SUPERPIXEL_FEATURE_LABEL(data) = float(superpixel_label[i]);		//superpixel label (3)
			SUPERPIXEL_FEATURE_INDEX(data) = float(i);							//superpixel index (0)
			data[1] = 0.0f;														//pyramid level (1)
			data[2] = 1.0f;														//pyramid scale (2)
			valid_superpixel_count++;
		}
	}

	return unit_feature;
}

void ObjectDetSemanticSuperpixel::setUnitData(void* data, SampleState sample_state,void* auxiliary_data)
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
void ObjectDetSemanticSuperpixel::getUnitPara(MemoryBlock& param_block)
{

}
void ObjectDetSemanticSuperpixel::setUnitPara(MemoryBlock param_block)
{

}

void ObjectDetSemanticSuperpixel::learnUnit(const char* parse_file)
{
	std::string parse_file_str = parse_file;
	InfoSignal<std::string> parse_file_info;
	parse_file_info.info = &parse_file_str;

	//dictionary trainer
	m_dic_trainer->setInputPort(&parse_file_info);
	m_dic_trainer->setTrainerFolder(m_model_folder);
	m_dic_trainer->setTrainerName(m_unit_name + "_dic");
	m_dic_trainer->setInvalidLabel(m_invalid_label);
	m_dic_trainer->setDescriptorExtractor(m_feature_extractor);
	m_dic_trainer->setGaussianProcessFlag(true,(int)m_kernel_size,(int)m_kernel_size);
	m_dic_trainer->startTrain();

	//dictionary file
	m_dictionary_file = TO_INFO(std::string,m_dic_trainer->getOutputPort(DictionaryTrainer::OUTPUT_PORT_DIC_FILE_INFO)) + 
		m_dic_trainer->getExtName();
	loadDictionary(m_dictionary_file);

	//extract feature description
	m_ef_trainer->setInputPort(&parse_file_info);
	m_ef_trainer->setTrainerFolder(m_model_folder);
	m_ef_trainer->setInvalidLabel(m_invalid_label);
	m_ef_trainer->setTrainerName(m_unit_name + "_feature");
	m_ef_trainer->setHoldSymbol(this);
	m_ef_trainer->startTrain();

	//semantic bagofwords trainer
	m_semantic_bow_trainer->setInputPort(m_ef_trainer->getOutputPort(ExtractFeatureTrainer::OUTPUT_PORT_SAMPLES_INFO),
		ClassifierTrainer::INPUT_PORT_SAMPLES_INFO);

	m_semantic_bow_trainer->setTrainerFolder(m_model_folder);
	m_semantic_bow_trainer->setParameters(m_class_num,m_words_num,m_clique_num);
	m_semantic_bow_trainer->setTrainerName(m_unit_name + "_semantic_model");
	m_semantic_bow_trainer->startTrain();

	//semantic model file
	m_semantic_model_file = TO_INFO(std::string,m_semantic_bow_trainer->getOutputPort()) + m_semantic_bow_trainer->getExtName();

	//load semantic model
	loadSemanticModels(m_semantic_model_file);
}

void ObjectDetSemanticSuperpixel::getSymbolOutput(std::vector<void*>& output_info)
{

}
void ObjectDetSemanticSuperpixel::findModelLatentInfo(void* info)
{

}

void ObjectDetSemanticSuperpixel::loadDictionary(std::string dictionary_file)
{
	if (dictionary_file.empty())
		return;

	//load dictionary data
	m_dic = Dictionary::loadDictionary((m_model_folder + dictionary_file).c_str());

	//set dictionary matrix for semantic bagofwords
	m_semantic_extractor->setVocabulary(m_dic);

	//feature dimension
	m_words_num = m_dic.rows();
	m_clique_num = m_words_num * (m_words_num - 1);
}

void ObjectDetSemanticSuperpixel::loadSemanticModels(std::string semantic_model_file)
{
	if (semantic_model_file.empty())
		return;

	//load semantic model
	//model num
	EagleeyeIO semantic_model_i;
	semantic_model_i.createReadHandle((m_model_folder + semantic_model_file).c_str(),READ_BINARY_MODE);
	int model_num;
	semantic_model_i.read(model_num);
	m_semantic_infer.resize(model_num);
	//model data
	for (int model_index = 0; model_index < model_num; ++model_index)
	{
		Matrix<float> unary_coe,clique_coe;
		semantic_model_i.read(unary_coe);
		semantic_model_i.read(clique_coe);

		SemanticBagOfWords* semantic_bagofwords = new SemanticBagOfWords(m_words_num,3);
		semantic_bagofwords->setModelParam(unary_coe,clique_coe);

		m_semantic_infer[model_index] = semantic_bagofwords;
	}
}

void ObjectDetSemanticSuperpixel::setSymbolTrainer(DictionaryTrainer* dic_trainer, SemanticBagOfWordsTrainer* semantic_bagofwords_trainer, ExtractFeatureTrainer* ef_trainer)
{
	m_dic_trainer = dic_trainer;
	m_ef_trainer = ef_trainer;
	m_semantic_bow_trainer = semantic_bagofwords_trainer;
}

}