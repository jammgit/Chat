#ifndef TEXTCHAT_H
#define TEXTCHAT_H

#include <QObject>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QHostAddress>
#include <QMessageBox>

/* 文本聊天的服务端端口是8888 */
#define TEXTCHAT_SERVER_PORT 9797
#define ACCEPT "accept"
#define REJECT "reject"

class TextChat : public QObject
{
    Q_OBJECT
public:
    explicit TextChat(QObject *parent = 0);
    ~TextChat();
    /* 建立连接函数 */
    void ConnectHost(const QHostAddress &addr);
    /* 发送信息函数 */
    int SendMsg(QString text);

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
    void signal_request_result(bool ret);
    /* 请求聊天消息到达,btn返回用户点击的按钮 */
    void signal_request_arrive(QString text, QMessageBox::StandardButton &btn);
    /* 消息到达，通知窗口更新 */
    void signal_recv_msg(QString text);
    /* 关闭连接信号函数 */
    void signal_peer_close();

private slots:
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
};

#endif // TEXTCHAT_H
