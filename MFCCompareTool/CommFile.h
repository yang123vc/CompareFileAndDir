#pragma once

DECLARE_PTR(CommFile)
class CommFile :
	public CObject
{
	DECLARE_DYNAMIC(CommFile)
	DISABLE_COPY(CommFile);
public:
	static CommFile_Ptr Instance();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Gets plugin path. </summary>
	///
	/// <remarks>	Hx, 2018/10/20. </remarks>
	///
	/// <returns>	The plugin path. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	CString GetPluginPath();

	// 判断文件是否存在 
	BOOL IsFileExist(const CString& csFile);
	 // 判断文件夹是否存在 
	BOOL IsDirExist(const CString & csDir);// { DWORD dwAttrib = GetFileAttributes(csDir); return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY); }
	 // 判断文件或文件夹是否存在 
	 BOOL IsPathExist(const CString & csPath);// { DWORD dwAttrib = GetFileAttributes(csPath); return INVALID_FILE_ATTRIBUTES != dwAttrib; }
	 // 变变变变变种(听说会更快一点)，见备注1 
	 BOOL IsPathExist(const CString & csPath);// { WIN32_FILE_ATTRIBUTE_DATA attrs = { 0 }; return 0 != GetFileAttributesEx(csPath, GetFileExInfoStandard, &attrs); }


private:
	explicit CommFile();
	static CommFile_Ptr m_p;
};
