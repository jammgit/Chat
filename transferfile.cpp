#include "transferfile.h"

TransferFile::TransferFile(QTcpSocket*socket, QObject *parent)
    : QObject(parent),m_pSocket(socket)
{
}

/* 发送图片、文件 */
void TransferFile::Process(const QString& filepath)
{
    if (!m_pSocket)
        return;

    if (m_pSocket)
    {
        QFile file(filepath);
        file.open(QFile::ReadOnly);
        while (!file.atEnd())
        {
            QString text(file.read(4096));
            /* 非/二进制文件，故最好先utf8 */
            qint64 ret = m_pSocket->write(text.toUtf8());
            if (ret == -1)
            {
                emit this->signal_peer_close();
            }
        }
        file.close();
    }
    else
        return;
}

