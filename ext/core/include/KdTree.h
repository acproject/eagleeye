#pragma once
//整合自定义的数据类型使用flann核心搜索算法进行最近邻检索
//模板类不涉及导出问题
#include "types.h"
#include "flann/flann.hpp"
#include <iostream>

template<template<typename> class T,typename DataType>
class KdTree
{
public:
	KdTree(void);
	~KdTree(void);
public:

	void setInput(T < DataType >* source,int featuredimension);
	void searchKN(DataType &queryvector,int k,std::vector<int> &nn_indices,std::vector<float> &nn_dists);
	void searchKN(int idx,int k,std::vector<int> &nn_indices,std::vector<float> &nn_dists);
	
	void searchRN(DataType &queryvector,float dis,std::vector<int> &nn_indices,std::vector<float> &nn_dists);
	void searchRN(int idx,float dis,std::vector<int> &nn_indices,std::vector<float> &nn_dists);
private:
	flann::Index<flann::L2<float>>* index_;
	flann::Matrix<float>* dataset_;
	int featuredimension_;
	int pointnum_;
};

#include "KdTree.hpp"