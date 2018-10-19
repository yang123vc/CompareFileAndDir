#pragma once

#include <QtWidgets/private/qwidget_p.h>
#include <qfilesystemmodel.h>

class CompareToolWidgetPrivate : public QWidgetPrivate
{
	Q_DECLARE_PUBLIC(QWidget)
//#define Q_DECLARE_PUBLIC(Class)                                    \
//    inline Class* q_func() { return static_cast<Class *>(q_ptr); } \
//    inline const Class* q_func() const { return static_cast<const Class *>(q_ptr); } \
//    friend class Class;
public:
	QFileSystemModel * m_LeftModel;
	QFileSystemModel * m_RightModel;
	QMenu			 * m_LeftMenu;
	QMenu			 * m_RightMenu;
	QThread			 * m_LeftThread;
	QThread			 * m_RightThread;
	//≈‰å¶…œµƒ
	QHash<QModelIndex, QModelIndex> m_Compare;
	CompareToolWidgetPrivate() :
		m_LeftModel(new QFileSystemModel()),
		m_RightModel(new QFileSystemModel()),
		m_LeftMenu(new QMenu()),
		m_RightMenu(new QMenu()),
		m_LeftThread(new QThread()),
		m_RightThread(new QThread())
	{
		//m_LeftModel->moveToThread(m_LeftThread);
		//m_LeftModel->moveToThread(m_RightThread);
	}
	~CompareToolWidgetPrivate()
	{
		SafeDeletePoint<QFileSystemModel>(m_LeftModel);
		SafeDeletePoint<QFileSystemModel>(m_RightModel);
		SafeDeletePoint<QMenu>(m_LeftMenu);
		SafeDeletePoint<QMenu>(m_RightMenu);
	}
};
