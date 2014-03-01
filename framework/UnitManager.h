#ifndef _UNITMANAGER_H_
#define _UNITMANAGER_H_

#include "EagleeyeMacro.h"
#include "AnyUnit.h"

namespace eagleeye
{
class UnitManager
{
public:
	UnitManager();
	~UnitManager();

	static AnyUnit* factory(const char* unit_id,const char* ext_info = "");
};
}

#endif