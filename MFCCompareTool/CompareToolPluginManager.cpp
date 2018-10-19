#include "stdafx.h"
#include "CompareToolPluginManager.h"

CompareToolPluginManager_Ptr CompareToolPluginManager::m_p = std::make_shared<CompareToolPluginManager>();

BOOL CompareToolPluginManager::Init()
{
	TCHAR szDir[MAX_PATH] = "0";
	GetCurrentDirectory(MAX_PATH, szDir);
	CFile file(szDir);
	CString dir(szDir);

	return 0;
}

CompareToolPluginManager::CompareToolPluginManager()
{
}


CompareToolPluginManager_Ptr CompareToolPluginManager::Instance()
{
	return m_p;
}
