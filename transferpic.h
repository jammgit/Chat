#ifndef TRANSFERPIC_H
#define TRANSFERPIC_H

#include <QObject>
#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QImage>
#include <QPixmap>
#include <QBuffer>
#include <QByteArray>
#include "msginfo.h"

/*  传输格式：        文件名(base64编码) +':'+ 文件内容(base64编码)+';'
 *  文件传输完成标志： 文件名(base64编码)  + ':' +剩余文件内容(base64编码) +':'+"END"(base64编码) ';'
*/

class TransferPic : public QObject
{
    Q_OBJECT
public:
    explicit TransferPic(QTcpSocket*socket, QObject *parent = 0);
    ~TransferPic()
    {
        delete m_pSocket;
    }
    void Process(Source& source);
    QTcpSocket *GetSocket()
    {
        return m_pSocket;
    }
signals:
    void signal_peer_close();

public slots:

private:
    QTcpSocket *m_pSocket;
};

#endif // TRANSFERPIC_H
