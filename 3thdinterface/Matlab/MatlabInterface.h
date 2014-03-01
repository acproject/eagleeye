#ifndef _MATLABINTERFACE_H_
#define _MATLABINTERFACE_H_

#include "EagleeyeMacro.h"
#include "TraitCenter.h"
#include "Array.h"

namespace eagleeye
{
template<typename T>
inline void putToMatlab( const Matrix<T>& img, const char* name = "A" );

template<>
inline void putToMatlab<ERGB>( const Matrix<ERGB>& img, const char* name );

}

#include "MatlabInterface.hpp"
#endif