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
#include <stdlib.h>
#include "findterminal.h"
#include "msginfo.h"
#include "threadmanagement.h"
#include "transferfile.h"
#include "transferpic.h"

/*
 * 连接建立顺序：text -> file -> picture
*/

//#define file    'f'
//#define picture 'p'
//#define text    't'


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
    void Close();
    /* 发送信息函数,input plain text,return html text */
    const QString SendMsg(char msgtype, const QString& text);
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
    /* 发送出错，对方已关闭 */
    void signal_send_error();

    void signal_shake_window();

private slots:
    /* connect成功,那么建立readyread，否则error()被发送 */
    void slot_connect_success();
    /* 请求失败时，error()信号的槽函数 */
    void slot_connect_failed(QAbstractSocket::SocketError err);
    /* 监听套接字：是否接受聊天请求 */
    void slot_is_accept();
    /* 连接套接字：接受新消息 */
    void slot_recv_msg();
    /* 接受图片 */
    void slot_recv_picture();
    /* 接受文件 */
    void slot_recv_file();

private:
    /* 判断是否能够开始聊天 */
    bool m_isConnect;
    /* 监听套接字 */
    QTcpServer *m_pListen;
    /* 文本连接套接字 */
    QTcpSocket *m_pTextConn;
    /* 真正传输文件时使用的套接字 */
    QTcpSocket *m_pFileConn;
    /* 传输图片时的套接字 */
    QTcpSocket *m_pPicConn;
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
