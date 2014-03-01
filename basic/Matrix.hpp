namespace eagleeye
{
template<typename T>
Matrix<T>::Matrix(unsigned int rows,unsigned int cols)
{
	m_rows = rows;
	m_cols = cols;

	m_r_range.s = 0;
	m_r_range.e = m_rows;
	m_c_range.s = 0;
	m_c_range.e = m_cols;

	m_ptr = new PtrType(new T[rows * cols]);
	memset(m_ptr->data,0,sizeof(T) * rows * cols);
}

template<typename T>
Matrix<T>::Matrix(unsigned int rows,unsigned int cols,T val)
{
	m_rows = rows;
	m_cols = cols;

	m_r_range.s = 0;
	m_r_range.e = m_rows;
	m_c_range.s = 0;
	m_c_range.e = m_cols;

	m_ptr = new PtrType(new T[rows * cols]);
	
	int total = rows * cols;
	for (int i = 0; i < total; ++i)
	{
		m_ptr->data[i] = val;
	}
}

template<typename T>
Matrix<T>::Matrix(unsigned int rows,unsigned int cols,void* data,bool copy_flag)
{
	m_rows = rows;
	m_cols = cols;

	m_r_range.s = 0;
	m_r_range.e = m_rows;
	m_c_range.s = 0;
	m_c_range.e = m_cols;
	
	if (copy_flag)
	{
		m_ptr = new PtrType(new T[rows * cols]);
		memcpy(m_ptr->data,data,sizeof(T) * rows * cols);
	}
	else
	{
		m_ptr = new PtrType((T*)data,false);
	}
}

// template<typename T>
// Matrix<T>::Matrix(unsigned int rows,unsigned int cols,T* data,bool charge_data)
// {
// 	m_rows = rows;
// 	m_cols = cols;
// 
// 	m_r_range.s = 0;
// 	m_r_range.e = m_rows;
// 	m_c_range.s = 0;
// 	m_c_range.e = m_cols;
// 
// 	if (charge_data)
// 	{
// 		m_ptr = new PtrType(data);
// 	}
// 	else
// 	{
// 		m_ptr = new PtrType(new T[rows * cols]);
// 		memcpy(m_ptr->data,data,sizeof(T) * rows * cols);
// 	}
// }

template<typename T>
T& Matrix<T>::at(unsigned int r_index,unsigned int c_index) const
{
	unsigned int real_pos = pos(r_index,c_index);
	return m_ptr->data[real_pos];
}

template<typename T>
T& Matrix<T>::at(unsigned int index) const
{
	unsigned int real_pos = pos(index);
	return m_ptr->data[real_pos];
}

template<typename T>
T& Matrix<T>::operator() (int r_index,int c_index) const
{
	unsigned int real_pos = pos(r_index,c_index);
	return m_ptr->data[real_pos];
}

template<typename T>
T& Matrix<T>::operator ()(int index) const
{
	unsigned int real_pos = pos(index);
	return m_ptr->data[real_pos];
}

template<typename T>
T& Matrix<T>::operator [](int index) const
{
	unsigned int real_pos = pos(index);
	return m_ptr->data[real_pos];
}

template<typename T>
T* Matrix<T>::row(unsigned int r_index)
{
	unsigned int real_r = pos_r(r_index);
	return m_ptr->data + real_r * m_cols + m_c_range.s;
}

template<typename T>
const T* Matrix<T>::row(unsigned int r_index) const
{
	unsigned int real_r = pos_r(r_index);
	return m_ptr->data + real_r * m_cols + m_c_range.s;
}

template<typename T>
T* Matrix<T>::anyptr(unsigned int index)
{
	unsigned int real_r = pos(index);
	return m_ptr->data + real_r;
}

template<typename T>
const T* Matrix<T>::anyptr(unsigned int index) const
{
	unsigned int real_r = pos(index);
	return m_ptr->data + real_r;
}

template<typename T>
void Matrix<T>::clone()
{
	unsigned int requested_rows = m_r_range.e - m_r_range.s;
	unsigned int requested_cols = m_c_range.e - m_c_range.s;

	T* data = new T[requested_rows * requested_cols];

	for (unsigned int i = 0; i < requested_rows; ++i)
	{
		T* r_ptr = row(i);
		memcpy(data + i * requested_cols,r_ptr,sizeof(T) * requested_cols);
	}

	if ((--(m_ptr->use_count)) == 0)
	{
		delete m_ptr;
	}
	
	m_ptr = new PtrType(data);
	
	m_rows = requested_rows;
	m_cols = requested_cols;

	m_r_range.s = 0;
	m_r_range.e = m_rows;
	m_c_range.s = 0;
	m_c_range.e = m_cols;
}

template<typename T>
Matrix<T> Matrix<T>::operator()(Range r_range,Range c_range)
{
	Matrix<T> sub_matrix = (*this);
	sub_matrix.m_r_range.s += r_range.s;
	sub_matrix.m_r_range.e = sub_matrix.m_r_range.s + (r_range.e - r_range.s);

	sub_matrix.m_c_range.s += c_range.s;
	sub_matrix.m_c_range.e = sub_matrix.m_c_range.s + (c_range.e - c_range.s);

	return sub_matrix;
}

template<typename T>
const Matrix<T> Matrix<T>::operator ()(Range r_range,Range c_range) const
{
	Matrix<T> sub_matrix = (*this);
	sub_matrix.m_r_range.s += r_range.s;
	sub_matrix.m_r_range.e = sub_matrix.m_r_range.s + (r_range.e - r_range.s);

	sub_matrix.m_c_range.s += c_range.s;
	sub_matrix.m_c_range.e = sub_matrix.m_c_range.s + (c_range.e - c_range.s);


	return sub_matrix;
}

template<typename T>
bool Matrix<T>::isfull()
{
	if ((m_r_range.e - m_r_range.s) != m_rows)
	{
		return false;
	}
	if ((m_c_range.e - m_c_range.s) != m_cols)
	{
		return false;
	}
	return true;
}

template<typename T>
bool Matrix<T>::isfull() const 
{
	if ((m_r_range.e - m_r_range.s) != m_rows)
	{
		return false;
	}
	if ((m_c_range.e - m_c_range.s) != m_cols)
	{
		return false;
	}
	return true;
}

template<typename T>
bool Matrix<T>::isContinuous()
{
	if ((m_r_range.e - m_r_range.s) != m_rows)
	{
		return false;
	}
	if ((m_c_range.e - m_c_range.s) != m_cols)
	{
		return false;
	}
	return true;
}

template<typename T>
unsigned int Matrix<T>::pos(unsigned int r_index,unsigned int c_index) const
{
	unsigned int real_r_index = pos_r(r_index);
	unsigned int real_c_index = pos_c(c_index);
	
	return real_r_index * m_cols + real_c_index;
}

template<typename T>
unsigned int Matrix<T>::pos_r(unsigned int r_index) const
{
	return r_index + m_r_range.s;
}

template<typename T>
unsigned int Matrix<T>::pos_c(unsigned int c_index) const
{
	return c_index + m_c_range.s;
}

template<typename T>
unsigned int Matrix<T>::pos(unsigned int index) const
{
	unsigned int r_index = index / (m_c_range.e - m_c_range.s);
	unsigned int c_index = index % (m_c_range.e - m_c_range.s);
	return pos(r_index,c_index);
}

template<typename T>
unsigned int Matrix<T>::rows() const
{
	return m_r_range.e - m_r_range.s;
}

template<typename T>
unsigned int Matrix<T>::cols() const
{
	return m_c_range.e - m_c_range.s;
}

template<typename T>
unsigned int Matrix<T>::size() const
{
	return (m_r_range.e - m_r_range.s) * (m_c_range.e - m_c_range.s);
}

template<typename T>
bool Matrix<T>::isin(unsigned int r_index,unsigned int c_index)
{
	if (r_index < 0 || r_index >= m_r_range.e)
	{
		return false;
	}
	if (c_index < 0 || c_index >= m_c_range.e)
	{
		return false;
	}
	return true;
}

template<typename T>
Matrix<T> Matrix<T>::t()
{
	unsigned int requested_rows = m_r_range.e - m_r_range.s;
	unsigned int requested_cols = m_c_range.e - m_c_range.s;

	unsigned int t_requested_rows = requested_cols;
	unsigned int t_requested_cols = requested_rows;

	Matrix<T> t_m(t_requested_rows,t_requested_cols,T(0));
	T* t_data = t_m.dataptr();

	for (unsigned int i = 0; i < requested_rows; ++i)
	{
		T* my_ptr = row(i);
		for (unsigned int j = 0; j < requested_cols; ++j)
		{
			t_data[j * requested_rows + i] = my_ptr[j];
		}
	}

	return t_m;
}

template<typename T>
T* Matrix<T>::dataptr()
{
	return m_ptr->data;
}

template<typename T>
const T* Matrix<T>::dataptr() const
{
	return m_ptr->data;
}

template<typename T>
void Matrix<T>::setzeros()
{
	unsigned int t_rows = m_r_range.e - m_r_range.s;
	unsigned int t_cols = m_c_range.e - m_c_range.s;

	for (unsigned int i = 0; i < t_rows; ++i)
	{
		T* r_ptr = row(i);
		memset(r_ptr,0,sizeof(T) * t_cols);
	}
}

template<typename T>
void Matrix<T>::setval(T val)
{
	unsigned int t_rows = m_r_range.e - m_r_range.s;
	unsigned int t_cols = m_c_range.e - m_c_range.s;

	for (unsigned int i = 0; i < t_rows; ++i)
	{
		T* r_ptr = row(i);
		for (unsigned int j = 0; j < t_cols; ++j)
		{
			r_ptr[j] = val;
		}
	}
}

template<typename T>
void Matrix<T>::copy(const Matrix<T>& src)
{
	assert(rows() <= src.rows());
	assert(cols() <= src.cols());

	int me_rows = rows();
	int me_cols = cols();

	for (int i = 0; i < me_rows; ++i)
	{
		const T* src_data = src.row(i);
		T* me_data = row(i);
		memcpy(me_data,src_data,sizeof(T) * me_cols);
	}
}

template<typename T>
Matrix<T> Matrix<T>::reshape(int r,int c,Order order) const
{
	int now_r = rows();
	int now_c = cols();

	assert(r * c == now_r * now_c);
	
	Matrix one(r,c);
	T* one_data = one.dataptr();

	for (int i = 0; i < now_r; ++i)
	{
		const T* data = row(i);
		memcpy(one_data,data,sizeof(T) * now_c);
		one_data = one_data + now_c;
	}

	switch(order)
	{
	case ROW:
		{
			return one;
			break;
		}
	case COL:
		{
			return one.t();
			break;
		}
	}

	return Matrix<T>();
}

template<typename T>
Matrix<T> Matrix<T>::dot(const Matrix<T>& m) const
{
	assert(rows() == m.rows());
	assert(cols() == m.cols());

	unsigned int t_rows = rows();
	unsigned int t_cols = cols();

	Matrix<T> product(t_rows,t_cols,T(0));
	T* data = product.dataptr();

	for (unsigned int i = 0; i < t_rows; ++i)
	{
		const T* left_ptr = row(i);
		const T* right_ptr = m.row(i);
		for (unsigned int j = 0; j < t_cols; ++j)
		{
			data[i*t_cols+j] = left_ptr[j] * right_ptr[j];
		}
	}

	return product;
}

template<typename T>
Matrix<T> Matrix<T>::operator +(const Matrix<T>& m)
{
	unsigned int t_rows = m_r_range.e - m_r_range.s;
	unsigned int t_cols = m_c_range.e - m_c_range.s;

	T* data = new T[t_rows * t_cols];
	memset(data,0,sizeof(T) * t_rows * t_cols);

	for (unsigned int i = 0; i < t_rows; ++i)
	{
		T* left_ptr = row(i);
		const T* right_ptr = m.row(i);
		for (unsigned int j = 0; j < t_cols; ++j)
		{
			data[i * t_cols + j] = left_ptr[j] + right_ptr[j];
		}
	}

	Matrix<T> a(t_rows,t_cols,data);
	return a;
}

template<typename T>
Matrix<T>& Matrix<T>::operator +=(const Matrix<T>& m)
{
	unsigned int t_rows = m_r_range.e - m_r_range.s;
	unsigned int t_cols = m_c_range.e - m_c_range.s;

	for (unsigned int i = 0; i < t_rows; ++i)
	{
		T* left_ptr = row(i);
		const T* right_ptr = m.row(i);
		for (unsigned int j = 0; j < t_cols; ++j)
		{
			left_ptr[j] = left_ptr[j] + right_ptr[j];
		}
	}

	return *this;
}

template<typename T>
Matrix<T> Matrix<T>::operator-(const Matrix<T>& m)
{
	unsigned int rows = m_r_range.e - m_r_range.s;
	unsigned int cols = m_c_range.e - m_c_range.s;

	Matrix<T> result(rows,cols);
	for (unsigned int i = 0; i < rows; ++i)
	{
		T* data_ptr = row(i);
		T* data_m_ptr = m.row(i);

		T* data_result_ptr = result.row(i);
		for (unsigned int j = 0; j < cols; ++j)
		{
			data_result_ptr[j] = data_ptr[j] - data_m_ptr[j];
		}
	}

	return result;
}

template<typename T>
Matrix<T>& Matrix<T>::operator -=(const Matrix<T>& m)
{
	unsigned int rows = m_r_range.e - m_r_range.s;
	unsigned int cols = m_c_range.e - m_c_range.s;

	for (unsigned int i = 0; i < rows; ++i)
	{
		T* data_ptr = row(i);
		T* data_m_ptr = m.row(i);

		for (unsigned int j = 0; j < cols; ++j)
		{
			data_ptr[j] = data_ptr[j] - data_m_ptr[j];
		}
	}
	return *this;
}

template<typename T>
Matrix<T> Matrix<T>::operator*(const Matrix<T>& r)
{
	unsigned int l_rows = rows();
	unsigned int l_cols = cols();

	unsigned int r_rows = r.rows();
	unsigned int r_cols = r.cols();
	assert(l_cols == r_rows);

	Matrix<T> result(l_rows,r_cols,T(0));
	T* data = result.dataptr();

	for (unsigned int left_i = 0; left_i < l_rows; ++left_i)
	{
		T* l_r_ptr = row(left_i);

		for (unsigned int right_j = 0; right_j < r_cols; ++right_j)
		{
			for (unsigned int left_j = 0; left_j < l_cols; ++left_j)
			{
				const T* r_r_ptr = r.row(left_j);
				data[left_i * r_cols + right_j] += l_r_ptr[left_j] * r_r_ptr[right_j];
			}

		}
	}

	return result;
}

template<typename T>
bool Matrix<T>::isempty()
{
	if (m_rows == 0 || m_cols == 0)
		return true;
	else
		return false;
}
template<typename T>
bool Matrix<T>::isempty() const
{
	if (m_rows == 0 || m_cols == 0)
		return true;
	else
		return false;
}

template<typename T>
unsigned int Matrix<T>::width() const
{
	return m_cols;
}

template<typename T>
void Matrix<T>::offset(unsigned int& offset_r,unsigned int& offset_c) const
{
	offset_r = m_r_range.s;
	offset_c = m_c_range.s;
}

template<typename T>
Matrix<T> Matrix<T>::mapfrom(unsigned int rows,unsigned int cols,void* data)
{
	return Matrix<T>(rows,cols,data);
}

template<typename T>
void Matrix<T>::fliplr()
{
	int rs = rows();
	int cs = cols();
	
	int half_cs = cs / 2;

	for (int i = 0; i < rs; ++i)
	{
		T* data = row(i);
		for (int j = 0; j < half_cs; ++j)
		{
			T val = data[j];
			data[j] = data[cs - 1 - j];
			data[cs - 1 - j] = val;
		}
	}
}

template<typename T>
void Matrix<T>::flipud()
{
	int rs = rows();
	int cs = cols();

	int half_rs = rs / 2;
	for (int i = 0; i < half_rs; ++i)
	{
		T* up_data = row(i);
		T* down_data = row(rs - 1 - i);
		for (int j = 0; j < cs; ++j)
		{
			T val = up_data[j];
			up_data[j] = down_data[j];
			down_data[j] = val;
		}
	}
}

}
