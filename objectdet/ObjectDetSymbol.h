#ifndef _OBJECTDETSYMBOL_H_
#define _OBJECTDETSYMBOL_H_

#include "EagleeyeMacro.h"

#include "Symbol.h"
#include "DataPyramid.h"
#include <vector>
#include <string>
#include <map>

namespace eagleeye
{
class DetSymbolInfo 
{
public:
	int superpixel;
	int x;				//x location for detection object
	int y;				//y location for detection object
	int width;			//width for detection object
	int height;			//height for detection object
	int label;			//object label
	int level;			//scale level for detection object
	float scale;		//scale for detection object
	int ds;				//of 2x scalings relative to the start symbol
	float val;			//score for detection object

	DetSymbolInfo(int s_superpixel = 0,int s_x = 0,int s_y = 0,int s_width = 0,int s_height = 0,int s_label = 1,
		int s_level = 0,float s_scale = 0.0f,int s_ds = 0,float s_val = 0.0f)
		:superpixel(s_superpixel),x(s_x),y(s_y),width(s_width),height(s_height),label(s_label),
		level(s_level),ds(s_ds),val(s_val),scale(s_scale){};
	~DetSymbolInfo(){};
};

struct AuxiliaryInfoInPixelSpace
{
	int obj_x;
	int obj_y;
	int obj_width;
	int obj_height;
};

struct AuxiliaryInfoInSuperpixelSpace
{
	Matrix<unsigned char> annotation_label;
	DynamicDataPyramid<int> superpixel_pyr;
	std::vector<int> superpixel_num_pyr;
};

class EAGLEEYE_API ObjectDetSymbol:public Symbol
{
public:
	ObjectDetSymbol(const char* name="ObjectDetSymbol",SymbolType type=TERMINAL);
	virtual ~ObjectDetSymbol();

	/**
	 *	@brief set/get fixed anchor of symbol
	 *	@note for PIXEL_SPACE
	 */
	void setAnchor(int anchor_r,int anchor_c,int anchor_level);
	void getAnchor(int& anchor_r,int& anchor_c,int& anchor_level);

protected:
	/**
	 *	@brief get the label of every superpixel by annotation img
	 *	@note for SUPERPIXEL_SPACE
	 */
	Matrix<unsigned char> getSuperpixelCentersLabel(const Matrix<int>& superpixel_index,int superpixel_num,const Matrix<unsigned char>& label_annotation);
	
	/**
	 *	@brief adjust superpixel by some extra info
	 *	@detail consider gray larger than 'invalid_gray' as invalid gray
	 *	@note for SUPERPIXEL_SPACE
	 */
	void squeezeInvalidSuperpixel(const Matrix<int>& superpixel_index,
		const int superpixel_num,
		const Matrix<unsigned char>& img,
		const unsigned char predict_invalid_gray,
		Matrix<int>& after_superpixel_img,
		int& after_superpixel_num,
		Matrix<int>& after_pnum_in_superpixel,
		Matrix<unsigned char>& predict_superpixel_label);

	/**
	 *	@brief adjust superpixel by some extra info
	 *	@note for SUPERPIXEL_SPACE
	 */
	void squeezeInvalidSuperpixel(const Matrix<int>& superpixel_index,
		const int superpixel_num,
		const Matrix<unsigned char>& label_annotation,
		Matrix<int>& after_superpixel_img,
		int& after_superpixel_num,
		Matrix<int>& after_pixel_num_of_every_superpixel,
		Matrix<unsigned char>& after_superpixel_label);

	int m_anchor_r;								/**< row index*/
	int m_anchor_c;								/**< col index*/
	int m_anchor_level;							/**< level index within pyramid*/
	
	static DetSymbolInfo m_symbol_info;			/**< Object detection info(latent info)*/
};
}

#endif