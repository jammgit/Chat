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


class TransferPic : public QObject
{
    Q_OBJECT
public:
    explicit TransferPic(QTcpSocket*socket, QObject *parent = 0);
    ~TransferPic()
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
    QTcpSocket *m_pSocket;
};

#endif // TRANSFERPIC_H
