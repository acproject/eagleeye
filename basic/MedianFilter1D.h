#ifndef _MEDIANFILTER1D_H_
#define _MEDIANFILTER1D_H_

#include <map>
#include <vector>

template <typename T> 
class MedianFilter1D
{
public:

	// constructors
	MedianFilter1D(): m_windowsize(3)
	{
	};

	MedianFilter1D(int windowsize)
	{
		setWindowSize(windowsize);
	};

	~MedianFilter1D()
	{
	};

	// operators
	T& operator [](int m) { return m_array[m]; };
	const T& operator[](int m) const { return m_array[m]; };


	// selectors
	int count() const { return m_array.size(); };
	bool isEmpty() const { return m_array.empty(); };

	// modifiers

	void setWindowSize(int windowsize)
	{
		if(windowsize < 3 || !(windowsize % 2))
			throw "Invalid window size";
		m_windowsize = windowsize;
	};


	std::vector<T> execute(const std::vector<T>& v, bool enabled = true)
	{
		// clear output
		m_array.clear();
		// clear histogram
		m_histogram.erase(m_histogram.begin(), m_histogram.end());

		if(enabled)
		{
			// if filter is enabled - perform filtering
			filterImpl(v);
		}
		else
		{
			// if filter is disabled - make simply copy of input
			m_array.insert(m_array.end(), v.begin(), v.end());
		}
		return m_array;
	};

private:
	// median filter aperture
	int m_windowsize;

	// output - filtered input
	std::vector<T> m_array;
	// histogram
	std::map<T, int> m_histogram;

	typedef typename std::map<T, int>::iterator iterator;
	// current median - position in tree
	iterator m_median;

	MedianFilter1D(const MedianFilter1D&);

	void inc(const T& key)
	{
		std::map<T, int>::iterator it = m_histogram.find(key);
		if(it != m_histogram.end())
			it->second++;
		else
			m_histogram.insert(std::pair<T,int>(key, 1));
	};

	void dec(const T& key)
	{
		std::map<T, int>::iterator it = m_histogram.find(key);
		if(it != m_histogram.end())
		{
			if(it->second == 1)
			{
				if(m_median != it)
					m_histogram.erase(it);
				else
					it->second = 0;
			}
			else
			{
				it->second--;
			}
		}
	};


	void filterImpl(const std::vector<T>& obj)
	{
		T av, dv;
		int sum = 0, dl,  middle = m_windowsize/2+1;

		for (int j = -m_windowsize/2; j <= m_windowsize/2; j++ )
		{
			if ( j < 0 )
			{
				av = obj.front();
			}
			else if(j > (int)obj.size()-1)
			{
				av = obj.back();
			}
			else
			{
				av = obj[j];
			}
			inc(av);
		}


		for(m_median = m_histogram.begin(); m_median != m_histogram.end(); m_median++)
		{
			sum += m_median->second;
			if( sum >= middle )
				break;
		}

		m_array.push_back(m_median->first);
		dl = sum - m_median->second;

		int N = obj.size();
		for (int j = 1; j < N; j++)
		{
			int k = j - m_windowsize/2-1;
			dv = k < 0 ? obj.front() : obj[k];
			k = j + m_windowsize/2;
			av = k > (int)obj.size()-1 ? obj.back() : obj[k];
			if(av != dv)
			{
				dec(dv);
				if(dv < m_median->first)
					dl--;
				inc(av);
				if( av < m_median->first )
					dl++;
				if(dl >= middle)
				{
					while(dl >= middle)
					{
						m_median--;
						dl -= m_median->second;
					}
				}
				else
				{
					while (dl + m_median->second < middle)
					{
						dl += m_median->second;
						m_median++;
					}
				}
			}
			m_array.push_back(m_median->first);
		}
	};

};
#endif

