#include "MatrixAuxiliary.h"

namespace eagleeye
{
Matrix<unsigned int> extractContourAroundSegment(const Matrix<unsigned int>& segment_labels,unsigned int fill_val/* =1 */)
{
	const int dx8[8] = {-1, -1,  0,  1, 1, 1, 0, -1};
	const int dy8[8] = { 0, -1, -1, -1, 0, 1, 1,  1};

	int width=segment_labels.cols();
	int height=segment_labels.rows();

	Matrix<unsigned int> contour_image(height,width,(unsigned int)0);

	int sz=width*height;
	std::vector<bool> istaken(sz, false);
	std::vector<int> contourx(sz);std::vector<int> contoury(sz);
	int mainindex(0);int cind(0);
	for( int j = 0; j < height; j++ )
	{
		for( int k = 0; k < width; k++ )
		{
			int np(0);
			for( int i = 0; i < 8; i++ )
			{
				int x = k + dx8[i];
				int y = j + dy8[i];

				if( (x >= 0 && x < width) && (y >= 0 && y < height) )
				{
					int index = y*width + x;

					//if( false == istaken[index] )//comment this to obtain internal contours
					{
						if( segment_labels.at(mainindex) != segment_labels.at(index) ) np++;
					}
				}
			}
			if( np > 1 )
			{
				contourx[cind] = k;
				contoury[cind] = j;
				istaken[mainindex] = true;
				//img[mainindex] = color;
				cind++;
			}
			mainindex++;
		}
	}

	int numboundpix = cind;//int(contourx.size());
	for( int j = 0; j < numboundpix; j++ )
	{
		int ii = contoury[j]*width + contourx[j];
		contour_image.at(ii) = 0xffffff;

		for( int n = 0; n < 8; n++ )
		{
			int x = contourx[j] + dx8[n];
			int y = contoury[j] + dy8[n];
			if( (x >= 0 && x < width) && (y >= 0 && y < height) )
			{
				int ind = y*width + x;
				if(!istaken[ind]) contour_image.at(ind) = 0;
			}
		}
	}

	return contour_image;
}

void enforceLabelConnectivity(const Matrix<int>& before_labels, 
							  const int K,
							  Matrix<int>& after_labels, 
							  int& after_labels_num)
{
	int width=before_labels.cols();
	int height=before_labels.rows();


	after_labels=Matrix<int>(height,width,int(0));
	int* after_labels_data=after_labels.dataptr();

	const int dx4[4] = {-1,  0,  1,  0};
	const int dy4[4] = { 0, -1,  0,  1};

	const int sz = width*height;
	const int SUPSZ = sz/K;
	//nlabels.resize(sz, -1);
	for( int i = 0; i < sz; i++ ) after_labels_data[i] = -1;
	int label(0);
	int* xvec = new int[sz];
	int* yvec = new int[sz];
	int oindex(0);
	int adjlabel(0);//adjacent label
	for( int j = 0; j < height; j++ )
	{
		for( int k = 0; k < width; k++ )
		{
			if( 0 > after_labels_data[oindex] )
			{
				after_labels_data[oindex] = label;
				//--------------------
				// Start a new segment
				//--------------------
				xvec[0] = k;
				yvec[0] = j;
				//-------------------------------------------------------
				// Quickly find an adjacent label for use later if needed
				//-------------------------------------------------------
				{for( int n = 0; n < 4; n++ )
				{
					int x = xvec[0] + dx4[n];
					int y = yvec[0] + dy4[n];
					if( (x >= 0 && x < width) && (y >= 0 && y < height) )
					{
						int nindex = y*width + x;
						if(after_labels_data[nindex] >= 0) adjlabel = after_labels_data[nindex];
					}
				}}

				int count(1);
				for( int c = 0; c < count; c++ )
				{
					for( int n = 0; n < 4; n++ )
					{
						int x = xvec[c] + dx4[n];
						int y = yvec[c] + dy4[n];

						if( (x >= 0 && x < width) && (y >= 0 && y < height) )
						{
							int nindex = y*width + x;

							if( 0 > after_labels_data[nindex] && before_labels.at(oindex) == before_labels.at(nindex) )
							{
								xvec[count] = x;
								yvec[count] = y;
								after_labels_data[nindex] = label;
								count++;
							}
						}

					}
				}
				//-------------------------------------------------------
				// If segment size is less then a limit, assign an
				// adjacent label found before, and decrement label count.
				//-------------------------------------------------------
				if(count <= SUPSZ >> 2)
				{
					for( int c = 0; c < count; c++ )
					{
						int ind = yvec[c]*width+xvec[c];
						after_labels_data[ind] = adjlabel;
					}
					label--;
				}
				label++;
			}
			oindex++;
		}
	}
	after_labels_num = label;

	if(xvec) delete [] xvec;
	if(yvec) delete [] yvec;
}
}