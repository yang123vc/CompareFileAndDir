/********************************************************************************
** Form generated from reading UI file 'CompareToolWidget.ui'
**
** Created by: Qt User Interface Compiler version 5.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMPARETOOLWIDGET_H
#define UI_COMPARETOOLWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CompareToolWidget
{
public:
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_Left;
    QHBoxLayout *horizontalLayout_Left;
    QComboBox *comboBox_Left;
    QDateTimeEdit *dateTimeEdit_Left;
    QPushButton *pushButton_Left;
    QTreeView *treeView_Left;
    QVBoxLayout *verticalLayout_Right;
    QHBoxLayout *horizontalLayout_Right;
    QComboBox *comboBox_Right;
    QDateTimeEdit *dateTimeEdit_Right;
    QPushButton *pushButton_Right;
    QTreeView *treeView_Right;

    void setupUi(QWidget *CompareToolWidget)
    {
        if (CompareToolWidget->objectName().isEmpty())
            CompareToolWidget->setObjectName(QStringLiteral("CompareToolWidget"));
        CompareToolWidget->resize(1013, 738);
        horizontalLayout = new QHBoxLayout(CompareToolWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        verticalLayout_Left = new QVBoxLayout();
        verticalLayout_Left->setSpacing(6);
        verticalLayout_Left->setObjectName(QStringLiteral("verticalLayout_Left"));
        horizontalLayout_Left = new QHBoxLayout();
        horizontalLayout_Left->setSpacing(6);
        horizontalLayout_Left->setObjectName(QStringLiteral("horizontalLayout_Left"));
        comboBox_Left = new QComboBox(CompareToolWidget);
        comboBox_Left->setObjectName(QStringLiteral("comboBox_Left"));

        horizontalLayout_Left->addWidget(comboBox_Left);

        dateTimeEdit_Left = new QDateTimeEdit(CompareToolWidget);
        dateTimeEdit_Left->setObjectName(QStringLiteral("dateTimeEdit_Left"));
        dateTimeEdit_Left->setMaximumSize(QSize(120, 16777215));

        horizontalLayout_Left->addWidget(dateTimeEdit_Left);

        pushButton_Left = new QPushButton(CompareToolWidget);
        pushButton_Left->setObjectName(QStringLiteral("pushButton_Left"));
        pushButton_Left->setMaximumSize(QSize(60, 16777215));

        horizontalLayout_Left->addWidget(pushButton_Left);


        verticalLayout_Left->addLayout(horizontalLayout_Left);

        treeView_Left = new QTreeView(CompareToolWidget);
        treeView_Left->setObjectName(QStringLiteral("treeView_Left"));

        verticalLayout_Left->addWidget(treeView_Left);


        horizontalLayout->addLayout(verticalLayout_Left);

        verticalLayout_Right = new QVBoxLayout();
        verticalLayout_Right->setSpacing(6);
        verticalLayout_Right->setObjectName(QStringLiteral("verticalLayout_Right"));
        horizontalLayout_Right = new QHBoxLayout();
        horizontalLayout_Right->setSpacing(6);
        horizontalLayout_Right->setObjectName(QStringLiteral("horizontalLayout_Right"));
        comboBox_Right = new QComboBox(CompareToolWidget);
        comboBox_Right->setObjectName(QStringLiteral("comboBox_Right"));

        horizontalLayout_Right->addWidget(comboBox_Right);

        dateTimeEdit_Right = new QDateTimeEdit(CompareToolWidget);
        dateTimeEdit_Right->setObjectName(QStringLiteral("dateTimeEdit_Right"));
        dateTimeEdit_Right->setMaximumSize(QSize(120, 16777215));

        horizontalLayout_Right->addWidget(dateTimeEdit_Right);

        pushButton_Right = new QPushButton(CompareToolWidget);
        pushButton_Right->setObjectName(QStringLiteral("pushButton_Right"));
        pushButton_Right->setMaximumSize(QSize(60, 16777215));

        horizontalLayout_Right->addWidget(pushButton_Right);


        verticalLayout_Right->addLayout(horizontalLayout_Right);

        treeView_Right = new QTreeView(CompareToolWidget);
        treeView_Right->setObjectName(QStringLiteral("treeView_Right"));

        verticalLayout_Right->addWidget(treeView_Right);


        horizontalLayout->addLayout(verticalLayout_Right);


        retranslateUi(CompareToolWidget);

        QMetaObject::connectSlotsByName(CompareToolWidget);
    } // setupUi

    void retranslateUi(QWidget *CompareToolWidget)
    {
        CompareToolWidget->setWindowTitle(QApplication::translate("CompareToolWidget", "CompareToolWidget", nullptr));
        pushButton_Left->setText(QApplication::translate("CompareToolWidget", "\346\211\223\345\274\200", nullptr));
        pushButton_Right->setText(QApplication::translate("CompareToolWidget", "\346\211\223\345\274\200", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CompareToolWidget: public Ui_CompareToolWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMPARETOOLWIDGET_H
