namespace eagleeye
{
//////////////////////////////////////////////////////////////////////////
//Some Auxiliary class
struct Elements
{
	int rank;
	int p;
	int size;
};

class GraphCutHelper
{
public:
	GraphCutHelper(int elements_num);
	~GraphCutHelper();

	int find(int x);
	void join(int x,int y);
	int size(int x) const;
	int classNum(){return m_class_num;}

private:
	Elements* m_elts;
	int m_class_num;
};

GraphCutHelper::GraphCutHelper(int elements_num)
{
	m_elts = new Elements[elements_num];
	m_class_num = elements_num;

	for (int i = 0; i < elements_num; ++i)
	{
		m_elts[i].rank = 0;
		m_elts[i].size = 1;
		m_elts[i].p = i;
	}
}

GraphCutHelper::~GraphCutHelper()
{
	delete []m_elts;
}

int GraphCutHelper::find(int x)
{
	int y = x;
	while(y != m_elts[y].p)
	{
		y = m_elts[y].p;
	}
	m_elts[x].p = y;
	return y;
}

void GraphCutHelper::join(int x,int y)
{
	if (m_elts[x].rank > m_elts[y].rank)
	{
		m_elts[y].p = x;
		m_elts[x].size += m_elts[y].size;
	}
	else
	{
		m_elts[x].p = y;
		m_elts[y].size += m_elts[x].size;

		if (m_elts[x].rank == m_elts[y].rank)
		{
			m_elts[y].rank++;
		}
	}

	//the class number
	m_class_num--;
}

int GraphCutHelper::size(int x) const
{
	return m_elts[x].size;
}

bool operator<(const Edge &a,const Edge &b)
{
	return a.weight < b.weight;
}
//////////////////////////////////////////////////////////////////////////
template<class ImageSigT>
GraphCutNode<ImageSigT>::GraphCutNode()
{
	m_edges = NULL;
	m_c = 500;
	m_min_size = 100;
	m_labels_num = 0;
	m_rows = 0;
	m_cols = 0;

	//set output signal number
	setNumberOfOutputSignals(2);
	//label image
	setOutputPort(new ImageSignal<int>,0);
	//gray image
	setOutputPort(new ImageSignal<float>,1);
	
	//set input signal number
	setNumberOfInputSignals(1);

	//////////////////////////////////////////////////////////////////////////
	//build monitor variable
	EAGLEEYE_MONITOR_VAR(int,setMinRegionSize,getMinRegionSize,"min_region_size");
	EAGLEEYE_MONITOR_VAR(float,setCWeight,getCWeight,"c_weight");
	//////////////////////////////////////////////////////////////////////////
}

template<class ImageSigT>
GraphCutNode<ImageSigT>::~GraphCutNode()
{
	if (m_edges)
	{
		delete []m_edges;
		m_edges = NULL;
	}
}

template<class ImageSigT>
Matrix<int> GraphCutNode<ImageSigT>::getLabelMap()
{
	return m_label_map;
}

template<class ImageSigT>
Matrix<float> GraphCutNode<ImageSigT>::getGrayMap()
{
	return m_input_img;
}

template<class ImageSigT>
int GraphCutNode<ImageSigT>::getLabelNum()
{
	return m_labels_num;
}

template<class ImageSigT>
void GraphCutNode<ImageSigT>::executeNodeInfo()
{
	//get input image from input image signal
	ImageSigT* input_img_sig = TO_IMAGE_SIGNAL(PixelType,getInputPort(INPUT_PORT_IMAGE_DATA));
	if (!input_img_sig)
	{
		EAGLEEYE_ERROR("input image is not correct...\n");
		return;
	}

	Matrix<PixelType> input_img = input_img_sig->img;
	//normalize to 0~1
	PixelType max_value;
	PixelType min_value;
	getMaxMin(input_img,max_value,min_value);

	m_input_img = input_img.transform(NormalizeOperation<PixelType,float>(min_value,max_value,0,1));

	m_rows = m_input_img.rows();
	m_cols = m_input_img.cols();
	m_vertices_num = m_rows * m_cols;

	//construct a whole graph
	constructEdgeSet();

	//segment this graph
	segmentGraph(m_vertices_num,m_edges_num,m_edges,m_c);

	//set output image for output image signal
	ImageSignal<int>* label_image_signal = TO_IMAGE_SIGNAL(int,getOutputPort(OUTPUT_PORT_LABEL_DATA));
	ImageSignal<float>* gray_image_signal = TO_IMAGE_SIGNAL(float,getOutputPort(OUTPUT_PORT_GRAY_DATA));


	if ((!label_image_signal) || (!gray_image_signal))
	{
		EAGLEEYE_ERROR("sorry, output image is not correct...\n");
		return;
	}

	label_image_signal->img = m_label_map;
	gray_image_signal->img = m_gray_map;
}

template<class ImageSigT>
bool GraphCutNode<ImageSigT>::selfcheck()
{
	Superclass::selfcheck();

	ImageSigT* input_img_sig = TO_IMAGE_SIGNAL(PixelType,getInputPort(INPUT_PORT_IMAGE_DATA));
	if (!input_img_sig)
	{
		EAGLEEYE_ERROR("input image is not correct..\n");
		return false;
	}

	if (AtomicTypeTrait<PixelType>::size != 1)
	{
		EAGLEEYE_ERROR("image channel must be 1\n");
	}

	return true;
}

template<class ImageSigT>
void GraphCutNode<ImageSigT>::segmentGraph(int num_vertices,int num_edges,Edge* edges,float c)
{
	//sort edges by weight
	std::sort(edges,edges + num_edges);

	GraphCutHelper* helper = new GraphCutHelper(num_vertices);

	//initialize thresholds
	float* threshold = new float[num_vertices];

	for (int i = 0; i < num_vertices; ++i)
	{
		threshold[i] = (1.0f / c);
	}

	//for each edge, in non-decreasing weight order...
	for (int i = 0; i < num_edges; ++i)
	{
		Edge* pe = &edges[i];

		//components connected by this edge
		int label_a = helper->find(pe->start_index);
		int label_b = helper->find(pe->end_index);

		if (label_a != label_b)
		{
			if ((pe->weight <= threshold[label_a])&&
				(pe->weight <= threshold[label_b]))
			{
				helper->join(label_a,label_b);
				label_a = helper->find(label_a);

				//how to set threshold?
				threshold[label_a] = pe->weight + (helper->size(label_a) / c);
			}
		}
	}

	delete []threshold;

	//merge some small regions
	for (int i = 0; i < m_edges_num; ++i)
	{
		int label_a = helper->find(m_edges[i].start_index);
		int label_b = helper->find(m_edges[i].end_index);

		if ((label_a != label_b)&&
			((helper->size(label_a) < m_min_size) || (helper->size(label_b) < m_min_size)))
		{
			helper->join(label_a,label_b);
		}
	}

	//reassign the pixel label
	// the label saved in GraphCutHelper is not continuous
	m_label_map = Matrix<int>(m_rows,m_cols);
	std::map<int,int> new_label_map;
	std::map<int,float> mean_gray_map;
	m_labels_num = 0;
	
	for (int i = 0; i < m_rows; ++i)
	{
		int *label_data = m_label_map.row(i);
		float* input_img_data = m_input_img.row(i);

		for (int j = 0; j < m_cols; ++j)
		{
			//get old label of this pixel
			int old_label = helper->find(i * m_cols + j);
			
			//build one new label for this pixel
			std::map<int,int>::iterator check_iter = new_label_map.find(old_label);
			if (check_iter == new_label_map.end())
			{
				new_label_map[old_label] = m_labels_num;
				mean_gray_map[m_labels_num] = 0;
				m_labels_num++;
			}
			
			int new_label = new_label_map[old_label];
			mean_gray_map[new_label] += input_img_data[j] / helper->size(i * m_cols + j);
			label_data[j] = new_label;
		}
	}

	//generate gray map
	m_gray_map = Matrix<float>(m_rows,m_cols);
	for (int i = 0; i < m_rows; ++i)
	{
		int *label_data = m_label_map.row(i);
		float* gray_data = m_gray_map.row(i);
		for (int j = 0; j < m_cols; ++j)
		{
			gray_data[j] = mean_gray_map[label_data[j]];
		}
	}

	delete helper;
}

template<class ImageSigT>
void GraphCutNode<ImageSigT>::constructEdgeSet()
{
	//rows*cols*4 is greater than the actual need
	//m_edges_num is the number of edges
	if (m_edges)
	{
		delete []m_edges;
		m_edges = NULL;
	}

	m_edges = new Edge[m_rows * m_cols * 4];
	float* img_data = m_input_img.dataptr();

	m_edges_num = 0;
	for (int i = 0; i < m_rows; ++i)
	{
		for (int j = 0; j < m_cols; ++j)
		{
			if (j < m_cols - 1)
			{
				m_edges[m_edges_num].start_index = i * m_cols + j;
				m_edges[m_edges_num].end_index = i * m_cols + (j + 1);
				m_edges[m_edges_num].weight = abs(img_data[m_edges[m_edges_num].start_index] - img_data[m_edges[m_edges_num].end_index]);

				m_edges_num++;
			}

			if (i < m_rows - 1)
			{
				m_edges[m_edges_num].start_index = i * m_cols + j;
				m_edges[m_edges_num].end_index = (i + 1) * m_cols + j;
				m_edges[m_edges_num].weight = abs(img_data[m_edges[m_edges_num].start_index] - img_data[m_edges[m_edges_num].end_index]);

				m_edges_num++;
			}

			if ((j < m_cols - 1)&&(i < m_rows - 1))
			{
				m_edges[m_edges_num].start_index = i * m_cols + j;
				m_edges[m_edges_num].end_index = (i + 1) * m_cols + (j + 1);
				m_edges[m_edges_num].weight = abs(img_data[m_edges[m_edges_num].start_index] - img_data[m_edges[m_edges_num].end_index]);

				m_edges_num++;
			}

			if ((j < m_cols - 1)&&(i > 1))
			{
				m_edges[m_edges_num].start_index = i * m_cols + j;
				m_edges[m_edges_num].end_index = (i - 1) * m_cols + (j + 1);
				m_edges[m_edges_num].weight = abs(img_data[m_edges[m_edges_num].start_index] - img_data[m_edges[m_edges_num].end_index]);

				m_edges_num++;
			}
		}
	}
}

template<class ImageSigT>
void GraphCutNode<ImageSigT>::setMinRegionSize(const int min_size)
{
	m_min_size = min_size;

	//force to update
	modified();
}

template<class ImageSigT>
void GraphCutNode<ImageSigT>::getMinRegionSize(int& min_size)
{
	min_size = m_min_size;
}

template<class ImageSigT>
void GraphCutNode<ImageSigT>::setCWeight(const float c_weight)
{
	m_c = c_weight;
	
	//force to update
	modified();
}

template<class ImageSigT>
void GraphCutNode<ImageSigT>::getCWeight(float& c_weight)
{
	c_weight = m_c;
}

}