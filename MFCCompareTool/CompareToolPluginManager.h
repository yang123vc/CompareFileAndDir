#pragma once
#include <afx.h>




DECLARE_PTR(CompareToolPluginManager)
class CompareToolPluginManager :
	public CObject
{
	DECLARE_DYNAMIC(CompareToolPluginManager)
	DISABLE_COPY(CompareToolPluginManager);
public:
	static CompareToolPluginManager_Ptr Instance();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>	Initializes this object. </summary>
	///
	/// <remarks>	��ȡ�����ļ� ��ò����Ϣ�б�
	/// 			.
	/// 			Hx, 2018/10/19. </remarks> 			 
	///
	/// <param name="conf">	The conf. </param>
	///
	/// <returns>	True if it succeeds, false if it fails. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////

	BOOL Init();


private:
	explicit CompareToolPluginManager();
	static CompareToolPluginManager_Ptr m_p;
};

