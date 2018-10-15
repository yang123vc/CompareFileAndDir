#include <QtGui>

#include "CompareToolFileModel_p.h"
//#include "CompareToolFileModel.h"

#define IN
#define OUT

const QString g_strConfigName = "config.xml";

const int g_nFileName = 0;
//文件类型所在列数
const int g_nFileType = 2;
//最后修改时间
const int g_nFileTime = 3;

const QColor g_FindNotColor = QColor(252,230,230, 200);
const QColor g_DiffColor = QColor(255,0,0, 200);
const QColor g_DefColor = QColor(255,255,255);

const QDateTime g_DefTime = QDateTime();

const int g_nColumn = 3;

template<typename T>
inline void SafeDeletePoint(T * p)
{
	if (p)
	{
		delete p;
		p = NULL;
	}
}
