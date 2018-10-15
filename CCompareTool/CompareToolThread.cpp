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
	if(!targetDir.exists()){    /**< ���Ŀ��Ŀ¼�����ڣ�����д��� */
		if(!targetDir.mkdir(targetDir.absolutePath()))
			return false;
	}

	QFileInfoList fileInfoList = sourceDir.entryInfoList();
	foreach(QFileInfo fileInfo, fileInfoList){
		if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
			continue;

		if(fileInfo.isDir()){    /**< ��ΪĿ¼ʱ���ݹ�Ľ���copy */

			if(!copyDirectoryFiles(fileInfo.filePath(), 
				targetDir.filePath(fileInfo.fileName()),
				coverFileIfExist))
				return false;
		}
		else{            /**< �������ǲ���ʱ�������ļ�����ɾ������ */

			if (m_FilterDisables && !passNameFilters(fromDir)) 
			{
				continue;
			}

			if(coverFileIfExist && targetDir.exists(fileInfo.fileName())){
				targetDir.remove(fileInfo.fileName()); 
			}

			/// �����ļ�copy
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
		//MessageBoxOtherThread::show("����", "�ļ��ƶ�ʧ��!");
		//QMessageBox::warning(NULL, "����", "�ļ�ɾ��ʧ�����!");
	}
	else
	{
		return ThreadMoveFail;
		//emit ResultSignal(ThreadMoveFail);
		//MessageBoxOtherThread::show("��ʾ", "�ļ��ƶ���ɣ�");

		//QMessageBox::about(NULL, "��ʾ", "�ļ�ɾ����ɣ�");
	}
}

bool CompareToolThread::passNameFilters( const QString & absoluteFilePath ) const
{
	//if (m_NameFilters.isEmpty())
	//	return true;
	QFileInfo node(absoluteFilePath);
	if (!node.isDir()) 
	{
		//�Ƚ�ʱ��
		if( node.lastModified() <= m_DateTimeLimit )
		{
			return false;
		}

		//�Ƚ�����
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
	/* ���Ȼ�ȡĿ���ļ����������ļ����ļ�����Ϣ */
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
			if(curFile.isFile()) /* ������ļ���ɾ���ļ� */
			{
				QFile fileTemp(curFile.filePath());
				fileTemp.remove();
				fileList.removeAt(i);
			}

			if(curFile.isDir()) /* ������ļ��� */
			{
				QDir dirTemp(curFile.filePath());
				fileListTemp = dirTemp.entryInfoList(QDir::Dirs | QDir::Files
					| QDir::Readable | QDir::Writable
					| QDir::Hidden | QDir::NoDotAndDotDot
					, QDir::Name);
				if(fileListTemp.size() == 0) /* �²�û���ļ����ļ��� ��ֱ��ɾ��*/
				{
					dirTemp.rmdir(".");
					fileList.removeAt(i);
				}
				else /* �²����ļ��л��ļ� ����Ϣ��ӵ��б�*/
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
	dir.rmdir(".");/*ɾ��Ŀ���ļ���,���ֻ������ļ���folderFullPath�����ݶ���ɾ��folderFullPath����,��ɾ�����м���*/
	
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
//	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); //���ù���
//	QFileInfoList fileList = dir.entryInfoList(); // ��ȡ���е��ļ���Ϣ
//	Q_FOREACH(const QFileInfo & file, fileList)
//	{ //�����ļ���Ϣ
//		if (file.isFile())
//		{ // ���ļ���ɾ��
//			if(!file.dir().remove(file.fileName()))
//			{
//				return false;
//			}
//		}
//		else
//		{ // �ݹ�ɾ��
//			removeDir(file.absoluteFilePath());
//		}
//	}
//	bool b = dir.rmpath(dir.absolutePath());
//	//Q_ASSERT(b == true);
//	return true; // ɾ���ļ���
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
		MessageBoxOtherThread::show("��ʾ", "�ļ�ɾ���ɹ�!");
		break;
	case ThreadDelFail:
		MessageBoxOtherThread::show("����", "�ļ�ɾ��ʧ��!");
		break;
	case ThreadMoveSucess:
		MessageBoxOtherThread::show("��ʾ", "�ļ��ƶ��ɹ�!");
		break;
	case ThreadMoveFail:
		MessageBoxOtherThread::show("����", "�ļ��ƶ�ʧ��!");
		break;
	case ThreadOutputSucess:
		MessageBoxOtherThread::show("��ʾ", "�ļ������ɹ�!");
		break;
	case ThreadOutputFail:
		MessageBoxOtherThread::show("����", "�ļ�����ʧ��!");
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

/* ����2 */
/*
*   ��ȡ�ļ�md5ֵ
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