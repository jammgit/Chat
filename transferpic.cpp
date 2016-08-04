#include "transferpic.h"

TransferPic::TransferPic(QObject* pwin, QObject *parent)
    : QThread(parent),m_pWin(pwin),m_pListen(nullptr),
      m_pSocket(nullptr), m_stop(false),m_pMutex(new QMutex),
      m_pSem(new QSemaphore),m_pForBlock(new QSemaphore)
{
    connect(this, SIGNAL(signal_recv_picture_success(const QString&)),
            pwin, SLOT(slot_recv_picture_success(const QString&)));
    connect(this, SIGNAL(signal_peer_close()), pwin, SLOT(slot_peer_close()));
    connect(this, SIGNAL(signal_process()), this, SLOT(slot_process()),
            Qt::QueuedConnection);
    connect(this, SIGNAL(signal_stop()), this, SLOT(slot_stop()),
            Qt::QueuedConnection);
}

void TransferPic::slot_create_socket(const QHostAddress& addr)
{
    m_pSocket = new QTcpSocket;
    m_pSocket->connectToHost(addr, PICTURE_SERVER_PORT);
    if (m_pSocket->waitForConnected())
    {
        m_pSocket->moveToThread(this);
        connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(slot_recv_picture()),
                Qt::DirectConnection);
        qDebug() << "connect so";
    }
    else
    {
        qDebug() << "connect error";
    }
}

void TransferPic::slot_get_listen_socket()
{
    if (m_pListen->hasPendingConnections())
    {
        m_pSocket = m_pListen->nextPendingConnection();
        if (!m_pSocket)
        {

        }
        else
        {
            qDebug() << "get connection";
            connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(slot_recv_picture()),
                    Qt::DirectConnection);
        }
    }
}

void TransferPic::run()
{
    m_pListen = new QTcpServer;
    if (!m_pListen)
    {
        QMessageBox::information(nullptr, "错误", "初始化网络出现错误");
        exit(0);
    }

    if(!m_pListen->listen(QHostAddress::AnyIPv4, PICTURE_SERVER_PORT))
    {
        QMessageBox::information(nullptr, "错误", "初始化网络出现错误");
        exit(0);
    }
    connect(m_pListen, SIGNAL(newConnection()), this, SLOT(slot_get_listen_socket()),
            Qt::DirectConnection);

    this->exec();
}

void TransferPic::Append(const QString &filename)
{
    qDebug() << "append";
    Source s;
    s.filepath = filename;
    //s.transname = transname;
    m_pMutex->lock();
    m_tasklist.push_back(s);
    m_pMutex->unlock();
    m_pSem->release();
    emit this->signal_process();
}

void TransferPic::stop()
{
    emit this->signal_stop();
}

void TransferPic::slot_stop()
{
    m_stop = true;
    m_pSocket->close();
    delete m_pSocket;
}

/* 发送文件 */
void TransferPic::slot_process()
{
    if (!m_pSocket)
        return;

    m_pSem->acquire();
    Source source;
    m_pMutex->lock();
    source = m_tasklist.front();
    m_tasklist.pop_front();
    m_pMutex->unlock();

    qDebug() << "Process one file";
    qDebug() << m_pSocket;
    /* */
    QString text = source.filepath;
    if (m_files.find(text.split("/").back()) != m_files.end())
    {/* 如果存在同名文件,插入一个时间值做分辨 */
        int idx = text.lastIndexOf(".");
        text.insert(idx, QString("_%1").arg(QString(QDateTime::currentDateTime().toString().toInt())));
    }
    source.transname = text;
    qDebug() << text.split("/").back();
    /* save */
    m_files[text.split("/").back()] = source.filepath;

    QString base = source.transname.toUtf8().toBase64();
    QFile file(source.filepath);
    file.open(QFile::ReadOnly);
    qint64 ret;
    while (!file.atEnd() && !m_stop)
    {
        QString text(file.read(1024));
        /* 非/二进制文件，故最好先utf8 */
        if (text.length() < 1024)
        {
            QString data(base + ':'
                         + END.toUtf8().toBase64() + ':'
                         + text.toUtf8().toBase64() + ';');
            ret = m_pSocket->write(data.toLatin1());
            //qDebug() << QString("send[%1]bytes").arg(QString::number(ret));
            qDebug() << m_pSocket->error();
            qDebug() << m_pSocket->errorString();
            break;
        }
        else
        {
            QString data(base + ':' + text.toUtf8().toBase64() + ';');
            ret = m_pSocket->write(data.toLatin1());
            qDebug() << QString("send[%1]bytes").arg(QString::number(ret));
            qDebug() << m_pSocket->error();
            qDebug() << m_pSocket->errorString();
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
