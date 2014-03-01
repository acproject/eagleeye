#ifndef _ARRAY_H_
#define _ARRAY_H_
#include <iostream>
namespace eagleeye
{
/**
 *	@brief Array struct with fixed type and dimension.
 *	@note Array with fixed type and dimension could be
 *	matrix element
 */
template<class T,int D>
class Array
{
public:
	typedef T								ElemType;

	/**
	 *	@brief All kinds of useful constructors
	 */
	Array()
	{
		memset(data,0,sizeof(T) * D);
	}
	Array(const Array& m)
	{
		memcpy(data,m.data,sizeof(T) * D);
	}
	Array(T val)
	{
		for (int i = 0; i < D; ++i)
		{
			data[i] = val;
		}
	}
	~Array(){};

	/**
	 *	@brief some cute members
	 */
	void copy(T* d)
	{
		memcpy(data,d,sizeof(T) * D);
	}

	T& operator[](int index);
	const T& operator[](int index) const;
	
	T& operator()(int index);
	const T& operator()(int index) const;

	Array& operator=(const Array& m);

	Array operator+(const Array& m);
	Array operator-(const Array& m);
	Array operator*(const Array& m);
	Array operator/(const Array& m);

	bool operator<(const Array& m);
	bool operator>(const Array& m);
	bool operator<=(const Array& m);
	bool operator>=(const Array& m);
	bool operator==(const Array& m);
	bool operator!=(const Array& m);

	template<typename ScalarT>
	Array& operator=(const ScalarT c)
	{
		for (int i = 0; i < D; ++i)
		{
			data[i] = (T)c;
		}

		return (*this);
	}

	template<typename ScalarT>
	Array operator+(const ScalarT c)
	{
		Array<T,D> another;
		for (int i = 0; i < D; ++i)
		{
			another.data[i] = data[i] + (T)c;
		}
		
		return another;
	}

	template<typename ScalarT>
	Array operator-(const ScalarT c)
	{
		Array<T,D> another;
		for (int i = 0; i < D; ++i)
		{
			another.data[i] = data[i] - (T)c;
		}

		return another;
	}

	template<typename ScalarT>
	Array operator*(const ScalarT c)
	{
		Array<T,D> another;
		for (int i = 0; i < D; ++i)
		{
			another.data[i] = data[i] * (T)c;
		}
		return another;
	}

	template<typename ScalarT>
	Array operator/(const ScalarT c)
	{
		Array<T,D> another;
		for (int i = 0; i < D; ++i)
		{
			another.data[i] = data[i] / (T)c;
		}

		return another;
	}

	T data[D];
};

//Array implement
template<typename T,int D>
T& Array<T,D>::operator[](int index)
{
	return data[index];
}

template<typename T,int D>
const T& Array<T,D>::operator[](int index) const
{
	return data[index];
}

template<typename T,int D>
T& Array<T,D>::operator()(int index)
{
	return data[index];
}

template<typename T,int D>
const T& Array<T,D>::operator()(int index) const
{
	return data[index];
}

template<typename T,int D>
Array<T,D>& Array<T,D>::operator=(const Array& m)
{
	memcpy(data,m.data,sizeof(T) * D);
	return *this;
}

template<typename T,int D>
Array<T,D> Array<T,D>::operator+(const Array& m)
{
	Array<T,D> another;
	for (int i = 0; i < D; ++i)
	{
		another.data[i] = data[i] + m.data[i];
	}

	return another;
}

template<typename T,int D>
Array<T,D> Array<T,D>::operator-(const Array& m)
{
	Array<T,D> another;
	for (int i = 0; i < D; ++i)
	{
		another.data[i] = data[i] - m.data[i];
	}

	return another;
}

template<typename T,int D>
Array<T,D> Array<T,D>::operator*(const Array& m)
{
	Array<T,D> another;
	for (int i = 0; i < D; ++i)
	{
		another.data[i] = data[i] * m.data[i];
	}

	return another;
}

template<typename T,int D>
Array<T,D> Array<T,D>::operator/(const Array& m)
{
	Array<T,D> another;
	for (int i = 0; i < D; ++i)
	{
		another.data[i] = data[i] / m.data[i];
	}

	return another;
}

template<typename T,int D>
bool Array<T,D>::operator<(const Array& m)
{
	for (int i = 0; i < D; ++i)
	{
		if (data[i] > m.data[i])
		{
			return false;
		}
	}
	return true;
}

template<typename T,int D>
bool Array<T,D>::operator>(const Array& m)
{
	for (int i = 0; i < D; ++i)
	{
		if (data[i] < m.data[i])
		{
			return false;
		}
	}
	return true;
}

template<typename T,int D>
bool Array<T,D>::operator<=(const Array& m)
{
	for (int i = 0; i < D; ++i)
	{
		if (data[i] > m.data[i])
		{
			return false;
		}
	}

	return true;
}

template<typename T,int D>
bool Array<T,D>::operator>=(const Array& m)
{
	for (int i = 0; i < D; ++i)
	{
		if (data[i] < m.data[i])
		{
			return false;
		}
	}

	return true;
}

template<typename T,int D>
bool Array<T,D>::operator==(const Array& m)
{
	for (int i = 0; i < D; ++i)
	{
		if (data[i] != m.data[i])
		{
			return false;
		}
	}

	return true;
}

template<typename T,int D>
bool Array<T,D>::operator !=(const Array& m)
{
	for (int i = 0; i < D; ++i)
	{
		if (data[i] != m.data[i])
		{
			return true;
		}
	}

	return false;
}

//overloaded output operator for Array
template<typename T,int D>
std::ostream& operator<<(std::ostream& os,const Array<T,D>& ob)
{
	for (int i = 0; i < D; ++i)
	{
		os<<ob[i]<<'\t';
	}
	return os;
}
template<typename T,int D>
std::istream& operator>>(std::istream& in,Array<T,D>& ob)
{
	for (int i = 0; i < D; ++i)
	{
		in>>ob[i];
	}
		
	if (!in)
	{
		ob = Array<T,D>();
	}
	return in;
}

/**
 *	@brief some useful type
 */
typedef Array<float,32>					HOG32Vec;
typedef Array<unsigned char,3>			ERGB;
typedef Array<unsigned char,4>			ERGBA;

}

#endif