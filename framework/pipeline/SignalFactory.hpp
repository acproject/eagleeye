namespace eagleeye
{
//////////////////////////////////////////////////////////////////////////
template<class T>
ImageSignal<T>::ImageSignal(Matrix<T> m,char* name,char* info)
				:BaseImageSignal(AtomicTypeTrait<T>::pixel_type,
				AtomicTypeTrait<T>::size,
				info),
				img(m),
				img_name(name)
{
	needed_size[0] = m.rows();
	needed_size[1] = m.cols();
}

template<class T>
void ImageSignal<T>::copyInfo(AnySignal* sig)
{
	//call the base class
	BaseImageSignal::copyInfo(sig);

	//receive some info from the upper signal
	ImageSignal<T>* from_sig = dynamic_cast<ImageSignal<T>*>(sig);	
	if (from_sig)
	{
		img_name = from_sig->img_name;
	}
}

template<class T>
void ImageSignal<T>::printUnit()
{
	Superclass::printUnit();

	EAGLEEYE_INFO("image name %s \n",img_name.c_str());
	EAGLEEYE_INFO("image type %d channels %d rows %d cols %d \n",pixel_type,channels,img.rows(),img.cols());
}

template<class T>
void ImageSignal<T>::makeempty()
{
	img = Matrix<T>();
	img_name = "";
	ext_info = "";
	
	//force time update
	modified();
}

template<class T>
void ImageSignal<T>::createImage(Matrix<T> m,char* name="",char* info="")
{
	img = m;
	img_name = name;
	ext_info = info;

	//force time update
	modified();
}

template<class T>
Matrix<T> ImageSignal<T>::getImage()
{
	return img;
}

template<class T>
void ImageSignal<T>::getEagleEyeData(void*& eagleeye_data,
									 int& eagleeye_rows,int& eagleeye_cols,
									 int& eagleeye_offset_r,int& eagleeye_offset_c,
									 int& eagleeye_width)
{
	eagleeye_data=img.dataptr();
	eagleeye_rows=img.rows();
	eagleeye_cols=img.cols();

	unsigned int offset_r,offset_c;
	img.offset(offset_r,offset_c);
	eagleeye_offset_r=(unsigned int)offset_r;
	eagleeye_offset_c=(unsigned int)offset_c;
	eagleeye_width=img.width();
}

//////////////////////////////////////////////////////////////////////////
template<class InfoT>
void InfoSignal<InfoT>::copyInfo(AnySignal* sig)
{
	//call the base class
	Superclass::copyInfo(sig);

	//receive some info from the upper signal
	InfoSignal<InfoT>* from_sig=dynamic_cast<InfoSignal<InfoT>*>(sig);	
	if (from_sig)
	{
		info=from_sig->info;
	}
}

template<class InfoT>
void InfoSignal<InfoT>::printUnit()
{
	Superclass::printUnit();

	EAGLEEYE_INFO("info pointer %d",info);
}

template<class InfoT>
void InfoSignal<InfoT>::makeempty()
{
	info=NULL;

	//force time update
	modified();
}

}