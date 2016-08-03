#include "textchat.h"

TextChat::TextChat(QObject *parent)
    : QObject(parent)
{
    this->__Init();
}

TextChat::~TextChat()
{
    this->__Close_Socket();
    m_pListen->close();
    delete m_pListen;
}

void TextChat::__Init()
{
    m_isConnect = false;
    m_pTextConn = nullptr;
    m_pFileMng = nullptr;
    m_pPicMng = nullptr;
    m_pTer = nullptr;
    m_pListen = new QTcpServer;
    if (!m_pListen)
    {
        QMessageBox::information(nullptr, "错误", "初始化网络出现错误");
        exit(0);
    }
    if(!m_pListen->listen(QHostAddress::AnyIPv4, TEXTCHAT_SERVER_PORT))
    {
        QMessageBox::information(nullptr, "错误", "初始化网络出现错误");
        exit(0);
    }
    /* 监听套接字：如果有连接到来则调用slot_is_accept */
    connect(m_pListen, SIGNAL(newConnection()), this, SLOT(slot_is_accept()));

}

void TextChat::__Close_Socket()
{
    if (m_pTextConn)
    {

        m_pTextConn->close();
        delete m_pTextConn;
        m_pTextConn = nullptr;
    }
    if (m_pFileMng)
    {
        m_pFileMng->exit();
        delete m_pFileMng;
        m_pFileMng = nullptr;
    }
    if (m_pPicMng)
    {
        m_pPicMng->exit();
        delete m_pPicMng;
        m_pPicMng = nullptr;
    }
    /* 清空列表 */
    m_isConnect = false;
}

bool TextChat::ConnectHost(const QHostAddress &addr, enum CONN_TYPE type)
{
    /* 建立连接完成，但需要等待对方确认后，若为同意才能开始聊天
     * 并且同时只能和一个人聊天
    */
    switch(type)
    {
    case TEXT:
        if (!m_pTextConn)
        {
            if (m_pTer)
                m_peerhost.hostname = m_pTer->GetMap()[(m_peerhost.address = addr.toString()).toInt()].hostname;
            else
                qDebug() << "in textchat.cpp:m_pTer is nullptr";
            m_pTextConn = new QTcpSocket;
            m_pTextConn->connectToHost(addr, TEXTCHAT_SERVER_PORT);
            if (m_pTextConn->waitForConnected())
            {
                qDebug() << "TextChat connected";
                connect(m_pTextConn, SIGNAL(readyRead()), this, SLOT(slot_recv_msg()));
                return true;
            }
            else
            {
                qDebug() << "TextChat connect failed";
                this->__Close_Socket();
                return false;
            }
    //        connect(m_pTextConn, SIGNAL(connected()), this, SLOT(slot_connect_success()));
    //        connect(m_pTextConn, SIGNAL(error(QAbstractSocket::SocketError)),
    //                this, SLOT(slot_connect_failed(QAbstractSocket::SocketError)));
    //        return true;

        }
        break;
    case FILE:
        if (!m_pFileMng)
        {
            m_pFileMng = ThreadManagement<TransferFile>::CreateThreadManagement(m_pListen,
                                                                                QHostAddress(m_peerhost.address));
            m_pFileMng->start();
            connect(m_pFileMng->GetClassPoniter(), SIGNAL(signal_peer_close()),
                    this, SLOT(slot_peer_close()));
            connect(m_pFileMng->GetClassPoniter(), SIGNAL(signal_recv_file_success(QString)),
                    this, SLOT(slot_recv_file_success(QString)));
//            if (!b)
//            {
//                this->__Close_Socket();
//                return false;
//            }
//            m_pFileMng->start();
//            return true;
        }
        break;
    case PICTURE:
        if (!m_pPicMng)
        {
            m_pPicMng = ThreadManagement<TransferPic>::CreateThreadManagement(m_pListen,
                                                                               QHostAddress(m_peerhost.address));
            m_pPicMng->start();
            connect(m_pPicMng->GetClassPoniter(), SIGNAL(signal_peer_close()),
                    this, SLOT(slot_peer_close()));
            connect(m_pPicMng->GetClassPoniter(), SIGNAL(signal_recv_picture_success(QString)),
                    this, SLOT(slot_recv_picture_success(QString)));
//            QTcpSocket* tmp = new QTcpSocket;
//            tmp->connectToHost(addr, TEXTCHAT_SERVER_PORT);
//            if (tmp->waitForConnected())
//            {
//                qDebug() << "TextChat connected";
//                m_pPicMng = ThreadManagement<TransferPic>::CreateThreadManagement(tmp);
//                connect(m_pPicMng->GetClassPoniter()->GetSocket(), SIGNAL(readyRead()), this, SLOT(slot_recv_picture()));
//                m_pPicMng->start();
//                return true;
//            }
//            else
//            {
//                qDebug() << "TextChat connect failed";
//                delete tmp;
//                this->__Close_Socket();
//                return false;
//            }
        }
        break;
    default:
        break;
    }
    return false;
}

/* connect成功,那么建立readyread，否则error()被发送 */
void TextChat::slot_connect_success()
{
    connect(m_pTextConn, SIGNAL(readyRead()), this, SLOT(slot_recv_msg()));
}
/* error()信号的槽函数，暂不使用 */
void TextChat::slot_connect_failed(QAbstractSocket::SocketError err)
{
    err = err;
    QMessageBox::information(nullptr,
                             "错误",
                             QString("请求错误[%1]，对方或许不在线，请刷新或者重试").arg(m_pTextConn->errorString()));
    this->__Close_Socket();
    m_isConnect = false;
}

/* 关闭连接 */
void TextChat::Close(QByteArray close)
{
    if (m_pTextConn)
    {
        m_pTextConn->write(close);
        m_pTextConn->waitForBytesWritten();
        qDebug() << "close";
        this->__Close_Socket();
        m_isConnect = false;
    }
}

/* 外层接口 */
const QString TextChat::SendMsg(char msgtype, QString& text)
{
    /* 当然是存在连接时才能发送数据了 */
    if (m_pTextConn)
    {
        qDebug() << "send before";
        qint64 ret=0;
        QString str_ret;
        switch(msgtype)
        {
        case MSG_EMOJI: /* text is a path of *.gif */
            ret = m_pTextConn->write(QString(MSG_EMOJI).toUtf8().toBase64() + ':'
                                        + QDateTime::currentDateTime().toString().toUtf8().toBase64()+':'
                                        + text.toUtf8().toBase64()+';');
            str_ret = PIC_HTML_STRING.arg(RIGHT, text, QString::number(60),QString::number(60));
            break;
        case MSG_SHAKE:
            ret = m_pTextConn->write(QString(MSG_SHAKE).toUtf8().toBase64() + ':'
                                        + QDateTime::currentDateTime().toString().toUtf8().toBase64()+':'
                                        + text.toUtf8().toBase64()+';');
            str_ret = TEXT_FRONT.arg(CENTER, FONT, TEXT_COLOR_3, FONT_SIZE)
                    + text + TEXT_BACK;
            break;
        case MSG_TEXT:
            ret = m_pTextConn->write(QString(MSG_TEXT).toUtf8().toBase64() + ':'
                                        + QDateTime::currentDateTime().toString().toUtf8().toBase64()+':'
                                        + text.toUtf8().toBase64()+';');
            str_ret = QString("<p align=\"right\" background-color=\"#895612\">") + text + QString("</p>");
            break;

        case MSG_FILE_INFO:     /* 相对、完整路径 */
            m_pFileMng->Append(text);
//            tmp = text;
//            if (m_files.find(text.split("/").back()) != m_files.end())
//            {/* 如果存在同名文件,插入一个时间值做分辨 */
//                int idx = text.lastIndexOf(".");
//                text.insert(idx, QString("_%1").arg(QDateTime::currentDateTime().toString()));
//            }
//            /* 添加记录 */
//            m_files[text.split("/").back()] = tmp;

//            ret = m_pTextConn->write(QString(MSG_FILE_INFO).toUtf8().toBase64() + ':'
//                                        + QDateTime::currentDateTime().toString().toUtf8().toBase64()+':'
//                                        + text.split("/").back().toUtf8().toBase64()+';');

            str_ret = TEXT_FRONT.arg(RIGHT, FONT, TEXT_COLOR_3, FONT_SIZE)
                    + "你发送了文件[" + text + "]" + TEXT_BACK;
            break;

        case MSG_IMAGE_INFO:    /* 相对、完整路径 */
            /* 向线程添加任务 */
            m_pPicMng->Append(text);

//            tmp = text;
//            if (m_files.find(text.split("/").back()) != m_files.end())
//            {/* 如果存在同名文件,插入一个时间值做分辨 */
//                int idx = text.lastIndexOf(".");
//                text.insert(idx, QString("_%1").arg(QDateTime::currentDateTime().toString()));
//            }
//            qDebug() << text.split("/").back();
//            m_files[text.split("/").back()] = tmp;


            str_ret = PIC_HTML_STRING.arg(RIGHT, text, QString::number(100),QString::number(200) );
            /* 添加记录 */

            break;
            /* 在combobox点击会选择此两种消息类型 */
//        case MSG_DOWNLOAD_FILE: /* text是纯文件名 */
//            ret = m_pTextConn->write(QString(MSG_DOWNLOAD_FILE).toUtf8().toBase64() + ':'
//                                        + QDateTime::currentDateTime().toString().toUtf8().toBase64()+':'
//                                        + text.toUtf8().toBase64()+';');
//            str_ret = TEXT_FRONT.arg(LEFT, FONT, TEXT_COLOR_3, FONT_SIZE)
//                    + "开始下载文件["+ text + "]..." + TEXT_BACK;
//            break;
//        case MSG_DOWNLOAD_IMAGE:
//            ret = m_pTextConn->write(QString(MSG_DOWNLOAD_FILE).toUtf8().toBase64() + ':'
//                                        + QDateTime::currentDateTime().toString().toUtf8().toBase64()+':'
//                                        + text.toUtf8().toBase64()+';');
//            str_ret = TEXT_FRONT.arg(LEFT, FONT, TEXT_COLOR_3, FONT_SIZE)
//                    + "开始下载图片["+ text + "]..." + TEXT_BACK;
//            break;
        default:
            break;
        }

        qDebug() << "send after";
        if (ret == -1)
        {
            QMessageBox::information(nullptr, QString("错误"), QString("通信出现错误，请重新连接"));
            this->__Close_Socket();
            m_isConnect = false;
            emit this->signal_send_error();
            return "";
        }
        return str_ret;
    }
    return "";
}


void TextChat::slot_is_accept()
{
    if (!m_pTextConn)
    {
        if (m_pListen->hasPendingConnections())
        {
            m_pTextConn = m_pListen->nextPendingConnection();
            if (!m_pTextConn)
                return;
            if (strncmp(m_pTextConn->peerAddress().toString().toStdString().c_str(),"127", 3) == 0)
            {/* 收到来自本地的连接，无语了 */
                delete m_pTextConn;
                m_pTextConn = nullptr;
                return;
            }
        }
    }
    else if (!m_pFileMng)
    {/* */
        m_pFileMng = ThreadManagement<TransferFile>::CreateThreadManagement(m_pListen,
                                                                            QHostAddress(m_peerhost.address));
        m_pFileMng->start();
//        bool b = m_pFileMng->CreateSocket(m_pListen);
//        if (!b)
//        {
//            this->__Close_Socket();
//            return;
//        }
    }
    else if (!m_pPicMng)
    {
        m_pPicMng = ThreadManagement<TransferPic>::CreateThreadManagement(m_pListen,
                                                                           QHostAddress(m_peerhost.address));
//        bool b = m_pFileMng->CreateSocket(m_pListen);
//        if (!b)
//        {
//            this->__Close_Socket();
//            return;
//        }
        m_pPicMng->start();

        /* 当三个必须的套接字都成功建立连接后，询问用户是否同意请求 */
        QMessageBox::StandardButton btn;
        /* 这是一个糟糕的设计：仅为获得findterminal类的map,从而获得peername */
        emit this->signal_request_arrive(QString("用户：")
                                         + (m_pTer==nullptr?"":(m_peerhost.hostname = m_pTer->GetMap()[(m_peerhost.address = m_pTextConn->peerAddress().toString()).toInt()].hostname))
                                         + "("+m_pTextConn->peerAddress().toString()+")"
                                         + "发起聊天请求，是否接受请求？", btn);

        connect(m_pTextConn, SIGNAL(readyRead()), this, SLOT(slot_recv_msg()));
        /* 接受请求 */
        if (btn == QMessageBox::Yes)
        {
            m_pTextConn->write(ACCEPT);
            m_isConnect = true;
            emit this->signal_request_result(true, m_peerhost);
        }
        else
        {/* 不接受请求 */
            m_pTextConn->write(REJECT);
            /* UNIX网络编程中，调用close会把没发送的缓冲区数据清空还是发送完？ */
            m_pTextConn->waitForBytesWritten();
            emit m_pTextConn->readyRead();
            this->__Close_Socket();
        }
    }
}


void TextChat::slot_recv_msg()
{
    //qDebug() << "result come";
    /* 请求信息 */
    if (!m_isConnect && m_pTextConn)
    {
        QString str(m_pTextConn->readAll());
        if (str == QString(ACCEPT))
        {/* 请求发起者 */
            m_isConnect = true;
            /* 提示请求成功 */
            emit this->signal_request_result(true, m_peerhost);
        }
        else if (str == QString(REJECT))
        {/* 请求发起者：不被接受请求，则释放socket */
            /* 先关闭关联再delete */
            qDebug() << "请求者关闭连接";
            this->__Close_Socket();
            /* 提示请求失败 */
            emit this->signal_request_result(false, m_peerhost);
        }
        else
        {/* 被请求者，注意：主动关闭应该有请求者发出，不然服务端口会处于timewait状态 */
            qDebug() << "被请求者关闭连接";
            this->__Close_Socket();
        }
    }
    else /* 聊天消息(Base64编码) */
    {
        QString str(m_pTextConn->readAll());

        if (str.size() == 0)
        {/* 对端关闭连接 */
            /* 先关闭关联再delete */
            qDebug() << "对端关闭连接";
            this->__Close_Socket();
            m_isConnect = false;
            emit this->signal_peer_close();
        }
        else
        {
            if (str == QString(CLOSE))          //非Base64编码,但不可能接收到CLOSE字符串
            {/* 对端关闭连接 */
                qDebug() << "对端关闭连接 by CLOSE";
                this->__Close_Socket();
                m_isConnect = false;
                /*通知窗口更新*/
                emit this->signal_peer_close();
            }
            else if (str == QString(CONN_ERR))
            {
                this->__Close_Socket();
                m_isConnect = false;
                emit this->signal_peer_conn_err();
            }
            else                                 //Base64编码
            {
                QList<QString> strlist = str.split(';');
                strlist.pop_back();
                QList<QString> textlist;
                QList<QString> emojis;
                while (!strlist.isEmpty())
                {
                    QList<QString> onemsg = strlist.front().split(':');
                    /* just need one time for the messages arriving in the same time*/
                    if (textlist.empty())
                    {
                        textlist.push_back(
                                    TEXT_FRONT.arg(CENTER, FONT, TIME_COLOR, FONT_SIZE)
                                    + QByteArray::fromBase64(onemsg[1].toLatin1())
                                    + TEXT_BACK);
                    }
                    strlist.pop_front();
                    QString who = QByteArray::fromBase64(onemsg[0].toLatin1());
                    QString text(QByteArray::fromBase64(onemsg[2].toLatin1()));
                    switch(who.toStdString()[0])
                    {
                    case MSG_EMOJI:
                        textlist.push_back(PIC_HTML_STRING.arg(LEFT, text, QString::number(60),QString::number(60)));
                        emojis.push_back(text);
                        break;
                    case MSG_SHAKE:
                        textlist.push_back(TEXT_FRONT.arg(CENTER, FONT, TEXT_COLOR_3, FONT_SIZE)
                                + "对方抖动了窗口" + TEXT_BACK);
                        emit this->signal_shake_window();
                        break;
                    case MSG_TEXT :
                        textlist.push_back(text);
                        break;
//                    case MSG_FILE_INFO:
//                        textlist.push_back(TEXT_FRONT.arg(CENTER, FONT, TEXT_COLOR_3, FONT_SIZE)
//                                           + "对方发送了文件[" + text + "]" + TEXT_BACK);
//                        emit this->signal_recv_file_info(text);
//                        break;
//                    case MSG_DOWNLOAD_FILE: /* 此时text为加上了时间戳的文件名 */
//                        m_pFileMng->Append(m_files[text], text);
//                        break;
//                    case MSG_DOWNLOAD_IMAGE:/* 此时text为加上了时间戳的文件名 */
//                        m_pPicMng->Append(m_files[text], text);
//                        break;
                    default:
                        qDebug() << "default";
                        break;
                    }
                }
                emit this->signal_recv_msg(textlist, emojis);
            }
        }
    }

}


void TextChat::slot_peer_close()
{
    this->__Close_Socket();
    emit this->signal_peer_close();
}

void TextChat::slot_recv_file_success(const QString& file)
{
    emit this->signal_recv_file_success(file);
}
void TextChat::slot_recv_picture_success(const QString& file)
{
    emit this->signal_recv_picture_success(file);
}



