#include "SemanticBOWDescriptorExtractor.h"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
SemanticBOWDescriptorExtractor::SemanticBOWDescriptorExtractor(DescriptorExtractor* des_extract /* = NULL */)
{
	m_des_matcher = cv::DescriptorMatcher::create("BruteForce");
	m_des_extractor = des_extract;

	m_descriptor_size = 0;
	m_min_statistic_num = 200;
	m_words_num = 0;
	m_wordpair_dis_dim = 0;
	m_wordpair_angle_dim = 0;

	m_max_dis = 100.0f;
}
SemanticBOWDescriptorExtractor::~SemanticBOWDescriptorExtractor()
{

}

void SemanticBOWDescriptorExtractor::setDescriptorExtractor(DescriptorExtractor* des_extract)
{
	m_des_extractor = des_extract;
}

void SemanticBOWDescriptorExtractor::compute(const Matrix<float>& img, std::vector<KeyPoint>& keypoints, Matrix<float>& img_descriptors,
			 std::vector<std::vector<int> >* point_idxs_of_clusters)
{
	//compute descriptors for the image
	Matrix<float> descriptors;
	m_des_extractor->compute(img,keypoints,descriptors);

	//finding word of every keypoint
	int keypoints_num = keypoints.size();
	std::vector<int> kp_word(keypoints_num);
	for (int kp_index = 0; kp_index < keypoints_num; ++kp_index)
	{
		cv::Mat cv_descriptor(1, descriptors.cols(), CV_32F, descriptors.row(kp_index));
		std::vector<cv::DMatch> cv_matches;
		m_des_matcher->match(cv_descriptor,cv_matches);

		int train_idx = cv_matches[0].trainIdx; // cluster index(word index)

		kp_word[kp_index] = train_idx;	//record word of every keypoint
	}

	//finding semantic info
	//initialize some matrix
	m_wordspair_dis.resize(1);
	m_wordspair_dis[0] = Matrix<float>(m_words_num,m_words_num,0.0f);
	m_wordspair_angle.resize(1);
	m_wordspair_angle[0] = Matrix<float>(m_words_num,m_words_num,0.0f);

	std::vector<bool> flag_vec(m_words_num,false);
	for (int kp_index = 0; kp_index < keypoints_num; ++kp_index)
	{
		int left_word = kp_word[kp_index];
		if (flag_vec[left_word])
			continue;

		//finding semantic info about left word
		for (int inner_index = 0; inner_index < keypoints_num; ++inner_index)
		{
			int right_word = kp_word[inner_index];
			if (right_word == left_word)
				continue;

			//finding distance and angle between two words
			float x_dis = keypoints[kp_index].pt[0] - keypoints[inner_index].pt[0];
			float y_dis = keypoints[kp_index].pt[1] - keypoints[inner_index].pt[1];
			float dis_val = sqrt(x_dis * x_dis + y_dis * y_dis);
			float angle_val = cos((keypoints[kp_index].pt[0] - keypoints[inner_index].pt[0]) / dis_val);

			//only record distance and angle between two neighbor words
			float norm_dis = dis_val / m_max_dis;
			if (m_wordspair_dis[0](left_word,right_word) > norm_dis)
			{
				m_wordspair_dis[0](left_word,right_word) = norm_dis;
				m_wordspair_angle[0](left_word,right_word) = angle_val;

				//symmetry
				m_wordspair_dis[0](right_word,left_word) = norm_dis;
				m_wordspair_angle[0](right_word,left_word) = angle_val;
			}
		}

		flag_vec[kp_word[kp_index]] = true;
	}
	
	//initialize matrix
	img_descriptors = Matrix<float>(1,m_descriptor_size,0.0f);
	
	//record word frequency
	for (int kp_index = 0; kp_index < keypoints_num; ++kp_index)
	{
		img_descriptors(kp_word[kp_index]) += 1.0f;
	}
	for (int index = 0; index < m_words_num; ++index)
	{
		img_descriptors(index) /= float(keypoints_num);
	}

	//record word pair distance
	int offset = m_words_num;
	int pair_count = 0;
	for (int left_word = 0; left_word < m_words_num; ++left_word)
	{
		for (int right_word = 0; right_word < m_words_num; ++right_word)
		{
			if (left_word != right_word)
			{
				img_descriptors(offset + pair_count) = m_wordspair_dis[0](left_word,right_word);
				pair_count++;
			}
		}
	}

	//record word pair angle
	pair_count = 0;
	offset = m_words_num + m_wordpair_dis_dim;
	for (int left_word = 0; left_word < m_words_num; ++left_word)
	{
		for (int right_word = 0; right_word < m_words_num; ++right_word)
		{
			if (left_word != right_word)
			{
				img_descriptors(offset + pair_count) = m_wordspair_angle[0](left_word,right_word);
				pair_count++;
			}
		}
	}
}

void SemanticBOWDescriptorExtractor::compute(const Matrix<float>& superpixel_img, const Matrix<int>& superpixel_index_img, 
											 int superpixel_num, std::vector<KeyPoint>& keypoints, 
											 Matrix<float>& superpixel_descriptors,std::vector<bool>& superpixel_flag)
{	
	//compute descriptors for the image
	Matrix<float> descriptors;
	m_des_extractor->compute(superpixel_img,keypoints,descriptors);

	int keypoints_num = keypoints.size();
	//split all keypoints to superpixels
	std::vector<std::vector<KeyPoint>> s_keypoints(superpixel_num);
	std::vector<std::vector<int>> s_kp_word(superpixel_num);
	for (int kp_index = 0; kp_index < keypoints_num; ++kp_index)
	{
		int x = (int)keypoints[kp_index].pt[0];
		int y = (int)keypoints[kp_index].pt[1];
		int s_index = superpixel_index_img(y,x); //superpixel index
		s_keypoints[s_index].push_back(keypoints[kp_index]);

		cv::Mat cv_descriptor(1, descriptors.cols(), CV_32F, descriptors.row(kp_index));
		std::vector<cv::DMatch> cv_matches;
		m_des_matcher->match(cv_descriptor,cv_matches);

		int train_idx = cv_matches[0].trainIdx; // cluster index(word index)
		s_kp_word[s_index].push_back(train_idx);
	}

	//superpixel flag true or false
	superpixel_flag.resize(superpixel_num,false);

	//fill superpixel flag
	for (int s_index = 0; s_index < superpixel_num; ++s_index)
	{
		if (s_keypoints[s_index].size() < m_min_statistic_num)
			superpixel_flag[s_index] = false;
		else
			superpixel_flag[s_index] = true;
	}

	//initialize matrix
	m_wordspair_dis.clear(); m_wordspair_angle.clear();
	m_wordspair_dis.resize(superpixel_num); m_wordspair_angle.resize(superpixel_num);
	//finding semantic info
	for (int s_index = 0; s_index < superpixel_num; ++s_index)
	{
		//skip invalid superpixel
		if (superpixel_flag[s_index] == false)
			continue;

		//assign zeros
		m_wordspair_dis[s_index] = Matrix<float>(m_words_num,m_words_num,float(EAGLEEYE_FINF));
		m_wordspair_angle[s_index] = Matrix<float>(m_words_num,m_words_num,0.0f);

		int s_keypoints_num = s_keypoints[s_index].size();//keypoints number in 's_index' superpixel 

		//finding semantic info of every superpixel
		std::vector<bool> flag_vec(m_words_num,false);
		for (int kp_index = 0; kp_index < s_keypoints_num; ++kp_index)
		{
			int left_word = s_kp_word[s_index][kp_index];
			if (flag_vec[left_word])
				continue;

			//finding semantic info about left word
			for (int inner_index = 0; inner_index < s_keypoints_num; ++inner_index)
			{
				int right_word = s_kp_word[s_index][inner_index];
				if (right_word == left_word)
					continue;

				//finding distance and angle between two words
				float x_dis = s_keypoints[s_index][kp_index].pt[0] - s_keypoints[s_index][inner_index].pt[0];
				float y_dis = s_keypoints[s_index][kp_index].pt[1] - s_keypoints[s_index][inner_index].pt[1];
				float dis_val = sqrt(x_dis * x_dis + y_dis * y_dis);
				float angle_val = cos((s_keypoints[s_index][kp_index].pt[0] - s_keypoints[s_index][inner_index].pt[0]) / dis_val);

				//only record distance and angle between two neighbor words
				float norm_dis = dis_val / m_max_dis;
				if (m_wordspair_dis[s_index](left_word,right_word) > norm_dis)
				{
					m_wordspair_dis[s_index](left_word,right_word) = norm_dis;
					m_wordspair_angle[s_index](left_word,right_word) = angle_val;

					//symmetry
					m_wordspair_dis[s_index](right_word,left_word) = norm_dis;
					m_wordspair_angle[s_index](right_word,left_word) = angle_val;
				}
			}

			flag_vec[s_kp_word[s_index][kp_index]] = true;
		}

		//assign zeros for invalid position in m_wordspair_dis
		m_wordspair_dis[s_index].setval(0.0f,GreaterThan<float>(float(EAGLEEYE_NEAR_INF)));
	}
		
	//define superpixel descriptors matrix
	superpixel_descriptors = Matrix<float>(superpixel_num,m_descriptor_size,0.0f);
	//record word frequency
	for (int s_index = 0; s_index < superpixel_num; ++s_index)
	{
		//skip invalid superpixel
		if (superpixel_flag[s_index] == false)
			continue;

		int kp_num = s_keypoints[s_index].size();
		for (int kp_index = 0; kp_index < kp_num; ++kp_index)
		{
			superpixel_descriptors(s_index,s_kp_word[s_index][kp_index]) += 1.0f;
		}
	}
	for (int s_index = 0; s_index < superpixel_num; ++s_index)
	{
		//skip invalid superpixel
		if (superpixel_flag[s_index] == false)
			continue;

		int kp_num = s_keypoints[s_index].size();
		for (int w_index = 0; w_index < m_words_num; ++w_index)
		{
			superpixel_descriptors(s_index,w_index) /= float(kp_num);
		}
	}
	
	//record word pair distance
	int offset = m_words_num;
	for (int s_index = 0; s_index < superpixel_num; ++s_index)
	{
		//skip invalid superpixel
		if (superpixel_flag[s_index] == false)
			continue;

		int pair_count = 0;
		for (int left_word = 0; left_word < m_words_num; ++left_word)
		{
			for (int right_word = 0; right_word < m_words_num; ++right_word)
			{
				if (left_word != right_word)
				{
					superpixel_descriptors(s_index,offset + pair_count) = m_wordspair_dis[s_index](left_word,right_word);
					pair_count++;
				}
			}
		}
	}

	//record word pair angle
	offset = m_words_num + m_wordpair_dis_dim;
	for (int s_index = 0; s_index < superpixel_num; ++s_index)
	{
		//skip invalid superpixel
		if (superpixel_flag[s_index] == false)
			continue;

		int pair_count = 0;
		for (int left_word = 0; left_word < m_words_num; ++left_word)
		{
			for (int right_word = 0; right_word < m_words_num; ++right_word)
			{
				if (left_word != right_word)
				{
					superpixel_descriptors(s_index,offset + pair_count) = m_wordspair_angle[s_index](left_word,right_word);
					pair_count++;
				}
			}
		}
	}
}

void SemanticBOWDescriptorExtractor::setVocabulary(const Matrix<float>& vocabulary)
{
	int rows = vocabulary.rows();
	int cols = vocabulary.cols();

	m_words_num = rows;
	m_wordpair_dis_dim = m_words_num * (m_words_num - 1);
	m_wordpair_angle_dim = m_words_num * (m_words_num - 1);

	cv::Mat cv_vocabulary(rows,cols,CV_32F);
	memcpy(cv_vocabulary.data,vocabulary.dataptr(),sizeof(float) * rows * cols);

	m_des_matcher->clear();
	m_des_matcher->add(std::vector<cv::Mat>(1,cv_vocabulary));

	//frequency + distance between a pair words + angle between a pair words
	//pair order: word12 word13 ... word 1n word 21 word23 ...
	m_descriptor_size = m_words_num + m_words_num * (m_words_num - 1) + m_words_num * (m_words_num - 1);
}
Matrix<float> SemanticBOWDescriptorExtractor::getVocabulary()
{
	std::vector<cv::Mat> vocabulary = m_des_matcher->getTrainDescriptors();
	cv::Mat vocabulary_mat = vocabulary[0];
	return Matrix<float>::mapfrom(vocabulary_mat.rows,vocabulary_mat.cols,vocabulary_mat.data);
}

int SemanticBOWDescriptorExtractor::descriptorSize()
{
	return m_descriptor_size;
}

void SemanticBOWDescriptorExtractor::setMinimumStatisticNum(int num)
{
	m_min_statistic_num = num;
}

}