#include "StdAfx.h"

#include <QString>

#include "MessageBoxOtherThread.h"


MessageBoxOtherThread::MessageBoxOtherThread(const QString &title, const QString &message)
    : m_title(title),    m_message(message)
{ }

void MessageBoxOtherThread::show(const QString &title, const QString &message)
{
    QEventLoop eventLoop;
    MessageBoxOtherThread * messageBox = new MessageBoxOtherThread(title, message);
    connect(messageBox, SIGNAL(destroyed()), &eventLoop, SLOT(quit()));
    messageBox->readyShow();
    eventLoop.exec();
}

void MessageBoxOtherThread::readyShow(void)
{
    this->moveToThread(qApp->thread());
    QTimer::singleShot(0, this, SLOT(onShow()));
}

void MessageBoxOtherThread::onShow(void)
{
    QMessageBox::information(NULL, m_title, m_message);
    this->deleteLater();
}