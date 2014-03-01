#include "OnlineDictionary.h"
#include "EagleeyeCore.h"

namespace eagleeye
{
OnlineDictionary::OnlineDictionary(int dictionary_size)
:Dictionary(dictionary_size)
{

}
OnlineDictionary::~OnlineDictionary()
{

}

void OnlineDictionary::trainOnline(const Matrix<float>& element,Matrix<float>& dic,Matrix<int>& dic_counts)
{
	skmeans(element,m_dictionary_capacity,dic,dic_counts);
}

Matrix<float> OnlineDictionary::train()
{
	int elements_num = 0;
	int elements_dim = 0;
	std::vector<Matrix<float>>::iterator iter,iend(m_elements.end());
	for (iter = m_elements.begin();iter != iend; ++iter)
	{
		elements_num += (*iter).rows();
	}

	elements_dim = m_elements[0].cols();

	int start = 0;
	Matrix<float> raw_elements(elements_num,elements_dim);
	for (iter = m_elements.begin(); iter != iend; ++iter)
	{
		memcpy(raw_elements.row(start),(*iter).dataptr(),sizeof(float)*(*iter).rows()*(*iter).cols());
		start += (*iter).rows();
	}

	//clear invalid elements
	m_elements.clear();

	Matrix<float> dic;
	Matrix<int> dic_counts;

	trainOnline(raw_elements,dic,dic_counts);

	return dic;
}

}