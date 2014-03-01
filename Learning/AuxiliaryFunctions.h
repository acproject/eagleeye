#ifndef _AUXILIARYFUNCTIONS_H_
#define _AUXILIARYFUNCTIONS_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "TraitCenter.h"
#include <stdlib.h>
#include <vector>
#include <map>

namespace eagleeye
{
/**
 *	@brief get color index of pixel c
 *	@note only support ERGB, unsigned char, char, unsigned short,
 *	short, unsigned int and int. every unit can be 0~255. It has the 
 *	same effect with getColor(const unsigned char* c).
 */
template<class T>
int getColor(const T p);
template<class T>
void putColor(T& p,int cc);

int getColor(const unsigned char* c);
void putColor(unsigned char* c,int cc);

/**
 *	@brief analyze the annotation image and get probability value of every pixel automatically
 *	@param img annotation image
 *	@param labels_num labels number
 *	@param probability the probability that every pixel belong to some label
 *	@param colors_map the relationship between label and color int[255]
 *	@param gt_prob the global probability
 *	@note we could adjust gt_prob to change the confidence degree of every pixel on annotation image
 */
template<class T>
void analyzeAnnotationImg(const Matrix<T>& img,int labels_num,
						  Matrix<float>& energy,
						  std::vector<int>& colors_map,
						  float gt_prob = 0.5f);

/**
 *	@brief resampling
 *	@note when the number of samples isn't balance critically, they should be resampled.
 *	we must put samples on order.For example, 0,0,0,0...,1,1,1,1,....,2,2,2,2,2,...
 */
void resampling(Matrix<float>& samples_representation,Matrix<float>& samples_label);

/**
 *	@brief shuffling algorithm
 *	@note http://en.wikipedia.org/wiki/Fisher¨CYates_shuffle
 */
enum ShuffleMode
{
	IN_PLACE
};
template<class T>
void shuffling(std::vector<T>& data,ShuffleMode shuffle_mode = IN_PLACE);
}

#include "AuxiliaryFunctions.hpp"
#endif