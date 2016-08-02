#ifndef FINDTERMINAL_H
#define FINDTERMINAL_H

#include <QMainWindow>
#include <QObject>
#include <QListWidget>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QHostAddress>
#include <QTimer>
#include <QString>
#include <QDate>
#include <QDebug>
#include <QList>
#include <QMap>
#include <string>
#include "mylistwidget.h"
#include "msginfo.h"


/* 用户信息结构体 */
typedef struct chat_host_s
{
    QString hostname;
    QString address;
}chat_host_t;

class MyListWidget;
class FindTerminal : public QObject
{
    Q_OBJECT
public:
    explicit FindTerminal(QObject *parent = 0);
    ~FindTerminal();
    /* 设置观察者 */
    void AddBrowser(MyListWidget *widget);
    /* 刷新好友列表，通过发送多播数据 */
    void RefreshHostList();
    /* 获取map */
    const QMap<int, chat_host_t>& GetMap()
    {
        return m_hostmap;
    }

private:
    void __Init();

signals:

private slots:
    /* 多播发送函数 */
    void slot_send_msg();
    /* 多播接受函数 */
    void slot_recv_multicast_msg();
    /* 单播接受函数 */
    void slot_recv_single_msg();
private:
    /* 接受信息，病更新用户列表 */
    void __Recv_Msg(QUdpSocket *socket, QHostAddress *address);

private:
    /* 观察者模式：保存界面的指针，当别的终端发生变化时（上线、下线），
    *            更新用户列表后通过emit通知界面用户列表进行更新。
    */
    MyListWidget *m_pWidget;
    /* 多播发送套接字 */
    QUdpSocket *m_pSend;
    /* 多播接受套接字 */
    QUdpSocket *m_pMultiRecv;
    /* 单播接受套接字 */
    QUdpSocket *m_pSingleRecv;
    /* 多播定时器,udp可能丢失，应该定时发送一个多播数据让对端更有可能发现 */
    QTimer *m_pTimer;
    /* 本地用户网卡接口列表(终端有多个网卡时，选择其中一个),也可能只有一个，那么不提供选择 */
    QHostInfo m_host;
    /* 在线用户列表,int为ip对应的整形 */
    QMap<int, chat_host_t> m_hostmap;
};

#endif // FINDTERMINAL_H
