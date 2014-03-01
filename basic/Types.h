#ifndef _EAGLEEYE_TYPES_H_
#define _EAGLEEYE_TYPES_H_

#include "Array.h"

namespace eagleeye
{
/**
 *	@brief define Complex struct
 */
template<typename T>
struct Complex
{
	T rd;
	T id;
	
	Complex():rd(0),id(0){};
	Complex(T r,T i):rd(r),id(i){};

	T magnitude()
	{
		return sqrt(rd*rd+id*id);
	}

	double angle()
	{
		return double(id)/double(rd);
	}

	template<typename ScalarType>
	Complex operator/(const ScalarType& s) const
	{
		Complex t;
		t.rd=rd/(T)s;
		t.id=id/(T)s;

		return t;
	}

	template<typename ScalarType>
	Complex operator*(const ScalarType& s) const
	{
		Complex t;
		t.rd=rd*s;
		t.id=id*s;

		return t;
	}

	Complex operator*(const Complex& c) const
	{
		Complex t;
		t.rd=rd*c.rd-id*c.id;
		t.id=rd*c.id+id*c.rd;

		return t;
	}

	Complex operator+(const Complex& c) const
	{
		Complex t;
		t.rd=rd+c.rd;
		t.id=id+c.id;

		return t;
	}

	Complex operator-(const Complex& c) const
	{
		Complex t;
		t.rd=rd-c.rd;
		t.id=id-c.id;

		return t;
	}

	template<typename ScalarT>
	Complex& operator=(const ScalarT& c)
	{
		rd=c;
		id=0;
		
		return (*this);
	}
	
	Complex& operator=(const Complex& c)
	{
		rd=c.rd;
		id=c.id;
		return (*this);
	}

	Complex conjugate()
	{
		Complex t;
		t.rd=rd;
		t.id=-id;

		return t;
	}
};

struct PointPos
{
	float x;	/**< The col index*/
	float y;	/**< The row index*/
};

struct Gradient
{
	float x;				/**< The horizontal direction*/
	float y;				/**< The vertical direction*/
	float magnitude;		/**< The magnitude of gradient*/
};

typedef Array<float,2>					Point2f;
typedef Array<float,3>					Point3f;
typedef Array<double,2>					Point2d;
typedef Array<double,3>					Point3d;

typedef __int64 eagleeye_int64;
typedef unsigned __int64 eagleeye_uint64;
typedef unsigned char eagleeye_uchar;
typedef unsigned short eagleeye_ushort;
typedef signed char eagleeye_schar;

#define  EAGLEEYE_DECL_ALIGNED(x) __declspec(align(x))

typedef union Eagleeye32suf
{
	int i;
	unsigned u;
	float f;
}
Eagleeye32suf;

typedef union Eagleeye64suf
{
	eagleeye_int64 i;
	eagleeye_uint64 u;
	double f;
}
Eagleeye64suf;

struct Rect
{
	unsigned long left;
	unsigned long top;
	unsigned long right;
	unsigned long bottom;
};
}

#endif