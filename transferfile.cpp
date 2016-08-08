#include "transferfile.h"

///////////////////////////////////////////////////////////////////////
/// MyFileThread_Client
///////////////////////////////////////////////////////////////////////////
MyFileThread_Client::MyFileThread_Client(QObject*pwin, const QHostAddress& addr, QObject *parent)
    :QThread(parent),m_pSocket(nullptr),m_peer_addr(addr),m_pWin(pwin),m_pFileSrv(nullptr)
{

}

void MyFileThread_Client::run()
{
    m_pSocket = new QTcpSocket();

    m_pSocket->connectToHost(m_peer_addr, FILE_SERVER_PORT);
    if (!m_pSocket->waitForConnected())
    {/* 未连接成功 */

    }

    m_pFileSrv = new TransferFile(m_pSocket);
    /* 套接字可读信号 */
    connect(m_pSocket, SIGNAL(readyRead()), m_pFileSrv, SLOT(slot_recv_file()));
    /* 对端关闭、接受完一个文件信号 */
    connect(m_pFileSrv, SIGNAL(signal_peer_close()), m_pWin, SLOT(slot_peer_close()));
    connect(m_pFileSrv, SIGNAL(signal_recv_file_success(QString)),
            m_pWin, SLOT(slot_recv_file_success(QString)));
    /* 主线程添加一个任务 */
    connect(m_pWin, SIGNAL(signal_append_file_task(const QString& )),
            m_pFileSrv, SLOT(slot_append_file_task(const QString& )));
    /* 线程结束 */
    connect(this, SIGNAL(finished()), this, SLOT(slot_finished()));

    this->exec();
}

void MyFileThread_Client::slot_finished()
{
    if (m_pSocket)
    {
        m_pSocket->close();
        delete m_pSocket;
        m_pSocket = nullptr;
    }
    if (m_pFileSrv)
    {
        delete m_pFileSrv;
        m_pFileSrv = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////
/// MyFileThread_Server
//////////////////////////////////////////////////////////////////////////////
MyFileThread_Server::MyFileThread_Server(QObject*pwin, QObject* parent)
    :QThread(parent),m_pListen(nullptr),m_pSocket(nullptr),
      m_pWin(pwin),m_pFileSrv(nullptr)
{

}

void MyFileThread_Server::run()
{
    if (!m_pListen)
    {
        m_pListen = new QTcpServer();
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

    }

    connect(m_pListen, SIGNAL(newConnection()), this, SLOT(slot_new_connection()));

    connect(this, SIGNAL(finished()), this, SLOT(slot_finished()));

    this->exec();
}

void MyFileThread_Server::slot_new_connection()
{
    if (m_pListen->hasPendingConnections())
    {
        m_pSocket = m_pListen->nextPendingConnection();


        m_pFileSrv = new TransferFile(m_pSocket);
        /* 套接字可读信号 */
        connect(m_pSocket, SIGNAL(readyRead()), m_pFileSrv, SLOT(slot_recv_file()));
        /* 对端关闭、接受完一个文件信号 */
        connect(m_pFileSrv, SIGNAL(signal_peer_close()), m_pWin, SLOT(slot_peer_close()));
        connect(m_pFileSrv, SIGNAL(signal_recv_file_success(QString)),
                m_pWin, SLOT(slot_recv_file_success(QString)));
        /* 主线程添加一个任务 */
        connect(m_pWin, SIGNAL(signal_append_file_task(const QString&)),
                m_pFileSrv, SLOT(slot_append_file_task(const QString&)));
    }
}

void MyFileThread_Server::slot_finished()
{/* 不需要关闭监听套接字 */
//    if (m_pListen)
//    {
//        m_pListen->close();
//        delete m_pListen;
//        m_pListen = nullptr;
//    }
    if (m_pSocket)
    {
        m_pSocket->close();
        delete m_pSocket;
        m_pSocket = nullptr;
    }
    if (m_pFileSrv)
    {
        delete m_pFileSrv;
        m_pFileSrv = nullptr;
    }
}


///////////////////////////////////////////////////////////////////////////////
/// TransferFile
/// ///////////////////////////////////////////////////////////////////////////

TransferFile::TransferFile(QTcpSocket*socket, QObject *parent)
    : QObject(parent),m_pSocket(socket),m_pMutex(new QMutex),
      m_send_file(nullptr),m_recv_file(nullptr)
{

    m_pSendTimer = new QTimer;
    connect(m_pSendTimer, SIGNAL(timeout()), this, SLOT(slot_send_file()));
    /* 每一秒检测一次是否有文件需要发送，有则发送一部分，避免阻塞 */
    m_pSendTimer->start(1000);
}


/* 添加任务 */
void TransferFile::slot_append_file_task(const QString &filepath)
{
    Source s;
    s.filepath = filepath;
    QString text = filepath;
    if (m_files.find(text.split("/").back()) != m_files.end())
    {/* 如果存在同名文件,插入一个时间值做分辨 */
        int idx = text.lastIndexOf(".");
        text.insert(idx, QString("_%1").arg(QString(QDateTime::currentDateTime().toString().toInt())));
    }

    s.transname = text.split("/").back();
    qDebug() << text.split("/").back();
    /* save */
    m_files[text.split("/").back()] = s.filepath;

    qDebug() << text.split("/").back();

    m_pMutex->lock();
    m_tasklist.push_back(s);
    m_pMutex->unlock();
}


void TransferFile::slot_recv_file()
{
    QString recv(m_pSocket->readAll());
    QList<QString> msgs = recv.split(";");
    msgs.pop_back();

    while (!msgs.isEmpty())
    {
        QList<QString> onemsg = msgs.front().split(":");
        msgs.pop_front();
        if (onemsg.size()<2)
        {
            continue;
        }

        QString file = QByteArray::fromBase64(onemsg[0].toLatin1());

        if (!m_recv_file)
        {
            m_recv_file_name = file;
            QString str("./tmp/");
            str.append(file);
            m_recv_file = fopen(str.toStdString().c_str(), "wb");
            if (!m_recv_file)
            {
                QMessageBox::information(nullptr, "错误", "打开文件错误，请重启");
                exit(0);
            }
        }

        QString text = QByteArray::fromBase64(onemsg[1].toLatin1());

        fwrite(text.toStdString().c_str(),1,text.length(),m_recv_file);

        if (onemsg.size() == 3)
        {/* 说明文件传输完成 */
            fflush(m_recv_file);
            fclose(m_recv_file);
            m_recv_file = nullptr;
            emit this->signal_recv_file_success(m_recv_file_name);
        }
    }
    if (m_recv_file)
        fflush(m_recv_file);
}

void TransferFile::slot_send_file()
{
    if (!m_pSocket)
        return;

    if (m_send_file)
    {/* 有一个正在发送的文件,分4次,每次1024byte */
        for (int i = 0; i < 4; ++i)
        {
            char buffer[1024];
            size_t size = fread(buffer,1,1023,m_send_file);
            buffer[size] = '\0';
            QString text(buffer);
            QString fn_base = m_send_file_name.toUtf8().toBase64();

            int ret;
            if (feof(m_send_file))                                    /* 文件读取完毕 */
            {
                QString data(fn_base + ':'
                             + text.toUtf8().toBase64() + ':'
                             + END.toUtf8().toBase64() + ';');
                ret = m_pSocket->write(data.toLatin1());
                if (ret < 0)
                {/* 对端出错 */
                    /* 清空信息 */
                    fclose(m_send_file);
                    m_send_file = nullptr;
                    m_tasklist.clear();
                    m_files.clear();
                    delete m_pMutex;
                    m_pSendTimer->stop();
                    delete m_pSendTimer;
                    if (m_recv_file)
                    {
                        fclose(m_recv_file);
                        m_recv_file = nullptr;
                    }
                    emit this->signal_peer_close();

                }
                else if ((size_t)ret < size)
                {/* 没有完全写进内核缓冲区,那么文件指针回溯 */
                    fseek(m_send_file,-size,SEEK_CUR);
                    break;
                }
                else/* 写完 */
                {
                    fclose(m_send_file);
                    m_send_file = nullptr;
                    break;
                }
            }
            else                                                /* 文件未读取完 */
            {
                QString data(fn_base + ':' + text.toUtf8().toBase64() + ';');
                ret = m_pSocket->write(data.toLatin1());
                if (ret < 0)
                {/* 对端出错 */
                    emit this->signal_peer_close();
                    /* 清空信息 */
                    fclose(m_send_file);
                    m_send_file = nullptr;
                    m_tasklist.clear();
                    m_files.clear();
                    delete m_pMutex;
                    m_pSendTimer->stop();
                    delete m_pSendTimer;
                    if (m_recv_file)
                    {
                        fclose(m_recv_file);
                        m_recv_file = nullptr;
                    }
                }
                else if ((size_t)ret < size)
                {/* 没有完全写进内核缓冲区,那么文件指针回溯 */
                    fseek(m_send_file,-(size-ret),SEEK_CUR);
                    break;
                }
            }
        }

    }
    else if (!m_tasklist.isEmpty())
    {/* 没有正在发的文件，但任务列表有需要发的文件 */
        m_pMutex->lock();
        Source s = m_tasklist.front();
        m_tasklist.pop_front();
        m_pMutex->unlock();
        /* 新建发送的文件 */
        m_send_file = fopen(s.filepath.toStdString().c_str(), "rb");
        m_send_file_name = s.transname;
    }

}




