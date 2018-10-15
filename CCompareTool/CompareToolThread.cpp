#include "StdAfx.h"

#include <algorithm>


#include "MessageBoxOtherThread.h"
#include "CompareToolThread.h"

//CompareToolThread::CompareToolThread(QObject *parent, CompareToolThread::CompareToolThreadType type)
//	: QThread(parent), m_Type(type), m_NameFilterDisables(false)
//{
//
//}

CompareToolThread::CompareToolThread( QObject *parent, CompareToolThreadType type )
	: QThread(parent), m_Type(type), m_FilterDisables(false)
{

}

CompareToolThread::~CompareToolThread()
{
	//mutex.lock();
	//abort = true;
	//condition.wakeOne();
	//mutex.unlock();

	//wait();
}

bool CompareToolThread::copyFileToPath( QString sourceDir ,QString toDir, bool coverFileIfExist /*= true*/ ) const
{
	//Q_D(const CompareToolFileModel);
	QString strTarget = toDir;
	strTarget.replace("\\","/");
	if (sourceDir == toDir){
		return true;
	}
	QFileInfo sourceFile(sourceDir);
	if (!sourceFile.exists()){
		return false;
	}

	if (m_FilterDisables && !passNameFilters(sourceDir)) 
	{
		return false;
	}
	//strTarget += "/" + sourceFile.fileName();
	QDir createfile;
	bool exist = createfile.exists(strTarget);
	if (exist)
	{
		if(coverFileIfExist)
		{
			createfile.remove(toDir);
		}
	}
	if(!QFile::copy(sourceDir, strTarget))
	{
		return false;
	}
	return true;
}

bool CompareToolThread::copyDirectoryFiles( const QString &fromDir, const QString &toDir, bool coverFileIfExist /*= true*/ ) const
{
	QDir sourceDir(fromDir);
	QDir targetDir(toDir);
	if(!targetDir.exists()){    /**< 如果目标目录不存在，则进行创建 */
		if(!targetDir.mkdir(targetDir.absolutePath()))
			return false;
	}

	QFileInfoList fileInfoList = sourceDir.entryInfoList();
	foreach(QFileInfo fileInfo, fileInfoList){
		if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;

		if(fileInfo.isDir()){    /**< 当为目录时，递归的进行copy */

			if(!copyDirectoryFiles(fileInfo.filePath(), 
				targetDir.filePath(fileInfo.fileName()),
				coverFileIfExist))
				return false;
		}
		else{            /**< 当允许覆盖操作时，将旧文件进行删除操作 */

			if (m_FilterDisables && !passNameFilters(fromDir)) 
			{
				continue;
			}

			if(coverFileIfExist && targetDir.exists(fileInfo.fileName())){
				targetDir.remove(fileInfo.fileName()); 
			}

			/// 进行文件copy
			if(!QFile::copy(fileInfo.filePath(), 
				targetDir.filePath(fileInfo.fileName()))){
					return false;
			}
		}
	}
	return true;
}

bool CompareToolThread::move( const QString &sourceDir, const QString & toDir, bool coverFileIfExist /*= true*/ ) const
{
	bool bResult = false;
	QFileInfo file(sourceDir);
	if (file.isDir())
	{
		//QString tarDir = toDir + '/' + file.fileName();
		bResult = copyDirectoryFiles(sourceDir, toDir, true);
	}
	if (m_FilterDisables && !passNameFilters(sourceDir)) 
	{
		bResult = false;
	}
	else
	{
		bResult = copyFileToPath(sourceDir, toDir, true);
	}

	return bResult;
}

CompareToolThread::CompareToolThreadResult CompareToolThread::move( const PathToPathList & list )
{
	bool bResult = true;
	Q_FOREACH(PairPath pair, list)
	{

		if(!move(pair.first, pair.second))
		{
			bResult = false;
		}
	}


	if (!bResult)
	{
		return ThreadMoveSucess;
		//emit ResultSignal(ThreadMoveSucess);
		//MessageBoxOtherThread::show("错误", "文件移动失败!");
		//QMessageBox::warning(NULL, "错误", "文件删除失败完成!");
	}
	else
	{
		return ThreadMoveFail;
		//emit ResultSignal(ThreadMoveFail);
		//MessageBoxOtherThread::show("提示", "文件移动完成！");

		//QMessageBox::about(NULL, "提示", "文件删除完成！");
	}
}

bool CompareToolThread::passNameFilters( const QString & absoluteFilePath ) const
{
	//if (m_NameFilters.isEmpty())
	//	return true;
	QFileInfo node(absoluteFilePath);
	if (!node.isDir()) 
	{
		//比较时间
		if( node.lastModified() <= m_DateTimeLimit )
		{
			return false;
		}

		//比较名称
		for (int i = 0; i < m_NameFilters.size(); ++i) {
			if (m_NameFilters.at(i).exactMatch(node.fileName()))
				return true;
		}

		return false;
	}
	return true;
}

void CompareToolThread::SetPath( const QStringList & sourceDir )
{
	QMutexLocker meLock(&m_Mutex);
	m_SouceDir = sourceDir;
}
bool CompareToolThread::removeDir( const QString & folderFullPath ) const
{
	if (folderFullPath.isEmpty())
	{
		return true;
	}
	QFileInfo file(folderFullPath);
	if(!file.exists())
	{
		return true;
	}

	if (!file.isDir())
	{
		return QFile::remove(folderFullPath);
	}


	QDir             dir(folderFullPath);
	QFileInfoList    fileList;
	QFileInfo        curFile;
	QFileInfoList    fileListTemp;
	int            infoNum;
	int            i;
	int           j;
	/* 首先获取目标文件夹内所有文件及文件夹信息 */
	fileList=dir.entryInfoList(QDir::Dirs|QDir::Files
		|QDir::Readable|QDir::Writable
		|QDir::Hidden|QDir::NoDotAndDotDot
		,QDir::Name);

	while(fileList.size() > 0)
	{
		infoNum = fileList.size();

		for(i = infoNum - 1; i >= 0; i--)
		{
			curFile = fileList[i];
			if(curFile.isFile()) /* 如果是文件，删除文件 */
			{
				QFile fileTemp(curFile.filePath());
				fileTemp.remove();
				fileList.removeAt(i);
			}

			if(curFile.isDir()) /* 如果是文件夹 */
			{
				QDir dirTemp(curFile.filePath());
				fileListTemp = dirTemp.entryInfoList(QDir::Dirs | QDir::Files
					| QDir::Readable | QDir::Writable
					| QDir::Hidden | QDir::NoDotAndDotDot
					, QDir::Name);
				if(fileListTemp.size() == 0) /* 下层没有文件或文件夹 则直接删除*/
				{
					dirTemp.rmdir(".");
					fileList.removeAt(i);
				}
				else /* 下层有文件夹或文件 则将信息添加到列表*/
				{
					for(j = 0; j < fileListTemp.size(); j++)
					{
						if(!(fileList.contains(fileListTemp[j])))
						{
							fileList.append(fileListTemp[j]);
						}
					}
				}
			}
		}
	}
	dir.rmdir(".");/*删除目标文件夹,如果只是清空文件夹folderFullPath的内容而不删除folderFullPath本身,则删掉本行即可*/
	
		return true;
}
//bool CompareToolThread::removeDir( const QString & path ) const
//{
//	if (path.isEmpty())
//	{
//		return true;
//	}
//	QFileInfo file(path);
//	if(!file.exists())
//	{
//		return true;
//	}
//
//	if (!file.isDir())
//	{
//		return QFile::remove(path);
//	}
//
//
//	QDir dir(path);
//	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //设置过滤
//	QFileInfoList fileList = dir.entryInfoList(); // 获取所有的文件信息
//	Q_FOREACH(const QFileInfo & file, fileList)
//	{ //遍历文件信息
//		if (file.isFile())
//		{ // 是文件，删除
//			if(!file.dir().remove(file.fileName()))
//			{
//				return false;
//			}
//		}
//		else
//		{ // 递归删除
//			removeDir(file.absoluteFilePath());
//		}
//	}
//	bool b = dir.rmpath(dir.absolutePath());
//	//Q_ASSERT(b == true);
//	return true; // 删除文件夹
//
//	//QDirIterator it(path,
//	//	QDir::AllDirs | QDir:: Files | QDir::NoDotAndDotDot,
//	//	QDirIterator::Subdirectories);
//	//QStringList children;
//	//while (it.hasNext())
//	//	children.prepend(it.next());
//	//children.append(path);*/
//
//	//bool error = false;
//	//for (int i = 0; i < children.count(); ++i) {
//	//	QFileInfo info(children.at(i));
//	//	//QModelIndex modelIndex = index(children.at(i));
//	//	if (info.isDir()) 
//	//	{
//	//		QDir dir;
//	//		if (children.at(i) != path)
//	//		{
//	//			error |= removeFile(children.at(i));
//	//		}
//	//		error |= dir.rmdir(children.at(i));
//	//	} else 
//	//	{
//	//		error |= QFile::remove(children.at(i));
//	//	}
//	//}
//	//return error;
//}

CompareToolThread::CompareToolThreadResult CompareToolThread::remove( const QStringList & indexList )
{
	QStringList pathList = indexList;
	pathList.sort();
	pathList.erase(std::unique(pathList.begin(), pathList.end()), pathList.end());

	bool bResult = true;
	Q_FOREACH(const QString & path, pathList)
	{

		if ( !removeDir(path) )
		{
			bResult = false;
		}
	}
	if (!bResult)
	{
		return ThreadDelFail;
		//emit ResultSignal(ThreadDelFail);
	}
	else
	{
		return ThreadDelSucess;
	}
}

void CompareToolThread::run()
{
	CompareToolThreadResult nResult;
	switch(m_Type)
	{
	case CompareToolThreadDel:
		nResult = remove(m_SouceDir);
		break;
	case CompareToolThreadCopy:
		nResult = move(m_ToDir);
		break;
	case CompareToolThreadDiff:
		nResult = OutputFile(m_DiffDir, m_ToDir);
		break;
	}
	switch(nResult)
	{
	case ThreadDelSucess:
		MessageBoxOtherThread::show("提示", "文件删除成功!");
		break;
	case ThreadDelFail:
		MessageBoxOtherThread::show("错误", "文件删除失败!");
		break;
	case ThreadMoveSucess:
		MessageBoxOtherThread::show("提示", "文件移动成功!");
		break;
	case ThreadMoveFail:
		MessageBoxOtherThread::show("错误", "文件移动失败!");
		break;
	case ThreadOutputSucess:
		MessageBoxOtherThread::show("提示", "文件导出成功!");
		break;
	case ThreadOutputFail:
		MessageBoxOtherThread::show("错误", "文件导出失败!");
		break;
	}
}

void CompareToolThread::AddNameFilter( const QRegExp & reg )
{
	QMutexLocker lock(&m_Mutex);
	if (reg.isValid())
	{
		m_FilterDisables = true;
		m_NameFilters.push_back(reg);
	}	
}

void CompareToolThread::SetCopy( const PathToPathList & list )
{
	QMutexLocker lo(&m_Mutex);
	m_ToDir = list;
}
//
//void CompareToolThread::SetDifference( const PathToPathList & list )
//{
//	QMutexLocker meLock(&m_Mutex);
//	m_DiffDir = list;
//}

void CompareToolThread::SetDifference( const PathToPathList & diff, const PathToPathList & copy )
{
	QMutexLocker meLock(&m_Mutex);
	m_DiffDir = diff;
	m_ToDir = copy;
}
CompareToolThread::CompareToolThreadResult CompareToolThread::OutputFile( const PathToPathList & Diff, PathToPathList & Output )
{
	//bool bResult = true;
	QSet<QString> EqualFileHash;
	Q_FOREACH(PairPath pair, Diff)
	{
		if(EqualFile(pair.first, pair.second))
		{
			EqualFileHash.insert(pair.first);
			//continue;
		}
	}

	for (PathToPathList::Iterator iter = Output.begin(); iter != Output.end(); ++iter)
	{
		if(EqualFileHash.find(iter->first) != EqualFileHash.end())
		{
			Output.erase(iter);
		}
	}
	CompareToolThreadResult result = move(Output);
	switch (result)
	{
	case ThreadMoveSucess:
		return ThreadOutputSucess;
		break;
	case ThreadMoveFail:
		return ThreadOutputFail;
		break;
	}
	return ThreadOutputFail;
}

bool CompareToolThread::EqualFile( const QString & fileLeft, const QString & fileRight ) const
{
	QByteArray & fileLeftMd5 = getFileMd5(fileLeft);
	QByteArray & fileRightMd5 = getFileMd5(fileRight);

	if (fileLeftMd5 == fileRightMd5)
	{
		return true;
	}
	return false;
}

/* 方法2 */
/*
*   获取文件md5值
*/
QByteArray CompareToolThread::getFileMd5(QString filePath) const
{
	QFile localFile(filePath);

	if (!localFile.open(QFile::ReadOnly))
	{
		qDebug() << "file open error.";
		return 0;
	}

	QCryptographicHash ch(QCryptographicHash::Md5);

	quint64 totalBytes = 0;
	quint64 bytesWritten = 0;
	quint64 bytesToWrite = 0;
	quint64 loadSize = 1024 * 4;
	QByteArray buf;

	totalBytes = localFile.size();
	bytesToWrite = totalBytes;

	while (1)
	{
		if(bytesToWrite > 0)
		{
			buf = localFile.read(qMin(bytesToWrite, loadSize));
			ch.addData(buf);
			bytesWritten += buf.length();
			bytesToWrite -= buf.length();
			buf.resize(0);
		}
		else
		{
			break;
		}

		if(bytesWritten == totalBytes)
		{
			break;
		}
	}

	localFile.close();
	QByteArray md5 = ch.result();

	return md5;
}

void CompareToolThread::SetDataTimeFilter( const QDateTime & time )
{
	QMutexLocker meLock(&m_Mutex);
	if (time.isValid())
	{
		m_FilterDisables = true;
		m_DateTimeLimit = time;
	}
}