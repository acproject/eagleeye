namespace eagleeye
{
template<class SrcT,class TargetT>
ImageProcessNode<SrcT,TargetT>::ImageProcessNode()
{
}

template<class SrcT,class TargetT>
ImageProcessNode<SrcT,TargetT>::~ImageProcessNode()
{

}

template<class SrcT,class TargetT>
SrcT* ImageProcessNode<SrcT,TargetT>::getInputImageSignal(unsigned int index)
{
	SrcT* img_signal=dynamic_cast<SrcT*>(m_input_signals[index]);
	return img_signal;
}

template<class SrcT,class TargetT>
const SrcT* ImageProcessNode<SrcT,TargetT>::getInputImageSignal(unsigned int index) const 
{
	SrcT* img_signal=dynamic_cast<SrcT*>(m_input_signals[index]);
	return img_signal;
}

template<class SrcT,class TargetT>
TargetT* ImageProcessNode<SrcT,TargetT>::getOutputImageSignal(unsigned int index)
{
	TargetT* img_signal=dynamic_cast<TargetT*>(m_output_signals[index]);
	return img_signal;
}

template<class SrcT,class TargetT>
const TargetT* ImageProcessNode<SrcT,TargetT>::getOutputImageSignal(unsigned int index) const
{
	TargetT* img_signal=dynamic_cast<TargetT*>(m_output_signals[index]);
	return img_signal;
}

template<class SrcT,class TargetT>
Matrix<typename TargetT::MetaType> ImageProcessNode<SrcT,TargetT>::getOutputImage(unsigned int index/* =0 */)
{
	if (index<m_output_signals.size())
	{
		TargetT* img_signal=dynamic_cast<TargetT*>(m_output_signals[index]);
		if (img_signal)
		{
			return img_signal->img;
		}
		else
		{
			return Matrix<OutputPixelType>();
		}
	}
	else
	{
		return Matrix<OutputPixelType>();
	}
}

template<class SrcT,class TargetT>
AnySignal* ImageProcessNode<SrcT,TargetT>::makeOutputSignal()
{
	return new TargetT();
}
}
