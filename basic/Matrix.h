#ifndef _MATRIX_H_
#define _MATRIX_H_
#include <assert.h>
#include <stdio.h>
#include <ostream>
#include <iomanip>
#include "TraitCenter.h"
#include "MetaOperation.h"

namespace eagleeye
{
enum Order{ROW,COL,OTHERS};

class Range
{
public:
	template<typename T> friend class Matrix;

	explicit Range(unsigned int start=0,unsigned int end=0)
		:s(start),e(end){};
	~Range(){};

	unsigned int s;
	unsigned int e;
};

template<typename T>
class Matrix
{
public:
	typedef T								ElemType;
	
	/**
	 *	@brief Matrix Help class
	 */
	class MatrixHelp
	{
		friend class								 Matrix;

	public:
		inline bool isneighbor(const unsigned int& r_index,const unsigned int& c_index)
		{
			const unsigned int n_r=m_center_r+r_index;
			const unsigned int n_c=m_center_c+c_index;

			return m_matrix_ptr->isin(n_r,n_c);
		}

		inline T& neighbor(const unsigned int& r_index,const unsigned int& c_index)
		{
			const unsigned int n_r=m_center_r+r_index;
			const unsigned int n_c=m_center_c+c_index;
			
			return m_matrix_ptr->at(n_r,n_c);
		}

		~MatrixHelp(){}

	private:
		MatrixHelp(Matrix* matrix_ptr,unsigned int center_r,unsigned int center_c)
			:m_matrix_ptr(matrix_ptr),
			m_center_r(center_r),
			m_center_c(center_c){}
		unsigned int m_center_r;
		unsigned int m_center_c;

		Matrix* m_matrix_ptr;
	};

	/**	@brief This would complete "Dynamic Memory Manage"\n
	 *	This is a simplified Smart Pointer version
	 */
	class Ptr
	{
		friend class								Matrix;

		T* data;
		unsigned int use_count;
		Ptr(T* d=NULL,bool complete_control=true,int count=1)
			:data(d),
			data_complete_control(complete_control),
			use_count(count){};
		bool data_complete_control;

		~Ptr()
		{
			if (data&&data_complete_control)
				delete []data;
		};
	};

	typedef Ptr										PtrType;

public:
	/**
	 *	@brief All kinds of useful constructor
	 */
	Matrix()
	{
		m_ptr = new PtrType();
		m_rows = 0;
		m_cols = 0;
		m_r_range.s = 0;
		m_r_range.e = 0;
		m_c_range.s = 0;
		m_c_range.e = 0;
	};
	Matrix(unsigned int rows,unsigned int cols);
	Matrix(unsigned int rows,unsigned int cols,T val);

	/**
	 *	@brief using matrix structure to wrap outside data
	 *	@note if copy_flag == true, it would copy this outside data; 
	 */
	Matrix(unsigned int rows,unsigned int cols,void* data,bool copy_flag = false);

	virtual ~Matrix()
	{
		if ((--m_ptr->use_count) == 0)
		{
			delete m_ptr;
		}
	}

	inline Matrix(const Matrix &m)
	{
		m_ptr = m.m_ptr;
		m_ptr->use_count++;

		m_rows = m.m_rows;
		m_cols = m.m_cols;

		m_r_range = m.m_r_range;
		m_c_range = m.m_c_range;
	}

	/**
	 *	@brief overload operator =
	 */
	template<class ScalarType>
	inline Matrix& operator=(ScalarType value)
	{
		int me_rows = rows();
		int me_cols = cols();
		for (int i = 0; i < me_rows; ++i)
		{
			T* data = row(i);
			for (int j = 0; j < me_cols; ++j)
			{
				data[j] = (T)value;
			}
		}
		return *this;
	}
	inline Matrix& operator=(const Matrix& m)
	{
		++(m.m_ptr->use_count);
		
		if ((--m_ptr->use_count) == 0)
		{
			delete m_ptr;
		}
		
		m_ptr = m.m_ptr;
	
		m_rows = m.m_rows;
		m_cols = m.m_cols;
		
		m_r_range = m.m_r_range;
		m_c_range = m.m_c_range;

		return *this;
	}

	inline Matrix operator()(Range r_range,Range c_range);
	inline const Matrix operator()(Range r_range,Range c_range) const;

	/**
	 *	@brief judge whether one specific pos is in the range of matrix
	 */
	inline bool isin(unsigned int r_index,unsigned int c_index);

	/**
	 *	@brief check whether this matrix is empty
	 */
	inline bool isempty();
	inline bool isempty() const;

	/**
	 *	@brief get data at one specific pos(with offset)
	 */
	inline T& at(unsigned int r_index,unsigned int c_index) const;
	inline T& at(unsigned int index) const;

	inline T& operator()(int r_index,int c_index) const;
	inline T& operator()(int index) const;

	inline T& operator[](int index) const;

	/**
	 *	@brief get the row data pointer(with offset)
	 */
	inline T* row(unsigned int r_index);
	inline const T* row(unsigned int r_index) const;
	
	/**
	 *	@brief get the data pointer at any pos(with offset)
	 */
	inline T* anyptr(unsigned int index);
	inline const T* anyptr(unsigned int index) const;

	/**
	 *	@brief transpose matrix
	 *	@detail It would generate a new matrix.
	 */
	inline Matrix t();

	/**
	 *	@brief build a independent data copy
	 *	@note It would use a new reference count
	 */
	inline void clone();

	/**
	 *	@brief get the raw data pointer(without offset)
	 */
	inline T* dataptr();
	inline const T* dataptr() const;

	/**
	 *	@brief set zero for all elements
	 */
	inline void setzeros();
	
	/**
	 *	@brief set val for matrix
	 */
	inline void setval(T val);
	template<class ConditionT>
	inline void setval(T val,ConditionT cond_t)
	{
		int me_rows = rows();
		int me_cols = cols();

		for (int i = 0; i < me_rows; ++i)
		{
			T* me_ptr = row(i);
			for (int j = 0; j < me_cols; ++j)
			{
				if (cond_t(me_ptr[j]))
					me_ptr[j] = val;
			}
		}
	}

	/**
	 *	@brief copy src data(with offset)
	 *	@note copy self size data from src matrix. Guarantee
	 *	self size isn't greater than src one
	 */
	inline void copy(const Matrix& src);

	/**
	 *	@brief map outside raw data to matrix structure
	 */
	inline static Matrix mapfrom(unsigned int rows,unsigned int cols,void* data);

	/**
	 *	@brief transform the current matrix to any target matrix
	 *	@note It forces to change every element type of every element in the current
	 *	matrix
	 */
	template<typename TargetT>
	inline Matrix<TargetT> transform() const
	{
		typedef AtomicTypeTrait<T>::AtomicType						SrcAtomicType;
		typedef AtomicTypeTrait<TargetT>::AtomicType				TargetAtomicType;

		int mat_rows = rows();
		int mat_cols = cols();
		Matrix<TargetT> target_mat(mat_rows,mat_cols);

		for (int i = 0; i < mat_rows; ++i)
		{
			TargetT* target_mat_data = target_mat.row(i);
			const T* src_mat_data = row(i);
			for (int j = 0; j < mat_cols; ++j)
			{
				for (int index = 0; index < AtomicTypeTrait<TargetT>::size; ++index)
				{
					OperateTrait<TargetT>::unit(target_mat_data[j],index) = 
						TargetAtomicType(OperateTrait<T>::unit(src_mat_data[j],index%AtomicTypeTrait<T>::size));
				}
			}
		}

		return target_mat;
	}
	template<>
	inline Matrix<T> transform() const
	{
		return (*this);
	}

	/**
	 *	@brief using the user defined operator to transform every element of the current 
	 *	matrix to target matrix
	 *	@note how to design transform operator, please see "MetaOperation.h" \n
	 *	Generally speaking, this kind of operator has to overlap "operator ()(element)" 
	 */
	template<typename OpT>
	inline Matrix<typename OpT::MetaTargetType> transform(OpT opt = OpT()) const
	{
		typedef typename OpT::MetaTargetType						TargetT;

		typedef AtomicTypeTrait<T>::AtomicType						SrcAtomicType;
		typedef AtomicTypeTrait<TargetT>::AtomicType				TargetAtomicType;

		int mat_rows = rows();
		int mat_cols = cols();
		Matrix<TargetT> target_mat(mat_rows,mat_cols);

		for (int i = 0; i < mat_rows; ++i)
		{
			TargetT* target_mat_data = target_mat.row(i);
			const T* src_mat_data = row(i);
			for (int j = 0; j < mat_cols; ++j)
			{
				opt(src_mat_data[j],target_mat_data[j]);
			}
		}

		return target_mat;
	}

	/**
	 *	@brief modify the matrix shape
	 *	@note it would create a new matrix
	 */
	inline Matrix reshape(int r,int c,Order order=ROW) const;

	/**
	 *	@brief flip matrix from left to right
	 */
	inline void fliplr();

	/**
	 *	@brief flip matrix from up to down
	 */
	inline void flipud();

	/**
	 *	@brief whether the data is continuous?
	 */
	inline bool isContinuous();

	/**
	 *	@brief overload operator * .multiply with scalar value or the same type matrix
	 */
	template<class ScalarType>
	inline Matrix operator*(ScalarType value)
	{
		unsigned int t_rows = m_r_range.e - m_r_range.s;
		unsigned int t_cols = m_c_range.e - m_c_range.s;
		
		T* data = new T[t_rows * t_cols];
		memset(data,0,sizeof(T) * t_rows * t_cols);

		for (unsigned int i = 0; i < t_rows; ++i)
		{
			T* r_ptr = row(i);
			for (unsigned int j = 0; j < t_cols; ++j)
			{
				data[i * t_cols + j]=
					r_ptr[j] * value;
			}
		}
		
		Matrix m(t_rows,t_cols,data);

		return m;
	}
	inline Matrix operator*(const Matrix& r);

	template<class ScalarType>
	inline Matrix& operator*=(ScalarType value)
	{
		unsigned int t_rows = m_r_range.e - m_r_range.s;
		unsigned int t_cols = m_c_range.e - m_c_range.s;
		
		for (unsigned int i = 0; i < t_rows; ++i)
		{
			T* r_ptr = row(i);
			for (unsigned int j = 0; j < t_cols; ++j)
			{
				r_ptr[j] *= value;
			}
		}

		return *this;
	}

	/**
	 *	@brief overload operator /. divide scalar value
	 */
	template<typename ScalarType>
	inline Matrix operator/(ScalarType value)
	{
		unsigned int rows = m_r_range.e - m_r_range.s;
		unsigned int cols = m_c_range.e - m_c_range.s;

		Matrix result_matrix(rows,cols);

		for (unsigned int i = 0; i < rows; ++i)
		{
			T* data_ptr = row(i);
			T* result_data_ptr = result_matrix.row(i);
			for (unsigned int j = 0; j < cols; ++j)
			{
				result_data_ptr[j] = data_ptr[j] / value;
			}
		}
		
		return result_matrix;
	}

	template<typename ScalarType>
	inline Matrix& operator/=(ScalarType value)
	{
		unsigned int rows = m_r_range.e - m_r_range.s;
		unsigned int cols = m_c_range.e - m_c_range.s;

		for (unsigned int i = 0; i < rows; ++i)
		{
			T* data_ptr = row(i);
			for (unsigned int j = 0; j < cols; ++j)
			{
				data_ptr[j] = data_ptr[j] / value;
			}
		}

		return *this;
	}
	/**
	 *	@brief Matrix plus operation. plus one scalar value or the same type matrix
	 *	@note The current matrix must possess the same size with matrix m
	 */
	template<typename ScalarType>
	inline Matrix operator+(ScalarType value)
	{
		unsigned int rows = m_r_range.e - m_r_range.s;
		unsigned int cols = m_c_range.e - m_c_range.s;

		Matrix result_matrix(rows,cols);

		for (unsigned int i = 0; i < rows; ++i)
		{
			T* data_ptr = row(i);
			T* result_data_ptr = result_matrix.row(i);
			for (unsigned int j = 0; j < cols; ++j)
			{
				result_data_ptr[j] = data_ptr[j] + (T)value;
			}
		}

		return result_matrix;
	}
	inline Matrix operator+(const Matrix& m);

	template<typename ScalarType>
	inline Matrix& operator+=(ScalarType value)
	{
		unsigned int rows = m_r_range.e - m_r_range.s;
		unsigned int cols = m_c_range.e - m_c_range.s;

		for (unsigned int i = 0; i < rows; ++i)
		{
			T* data_ptr = row(i);
			for (unsigned int j = 0; j < cols; ++j)
			{
				data_ptr[j] = data_ptr[j] + (T)value;
			}
		}

		return *this;
	}
	inline Matrix& operator+=(const Matrix& m);

	/**
	 *	@brief Matrix subtract operation. subtract one scalar value or the same
	 *	type matrix 
	 *	@note The current matrix must possess the same size with matrix m
	 */
	template<typename ScalarType>
	inline Matrix operator-(ScalarType value)
	{
		unsigned int rows = m_r_range.e - m_r_range.s;
		unsigned int cols = m_c_range.e - m_c_range.s;

		Matrix result_matrix(rows,cols);

		for (unsigned int i = 0; i < rows; ++i)
		{
			T* data_ptr = row(i);
			T* result_data_ptr = result_matrix.row(i);
			for (unsigned int j = 0; j < cols; ++j)
			{
				result_data_ptr[j] = data_ptr[j] - (T)value;
			}
		}

		return result_matrix;
	}
	inline Matrix operator-(const Matrix& m);

	template<typename ScalarType>
	inline Matrix& operator-=(ScalarType value)
	{
		unsigned int rows = m_r_range.e - m_r_range.s;
		unsigned int cols = m_c_range.e - m_c_range.s;

		for (unsigned int i = 0; i < rows; ++i)
		{
			T* data_ptr = row(i);
			for (unsigned int j = 0; j < cols; ++j)
			{
				data_ptr[j] = data_ptr[j] - (T)value;
			}
		}

		return *this;
	}
	inline Matrix& operator-=(const Matrix& m);

	/**
	 *	@brief Matrix dot product
	 *	@note The current matrix must possess the same size with matrix m
	 */
	inline Matrix dot(const Matrix& m) const;

	inline MatrixHelp center(const unsigned int& r_index,const unsigned int& c_index)
	{
		return MatrixHelp(this,r_index,c_index);
	}
	
	/**
	 *	@brief get row and col of matrix
	 */
	inline unsigned int rows() const;
	inline unsigned int cols() const;

	/**
	 *	@brief get total element number
	 */
	inline unsigned int size() const;

	/**
	 *	@brief get width of matrix
	 *	@note sometimes, one matrix is only sub-matrix of some other matrix
	 */
	inline unsigned int width() const;

	/**
	 *	@brief get offset row and offset col
	 *	@note sometimes, one matrix is only sub-matrix of some other matrix
	 */
	inline void offset(unsigned int& offset_r,unsigned int& offset_c) const;

	/**
	 *	@brief check whether matrix is full
	 */
	inline bool isfull();
	inline bool isfull() const;

private:
	inline unsigned int pos(unsigned int r_index,unsigned int c_index) const;
	inline unsigned int pos(unsigned int index) const;
	inline unsigned int pos_r(unsigned int r_index) const;
	inline unsigned int pos_c(unsigned int c_index) const;

	PtrType* m_ptr;

	Range m_r_range;
	Range m_c_range;

	unsigned int m_rows;
	unsigned int m_cols;
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const Matrix<T>& ob) 
{
	std::setiosflags(std::ios::fixed);
	int rows = ob.rows();
	int cols = ob.cols();
	for (int i = 0; i < rows; ++i)
	{
		const T* ob_data = ob.row(i);
		for (int j = 0; j < cols; ++j)
		{
			os<<ob_data[j]<<'\t';
		}
		os<<'\n';
	}

	return os;
}

template<typename T>
Matrix<T>& operator<<(Matrix<T>& ob,const T val)
{
	static int serialization_index = 0;
	static int total_elem_num = 0;
	static int ob_p = 0;
	if (ob_p != int(&ob))
	{
		total_elem_num = ob.size();
		serialization_index = 0;
		ob_p = int(&ob);
	}
	
	//get serialization index
	serialization_index = serialization_index % total_elem_num;
	
	//assign value
	ob(serialization_index) = val;
	serialization_index++;

	return ob;
}

template<typename T>
std::istream& operator>>(std::istream& in,Matrix<T>& ob)
{
	int rows = ob.rows();
	int cols = ob.cols();
	for (int i = 0; i < rows; ++i)
	{
		T* ob_data = ob.row(i);
		for (int j = 0; j < cols; ++j)
		{
			in>>ob_data[j];
		}
	}

	if (!in)
	{
		ob = Matrix<T>();
	}

	return in;
}

/**
 *	@brief some useful type
 */
 typedef Matrix<float>						VecF;
 typedef Matrix<double>						VecD;
 typedef Matrix<int>						VecI;
 typedef Matrix<unsigned int>				VecUI;
 typedef Matrix<short>						VecS;
 typedef Matrix<unsigned short>				VecUS;
 typedef Matrix<char>						VecC;
 typedef Matrix<unsigned char>				VecUC;
	
}

#include "Matrix.hpp"
#endif