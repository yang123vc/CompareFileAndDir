#include "stdafx.h"

#include "CompareToolConfigManager.h"
#include <QFileSystemModel> 

#include "CompareToolWidget.h"
#include "ui_CompareToolWidget.h"

class CompareToolWidgetPrivate
{
public:
	QFileSystemModel * m_LeftModel;
	QFileSystemModel * m_RightModel;
	QMenu				 * m_LeftMenu;
	QMenu				 * m_RightMenu;
	QThread				 * m_LeftThread;
	QThread				 * m_RightThread;
	//≈‰å¶…œµƒ
	QHash<QModelIndex, QModelIndex> m_Compare;
	CompareToolWidgetPrivate() :
		m_LeftModel(new QFileSystemModel()),
		m_RightModel(new QFileSystemModel()),
		m_LeftMenu(new QMenu()),
		m_RightMenu(new QMenu()),
		m_LeftThread(new QThread()),
		m_RightThread(new QThread())
	{
		//m_LeftModel->moveToThread(m_LeftThread);
		//m_LeftModel->moveToThread(m_RightThread);
	}
	~CompareToolWidgetPrivate()
	{
		SafeDeletePoint<QFileSystemModel>(m_LeftModel);
		SafeDeletePoint<QFileSystemModel>(m_RightModel);
		SafeDeletePoint<QMenu>(m_LeftMenu);
		SafeDeletePoint<QMenu>(m_RightMenu);
	}
};


CompareToolWidgetPrivate *qt_CompareToolWidget_private(CompareToolWidget *widget)
{
	return widget->d_func();
}

CompareToolWidget::CompareToolWidget(QWidget *parent)
	: QWidget(parent), QObject(*new CompareToolWidgetPrivate, 0)
{
	ui = new Ui::CompareToolWidget();
	ui->setupUi(this);

	//connect(ui->pushButton_Left, SIGNAL(clicked()), this, SLOT(on_pushButton_Left_clicked()));
	connect(ui->comboBox_Left, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(on_comboBox_Left_currentIndexchanged(const QString&)));
	connect(ui->comboBox_Right, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(on_comboBox_Right_currentIndexchanged(const QString&)));

}

CompareToolWidget::~CompareToolWidget()
{

	//SafeDeletePoint(m_p);
	delete ui;
}
