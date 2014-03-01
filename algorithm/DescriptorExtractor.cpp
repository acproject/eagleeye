#include "DescriptorExtractor.h"
#include "DenseFeatureDetector.h"

namespace eagleeye
{
DescriptorExtractor::DescriptorExtractor()
{

}
DescriptorExtractor::~DescriptorExtractor()
{

}

void DescriptorExtractor::compute( const Matrix<float>& img,std::vector<KeyPoint>& keypoints, Matrix<float>& img_descriptors )
{
	computeImpl(img,keypoints,img_descriptors);
}

void DescriptorExtractor::compute( const Matrix<float>& superpixel_img, const Matrix<int>& superpixel_index_img, int superpixel_num,
								  std::vector<KeyPoint>& keypoints, Matrix<float>& superpixel_descriptors,std::vector<bool>&  superpixel_flag)
{
	int rows = superpixel_img.rows();
	int cols = superpixel_img.cols();

	if (keypoints.size() == 0 )
	{
		//make all pixels as keypoints
		DenseFeatureDetector dense_feature_det;
		dense_feature_det.setDetectorParams(1.0f,1,1.0f,4);
		dense_feature_det.detect(rows,cols,keypoints);
	}

	superpixel_flag.resize(superpixel_num,false);

	Matrix<float> keypoints_descriptors;
	computeImpl(superpixel_img,keypoints,keypoints_descriptors);

	int descriptors_size = descriptorSize();

	//transform to superpixel descriptors
	int keypoints_num = keypoints.size();
	superpixel_descriptors = Matrix<float>(superpixel_num,descriptors_size,0.0f);

	std::vector<int> counts;
	counts.resize(superpixel_num,0);

	for (int keypoint_index = 0; keypoint_index < keypoints_num; ++keypoint_index)
	{
		int keypoint_c_index = int(keypoints[keypoint_index].pt[0]);
		int keypoint_r_index = int(keypoints[keypoint_index].pt[1]);

		int superpixel_index = superpixel_index_img.at(keypoint_r_index,keypoint_c_index);
		superpixel_flag[superpixel_index] = true;
		counts[superpixel_index]++;

		float* superpixel_descriptors_data = superpixel_descriptors.row(superpixel_index);
		float* keypoints_descriptors_data = keypoints_descriptors.row(keypoint_index);

		for (int i = 0; i < descriptors_size; ++i)
		{
			superpixel_descriptors_data[i] += keypoints_descriptors_data[i];
		}
	}
	
	//normalize superpixel descriptors
	for (int superpixel_index = 0; superpixel_index < superpixel_num; ++superpixel_index)
	{
		if (counts[superpixel_index] != 0)
		{
			float* superpixel_descriptors_data = superpixel_descriptors.row(superpixel_index);
			for (int i = 0; i < descriptors_size; ++i)
			{
				superpixel_descriptors_data[i] = superpixel_descriptors_data[i] / float(counts[superpixel_index]);
			}
		}
	}
}
}