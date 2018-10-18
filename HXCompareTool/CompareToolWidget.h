#pragma once

#include "CompareToolThread.h"
#include <QWidget> 

namespace Ui { class CompareToolWidget; };

class CompareToolWidgetPrivate;
class CompareToolWidget : public QWidget
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(CompareToolWidget)
//#define Q_DECLARE_PRIVATE(Class) \
//    inline Class##Private* d_func() { return reinterpret_cast<Class##Private *>(qGetPtrHelper(d_ptr)); } \
//    inline const Class##Private* d_func() const { return reinterpret_cast<const Class##Private *>(qGetPtrHelper(d_ptr)); } \
//    friend class Class##Private;
public:
	explicit CompareToolWidget(QWidget *parent = Q_NULLPTR);
	~CompareToolWidget();


public:
	enum ConfigType
	{
		RightConfig,
		LeftConfig,
		OutputConfig
	};
	enum CompareType
	{
		EquelCompareType,
		NotFindCompareType,
		DiffCompareType
	};

//
	void Init();
//
//	void CompareAll();
//
//	//获得目标字符串
//	void ModeIndexListToQStringList(const QModelIndexList& indexList, QStringList& list);
//	void ModeIndexListToQStringList(const QModelIndexList&, PathToPathList&, ConfigType type);
//	void ModeIndexListToQStringList(const QModelIndexList&selectList, PathToPathList&tarList, const QString &dir, ConfigType type);
//	//	void ModeIndexListToQStringList(const QModelIndexList& indexList,QStringList& list);
//public Q_SLOTS:
//	void on_pushButton_Left_clicked();
//	void on_pushButton_Right_clicked();
//
//	void on_comboBox_Left_currentIndexchanged(const QString&);
//	void on_comboBox_Right_currentIndexchanged(const QString&);
//
//	void on_action_MoveRight_triggered(bool);
//	void on_treeView_Left_expanded(const QModelIndex &);
//
//	void on_Left_delFileSignal(const QModelIndexList&);
//	void on_Right_delFileSignal(const QModelIndexList&);
//
//	void on_Left_moveFileSignal(const QModelIndexList&);
//	void on_Right_moveFileSignal(const QModelIndexList&);
//
//	void on_Left_outputFileSignal(const QModelIndexList&);
//	void on_Right_outputFileSignal(const QModelIndexList&);
//
//	void OnResultSignal(CompareToolThread::CompareToolThreadResult resu);
//	void on_dateTimeEdit_Left_dateTimeChanged(const QDateTime & date);
//	void on_dateTimeEdit_Right_dateTimeChanged(const QDateTime &date);
//
//	//void GetPathToPathList(const QStringList & )
//Q_SIGNALS:
//	void removeRightSignal(const QModelIndexList & indexList);
//	void removeLeftSignal(const QModelIndexList & indexList);
//protected:
//	void mouseReleaseEvent(QMouseEvent * event);
//private:
//	QDateTime GetDateTimeLimit(ConfigType type);
//	void on_outputFileSignal(const QModelIndexList&, ConfigType type);
//	void on_delFileSignal(const QModelIndexList&, ConfigType type);
//	void on_moveFileSignal(ConfigType type);
//	//void CompareIndex(QModelIndex & RightIndex, QModelIndex & LeftIndex);
//	void on_comboBox_currentIndexchanged(const QString&, ConfigType);
//	void on_pushButton_clicked(QComboBox * box, ConfigType);
//
//	//同級對比
//	void CompareEqualIndex(const QModelIndex & LeftIndex, const QModelIndex & RightIndex);
//	QModelIndexList GetChildrenModeIndex(const QModelIndex & index, ConfigType type);
//	CompareType CompareIndex(const QModelIndex & LeftIndex, const QModelIndex & RightIndex);
//
//
//	void DateTimeLimit(const QModelIndex & index, const QDateTime & configTime, ConfigType type);
//	//int GetModelIndexDeep(const QModelIndex & index, QStringList & path);
//
//	QString GetTargetPath(const QModelIndex & index, ConfigType type);
//	QString GetTargetPath(const QModelIndex & index, const QString & tarDir, ConfigType type);
//	QString GetRelativeFilePath(const QModelIndex & index, ConfigType type);


	//friend CompareToolWidgetPrivate *qt_CompareToolWidget_private(CompareToolWidget *widget);
private:
	Ui::CompareToolWidget *ui;
	Q_DISABLE_COPY(CompareToolWidget)
//	Q_PRIVATE_SLOT(d_func(), void _q_showIfNotHidden())
};

//Q_DECLARE_OPERATORS_FOR_FLAGS(CompareToolWidget::RenderFlags)
