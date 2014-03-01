#include "MatlabShowImp.h"

namespace eagleeye
{
template<typename T>
void putToMatlab( const Matrix<T>& img, const char* name)
{
	Matrix<float> one_channel_img(img.rows(),img.cols());

	int rows=img.rows();
	int cols=img.cols();
	for (int i=0;i<rows;++i)
	{
		for (int j=0;j<cols;++j)
		{
			T pixel=img(i,j);
			one_channel_img(i,j) = (float)OperateTrait<T>::unit(pixel,0);
		}
	}
	int byte_step = cols * sizeof(float);

	MatlabShowImp::getInstance()->imgShow(one_channel_img.dataptr(),
		one_channel_img.rows(), one_channel_img.cols(), byte_step, name );
	return;
}

template<>
void putToMatlab<ERGB>( const Matrix<ERGB>& img, const char* name )
{
	int byte_step = img.cols() * 3 * sizeof(unsigned char);

	MatlabShowImp::getInstance()->rgbShow(img.dataptr()->data,
		img.rows(), img.cols(), byte_step, name );
	return;
}

}