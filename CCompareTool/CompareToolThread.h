#ifndef COMPARETOOLDELTHREAD_H
#define COMPARETOOLDELTHREAD_H

#include <QThread>

typedef std::pair<QString,QString>  PairPath;
typedef QList< PairPath > PathToPathList;
class CompareToolThread : public QThread
{
	Q_OBJECT

public:
	enum CompareToolThreadType
	{
		CompareToolThreadDel,
		CompareToolThreadCopy,
		CompareToolThreadDiff
	};
	enum CompareToolThreadResult
	{
		ThreadDelSucess,
		ThreadDelFail,
		ThreadMoveSucess,
		ThreadMoveFail,
		ThreadOutputSucess,
		ThreadOutputFail
	};

	CompareToolThread(QObject *parent, CompareToolThreadType type);
	~CompareToolThread();

	//如果是CompareToolThreadDel 不用设置toDir
	void SetPath(const QStringList & sourceDir);
	void SetCopy(const PathToPathList & list);
	void SetDifference(const PathToPathList & diff, const PathToPathList & copy);
	void AddNameFilter(const QRegExp & reg);
	void SetDataTimeFilter(const QDateTime & time);

	CompareToolThreadResult remove( const QStringList & indexList ) ;
	//TODU:添加相同文件时不覆盖
	CompareToolThreadResult move(const PathToPathList & indexList);
	//Output	key为当前文件的绝对路径 value为拷贝到目标位置的绝对路径
	//			返回差异文件
	CompareToolThreadResult OutputFile(const PathToPathList & Diff, PathToPathList & Output);


signals:
	void ResultSignal(CompareToolThreadResult resu);
protected:
	void run();

private:
	bool copyFileToPath(QString sourceDir ,QString toDir, bool coverFileIfExist = true)const;
	bool copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool coverFileIfExist = true)const;
	bool move(const QString &index, const QString & toDir, bool coverFileIfExist = true) const;
	bool EqualFile(const QString &index, const QString & toDir) const;
	
	//删除可能会出问题  程序占用文件夹 当程序结束时才完全删除文件
	bool removeDir(const QString & dir) const;
	//bool removeFile(const QString & filrPath) const;
	bool passNameFilters(const QString & fileName) const;
	QByteArray getFileMd5(QString filePath) const;


	QMutex					m_Mutex;
	CompareToolThreadType	m_Type;
	QList<QRegExp>			m_NameFilters;
	bool					m_FilterDisables;
	QStringList				m_SouceDir;
	QDateTime				m_DateTimeLimit;
	PathToPathList		 	m_ToDir;
	PathToPathList		 	m_DiffDir;
//	QWaitCondition condition;
//	double centerX;
//	double centerY;
//	double scaleFactor;
//	QSize resultSize;
//	bool restart;
//	bool abort;
//
//	enum { ColormapSize = 512 };
//	uint colormap[ColormapSize];
	
};

#endif // COMPARETOOLDELTHREAD_H
