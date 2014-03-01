#ifdef CORE_BUILD_DLL
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif

#ifndef _COREMACRO_H_
#define _COREMACRO_H_
#include "boost/smart_ptr/shared_array.hpp"

namespace core
{
#define CoreSmartPointer boost::shared_ptr
}

#ifndef CMAX
#define CMAX(a,b) ((a)>(b))?(a):(b)
#endif

#ifndef CMIN
#define CMIN(a,b) ((a)<(b))?(a):(b)
#endif

#endif
