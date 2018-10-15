#ifndef COMPARETOOLTREEVIEW_H
#define COMPARETOOLTREEVIEW_H

#include <QTreeView>
namespace Ui {class CompareToolTreeView;};

class CompareToolTreeView : public QTreeView
{
	Q_OBJECT

public:
	CompareToolTreeView(QWidget *parent = 0);
	~CompareToolTreeView();

	void UpdateShowView();
	void SetMoveActionText(const QString & txt);
signals:
	void delFileSignal(const QModelIndexList&);
	void moveFileSignal(const QModelIndexList&);
	void outputFileSignal(const QModelIndexList&);
public slots:
	void on_action_Del_triggered(bool);
	void on_action_Move_triggered(bool);
	void on_action_Output_triggered(bool);
	void repaint();
protected:
	void mouseReleaseEvent ( QMouseEvent * event );
private:
	Ui::CompareToolTreeView *ui;
	QMenu m_Menu;
};

#endif // COMPARETOOLTREEVIEW_H
