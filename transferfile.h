#ifndef TRANSFERFILE_H
#define TRANSFERFILE_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QImage>
#include <QPixmap>
#include <QMap>
#include <QBuffer>
#include <QByteArray>
#include "msginfo.h"

/*  传输格式：        文件名(base64编码) +':'+ 文件内容(base64编码)+';'
 *  文件传输完成标志： 文件名(base64编码)  + ':' +剩余文件内容(base64编码) +':'+"END"(base64编码) ';'
*/

class TransferFile : public QObject
{
    Q_OBJECT
public:
    explicit TransferFile(QTcpSocket* socket = nullptr, QObject *parent = 0);
    ~TransferFile()
    {
        if(m_pSocket)
        {
            m_pSocket->close();
            delete m_pSocket;
        }
    }
    void Process(Source& source);
    void SetSocket(QTcpSocket *socket)
    {
        m_pSocket = socket;
        if (m_pSocket)
            connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(slot_recv_file()));
    }
    QTcpSocket *GetSocket()
    {
        return m_pSocket;
    }

signals:
    void signal_peer_close();
    void signal_recv_file_success(const QString& file);

private slots:
    void slot_recv_file();

private:
    /* 图片、文件传输套接字 */
    QTcpSocket *m_pSocket;
    /* 用户发送，接受到的文件、图片列表记录<不包含路径文件名,完整路径名> */
    QMap<QString, QString> m_files;
    /* 保存打开的文件描述符，<文件名，打开文件的描述符>*/
    QMap<QString, QFile*> m_openfiles;
    QMap<QString, QFile*> m_openpics;
};

#endif // TRANSFERFILE_H
