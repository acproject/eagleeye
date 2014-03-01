#ifndef _FEATUREDETECTOR_H_
#define _FEATUREDETECTOR_H_

#include "EagleeyeMacro.h"
#include "Algorithm.h"
#include "Matrix.h"
#include "Types.h"
#include <vector>
#include "EagleeyeCore.h"

namespace eagleeye 
{
class EAGLEEYE_API FeatureDetector:public Algorithm
{
public:
	FeatureDetector();
	virtual ~FeatureDetector();

	/**
	 *	@brief detect keypoints on image
	 */
	virtual void detect(const Matrix<float>& img,std::vector<KeyPoint>& keypoints){};

	/**
	 *	@brief set some common parameters
	 */
	void setCommonParams(int n_octaves, int n_octavelayers);

	/**
	 *	@brief set exclude region image
	 */
	void setExcludeRegion(Matrix<unsigned char> exclude_region);

	void enableCalcMainDir();
	void disableCalcMainDir();

protected:
	int m_n_octaves;
	int m_n_octavelayers;
	bool m_calc_main_dir_flag;
	Matrix<unsigned char> m_exclude_region;		/**< 0 - invalid region; 1 - valid region*/
};
}


#endif