#include "transferpic.h"

TransferPic::TransferPic(QTcpSocket*socket, QObject *parent)
    : QObject(parent),m_pSocket(socket)
{

}


/* 发送图片、文件 */
void TransferPic::Process(Source& source)
{
    if (!m_pSocket)
        return;

    if (source.is)
    {
        /* 发送缩略图 */
        QImage image(source.filepath);
        qDebug() << image.byteCount();
        QImage image1 = image.scaled(200,image.height()/(image.width()/200));
        qDebug() << image1.byteCount();
        image1.save(source.filepath = QString("./tmp/%1").arg(source.filepath.split("\\").back()));
    }
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
//    QByteArray bytes;
//    QBuffer buffer(&bytes);
//    if (source.filepath.contains(".jpg", Qt::CaseInsensitive))
//    {
//         QPixmap(filepath).save(&buffer, "jpg");
//    }
//    else if (source.filepath.contains(".png", Qt::CaseInsensitive))
//    {
//        QPixmap(filepath).save(&buffer, "png");
//    }
//    else
//    {
//        qDebug() << "unknow image";
//        return;
//    }
//    qint64 ret;
//    ret = m_pSocket->write(QString(source.transname).toUtf8().toBase64() + ":");
//    if (ret == -1)
//    {
//        emit this->signal_peer_close();
//        return;
//    }
//    ret = m_pSocket->write(buffer.data());
//    if (ret == -1)
//    {
//        emit this->signal_peer_close();
//        return;
//    }

//    ret = m_pSocket->write(QString(":") + END.toUtf8().toBase64());
//    if (ret == -1)
//    {
//        emit this->signal_peer_close();
//    }
}
