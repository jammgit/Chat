#ifndef TEXTCHAT_H
#define TEXTCHAT_H

#include <QObject>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QHostAddress>
#include <QMessageBox>
#include <QHostInfo>
#include <QByteArray>

#include "findterminal.h"

/* 文本聊天的服务端端口是8888 */
#define TEXTCHAT_SERVER_PORT 8888
/* 类内部调用的控制消息，非Base64编码 */
#define ACCEPT QString("accept").toUtf8()
#define REJECT QString("reject").toUtf8()
/* 这是糟糕的一个设计：发送此文本表示结束聊天，所以如果用户输入
 * 此文本即发生结束聊天，问题来源：QT socket同步关闭！不过Base64编码解决了问题  */
#define CLOSE  QString("!").toUtf8()

class TextChat : public QObject
{
    Q_OBJECT
public:
    explicit TextChat(QObject *parent = 0);
    ~TextChat();
    /* 建立连接函数 */
    void ConnectHost(const QHostAddress &addr);
    /* 断开聊天，此时需要注意避开服务端主动断开的timewait状态 */
    void Close();
    /* 发送信息函数 */
    int SendMsg(QString text);
    /* 这是一个糟糕的设计：仅为获得findterminal类的map,从而获得peername */
    void SetFindTerminal(FindTerminal *ter)
    {
        m_pTer = ter;
    }

    /* 是否已建立连接,注意：是否能够开始聊天 和 是否建立连接是两回事，
     * 建立连接之后，还需要等待对方接受请求才能开始聊天。
    */
    bool IsConnect()
    {
        return m_isConnect;
    }

private:
    void __Init();

signals:
    /* 请求聊天的结果，被接受（true）或者拒绝（false）*/
    void signal_request_result(bool ret, const chat_host_t& peerhost);
    /* 请求聊天消息到达,btn返回用户点击的按钮 */
    void signal_request_arrive(QString text, QMessageBox::StandardButton &btn);
    /* 消息到达，通知窗口更新 */
    void signal_recv_msg(QString text);
    /* 关闭连接信号函数 */
    void signal_peer_close();
    /* 发送出错，对方已关闭 */
    void signal_send_error();

private slots:
    /* connect成功,那么建立readyread，否则error()被发送 */
    void slot_connect_success();
    /* 请求失败时，error()信号的槽函数 */
    void slot_connect_failed(QAbstractSocket::SocketError err);
    /* 监听套接字：是否接受聊天请求 */
    void slot_is_accept();
    /* 连接套接字：接受新消息 */
    void slot_recv_msg();

private:
    /* 判断是否能够开始聊天 */
    bool m_isConnect;
    /* 监听套接字 */
    QTcpServer *m_pListen;
    /* 连接套接字 */
    QTcpSocket *m_pConn;
    /* 这是一个糟糕的设计：仅为获得findterminal类的map,从而获得peername */
    FindTerminal *m_pTer;
    /* 在连接时，对端的地址信息 */
    chat_host_t m_peerhost;
    /* 文本消息接受的缓冲区（主要为了消息的分隔复原） */
    QString m_recvBuffer;
};

#endif // TEXTCHAT_H
