/**  @file   
 * @brief  vector容器中的数据基本运算   
 * @author  liqiang
 * @date  2013.9.12
 * @version  1.0 
 * @note   
 * detailed  description  
 */ 
#ifndef _VECTORMATH_H_
#define _VECTORMATH_H_

#include "EagleEyeMacro.h"
#include <vector>

namespace eagleeye
{
template<typename T>
float mean( const std::vector<T>& m, int start_index=0, int end_index=0 )
{
	if( 0 == m.size() )
		return 0.0f;

	if( 0 == end_index )
		end_index = m.size()-1;
	T sum = 0;
	for( int i=start_index; i<=end_index; ++i )
	{
		sum += m[i];
	}
	float avg = (float)sum/(float)(end_index-start_index+1);
	return avg;
}

template<typename T>
T median( std::vector<T>& m, int start_index=0, int end_index=0 )
{
	if( 0 == m.size() )
		return (T)0;
	if( 0 == end_index )
		end_index = m.size();
	std::vector<T> m_copy;
	m_copy.assign( m.begin()+start_index, m.begin()+end_index );

	T res_val;
	if( 0== (m_copy.size()%2) )
	{
		int loc1 = m_copy.size()/2;
		int loc2 = loc1-1;
		std::nth_element(  m_copy.begin(), m_copy.begin()+loc1, m_copy.end() );
		T val1 = m_copy[loc1];
		std::nth_element(  m_copy.begin(), m_copy.begin()+loc2, m_copy.end() );
		T val2 = m_copy[loc2];
		res_val = (val1+val2) / 2;
	}
	else
	{
		int loc = m_copy.size()/2;
		std::nth_element( m_copy.begin(), m_copy.begin()+loc, m_copy.end(), std::greater<T>() );
		res_val = m_copy[loc];
	}
	return res_val;
}

template<typename T>
T max_val( std::vector<T>& m, int start_index=0, int end_index=0 )
{
	if( 0 == end_index )
		end_index = m.size();
	std::vector<T>::iterator it = max_element( m.begin()+start_index, m.begin()+end_index );

	return *it;
}

template<typename T>
T min_val( std::vector<T>& m, int start_index=0, int end_index=0 )
{
	if( 0 == end_index )
		end_index = m.size();
	std::vector<T>::iterator it = min_element( m.begin()+start_index, m.begin()+end_index );
	return *it;
}
}

#endif