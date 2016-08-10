#include "transferpic.h"

///////////////////////////////////////////////////////////////////////
/// MyPictureThread_Client
///////////////////////////////////////////////////////////////////////////
MyPictureThread_Client::MyPictureThread_Client(QObject*pwin, const QHostAddress& addr, QObject *parent)
    :QThread(parent),m_pSocket(nullptr),m_peer_addr(addr),m_pWin(pwin),m_pPicSrv(nullptr)
{

}

MyPictureThread_Client::~MyPictureThread_Client()
{
    if (m_pSocket)
    {
        m_pSocket->deleteLater();
        m_pSocket = nullptr;
    }
    if (m_pPicSrv)
    {
        m_pPicSrv->deleteLater();
        m_pPicSrv = nullptr;
    }
    this->quit();
    this->wait();
}

void MyPictureThread_Client::run()
{
    m_pSocket = new QTcpSocket();

    m_pSocket->connectToHost(m_peer_addr, PICTURE_SERVER_PORT);
    if (!m_pSocket->waitForConnected())
    {/* 未连接成功 */

    }

    m_pPicSrv = new TransferPic(m_pSocket);
    /* 套接字可读信号 */
    connect(m_pSocket, SIGNAL(readyRead()), m_pPicSrv, SLOT(slot_recv_file()));
    /* 对端关闭、接受完一个文件信号 */
    connect(m_pPicSrv, SIGNAL(signal_peer_close()), m_pWin, SLOT(slot_peer_close()));
    connect(m_pPicSrv, SIGNAL(signal_send_error()), m_pWin, SLOT(slot_send_error()));
    connect(m_pPicSrv, SIGNAL(signal_recv_picture_success(QString)),
            m_pWin, SLOT(slot_recv_picture_success(QString)));
    /* 主线程添加一个任务 */
    connect(m_pWin, SIGNAL(signal_append_picture_task(const QString& )),
            m_pPicSrv, SLOT(slot_append_picture_task(const QString& )));
    /* 线程结束 */
    connect(this, SIGNAL(finished()), this, SLOT(slot_finished()));

    this->exec();
}

void MyPictureThread_Client::slot_finished()
{
    if (m_pSocket)
    {
        m_pSocket->deleteLater();
        m_pSocket = nullptr;
    }
    if (m_pPicSrv)
    {
        m_pPicSrv->deleteLater();
        m_pPicSrv = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////
/// MyPictureThread_Server
//////////////////////////////////////////////////////////////////////////////
MyPictureThread_Server::MyPictureThread_Server(QObject*pwin, QObject* parent)
    :QThread(parent),m_pListen(nullptr),m_pSocket(nullptr),
      m_pWin(pwin),m_pPicSrv(nullptr)
{

}

MyPictureThread_Server::~MyPictureThread_Server()
{
    if (m_pListen)
    {
        m_pListen->deleteLater();
        m_pListen = nullptr;
    }
    if (m_pSocket)
    {
        m_pSocket->deleteLater();
        m_pSocket = nullptr;
    }
    if (m_pPicSrv)
    {
        m_pPicSrv->deleteLater();
        m_pPicSrv = nullptr;
    }
    this->quit();
    this->wait();
}

void MyPictureThread_Server::run()
{
    if (!m_pListen)
    {
        m_pListen = new QTcpServer();
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

    }

    connect(m_pListen, SIGNAL(newConnection()), this, SLOT(slot_new_connection()));

    connect(this, SIGNAL(finished()), this, SLOT(slot_finished()));

    this->exec();
}

void MyPictureThread_Server::slot_new_connection()
{
    if (m_pListen->hasPendingConnections())
    {
        m_pSocket = m_pListen->nextPendingConnection();


        m_pPicSrv = new TransferPic(m_pSocket);
        /* 套接字可读信号 */
        connect(m_pSocket, SIGNAL(readyRead()), m_pPicSrv, SLOT(slot_recv_file()));
        /* 对端关闭、接受完一个文件信号 */
        connect(m_pPicSrv, SIGNAL(signal_peer_close()), m_pWin, SLOT(slot_peer_close()));
        connect(m_pPicSrv, SIGNAL(signal_send_error()), m_pWin, SLOT(slot_send_error()));
        connect(m_pPicSrv, SIGNAL(signal_recv_picture_success(QString)),
                m_pWin, SLOT(slot_recv_picture_success(QString)));
        /* 主线程添加一个任务 */
        connect(m_pWin, SIGNAL(signal_append_picture_task(const QString&)),
                m_pPicSrv, SLOT(slot_append_picture_task(const QString&)));
    }
}

void MyPictureThread_Server::slot_finished()
{
    if (m_pSocket)
    {
        m_pSocket->deleteLater();
        m_pSocket = nullptr;
    }
    if (m_pPicSrv)
    {
        m_pPicSrv->deleteLater();
        m_pPicSrv = nullptr;
    }
}


///////////////////////////////////////////////////////////////////////////////
/// TransferPic
/// ///////////////////////////////////////////////////////////////////////////

TransferPic::TransferPic(QTcpSocket*socket, QObject *parent)
    : QObject(parent),m_pSocket(socket),m_pMutex(new QMutex),
      m_send_file(nullptr),m_recv_file(nullptr)
{
    qDebug() << "size of chat_pic_pack_t:" << sizeof(chat_pic_pack_t);
    /* 预先分配好发送缓冲 */
    m_pSendPack = (chat_pic_pack_t*)malloc(sizeof(chat_pic_pack_t)
                                           + BUFFER_LEN);

    m_pSendTimer = new QTimer;
    connect(m_pSendTimer, SIGNAL(timeout()), this, SLOT(slot_send_file()));
    /* 每一秒检测一次是否有文件需要发送，有则发送一部分，避免阻塞 */
    m_pSendTimer->start(1000);
}


/* 添加任务 */
void TransferPic::slot_append_picture_task(const QString &filepath)
{
    Source s;
    s.filepath = filepath;
    QString text = filepath;
    if (m_files.find(text.split("/").back()) != m_files.end())
    {/* 如果存在同名文件,插入一个时间值做分辨 */
        int idx = text.lastIndexOf(".");
        text.insert(idx, QString("_%1").arg(QString::number(QDateTime::currentDateTime().toTime_t())));
    }

    s.transname = text.split("/").back();
    /* save */
    m_files[text.split("/").back()] = s.filepath;
    qDebug() << text.split("/").back();

    m_pMutex->lock();
    m_tasklist.push_back(s);
    m_pMutex->unlock();
}


void TransferPic::slot_recv_file()
{
    static char buff_for_buff[1460]; //QTcpSocket read不会一定但会返回包大小的整数倍
    static int idx_for_buffer = 0;
    static int pack_size = BUFFER_LEN + sizeof(chat_file_pack_t);

    char buffer[32767];
    /* 将上次不完整的包拼接起来 */
    strncpy(buffer, buff_for_buff, idx_for_buffer);
    qint64 ret = m_pSocket->read(buffer+idx_for_buffer, 32767-idx_for_buffer);
    /* 将尾部不完整的包保存起来 */
    idx_for_buffer = (idx_for_buffer+ret) % pack_size;
    /* 接下来实际要处理的数据量while */
    ret = (idx_for_buffer+ret)/pack_size*pack_size;
    strncpy(buff_for_buff, buffer+ret, idx_for_buffer);
    //qDebug() << "recv bytes:" << ret-idx_for_buffer;
    int idx = 0;

    while ((qint64)idx < ret)
    {
        m_pRecvPack = reinterpret_cast<chat_pic_pack_t*>(buffer+idx);
        /* 提取出本条数据所属的文件 */
        int flen = ntohs(m_pRecvPack->file_name_len);
        char buf[256];
        strncpy(buf, m_pRecvPack->data, flen);
        buf[flen] = '\0';
        QString fname(buf);
        int dlen = ntohs(m_pRecvPack->data_used_len);

        if (m_recv_file_name.size() == 0)   /* 刚创建文件的数据 */
        {
            m_recv_file_name = fname;
            m_recv_file = fopen(m_recv_file_name.toStdString().c_str(), "ab+");
            if (m_recv_file)
            {
                fwrite(m_pRecvPack->data+flen,1,dlen-flen,m_recv_file);
            }
            else
                qDebug() << "write not opened file";
        }
        else if (m_recv_file_name == fname) /* 还是已创建文件的数据 */
        {
            if (m_recv_file)
                fwrite(m_pRecvPack->data+flen,1,dlen-flen,m_recv_file);
            else
                qDebug() << "write not opened file";

        }

        /* 判断文件是否接受完毕 */
        if (ntohs(m_pRecvPack->is_finish) == 1)
        {
            emit this->signal_recv_picture_success(m_recv_file_name);
            m_recv_file_name.clear();
            fclose(m_recv_file);
            m_recv_file = nullptr;

        }
        idx += (sizeof(chat_pic_pack_t)+dlen);

    }

}

void TransferPic::slot_send_file()
{
    if (!m_pSocket)
        return;


    qint64 ret;
    if (m_send_file)
    {/* 有一个正在发送的文件,分8次,每次1024byte */
        qDebug() << "before lock";
        //m_read_file.lock();
        qDebug() << "after lock";
        for (int i = 0; i < 8; ++i)
        {
            if (!feof(m_send_file))
            {
                m_pSendPack->file_name_len = static_cast<short>(m_send_file_name.length());

                size_t size = fread(m_pSendPack->data+(int)m_pSendPack->file_name_len,
                                    1,
                                    BUFFER_LEN-(int)m_pSendPack->file_name_len,
                                    m_send_file);
                /* 在判断文件是否结束 */
                if (feof(m_send_file))
                    m_pSendPack->is_finish = htons(1);
                else
                    m_pSendPack->is_finish = htons(0);

                m_pSendPack->data[(int)m_pSendPack->file_name_len+size] = '\0';
                /* 复制文件名 */
                strncpy(m_pSendPack->data,m_send_file_name.toStdString().c_str(),
                        static_cast<size_t>( m_pSendPack->file_name_len));
                /* 计算文件名、数据总长度 */
                m_pSendPack->data_used_len = m_send_file_name.length()+size;
                /* 转换网络字节序 */
                m_pSendPack->data_used_len = htons(m_pSendPack->data_used_len);
                m_pSendPack->file_name_len = htons(m_pSendPack->file_name_len);

                ret = m_pSocket->write((char *)m_pSendPack, sizeof(chat_pic_pack_t)+
                                                            m_send_file_name.length()+
                                                            size);
                qDebug() << m_pSocket->errorString();
                //m_pSocket->waitForBytesWritten();
                qDebug() << "send bytes:" <<ret;

                if (ret < 0)
                {
                    //m_read_file.unlock();
                    emit this->signal_send_error();
                    return;
                }
                else if (ret < (qint64)size)
                {
                    qDebug() << "send not completed";
                }
                qDebug() << "send packet";
            }
            else
            {
                if (m_send_file)
                {
                    fclose(m_send_file);
                    m_send_file = nullptr;
                }
                qDebug() << "send ok";
                break;
            }


        }//for
        //m_read_file.unlock();
    }
    else
    {/* 没有正在发的文件，但任务列表有需要发的文件 */
        Source s;
        m_pMutex->lock();
        if (!m_tasklist.isEmpty() && m_send_file==nullptr)
        {
            s = m_tasklist.front();
            m_tasklist.pop_front();

            /* 新建发送的文件 */
            m_send_file = fopen(s.filepath.toStdString().c_str(), "rb");
            m_send_file_name = s.transname;
            m_pMutex->unlock();
        }
        else
            m_pMutex->unlock();

    }

}
