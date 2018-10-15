#include "StdAfx.h"

#include "CompareToolConfigManager.h"
#include "CompareToolFileModel.h"
#include "CompareToolThread.h"
//#include <CompareToolFileModel>

#include "CompareToolWidget.h"
#include "ui_CompareToolWidget.h"



class CompareToolWidgetImpl
{
public:
	CompareToolFileModel * m_LeftModel;
	CompareToolFileModel * m_RightModel;
	QMenu				 * m_LeftMenu;
	QMenu				 * m_RightMenu;
	QThread				 * m_LeftThread;
	QThread				 * m_RightThread;
	//配對上的
	QHash<QModelIndex,QModelIndex> m_Compare;
	CompareToolWidgetImpl():
	m_LeftModel(new CompareToolFileModel()),
		m_RightModel(new CompareToolFileModel()),
		m_LeftMenu(new QMenu()),
		m_RightMenu(new QMenu()),
		m_LeftThread(new QThread()),
		m_RightThread(new QThread())
	{
		//m_LeftModel->moveToThread(m_LeftThread);
		//m_LeftModel->moveToThread(m_RightThread);
	}
	~CompareToolWidgetImpl()
	{
		SafeDeletePoint<CompareToolFileModel>(m_LeftModel);
		SafeDeletePoint<CompareToolFileModel>(m_RightModel);
		SafeDeletePoint<QMenu>(m_LeftMenu);
		SafeDeletePoint<QMenu>(m_RightMenu);
	}
};

CompareToolWidget::CompareToolWidget(QWidget *parent)
	: QWidget(parent),m_p(new CompareToolWidgetImpl())
{
	ui = new Ui::CompareToolWidget();
	ui->setupUi(this);


	//初始化tree


	//connect(ui->pushButton_Left, SIGNAL(clicked()), this, SLOT(on_pushButton_Left_clicked()));
	connect(ui->comboBox_Left, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(on_comboBox_Left_currentIndexchanged(const QString&)));
	connect(ui->comboBox_Right, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(on_comboBox_Right_currentIndexchanged(const QString&)));

}

CompareToolWidget::~CompareToolWidget()
{
	SafeDeletePoint(m_p);
	delete ui;
}

void CompareToolWidget::on_pushButton_Left_clicked()
{
	on_pushButton_clicked(ui->comboBox_Left, LeftConfig);
}

void CompareToolWidget::on_pushButton_Right_clicked()
{
	on_pushButton_clicked(ui->comboBox_Right, RightConfig);
}

void CompareToolWidget::on_pushButton_clicked( QComboBox * box, ConfigType type )
{
	QString & path = QFileDialog::getExistingDirectory(this,"打开",".");
	if ("" == path)
	{
		return;
	}
	box->insertItem(0, path);
	//	ComboxInsertItem(file_name);
	box->setCurrentIndex(0);
	switch(type)
	{
	case RightConfig:
		CompareToolConfigManager::Instance()->AddRightList(path);
		break;
	case LeftConfig:
		CompareToolConfigManager::Instance()->AddLeftList(path);
		break;
	default:
		break;
	}
}

void CompareToolWidget::Init()
{

	m_p->m_RightModel->setReadOnly(false);
	m_p->m_RightModel->setRootPath("");
	m_p->m_LeftModel->setReadOnly(false);
	m_p->m_LeftModel->setRootPath("");
	ui->treeView_Right->setModel(m_p->m_RightModel);
	ui->treeView_Left->setModel(m_p->m_LeftModel);
	ui->treeView_Left->setColumnWidth (0, 400);
	ui->treeView_Right->setColumnWidth (0, 400);


	ui->treeView_Left->SetMoveActionText("移动到右边");
	ui->treeView_Right->SetMoveActionText("移动到左边");

	//m_p->m_LeftMenu = new QMenu();
	m_p->m_LeftMenu->addAction(ui->action_MoveRight);

	//m_p->m_RightMenu = new QMenu();

	ui->comboBox_Left->addItems(CompareToolConfigManager::Instance()->LeftList());
	ui->comboBox_Right->addItems(CompareToolConfigManager::Instance()->RightList());
	on_comboBox_Left_currentIndexchanged(ui->comboBox_Left->currentText());
	on_comboBox_Right_currentIndexchanged(ui->comboBox_Right->currentText());

	connect(ui->treeView_Left, SIGNAL(expanded (const QModelIndex &)), this, SLOT(on_treeView_Left_expanded(const QModelIndex &)));
	//expanded ( const QModelIndex & index )
	//TODU:窗口刷新問題
	//ui->treeView_Left->setFocusPolicy(Qt::NoFocus); 

	connect(this, SIGNAL(removeRightSignal(const QModelIndexList&)), m_p->m_RightModel,SLOT(remove(const QModelIndexList&)));
	connect(this, SIGNAL(removeLeftSignal(const QModelIndexList&)), m_p->m_LeftModel,SLOT(remove(const QModelIndexList&)));


	//
	//connect(ui->treeView_Left, SIGNAL(), this, SLOT());
	connect(ui->treeView_Left, SIGNAL(delFileSignal(const QModelIndexList&)), this, SLOT(on_Left_delFileSignal(const QModelIndexList&)));
	connect(ui->treeView_Right, SIGNAL(delFileSignal(const QModelIndexList&)), this, SLOT(on_Left_delFileSignal(const QModelIndexList&)));


	connect(ui->treeView_Left, SIGNAL(moveFileSignal(const QModelIndexList&)), this, SLOT(on_Left_moveFileSignal(const QModelIndexList&)));
	connect(ui->treeView_Right, SIGNAL(moveFileSignal(const QModelIndexList&)), this, SLOT(on_Right_moveFileSignal(const QModelIndexList&)));

	connect(ui->treeView_Left, SIGNAL(outputFileSignal(const QModelIndexList&)), this, SLOT(on_Left_outputFileSignal(const QModelIndexList&)));
	connect(ui->treeView_Right, SIGNAL(outputFileSignal(const QModelIndexList&)), this, SLOT(on_Right_outputFileSignal(const QModelIndexList&)));

	//connect(ui->dateTimeEdit_Left, SIGNAL(dateTimeChanged(const QDateTime &)), ui->treeWidget_Left, SLOT(On_DateTimeChange(const QDateTime &)));
	//connect(ui->dateTimeEdit_Right, SIGNAL(valueChanged(const QDateTime &)), ui->treeWidget_Left, SLOT(On_DateTimeChange(const QDateTime &)));
}

void CompareToolWidget::on_comboBox_Left_currentIndexchanged( const QString& path)
{
	on_comboBox_currentIndexchanged(path, LeftConfig);
}

void CompareToolWidget::on_comboBox_Right_currentIndexchanged( const QString& path)
{
	on_comboBox_currentIndexchanged(path, RightConfig);
}

void CompareToolWidget::on_comboBox_currentIndexchanged( const QString& path, ConfigType type)
{
	QString strTemp = path;
	strTemp.replace('\\',"\\\\");
	switch(type)
	{
	case RightConfig:
		ui->treeView_Right->setRootIndex(m_p->m_RightModel->index(strTemp));
		break;
	case LeftConfig:
		ui->treeView_Left->setRootIndex(m_p->m_LeftModel->index(strTemp));
		break;
	default:
		break;
	}
	CompareAll();
}

void CompareToolWidget::mouseReleaseEvent( QMouseEvent * event )
{
	if (event->button() == Qt::RightButton)
	{
		m_p->m_LeftMenu->exec(QCursor::pos());
	}
	else
	{
		QWidget::mouseReleaseEvent(event);
	}
}
//QModelIndex GetRootPath(QModelIndex & node)
//{
//	if (node.column()!= 0)
//	{
//		return HXfind(node.parent());
//	}
//	else
//	{
//		return node;
//	}
//}

void CompareToolWidget::on_action_MoveRight_triggered( bool )
{
	CompareAll();
}

void CompareToolWidget::CompareAll()
{
	//QString x = ui->treeView_Left->rootIndex().data(CompareToolFileModel::FileNameRole).toString();
	//QString y = ui->treeView_Right->rootIndex().data(Qt::DecorationRole).toString();
	CompareEqualIndex(ui->treeView_Left->rootIndex(), ui->treeView_Right->rootIndex());

	//emit m_p->m_LeftModel->layoutChanged ();
	//ui->treeView_Right->rootIndex().D3D10_ASYNC_GETDATA_DONOTFLUSH(CompareToolFileModel::FileNameRole).toString();
	//ui->treeView_Left->rootIndex();
}

CompareToolWidget::CompareType CompareToolWidget::CompareIndex(const QModelIndex & LeftIndex, const QModelIndex & RightIndex)
{
	CompareType type = NotFindCompareType;
	QString fileNameLeft = LeftIndex.data(CompareToolFileModel::FileNameRole).toString();
	QString fileNameRight = RightIndex.data(CompareToolFileModel::FileNameRole).toString();
	QModelIndex indexNode = LeftIndex.parent();

	if (fileNameLeft == fileNameRight)
	{
		QDateTime modeTimeLeft =  m_p->m_LeftModel->lastModified (LeftIndex);
		//LeftIndex.sibling(LeftIndex.row(), g_nFileTime).data(Qt::DisplayRole).toString();
		QDateTime modeTimeRight = m_p->m_RightModel->lastModified (RightIndex);
		//RightIndex.sibling(RightIndex.row(), g_nFileTime).data(Qt::DisplayRole).toString();


		if (modeTimeLeft == modeTimeRight)
		{
			type = EquelCompareType;
		}
		else
		{
			type = DiffCompareType;
		}
	}
	return type;
}

void CompareToolWidget::CompareEqualIndex( const QModelIndex & LeftIndex, const QModelIndex & RightIndex )
{
	QModelIndexList modelListLeft = GetChildrenModeIndex(LeftIndex, LeftConfig);
	QModelIndexList modelListRight = GetChildrenModeIndex(RightIndex, RightConfig);
	Q_FOREACH(const QModelIndex & itemLeft, modelListLeft)
	{
		//设置背景色
		//ui->treeView_Left->setdata
		//m_p->m_LeftModel->setData(item,QColor(240,0,0),Qt::BackgroundColorRole);//设置背景色
		//ui->treeView_Left->update();
		//匹配
		CompareType type = NotFindCompareType;
		Q_FOREACH(const QModelIndex & itemRight, modelListRight)
		{
			m_p->m_LeftModel->setData(itemLeft,g_DefColor,Qt::BackgroundRole);//设置背景色
			m_p->m_RightModel->setData(itemRight,g_DefColor,Qt::BackgroundRole);//设置背景色

			type = CompareIndex(itemLeft, itemRight);
			if ( type == EquelCompareType )
			{
				//modelListLeft.removeOne(itemLeft)
				modelListLeft.removeOne(itemLeft);
				modelListRight.removeOne(itemRight);
				break;;
			}
			else if (type == DiffCompareType)
			{
				m_p->m_LeftModel->setData(itemLeft,g_DiffColor,Qt::BackgroundRole);//设置背景色
				m_p->m_RightModel->setData(itemRight,g_DiffColor,Qt::BackgroundRole);//设置背景色
				break;
			}
		}
		if (type == NotFindCompareType)
		{
			m_p->m_LeftModel->setData(itemLeft,g_FindNotColor,Qt::BackgroundRole);//设置背景色
		}
	}
	Q_FOREACH(const QModelIndex & itemRight, modelListRight)
	{
		m_p->m_RightModel->setData(itemRight,g_FindNotColor,Qt::BackgroundRole);//设置背景色
	}	
}

QModelIndexList CompareToolWidget::GetChildrenModeIndex( const QModelIndex & index, ConfigType type )
{
	QString xLeft = index.data(CompareToolFileModel::FilePathRole).toString();
	//QStringList listLeft;
	QDirIterator itLeft(xLeft, QDir::AllDirs | QDir:: Files | QDir::NoDotAndDotDot);
	QStringList childrenLeft;
	while (itLeft.hasNext())
		childrenLeft.prepend(itLeft.next());
	QModelIndexList modelListLeft;
	CompareToolFileModel * model = NULL;
	switch(type)
	{
	case RightConfig:
		model = m_p->m_RightModel;
		break;
	case LeftConfig:
		model = m_p->m_LeftModel;
		break;
	default:
		break;
	}
	Q_ASSERT(model);
	//QModelIndex tempIndex = m_p->m_LeftModel->index(0, 0, LeftIndex);
	for (int nIddex = 0; nIddex < childrenLeft.length(); ++nIddex)
	{
		QModelIndex tempIndex = model->index(nIddex, 0, index);
		if (m_p->m_LeftModel->isDir(tempIndex))
		{
			continue;
		}
		modelListLeft.push_back(tempIndex);
		//listLeft.push_back(tempIndex.data(Qt::DisplayRole).toString());
	}
	return modelListLeft;
}

void CompareToolWidget::on_treeView_Left_expanded( const QModelIndex & index)
{
	QString relativeFilePath = GetRelativeFilePath(index, LeftConfig);
	QString rootRight = ui->treeView_Right->rootIndex().data(CompareToolFileModel::FilePathRole).toString();
	QString path = rootRight + '/' + relativeFilePath;
	QModelIndex rightIndex = m_p->m_RightModel->index(path);
	if (!rightIndex.isValid())
	{
		return;
	}
	ui->treeView_Right->expand(rightIndex);

	CompareEqualIndex(index, rightIndex);
}

//int CompareToolWidget::GetModelIndexDeep(const QModelIndex & index, QStringList & path )
//{
//	if ( index.parent().isValid() )
//	{
//		//path.
//		//++nDeep;
//		//GetModelIndexDeep(index.parent(), nDeep);
//	}
//	return 1;
//}

QString CompareToolWidget::GetRelativeFilePath( const QModelIndex & index, ConfigType type )
{
	CompareToolTreeView * temp = NULL;
	switch(type)
	{
	case RightConfig:
		temp = ui->treeView_Right;
		break;
	case LeftConfig:
		temp = ui->treeView_Left;
		break;
	default:
		break;
	}
	Q_ASSERT(temp);
	QString rootPath = temp->rootIndex().data(CompareToolFileModel::FilePathRole).toString();
	QDir rootDir(rootPath);

	QString currentPath = index.data(CompareToolFileModel::FilePathRole).toString();
	//QDir currentDir(currentPath);

	return rootDir.relativeFilePath(currentPath);
}

void CompareToolWidget::on_Left_delFileSignal( const QModelIndexList& list)
{
	on_delFileSignal(list,LeftConfig);
}

void CompareToolWidget::on_Right_delFileSignal( const QModelIndexList& list)
{
	on_delFileSignal(list,RightConfig);
}

void CompareToolWidget::on_delFileSignal( const QModelIndexList&selectList,ConfigType type )
{
	QStringList list;
	ModeIndexListToQStringList(selectList, list);


	CompareToolThread * workerThread = new CompareToolThread(this, CompareToolThread::CompareToolThreadDel);  
	workerThread->SetPath(list);
//	QDateTime & time = ui
//workerThread
	//bool x = connect(workerThread, SIGNAL(ResultSignal(CompareToolThread::CompareToolThreadResult)), 
	//	this, SLOT(OnResultSignal(CompareToolThread::CompareToolThreadResult resu)), Qt::DirectConnection);
	//bool x1 = connect(workerThread, SIGNAL(ResultSignal(CompareToolThread::CompareToolThreadResult)), 
	//	this, SLOT(OnResultSignal(CompareToolThread::CompareToolThreadResult resu)));
	//bool x2 = connect(workerThread, SIGNAL(ResultSignal(CompareToolThread::CompareToolThreadResult)), 
	//	this, SLOT(OnResultSignal(CompareToolThread::CompareToolThreadResult resu)), Qt::QueuedConnection);
	//bool x3 = connect(workerThread, SIGNAL(ResultSignal(CompareToolThread::CompareToolThreadResult)), 
	//	this, SLOT(OnResultSignal(CompareToolThread::CompareToolThreadResult resu)), Qt::AutoCompatConnection);
	//bool x4 = connect(workerThread, SIGNAL(ResultSignal(CompareToolThread::CompareToolThreadResult)), 
	//	this, SLOT(OnResultSignal(CompareToolThread::CompareToolThreadResult resu)), Qt::BlockingQueuedConnection);
	//bool x5 = connect(workerThread, SIGNAL(ResultSignal(CompareToolThread::CompareToolThreadResult)), 
	//	this, SLOT(OnResultSignal(CompareToolThread::CompareToolThreadResult resu)), Qt::UniqueConnection);
	workerThread->start();

	//CompareToolTreeView * treeView = NULL;
	//CompareToolFileModel * model = NULL;
	//switch(type)
	//{
	//case RightConfig:
	//	treeView = ui->treeView_Right;
	//	model = m_p->m_RightModel;
	//	break;
	//case LeftConfig:
	//	treeView = ui->treeView_Left;
	//	model = m_p->m_LeftModel;
	//	break;
	//default:
	//	break;
	//}
	//Q_ASSERT(model&&treeView);
	//switch(type)
	//{
	//case RightConfig:
	//	emit removeRightSignal(selectList);
	//	break;
	//case LeftConfig:
	//	emit removeLeftSignal(selectList);
	//	break;
	//default:
	//	break;
	//}
}

void CompareToolWidget::ModeIndexListToQStringList(const QModelIndexList& indexList,QStringList& list)
{
	Q_FOREACH(const QModelIndex & index, indexList)
	{
		if (index.column() == g_nFileName)
		{
			QString temp = index.data(CompareToolFileModel::FilePathRole).toString();
			if (temp != "")
			{
				list.push_back(temp);
			}
		}
	}
}

void CompareToolWidget::ModeIndexListToQStringList( const QModelIndexList & indexList, PathToPathList & list, ConfigType type )
{
	Q_FOREACH(const QModelIndex & index, indexList)
	{
		if (index.column() == g_nFileName)
		{
			QString temp = index.data(CompareToolFileModel::FilePathRole).toString();
			if (temp != "")
			{
				QString & tarPath = GetTargetPath(index, type);
				list.push_back(std::make_pair(temp, tarPath));
			}
		}
	}
}

void CompareToolWidget::ModeIndexListToQStringList( const QModelIndexList&selectList, PathToPathList&tarList, const QString &dir,ConfigType type )
{
	Q_FOREACH(const QModelIndex & index, selectList)
	{
		if (index.column() == g_nFileName)
		{
			QString temp = index.data(CompareToolFileModel::FilePathRole).toString();
			if (temp != "")
			{
				QString & tarPath = GetTargetPath(index, dir, type);
				tarList.push_back(std::make_pair(temp, tarPath));
			}
		}
	}
}

void CompareToolWidget::on_Left_moveFileSignal( const QModelIndexList& selectList)
{
	PathToPathList list;
	ModeIndexListToQStringList(selectList, list, LeftConfig);

	CompareToolThread * workerThread = new CompareToolThread(this, CompareToolThread::CompareToolThreadCopy);  
	workerThread->SetCopy(list);
	QDateTime date = GetDateTimeLimit(LeftConfig);
	if(date.isValid())
	{
		workerThread->SetDataTimeFilter(date);
	}
	workerThread->start();
}

void CompareToolWidget::on_Right_moveFileSignal( const QModelIndexList& selectList)
{
	PathToPathList list;
	ModeIndexListToQStringList(selectList, list, RightConfig);

	CompareToolThread * workerThread = new CompareToolThread(this, CompareToolThread::CompareToolThreadCopy);  
	workerThread->SetCopy(list);
	QDateTime date = GetDateTimeLimit(RightConfig);
	if(date.isValid())
	{
		workerThread->SetDataTimeFilter(date);
	}
	workerThread->start();
}

void CompareToolWidget::OnResultSignal( CompareToolThread::CompareToolThreadResult resu )
{
	switch(resu)
	{
	case CompareToolThread::ThreadDelSucess:
		QMessageBox::about(NULL, "提示", "文件删除完成！");
		break;
	case CompareToolThread::ThreadDelFail:
		QMessageBox::warning(NULL, "错误", "文件删除失败！");
		break;
	case CompareToolThread::ThreadMoveSucess:
		QMessageBox::warning(NULL, "错误", "文件移动完成!");
		break;
	case CompareToolThread::ThreadMoveFail:
		QMessageBox::about(NULL, "提示", "文件删除完成！");
		break;
	}
}

void CompareToolWidget::DateTimeLimit( const QModelIndex & index, const QDateTime & configTime, ConfigType type )
{
	if (!index.isValid())
	{
		return;
	}
	QString xLeft = index.data(CompareToolFileModel::FilePathRole).toString();
	//QStringList listLeft;
	QDirIterator itLeft(xLeft, QDir::AllDirs | QDir:: Files | QDir::NoDotAndDotDot);
	QStringList childrenLeft;
	while (itLeft.hasNext())
		childrenLeft.prepend(itLeft.next());
	QModelIndexList modelListLeft;
	CompareToolFileModel * model = NULL;
	CompareToolTreeView * treeView = NULL;
	switch(type)
	{
	case RightConfig:
		model = m_p->m_RightModel;
		treeView = ui->treeView_Right;
		break;
	case LeftConfig:
		model = m_p->m_LeftModel;
		treeView = ui->treeView_Left;
		break;
	default:
		break;
	}
	Q_ASSERT(model&&treeView);
	//QModelIndex tempIndex = m_p->m_LeftModel->index(0, 0, LeftIndex);
	for (int nIddex = 0; nIddex < childrenLeft.length(); ++nIddex)
	{
		QModelIndex tempIndex = model->index(nIddex, 0, index);
		if (m_p->m_LeftModel->isDir(tempIndex))
		{
			continue;
		}
		if (model->lastModified(tempIndex) > configTime)
		{
			continue;
		}
		treeView->setRowHidden(tempIndex.row(), index, true);

		//listLeft.push_back(tempIndex.data(Qt::DisplayRole).toString());

	}
}

void CompareToolWidget::on_dateTimeEdit_Left_dateTimeChanged( const QDateTime & date)
{
	DateTimeLimit(ui->treeView_Left->rootIndex(), date, LeftConfig);
}

void CompareToolWidget::on_dateTimeEdit_Right_dateTimeChanged( const QDateTime &date )
{
	DateTimeLimit(ui->treeView_Right->rootIndex(), date, RightConfig);
}

QString CompareToolWidget::GetTargetPath( const QModelIndex & index, ConfigType type )
{
	QTreeView * treeView = NULL;
	switch(type)
	{
	case LeftConfig:
		treeView = ui->treeView_Right;
		break;
	case RightConfig:
		treeView = ui->treeView_Left;
		break;
	default:
		break;
	}
	Q_ASSERT(treeView);
	QString & strRelativePath = GetRelativeFilePath(index, type);
	QString path = treeView->rootIndex().data(CompareToolFileModel::FilePathRole).toString(); 
	path += '/' + strRelativePath;
	return path;
}

QString CompareToolWidget::GetTargetPath( const QModelIndex & index, const QString & tarDir, ConfigType type )
{
	QString & strRelativePath = GetRelativeFilePath(index, type);
	QString path = tarDir; 
	path += '/' + strRelativePath;
	return path;
}

void CompareToolWidget::on_Left_outputFileSignal( const QModelIndexList& selectList)
{
	on_outputFileSignal(selectList, LeftConfig);
}

void CompareToolWidget::on_Right_outputFileSignal( const QModelIndexList& selectList)
{
	on_outputFileSignal(selectList, RightConfig);
}

void CompareToolWidget::on_outputFileSignal( const QModelIndexList&selectList, ConfigType type )
{
	QStringList dir = CompareToolConfigManager::Instance()->Output();
	if(dir.count() == 0)
	{
		dir.push_back(".");
	}
	QString OutputDir = QFileDialog::getExistingDirectory(this, tr("导出差异文件"),
		dir.at(0),
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);

	if(OutputDir == "")
	{
		return;
	}
	OutputDir.replace('\\','/');
	CompareToolConfigManager::Instance()->AddOutput(OutputDir);

	PathToPathList list;
	ModeIndexListToQStringList(selectList, list, type);


	PathToPathList tarList;
	ModeIndexListToQStringList(selectList, tarList, OutputDir, type);

	CompareToolThread * workerThread = new CompareToolThread(this, CompareToolThread::CompareToolThreadCopy);  
	workerThread->SetDifference(list, tarList);
	
	QDateTime date = GetDateTimeLimit(type);
	if(date.isValid())
	{
		workerThread->SetDataTimeFilter(date);
	}
	workerThread->start();
}

QDateTime CompareToolWidget::GetDateTimeLimit( ConfigType type )
{
	QDateTimeEdit * pDateTimeEdit = NULL;
	switch(type)
	{
	case LeftConfig:
		pDateTimeEdit = ui->dateTimeEdit_Left;
		break;
	case RightConfig:
		pDateTimeEdit = ui->dateTimeEdit_Right;
		break;
	default:
		break;
	}
	Q_ASSERT(pDateTimeEdit);
	return pDateTimeEdit->dateTime();
}