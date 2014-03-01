namespace eagleeye
{
template<class ImgSigT>
ImageIONode<ImgSigT>::ImageIONode()
{
	m_file_path = "";
	m_channels_order = CHANELS_NO_CHANGE;
}
template<class ImgSigT>
ImageIONode<ImgSigT>::~ImageIONode()
{

}

template<class ImgSigT>
void ImageIONode<ImgSigT>::setFilePath(const char* file_path)
{
	m_file_path = file_path;

	//force time to update
	modified();
}

template<class ImgSigT>
std::string ImageIONode<ImgSigT>::getFilePath()
{
	return m_file_path;
}

template<class ImgSigT>
void ImageIONode<ImgSigT>::switchChanelsOrder(ChanelsOrder c_order)
{
	m_channels_order = c_order;
}

template<class ImgSigT>
AnySignal* ImageIONode<ImgSigT>::makeOutputSignal()
{
	return new ImgSigT();
}

template<class ImgSigT>
Matrix<typename ImgSigT::MetaType> ImageIONode<ImgSigT>::getImage()
{
	return TO_IMAGE(PixelType,getOutputPort(0));
}

}