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
#include <stdlib.h>
#include <winsock.h>

#include "msginfo.h"


/*  |1bit|7bit|16bits|[filename]|data
 *
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
            delete m_pMutex;
        }
        if (m_send_file)
            fclose(m_send_file);
        if (m_recv_file)
            fclose(m_recv_file);

        if (m_pSendPack)
        {
            free(m_pSendPack);
            m_pSendPack = nullptr;
        }

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
    QMutex                    m_read_file;
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
    chat_pic_pack_t         * m_pSendPack;
    chat_pic_pack_t         * m_pRecvPack;
};


#endif // TRANSFERPIC_H
