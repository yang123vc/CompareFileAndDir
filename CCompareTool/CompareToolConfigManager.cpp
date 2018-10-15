#include "StdAfx.h"

#include "pugixml.hpp"

#include "CompareToolConfigManager.h"


class CompareToolConfigManagerImpl
{
public:
	QStringList m_Right;
	QStringList m_Left;
	QStringList	m_Output;
	bool		m_HaveChangeConfig;
	CompareToolConfigManagerImpl():m_HaveChangeConfig(false)
	{}
};
//CompareToolConfigManager * CompareToolConfigManager::m_p = new CompareToolConfigManager();

CompareToolConfigManager::CompareToolConfigManager(void):m_p(new CompareToolConfigManagerImpl())
{
}


CompareToolConfigManager::~CompareToolConfigManager(void)
{
	SaveConfig();
	SafeDeletePoint<CompareToolConfigManagerImpl>(m_p);
}

CompareToolConfigManager * CompareToolConfigManager::Instance()
{
	static CompareToolConfigManager instance;     
	return &instance;   
}

bool CompareToolConfigManager::Load( const QString & PathName )
{
	//SaveConfig();
	if (PathName == "")
	{
		return false;
	}
	using namespace pugi;
	xml_document doc;
	xml_parse_result result = doc.load_file("config.xml", pugi::parse_default);
	if (result.status != status_ok)
	{
		return false;
	}

	xml_node configNode = doc.child("Config");
	xml_node RightNode = configNode.child("Right");
	LoadItem(m_p->m_Right, RightNode);
	xml_node LeftNode = configNode.child("Left");
	LoadItem(m_p->m_Left, LeftNode);


	xml_node OutputNode = configNode.child("Output");
	LoadItem(m_p->m_Output, OutputNode);

	//SaveConfig();
	return true;
}


void CompareToolConfigManager::AddList( const QString & text, QStringList & list )
{
	bool bEque = false;
	Q_FOREACH(const QString & str, list)
	{
		if (str == text)
		{
			bEque = true;
			break;
		}
	}
	if (!bEque)
	{
		m_p->m_HaveChangeConfig = true;
		list.push_back(text);
	}
}

void CompareToolConfigManager::AddRightList( const QString & text )
{
	AddList(text, m_p->m_Right);
}

void CompareToolConfigManager::AddLeftList( const QString & text )
{
	AddList(text, m_p->m_Left);
}

void CompareToolConfigManager::SaveConfig()
{
	if(m_p->m_HaveChangeConfig)
	{
		QString path = "config.xml";
		QFile file(path);
		if (file.exists())
		{
			file.remove();
		}
		//¢òÖØÐ´.XMLÎÄ¼þ;
		pugi::xml_document doc;
		pugi::xml_node nodeDeclaration = doc.append_child( pugi::node_declaration );
		nodeDeclaration.set_name( "xml" );
		nodeDeclaration.append_attribute( "version" ).set_value( "1.0" );
		nodeDeclaration.append_attribute( "encoding" ).set_value( "utf-8" );

		pugi::xml_node nodeRoot = doc.append_child( "Config" );
		pugi::xml_node nodeLeft = nodeRoot.append_child( "Left" );
		SaveNode(m_p->m_Left, nodeLeft);

		//nodeLeft.append_child("Item").append_attribute("path").set_value("C:");
		//nodeLeft.append_child("Item").append_attribute("path").set_value("D:");
		pugi::xml_node nodeRight = nodeRoot.append_child( "Right" );
		SaveNode(m_p->m_Right, nodeRight);
		//nodeRight.append_child("Item").append_attribute("path").set_value("E:");
		//nodeRight.append_child("Item").append_attribute("path").set_value("F:");

		pugi::xml_node nodeOutput = nodeRoot.append_child( "Output" );
		SaveNode(m_p->m_Output, nodeOutput);
		
		doc.save_file(path.toStdString().c_str(), "\t", 1U, pugi::encoding_utf8);;
		
	}
}

void CompareToolConfigManager::LoadItem( QStringList & configList, const pugi::xml_node & node )
{
	pugi::xml_node tempNode = node.first_child();
	while(tempNode)
	{
		QString temp = tempNode.attribute("path").value();
		if (temp != "")
		{
			configList.push_back(temp);
		}
		tempNode = tempNode.next_sibling();
	}
}

void CompareToolConfigManager::SaveNode( const QStringList & list, pugi::xml_node & node )
{
	Q_FOREACH(const QString & str, list)
	{
		node.append_child("Item").append_attribute("path").set_value(str.toStdString().c_str());
	}
}

void CompareToolConfigManager::AddOutput( const QString & text )
{
	m_p->m_Output.clear();
	AddList(text, m_p->m_Output);
}

QStringList CompareToolConfigManager::Output() const
{
	return m_p->m_Output;
}

QStringList CompareToolConfigManager::LeftList() const
{
	return m_p->m_Left;
}

QStringList CompareToolConfigManager::RightList() const
{
	return m_p->m_Right;
}
