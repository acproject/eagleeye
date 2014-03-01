namespace eagleeye
{
template<class ImgSigT>
ImageWriteNode<ImgSigT>::ImageWriteNode()
{
	//generate input signal
	setNumberOfInputSignals(1);
	
	//set one empty output port
	setNumberOfOutputSignals(1);
	setOutputPort(makeOutputSignal(),0);
}

template<class ImgSigT>
ImageWriteNode<ImgSigT>::~ImageWriteNode()
{

}

template<class ImgSigT>
void ImageWriteNode<ImgSigT>::passonNodeInfo()
{
	//now do nothing
}

template<class ImgSigT>
void ImageWriteNode<ImgSigT>::executeNodeInfo()
{
	core::FileManager::FileHandle file_handle = core::FileManager::FileFactory(m_file_path.c_str());

	if (!file_handle.get())
	{
		EAGLEEYE_ERROR("sorry, I couldn't write image\n");
		return;
	}

	ImgSigT* input_img_signal = dynamic_cast<ImgSigT*>(getInputPort(INPUT_PORT_IMAGE_DATA));
	Matrix<PixelType> input_img = input_img_signal->img;
	input_img.clone();

	//write image file
	switch(AtomicTypeTrait<PixelType>::pixel_type)
	{
	case EAGLEEYE_CHAR:
		{
			file_handle->saveImageData((void*)input_img.dataptr(),
				input_img.rows(),
				input_img.cols(),
				core::CORE_CHAR);
			break;
		}
	case EAGLEEYE_UCHAR:
		{
			file_handle->saveImageData((void*)input_img.dataptr(),
				input_img.rows(),
				input_img.cols(),
				core::CORE_UCHAR);
			break;
		}
	case EAGLEEYE_SHORT:
		{
			file_handle->saveImageData((void*)input_img.dataptr(),
				input_img.rows(),
				input_img.cols(),
				core::CORE_SHORT);
			break;
		}
	case EAGLEEYE_USHORT:
		{
			file_handle->saveImageData((void*)input_img.dataptr(),
				input_img.rows(),
				input_img.cols(),
				core::CORE_USHORT);
			break;
		}
	case EAGLEEYE_INT:
		{
			file_handle->saveImageData((void*)input_img.dataptr(),
				input_img.rows(),
				input_img.cols(),
				core::CORE_INT);
			break;
		}
	case EAGLEEYE_UINT:
		{
			file_handle->saveImageData((void*)input_img.dataptr(),
				input_img.rows(),
				input_img.cols(),
				core::CORE_UINT);
			break;
		}
	case EAGLEEYE_FLOAT:
		{
			file_handle->saveImageData((void*)input_img.dataptr(),
				input_img.rows(),
				input_img.cols(),
				core::CORE_FLOAT);
			break;
		}
	case EAGLEEYE_RGB:
		{
			//switch channels order
			Matrix<ERGB> swith_img = input_img.transform<ERGB>();
			
			//switch pixel channel
			switchPixelChannels(swith_img);

			file_handle->saveImageData((void*)swith_img.dataptr(),
				swith_img.rows(),
				swith_img.cols(),
				core::CORE_RGB_UCHAR);
			break;
		}
	case EAGLEEYE_RGBA:
		{
			//switch channels order
			Matrix<ERGBA> swith_img = input_img.transform<ERGBA>();
			//switch pixel channel
			switchPixelChannels(swith_img);

			file_handle->saveImageData((void*)swith_img.dataptr(),
				swith_img.rows(),
				swith_img.cols(),
				core::CORE_RGBA_UCHAR);
			break;
		}
	default:
		{
			EAGLEEYE_ERROR("couldn't write image\n");
			break;
		}
	}
}

template<class ImgSigT>
bool ImageWriteNode<ImgSigT>::selfcheck()
{
	if (m_file_path == "")
	{
		EAGLEEYE_ERROR("file path couldn't be empty\n");
		return false;
	}

	core::FileManager::FileHandle file_handle = core::FileManager::FileFactory(m_file_path.c_str());
	if (!file_handle.get())
	{
		EAGLEEYE_ERROR("sorry,I couldn't support this file type ()\n");
		return false;
	}

	//judge whether input image signal is correct
	ImgSigT* input_img_signal = dynamic_cast<ImgSigT*>(getInputPort(0));
	if (!input_img_signal)
	{
		EAGLEEYE_ERROR("sorry, image type isn't consistent.please be careful...\n");
		return false;
	}

	return true;
}
}
