#ifndef COMPARETOOL_H
#define COMPARETOOL_H

#include <QtGui/QMainWindow>
#include "ui_CompareTool.h"

class CompareToolPluginManager : public QMainWindow
{
	Q_OBJECT

public:
	CompareTool(QWidget *parent = 0, Qt::WFlags flags = 0);
	~CompareToolPluginManager();

private:
	Ui::CompareToolClass ui;
};

#endif // COMPARETOOL_H
