#ifndef MESSAGEBOXOTHERTHREAD_H
#define MESSAGEBOXOTHERTHREAD_H

#include <QMessageBox>
#include <QEventLoop>


class MessageBoxOtherThread : public QObject
{
    Q_OBJECT

public:
    MessageBoxOtherThread(const QString &title, const QString &message);
    static void show(const QString &title, const QString &message);

private:
    void readyShow(void);

private slots:
        void onShow(void);
private:
    const QString m_title;
    const QString m_message;
    
};
#endif // MESSAGEBOXOTHERTHREAD_H
