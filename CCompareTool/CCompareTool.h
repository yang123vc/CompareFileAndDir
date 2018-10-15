#ifndef CCOMPARETOOL_H
#define CCOMPARETOOL_H

#include <QtGui/QWidget>
#include "ui_CCompareTool.h"

class CCompareTool : public QWidget
{
	Q_OBJECT

public:
	CCompareTool(QWidget *parent = 0, Qt::WFlags flags = 0);
	~CCompareTool();

private:
	Ui::CCompareToolClass ui;
};

#endif // CCOMPARETOOL_H
