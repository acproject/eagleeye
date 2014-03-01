#include <stdio.h>
#include "MatlabEng.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace eagleeye
{
CMatlabEng::CMatlabEng()
{
	m_matlab_engine=NULL;
}

CMatlabEng::~CMatlabEng()
{
#ifdef _USE_MATLAB_
	if (m_matlab_engine!=NULL)
		close();
#endif
}

CMatlabEng::CMatlabEng(const char* StartCmd)
{
	open(StartCmd);
}


void CMatlabEng::open(const char *StartCmd)
{
#ifdef _USE_MATLAB_
	m_matlab_engine=engOpen(StartCmd);
#endif
}

int CMatlabEng::close()
{
#ifdef _USE_MATLAB_
	int Result=engClose(m_matlab_engine);
	if (Result==0)	//Success
		m_matlab_engine=NULL;

	return Result;
#else
	return 0;
#endif
}

int CMatlabEng::evalString(const char *string)
{
#ifdef _USE_MATLAB_
	return (engEvalString(m_matlab_engine, string));
#else
	return 0;
#endif
}


mxArray* CMatlabEng::getVariable(const char *name)
{
#ifdef _USE_MATLAB_
	return (engGetVariable(m_matlab_engine, name));
#else
	return 0;
#endif
}



int CMatlabEng::getVisible(bool* value)
{
#ifdef _USE_MATLAB_
	return (engGetVisible(m_matlab_engine, value));
#else
	return 0;
#endif
}

void CMatlabEng::openSingleUse(const char *startcmd, void *dcom, int *retstatus)
{
#ifdef _USE_MATLAB_
	m_matlab_engine=engOpenSingleUse(startcmd, dcom, retstatus);
#endif
}

int CMatlabEng::outputBuffer(char *p, int n)
{
#ifdef _USE_MATLAB_
	return (engOutputBuffer(m_matlab_engine, p, n));
#else
	return 0;
#endif
}

int CMatlabEng::gutVariable(const char *name, const mxArray *mp)
{
#ifdef _USE_MATLAB_
	return (engPutVariable(m_matlab_engine, name, mp));
#else
	return 0;
#endif
}

int CMatlabEng::setVisible(bool value)
{
#ifdef _USE_MATLAB_
	return (engSetVisible(m_matlab_engine, value));
#else
	return 0;
#endif
}
}
