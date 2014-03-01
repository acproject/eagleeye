#include "ObjectDetTerminalSymbol.h"
#include "ModelTrainerIO.h"
#include "Learning/ObjectDetGrammarTreeLatentSVM.h"
#include "GrammarTree.h"
#include "SignalFactory.h"
#include "ProcessNode/ImageReadNode.h"
#include "Print.h"
#include "EagleeyeStr.h"
#include "Matlab/MatlabInterface.h"
#include <map>

namespace eagleeye
{
HOG32Pyramid ObjectDetTerminalSymbol::m_feat_pyr;
Matrix<unsigned char> ObjectDetTerminalSymbol::m_detect_img;

ObjectDetTerminalSymbol::ObjectDetTerminalSymbol(const char* name)
	:ObjectDetSymbol(name,TERMINAL)
{
	m_hog_sbin = 8;
	m_hog_pyramid_interval = 2;
	m_accepted_min_ratio = 0.0f;
	m_accepted_max_ratio = 1.0f;

	m_offset = 0.0f;
}

ObjectDetTerminalSymbol::~ObjectDetTerminalSymbol()
{

}

void* ObjectDetTerminalSymbol::getSymbolScore(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat /* = Matrix<float>() */)
{
	if (search_space != PIXEL_SPACE || search_mode != INDEPENDENT_SEARCH)
		return NULL;

	//if data isn't updated, it would return score pyramid directly
	if (!m_data_update_flag)
		return &m_hog_score_pyr;

	//make convolution between filter kernel and feature data
	int levels = m_feat_pyr.levels();

	//shallowcopy only copy pyramid struct info(size, levels, scales and others)
	m_hog_score_pyr.shallowcopy(m_feat_pyr);

	switch(m_sample_state)
	{
	case EAGLEEYE_POSITIVE_SAMPLE:
		{
			for (int levels_index = 0; levels_index < m_feat_pyr.levels(); ++levels_index)
			{
				int vl = levels_index;

				//set score zero outside the bounding box
				//transform to feature space
				float s = m_hog_sbin / m_feat_pyr.scales(vl);
				int bounding_box_r_min = int(m_bounding_box[2] / s);
				int bounding_box_r_max = int(m_bounding_box[3] / s);
				int bounding_box_c_min = int(m_bounding_box[0] / s);
				int bounding_box_c_max = int(m_bounding_box[1] / s);

				Matrix<HOG32Vec> feat_img = m_feat_pyr[vl];

				//get one sub region computing convolution
				int conv_sub_r_min = EAGLEEYE_MAX((bounding_box_r_min - m_filter_win[1]),0);
				int conv_sub_r_max = EAGLEEYE_MIN((bounding_box_r_max + m_filter_win[1]),int(feat_img.rows()));
				int conv_sub_c_min = EAGLEEYE_MAX((bounding_box_c_min - m_filter_win[0]),0);
				int conv_sub_c_max = EAGLEEYE_MIN((bounding_box_c_max + m_filter_win[0]),int(feat_img.cols()));

				//compute convolution
				Matrix<float> sub_score = convWithHOG32Features(feat_img(Range(conv_sub_r_min,conv_sub_r_max),Range(conv_sub_c_min,conv_sub_c_max)),m_filter_weight);
				int sub_cols = sub_score.cols();

				int rows = feat_img.rows();
				int cols = feat_img.cols();
				for (int i = 0; i < rows; ++i)
				{
					float* hog_score_data = m_hog_score_pyr[levels_index].row(i);
					for (int j = 0; j < cols; ++j)
					{
						if ((i < bounding_box_r_min) || (i > bounding_box_r_max) ||
							(j < bounding_box_c_min) || (j > bounding_box_c_max))
						{
							hog_score_data[j] = -EAGLEEYE_FINF;
						}
						else
						{
							hog_score_data[j] = sub_score((i - bounding_box_r_min) * sub_cols + (j - bounding_box_c_min)) + m_offset;
						}

					}
				}

				//only the flag = 1 level is valid in the ScorePyramid
				m_hog_score_pyr.flags(vl) = 1;
			}
			break;
		}
	default:
		{
			//we have to compute all levels
			for (int levels_index = 0; levels_index < levels; ++levels_index)
			{
				Matrix<HOG32Vec> feat_img = m_feat_pyr[levels_index];

				//compute convolution
				Matrix<float> score = convWithHOG32Features(feat_img,m_filter_weight);
				int rows = score.rows();
				int cols = score.cols();
				for (int i = 0; i < rows; ++i)
				{
					float* hog_score_data = m_hog_score_pyr[levels_index].row(i);
					float* score_data = score.row(i);
					for (int j = 0; j < cols; ++j)
					{
						hog_score_data[j] = score_data[j] + m_offset;
					}
				}

				m_hog_score_pyr.flags(levels_index) = 1;
			}

			break;
		}
	}
	
	//disable data update flag
	m_data_update_flag = false;
	return &m_hog_score_pyr;
}

Matrix<float> ObjectDetTerminalSymbol::getUnitFeature(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat /* = Matrix<float>() */)
{
	if (search_space != PIXEL_SPACE || search_mode != OPTIMUM_SEARCH)
		return Matrix<float>();

	//In general, pos_mat is in image space, we should transform it to some proper space,such as feature space
	//here, we don't use it

	//ignore pos_mat argument

	//we use latent symbol position info
	Matrix<HOG32Vec> feat = m_feat_pyr[m_symbol_info.level];

	int feat_end_row = EAGLEEYE_MIN(int(feat.rows()), m_symbol_info.y + m_filter_win[1]);
	int feat_end_col = EAGLEEYE_MIN(int(feat.cols()), m_symbol_info.x + m_filter_win[0]);

	Matrix<float> object_feat(1,PIXEL_FEATURE_HEAD_SIZE + m_filter_win[0] * m_filter_win[1] * 32 + 1,0.0f);
	for (int i = m_symbol_info.y; i < feat_end_row; ++i)
	{
		float* object_feat_data = object_feat.dataptr() + PIXEL_FEATURE_DATA_OFFSET + (i - m_symbol_info.y) * (m_filter_win[0] * 32);
		memcpy(object_feat_data, feat.row(i) + m_symbol_info.x, sizeof(float) * (feat_end_col - m_symbol_info.x) * 32);
	}
	
	object_feat.at(PIXEL_FEATURE_HEAD_SIZE + m_filter_win[0] * m_filter_win[1] * 32) = 1.0f;

	object_feat[0] = float(m_symbol_info.x);//position
	object_feat[1] = float(m_symbol_info.y);//position
	object_feat[2] = float(m_symbol_info.level);	//pyramid level
	object_feat[3] = float(m_symbol_info.scale);	//pyramid scale
	if (m_sample_state == EAGLEEYE_POSITIVE_SAMPLE)
		object_feat[4] = 1.0f;	//label
	else
		object_feat[4] = 0.0f;	//label
	return object_feat;
}

Matrix<float> ObjectDetTerminalSymbol::getUnitWeight()
{
	Matrix<float> symbol_weight(1,m_filter_win[0] * m_filter_win[1] * 32 + 1,float(0.0f));
	memcpy(symbol_weight.dataptr(),m_filter_weight.dataptr(),sizeof(float) * m_filter_win[0] * m_filter_win[1] * 32);
	symbol_weight.at(m_filter_win[0] * m_filter_win[1] * 32) = m_offset;

	return symbol_weight;
}

void ObjectDetTerminalSymbol::setUnitWeight(const Matrix<float>& weight)
{
	memcpy(m_filter_weight.dataptr(),weight.dataptr(),sizeof(float) * m_filter_win[0] * m_filter_win[1] * 32);
	m_offset = weight.at(m_filter_win[0] * m_filter_win[1] * 32);
}

void ObjectDetTerminalSymbol::getSymbolOutput(std::vector<void*>& output_info)
{
	//transform to image space
	//if the current symbol is Active,
	//we should get output info from this Symbol
	if (getUnitState() == GRAMMAR_UNIT_ACTIVE)
	{
		//we need to transform to the original image coordinate
		float used_scale = float(m_hog_sbin) / m_feat_pyr.scales(m_symbol_info.level);
		float x1 = m_symbol_info.x * used_scale;
		float y1 = m_symbol_info.y * used_scale;
		float x2 = x1 + m_filter_win[0] * used_scale - 1;
		float y2 = y1 + m_filter_win[1] * used_scale - 1;

		m_predict_object_region[0] = int(x1);
		m_predict_object_region[1] = EAGLEEYE_MIN(int(x2),int(m_detect_img.cols()));
		m_predict_object_region[2] = int(y1);
		m_predict_object_region[3] = EAGLEEYE_MIN(int(y2),int(m_detect_img.rows()));

		output_info.push_back(&m_predict_object_region);
	}
}

void ObjectDetTerminalSymbol::findModelLatentInfo(void* info)
{	
	DetSymbolInfo det_info = *((DetSymbolInfo*)info);

	//symbol position
	m_symbol_info = det_info;
}

void ObjectDetTerminalSymbol::getUnitPara(MemoryBlock& param_block)
{
	param_block = MemoryBlock(sizeof(_Parameter));
	_Parameter* me_param_block = (_Parameter*)param_block.block();

	me_param_block->filter_width = m_filter_win[0];
	me_param_block->filter_height = m_filter_win[1];
	me_param_block->hog_sbin = m_hog_sbin;

	me_param_block->anchor_x = m_anchor_c;
	me_param_block->anchor_y = m_anchor_r;
	me_param_block->anchor_l = m_anchor_level;
}
void ObjectDetTerminalSymbol::setUnitPara(MemoryBlock param_block)
{
	_Parameter* me_param_block = (_Parameter*)param_block.block();

	setFilterWin(me_param_block->filter_width,me_param_block->filter_height);
	m_hog_sbin = me_param_block->hog_sbin;
	m_anchor_c = me_param_block->anchor_x;
	m_anchor_r = me_param_block->anchor_y;
	m_anchor_level = me_param_block->anchor_l;
}

//////////////////////////////////////////////////////////////////////////
//all functions needed to be called explicitly
void ObjectDetTerminalSymbol::setFilterWin(int width,int height)
{
	m_filter_win[0] = width;
	m_filter_win[1] = height;

	m_filter_weight = Matrix<HOG32Vec>(m_filter_win[1],m_filter_win[0]);
}

void ObjectDetTerminalSymbol::setImageWin(int width,int height)
{
	if (m_hog_sbin > 0)
	{
		//transform to feature space
		m_filter_win[0] = int(float(width) / float(m_hog_sbin));
		m_filter_win[1] = int(float(height) / float(m_hog_sbin));

		m_filter_weight = Matrix<HOG32Vec>(m_filter_win[1],m_filter_win[0]);
	}
}

void ObjectDetTerminalSymbol::setAcceptedHeightWidthRatio(float min_ratio,float max_ratio)
{
	m_accepted_min_ratio = min_ratio;
	m_accepted_max_ratio = max_ratio;
}

void ObjectDetTerminalSymbol::setHOG32PyramidPara(int sbin,int interval)
{
	m_hog_sbin = sbin;
	m_hog_pyramid_interval = interval;
}

void ObjectDetTerminalSymbol::setUnitData(void* data,SampleState sample_state,void* auxiliary_data)
{
	if (m_detect_img.dataptr() != ((Matrix<unsigned char>*)data)->dataptr())
	{
		m_detect_img = *((Matrix<unsigned char>*)data);

		//re-generate HOG pyramid again
		m_feat_pyr = generateHOG32Pyramid(m_detect_img.transform<float>(), m_hog_pyramid_interval, m_hog_sbin);
	}

	m_sample_state = sample_state;

	//auxiliary_data is defined in image space
	//we have to transform it to feature space
	if (auxiliary_data)
	{
		m_bounding_box = *((Array<int,4>*)auxiliary_data);
	}
}

void ObjectDetTerminalSymbol::learnUnit(const char* parse_file)
{
	//write code here directly
	if (!EagleeyeIO::isexist(m_model_folder + m_unit_name + "_weight_ini.dat"))
	{
		if (!EagleeyeIO::isexist(m_model_folder + m_unit_name + "_training_samples_for_ini.dat") &&
			!EagleeyeIO::isexist(m_model_folder + m_unit_name + "_latents_for_ini.dat"))
		{
			std::string parse_file_path = m_model_folder + parse_file;
			std::map<std::string,std::vector<Array<int,4>>> p_samples;
			std::map<std::string,std::vector<Array<int,4>>> n_samples;
			analyzeSimpleParseFile(parse_file_path.c_str(),p_samples,n_samples);

			ImageReadNode<ImageSignal<unsigned char>> img_read_node;
			EagleeyeIO training_samples_o;
			training_samples_o.createWriteHandle(m_model_folder + m_unit_name + "_training_samples_for_ini.dat",true,WRITE_BINARY_MODE);

			int training_samples_num = 0;
			int training_samples_dim = 0;

			//parse all positive samples and write to file sequentially
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

						int height = object_region[3] - object_region[2];
						int width = object_region[1] - object_region[0];

						//finding whether we should use this sample	
						if (float(height) / float(width) >= m_accepted_min_ratio && 
							float(height) / float(width) <= m_accepted_max_ratio)
						{
							Matrix<unsigned char> local_img = sample_img(Range(object_region[2],object_region[3]),Range(object_region[0],object_region[1]));
							Matrix<unsigned char> resized_local_img = resize(local_img,(m_filter_win[1] + 2) * m_hog_sbin,(m_filter_win[0] + 2) * m_hog_sbin);
							Matrix<HOG32Vec> hog_img = generateHOG32Features(resized_local_img.transform<float>(),m_hog_sbin);

							Matrix<float> f_hog_img(1,hog_img.rows() * hog_img.cols() * 32,hog_img.dataptr(),false);
							Matrix<float> sample(1,f_hog_img.cols() + 1 + 1,0.0f);
							sample(0,0) = 1.0f;
							sample(Range(0,1),Range(1,hog_img.rows() * hog_img.cols() * 32 + 1)).copy(f_hog_img);
							sample(0,sample.cols() - 1) = 1.0f; //corresponding to offset
							training_samples_o.write(sample);

							training_samples_num++;
							training_samples_dim = f_hog_img.cols() + 1 + 1;
						}
					}
				}
			}

			//parse all negative samples and write to file sequentially
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

					Variable<float> uniform_random = Variable<float>::uniform(0.0f,1.0f);
					int height = sample_img.rows();
					int width = sample_img.cols();
					int pick_randomly_num = 30;

					for (int i = 0 ; i < pick_randomly_num; ++i)
					{
						int pick_x = int(EAGLEEYE_MIN(uniform_random.var() * float(width),float(width - 1)));
						int pick_y = int(EAGLEEYE_MIN(uniform_random.var() * float(height),float(height - 1)));

						int pick_width = int(uniform_random.var() * float(width - 1 - pick_x));
						int pick_height = int(uniform_random.var() * float(height - 1 - pick_y));

						Matrix<unsigned char> pick_img = sample_img(Range(pick_y,pick_y + pick_height),Range(pick_x,pick_x + pick_width));
						Matrix<unsigned char> resized_pick_img = resize(pick_img,(m_filter_win[1] + 2) * m_hog_sbin,(m_filter_win[0] + 2) * m_hog_sbin);
						Matrix<HOG32Vec> hog_img = generateHOG32Features(resized_pick_img.transform<float>(),m_hog_sbin);

						Matrix<float> f_hog_img(1,hog_img.rows() * hog_img.cols() * 32,hog_img.dataptr(),false);
						Matrix<float> sample(1,f_hog_img.cols() + 1 + 1,0.0f);
						sample(0,0) = -1.0f;
						sample(Range(0,1),Range(1,hog_img.rows() * hog_img.cols() * 32 + 1)).copy(f_hog_img);
						sample(0,sample.cols() - 1) = 1.0f; //corresponding to offset
						training_samples_o.write(sample);

						training_samples_num++;
					}
				}
			}

			training_samples_o.destroyHandle();

			//
			DynamicArray<int> key_parameters(2);
			key_parameters[0] = training_samples_num;
			key_parameters[1] = training_samples_dim;
			EagleeyeIO::write(key_parameters,m_model_folder + m_unit_name + "_training_samples_para_for_ini.dat",WRITE_ASCII_MODE);
		}

		//learning weight
		//reading data from file
		DynamicArray<int> training_samples_key_parameters(2);
		EagleeyeIO::read(training_samples_key_parameters,m_model_folder + m_unit_name + "_training_samples_para_for_ini.dat",READ_ASCII_MODE);
		int training_samples_num = training_samples_key_parameters[0];
		int training_samples_dim = training_samples_key_parameters[1];

		EagleeyeIO training_samples_i;
		training_samples_i.createReadHandle(m_model_folder + m_unit_name + "_training_samples_for_ini.dat",READ_BINARY_MODE);
		Matrix<float> training_samples(training_samples_num,training_samples_dim);
		for (int i = 0; i < training_samples_num; ++i)
		{
			Matrix<float> sample;
			training_samples_i.read(sample);
			training_samples(Range(i,i + 1),Range(0,training_samples_dim)).copy(sample);
		}
		training_samples_i.destroyHandle();

		learnUnit(training_samples);
	}

	Matrix<float> symbol_weight;
	EagleeyeIO::read(symbol_weight,m_model_folder + m_unit_name + "_weight_ini.dat",READ_BINARY_MODE);

	//set unit weight
	setUnitWeight(symbol_weight);
}

void ObjectDetTerminalSymbol::learnUnit(const Matrix<float>& samples)
{
	int samples_num = samples.rows();
	int feature_dim = samples.cols() - 1;
	Matrix<int> latent_variables(samples_num,1);
	for (int i = 0; i < samples_num; ++i)
	{
		latent_variables(i) = i;
	}

	//get default model coefficient from grammar tree model
	GrammarTreeStructureInfo gt_info = wraptoDummyGrammarTree(this);
	ObjectDetGrammarTreeLatentSVM gt_latent_svm(gt_info);
	gt_latent_svm.setTrainingSamples(samples(Range(0,samples_num),Range(0,1)).transform<int>(),latent_variables,samples(Range(0,samples_num),Range(1,feature_dim + 1)));
	gt_latent_svm.learn();

	Matrix<float> optimum_weight = gt_latent_svm.getOptimumWeight();
	
	EagleeyeIO::write(optimum_weight,m_model_folder + m_unit_name + "_weight_ini.dat",WRITE_BINARY_MODE);
}

}
