#include "EagleeyeIO.h"

namespace eagleeye
{
MemoryBlock EagleeyeIO::m_io_info_block;

bool EagleeyeIO::createWriteHandle(std::string file_path,bool isapp,EagleeyeIOMode iomode/* =WriteAsciiMode */)
{
	m_iomode=iomode;
	switch(m_iomode)
	{
	case WRITE_ASCII_MODE:
		{
			if (isapp)
			{
				m_o_file_handle.open(file_path.c_str(),std::ios::app);
			}
			else
			{
				m_o_file_handle.open(file_path.c_str());
			}

			if (!m_o_file_handle)
			{
				m_iomode=UNDEFINED_IO;
				return false;
			}
			else
				return true;
		}
	case WRITE_BINARY_MODE:
		{
			if (isapp)
			{
				m_o_file_handle.open(file_path.c_str(),std::ios::app|std::ios::binary|std::ios::out);
			}
			else
			{
				m_o_file_handle.open(file_path.c_str(),std::ios::binary);
			}
			
			if (!m_o_file_handle)
			{
				m_iomode=UNDEFINED_IO;
				return false;
			}
			else
			{
				return true;
			}
		}
	default:
		{
			return false;
		}
	}

	return false;
}

bool EagleeyeIO::createReadHandle(std::string file_path,EagleeyeIOMode iomode/* =ReadAsciiMode */)
{
	m_iomode=iomode;
	switch(m_iomode)
	{
	case READ_ASCII_MODE:
		{
			m_i_file_handle.open(file_path.c_str());
			if (!m_i_file_handle)
			{
				m_iomode=UNDEFINED_IO;
				return false;
			}
			else
				return true;
		}
	case READ_BINARY_MODE:
		{
			m_i_file_handle.open(file_path.c_str(),std::ios::binary);
			if (!m_i_file_handle)
			{
				m_iomode=UNDEFINED_IO;
				return false;
			}
			else
			{
				return true;
			}
		}
	default:
		{
			return false;
		}
	}

	return false;
}

bool EagleeyeIO::destroyHandle()
{
	switch(m_iomode)
	{
	case READ_ASCII_MODE:
	case READ_BINARY_MODE:
		{
			m_i_file_handle.close();
			return true;
		}
	case WRITE_ASCII_MODE:
	case WRITE_BINARY_MODE:
		{
			m_o_file_handle.close();
			return true;
		}
	}
	return false;
}

bool EagleeyeIO::isexist(std::string filename)
{
	std::ifstream file_check(filename.c_str());
	if (!file_check)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool EagleeyeIO::write(std::string str)
{
	switch(m_iomode)
	{
	case WRITE_ASCII_MODE:
		{
			m_o_file_handle<<str<<'\n';
			return true;
		}
	case WRITE_BINARY_MODE:
		{
			int str_len=str.length();
			m_o_file_handle.write((char*)(&str_len),sizeof(int));
			m_o_file_handle.write(str.c_str(),sizeof(char)*str_len);
			return true;
		}
	}

	return false;
}

bool EagleeyeIO::write(const char* str)
{
	const char* temp_str=str;
	switch(m_iomode)
	{
	case WRITE_ASCII_MODE:
		{
			while((*temp_str)!='\0')
			{
				m_o_file_handle<<*temp_str;
				temp_str=temp_str+1;
			}

			m_o_file_handle<<'\n';
			return true;
		}
	case WRITE_BINARY_MODE:
		{
			int str_len=0;
			while((*temp_str)!='\0')
			{
				str_len++;
				temp_str=temp_str+1;
			}

			m_o_file_handle.write((char*)(&str_len),sizeof(int));
			m_o_file_handle.write(str,sizeof(char)*str_len);

			break;
		}
	}
	return false;
}

bool EagleeyeIO::read(std::string& str)
{
	switch(m_iomode)
	{
	case READ_ASCII_MODE:
		{
			m_i_file_handle>>str;
			return true;
		}
	case READ_BINARY_MODE:
		{
			int str_len;
			m_i_file_handle.read((char*)(&str_len),sizeof(int));
			
			char* data=new char[str_len+1];
			m_i_file_handle.read(data,sizeof(char)*str_len);
			data[str_len]='\0';
			str=std::string(data);
			delete data;

			return true;
		}
	}

	return false;
}

bool EagleeyeIO::write(const void* info,int size)
{
	switch(m_iomode)
	{
	case WRITE_BINARY_MODE:
		{
			m_o_file_handle.write((const char*)(&size),sizeof(int));
			m_o_file_handle.write((const char*)(info),sizeof(char)*size);
			return true;
		}
	}
	return false;
}

bool EagleeyeIO::read(void*& info,int& size)
{
	switch(m_iomode)
	{
	case READ_BINARY_MODE:
		{
			m_i_file_handle.read((char*)(&size),sizeof(int));

			m_io_info_block=MemoryBlock(size);
			m_i_file_handle.read((char*)(m_io_info_block.block()),sizeof(char)*size);
			info=m_io_info_block.block();
			return true;
		}
	}

	return false;
}

bool EagleeyeIO::deletefile(std::string filename)
{
	if (remove(filename.c_str()))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void EagleeyeIO::destroyIOMemRes()
{
	m_io_info_block=MemoryBlock();
}
}