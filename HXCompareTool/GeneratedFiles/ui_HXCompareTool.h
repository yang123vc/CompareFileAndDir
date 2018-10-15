/********************************************************************************
** Form generated from reading UI file 'HXCompareTool.ui'
**
** Created by: Qt User Interface Compiler version 5.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HXCOMPARETOOL_H
#define UI_HXCOMPARETOOL_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_HXCompareToolClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *HXCompareToolClass)
    {
        if (HXCompareToolClass->objectName().isEmpty())
            HXCompareToolClass->setObjectName(QStringLiteral("HXCompareToolClass"));
        HXCompareToolClass->resize(600, 400);
        menuBar = new QMenuBar(HXCompareToolClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        HXCompareToolClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(HXCompareToolClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        HXCompareToolClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(HXCompareToolClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        HXCompareToolClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(HXCompareToolClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        HXCompareToolClass->setStatusBar(statusBar);

        retranslateUi(HXCompareToolClass);

        QMetaObject::connectSlotsByName(HXCompareToolClass);
    } // setupUi

    void retranslateUi(QMainWindow *HXCompareToolClass)
    {
        HXCompareToolClass->setWindowTitle(QApplication::translate("HXCompareToolClass", "HXCompareTool", nullptr));
    } // retranslateUi

};

namespace Ui {
    class HXCompareToolClass: public Ui_HXCompareToolClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HXCOMPARETOOL_H
