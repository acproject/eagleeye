#ifndef _GRAPH_H_
#define _GRAPH_H_
#include <vector>
#include <map>

namespace eagleeye
{
struct Edge;

struct Node
{
	Node(int i,float v = 0):index(i),value(v){};

	int index;
	float value;

	std::vector<Edge*> out_edges;
	std::vector<Edge*> in_edges;
};

struct Edge
{
	Edge(){};
	Edge(int s,int e):start_index(s),end_index(e){};
	Edge(int s,int e,float w):start_index(s),end_index(e),weight(w){};
	int start_index;
	int end_index;
	float weight;
};

class Graph
{
public:
	typedef std::vector<Node*>							NodesType;
	typedef std::vector<Edge*>							EdgesType;

	Graph();
	~Graph();

	void addEdge(Edge* edge);
	void addNode(Node* node);

	inline EdgesType& getEdge()
	{
		return m_edge_set;
	}

	inline NodesType& getNode()
	{
		return m_node_set;
	}

	inline Edge* getEdge(int index)
	{
		return m_edge_set[index];
	}

	inline Node* getNode(int index)
	{
		return m_node_set[index];
	}

private:
	NodesType m_node_set;	
	EdgesType m_edge_set;
};
}

#endif