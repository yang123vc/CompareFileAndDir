#include "stdafx.h"
#include "CompareToolWidget.h"


#include "CompareToolConfigManager.h"

#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	CompareToolConfigManager::Instance()->Load("config.xml");
	QTextCodec *codec = QTextCodec::codecForName("gbk");
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForTr(codec);
	QApplication a(argc, argv);
	CompareToolWidget w;
	w.Init();
	w.setWindowState(Qt::WindowMaximized);
	w.show();
	return a.exec();
}
