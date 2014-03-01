#ifndef _OBJECTDETTERMINALSYMBOL_H_
#define _OBJECTDETTERMINALSYMBOL_H_

#include "EagleeyeMacro.h"

#include "Matrix.h"
#include "Array.h"
#include "Symbol.h"
#include "DataPyramid.h"
#include "HOGFeatureFunctions.h"
#include "ObjectDetSymbol.h"
#include <string>

namespace eagleeye
{
class EAGLEEYE_API ObjectDetTerminalSymbol:public ObjectDetSymbol
{
public:
	typedef ObjectDetTerminalSymbol							Self;
	typedef ObjectDetSymbol									Superclass;

	struct _Parameter
	{
		int filter_width;
		int filter_height;
		int hog_sbin;
		int anchor_x;
		int anchor_y;
		int anchor_l;
	};

	ObjectDetTerminalSymbol(const char* name);
	virtual ~ObjectDetTerminalSymbol();

	/**
	 *	@brief get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(ObjectDetTerminalSymbol);

	/**
	 *	@brief get score at predefined position for detecting
	 */
	virtual void* getSymbolScore(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat = Matrix<float>());

	/**
	 *	@brief get feature at predefined position for learning
	 */
	virtual Matrix<float> getUnitFeature(SearchSpace search_space,SearchMode search_mode,Matrix<float> pos_mat = Matrix<float>());

	/**
	 *	@brief get/set weight vector(Filter coefficient)
	 *	@note Symbol weight and Symbol feature are one \n
	 *	pair(what's the symbol feature?? good question. In the terminal symbol, \n
	 *	the symbol feature is the hog data under the predefined filter window)
	 */
	virtual Matrix<float> getUnitWeight();
	virtual void setUnitWeight(const Matrix<float>& weight);

	/**
	 *	@brief find latent info by prior-info
	 *	@param info DetSymbolInfo
	 *	@note This function would be called implicitly.
	 */
	virtual void findModelLatentInfo(void* info);

	/**
	 *	@brief set filter window(defined in feature space)
	 *	@note we must set these two parameters. This function must be called explicitly
	 */
	void setFilterWin(int width,int height);

	/**
	 *	@brief this window size has to be transformed to filter window in feature space
	 */
	void setImageWin(int width,int height);

	/**
	 *	@brief set accepted height/width ratio
	 *	@note min_ratio and max_ratio are used to pick training samples for initialization
	 */
	void setAcceptedHeightWidthRatio(float min_ratio,float max_ratio);

	/**
	 *	@brief set hog32 pyramid parameters
	 */
	void setHOG32PyramidPara(int sbin,int interval);

protected:
	/**
	 *	@brief set this unit data
	 *	@note This function would be called by Grammar Tree implicitly
	 */
	virtual void setUnitData(void* data,SampleState sample_state,void* auxiliary_data);

	/**
	 *	@brief Set/Get this unit parameter
	 */
	virtual void getUnitPara(MemoryBlock& param_block);
	virtual void setUnitPara(MemoryBlock param_block);

	/**
	 *	@brief get the output of this terminal symbol
	 *	@note this symbol output is bounding box.(the filter window position
	 *	under the original image coordinate). By calling this function, we would 
	 *	gain the ultimate goal.
	 */
	virtual void getSymbolOutput(std::vector<void*>& output_info);

	/**
	 *	@brief learn weight only once
	 *	@note In general, learnUnit would be called once. It could help
	 *	this symbol unit find initial optimum weight
	 */
	virtual void learnUnit(const char* parse_file);

	/**
	 *	@brief using samples to learn weight
	 *	@note the 3th part provides training samples to help 
	 *	this symbol unit learn
	 */
	virtual void learnUnit(const Matrix<float>& samples);

private:
	static HOG32Pyramid m_feat_pyr;					/**< The HOG feature pyramid; shared by others*/
	static Matrix<unsigned char> m_detect_img;		/**< The detect image; shared by others*/

	Matrix<HOG32Vec> m_filter_weight;				/**< The filter(symbol weights) defined in feature space*/
	Array<int,2> m_filter_win;						/**< The filter size. defined in feature space*/

	float m_offset;

	float m_accepted_min_ratio;
	float m_accepted_max_ratio;

	int m_hog_sbin;									/**< The hog bins*/
	int m_hog_pyramid_interval;						/**< The hog pyramid octave*/

	Array<int,4> m_predict_object_region;			/**< (x1,x2,y1,y2) in image space*/
	Array<int,4> m_bounding_box;

	ScorePyramid m_hog_score_pyr;					/**< defined in feature space*/
};
}

#endif