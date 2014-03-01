#ifndef _OPERATIONS_HPP_
#define _OPERATIONS_HPP_
#include "EagleeyeMacro.h"
#include "Types.h"
#include <math.h>

namespace eagleeye
{
template<typename _Tp> static inline _Tp saturate_cast(eagleeye_uchar v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(eagleeye_schar v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(eagleeye_ushort v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(short v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(unsigned v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(int v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(float v) { return _Tp(v); }
template<typename _Tp> static inline _Tp saturate_cast(double v) { return _Tp(v); }

template<> inline eagleeye_uchar saturate_cast<eagleeye_uchar>(eagleeye_schar v)
{ return (eagleeye_uchar)EAGLEEYE_MAX((int)v, 0); }
template<> inline eagleeye_uchar saturate_cast<eagleeye_uchar>(eagleeye_ushort v)
{ return (eagleeye_uchar)EAGLEEYE_MIN((unsigned)v, (unsigned)UCHAR_MAX); }
template<> inline eagleeye_uchar saturate_cast<eagleeye_uchar>(int v)
{ return (eagleeye_uchar)((unsigned)v <= UCHAR_MAX ? v : v > 0 ? UCHAR_MAX : 0); }
template<> inline eagleeye_uchar saturate_cast<eagleeye_uchar>(short v)
{ return saturate_cast<eagleeye_uchar>((int)v); }
template<> inline eagleeye_uchar saturate_cast<eagleeye_uchar>(unsigned v)
{ return (eagleeye_uchar)EAGLEEYE_MIN(v, (unsigned)UCHAR_MAX); }
template<> inline eagleeye_uchar saturate_cast<eagleeye_uchar>(float v)
{ int iv = round(v); return saturate_cast<eagleeye_uchar>(iv); }
template<> inline eagleeye_uchar saturate_cast<eagleeye_uchar>(double v)
{ int iv = round(v); return saturate_cast<eagleeye_uchar>(iv); }

template<> inline eagleeye_schar saturate_cast<eagleeye_schar>(eagleeye_uchar v)
{ return (eagleeye_schar)EAGLEEYE_MIN((int)v, SCHAR_MAX); }
template<> inline eagleeye_schar saturate_cast<eagleeye_schar>(eagleeye_ushort v)
{ return (eagleeye_schar)EAGLEEYE_MIN((unsigned)v, (unsigned)SCHAR_MAX); }
template<> inline eagleeye_schar saturate_cast<eagleeye_schar>(int v)
{
	return (eagleeye_schar)((unsigned)(v-SCHAR_MIN) <= (unsigned)UCHAR_MAX ?
v : v > 0 ? SCHAR_MAX : SCHAR_MIN);
}
template<> inline eagleeye_schar saturate_cast<eagleeye_schar>(short v)
{ return saturate_cast<eagleeye_schar>((int)v); }
template<> inline eagleeye_schar saturate_cast<eagleeye_schar>(unsigned v)
{ return (eagleeye_schar)EAGLEEYE_MIN(v, (unsigned)SCHAR_MAX); }

template<> inline eagleeye_schar saturate_cast<eagleeye_schar>(float v)
{ int iv = round(v); return saturate_cast<eagleeye_schar>(iv); }
template<> inline eagleeye_schar saturate_cast<eagleeye_schar>(double v)
{ int iv = round(v); return saturate_cast<eagleeye_schar>(iv); }

template<> inline eagleeye_ushort saturate_cast<eagleeye_ushort>(eagleeye_schar v)
{ return (eagleeye_ushort)EAGLEEYE_MAX((int)v, 0); }
template<> inline eagleeye_ushort saturate_cast<eagleeye_ushort>(short v)
{ return (eagleeye_ushort)EAGLEEYE_MAX((int)v, 0); }
template<> inline eagleeye_ushort saturate_cast<eagleeye_ushort>(int v)
{ return (eagleeye_ushort)((unsigned)v <= (unsigned)USHRT_MAX ? v : v > 0 ? USHRT_MAX : 0); }
template<> inline eagleeye_ushort saturate_cast<eagleeye_ushort>(unsigned v)
{ return (eagleeye_ushort)EAGLEEYE_MIN(v, (unsigned)USHRT_MAX); }
template<> inline eagleeye_ushort saturate_cast<eagleeye_ushort>(float v)
{ int iv = round(v); return saturate_cast<eagleeye_ushort>(iv); }
template<> inline eagleeye_ushort saturate_cast<eagleeye_ushort>(double v)
{ int iv = round(v); return saturate_cast<eagleeye_ushort>(iv); }

template<> inline short saturate_cast<short>(eagleeye_ushort v)
{ return (short)EAGLEEYE_MIN((int)v, SHRT_MAX); }
template<> inline short saturate_cast<short>(int v)
{
	return (short)((unsigned)(v - SHRT_MIN) <= (unsigned)USHRT_MAX ?
v : v > 0 ? SHRT_MAX : SHRT_MIN);
}
template<> inline short saturate_cast<short>(unsigned v)
{ return (short)EAGLEEYE_MIN(v, (unsigned)SHRT_MAX); }
template<> inline short saturate_cast<short>(float v)
{ int iv = round(v); return saturate_cast<short>(iv); }
template<> inline short saturate_cast<short>(double v)
{ int iv = round(v); return saturate_cast<short>(iv); }

template<> inline int saturate_cast<int>(float v) { return round(v); }
template<> inline int saturate_cast<int>(double v) { return round(v); }

// we intentionally do not clip negative numbers, to make -1 become 0xffffffff etc.
template<> inline unsigned saturate_cast<unsigned>(float v){ return round(v); }
template<> inline unsigned saturate_cast<unsigned>(double v) { return round(v); }

inline int fast_abs(eagleeye_uchar v) { return v; }
inline int fast_abs(eagleeye_schar v) { return abs((int)v); }
inline int fast_abs(eagleeye_ushort v) { return v; }
inline int fast_abs(short v) { return abs((int)v); }
inline int fast_abs(int v) { return abs(v); }
inline float fast_abs(float v) { return abs(v); }
inline double fast_abs(double v) { return abs(v); }

}

#endif