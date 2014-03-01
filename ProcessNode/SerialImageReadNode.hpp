namespace eagleeye
{
template<class T>
SerialImageReadNode<T>::SerialImageReadNode()
{
	m_search_folder = "";
	m_search_word = "";
	m_current_file_index = -2;

	m_search_flag = false;
	m_find_mode = FIND_CURRENT_DIR;
}

template<class T>
SerialImageReadNode<T>::~SerialImageReadNode()
{

}

template<class T>
void SerialImageReadNode<T>::setSearchFolder(const char* search_folder,const char* search_word)
{
	m_search_folder = search_folder;
	m_search_word = search_word;

	//modify the update time
	modified();

	m_search_flag = true;
}

template<class T>
bool SerialImageReadNode<T>::next()
{
	if (((m_current_file_index+1) >= 0)&&((m_current_file_index+1) < int(m_files_list.size())))
	{
		m_current_file_index++;

		setFilePath(m_files_list[m_current_file_index].c_str());
		return true;
	}

	return false;
}

template<class T>
void SerialImageReadNode<T>::passonNodeInfo()
{
	if (m_search_flag)
	{
		m_files_list.clear();
		switch(m_find_mode)
		{
		case FIND_CURRENT_DIR:
			{
				//build file list
				core::FileManager::getFileList(m_search_folder.c_str(),m_search_word.c_str(),m_files_list);
				break;
			}
		case FIND_ALL_DIR:
			{
				core::FileManager::getAllFileList(m_search_folder.c_str(),m_search_word.c_str(),m_files_list);
				break;
			}
		}
		m_current_file_index = -1;
	}
	
	m_search_flag = false;

	//call the superclass function
	ImageReadNode::passonNodeInfo();
}

template<class T>
void SerialImageReadNode<T>::executeNodeInfo()
{
	if (m_current_file_index >= 0 && m_current_file_index < int(m_files_list.size()))
	{
		ImageReadNode::executeNodeInfo();
	}
}

template<class T>
bool SerialImageReadNode<T>::selfcheck()
{
	if (m_search_folder == "")
	{
		EAGLEEYE_ERROR("sorry, search folder is empty...\n");
		return false;
	}

	return true;
}

template<class T>
void SerialImageReadNode<T>::setFindMode(FindFileMode find_mode)
{
	m_find_mode = find_mode;
}

}