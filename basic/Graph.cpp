#include "Graph.h"

namespace eagleeye
{
Graph::Graph()
{

}

Graph::~Graph()
{
	std::vector<Node*>::iterator n_iter,n_iend(m_node_set.end());
	for (n_iter = m_node_set.begin(); n_iter != n_iend; ++n_iter)
	{
		delete (*n_iter);
	}

	std::vector<Edge*>::iterator e_iter,e_iend(m_edge_set.end());
	for (e_iter = m_edge_set.begin(); e_iter != e_iend; ++e_iter)
	{
		delete (*e_iter);
	}
}

void Graph::addEdge(Edge* edge)
{
	int start_index = edge->start_index;
	int end_index = edge->end_index;

	m_node_set[start_index]->out_edges.push_back(edge);
	m_node_set[end_index]->in_edges.push_back(edge);

	m_edge_set.push_back(edge);
}

void Graph::addNode(Node* node)
{
	m_node_set.push_back(node);
}
}
