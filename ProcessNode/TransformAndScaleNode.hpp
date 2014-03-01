namespace eagleeye
{
template<class SrcImageSigT,class TargetImageSigT>
TransformAndScaleNode<SrcImageSigT,TargetImageSigT>::TransformAndScaleNode()
{
	m_control_type = THRESHOLD_BINARY_CONTROL;

	m_binary_threshold = 0;
	memset( m_clip_threshold, 0, sizeof(float) * 2 );
	m_clip_normalize_min_value = 0;
	m_clip_normalize_max_value = 0;

	//set input port number
	setNumberOfInputSignals(1);

	//set output port property
	setNumberOfOutputSignals(1);
	setOutputPort(makeOutputSignal(),OUTPUT_PORT_TRANSFORMED_IMAGE_DATA);
}

template<class SrcImageSigT,class TargetImageSigT>
TransformAndScaleNode<SrcImageSigT,TargetImageSigT>::~TransformAndScaleNode()
{

}

template<class SrcImageSigT,class TargetImageSigT>
void TransformAndScaleNode<SrcImageSigT,TargetImageSigT>::setControlType(ControlType control_type)
{
	m_control_type = control_type;
}

template<class SrcImageSigT,class TargetImageSigT>
void TransformAndScaleNode<SrcImageSigT,TargetImageSigT>::setBinaryThreshold(float threshold)
{
	m_binary_threshold = threshold;
}

template<class SrcImageSigT,class TargetImageSigT>
void TransformAndScaleNode<SrcImageSigT,TargetImageSigT>::setClipThreshold(float min_threshold,float max_threshold)
{
	m_clip_threshold[0] = min_threshold;
	m_clip_threshold[1] = max_threshold;
}

template<class SrcImageSigT,class TargetImageSigT>
void TransformAndScaleNode<SrcImageSigT,TargetImageSigT>::setClipNormalizeMinMax(float min_val,float max_val)
{
	m_clip_normalize_min_value = min_val;
	m_clip_normalize_max_value = max_val;
}

template<class SrcImageSigT,class TargetImageSigT>
void TransformAndScaleNode<SrcImageSigT,TargetImageSigT>::executeNodeInfo()
{
	SrcImageSigT* src_img_sig=getInputImageSignal();
	if ( !src_img_sig )
	{
		EAGLEEYE_ERROR("sorry, couldn't transform to predefined input image signal... \n");
		return;
	}

	Matrix<InputPixelType> input_img=src_img_sig->img;
	Matrix<OutputPixelType> output_img;

	switch(m_control_type)
	{
	case THRESHOLD_CLIP_CONTROL:
		{
			clipImage(input_img,output_img);
			EAGLEEYE_INFO("finish clip operation... \n");
			break;
		}
	case THRESHOLD_BINARY_CONTROL:
		{
			binaryImage(input_img,output_img);
			EAGLEEYE_INFO("finish binary operation... \n");
			break;
		}
	case NORMALIZE_CLIP_CONTROL:
		{
			clipNormalizeImage(input_img,output_img);
			EAGLEEYE_INFO("finish normalization operation... \n");
			break;
		}
	case TYPE_CONTROL:
		{
			output_img=input_img.transform<OutputPixelType>();
			EAGLEEYE_INFO("finish type transform operation... \n");
		}
	default:
		{
			output_img=input_img.transform<OutputPixelType>();
			EAGLEEYE_ERROR("sorry, couldn't finish transform and scale operation... \n");
			break;
		}
	}

	TargetImageSigT* output_img_sig=getOutputImageSignal();
	output_img_sig->img=output_img;
}

template<class SrcImageSigT,class TargetImageSigT>
bool TransformAndScaleNode<SrcImageSigT,TargetImageSigT>::selfcheck()
{
	if ( AtomicTypeTrait<InputPixelType>::size != 1 || AtomicTypeTrait<OutputPixelType>::size != 1 )
	{
		EAGLEEYE_ERROR("sorry, input and output image channels must be one... \n");
		return false;
	}

	if ( !getInputImageSignal() )
	{
		EAGLEEYE_ERROR("sorry, input image pixel isn't consistent... \n please be careful... \n");
		return false;
	}

	return true;
}

template<class SrcImageSigT,class TargetImageSigT>
void TransformAndScaleNode<SrcImageSigT,TargetImageSigT>::printUnit()
{
	Superclass::printUnit();

	switch(m_control_type)
	{
	case THRESHOLD_CLIP_CONTROL:
		{
			EAGLEEYE_INFO("control type %s \n","THRESHOLD_CLIP_CONTROL");
			EAGLEEYE_INFO("clip threshold min -- (%f) ; max -- (%f) \n", m_clip_threshold[0], m_clip_threshold[1]);
			break;
		}
	case THRESHOLD_BINARY_CONTROL:
		{
			EAGLEEYE_INFO("control type %s \n","THRESHOLD_BINARY_CONTROL");
			EAGLEEYE_INFO("binary threashold %f \n", m_binary_threshold);
			break;
		}
	case NORMALIZE_CLIP_CONTROL:
		{
			EAGLEEYE_INFO("control type %s \n","NORMALIZE_CLIP_CONTROL");
			EAGLEEYE_INFO("clip threshold min -- (%f) ; max -- (%f) \n", m_clip_normalize_min_value,m_clip_normalize_max_value);
			break;
		}
	case TYPE_CONTROL:
		{
			EAGLEEYE_INFO("control type %s \n","TYPE_CONTROL");
			break;
		}
	}
}

template<class SrcImageSigT,class TargetImageSigT>
void TransformAndScaleNode<SrcImageSigT,TargetImageSigT>::clipImage(const Matrix<InputPixelType>& input_img, Matrix<OutputPixelType>& output_img)
{
	int rows = input_img.rows();
	int cols = input_img.cols();

	output_img = Matrix<OutputPixelType>(rows,cols,OutputPixelType(0));

	for (int i = 0; i < rows; ++i)
	{
		const InputPixelType* input_img_data = input_img.row(i);
		OutputPixelType* output_img_data = output_img.row(i);

		for (int j = 0; j < cols; ++j)
		{
			if ( input_img_data[j] > (InputPixelType)m_clip_threshold[0] && input_img_data[j] < (InputPixelType)m_clip_threshold[1] )
			{
				output_img_data[j] = (OutputPixelType)input_img_data[j];
			}
			else
			{
				if ( input_img_data[j] < (InputPixelType)m_clip_threshold[0] )
				{
					output_img_data[j] = (OutputPixelType)m_clip_threshold[0];
				}
				else
				{
					output_img_data[j] = (OutputPixelType)m_clip_threshold[1];
				}
			}
		}
	}
}

template<class SrcImageSigT,class TargetImageSigT>
void TransformAndScaleNode<SrcImageSigT,TargetImageSigT>::binaryImage(const Matrix<InputPixelType>& input_img, Matrix<OutputPixelType>& output_img)
{
	int rows = input_img.rows();
	int cols = input_img.cols();

	output_img = Matrix<OutputPixelType>(rows,cols,OutputPixelType(0));

	for (int i = 0; i < rows; ++i)
	{
		const InputPixelType* input_img_data = input_img.row(i);
		OutputPixelType* output_img_data = output_img.row(i);

		for (int j = 0; j < cols; ++j)
		{
			if (input_img_data[j] > (InputPixelType)m_binary_threshold)
			{
				output_img_data[j] = OutputPixelType(1);
			}
			else
			{
				output_img_data[j] = OutputPixelType(0);
			}
		}
	}
}

template<class SrcImageSigT,class TargetImageSigT>
void TransformAndScaleNode<SrcImageSigT,TargetImageSigT>::clipNormalizeImage(const Matrix<InputPixelType>& input_img, Matrix<OutputPixelType>& output_img)
{
	int rows=input_img.rows();
	int cols=input_img.cols();
	typedef NormalizeOperation<InputPixelType,OutputPixelType> NormalizeType;

	if ( m_clip_normalize_min_value == 0 && m_clip_normalize_max_value == 0 )
	{
		InputPixelType min_val,max_val;
		getMaxMin( input_img, max_val, min_val );

		output_img = 
			input_img.transform(NormalizeType(min_val,max_val,
			OutputPixelType(0),OutputPixelType(1)));
	}
	else
	{
		output_img = 
			input_img.transform(NormalizeType(InputPixelType(m_clip_normalize_min_value),InputPixelType(m_clip_normalize_max_value),
			OutputPixelType(0),OutputPixelType(1)));
	}
}

}