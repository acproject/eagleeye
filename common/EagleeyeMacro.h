#ifndef _EAGLEEYEMACRO_H_
#define _EAGLEEYEMACRO_H_

#ifdef EAGLEEYE_EXPORTS
#define EAGLEEYE_API __declspec(dllexport)
#else
#define EAGLEEYE_API __declspec(dllimport)
#endif

namespace eagleeye
{
#ifndef eagleeye_eps
#define eagleeye_eps  2.2204e-16
#endif

#ifndef EAGLEEYE_PI
#define EAGLEEYE_PI 3.1415926f
#endif

#ifndef EAGLEEYE_FINF
#define EAGLEEYE_FINF 1.0e38f
#endif

#ifndef EAGLEEYE_NEAR_INF
#define EAGLEEYE_NEAR_INF 1.0e37f
#endif

#ifndef EAGLEEYE_MAX
#define EAGLEEYE_MAX(a,b) ((a)>(b))?(a):(b)
#endif

#ifndef EAGLEEYE_MIN
#define EAGLEEYE_MIN(a,b) ((a)<(b))?(a):(b)
#endif

#ifndef eagleeye_max
#define eagleeye_max(a,b) ((a)>(b))?(a):(b)
#endif

#ifndef eagleeye_min
#define eagleeye_min(a,b) ((a)<(b))?(a):(b)
#endif

#ifndef EAGLEEYE_MAX_PATH
#define EAGLEEYE_MAX_PATH          260
#endif

#ifndef EAGLEEYE_MAX_NAME
#define EAGLEEYE_MAX_NAME			100
#endif

#ifndef EAGLEEYE_MAX_BLOCK
#define EAGLEEYE_MAX_BLOCK			400
#endif

#ifndef EAGLEEYE_MAX_STR
#define EAGLEEYE_MAX_STR			500
#endif

#ifndef EAGLEEYE_BUFFER_SIZE
#define EAGLEEYE_BUFFER_SIZE		1024
#endif

#define bzero(a,b) memset(a,0,b)

enum SampleState
{
	EAGLEEYE_UNDEFINED_SAMPLE = -1	,
	EAGLEEYE_POSITIVE_SAMPLE  = 0	,
	EAGLEEYE_NEGATIVE_SAMPLE  = 1
};

#define EAGLEEYE_CLASSIDENTITY(NAME) \
	virtual const char* getClassIdentity() const \
	{ \
		return #NAME; \
	}
#define EAGLEEYE_GETUNITNAME \
	virtual const char* getUnitName() const \
	{ \
		return m_unit_name.c_str(); \
	}

#define EAGLEEYE_SETUNITNAME(NAME) \
	virtual void setUnitName(const char* unit_name=#NAME) \
	{ \
		m_unit_name=unit_name; \
	}

#define EAGLEEYE_OUTPUT_PORT_TYPE(TYPE,INDEX,NAME) \
	typedef TYPE OutputPort_##NAME##_Type; \
	const static int OUTPUT_PORT_##NAME = INDEX; \
	void enableOutputPort_##NAME() \
	{ \
		m_output_port_state[OUTPUT_PORT_##NAME] = true; \
	} \
	void disableOutputPort_##NAME() \
	{ \
		m_output_port_state[OUTPUT_PORT_##NAME] = false; \
	}

#define EAGLEEYE_INPUT_PORT_TYPE(TYPE,INDEX,NAME) \
	typedef TYPE InputPort_##NAME##_Type; \
	const static int INPUT_PORT_##NAME = INDEX;

#define ELEM(L, r, c, col) (L[(r) * (col) + c])  

enum EagleeyePixel
{
	EAGLEEYE_UNDEFINED =-1,
	EAGLEEYE_CHAR,
	EAGLEEYE_UCHAR,
	EAGLEEYE_SHORT,
	EAGLEEYE_USHORT,
	EAGLEEYE_INT,
	EAGLEEYE_UINT,
	EAGLEEYE_FLOAT,
	EAGLEEYE_DOUBLE,
	EAGLEEYE_RGB,
	EAGLEEYE_RGBA
};

enum EagleeyeError
{
	EAGLEEYE_ARG_ERROR			=-1,// parameter error
	EAGLEEYE_FILE_ERROR			=-2,// file io error
	EAGLEEYE_MEM_ERROR			=-3,// memory operation error
	EAGLEEYE_TABLE_ERROR		=-4,// table error
	EAGLEEYE_UNKNOWN_ERROR		=-5,//	unknown error
	EAGLEEYE_NO_ERROR			=0	//  no error
};

#pragma warning( disable: 4251 )
#pragma warning( disable: 4996 )
}

#endif