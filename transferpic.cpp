#include "transferpic.h"

TransferPic::TransferPic(QTcpSocket*socket, QObject *parent)
    : QObject(parent),m_pSocket(socket)
{
    if (m_pSocket)
        connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(slot_recv_picture()));
}


/* 发送图片 */
void TransferPic::Process(Source& source)
{

    if (!m_pSocket)
        return;

    /* */
    QString text = source.filepath;
    if (m_files.find(text.split("/").back()) != m_files.end())
    {/* 如果存在同名文件,插入一个时间值做分辨 */
        int idx = text.lastIndexOf(".");
        text.insert(idx, QString("_%1").arg(QDateTime::currentDateTime().toString()));
    }
    source.transname = text;
    qDebug() << text.split("/").back();
    /* save */
    m_files[text.split("/").back()] = source.filepath;

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
            QString data(base + ':'
                         + END.toUtf8().toBase64() + ':'
                         + text.toUtf8().toBase64() + ';');
            ret = m_pSocket->write(data.toLatin1());
            break;
        }
        else
        {
            QString data(base + ':' + text.toUtf8().toBase64() + ';');
            ret = m_pSocket->write(data.toLatin1());
        }
        if (ret == -1)
        {
            emit this->signal_peer_close();
        }
    }
    file.close();

    if (ret == -1)
    {
        qDebug() << "send error";
        emit this->signal_peer_close();
    }
    qDebug() << "Send finished";
}

/* 接受图片 */
void TransferPic::slot_recv_picture()
{

    qDebug() << "recv picture";
    QString recv(m_pSocket->readAll());
    QList<QString> msgs = recv.split(";");
    msgs.pop_back();

    while (!msgs.isEmpty())
    {
        QList<QString> onemsg = msgs.front().split(":");
        msgs.pop_front();
        QString file = QByteArray::fromBase64(onemsg[0].toLatin1());
        if (m_openpics.find(file) == m_openpics.end())
        {/* 如果该文件还没创建,则创建并保存 */
            QFile* fd = new QFile(QString("./tmp/")+file);
            fd->open(QFile::WriteOnly);
            m_openpics[file] = fd;
        }

        m_openpics[file]->write(QByteArray::fromBase64(onemsg[1].toLatin1()));
        if (msgs.size() == 3)
        {/* 说明文件传输完成 */
            QFile* tmp = m_openpics[file];
            m_openpics.remove(file);
            tmp->close();
            delete tmp;

            emit this->signal_recv_picture_success(file);
        }
    }
}
