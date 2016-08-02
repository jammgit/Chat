#include "transferfile.h"

TransferFile::TransferFile(QTcpSocket*socket, QObject *parent)
    : QObject(parent),m_pSocket(socket)
{
}

/* 发送图片、文件 */
void TransferFile::Process(Source& source)
{
    if (!m_pSocket)
        return;

    QString base = source.transname.toUtf8().toBase64();
    QFile file(source.filepath);
    file.open(QFile::ReadOnly);
    qint64 ret;
    while (!file.atEnd())
    {
        QString text(file.read(1024));
        /* 非/二进制文件，故最好先utf8 */
        if (text.length() < 1024)
        {
             ret = m_pSocket->write((base + ":"
                                     + END.toUtf8().toBase64() + ":"
                                     + text.toUtf8().toBase64() + ";").toLatin1());
             break;
        }
        else
        {
            ret = m_pSocket->write((base + ":" + text.toUtf8().toBase64() + ";").toLatin1());
        }
        if (ret == -1)
        {
            emit this->signal_peer_close();
        }
    }
    file.close();
    if (ret == -1)
        emit this->signal_peer_close();

}

