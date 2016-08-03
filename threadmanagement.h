/*  @author:江伟霖
 *  @create date:2016/08/01
 *  @description:这是一个线程类，管理文件、图片的发送
 *  @email:269337654@qq.com
 */

#ifndef THREADMANAGEMENT_H
#define THREADMANAGEMENT_H

#include <QObject>
#include <QMutex>
#include <QSemaphore>
#include <QString>
#include <QMessageBox>
#include <QThread>
#include <QList>
#include <QTcpServer>
#include <QHostAddress>
#include <QtNetwork/QTcpSocket>
#include "msginfo.h"
#include "transferfile.h"
#include "transferpic.h"


template<class T>
class ThreadManagement : public QThread
{
    /* 运行线程 */
    void run();
public:
    static ThreadManagement<T>* CreateThreadManagement(
            QTcpServer*socket,const QHostAddress& addr,
            QObject* up, int maxqueue = 128);

    /* 停止线程 */
    void stop();
    /* 添加任务 */
    int Append(const QString& filename);
    /* */

    T *GetClassPoniter()
    {
        return m_pClass;
    }

    ~ThreadManagement();
private:
    explicit ThreadManagement(QTcpServer*socket, const QHostAddress& addr,
                              QObject* up,
                              int maxqueue = 128, QObject *parent = 0);
    /* */
    bool ConnectHost();

signals:

public slots:

private:
    /* 某一时刻最大的任务数量 */
    static const int MAX_TASK_NUM = 128;
    /* 类 */
    T *m_pClass;
    /* 是否停止线程 */
    bool m_stop;
    /* 任务链表 */
    QList<Source> m_tasklist;
    /* 资源访问互斥量 */
    QMutex *m_pMutex;
    /* 通知有新任务 */
    QSemaphore *m_pSem;
    /* 设定的任务数量最大值 */
    int m_maxtask;
    /* */
    QTcpServer *m_pListen;
    /* */
    QHostAddress m_peer_addr;
    /* */
    QObject *m_pobj;
};


template<class T>
ThreadManagement<T>::ThreadManagement(QTcpServer*socket,const QHostAddress&addr, QObject* up,int maxqueue, QObject *parent)
    : QThread(parent),m_pClass(nullptr),m_pMutex(new QMutex),m_pSem(new QSemaphore)
    ,m_pListen(socket),m_pobj(up)
{
    m_maxtask = maxqueue;
    m_peer_addr = addr;
    m_stop = false;
}

template<class T>
ThreadManagement<T>::~ThreadManagement()
{
    delete m_pSem;
    delete m_pMutex;
    delete m_pClass;
}

/* 创建线程 */
template<class T>
ThreadManagement<T>* ThreadManagement<T>::CreateThreadManagement(
        QTcpServer*socket,const QHostAddress& addr, QObject* up,int maxqueue)
{
    if (maxqueue > MAX_TASK_NUM)
        return nullptr;
    return new ThreadManagement<T>(socket, addr, up, maxqueue);
}



template<class T>
bool ThreadManagement<T>::ConnectHost()
{
    QTcpSocket *tmp = new QTcpSocket;
    tmp->connectToHost(m_peer_addr, TEXTCHAT_SERVER_PORT);
    if (tmp->waitForConnected())
    {
        qDebug() << "TextChat connected";
        m_pClass = new T(tmp);
        connect(m_pClass, SIGNAL(signal_peer_close()),
                m_pobj, SLOT(slot_peer_close()));
        if (dynamic_cast<TransferFile*>(m_pClass))
            connect(m_pClass, SIGNAL(signal_recv_file_success(QString)),
                    m_pobj, SLOT(slot_recv_file_success(QString)));
        else
            connect(m_pClass, SIGNAL(signal_recv_picture_success(QString)),
                    m_pobj, SLOT(slot_recv_picture_success(QString)));
        return true;
    }
    else
    {
        qDebug() << "TextChat connect failed";
        delete tmp;
        return false;
    }
}

/* 运行线程 */
template<class T>
void ThreadManagement<T>::run()
{
    if (m_pListen->hasPendingConnections())
    {/* recv */
        QTcpSocket *tmp = m_pListen->nextPendingConnection();
        if (!tmp)
        {

        }
        m_pClass = new T(tmp);
        connect(m_pClass, SIGNAL(signal_peer_close()),
                m_pobj, SLOT(slot_peer_close()));
        if (dynamic_cast<TransferFile*>(m_pClass))
            connect(m_pClass, SIGNAL(signal_recv_file_success(QString)),
                    m_pobj, SLOT(slot_recv_file_success(QString)));
        else
            connect(m_pClass, SIGNAL(signal_recv_picture_success(QString)),
                    m_pobj, SLOT(slot_recv_picture_success(QString)));
    }
    else
    {/* send */
        bool tmp = this->ConnectHost();
        if (!tmp)
        {

        }
    }
    //QMessageBox::information(nullptr, "sada", "sadas");
    while (!m_stop)
    {
        qDebug() << "acquire";
        m_pSem->acquire();
        qDebug() << "Process one picture";
        m_pMutex->lock();
        Source file = m_tasklist.front();
        m_tasklist.pop_front();
        m_pMutex->unlock();
        /* 开始读取文件并发送,实例类必须实现Process函数 */
        m_pClass->Process(file);
    }
}

/* 添加任务 */
template<class T>
int ThreadManagement<T>::Append(const QString& filename)
{
    qDebug() << "append";
    m_pMutex->lock();
    if (m_tasklist.size() > m_maxtask)
    {
        m_pMutex->unlock();
        return -1;
    }
    Source s;
    s.filepath = filename;
    //s.transname = transname;
    m_tasklist.push_back(s);
    m_pMutex->unlock();
    m_pSem->release();
    return 1;
}

template<class T>
void ThreadManagement<T>::stop()
{
    m_stop = true;
}

#endif // THREADMANAGEMENT_H
