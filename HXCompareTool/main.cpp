#include "stdafx.h"
#include "HXCompareTool.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	HXCompareTool w;
	w.show();
	return a.exec();
}
