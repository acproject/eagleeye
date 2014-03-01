#include "EagleeyeTimeStamp.h"
#include <Windows.h>

namespace eagleeye
{
/**
 * Instance creation.
 */
void EagleeyeTimeStamp::modified()
{
static LONG time_stamp_time = 0;
m_modified_time = (unsigned long)InterlockedIncrement(&time_stamp_time);
}
}
