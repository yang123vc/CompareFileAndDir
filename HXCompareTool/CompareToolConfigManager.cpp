#include "StdAfx.h"

#include "pugixml.hpp"

#include "CompareToolConfigManager_p.h"
#include "CompareToolConfigManager.h"


//CompareToolConfigManager * CompareToolConfigManager::d = new CompareToolConfigManager();

CompareToolConfigManager::CompareToolConfigManager(void)
	:QObject(*new CompareToolConfigManagerPrivate(), 0)
{
}


CompareToolConfigManager::~CompareToolConfigManager(void)
{
	SaveConfig();
	//SafeDeletePoint<CompareToolConfigManagerImpl>(d);
}

CompareToolConfigManager * CompareToolConfigManager::Instance()
{
	static CompareToolConfigManager instance;     
	return &instance;   
}

bool CompareToolConfigManager::Load( const QString & PathName )
{
	Q_D(CompareToolConfigManager);
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
	LoadItem(d->m_Right, RightNode);
	xml_node LeftNode = configNode.child("Left");
	LoadItem(d->m_Left, LeftNode);


	xml_node OutputNode = configNode.child("Output");
	LoadItem(d->m_Output, OutputNode);

	//SaveConfig();
	return true;
}


void CompareToolConfigManager::AddList( const QString & text, QStringList & list )
{
	Q_D(CompareToolConfigManager);
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
		d->m_HaveChangeConfig = true;
		list.push_back(text);
	}
}

void CompareToolConfigManager::AddRightList( const QString & text )
{
	Q_D(CompareToolConfigManager);
	AddList(text, d->m_Right);
}

void CompareToolConfigManager::AddLeftList( const QString & text )
{
	Q_D(CompareToolConfigManager);
	AddList(text, d->m_Left);
}

void CompareToolConfigManager::SaveConfig()
{
	Q_D(CompareToolConfigManager);
	if(d->m_HaveChangeConfig)
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
		SaveNode(d->m_Left, nodeLeft);

		//nodeLeft.append_child("Item").append_attribute("path").set_value("C:");
		//nodeLeft.append_child("Item").append_attribute("path").set_value("D:");
		pugi::xml_node nodeRight = nodeRoot.append_child( "Right" );
		SaveNode(d->m_Right, nodeRight);
		//nodeRight.append_child("Item").append_attribute("path").set_value("E:");
		//nodeRight.append_child("Item").append_attribute("path").set_value("F:");

		pugi::xml_node nodeOutput = nodeRoot.append_child( "Output" );
		SaveNode(d->m_Output, nodeOutput);
		
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
	Q_D(CompareToolConfigManager);
	d->m_Output.clear();
	AddList(text, d->m_Output);
}

QStringList CompareToolConfigManager::Output() const
{
	Q_D(CompareToolConfigManager);
	return d->m_Output;
}

QStringList CompareToolConfigManager::LeftList() const
{
	Q_D(CompareToolConfigManager);
	return d->m_Left;
}

QStringList CompareToolConfigManager::RightList() const
{
	Q_D(CompareToolConfigManager);
	return d->m_Right;
}
