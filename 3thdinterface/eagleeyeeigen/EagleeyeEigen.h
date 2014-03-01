#ifndef _EAGLEEYEEIGEN_H_
#define _EAGLEEYEEIGEN_H_

namespace eagleeye
{
typedef Eigen::Matrix<float,Eigen::Dynamic,Eigen::Dynamic,Eigen::RowMajor> EigenMatf;
typedef Eigen::Map<EigenMatf> EigenMapf;
}

#endif