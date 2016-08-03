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

/* 接受文件 */
void TransferFile::slot_recv_file()
{

    QString recv(m_pSocket->readAll());
    QList<QString> msgs = recv.split(";");
    msgs.pop_back();

    while (!msgs.isEmpty())
    {
        QList<QString> onemsg = msgs.front().split(":");
        msgs.pop_front();
        QString file = QByteArray::fromBase64(onemsg[0].toLatin1());
        if (m_openfiles.find(file) == m_openfiles.end())
        {/* 如果该文件还没创建,则创建并保存 */
            QFile* fd = new QFile(QString("./tmp/")+file);
            fd->open(QFile::WriteOnly);
            m_openfiles[file] = fd;
        }

        m_openfiles[file]->write(QByteArray::fromBase64(onemsg[1].toLatin1()));
        if (msgs.size() == 3)
        {/* 说明文件传输完成 */
            QFile* tmp = m_openfiles[file];
            m_openfiles.remove(file);
            tmp->close();
            delete tmp;
            emit this->signal_recv_file_success(file);
        }
    }
}

