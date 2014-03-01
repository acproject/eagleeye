#ifndef _IMAGEDESTRIPE_H_
#define _IMAGEDESTRIPE_H_

#include "EagleeyeMacro.h"
#include "ImageProcessNode.h"
#include "MatrixMath.h"
#include "Matrix.h"

namespace eagleeye
{
	template<class ImageSigT>
	class ImageDestripe:public ImageProcessNode<ImageSigT, ImageSignal<float>>
	{
	public:
		/**
		 * @brief define some basic type
		 * @note must typedef Self/Superclass
		 */
		typedef ImageDestripe							Self;
		typedef ImageProcessNode<ImageSigT,ImageSignal<float>>	Superclass;
		typedef typename ImageSigT::MetaType			PixelType;

		ImageDestripe();
		virtual ~ImageDestripe();

		/**
		* @brief Get class identity
		*/
		EAGLEEYE_CLASSIDENTITY(ImageDestripe);

		EAGLEEYE_INPUT_PORT_TYPE(ImageSigT,0,IMAGE_DATA);
		EAGLEEYE_OUTPUT_PORT_TYPE(ImageSignal<float>,0,NORMALIZE_IMAGE_DATA);

		/**
		* @brief set search range parameter
		*/
		void setSearchRange( const int search_range );
		void setGrayThreshold( const int gray_threshold );

		virtual void executeNodeInfo();

		virtual bool selfcheck();

	private:

		void horizontalDestripe( Matrix<float>& reference_img );

		int m_search_range;
		int m_nth_biggest;
		int m_gray_threshold;
	};
}

#include "ImageDestripe.hpp"

#endif