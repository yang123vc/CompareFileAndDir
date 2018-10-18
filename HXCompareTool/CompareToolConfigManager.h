#pragma once
#include <qobject.h>
#include "pugixml.hpp"

class CompareToolConfigManagerPrivate;
class CompareToolConfigManager : public QObject
{
	Q_OBJECT
		Q_DECLARE_PRIVATE(CompareToolConfigManager)
public:
	static CompareToolConfigManager * Instance();
	~CompareToolConfigManager(void);

	bool Load(const QString & PathName);


	void AddRightList(const QString & text);
	void AddLeftList(const QString & text);
	//只保存当前导出路径
	void AddOutput(const QString & text);

	QStringList Output() const;
	QStringList LeftList() const;
	QStringList RightList() const;

	//释放时自动调用
	void SaveConfig();
private:
	enum ConfigType
	{
		RightConfig,
		LeftConfig
	};
	//添加到配置字符串
	void AddList(const QString & text, QStringList & list);
	void SaveNode(const QStringList & list, pugi::xml_node & node);

	void LoadItem(QStringList & config, const pugi::xml_node & node);

	CompareToolConfigManager();


	Q_DISABLE_COPY(CompareToolConfigManager)

};

