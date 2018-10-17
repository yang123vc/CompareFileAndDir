#include "stdafx.h"
#include "HXCompareTool.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	CompareToolConfigManager::Instance()->Load("config.xml");
	QApplication a(argc, argv);
	QTextCodec *codec = QTextCodec::codecForName("gbk");
	QTextCodec::setCodecForLocale(codec);
	QTextCodec::setCodecForCStrings(codec);
	QTextCodec::setCodecForTr(codec);
	HXCompareTool w;
	w.show();
	return a.exec();
}
