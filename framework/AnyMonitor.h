#ifndef _ANYMONITOR_H_
#define _ANYMONITOR_H_

#include "EagleeyeMacro.h"
#include <string>

namespace eagleeye
{
enum MonitorVarType
{
	EAGLEEYE_MONITOR_UNDEFINED = -1,
	EAGLEEYE_MONITOR_CHAR,
	EAGLEEYE_MONITOR_UCHAR,
	EAGLEEYE_MONITOR_SHORT,
	EAGLEEYE_MONITOR_USHORT,
	EAGLEEYE_MONITOR_INT,
	EAGLEEYE_MONITOR_UINT,
	EAGLEEYE_MONITOR_FLOAT,
	EAGLEEYE_MONITOR_DOUBLE,
	EAGLEEYE_MONITOR_RGB,
	EAGLEEYE_MONITOR_RGBA,
	EAGLEEYE_MONITOR_STR
};

template<class T>
class MonitorVarTrait
{
public:
	static const MonitorVarType var_type = EAGLEEYE_MONITOR_UNDEFINED;
};
template<>
class MonitorVarTrait<char>
{
public:
	static const MonitorVarType var_type = EAGLEEYE_MONITOR_CHAR;
};
template<>
class MonitorVarTrait<unsigned char>
{
public:
	static const MonitorVarType var_type = EAGLEEYE_MONITOR_UCHAR;
};
template<>
class MonitorVarTrait<short>
{
public:
	static const MonitorVarType var_type = EAGLEEYE_MONITOR_SHORT;
};
template<>
class MonitorVarTrait<int>
{
public:
	static const MonitorVarType var_type = EAGLEEYE_MONITOR_INT;
};
template<>
class MonitorVarTrait<unsigned int>
{
public:
	static const MonitorVarType var_type = EAGLEEYE_MONITOR_UINT;
};
template<>
class MonitorVarTrait<float>
{
public:
	static const MonitorVarType var_type = EAGLEEYE_MONITOR_FLOAT;
};
template<>
class MonitorVarTrait<double>
{
public:
	static const MonitorVarType var_type = EAGLEEYE_MONITOR_DOUBLE;
};
template<>
class MonitorVarTrait<std::string>
{
public:
	static const MonitorVarType var_type = EAGLEEYE_MONITOR_STR;
};

class AnyMonitor
{
public:
	AnyMonitor(MonitorVarType var_type,char* monitor_text = "NON",char* def_text = "")
		:monitor_var_type(var_type),
		monitor_var_text(monitor_text),
		def_var_text(def_text){}
	virtual ~AnyMonitor(){};
	virtual void setVar(const void* var)=0;
	virtual void getVar(void* var)=0;

	MonitorVarType monitor_var_type;
	char* monitor_var_text;
	char* def_var_text;
};

template<class HostT,class T>
class VarMonitor:public AnyMonitor
{
public:
	typedef void(HostT::*SetT)(const T);
	typedef void(HostT::*GetT)(T&);

	VarMonitor(HostT* host,SetT set_fun,GetT get_fun,char* text,char* def_text="")
		:AnyMonitor(MonitorVarTrait<T>::var_type,text,def_text),
		monitor_host(host),
		set_var_fun(set_fun),
		get_var_fun(get_fun){}
	virtual ~VarMonitor(){};

	virtual void setVar(const void* var)
	{
		const T* v=static_cast<const T*>(var);
		(monitor_host->*set_var_fun)(*v);
	}
	virtual void getVar(void* var)
	{
		T* v=(T*)var;
		(monitor_host->*get_var_fun)(*v);
	}
	
	SetT set_var_fun;
	GetT get_var_fun;

	HostT* monitor_host;
};

#define EAGLEEYE_MONITOR_VAR(VarT,set_fun,get_fun,text) \
	m_unit_monitor_pool.push_back(new VarMonitor<Self,VarT>(this,&Self::set_fun,&Self::get_fun,text));

#define EAGLEEYE_MONITOR_VAR_EXT(VarT,set_fun,get_fun,text,def_text) \
	m_unit_monitor_pool.push_back(new VarMonitor<Self,VarT>(this,&Self::set_fun,&Self::get_fun,text,def_text));
}

#endif