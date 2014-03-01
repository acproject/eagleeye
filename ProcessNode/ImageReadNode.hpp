namespace eagleeye
{
template<class ImgSigT>
ImageReadNode<ImgSigT>::ImageReadNode()
{
	m_file_path = "";

	setNumberOfOutputSignals(1);
	setOutputPort(makeOutputSignal(),OUTPUT_PORT_IMAGE_DATA);
}

template<class ImgSigT>
ImageReadNode<ImgSigT>::~ImageReadNode()
{

}

template<class ImgSigT>
bool ImageReadNode<ImgSigT>::selfcheck()
{
	if (m_file_path == "")
	{
		EAGLEEYE_ERROR("file path is empty\n");
		return false;
	}

	core::FileManager::FileHandle file_handle = core::FileManager::FileFactory(m_file_path.c_str());
	if (!file_handle.get())
	{
		EAGLEEYE_ERROR("sorry, I couldn't support this file type ()\n");
		return false;
	}

	return true;
}

template<class ImgSigT>
void ImageReadNode<ImgSigT>::passonNodeInfo()
{
	ImgSigT* output_img_signal = dynamic_cast<ImgSigT*>(getOutputPort(OUTPUT_PORT_IMAGE_DATA));
	
	core::FileManager::FileHandle file_handle = core::FileManager::FileFactory(m_file_path.c_str());

	if (!file_handle.get())
	{
		EAGLEEYE_ERROR("sorry, I couldn't parse image data\n");
		return;
	}

	output_img_signal->img_name = file_handle->getFileName();
	output_img_signal->ext_info = file_handle->getFileNameWithoutExt();
}

template<class ImgSigT>
void ImageReadNode<ImgSigT>::executeNodeInfo()
{
	ImgSigT* output_img_signal = dynamic_cast<ImgSigT*>(getOutputPort(OUTPUT_PORT_IMAGE_DATA));

	core::FileManager::FileHandle file_handle = core::FileManager::FileFactory(m_file_path.c_str());
	
	if (!file_handle.get())
	{
		EAGLEEYE_ERROR("sorry, I couldn't read image\n");
		output_img_signal->makeempty();
		return;
	}

	void* data;
	int rows,cols;
	core::DataType d_type;

	if (file_handle->getImageData(data,rows,cols,d_type))
	{
		Matrix<PixelType> me_img;

		switch(d_type)
		{
		case core::CORE_CHAR:
		case core::CORE_UCHAR:
			{
				if (AtomicTypeTrait<PixelType>::pixel_type == EAGLEEYE_CHAR||
					AtomicTypeTrait<PixelType>::pixel_type == EAGLEEYE_UCHAR)
				{
					Matrix<PixelType> img(rows,cols,data,true);
					me_img = img;
				}
				else
				{
					Matrix<unsigned char> img(rows,cols,data,true);
					me_img = img.transform<PixelType>();
				}

				output_img_signal->img = me_img;
				break;
			}
		case core::CORE_SHORT:
		case core::CORE_USHORT:
			{
				if (AtomicTypeTrait<PixelType>::pixel_type == EAGLEEYE_SHORT||
					AtomicTypeTrait<PixelType>::pixel_type == EAGLEEYE_USHORT)
				{
					Matrix<PixelType> img(rows,cols,data,true);
					me_img = img;
				}
				else
				{
					Matrix<unsigned short> img(rows,cols,data,true);
					me_img = img.transform<PixelType>();
				}

				output_img_signal->img = me_img;
				break;
			}
		case core::CORE_INT:
		case core::CORE_UINT:
			{
				if (AtomicTypeTrait<PixelType>::pixel_type == EAGLEEYE_INT||
					AtomicTypeTrait<PixelType>::pixel_type == EAGLEEYE_UINT)
				{
					Matrix<PixelType> img(rows,cols,data,true);
					me_img = img;
				}
				else
				{
					Matrix<unsigned int> img(rows,cols,data,true);
					me_img = img.transform<PixelType>();
				}

				output_img_signal->img = me_img;
				break;
			}
		case core::CORE_FLOAT:
			{
				if (AtomicTypeTrait<PixelType>::pixel_type == EAGLEEYE_FLOAT)
				{
					Matrix<PixelType> img(rows,cols,data,true);
					me_img = img;
				}
				else
				{
					Matrix<float> img(rows,cols,data,true);
					me_img = img.transform<PixelType>();
				}

				output_img_signal->img = me_img;
				break;
			}
		case core::CORE_RGB_UCHAR:
			{
				if (AtomicTypeTrait<PixelType>::pixel_type == EAGLEEYE_RGB)
				{
					Matrix<PixelType> img(rows,cols,data,true);
					me_img = img;
				}
				else
				{
					Matrix<ERGB> img(rows,cols,data,true);
					me_img = img.transform<PixelType>();
				}

				output_img_signal->img = me_img;
				break;
			}
		case core::CORE_RGBA_UCHAR:
			{
				if (AtomicTypeTrait<PixelType>::pixel_type == EAGLEEYE_RGBA)
				{
					Matrix<PixelType> img(rows,cols,data,true);
					me_img = img;
				}
				else
				{
					Matrix<ERGBA> img(rows,cols,data,true);
					me_img = img.transform<PixelType>();
				}

				output_img_signal->img = me_img;
				break;
			}
		default:
			{
				EAGLEEYE_ERROR("couldn't read image\n");
				output_img_signal->makeempty();
				break;
			}
		}

		//switch pixel channels
		switchPixelChannels(output_img_signal->img);
	}
	else
	{
		EAGLEEYE_ERROR("couldn't read image\n");
		output_img_signal->makeempty();
	}
}
}
