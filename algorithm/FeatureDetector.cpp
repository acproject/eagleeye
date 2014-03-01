#include "FeatureDetector.h"

namespace eagleeye
{
FeatureDetector::FeatureDetector()
{
	m_n_octaves=4;
	m_n_octavelayers=3;

	m_calc_main_dir_flag = false;
}

FeatureDetector::~FeatureDetector()
{

}

void FeatureDetector::setCommonParams(int n_octaves, int n_octavelayers)
{
	m_n_octaves=n_octavelayers;
	m_n_octavelayers=n_octavelayers;
}

void FeatureDetector::enableCalcMainDir()
{
	m_calc_main_dir_flag = true;
}

void FeatureDetector::disableCalcMainDir()
{
	m_calc_main_dir_flag = false;
}

void FeatureDetector::setExcludeRegion(Matrix<unsigned char> exclude_region)
{
	m_exclude_region = exclude_region;
}

}
