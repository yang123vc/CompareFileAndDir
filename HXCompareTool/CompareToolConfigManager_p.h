#pragma once

#include <QtCore/private/qobject_p.h>
class CompareToolConfigManager;
class CompareToolConfigManagerPrivate : public QObjectPrivate
{
	Q_DECLARE_PUBLIC(CompareToolConfigManager)
public:
	QStringList m_Right;
	QStringList m_Left;
	QStringList	m_Output;
	bool		m_HaveChangeConfig;
	CompareToolConfigManagerPrivate() :m_HaveChangeConfig(false)
	{}
};
