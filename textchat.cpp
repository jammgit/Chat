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
        disconnect(m_pConn, SIGNAL(readyRead()), this, SLOT(slot_recv_msg()));
        m_pConn->close();
        delete m_pConn;
    }
    disconnect(m_pListen, SIGNAL(newConnection()), this, SLOT(slot_is_accept()));
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
        QMessageBox::information(nullptr, "错误", "初始化网络连接出现错误");
        exit(0);
    }
    if(!m_pListen->listen(QHostAddress::AnyIPv4, TEXTCHAT_SERVER_PORT))
    {
        QMessageBox::information(nullptr, "错误", "初始化网络连接出现错误");
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
        m_pConn = new QTcpSocket;
        m_pConn->open(QIODevice::ReadWrite);
        m_pConn->connectToHost(addr, TEXTCHAT_SERVER_PORT);
        connect(m_pConn, SIGNAL(readyRead()), this, SLOT(slot_recv_msg()));
    }
}


int TextChat::SendMsg(QString text)
{
    /* 当然是存在连接时才能发送数据了 */
    if (m_pConn)
    {
        qint64 ret = m_pConn->write(text.toUtf8());
        if (ret == -1)
        {
            QMessageBox::information(nullptr, QString("错误"), QString("通信出现错误，请重新连接"));
            disconnect(m_pConn, SIGNAL(readyRead()), this, SLOT(slot_recv_msg()));
            m_pConn->disconnectFromHost();
            m_pConn->waitForDisconnected();
            m_pConn->close();
            delete m_pConn;
            m_pConn = nullptr;
            m_isConnect = false;
        }
        return ret;
    }
    return 0;
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
                                             + (m_pTer==nullptr?"":(m_pTer->GetMap()[m_pConn->peerAddress().toString().toInt()]).hostname)
                                             + "("+m_pConn->peerAddress().toString()+")"
                                             + "发起聊天请求，是否接受请求？", btn);
            m_pConn->open(QIODevice::ReadWrite);
            connect(m_pConn, SIGNAL(readyRead()), this, SLOT(slot_recv_msg()));
            /* 接受请求 */
            if (btn == QMessageBox::Yes)
            {
                m_pConn->write(ACCEPT, strlen(ACCEPT));
                m_isConnect = true;
            }
            else
            {/* 不接受请求 */
                m_pConn->write(REJECT, strlen(REJECT));
            }
        }
    }
}


void TextChat::slot_recv_msg()
{
    qDebug() << "result come";
    /* 请求信息 */
    if (!m_isConnect && m_pConn)
    {
        QString str(m_pConn->readAll());
        if (str == QString(ACCEPT))
        {/* 请求发起者 */
            m_isConnect = true;
            /* 提示请求成功 */
            emit this->signal_request_result(true);
        }
        else if (str == QString(REJECT))
        {/* 请求发起者：不接受请求，则释放socket */
            /* 先关闭关联再delete */
            qDebug() << "请求者关闭连接";
            disconnect(m_pConn, SIGNAL(readyRead()), this, SLOT(slot_recv_msg()));
            m_pConn->disconnectFromHost();
            m_pConn->waitForDisconnected();
            m_pConn->close();
            delete m_pConn;
            m_pConn = nullptr;
            /* 提示请求失败 */
            emit this->signal_request_result(false);
        }
        else
        {/* 被请求者，注意：主动关闭应该有请求者发出，不然服务端口会处于timewait状态 */
            qDebug() << "被请求者关闭连接";
            disconnect(m_pConn, SIGNAL(readyRead()), this, SLOT(slot_recv_msg()));
            m_pConn->disconnectFromHost();
            m_pConn->waitForDisconnected();
            m_pConn->close();
            delete m_pConn;
            m_pConn = nullptr;
        }
    }
    else /* 聊天消息 */
    {
        QString str(m_pConn->readAll());
        if (str.size() == 0)
        {/* 对端关闭连接 */
            /* 先关闭关联再delete */
            disconnect(m_pConn, SIGNAL(readyRead()), this, SLOT(slot_recv_msg()));
            m_pConn->disconnectFromHost();
            m_pConn->waitForDisconnected();
            m_pConn->close();
            delete m_pConn;
            m_pConn = nullptr;
            m_isConnect = false;
            emit this->signal_peer_close();
        }
        else
        {
            emit this->signal_recv_msg(str);
        }
    }

}
