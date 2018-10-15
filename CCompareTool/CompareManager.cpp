#include "StdAfx.h"

#include <QHash>
#include <QModelIndex>

#include "CompareManager.h"

class CompareManagerImpl
{
public:
	//ÅäŒ¦ÉÏµÄ
	QHash<QModelIndex,QModelIndex> m_Compare;
};

CompareManager::CompareManager(void)
	:m_p(new CompareManagerImpl())
{
}


CompareManager::~CompareManager(void)
{
	SafeDeletePoint<CompareManagerImpl>(m_p);
}

CompareManager * CompareManager::Instance()
{
	static CompareManager instan;
	return &instan;
}
