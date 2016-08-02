#ifndef TRANSFERFILE_H
#define TRANSFERFILE_H

#include <QObject>
#include <QTcpSocket>
#include <QFile>
#include <QImage>
#include <QPixmap>
#include <QBuffer>
#include <QByteArray>
#include "msginfo.h"

class TransferFile : public QObject
{
    Q_OBJECT
public:
    explicit TransferFile(QTcpSocket* socket, QObject *parent = 0);
    ~TransferFile()
    {
        delete m_pSocket;
    }
    void Process(const QString& filepath);
    QTcpSocket *GetSocket()
    {
        return m_pSocket;
    }

signals:
    void signal_peer_close();

public slots:

private:
    /* 图片、文件传输套接字 */
    QTcpSocket *m_pSocket;
};

#endif // TRANSFERFILE_H