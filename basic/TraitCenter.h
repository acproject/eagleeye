#ifndef _OPERATETRAIT_H_
#define _OPERATETRAIT_H_
#include "EagleeyeMacro.h"
#include "Array.h"
#include <limits.h>
#include <float.h>

namespace eagleeye
{
/**
 *	@brief trait atomic type
 */
template<typename T>
class AtomicTypeTrait
{
public:
	typedef typename T::ElemType	AtomicType;
	static const int size			= sizeof(T) / sizeof(AtomicType);

	static inline T minval()
	{
		T data;
		for (int i = 0; i < size; ++i)
		{	
			data[i] = AtomicTypeTrait<AtomicType>::minval();
		}

		return data;
	}
	static inline T maxval()
	{
		T data;
		for (int i = 0; i < size; ++i)
		{	
			data[i] = AtomicTypeTrait<AtomicType>::maxval();
		}

		return data;
	}

	static const EagleeyePixel pixel_type = EAGLEEYE_UNDEFINED;
};

template<>
class AtomicTypeTrait<char>
{
public:
	typedef char					AtomicType;
	static const int size			= 1;

	static inline AtomicType minval()
	{
		return CHAR_MIN;
	}
	static inline AtomicType maxval()
	{
		return CHAR_MAX;
	}

	static const EagleeyePixel pixel_type = EAGLEEYE_CHAR;
};

template<>
class AtomicTypeTrait<unsigned char>
{
public:
	typedef unsigned char			AtomicType;
	static const int size			= 1;

	static inline AtomicType minval()
	{
		return 0;
	}
	static inline AtomicType maxval()
	{
		return UCHAR_MAX;
	}

	static const EagleeyePixel pixel_type = EAGLEEYE_UCHAR;
};

template<>
class AtomicTypeTrait<short>
{
public:
	typedef short					AtomicType;
	static const int size			= 1;

	static inline AtomicType minval()
	{
		return SHRT_MIN;
	}
	static inline AtomicType maxval()
	{
		return SHRT_MAX;
	}

	static const EagleeyePixel pixel_type=EAGLEEYE_SHORT;
};

template<>
class AtomicTypeTrait<unsigned short>
{
public:
	typedef unsigned short			AtomicType;
	static const int size			= 1;

	static inline AtomicType minval()
	{
		return 0;
	}
	static inline AtomicType maxval()
	{
		return USHRT_MAX;
	}

	static const EagleeyePixel pixel_type = EAGLEEYE_USHORT;
};

template<>
class AtomicTypeTrait<int>
{
public:
	typedef int						AtomicType;
	static const int size			= 1;

	static inline AtomicType minval()
	{
		return INT_MIN;
	}
	static inline AtomicType maxval()
	{
		return INT_MAX;
	}

	static const EagleeyePixel pixel_type = EAGLEEYE_INT;
};

template<>
class AtomicTypeTrait<unsigned int>
{
public:
	typedef unsigned int			AtomicType;
	static const int size			= 1;

	static inline AtomicType minval()
	{
		return 0;
	}
	static inline AtomicType maxval()
	{
		return UINT_MAX;
	}

	static const EagleeyePixel pixel_type = EAGLEEYE_UINT;
};

template<>
class AtomicTypeTrait<float>
{
public:
	typedef float					AtomicType;
	static const int size			= 1;

	static inline AtomicType minval()
	{
		return FLT_MIN;
	}
	static inline AtomicType maxval()
	{
		return FLT_MAX;
	}

	static const EagleeyePixel pixel_type = EAGLEEYE_FLOAT;
};

template<>
class AtomicTypeTrait<double>
{
public:
	typedef double					AtomicType;
	static const int size			= 1;

	static inline AtomicType minval()
	{
		return DBL_MIN;
	}
	static inline AtomicType maxval()
	{
		return DBL_MAX;
	}

	static const EagleeyePixel pixel_type = EAGLEEYE_DOUBLE;
};

template<>
class AtomicTypeTrait<ERGB>
{
public:
	typedef unsigned char			AtomicType;
	static const int size			= 3;

	static inline ERGB minval()
	{
		ERGB data;
		data[0] = 0; data[1] = 0; data[2] = 0; data[3] = 0;
		return data;
	}
	static inline ERGB maxval()
	{
		ERGB data;
		data[0] = 255; data[1] = 255; data[2] = 255; data[3] = 255;
		return data;
	}

	static const EagleeyePixel pixel_type = EAGLEEYE_RGB;
};

template<>
class AtomicTypeTrait<ERGBA>
{
public:
	typedef unsigned char			AtomicType;
	static const int size			= 3;

	static inline ERGBA minval()
	{
		ERGBA data;
		data[0] = 0; data[1] = 0; data[2] = 0; data[3] = 0;
		return data;
	}
	static inline ERGBA maxval()
	{
		ERGBA data;
		data[0] = 255; data[1] = 255; data[2] = 255; data[3] = 255;
		return data;
	}

	static const EagleeyePixel pixel_type = EAGLEEYE_RGBA;
};

template<typename T>
class OperateTrait
{
public:
	typedef typename T::ElemType AtomicType;

	static inline AtomicType& unit(T& val,int index = 0)
	{
		return val[index];
	}
	static inline const AtomicType& unit(const T& val,int index = 0)
	{
		return val[index];
	}

	static inline AtomicType maxunit(const T& val)
	{
		AtomicType max_atomic_val = NumericTraits<T>::minval();
		for (int i = 0; i < NumericTraits<T>::size; ++i)
		{
			if (val[i] > max_atomic_val)
			{
				max_atomic_val = val[i];
			}
		}

		return max_atomic_val;
	}

	static inline AtomicType minunit(const T& val)
	{
		AtomicType min_atomic_val = NumericTraits<T>::maxval();
		for (int i = 0; i < NumericTraits<T>::size; ++i)
		{
			if (val[i] < min_atomic_val)
			{
				min_atomic_val = val[i];
			}
		}

		return min_atomic_val;
	}

	static inline AtomicType square(const T& val)
	{
		T square_val(0);
		for (int i = 0; i < NumericTraits<T>::size; ++i)
		{
			square_val[i] = val[i] * val[i];
		}
		return square_val;
	}
};

template<>
class OperateTrait<char>
{
public:
	typedef char							AtomicType;

	static inline AtomicType& unit(char& val,int index = 0)
	{
		return val;
	}
	static inline const AtomicType& unit(const char& val,int index = 0)
	{
		return val;
	}

	static inline AtomicType maxunit(const char& val)
	{
		return val;
	}
	static inline AtomicType minunit(const char& val)
	{
		return val;
	}
	static inline AtomicType square(const char& val)
	{
		return AtomicType(val * val);
	}

};

template<>
class OperateTrait<unsigned char>
{
public:
	typedef unsigned char						AtomicType;

	static inline AtomicType& unit(unsigned char& val,int index = 0)
	{
		return val;
	}
	static inline const AtomicType& unit(const unsigned char& val,int index = 0)
	{
		return val;
	}

	static inline AtomicType maxunit(const unsigned char& val)
	{
		return val;
	}
	static inline AtomicType minunit(const unsigned char& val)
	{
		return val;
	}
	static inline AtomicType square(const unsigned char& val)
	{
		return AtomicType(val * val);
	}
};

template<>
class OperateTrait<short>
{
public:
	typedef short								AtomicType;

	static inline AtomicType& unit(short& val,int index = 0)
	{
		return val;
	}
	static inline const AtomicType& unit(const short& val,int index = 0)
	{
		return val;
	}

	static inline AtomicType maxunit(const short& val)
	{
		return val;
	}
	static inline AtomicType minunit(const short& val)
	{
		return val;
	}
	static inline AtomicType square(const short& val)
	{
		return AtomicType(val * val);
	}
};

template<>
class OperateTrait<unsigned short>
{
public:
	typedef unsigned short						AtomicType;

	static inline AtomicType& unit(unsigned short& val,int index = 0)
	{
		return val;
	}
	static inline const AtomicType& unit(const unsigned short& val,int index = 0)
	{
		return val;
	}

	static inline AtomicType maxunit(const unsigned short& val)
	{
		return val;
	}
	static inline AtomicType minunit(const unsigned short& val)
	{
		return val;
	}
	static inline AtomicType square(const unsigned short& val)
	{
		return AtomicType(val * val);
	}
};

template<>
class OperateTrait<int>
{
public:
	typedef int									AtomicType;

	static inline AtomicType& unit(int& val,int index = 0)
	{
		return val;
	}
	static inline const AtomicType& unit(const int& val,int index = 0)
	{
		return val;
	}

	static inline AtomicType maxunit(const int& val)
	{
		return val;
	}
	static inline AtomicType minunit(const int& val)
	{
		return val;
	}
	static inline AtomicType square(const int& val)
	{
		return AtomicType(val * val);
	}
};

template<>
class OperateTrait<unsigned int>
{
public:
	typedef unsigned int						AtomicType;

	static inline AtomicType& unit(unsigned int& val,int index = 0)
	{
		return val;
	}
	static inline const AtomicType& unit(const unsigned int& val,int index = 0)
	{
		return val;
	}

	static inline AtomicType maxunit(const unsigned int& val)
	{
		return val;
	}
	static inline AtomicType minunit(const unsigned int& val)
	{
		return val;
	}
	static inline AtomicType square(const unsigned int& val)
	{
		return AtomicType(val * val);
	}
};

template<>
class OperateTrait<float>
{
public:
	typedef float								AtomicType;

	static inline AtomicType& unit(float& val,int index = 0)
	{
		return val;
	}
	static inline const AtomicType& unit(const float& val,int index = 0)
	{
		return val;
	}

	static inline AtomicType maxunit(const float& val)
	{
		return val;
	}
	static inline AtomicType minunit(const float& val)
	{
		return val;
	}
	static inline AtomicType square(const float& val)
	{
		return AtomicType(val * val);
	}
};

template<>
class OperateTrait<double>
{
public:
	typedef double								AtomicType;

	static inline AtomicType& unit(double& val,int index = 0)
	{
		return val;
	}
	static inline const AtomicType& unit(const double& val,int index = 0)
	{
		return val;
	}

	static inline AtomicType maxunit(const double& val)
	{
		return val;
	}
	static inline AtomicType minunit(const double& val)
	{
		return val;
	}
	static inline AtomicType square(const double& val)
	{
		return AtomicType(val * val);
	}
};

}
#endif