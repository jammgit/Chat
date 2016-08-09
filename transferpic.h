#ifndef TRANSFERPIC_H
#define TRANSFERPIC_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMutex>
#include <QSemaphore>
#include <QFile>
#include <QImage>
#include <QPixmap>
#include <QMap>
#include <QBuffer>
#include <QDateTime>
#include <QMessageBox>
#include <QByteArray>
#include <QThread>
#include <QTimer>
#include <stdio.h>
#include "msginfo.h"

/*  传输格式：        文件名(base64编码) +':'+ 文件内容(base64编码)+';'
 *  文件传输完成标志： 文件名(base64编码)  + ':' +剩余文件内容(base64编码) +':'+"END"(base64编码) ';'
*/

///////////////////////////////////////////////////////////////////////
/// MyPictureThread_Client
///////////////////////////////////////////////////////////////////////////

class TransferPic;

class MyPictureThread_Client : public QThread
{
    Q_OBJECT
public:
    explicit MyPictureThread_Client(QObject*pwin, const QHostAddress& addr, QObject* parent=0);
    ~MyPictureThread_Client();

protected:
    void run();

protected slots:
    void slot_finished();

private:
    QTcpSocket         * m_pSocket;
    QHostAddress         m_peer_addr;
    QObject            * m_pWin;

    TransferPic        * m_pPicSrv;

};
////////////////////////////////////////////////////////////////////////////
/// MyPictureThread_Server
//////////////////////////////////////////////////////////////////////////////
class MyPictureThread_Server : public QThread
{
    Q_OBJECT
public:
    explicit MyPictureThread_Server(QObject*pwin, QObject* parent=0);
    ~MyPictureThread_Server();
protected:
    void run();

protected slots:
    void slot_finished();

    void slot_new_connection();

private:
    QTcpServer         * m_pListen;
    QTcpSocket         * m_pSocket;
    QObject            * m_pWin;
    TransferPic        * m_pPicSrv;
};

///////////////////////////////////////////////////////////////////////////////
/// TransferFile
/// ///////////////////////////////////////////////////////////////////////////
class TransferPic : public QObject
{
    Q_OBJECT
public:
    explicit TransferPic(QTcpSocket* socket,QObject *parent = 0);
    ~TransferPic()
    {
        if (m_pSendTimer)
            delete m_pSendTimer;
        if (m_pMutex)
        {
            qDebug() << "a";
            delete m_pMutex;
            qDebug() << "a";
        }
        if (m_send_file)
            fclose(m_send_file);
        if (m_recv_file)
            fclose(m_recv_file);

    }

signals:
    /* 通知主线程 */
    void signal_peer_close();
    /* 通知主线程 */
    void signal_recv_picture_success(const QString& file);

public slots:
    /* 主线程通知 */
    void slot_append_picture_task(const QString& filepath);



private slots:
    /* readyread trigger */
    void slot_recv_file();
    /* */
    void slot_send_file();

private:

    /* 图片、文件传输套接字 */
    QTcpSocket              * m_pSocket;
    /* 任务链表 */
    QList<Source>             m_tasklist;
    /* 用户发送，接受到的文件、图片列表记录<不包含路径文件名（同名文件加上时间戳后缀）,完整路径名> */
    QMap<QString, QString>    m_files;
    /* 资源访问互斥量 */
    QMutex                  * m_pMutex;

    /* 发送：当前正在发送的文件、文件名 */
    FILE                    * m_send_file;
    QString                   m_send_file_name;
    FILE                    * m_recv_file;
    QString                   m_recv_file_name;
    /* 定时检测是否有需要发送的文件，同时每次只发送一部分，避免线程阻塞 */
    QTimer                  * m_pSendTimer;
};


#endif // TRANSFERPIC_H
