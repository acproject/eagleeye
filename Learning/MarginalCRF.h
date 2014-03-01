#ifndef _MARGINALCRF_H_
#define _MARGINALCRF_H_

#include "EagleeyeMacro.h"
#include "Matrix.h"
#include "Learning/lossfunction.h"
#include <vector>

namespace eagleeye
{
class CRFLFGS;
/**
 *	@brief CRF model
 *	@detail 
 *	Exponential Family	p(x;theta)=exp(theta*f(x)-A(theta));
 *	@note reference
 *	Justin Domke, Beating the Likelihood Marginalization based parameter learning in 
 *	Graphical Models
 *	@par example:
 *	@code
 *	Matrix<float> unary_pot(nodes_num,2);
 *	Matrix<float> edge_pot(edges_num,1);
 *	unary_pot = unary_pot.t();
 *	edge_pot = edge_pot.t();
 *	Matrix<int> target_states(1,rows * cols);
 *	ImageCRF img_crf;
 *	img_crf.constructImgGraph(rows,cols,2);
 *	Matrix<float> unary_coe,clique_coe;
 *	img_crf.train(target_states,unary_pot,edge_pot,unary_coe,clique_coe);
 *	img_crf.saveMarginalCRF("E://hello_mode.dat");
 *	//infer process
 *	Matrix<int> state;
 *	Matrix<float> probability;
 *	img_crf.infer(unary_pot,edge_pot,state,probability);
 *	@endcode
 */
struct CRFGraph
{
	int n_states;
	int n_nodes;
	int n_clique;
	Matrix<int> pairs;
	Matrix<int> n1;
	Matrix<int> n2;
};
class EAGLEEYE_API MarginalCRF
{
public:
	enum LossFunType
	{
		//Marginal-based loss functions
		LOSS_UL = 0 ,
		LOSS_CL ,
		LOSS_UQUAD ,
		LOSS_CQUAD
	};
	enum InferenceType
	{
		MEANFIELD ,
		TRW
	};
	friend class CRFLFGS;

public:
	MarginalCRF(InferenceType infer_type = MEANFIELD,LossFunType loss_fun_type = LOSS_CL);
	virtual ~MarginalCRF();

	/**
	 *	@brief train CRF model
	 */
	void train(const Matrix<int>& target_states,
		const Matrix<float>& unary_pot,const Matrix<float>& clique_pot,
		Matrix<float>& unary_coe,Matrix<float>& clique_coe);

	void train(const std::vector<Matrix<int>>& target_states_samples,
		const std::vector<Matrix<float>>& unary_samples,
		const std::vector<Matrix<float>>& clique_samples,
		Matrix<float>& unary_coe,Matrix<float>& clique_coe);

	/**
	 *	@brief inference nodes state and probability
	 */
	void infer(const Matrix<float>& unary_pot,const Matrix<float>& clique_pot,
		Matrix<int>& state,Matrix<float>& probability);

	/**
	 *	@brief get probability of target states
	 */
	float targetProbability(const Matrix<int>& target_states,const Matrix<float>& unary_pot,
		const Matrix<float>& clique_pot);

	void setModelParam(const Matrix<float>& unary_coe,const Matrix<float>& clique_coe);
	void getModelParam(Matrix<float>& unary_coe,Matrix<float>& clique_coe);

	/**
	 *	@brief save/load Marginal CRF model
	 */
	void saveMarginalCRF(const char* crf_model_file);
	void loadMarginalCRF(const char* crf_model_file);

protected:
	/**
	 *	@brief using meanfield back propagation algorithm to compute the loss
	 *	function derivative with respect to implicit variable(unary state weight and
	 *	clique state weight)
	 */
	float backpropMeanfield();

	/**
	 *	@brief using meanfield algorithm to infer
	 */
	void meanfieldInfer();

	/**
	 *	@brief using trw back propagation algorithm to compute the loss 
	 *	function derivative with respect to implicit variable
	 */
	float backpropTRW();

	/**
	 *	@brief using trw algorithm to infer
	 */
	void trwInfer();

	/**
	 *	@brief start infer algorithm
	 */
	void infer();
	
	/**
	 *	@brief start back propagation to compute the loss function
	 *	derivative with respect to implicit variable
	 */
	float backprop();
	
	/**
	 *	@brief set potential function value for all nodes and cliques
	 */
	void setUsingData(const Matrix<float>& unary_pot,const Matrix<float>& clique_pot,const Matrix<int>& target_states);
	
protected:
	Matrix<float> m_theta_i;				/**< states * nodes*/
	Matrix<float> m_theta_ij;				/**< states * clique*/
	Matrix<float> m_dtheta_i;				/**< states * nodes*/
	Matrix<float> m_dtheta_ij;				/**< states * clique*/

	Matrix<float> m_psi_i;					/**< exp(m_theta_i)*/
	Matrix<float> m_psi_ij;					/**< exp(m_theta_ij)*/

	Matrix<float> m_b_i;					/**< the marginal distribution for 
											every node (states * nodes)*/
	Matrix<float> m_b_ij;					/**< the marginal distribution for 
											every clique (states * cliques)*/
	
	Matrix<float> m_b_i_z;
	Matrix<float> m_b_ij_z;

	Matrix<float> m_db_i;					/**< the marginal distribution gradient 
											of every node (states * nodes)*/
	Matrix<float> m_db_ij;					/**< the marginal distribution gradient 
											of every clique (states * cliques)*/

	Matrix<float> m_unary_pot;				/**< the unary potential for every node 
											(category * nodes)*/
	Matrix<float> m_clique_pot;				/**< the clique potential for every clique 
											(category * clique)*/
	Matrix<float> m_unary_pot_t;			/**< easy to use*/
	Matrix<float> m_clique_pot_t;			/**< easy to use*/

	Matrix<float> m_unary_coe;				/**< the unary state weight (states * category)*/
	Matrix<float> m_clique_coe;				/**< the clique state weight (states * category)*/

	int m_n_states;							/**< the states number*/
	int m_n_nodes;							/**< the nodes number*/
	int m_n_cliques;						/**< the cliques number(edges number)*/

	Matrix<int> m_pairs;					/**< edge number * 2. (leftnode rightnode)*/
	Matrix<int> m_n1;						/**< leftnode edgeindex edgeindex edgeindex ...*/
	Matrix<int> m_n2;						/**< rightnode edgeindex edgeindex edgeindex ...*/

	int m_maxiter;							/**< the maximum iteration number*/

	float m_convthreshold;					/**< convergence threshold*/
	LossFunction* m_loss_fun;				/**< loss function*/
	float m_loss;							/**< loss value*/

	LossFunType m_loss_fun_type;			/**< loss function type*/
	InferenceType m_infer_type;				/**< inference algorithm type*/

	CRFLFGS* m_crf_lfgs;					/**< LFGS optimization object*/

	Matrix<float> m_trw_m1;
	Matrix<float> m_trw_m2;
	Matrix<float> m_trw_n;
	Matrix<float> m_rho_psi_ij;
	float m_trw_rho;

	Matrix<float> m_back_propagation_marginal;
	int m_back_propagation_count;
	
	int m_unary_feature_dim;
	int m_clique_feature_dim;
	std::vector<Matrix<float>> m_unary_samples;				/**< for training*/
	std::vector<Matrix<float>> m_clique_samples;			/**< for training*/
	std::vector<Matrix<int>> m_states_samples;				/**< for training*/
};

class EAGLEEYE_API ImageCRF:public MarginalCRF
{
public:
	ImageCRF(InferenceType infer_type = MEANFIELD,LossFunType loss_fun_type = LOSS_CL);
	virtual ~ImageCRF(){};

	/**
	 *	@brief construct graph structure using the image structure info
	 *	@note this is a help function
	 */
	void constructImgGraph(int rows,int cols,int states_num);
};
}
#endif