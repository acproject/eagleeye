#include "AnyUnit.h"
#include "Array.h"
#include "Print.h"
#include "EagleeyeIO.h"
#include "EagleeyeStr.h"
#include "tinyxml/tinyxml.h"

namespace eagleeye
{
AnyUnit::AnyUnit(const char* unit_name)
{
	m_unit_name = unit_name;
	m_unit_config_file = "";

	m_load_config_param_flag = false;
	m_pipeline_name = "eagleeye";
	modified();
}

AnyUnit::~AnyUnit()
{
	std::vector<AnyMonitor*>::iterator iter,iend(m_unit_monitor_pool.end());
	for (iter = m_unit_monitor_pool.begin(); iter != iend; ++iter)
	{
		delete (*iter);
	}
}

AnyMonitor* AnyUnit::getMonitor(int index)
{
	if (index < int(m_unit_monitor_pool.size()))
	{
		return m_unit_monitor_pool[index];
	}
	
	return NULL;
}

int AnyUnit::getMonitorPoolSize()
{
	return m_unit_monitor_pool.size();
}

unsigned long AnyUnit::getMTime() const
{
	return m_mtime.getMTime();
}
void AnyUnit::modified() const
{
	m_mtime.modified();
}

bool AnyUnit::saveUnitConfig()
{
	if (!m_unit_config_file.empty())
	{
		//if configure file doesn't exist, create one empty xml
		TiXmlDocument doc(m_unit_config_file.c_str());
		if (!doc.LoadFile())
		{
			//build one empty xml
			doc = TiXmlDocument();
		}

		TiXmlHandle h_doc(&doc);
		if (!h_doc.FirstChild(m_pipeline_name.c_str()).Element())
		{
			//build one item
			TiXmlElement global_node(m_pipeline_name.c_str());
			h_doc.ToNode()->InsertEndChild(global_node);
		}

		if (!h_doc.FirstChild(m_pipeline_name.c_str()).FirstChild(getUnitName()).Element())
		{
			//build one item
			TiXmlElement unit_node(getUnitName());
			h_doc.FirstChild(m_pipeline_name.c_str()).ToNode()->InsertEndChild(unit_node);
		}

		std::vector<AnyMonitor*>::iterator iter,iend(m_unit_monitor_pool.end());
		for (iter = m_unit_monitor_pool.begin(); iter != iend; ++iter)
		{
			AnyMonitor* var_monitor = *iter;

			if (!h_doc.FirstChild(m_pipeline_name.c_str()).FirstChild(getUnitName()).FirstChild(var_monitor->monitor_var_text).Element())
			{
				//build one item
				TiXmlElement var_monitor_node(var_monitor->monitor_var_text);
				h_doc.FirstChild(m_pipeline_name.c_str()).FirstChild(getUnitName()).ToNode()->InsertEndChild(var_monitor_node);
			}

			TiXmlElement* p_elem = h_doc.FirstChild(m_pipeline_name.c_str()).FirstChild(getUnitName()).FirstChild(var_monitor->monitor_var_text).Element();

			switch(var_monitor->monitor_var_type)
			{
			case EAGLEEYE_MONITOR_CHAR:
			case EAGLEEYE_MONITOR_UCHAR:
				{
					unsigned char val;
					var_monitor->getVar(&val);
					p_elem->SetAttribute("value",int(val));

					break;
				}
			case EAGLEEYE_MONITOR_SHORT:
			case EAGLEEYE_MONITOR_USHORT:
				{
					unsigned short val;
					var_monitor->getVar(&val);
					p_elem->SetAttribute("value",int(val));

					break;
				}
			case EAGLEEYE_MONITOR_INT:
			case EAGLEEYE_MONITOR_UINT:
				{
					unsigned int val;
					var_monitor->getVar(&val);
					p_elem->SetAttribute("value",int(val));

					break;
				}
			case EAGLEEYE_MONITOR_FLOAT:
				{
					float val;
					var_monitor->getVar(&val);
					p_elem->SetDoubleAttribute("value",double(val));

					break;
				}
			case EAGLEEYE_MONITOR_DOUBLE:
				{
					double val;
					var_monitor->getVar(&val);
					p_elem->SetDoubleAttribute("value",double(val));

					break;
				}
			case EAGLEEYE_MONITOR_RGB:
				{
					ERGB val;
					var_monitor->getVar(&val);

					p_elem->SetAttribute("red",int(val[0]));
					p_elem->SetAttribute("green",int(val[1]));
					p_elem->SetAttribute("blue",int(val[2]));

					break;
				}
			case EAGLEEYE_MONITOR_RGBA:
				{
					ERGBA val;
					var_monitor->getVar(&val);

					p_elem->SetAttribute("red",int(val[0]));
					p_elem->SetAttribute("green",int(val[1]));
					p_elem->SetAttribute("blue",int(val[2]));
					p_elem->SetAttribute("alpha",int(val[3]));
					break;
				}
			case EAGLEEYE_MONITOR_STR:
				{
					std::string* str = NULL;
					var_monitor->getVar(str);
					p_elem->SetValue((*str).c_str());
					break;
				}
			case EAGLEEYE_MONITOR_UNDEFINED:
				{
					//special process
					break;
				}
			}
		}		

		doc.SaveFile(m_unit_config_file.c_str());
		return true;
	}

	return false;
}

bool AnyUnit::loadUnitConfig()
{
	if ((!m_unit_config_file.empty()) && (!m_load_config_param_flag))
	{
		TiXmlDocument doc(m_unit_config_file.c_str());
		if (!doc.LoadFile())
		{
			EAGLEEYE_ERROR("load configure file %s fail...\n",m_unit_config_file.c_str());
			return false;
		}
		else
		{
			TiXmlHandle h_doc(&doc);
			TiXmlElement* p_elem;

			std::vector<AnyMonitor*>::iterator iter,iend(m_unit_monitor_pool.end());
			for (iter = m_unit_monitor_pool.begin(); iter != iend; ++iter)
			{
				AnyMonitor* var_monitor = *iter;
				p_elem = h_doc.FirstChild(m_pipeline_name.c_str()).FirstChild(getUnitName()).FirstChild(var_monitor->monitor_var_text).Element();
				if (p_elem)
				{
					switch(var_monitor->monitor_var_type)
					{
					case EAGLEEYE_MONITOR_CHAR:
					case EAGLEEYE_MONITOR_UCHAR:
						{
							int val;
							if(p_elem->QueryIntAttribute("value",&val) == TIXML_SUCCESS)
							{
								char char_val = (char)val;
								var_monitor->setVar(&char_val);
							}
							break;
						}
					case EAGLEEYE_MONITOR_SHORT:
					case EAGLEEYE_MONITOR_USHORT:
						{
							int val;
							if(p_elem->QueryIntAttribute("value",&val) == TIXML_SUCCESS)
							{
								short short_val = (short)val;
								var_monitor->setVar(&short_val);
							}
							break;
						}
					case EAGLEEYE_MONITOR_INT:
					case EAGLEEYE_MONITOR_UINT:
						{
							int val;
							if(p_elem->QueryIntAttribute("value",&val) == TIXML_SUCCESS)
							{
								var_monitor->setVar(&val);
							}
							break;
						}
					case EAGLEEYE_MONITOR_FLOAT:
						{
							float val;
							if(p_elem->QueryFloatAttribute("value",&val) == TIXML_SUCCESS)
							{
								var_monitor->setVar(&val);
							}
							break;
						}
					case EAGLEEYE_MONITOR_DOUBLE:
						{
							double val;
							if(p_elem->QueryDoubleAttribute("value",&val) == TIXML_SUCCESS)
							{
								var_monitor->setVar(&val);
							}
							break;
						}
					case EAGLEEYE_MONITOR_RGB:
						{
							int r_val,g_val,b_val;
							r_val = 0; g_val = 0; b_val = 0;

							if (p_elem->QueryIntAttribute("red",&r_val) != TIXML_SUCCESS)
								continue;
							if (p_elem->QueryIntAttribute("green",&g_val) != TIXML_SUCCESS)
								continue;
							if (p_elem->QueryIntAttribute("blue",&b_val) != TIXML_SUCCESS)
								continue;
							
							ERGB rgb_val;
							rgb_val[0] = unsigned char(r_val); rgb_val[1] = unsigned char(g_val); rgb_val[2] = unsigned char(b_val);

							var_monitor->setVar(&rgb_val);

							break;
						}
					case EAGLEEYE_MONITOR_RGBA:
						{
							int r_val,g_val,b_val,a_val;
							r_val = 0; g_val = 0; b_val = 0; a_val = 0;

							if (p_elem->QueryIntAttribute("red",&r_val) != TIXML_SUCCESS)
								continue;
							if (p_elem->QueryIntAttribute("green",&g_val) != TIXML_SUCCESS)
								continue;
							if (p_elem->QueryIntAttribute("blue",&b_val) != TIXML_SUCCESS)
								continue;
							if (p_elem->QueryIntAttribute("alpha",&a_val) != TIXML_SUCCESS)
								continue;

							ERGBA rgba_val;
							rgba_val[0] = r_val; rgba_val[1] = g_val; rgba_val[2] = b_val; rgba_val[3] = a_val;
							var_monitor->setVar(&rgba_val);

							break;
						}
					case EAGLEEYE_MONITOR_STR:
						{
							std::string str;
							str = p_elem->GetText();
							if (!str.empty())
								var_monitor->setVar(&str);
							break;
						}
					case EAGLEEYE_MONITOR_UNDEFINED:
						{
							//special process
							break;
						}
					}
				}
			}

			m_load_config_param_flag = true;
			return true;
		}
	}

	return false;
}

void AnyUnit::setConfigFile(const char* config_file)
{
	m_unit_config_file = config_file;
}

void AnyUnit::setPipelineName(const char* pipeline_name)
{
	m_pipeline_name = pipeline_name;
}

void AnyUnit::wrapToUnitBlock(MemoryBlock& unit_block)
{
	MemoryBlock param_block;
	getUnitPara(param_block);

	std::string class_id = getClassIdentity();
	
	unit_block = MemoryBlock(EAGLEEYE_MAX_NAME + param_block.blockSize());

	char* unit_block_ptr = (char*)unit_block.block();
	copystr(unit_block_ptr,class_id.c_str());
	memcpy(unit_block_ptr + EAGLEEYE_MAX_NAME,param_block.block(),param_block.blockSize());
}

}