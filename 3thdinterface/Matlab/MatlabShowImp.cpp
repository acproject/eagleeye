#include "MatlabShowImp.h"
#include "MatlabEng.h"
#include <sstream>

namespace eagleeye
{
MatlabShowImp::Pointer MatlabShowImp::m_instance = 0;
CMatlabEng * MatlabShowImp::m_matlabeng = 0;

MatlabShowImp::MatlabShowImp()
{
	m_matlabeng = 0;
}

MatlabShowImp::~MatlabShowImp()
{
	if (m_matlabeng) 
	{
		m_matlabeng->close();
		delete m_matlabeng;
	}
}

MatlabShowImp::Pointer MatlabShowImp::New()
{
	return getInstance();
}

MatlabShowImp::Pointer MatlabShowImp::getInstance()
{
	if (! MatlabShowImp::m_instance)
	{
		MatlabShowImp::m_instance = new MatlabShowImp;
	}
	return MatlabShowImp::m_instance;
}

void MatlabShowImp::imgShow(float *a, int height, int width, int step, const char *name)
{
#ifdef _USE_MATLAB_
	if (!a) return;
	if (!initialize()) return;

	char defName[] = "img";

	const char *arrName = defName;

	if (name)
	{
		arrName = name;
	}

	mxArray *T = NULL;

	//create matrix
	T = mxCreateDoubleMatrix(height,width, mxREAL);

	double *ptr = mxGetPr(T);
	for (int i=0; i<height; i++)
	{
		for (int j=0; j<width; j++)
		{
			ptr[j*height+i] = a[i*width+j];
		}
	}

	std::ostringstream code;
	//code << "imshow(" << arrName << "', [])";
	// code << "run d:\\debug\\test.m;";

	m_matlabeng->gutVariable(arrName, T);
	char buffer[1024];
	m_matlabeng->outputBuffer(buffer, 1024);
	buffer[1023] = 0;

	int res = m_matlabeng->evalString(code.str().c_str());
	printf("%s", buffer);
	mxDestroyArray(T);
#endif
}

void MatlabShowImp::rgbShow(const unsigned char* a, int height, int width, int step, const char *name /* =NULL */)
{
#ifdef _USE_MATLAB_
	if (!a) return;
	if (!initialize()) return;

	char defName[] = "img";

	const char *arrName = defName;

	if (name)
	{
		arrName = name;
	}

	//create 3d array
	const mwSize dims[3]={height,width,3};
	mxArray* T = mxCreateNumericArray(3, dims, mxUINT8_CLASS, mxREAL);
	unsigned char* ptr=(unsigned char*)mxGetData(T);
	
	int slice_offset = height * width;
	
	for (int i = 0; i < height; ++i)
	{
		const unsigned char* a_data = a + step * i;
		for (int j = 0; j < width; ++j)
		{
			*(ptr+slice_offset*0+j*height+i)=a_data[j*3];		//red
			*(ptr+slice_offset*1+j*height+i)=a_data[j*3+1];		//green
			*(ptr+slice_offset*2+j*height+i)=a_data[j*3+2];		//blue
		}
	}

	std::ostringstream code;
	//code << "imshow(" << arrName << "', [])";
	// code << "run d:\\debug\\test.m;";

	m_matlabeng->gutVariable(arrName, T);
	char buffer[1024];
	m_matlabeng->outputBuffer(buffer, 1024);
	buffer[1023] = 0;

	int res = m_matlabeng->evalString(code.str().c_str());
	printf("%s", buffer);
	mxDestroyArray(T);
#endif
}

int MatlabShowImp::initialize()
{
	if (MatlabShowImp::m_matlabeng)
	{
		return 1;
	}
	MatlabShowImp::m_matlabeng = new CMatlabEng(NULL);
	MatlabShowImp::m_matlabeng->setVisible(true);

	return 1;
}
}
