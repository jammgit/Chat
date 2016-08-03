#ifndef TEXTCHAT_H
#define TEXTCHAT_H

#include <QObject>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QHostAddress>
#include <QMessageBox>
#include <QHostInfo>
#include <QByteArray>
#include <QString>
#include <QDate>
#include <QDir>
#include <QMap>
#include <stdlib.h>
#include "findterminal.h"
#include "msginfo.h"
#include "threadmanagement.h"
#include "transferfile.h"
#include "transferpic.h"

/*
 * 连接建立顺序：text -> file -> picture
*/



class TextChat : public QObject
{
    Q_OBJECT
public:
    enum CONN_TYPE
    {TEXT, FILE, PICTURE};
public:
    explicit TextChat(QObject *parent = 0);
    ~TextChat();
    /* 建立连接函数 */
    bool ConnectHost(const QHostAddress &addr, enum CONN_TYPE type);
    /* 断开聊天，此时需要注意避开服务端主动断开的timewait状态 */
    void Close(QByteArray close = CLOSE);
    /* 发送信息函数,input plain text,return html text */
    const QString SendMsg(char msgtype, QString& text);
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
    void __Close_Socket();

signals:
    /* 请求聊天的结果，被接受（true）或者拒绝（false）*/
    void signal_request_result(bool ret, const chat_host_t& peerhost);
    /* 请求聊天消息到达,btn返回用户点击的按钮 */
    void signal_request_arrive(QString text, QMessageBox::StandardButton &btn);
    /* 消息到达，通知窗口更新 */
    void signal_recv_msg(QList<QString>& text, QList<QString>& emojis);
    /* 关闭连接信号函数 */
    void signal_peer_close();
    /* 建立连接过程网络出错 */
    void signal_peer_conn_err();

    /* 发送出错，对方已关闭 */
    void signal_send_error();
    /* 抖动窗口 */
    void signal_shake_window();

    /* */
    void signal_recv_picture_info(const QString& info);
    void signal_recv_file_info(const QString& info);




private slots:
    /* connect成功,那么建立readyread，否则error()被发送 */
    void slot_connect_success();
    /* 请求失败时，error()信号的槽函数 */
    void slot_connect_failed(QAbstractSocket::SocketError err);
    /* 监听套接字：是否接受聊天请求 */
    void slot_is_accept();
    /* 连接套接字：接受新消息 */
    void slot_recv_msg();
    /* 转发来自传输图片、文件类的信号 */
    void slot_peer_close();



private:
    /* 判断是否能够开始聊天 */
    bool m_isConnect;
    /* 监听套接字 */
    QTcpServer *m_pListen;
    /* 文本连接套接字 */
    QTcpSocket *m_pTextConn;
    /* 文件管理线程 */
    ThreadManagement<TransferFile> *m_pFileMng;
    /* 图片管理线程 */
    ThreadManagement<TransferPic> *m_pPicMng;
    /* 这是一个糟糕的设计：仅为获得findterminal类的map,从而获得peername */
    FindTerminal *m_pTer;
    /* 在连接时，对端的地址信息 */
    chat_host_t m_peerhost;

};

#endif // TEXTCHAT_H
