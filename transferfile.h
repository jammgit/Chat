#ifndef TRANSFERFILE_H
#define TRANSFERFILE_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QImage>
#include <QPixmap>
#include <QMap>
#include <QBuffer>
#include <QDateTime>
#include <QByteArray>
#include <QThread>
#include <QTcpServer>
#include <QMutex>
#include <QMessageBox>
#include <QHostAddress>
#include <QSemaphore>
#include "msginfo.h"

/*  传输格式：        文件名(base64编码) +':'+ 文件内容(base64编码)+';'
 *  文件传输完成标志： 文件名(base64编码)  + ':' +剩余文件内容(base64编码) +':'+"END"(base64编码) ';'
*/

class TransferFile : public QThread
{
    Q_OBJECT
public:
    explicit TransferFile(QObject* pwin, QObject *parent = 0);
    ~TransferFile()
    {
//        if(m_pSocket)
//        {
//            m_pSocket->close();
//            delete m_pSocket;
//        }
    }
    /* 停止线程 */
    void stop();
    /* 添加任务 */
    void Append(const QString& filename);



protected:
    void run();

signals:
    void signal_peer_close();
    void signal_recv_file_success(const QString& file);
    void signal_process();
    void signal_stop();


public slots:
    /* mainwiddow trigger */
    void slot_create_socket(const QHostAddress& addr);

private slots:
    /* readyread trigger */
    void slot_recv_file();
    /* newConnection trigger */
    void slot_get_listen_socket();

    void slot_process();
    void slot_stop();

private:
    bool m_thread_is_in_acquire;

    QHostAddress m_addr;

    bool m_is_get_socket_from_listen;

    QObject *m_pWin;

    QTcpServer *m_pListen;
    /* 图片、文件传输套接字 */
    QTcpSocket *m_pSocket;
    /* 是否停止线程 */
    bool m_stop;
    /* 任务链表 */
    QList<Source> m_tasklist;
    /* 资源访问互斥量 */
    QMutex *m_pMutex;
    /* 通知有新任务 */
    QSemaphore *m_pSem;
    QSemaphore *m_pForBlock;
    /* 用户发送，接受到的文件、图片列表记录<不包含路径文件名,完整路径名> */
    QMap<QString, QString> m_files;
    /* 保存打开的文件描述符，<文件名，打开文件的描述符>*/
    QMap<QString, QFile*> m_openfiles;
};

#endif // TRANSFERFILE_H
