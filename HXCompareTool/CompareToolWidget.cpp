#include "stdafx.h"

#include "CompareToolConfigManager.h"
//#include <QFileSystemModel> 

#include "CompareToolWidget.h"
#include "CompareToolWidget_p.h"
#include "ui_CompareToolWidget.h"




//CompareToolWidgetPrivate *qt_CompareToolWidget_private(CompareToolWidget *widget)
//{
//	return widget->d_func();
//}

CompareToolWidget::CompareToolWidget(QWidget *parent)
	: QWidget(*new CompareToolWidgetPrivate(),0, Qt::WindowFlags())
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

void CompareToolWidget::Init()
{
	Q_D(CompareToolWidget);
	d->m_RightModel->setReadOnly(false);
	d->m_RightModel->setRootPath("");
	d->m_LeftModel->setReadOnly(false);
	d->m_LeftModel->setRootPath("");
	ui->treeView_Right->setModel(d->m_RightModel);
	ui->treeView_Left->setModel(d->m_LeftModel);
	ui->treeView_Left->setColumnWidth(0, 400);
	ui->treeView_Right->setColumnWidth(0, 400);
}