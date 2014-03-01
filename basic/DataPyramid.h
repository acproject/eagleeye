#ifndef _IMAGEPYRAMID_H_
#define _IMAGEPYRAMID_H_
#include "Matrix.h"
#include "Types.h"
#include "Array.h"
#include <vector>

namespace eagleeye
{
/**
 *	@brief define data pyramid struct
 *	@note It's not used here. It would be completed in the future.
 */
template<class T,int L>
struct DataPyramid
{
	static const int Levels = L;
	DataPyramid():padx(0),pady(0){};
	Matrix<T>& operator[](int index)
	{
		return data[index];
	}
	
	void operator=(const DataPyramid& p)
	{
		for (int i = 0; i < L; ++i)
		{
			data[i] = p.data[i];
		}
	}

	float scale[L];
	Matrix<T> data[L];
	int padx;
	int pady;
};

/**
 *	@brief Define Image Pyramid(It's very easy and convenient)
 */
template<class T>
class DynamicDataPyramid
{
public:
	DynamicDataPyramid():padx(0),pady(0),interval(0){};
	virtual ~DynamicDataPyramid(){};
	DynamicDataPyramid& create(int size)
	{
		m_data.resize(size);
		m_scale.resize(size,0);
		m_flag.resize(size,0);
		return *this;
	}

	/**
	 *	@brief Get the level number of this pyramid
	 */
	int levels() const
	{
		return m_data.size();
	}

	/**
	 *	@brief Get the 'index' level scale of this pyramid
	 */
	float& scales(int index)
	{
		return m_scale[index];
	}
	const float& scales(int index) const 
	{
		return m_scale[index];
	}

	/**
	 *	@brief Get all levels scale of this pyramid
	 */
	std::vector<float> scales() const
	{
		return m_scale;
	}
	
	/**
	 *	@brief Get the level flag of this pyramid
	 */
	int& flags(int index)
	{
		return m_flag[index];
	}
	const int& flags(int index) const
	{
		return m_flag[index];
	}

	std::vector<int> flags() const
	{
		return m_flag;
	}

	/**
	 *	@brief Some useful overload operators
	 */
	Matrix<T>& operator[](int index)
	{
		return m_data[index];
	}

	const Matrix<T>& operator[](int index) const
	{
		return m_data[index];
	}
	
	DynamicDataPyramid& operator+(const DynamicDataPyramid& summand_pry)
	{
		assert(m_data.size() == summand_pry.m_data.size());
		
		DynamicDataPyramid sum_pyr = summand_pry;		//structure has been copy
		
		int pyr_levels = m_data.size();
		for (int i = 0; i < pyr_levels; ++i)
		{
			sum_pyr[i] = m_data[i] + summand_pry[i];
		}

		return sum_pyr;
	}

	DynamicDataPyramid& operator-(const DynamicDataPyramid& minuend_pyr)
	{
		assert(m_data.size() == minuend_pyr.m_data.size());

		DynamicDataPyramid subtract_pry = minuend_pyr;	//structure has been copy

		int pyr_levels = m_data.size();
		for (int i = 0; i < pyr_levels; ++i)
		{
			subtract_pry[i] = m_data[i] - minuend_pyr[i];
		}

		return subtract_pry;
	}

	template<class ScalarT>
	DynamicDataPyramid& operator+(ScalarT value)
	{
		DynamicDataPyramid sum_pyr = *this;

		int pyr_levels = m_data.size();
		for (int i = 0; i < pyr_levels; ++i)
		{
			sum_pyr[i] = m_data[i] + value;
		}

		return sum_pyr;
	}
	
	template<class ScalarT>
	DynamicDataPyramid& operator-(ScalarT value)
	{
		DynamicDataPyramid subtract_pyr = *this;

		int pyr_levels = m_data.size();
		for (int i = 0; i < pyr_levels; ++i)
		{
			subtract_pyr[i] = m_data[i] - value;
		}

		return subtract_pyr;
	}

	template<class ScalarT>
	DynamicDataPyramid& operator*(ScalarT value)
	{
		DynamicDataPyramid multi_pyr = *this;

		int pyr_levels = m_data.size();
		for (int i = 0; i < pyr_levels; ++i)
		{
			multi_pyr[i] = m_data[i] * value;
		}

		return multi_pyr;
	}

	DynamicDataPyramid(const DynamicDataPyramid& p)
	{
		padx = p.padx;
		pady = p.pady;
		m_scale = p.m_scale;
		m_data = p.m_data;
		interval = p.interval;
		m_flag = p.m_flag;
	}

	void operator=(const DynamicDataPyramid& p)
	{
		padx = p.padx;
		pady = p.pady;
		m_scale = p.m_scale;
		m_data = p.m_data;
		interval = p.interval;
		m_flag = p.m_flag;
	}
	
	//don't copy image data
	//generate independent pyramid(don't share memory)
	template<typename OthersT>
	void shallowcopy(const DynamicDataPyramid<OthersT>& p)
	{
		padx = p.padx;
		pady = p.pady;
		interval = p.interval;
		m_scale = p.scales();
		m_flag = p.flags();
		
		int l = m_scale.size();
		m_data.resize(l);
		for (int i = 0; i < l; ++i)
		{
			m_data[i] = Matrix<T>(p[i].rows(),p[i].cols());
			m_data[i].setzeros();
		}
	}

	//copy image data
	//generate independent pyramid(don't share memory)
	void deepcopy(const DynamicDataPyramid& p)
	{
		padx = p.padx;
		pady = p.pady;
		interval = p.interval;
		m_scale = p.scales();
		m_flag = p.flags();

		int l = m_scale.size();
		m_data.resize(l);
		for (int i = 0; i < l; ++i)
		{
			m_data[i] = Matrix<T>(p[i].rows(),p[i].cols());
			m_data[i].copy(p[i]);
		}
	}
	void setValue(T val)
	{
		for (int level_index = 0; level_index < int(m_scale.size()); ++level_index)
		{
			Matrix<T> level_mat = m_data[level_index];
			level_mat.setval(val);
		}
	}

	int padx;
	int pady;
	int interval;

private:
	std::vector<float> m_scale;
	std::vector<Matrix<T>> m_data;
	std::vector<int> m_flag;
};

/**
 *	@brief some useful types
 */
typedef DynamicDataPyramid<HOG32Vec>			HOG32Pyramid;
typedef DynamicDataPyramid<float>				ScorePyramid;			//score
typedef DynamicDataPyramid<Array<float,2>>		TopHitScorePyramid;		
}

#endif