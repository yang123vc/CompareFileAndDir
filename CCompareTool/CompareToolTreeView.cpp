#include "StdAfx.h"
#include "CompareToolTreeView.h"
#include "ui_CompareToolTreeView.h"

CompareToolTreeView::CompareToolTreeView(QWidget *parent)
	: QTreeView(parent)
{
	ui = new Ui::CompareToolTreeView();
	ui->setupUi(this);
	m_Menu.addAction(ui->action_Move);
	m_Menu.addAction(ui->action_Del);
	m_Menu.addAction(ui->action_Output);

	
	
	//QTreeView::setSelectionModel(QAbstractItemView::SelectRows);
}

CompareToolTreeView::~CompareToolTreeView()
{
	delete ui;
}

void CompareToolTreeView::mouseReleaseEvent( QMouseEvent * event )
{
	if (event->button() == Qt::RightButton)
	{
		m_Menu.exec(QCursor::pos());
	}
	else
	{
		QWidget::mouseReleaseEvent(event);
	}

}

void CompareToolTreeView::UpdateShowView()
{
	updateGeometries();
}

void CompareToolTreeView::SetMoveActionText( const QString & txt )
{
	ui->action_Move->setText(txt);
}

void CompareToolTreeView::on_action_Del_triggered( bool )
{
	QModelIndexList & selectList = selectedIndexes();
	emit delFileSignal(selectList);
	//QAbstractItemModel * p = model();
	//if (!p)
	//{
	//	return;
	//}
	//CompareToolFileModel * model = dynamic_cast<CompareToolFileModel *>(p);

	//Q_FOREACH(const QModelIndex & index, selectList)
	//{
	//	model->remove(index);
	//}
}

void CompareToolTreeView::on_action_Move_triggered( bool )
{
	QModelIndexList & selectList = selectedIndexes();
	emit moveFileSignal(selectList);
}

void CompareToolTreeView::repaint()
{
	QTreeView::repaint();
}

void CompareToolTreeView::on_action_Output_triggered( bool )
{
	QModelIndexList & selectList = selectedIndexes();
	emit outputFileSignal(selectList);
}