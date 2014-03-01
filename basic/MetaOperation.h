#ifndef _METAOPERATION_H_
#define _METAOPERATION_H_

#include "EagleeyeMacro.h"
#include "TraitCenter.h"

namespace eagleeye
{
template<typename MetaSrcT,typename MetaTargetT>
class NormalizeOperation
{
public:
	typedef MetaSrcT							MetaSrcType;
	typedef MetaTargetT							MetaTargetType;
	
	NormalizeOperation(MetaSrcT src_min_val=MetaSrcType(0),MetaSrcT src_max_val=MetaSrcType(1),
		MetaTargetT target_min_val=MetaTargetType(0),MetaTargetT target_max_val=MetaTargetType(1))
	{
		src_min_meta_val = double(src_min_val);
		src_max_meta_val = double(src_max_val);
		save_time_src_var = src_max_meta_val - src_min_meta_val;

		target_min_meta_val = double(target_min_val);
		target_max_meta_val = double(target_max_val);
		save_time_target_var = target_max_meta_val - target_min_meta_val;		
	};
	inline void operator()(const MetaSrcT& meta_val,MetaTargetType& target_meta_val)
	{
		if ( double(meta_val) < src_min_meta_val )
		{
			target_meta_val = MetaTargetT(target_min_meta_val);
			return;
		}
		else if ( double(meta_val) > src_max_meta_val )
		{
			target_meta_val = MetaTargetT(target_max_meta_val);
			return;
		}

		target_meta_val = MetaTargetT(((double(meta_val) - src_min_meta_val) / save_time_src_var) * save_time_target_var + target_min_meta_val);
		return;
	}

private:
	double src_min_meta_val;
	double src_max_meta_val;
	double save_time_src_var;

	double target_min_meta_val;
	double target_max_meta_val;
	double save_time_target_var;
};

template<typename MetaSrcT, typename MetaTargetT>
class ThresholdOperation
{
public:
	typedef MetaSrcT							MetaSrcType;
	typedef MetaTargetT							MetaTargetType;

	ThresholdOperation(MetaSrcT threadshold = MetaSrcT( 0 )):meta_threshold(threadshold){};
	
	inline void operator()(const MetaSrcT& meta_val,MetaTargetT& target_meta_val)
	{
		if ( meta_val > meta_threshold )
		{
			target_meta_val = MetaTargetT( 1 );
			return;
		}
		else
		{	
			target_meta_val = MetaTargetT( 0 );
			return;
		}	
	}

	MetaSrcT meta_threshold;
};

template<typename MetaSrcT,typename MetaTargetT,int s_one,int s_another>
class SwitchOperations
{
public:
	typedef MetaSrcT							MetaSrcType;
	typedef MetaTargetT							MetaTargetType;

	typedef typename MetaSrcT::ElemType			MetaSrcElemType;
	typedef typename MetaTargetT::ElemType		MetaTargetElemType;

	SwitchOperations(){};

	inline void operator ()(const MetaSrcT& src_meta_val, MetaTargetT& target_meta_val)
	{
		target_meta_val = src_meta_val;
		memcpy(&target_meta_val,&src_meta_val,EAGLEEYE_MIN(sizeof(MetaSrcT),sizeof(MetaTargetT)));

		MetaSrcElemType temp = src_meta_val[s_one];
		target_meta_val[s_one] = src_meta_val[s_another];
		target_meta_val[s_another] = (MetaTargetElemType)temp;
	}
};

template<typename MetaSrcT,typename MetaTargetT>
class AverageOperations
{
public:
	typedef MetaSrcT							MetaSrcType;
	typedef MetaTargetT							MetaTargetType;

	inline void operator()(const MetaSrcT& src_meta_val,MetaTargetT& target_meta_val)
	{
		target_meta_val = 0;
		for (int i = 0; i < AtomicTypeTrait<MetaSrcType>::size; ++i)
		{
			target_meta_val += MetaTargetType(OperateTrait<MetaSrcType>::unit(src_meta_val,i));
		}

		target_meta_val = target_meta_val / AtomicTypeTrait<MetaSrcType>::size;
	}
};

//////////////////////////////////////////////////////////////////////////
template<class ValT>
class GreaterThan
{
public:
	GreaterThan(ValT val = ValT(0)):threshold_val(val){};
	~GreaterThan(){};

	inline bool operator()(const ValT& cur_val)
	{
		if (cur_val > threshold_val)
			return true;
		else
			return false;
	}

private:
	ValT threshold_val;
};

template<class ValT>
class LessThan
{
public:
	LessThan(ValT val = ValT(0)):threshold_val(val){};
	~LessThan(){};

	inline bool operator()(const ValT& cur_val)
	{
		if (cur_val > threshold_val)
			return true;
		else
			return false;
	}

private:
	ValT threshold_val;
};

}
#endif