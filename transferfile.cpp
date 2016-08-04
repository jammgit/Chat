#include "transferfile.h"

TransferFile::TransferFile(QObject* pwin, QObject *parent)
    : QThread(parent),m_pWin(pwin),m_pListen(nullptr),
      m_pSocket(nullptr), m_stop(false),m_pMutex(new QMutex),
      m_pSem(new QSemaphore),m_pForBlock(new QSemaphore)
{

    connect(this, SIGNAL(signal_recv_file_success(QString)), pwin, SLOT(
                slot_recv_file_success(QString)));

    connect(this, SIGNAL(signal_peer_close()), pwin, SLOT(slot_peer_close()));
}

void TransferFile::slot_create_socket(const QHostAddress& addr)
{
    m_addr = addr;
    m_is_get_socket_from_listen = false;
    m_pForBlock->release();
}

void TransferFile::slot_get_listen_socket()
{
    m_is_get_socket_from_listen = true;
    m_pForBlock->release();
}

void TransferFile::run()
{
    m_pListen = new QTcpServer;
    if (!m_pListen)
    {
        QMessageBox::information(nullptr, "错误", "初始化网络出现错误");
        exit(0);
    }

    if(!m_pListen->listen(QHostAddress::AnyIPv4, FILE_SERVER_PORT))
    {
        QMessageBox::information(nullptr, "错误", "初始化网络出现错误");
        exit(0);
    }
    connect(m_pListen, SIGNAL(newConnection()), this, SLOT(slot_get_listen_socket()));
    /* block */
    while (true)
    {
        qDebug() << "thread in acquire";
        m_pForBlock->acquire();

        if (m_is_get_socket_from_listen)
        {
            if (m_pListen->hasPendingConnections())
            {
                m_pSocket = m_pListen->nextPendingConnection();
                if (!m_pSocket)
                {

                }
                else
                {
                    connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(slot_recv_file()));
                }
            }
        }
        else
        {
            m_pSocket = new QTcpSocket;
            m_pSocket->connectToHost(m_addr, FILE_SERVER_PORT);
            if (m_pSocket->waitForConnected())
            {
                connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(slot_recv_file()));
            }
            else
            {

            }
        }

        while (!m_stop)
        {
            qDebug() << "acquire";
            m_thread_is_in_acquire = true;
            m_pSem->acquire();
            m_thread_is_in_acquire = false;
            qDebug() << "Process one picture";
            //m_pMutex->lock();
            if (m_pMutex->tryLock(1000))           // just wait 3 seconds(for close session)
            {
                Source file = m_tasklist.front();
                m_tasklist.pop_front();
                m_pMutex->unlock();
                /* 开始读取文件并发送,实例类必须实现Process函数 */
                this->Process(file);
            }
            else
            {/* slient close session:see TransferFile::stop function */
                m_pMutex->unlock();
            }
        }
        /* when close session */
        m_pSocket->close();
        delete m_pSocket;
        m_stop = false;
    }
}

void TransferFile::Append(const QString &filename)
{
    qDebug() << "append";
    Source s;
    s.filepath = filename;
    //s.transname = transname;
    m_pMutex->lock();
    m_tasklist.push_back(s);
    m_pMutex->unlock();
    m_pSem->release();
}

void TransferFile::stop()
{
    m_stop = true;
    if (m_thread_is_in_acquire)
    {/* thread is in acquire */
        m_pMutex->lock();
        m_pSem->release();
    }
}

/* 发送文件 */
void TransferFile::Process(Source& source)
{
    if (!m_pSocket)
        return;
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

