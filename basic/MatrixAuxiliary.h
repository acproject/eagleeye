#ifndef _MATRIXAUXILIARY_H_
#define _MATRIXAUXILIARY_H_

#include "EagleeyeMacro.h"

#include "Array.h"
#include "DynamicArray.h"
#include "Matrix.h"
#include <vector>
#include "TraitCenter.h"

namespace eagleeye
{
/**
 *	@brief Help padArray to define pad pattern
 */
enum PadPattern
{
	Pre,
	Post,
	PrePost
};

/**
 *	@brief pad the src matrix 
 *	@param src the src matrix
 *	@param ext the extent size
 *	@param padvalue the padding value
 *	@param pat the padding pattern
 */
template<typename T>
Matrix<T> padArray(const Matrix<T>& src,const Array<int,2>& ext,T padval,PadPattern pat=PrePost);

/**
 *	@fn template<typename T>\n
 *	void normalize(Matrix<T> m,const T minvalue,const T maxvalue);
 *	@param m any matrix
 *	@param minvalue the minimum value
 *	@param maxvalue the maximum value
 *	@note After implementing this function, the data in m would be changed.
 */
template<typename T>
void normalize(Matrix<T>& m,T minvalue=0,T maxvalue=0);

/**
 *	@fn template<typename T>
 *	void normalize(Array<T>& arr,T minvalue=0,T maxvalue=0);
 *	@param arr any array
 *	@param minvalue the minimum value
 *	@param maxvalue the maximum value
 *	@note After implementing this function,the data in arr would be changed.
 */
template<typename T>
void normalize(DynamicArray<T>& arr,T minvalue=0,T maxvalue=0);

/**
 *	@brief extract contour on segment label image
 *	@note Can also be used to draw boundaries around superpixel
 */
Matrix<unsigned int> extractContourAroundSegment(const Matrix<unsigned int>& segment_labels,
												 unsigned int fill_val=1);

/**
 *	@brief remove some isolated labels. the labels number would be decreased.
 *	@detail 
 *	(1) finding an adjacent label for each new component at the start \n
 *	(2) if a certain component is too small, assigning the previously found \n
 *	adjacent label to this component and not incrementing the label.
 *	@param labels			input labels that need to be corrected to remove stray labels
 *	@param new_labels		new labels
 *	@param new_labels_num	the number of labels changes in the end if segments are removed
 *	@param k				the number of segments desired by the user
 *	@note before_labels and after_labels shouldn't be the same matrix.
 */
EAGLEEYE_API void enforceLabelConnectivity(const Matrix<int>& before_labels,
							  const int K,
							  Matrix<int>& after_labels,
							  int& after_labels_num);

template<class T>
Matrix<T> averageImageWithLabel(const Matrix<int>& label,const Matrix<T>& img);

/**
 *	@brief split multi layer matrix to some single layer matrix
 *	@param multi_layers_matrix any matrix, such as Matrix<Array<float,4>>
 *	@param single_layer_matrixs some matrixs based on element "float"
 */
template<class T>
void splitsToMultiLayers(const Matrix<T>& multi_layers_matrix,
						 std::vector<Matrix<typename AtomicTypeTrait<T>::AtomicType>>& single_layer_matrixs);
}

#include "MatrixAuxiliary.hpp"
#endif