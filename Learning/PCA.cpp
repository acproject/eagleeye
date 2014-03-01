#include "Learning/PCA.h"
#include "Eigen/Eigen"
#include "MatrixMath.h"
#include "eagleeyeeigen/EagleeyeEigen.h"
#include "Matlab/MatlabInterface.h"

namespace eagleeye
{
PCA::PCA(float contribution_ratio)
{
	m_contribution_val = contribution_ratio;
}

PCA::~PCA()
{

}

void PCA::compute(const Matrix<float>& data)
{
	//compute mean value
	int samples_num = data.rows();
	int samples_dim = data.cols();

	m_mean_vec = rowmean(data);
	Matrix<float> dif_matrix(samples_num,samples_dim,float(0.0f));
	
	float* mean_vec_data = m_mean_vec.row(0);
	for (int i = 0; i < samples_num; ++i)
	{
		float* dif_matrix_data = dif_matrix.row(i);
		const float* matrix_data = data.row(i);
		for (int j = 0; j < samples_dim; ++j)
		{
			dif_matrix_data[j] = matrix_data[j] - mean_vec_data[j];
		}
	}
	
	EigenMapf eigen_dif_mat(dif_matrix.dataptr(),dif_matrix.rows(),dif_matrix.cols()); 
	Eigen::JacobiSVD<EigenMatf> svd(eigen_dif_mat,Eigen::ComputeThinU | Eigen::ComputeThinV);
	
	Eigen::JacobiSVD<EigenMatf>::SingularValuesType singular_val = svd.singularValues();
	int singular_num = singular_val.rows();

	float total_energy = 0;
	float current_energy = 0;
	for (int i = 0; i < singular_num; ++i)
	{
		total_energy = total_energy + singular_val(i);
	}
	int pick_num_from_front = 0;
	for (int i = 0; i < singular_num; ++i)
	{
		pick_num_from_front = i + 1;

		current_energy = current_energy + singular_val(i);
		if ((current_energy / total_energy) >= m_contribution_val)
			break;
	}

	EigenMatf eigen_vec = svd.matrixV();
	m_principal_component = Matrix<float>::mapfrom(eigen_vec.rows(),eigen_vec.cols(),eigen_vec.data());
	m_principal_component = m_principal_component(Range(0,m_principal_component.rows()),Range(0,pick_num_from_front));
	m_principal_component.clone();

	m_principle_component_coe = Matrix<float>::mapfrom(singular_val.rows(),singular_val.cols(),singular_val.data());
	m_principle_component_coe = m_principle_component_coe(Range(0,pick_num_from_front),Range(0,1));
	m_principle_component_coe.clone();
}

Matrix<float> PCA::pcaComponents()
{
	return m_principal_component;
}

Matrix<float> PCA::pcaComponentCoe()
{
	return m_principle_component_coe;
}

Matrix<float> PCA::decompose(Matrix<float>& data,Matrix<float>& mean_vec,Matrix<float>& pc)
{
	int rows = data.rows();
	int cols = data.cols();
	Matrix<float> dif_data(rows,cols);
	memcpy(dif_data.dataptr(),data.dataptr(),sizeof(float) * rows * cols);
	
	float* mean_vec_data = mean_vec.row(0);
	for (int i = 0; i < rows; ++i)
	{
		float* dif_data_data = dif_data.row(i);
		for (int j = 0; j < cols; ++j)
		{
			dif_data_data[j] = dif_data_data[j] - mean_vec_data[j];
		}
	}

	EigenMapf eigen_dif_mat(dif_data.dataptr(),dif_data.rows(),dif_data.cols());
	EigenMapf eigen_pc(pc.dataptr(),pc.rows(),pc.cols());

	EigenMatf eigen_decompose= eigen_dif_mat * eigen_pc;
	Matrix<float> me_result = Matrix<float>::mapfrom(eigen_decompose.rows(),eigen_decompose.cols(),eigen_decompose.data());
	me_result.clone();

	return me_result;
}

Matrix<float> PCA::reconstruct(Matrix<float>& coeff,Matrix<float>& mean_vec,Matrix<float>& pc)
{
	EigenMapf eigen_coeff_mat(coeff.dataptr(),coeff.rows(),coeff.cols());
	EigenMapf eigen_pc(pc.dataptr(),pc.rows(),pc.cols());
	EigenMatf eigen_center_mat = eigen_coeff_mat * eigen_pc.transpose();

	Matrix<float> center_data = Matrix<float>::mapfrom(eigen_center_mat.rows(),eigen_center_mat.cols(),eigen_center_mat.data());
	int rows = center_data.rows();
	int cols = center_data.cols();

	Matrix<float> original_data(rows,cols);
	float* mean_vec_data = mean_vec.row(0);
	for (int i = 0; i < rows; ++i)
	{
		float* original_data_data = original_data.row(i);
		float* center_data_data = center_data.row(i);
		for (int j = 0; j < cols; ++j)
		{
			original_data_data[j] = center_data_data[j] + mean_vec_data[j];
		}
	}

	return original_data;
}

Matrix<float> PCA::reconstructErr(const Matrix<float>& data, const Matrix<float>& reconstruct_data)
{
	int rows = data.rows();
	int cols = data.cols();
	Matrix<float> err(rows ,1, float(0.0f));
	
	for (int i = 0; i < rows; ++i)
	{
		const float* data_data = data.row(i);
		const float* reconstruct_data_data =reconstruct_data.row(i);
		float* err_data = err.row(i);

		for (int j = 0; j < cols; ++j)
		{
			err_data[0] += abs(data_data[j] - reconstruct_data_data[j]);
		}
	}

	return err;
}

Matrix<float> PCA::pcaMeanVec()
{
	return m_mean_vec;
}

}