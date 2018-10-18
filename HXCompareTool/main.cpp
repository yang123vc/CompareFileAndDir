#include "stdafx.h"
#include "CompareToolWidget.h"
#include "CompareToolConfigManager.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	CompareToolConfigManager::Instance()->Load("config.xml");
	QApplication a(argc, argv);
	QTextCodec *codec = QTextCodec::codecForName("gbk");
	QTextCodec::setCodecForLocale(codec);
	CompareToolWidget w;
	w.Init();
	w.show();
	return a.exec();
}
