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
    /* 清空列表 */
    m_isConnect = false;
}

bool TextChat::ConnectHost(const QHostAddress &addr)
{
    /* 建立连接完成，但需要等待对方确认后，若为同意才能开始聊天
     * 并且同时只能和一个人聊天
    */

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
    }

    return false;
}

/* 关闭连接 */
void TextChat::Close()
{
    if (m_pTextConn)
    {
        m_pTextConn->write(CLOSE);
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




