namespace eagleeye
{
template<class ImageSigT>
SuperpixelSplitCargo<ImageSigT>::SuperpixelSplitCargo()
{
	//set input port number
	setNumberOfInputSignals(1);

	//set output port property
	setNumberOfOutputSignals(1);
	setOutputPort(new ImageSigT,OUTPUT_PORT_DISPLAY_SPLIT_IMAGE);

	m_gray_threshold = 190;
}
template<class ImageSigT>
SuperpixelSplitCargo<ImageSigT>::~SuperpixelSplitCargo()
{

}
template<class ImageSigT>
void SuperpixelSplitCargo<ImageSigT>::executeNodeInfo()
{
	//get input image
	Matrix<PixelType> input_img = TO_IMAGE(PixelType,getInputPort(INPUT_PORT_IMAGE_DATA));
	Matrix<unsigned char> uimg = input_img.transform<unsigned char>();
	
	int rows = uimg.rows();
	int cols = uimg.cols();

	float fore_gray,back_gray;
	float threshold_gray;
	autoBWSplit(uimg,threshold_gray,fore_gray,back_gray);

	m_gray_threshold = unsigned char(threshold_gray);

	//project to bottom
	std::vector<float> project_data(cols,0);
	Matrix<float> project_img(1,uimg.cols(),&project_data[0],false);
	Matrix<unsigned char> mask_img(rows,cols,unsigned char(0));
	float* project_img_data = project_img.row(0);
	for (int i = 0; i < rows; ++i)
	{
		unsigned char* uimg_data = uimg.row(i);
		unsigned char* mask_data = mask_img.row(i);
		for (int j = 0; j < cols; ++j)
		{
			if (uimg_data[j] < m_gray_threshold)
			{
				project_img_data[j] += 1.0f;
				mask_data[j] = 1;
			}
			else
			{
				mask_data[j] = 0;
			}
		}
	}

	MedianFilter1D<float> median_filter(81);
	std::vector<float> filter_data = median_filter.execute(project_data);
	
	putToMatlab(Matrix<float>(1,cols,&filter_data[0],false),"haha");
	putToMatlab(mask_img,"mask");
}
template<class ImageSigT>
bool SuperpixelSplitCargo<ImageSigT>::selfcheck()
{
	return true;
}
template<class ImageSigT>
void SuperpixelSplitCargo<ImageSigT>::splitToSuperpixel()
{

}
}
