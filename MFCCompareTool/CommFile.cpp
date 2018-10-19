#include "stdafx.h"
#include "CommFile.h"

CommFile_Ptr CommFile::m_p = std::make_shared<CommFile>();
CommFile_Ptr CommFile::Instance()
{
	return m_p;
}

CString CommFile::GetPluginPath()
{
	TCHAR szDir[MAX_PATH] = "0";
	GetCurrentDirectory(MAX_PATH, szDir);
	CString dir(szDir);
	int nIndex = dir.ReverseFind(_T('\\'));
	if (-1 == nIndex)
	{
		return CString();
	}

	dir = dir.Mid(0, nIndex);
	dir += "\\Plugin";
	CDir


	return CString();
}

BOOL CommFile::IsFileExist(const CString & csFile)
{
	DWORD dwAttrib = GetFileAttributes(csFile); 
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY); 
}
BOOL CommFile::IsDirExist(const CString & csDir)
{
	DWORD dwAttrib = GetFileAttributes(csDir);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
BOOL CommFile::IsPathExist(const CString & csPath)
{
	return 0;
}

CommFile::CommFile()
{
}


