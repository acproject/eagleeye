#include "ObjectDetGrammarTree.h"
#include "Trainer/ObjectDetGrammarTreeTrainer.h"
#include "SignalFactory.h"
#include "MatrixMath.h"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
ObjectDetGrammarTree::ObjectDetGrammarTree(const char* name,const char* model_folder,ObjectDetNonterminalSymbol* root)
	:GrammarTree(name,model_folder,root)
{
	m_negative_samples = 100;
	m_score_threshold = -20.0f;
	m_min_constrained_area = 20;

	m_structure_filter_flag = false;	//whether enable structure filtering
	m_constrained_area_flag = false;	//whether enable area constraining
	m_constrained_rec_flag = false;		//whether enable width/height ratio constraining

	m_constrained_wh_ratio[0] = 0.0f;
	m_constrained_wh_ratio[1] = 1.0f;

	//////////////////////////////////////////////////////////////////////////
	//build monitor var
	EAGLEEYE_MONITOR_VAR(float,setScoreThreshold,getScoreThreshold,"score_threshold");
}

ObjectDetGrammarTree::~ObjectDetGrammarTree()
{

}

void ObjectDetGrammarTree::setRootSymbol(Symbol* root)
{
	GrammarTree::setRootSymbol(root);
	m_det_root_symbol = dynamic_cast<ObjectDetNonterminalSymbol*>(root);
}

bool greater_second(const DetSymbolInfo& m1, const DetSymbolInfo& m2) 
{
	return m1.val > m2.val;
}

void ObjectDetGrammarTree::parseData(void* data,int width,int height,void* auxiliary_data)
{
	m_detect_img_width = width;
	m_detect_img_height = height;

	//clear output regions
	m_output_regions.clear();

	//set data for the whole grammar tree
	//this data would be sent to all symbols of this grammar tree
	m_gt_root->setModelData(data,EAGLEEYE_UNDEFINED_SAMPLE,NULL);

	std::vector<DetSymbolInfo> latent_info;

	//get max value
	m_score_pyr = *((ScorePyramid*)m_det_root_symbol->getSymbolScore(PIXEL_SPACE,INDEPENDENT_SEARCH));

	ScorePyramid structure_score_pyr;
	structure_score_pyr.deepcopy(m_score_pyr);

	//structure filter
	if (m_structure_filter_flag)
	{
		//structure kernel
		Matrix<float> struct_kernel(3,3,0.0f);
		struct_kernel(Range(0,1),Range(0,1)) = 1.0f / 5.0f;
		struct_kernel(Range(0,1),Range(2,3)) = 1.0f / 5.0f;
		struct_kernel(Range(1,2),Range(1,2)) = 1.0f / 5.0f;
		struct_kernel(Range(2,3),Range(0,1)) = 1.0f / 5.0f;
		struct_kernel(Range(2,3),Range(2,3)) = 1.0f / 5.0f;

		for (int level_index = m_score_pyr.interval; level_index < m_score_pyr.levels(); ++level_index)
		{
			Matrix<float> score_mat = m_score_pyr[level_index];
			structure_score_pyr[level_index] = conv2DInSpace(score_mat,struct_kernel);
		}
	}

	//make some constrained operation
	//constrained area
	if (m_constrained_area_flag)
	{
		for (int level_index = m_score_pyr.interval; level_index < m_score_pyr.levels(); ++level_index)
		{
			Matrix<float> structure_mat = structure_score_pyr[level_index];
			int rows = structure_mat.rows();
			int cols = structure_mat.cols();

			Matrix<unsigned char> binary_mat(rows,cols,unsigned char(0));
			for ( int i = 0; i < rows; ++i )
			{
				float* structure_mat_data = structure_mat.row(i);
				unsigned char* binary_mat_data = binary_mat.row(i);
				for (int j = 0; j < cols; ++j)
				{
					if (structure_mat_data[j] > m_score_threshold)
					{
						binary_mat_data[j] = unsigned char(1);
					}
					else
					{
						binary_mat_data[j] = unsigned char(0);
					}
				}
			}

			ImageSignal<unsigned char>* shape_constrained_input_sig = new ImageSignal<unsigned char>;
			shape_constrained_input_sig->img = binary_mat;

			ShapeConstrainedNode<ImageSignal<unsigned char>> sc_node;
			sc_node.setConstrainedArea(float(m_min_constrained_area),10000000.0f);
			sc_node.setConstrainedFlags(ShapeConstrainedNode<ImageSignal<unsigned char>>::area_constrained);
			sc_node.setInputPort(shape_constrained_input_sig);
			sc_node.start();

			Matrix<unsigned char> constrained_img = sc_node.getOutputImage();
			for ( int i = 0; i < rows; ++i )
			{
				float* structure_mat_data = structure_mat.row(i);
				unsigned char* constrained_img_data = constrained_img.row(i);
				for (int j = 0; j < cols; ++j)
				{
					if (constrained_img_data[j] < unsigned char(1))
					{
						structure_mat_data[j] = m_score_threshold - 10000.0f;
					}
				}
			}

			delete shape_constrained_input_sig;
		}
	}

	//find object region
	for (int level_index = m_score_pyr.interval; level_index < m_score_pyr.levels(); ++level_index)
	{
		if (m_score_pyr.flags(level_index))
		{
			Matrix<float> score_mat = m_score_pyr[level_index];
			Matrix<float> structure_score_mat = structure_score_pyr[level_index];

			int rows = score_mat.rows();
			int cols = score_mat.cols();

			for (int i = 0; i < rows; ++i)
			{
				float* score_data = score_mat.row(i);
				float* structure_score_mat_data = structure_score_mat.row(i);

				for (int j = 0; j < cols; ++j)
				{
					if (structure_score_mat_data[j] > m_score_threshold)
					{
						DetSymbolInfo info;
						info.x = j;
						info.y = i;
						info.level = level_index;
						info.ds = 2;
						info.val = score_data[j];

						latent_info.push_back(info);
					}
				}
			}
		}
	}

	if (latent_info.size() > 0)
	{
		std::sort(latent_info.begin(),latent_info.end(),greater_second);
		for (unsigned int i = 0; i < latent_info.size(); ++i)
		{
			//find some latent info of this GrammarTree model
			m_det_root_symbol->findModelLatentInfo(&latent_info[i]);

			//get GrammarModel output
			std::vector<void*> output_info;
			m_det_root_symbol->getModelOutput(output_info);

			int num = output_info.size();
			for (int i = 0; i < num; ++i)
			{
				Array<int,4> key_region =*((Array<int,4>*)output_info[i]);
				ObjectRegion obj;
				obj.left_bottom_c = key_region[0];
				obj.right_top_c = key_region[1];
				obj.left_bottom_r = key_region[2];
				obj.right_top_r = key_region[3];

				m_output_regions.push_back(obj);
			}
		}
	}
}

void ObjectDetGrammarTree::getObjectRegions(std::vector<ObjectRegion>& regions)
{
	regions = m_output_regions;
}

ScorePyramid ObjectDetGrammarTree::getScorePyramid()
{
	return m_score_pyr;
}

Matrix<unsigned char> ObjectDetGrammarTree::getPredictMaskImage()
{
	Matrix<unsigned char> pre_mask_img( m_detect_img_height, m_detect_img_width, unsigned char( 0 ) );
	
	std::vector<ObjectRegion>::iterator iter,iend( m_output_regions.end() );
	for ( iter = m_output_regions.begin(); iter != iend; ++iter )
	{
		pre_mask_img(Range((*iter).left_bottom_r,(*iter).right_top_r),Range((*iter).left_bottom_c,(*iter).right_top_c)) = unsigned char(1);
	}

	return pre_mask_img;
}

void ObjectDetGrammarTree::enableConstrainedArea(int min_constrained_area)
{
	m_constrained_area_flag = true;
	m_min_constrained_area = min_constrained_area;
}
void ObjectDetGrammarTree::disableConstrainedArea()
{
	m_constrained_area_flag = false;
}

void ObjectDetGrammarTree::enableConstrainedRec(float min_constrained_wh_ratio,float max_constrained_wh_ratio)
{
	m_constrained_rec_flag = true;

	m_constrained_wh_ratio[0]=min_constrained_wh_ratio;
	m_constrained_wh_ratio[1]=max_constrained_wh_ratio;
}
void ObjectDetGrammarTree::disableConstrainedRec()
{
	m_constrained_rec_flag = false;
}

void ObjectDetGrammarTree::enableStructureFilter()
{
	m_structure_filter_flag = true;
}
void ObjectDetGrammarTree::disableStructureFilter()
{
	m_structure_filter_flag = false;
}

float ObjectDetGrammarTree::parseTrainingData(void* data,SampleState sample_state,
											 void* auxiliary_data,
											 Matrix<float>& gt_sample,
											 Matrix<int>& gt_latent_variables)
{
	//set data for the whole grammar tree
	//this data would be sent to all symbols of this grammar tree
	m_gt_root->setModelData(data,sample_state,auxiliary_data);

	//find some latent info of this grammar tree model
	std::vector<DetSymbolInfo> latent_info;

	//get max value
	ScorePyramid score_pyr = *((ScorePyramid*)m_det_root_symbol->getSymbolScore(PIXEL_SPACE,INDEPENDENT_SEARCH));
	int score_pyr_levels_num = score_pyr.levels();

	//find latent information
	int beyound_threshold_score_num = 0;
	for (int level_index = score_pyr.interval; level_index < score_pyr_levels_num; ++level_index)
	{
		if (score_pyr.flags(level_index))
		{
			Matrix<float> score_mat = score_pyr[level_index];
			int rows = score_mat.rows();
			int cols = score_mat.cols();
			for (int i = 0; i < rows; ++i)
			{
				float* score_data = score_mat.row(i);
				for (int j = 0; j < cols; ++j)
				{
					if (score_data[j] > m_score_threshold)
					{
						beyound_threshold_score_num++;
					}
				}
			}
		}
	}
	latent_info.resize(beyound_threshold_score_num);

	int beyound_threshold_score_count = 0;
	for (int level_index = score_pyr.interval; level_index < score_pyr_levels_num; ++level_index)
	{
		if (score_pyr.flags(level_index))
		{
			Matrix<float> score_mat = score_pyr[level_index];
			int rows = score_mat.rows();
			int cols = score_mat.cols();

			for (int i = 0; i < rows; ++i)
			{
				float* score_data = score_mat.row(i);
				for (int j = 0; j < cols; ++j)
				{
					if (score_data[j] > m_score_threshold)
					{
						DetSymbolInfo info;
						info.x = j;
						info.y = i;
						info.level = level_index;
						info.ds = 2;
						info.val = score_data[j];

						latent_info[beyound_threshold_score_count] = info;
						beyound_threshold_score_count++;
					}
				}
			}
		}
	}

	//construct feature and latent variables
	if (latent_info.size() > 0)
	{
		std::sort(latent_info.begin(),latent_info.end(),greater_second);

		switch(sample_state)
		{
		case EAGLEEYE_POSITIVE_SAMPLE:
			{
				m_det_root_symbol->findModelLatentInfo(&latent_info[0]);

				Matrix<float> gt_feature = reOrganizeGrammarTreeFeature(PIXEL_SPACE);

				//positive training sample
				//waring: m_gt_feature_size == gt_feature.cols()
				gt_sample = Matrix<float>(1,m_gt_feature_size + 1,float(0.0f));
				gt_sample(0,0) = 1.0f;
				gt_sample(Range(0,1),Range(1,m_gt_feature_size + 1)).copy(gt_feature);
				
				//latent variables of positive training sample
				gt_latent_variables = Matrix<int>(1,3,int(0));
				gt_latent_variables(0,0) = latent_info[0].x;
				gt_latent_variables(0,1) = latent_info[0].y;
				gt_latent_variables(0,2) = latent_info[0].level;
				
				break;
			}
		case EAGLEEYE_NEGATIVE_SAMPLE:
			{
				int num = latent_info.size();
				int negative_num = EAGLEEYE_MIN(num,m_negative_samples);
				
				//build matrix for negative training samples
				gt_sample = Matrix<float>(negative_num,m_gt_feature_size + 1,float(0.0f));

				//build matrix for latent variables of negative training samples
				gt_latent_variables = Matrix<int>(negative_num,3,int(0));

				for (int i = 0; i < negative_num; ++i)
				{
					//negative training samples
					gt_sample(i,0) = -1.0f;
					m_det_root_symbol->findModelLatentInfo(&latent_info[i]);
					Matrix<float> gt_feature = reOrganizeGrammarTreeFeature(PIXEL_SPACE);
					gt_sample(Range(i,i + 1),Range(1,m_gt_feature_size + 1)).copy(gt_feature);

					//latent variables of negative training samples
					gt_latent_variables(i,0) = latent_info[i].x;
					gt_latent_variables(i,1) = latent_info[i].y;
					gt_latent_variables(i,2) = latent_info[i].level;
				}					

				break;
			}
		}

		//return max score value
		return latent_info[0].val;
	}

	return -EAGLEEYE_FINF;
}

void ObjectDetGrammarTree::learn(const char* parse_file)
{
	if (m_gt_root)
	{
		//if some symbols possess their own learn plan
		//notify them to start learning
		m_gt_root->learn(parse_file);

		//using grammar tree learner to learn this whole tree
		ObjectDetGrammarTreeTrainer object_det_grammar_tree_trainer(this);
		std::string tree_id_name = m_gt_info.units_name[0];
		int units_num = m_gt_info.units_name.size();
		for (int i = 1; i < units_num; ++i)
		{
			tree_id_name = tree_id_name + "_" + m_gt_info.units_name[i];
		}
		object_det_grammar_tree_trainer.setUnitName(tree_id_name.c_str());
		InfoSignal<std::string> parse_file_info;
		std::string parse_file_str = parse_file;
		parse_file_info.info = & parse_file_str;
		object_det_grammar_tree_trainer.setInputPort(&parse_file_info);
		object_det_grammar_tree_trainer.setTrainerFolder(GrammarUnit::getModelFolder());
		object_det_grammar_tree_trainer.startTrain();
	}
}

void ObjectDetGrammarTree::setScoreThreshold(float threshold)
{
	m_score_threshold = threshold;
}
void ObjectDetGrammarTree::getScoreThreshold(float& threshold)
{
	threshold = m_score_threshold;
}

void ObjectDetGrammarTree::initialize()
{
	//load grammar tree structure
	loadGrammarTreeStructure();

	//load grammar tree coefficients
	loadGrammarTreeWeight();

	//analyze grammar tree structure
	analyzeGrammarTreeStructureInfo();

	//load additional parameters
	loadUnitConfig();

	//initialize all units
	m_gt_root->initialize();
}
}
