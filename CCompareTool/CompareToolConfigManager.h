#pragma once

#include "pugixml.hpp"

class CompareToolConfigManagerImpl;
class CompareToolConfigManager
{

public:
	static CompareToolConfigManager * Instance();
	~CompareToolConfigManager(void);

	bool Load(const QString & PathName);


	void AddRightList(const QString & text);
	void AddLeftList(const QString & text);
	//ֻ���浱ǰ����·��
	void AddOutput(const QString & text);

	QStringList Output() const;
	QStringList LeftList() const;
	QStringList RightList() const;

	//�ͷ�ʱ�Զ�����
	void SaveConfig();
private:
	enum ConfigType
	{
		RightConfig,
		LeftConfig
	};
	//��ӵ������ַ���
	void AddList(const QString & text, QStringList & list);
	void SaveNode(const QStringList & list, pugi::xml_node & node);

	void LoadItem(QStringList & config, const pugi::xml_node & node);

	CompareToolConfigManager();
	CompareToolConfigManagerImpl * m_p;

};

