#ifndef _GRAPHCUTNODE_H_
#define _GRAPHCUTNODE_H_

#include "AnyNode.h"
#include "MetaOperation.h"
#include "MatrixMath.h"
#include "Graph.h"

namespace eagleeye
{
template<class ImageSigT>
class GraphCutNode:public AnyNode
{
public:
	/**
	 *	@brief define some basic type
	 *	@note you must do these
	 */
	typedef GraphCutNode								Self;
	typedef AnyNode										Superclass;

	typedef typename ImageSigT::MetaType				PixelType;

	GraphCutNode();
	virtual ~GraphCutNode();

	/**
	 *	@brief Get class identity
	 */
	EAGLEEYE_CLASSIDENTITY(GraphCutNode);
	
	/**
	 *	@brief define input and output image signal type
	 */
	EAGLEEYE_INPUT_PORT_TYPE(ImageSigT,0,IMAGE_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSignal<int>,0,LABEL_DATA);
	EAGLEEYE_OUTPUT_PORT_TYPE(ImageSignal<float>,1,GRAY_DATA);

	/**
	 *	@brief Get the segmentation label map
	 *	@note Firstly, you have to run the pipeline update. Otherwise, you 
	 *	couldn't get anything.
	 */
	Matrix<int> getLabelMap();
	
	/**
	 *	@brief Get the segmentation gray map
	 *	@note Firstly, you have to run the pipeline update. Otherwise, you 
	 *	couldn't get anything.
	 */
	Matrix<float> getGrayMap();

	/**
	 *	@brief get the number of labels
	 *	@note Firstly, you have to run the pipeline update. Otherwise, you 
	 *	couldn't get anything.
	 */
	int getLabelNum();

	/**
	 *	@brief set/get min region size
	 */
	void setMinRegionSize(const int min_size);
	void getMinRegionSize(int& min_size);

	/**
	 *	@brief set/get meage threshold
	 *	@note threshold=1.0f/c_weight
	 */
	void setCWeight(const float c_weight);
	void getCWeight(float& c_weight);

	/**
	 *	@brief execute segmentation algorithm
	 */
	virtual void executeNodeInfo();

	/**
	 *	@brief make self check
	 *	@note judge whether some preliminary conditions have been satisfied.
	 */
	virtual bool selfcheck();

protected:
	/**
	 *	@brief segment the graph by using edge weight
	 */
	void segmentGraph(int num_vertices,int num_edges,Edge* edges,float c);
	
	/**
	 *	@brief construct the edge connection of graph
	 */
	void constructEdgeSet();

private:
	GraphCutNode(const GraphCutNode&);
	void operator=(const GraphCutNode&);

	Matrix<float> m_input_img;			/**< the input image*/
	Matrix<int> m_label_map;			/**< the label map of segmentation*/
	Matrix<float> m_gray_map;			/**< the gray map of segmentation*/
	int m_labels_num;			
	Edge* m_edges;						/**< the edges in the graph*/
	int m_edges_num;					/**< the edges number of the graph*/
	int m_vertices_num;					/**< the vertices number of the graph*/
	int m_rows;							/**< the rows number of the input image*/
	int m_cols;							/**< the cols number of the input image*/

	float m_c;
	int m_min_size;						/**< the minimum area of region*/
};
}

#include "GraphCutNode.hpp"
#endif