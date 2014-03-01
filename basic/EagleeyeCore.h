#ifndef _CORE_H_
#define _CORE_H_
#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "Array.h"
#include <vector>
namespace eagleeye
{
#ifndef EAGLEEYE_LOG2
#define EAGLEEYE_LOG2 0.693147180559945
#endif
/**
 *	@brief sequential k-Means Clustering
 *	@param [in]data cluster data, rows - samples number, cols - the dimension of sample
 *	@param [in]k the cluster number
 *	@param [out]centers the cluster centers
 *	@param [out]centers_count the counts of every center
 *	@see "http://www.cs.princeton.edu/courses/archive/fall08/cos436/Duda/C/sk_means.htm"
 */
EAGLEEYE_API void skmeans(const Matrix<float>& data,int k,Matrix<float>& centers,
			 Matrix<int>& centers_count);

/**
 *	@brief forgetful sequential k-means clustering
 *	@param [in]data cluster data, rows - samples number; cols - the dimension of sample
 *	@param [in]k the cluster number
 *	@param [out]centers the cluster centers
 *	@param [out]a forgetful ratio
 *	@note Electrical engineers familiar with DSP will say that we have replaced an averaging operation \n
 *		by a low-pass-filtering operation. The result might be called the "forgetful" sequential k-means procedure. \n
 *		It is not hard to show that mi is a weighted average of the examples that were closest to mi, \n
 *		where the weight decreases exponentially with the "age" to the example.\n
 *		Thus, the initial value mo is eventually forgotten, and recent examples receive more weight than ancient examples. \n
 *		This variation of k-means is particularly simple to implement, \n
 *		and it is attractive when the nature of the problem changes over time and the cluster centers "drift."
 *	@see "http://www.cs.princeton.edu/courses/archive/fall08/cos436/Duda/C/sk_means.htm"
 */
EAGLEEYE_API void forgetful_skmeans(const Matrix<float>& data,int k,Matrix<float>& centers,float a = 0.5f);

inline int round(float a)
{
	float tmp = a - (int)a;
	if(tmp >= 0.5f)
		return (int)(a + 1);
	else
		return (int)a;
}
inline int round(double a)
{
	double tmp = a - (int)a;
	if(tmp >= 0.5)
		return (int)(a + 1);
	else
		return (int)a;
}

/**
 *	@brief draw line and circle on img
 *	@note Bresenham
 */
EAGLEEYE_API void drawLine(Matrix<ERGB>& img,int x1,int y1,int x2,int y2,ERGB c);
EAGLEEYE_API void drawCircle(Matrix<ERGB>& img,int xc,int yc,int r,int fill,ERGB c);
EAGLEEYE_API void drawRect(Matrix<ERGB>& img,int x,int y ,int width,int height,ERGB c);

class KeyPoint
{
public:
	KeyPoint():pt(0.0f),size(0),angle(-1),response(0),octave(256),class_id(-1){};
	KeyPoint(float x, float y, float _size, float _angle=-1,
		float _response=0, int _octave=256, int _class_id=-1)
		:size(_size),angle(_angle),response(_response),
		octave(_octave),class_id(_class_id){pt[0]=x;pt[1]=y;};

	Array<float,2> pt;	/**< coordinates of the keypoints (x,y)*/
	float size;			/**< diameter of the meaningful keypoint neighborhood*/
	float angle;		/**< computed orientation of the keypoint (-1 if not applicable)*/
	float response;		/**< the response by which the most strong keypoints have been selected.
						Can be used for the further sorting or subsampling*/
	int octave;			/**< octave (pyramid layer) from which the keypoint has been extracted*/
	int class_id;		/**< object clsss (if keypoints need to be clustered by an object they belong to)*/
};

/**
 *	@brief Computes a gradient orientation histogram at a specified pixel
 *	@note copy from sift
 */
EAGLEEYE_API float calcOrientationHist( const Matrix<float>& img, const Array<float,2>& pt, int radius,
	float sigma, float* hist, int n );

/**
 *	@brief Compute the main direction at "pt" on "img"
 */
EAGLEEYE_API float calcMainDirection( const Matrix<float>& img, const Array<float,2>& pt,int radius,float sigma,int n); 

/**
 *	@brief draw keypoints on img and write result to output_img
 */
EAGLEEYE_API void drawKeypoints(Matrix<ERGB>& img,std::vector<KeyPoint> keypoints, Matrix<ERGB>& output_img, float ratio=2);

enum NormMode
{
	WHITEING,
	S_BINARY,
	MIM_BINARY
};
struct WhitingParam
{
	Matrix<float> mean_vec;
	Matrix<float> var_vec;
};
struct MIMParam
{
	Matrix<float> label_vec;
	Matrix<float> thre_vec;
};
/**
 *	@brief normalize feature vector
 *	@note now support whiting, binary and binary based on max mutual information \n
 *	every sample occupy one row
 */
EAGLEEYE_API Matrix<float> featureNormalize(const Matrix<float>& data,NormMode feat_norm = WHITEING,void* norm_param = NULL);

}

#endif