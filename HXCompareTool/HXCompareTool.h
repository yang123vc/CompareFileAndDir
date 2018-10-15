#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_HXCompareTool.h"

class HXCompareTool : public QMainWindow
{
	Q_OBJECT

public:
	HXCompareTool(QWidget *parent = Q_NULLPTR);

private:
	Ui::HXCompareToolClass ui;
};
