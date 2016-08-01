#include "textchat.h"

TextChat::TextChat(QObject *parent)
    : QObject(parent)
{
    this->__Init();
}


TextChat::~TextChat()
{
    if (m_pConn)
    {
        m_pConn->close();
        delete m_pConn;
    }
    m_pListen->close();
    delete m_pListen;
}

void TextChat::__Init()
{
    m_isConnect = false;
    m_pConn = nullptr;
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

void TextChat::ConnectHost(const QHostAddress &addr)
{
    /* 建立连接完成，但需要等待对方确认后，若为同意才能开始聊天
     * 并且同时只能和一个人聊天
    */
    if (!m_pConn)
    {
        if (m_pTer)
            m_peerhost.hostname = m_pTer->GetMap()[(m_peerhost.address = addr.toString()).toInt()].hostname;
        else
            qDebug() << "in textchat.cpp:m_pTer is nullptr";
        m_pConn = new QTcpSocket;
        m_pConn->connectToHost(addr, TEXTCHAT_SERVER_PORT);

        connect(m_pConn, SIGNAL(connected()), this, SLOT(slot_connect_success()));
//        connect(m_pConn, SIGNAL(error(QAbstractSocket::SocketError)),
//                this, SLOT(slot_connect_failed(QAbstractSocket::SocketError)));

    }
}

/* connect成功,那么建立readyread，否则error()被发送 */
void TextChat::slot_connect_success()
{
    connect(m_pConn, SIGNAL(readyRead()), this, SLOT(slot_recv_msg()));
}
/* error()信号的槽函数，暂不使用 */
void TextChat::slot_connect_failed(QAbstractSocket::SocketError err)
{
    err = err;
    QMessageBox::information(nullptr,
                             "错误",
                             QString("请求错误[%1]，对方或许不在线，请刷新或者重试").arg(m_pConn->errorString()));
    if (m_pConn)
    {
        delete m_pConn;
        m_pConn = nullptr;
    }
    m_isConnect = false;
}

/* 关闭连接 */
void TextChat::Close()
{
    if (m_pConn)
    {
        m_pConn->write(CLOSE);
        m_pConn->waitForBytesWritten();
        qDebug() << "close";
        m_pConn->close();
        delete m_pConn;
        m_pConn = nullptr;
        m_isConnect = false;
    }
}

/* 外层接口 */
const QString TextChat::SendMsg(char msgtype, const QString& text)
{
    /* 当然是存在连接时才能发送数据了 */
//    if (m_pConn)
    {
        qDebug() << "send before";
        qint64 ret=0;
        QString str_ret;
        switch(msgtype)
        {
        case MSG_EMOJI: /* text is a path of *.gif */
            ret = m_pConn->write(QString(MSG_EMOJI).toUtf8().toBase64() + ':'
                                        + QDateTime::currentDateTime().toString().toUtf8().toBase64()+':'
                                        + text.toUtf8().toBase64()+';');
            str_ret = PIC_HTML_STRING.arg(RIGHT, text, QString::number(60),QString::number(60));
            break;
        case MSG_SHAKE:
            ret = m_pConn->write(QString(MSG_SHAKE).toUtf8().toBase64() + ':'
                                        + QDateTime::currentDateTime().toString().toUtf8().toBase64()+':'
                                        + text.toUtf8().toBase64()+';');
            str_ret = TEXT_FRONT.arg(CENTER, FONT, TEXT_COLOR_3, FONT_SIZE)
                    + text + TEXT_BACK;
            break;
        case MSG_TEXT:
            ret = m_pConn->write(QString(MSG_TEXT).toUtf8().toBase64() + ':'
                                        + QDateTime::currentDateTime().toString().toUtf8().toBase64()+':'
                                        + text.toUtf8().toBase64()+';');
            str_ret = TEXT_FRONT.arg(RIGHT, FONT, TEXT_COLOR, FONT_SIZE)
                    + text + TEXT_BACK ;
            break;
        default:
            break;
        }

        qDebug() << "send after";
        if (ret == -1)
        {
            QMessageBox::information(nullptr, QString("错误"), QString("通信出现错误，请重新连接"));
            m_pConn->close();
            delete m_pConn;
            m_pConn = nullptr;
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
    if (!m_pConn)
    {
        if (m_pListen->hasPendingConnections())
        {
            m_pConn = m_pListen->nextPendingConnection();
            QMessageBox::StandardButton btn;
            /* 这是一个糟糕的设计：仅为获得findterminal类的map,从而获得peername */
            emit this->signal_request_arrive(QString("用户：")
                                             + (m_pTer==nullptr?"":(m_peerhost.hostname = m_pTer->GetMap()[(m_peerhost.address = m_pConn->peerAddress().toString()).toInt()].hostname))
                                             + "("+m_pConn->peerAddress().toString()+")"
                                             + "发起聊天请求，是否接受请求？", btn);

            connect(m_pConn, SIGNAL(readyRead()), this, SLOT(slot_recv_msg()));
            /* 接受请求 */
            if (btn == QMessageBox::Yes)
            {
                m_pConn->write(ACCEPT);
                m_isConnect = true;
                emit this->signal_request_result(true, m_peerhost);
            }
            else
            {/* 不接受请求 */
                m_pConn->write(REJECT);
                /* UNIX网络编程中，调用close会把没发送的缓冲区数据清空还是发送完？ */
                m_pConn->waitForBytesWritten();
                emit m_pConn->readyRead();
            }
        }
    }
}


void TextChat::slot_recv_msg()
{
    //qDebug() << "result come";
    /* 请求信息 */
    if (!m_isConnect && m_pConn)
    {
        QString str(m_pConn->readAll());
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
            m_pConn->close();
            delete m_pConn;
            m_pConn = nullptr;
            /* 提示请求失败 */
            emit this->signal_request_result(false, m_peerhost);
        }
        else
        {/* 被请求者，注意：主动关闭应该有请求者发出，不然服务端口会处于timewait状态 */
            qDebug() << "被请求者关闭连接";
            m_pConn->close();
            delete m_pConn;
            m_pConn = nullptr;
        }
    }
    else /* 聊天消息(Base64编码) */
    {
        QString str(m_pConn->readAll());

        if (str.size() == 0)
        {/* 对端关闭连接 */
            /* 先关闭关联再delete */
            qDebug() << "对端关闭连接";
            m_pConn->close();
            delete m_pConn;
            m_pConn = nullptr;
            m_isConnect = false;
            emit this->signal_peer_close();
        }
        else
        {
            if (str == QString(CLOSE))          //非Base64编码,但不可能接收到CLOSE字符串
            {/* 对端关闭连接 */
                qDebug() << "对端关闭连接 by CLOSE";
                m_pConn->close();
                delete m_pConn;
                m_pConn = nullptr;
                m_isConnect = false;
                /*通知窗口更新*/
                emit this->signal_peer_close();
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
                                + text + TEXT_BACK);
                        emit this->signal_shake_window();
                        break;
                    case MSG_TEXT :
                        textlist.push_back(
                                    TEXT_FRONT.arg(LEFT, FONT, TEXT_COLOR_2, FONT_SIZE)
                                    + text
                                    + TEXT_BACK);
                        break;
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


