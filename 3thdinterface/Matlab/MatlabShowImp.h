#ifndef _MATLABSHOWIMP_H_
#define _MATLABSHOWIMP_H_

#include "EagleeyeMacro.h"
#include <string>

//static���ܷŵ�������룬static���Ǳ�֤matlabΨһʵ���ı��룬
//����hpp���С���ôŪ��ȥ��matlab������

namespace eagleeye
{
class CMatlabEng;

class EAGLEEYE_API MatlabShowImp  
{
public:
	typedef MatlabShowImp			Self;
	typedef Self *					Pointer;
	typedef const Self *			ConstPointer;

	void imgShow(float *a, int height, int width, int step, const char *name = NULL);
	void rgbShow(const unsigned char* a, int height, int width, int step, const char *name =NULL);

	static Pointer New();
	static Pointer getInstance();
	static int initialize();

	MatlabShowImp();
	virtual ~MatlabShowImp();

private:
	static Pointer m_instance;
	static CMatlabEng *m_matlabeng;
};
}

#endif 
