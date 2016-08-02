#include "transferpic.h"

TransferPic::TransferPic(QTcpSocket*socket, QObject *parent)
    : QObject(parent),m_pSocket(socket)
{

}


/* 发送图片、文件 */
void TransferPic::Process(const QString& filepath)
{
    if (!m_pSocket)
        return;

    QByteArray bytes;
    QBuffer buffer(&bytes);
    if (filepath.contains(".jpg", Qt::CaseInsensitive))
    {
         QPixmap(filepath).save(&buffer, "jpg");
    }
    else if (filepath.contains(".png", Qt::CaseInsensitive))
    {
        QPixmap(filepath).save(&buffer, "png");
    }
    else
    {
        qDebug() << "unknow image";
        return;
    }
    qint64 ret = m_pSocket->write(buffer.data());
    if (ret == -1)
    {
        emit this->signal_peer_close();
    }


}
