namespace eagleeye
{
template<typename T>
Variable<T>::Variable(Generator<T>* gen)
{
	m_shared_gen=SharedData<Generator<T>>(gen);
}

template<typename T>
Variable<T>::~Variable()
{
}

template<typename T>
Variable<T> Variable<T>::uniform(T low,T high)
{
	Generator<T>* gen=new UniformGenerator<T>(low,high);
	return Variable<T>(gen);
}

template<typename T>
Variable<T> Variable<T>::gaussian(double mean_val,double variance_val)
{
	Generator<T>* gen=new GaussianGenerator<T>(mean_val,variance_val,
		new UniformGenerator<double>(-1.0,1.0),
		new UniformGenerator<double>(-1.0,1.0));
	return Variable<T>(gen);
}

template<typename T>
Variable<T> Variable<T>::discreteDis(DynamicArray<T> discrete_data,DynamicArray<float> distribution)
{
	Generator<T>* gen = new DiscreteDisGenerator<T>(discrete_data,distribution,new UniformGenerator<float>(0.0f,1.0f));
	return Variable<T>(gen);
}

template<typename T>
T Variable<T>::var()
{
	return m_shared_gen->gen();
}

template<typename T>
void Variable<T>::switchToDebug(unsigned int seed/* =1000 */)
{
	m_shared_gen->setSeed(seed);
}
}
