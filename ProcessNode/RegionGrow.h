#ifndef _REGIONGROW_H_
#define _REGIONGROW_H_

/**
 * @brief 区域增长
 * @note 输入图像被当做(0/其他)二值图，在这个二值图上做区域增长，得到每个区域的rect和mask
 */

#include "EagleeyeMacro.h"
#include "ImageProcessNode.h"
#include "Matrix.h"
#include "MatrixMath.h"

namespace eagleeye
{
	template<class ImageSigT>
	class RegionGrow: public ImageProcessNode<ImageSigT,ImageSignal<unsigned char>>
	{
	public:
		/**
		 * @brief define some basic type
		 * @note must typedef Self/Superclass
		 */
		typedef RegionGrow						Self;
		typedef ImageProcessNode<ImageSigT,ImageSignal<unsigned char>>	Superclass;
		typedef typename ImageSigT::MetaType											PixelType;
		RegionGrow();
		virtual ~RegionGrow();
		
		/** 
		 * @brief Get class identity
		 */
		EAGLEEYE_CLASSIDENTITY( RegionGrow );

		/**
		 *	@brief define input and output port
		 */
		EAGLEEYE_INPUT_PORT_TYPE(ImageSigT,0,IMAGE_DATA);
		EAGLEEYE_OUTPUT_PORT_TYPE(ImageSignal<unsigned char>,0,LABEL_IMAGE_DATA);

		/**
		 * @brief 输出最小区域像素数限制
		 */
		void setRegionMinArea( const int pix_num )
		{
			m_min_area = pix_num;
		}
		void getRegionMinArea(int& pix_num)
		{
			pix_num = m_min_area;
		}
		/**
		 * @brief 4邻域或8邻域。输入4或8
		 */
		void setNeibMode( int neib_num )
		{
			if( neib_num<8 )
				m_neib_num = 4;
			else
				m_neib_num = 8;
		}
		/**
		 * @brief 得到大于面积阈值的区域数量
		 */
		int getRegionNum()
		{
			return m_region_counter_area_limited;
		}
		/**
		 * @brief 得到每个区域的像素大小。输出为std::vector
		 */
		std::vector<int> getRegionPixelNums()
		{
			return m_pixel_num_vector;
		}
		/**
		 * @brief 得到第index个区域的mask图像，图像为0/255取值的二值图。标号从0开始
		 */
		Matrix<unsigned char> getMask( int index )
		{
			if( index<(int)m_masks.size() )
				return m_masks[index];
			else
				return Matrix<unsigned char>(1,1,(unsigned char)0);
		}
		/**
		 * @brief 得到第index个区域的rect
		 */
		RECT getRect( int index )
		{
			RECT res;
			res.bottom = 0; res.left = 0; res.right = 0; res.top = 0;
			if( index<(int)m_rects.size() )
				return m_rects[index];
			else
				return res;
		}
		/**
		 * @brief 得到最大区域标号
		 */
		int getBiggestIndex();
		/**
		 * @brief 实现虚函数
		 */
		virtual void executeNodeInfo();
		virtual bool selfcheck();
		
	private:
		void grow( Matrix<PixelType>& image );
		void getBlock( int y, int x, Matrix<unsigned char>& image );

		std::vector<int> m_pixel_num_vector;
		int m_region_counter;
		Matrix<float> m_label_image;
		int m_neib_num;
		int m_min_area;
		int m_region_counter_area_limited;
		std::vector<RECT> m_rects;
		std::vector<Matrix<unsigned char>> m_masks;
	};
}

#include "RegionGrow.hpp"

#endif