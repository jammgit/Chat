#include "findterminal.h"

FindTerminal::FindTerminal(QObject *parent) : QObject(parent)
{
    m_pWidget = nullptr;
    this->__Init();
}

FindTerminal::~FindTerminal()
{
    m_pSend->close();
    m_pMultiRecv->close();
    m_pSingleRecv->close();

    delete m_pTimer;
    delete m_pSingleRecv;
    delete m_pMultiRecv;
    delete m_pSend;
}

void FindTerminal::__Init()
{
    QString hname = QHostInfo::localHostName();
    m_host = QHostInfo::fromName(hname);
    qDebug() << "主机名" << hname;
    /* 初始化本地地址,获取所有可能的网卡接口 */
    QList<QHostAddress> hostlist;
    foreach (QHostAddress address, QNetworkInterface::allAddresses())
    {
        if (strncmp(address.toString().toStdString().c_str(),"10", 2) == 0
                || strncmp(address.toString().toStdString().c_str(),"172", 3) == 0
                || strncmp(address.toString().toStdString().c_str(),"192", 3) == 0)
        {/* 如果是三种内网地址的一种，则添加到host链表 */
            qDebug() << "Address:" << address;
            hostlist.push_back(address);
        }
    }
    m_host.setAddresses(hostlist);


    /* 初始化网络接口 */
    m_pSend = new QUdpSocket(this);
    m_pSend->open(QIODevice::WriteOnly);
    m_pMultiRecv = new QUdpSocket(this);
    m_pMultiRecv->open(QIODevice::ReadOnly);
    m_pSingleRecv = new QUdpSocket(this);

    /* 单播接受所有ipv4且端口是9999的数据 */
    m_pSingleRecv->bind(QHostAddress::AnyIPv4, SINGLE_PORT,
                       QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);
    connect(m_pSingleRecv, SIGNAL(readyRead()), this, SLOT(slot_recv_single_msg()));

    /* 多播接受所有ipv4且端口是9999的数据 */
    m_pMultiRecv->bind(QHostAddress::AnyIPv4, MULTICAST_PORT,
                       QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);
    /* 加入多播组 */
    m_pMultiRecv->joinMulticastGroup(QHostAddress(MULTICAST_ADDR));
    connect(m_pMultiRecv, SIGNAL(readyRead()), this, SLOT(slot_recv_multicast_msg()));

    m_pTimer = new QTimer;
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(slot_send_msg()));
    //m_pTimer->start(1000);

}

void FindTerminal::AddBrowser(MyListWidget *widget)
{
    m_pWidget = widget;
}

void FindTerminal::RefreshHostList()
{
    /* 先清空map */
    m_hostmap.clear();
    m_pWidget->clear();
    this->slot_send_msg();
}

void FindTerminal::slot_send_msg()
{
    QHostAddress addr(MULTICAST_ADDR);
    if (m_pSend->writeDatagram(m_host.hostName().toUtf8(), addr, MULTICAST_PORT) < 0)
    {
        QMessageBox::information(nullptr, "Multicast Error", "Please reclick refresh button of host list");
    }
}

/*  描述：用户上线时，通过多播通知已在线的用户，此时接收到新用户信息的用户
 *       需要返回自己的主机信息给此新用户。
 */
void FindTerminal::slot_recv_multicast_msg()
{
    QHostAddress addr;
    this->__Recv_Msg(m_pMultiRecv, &addr);
    /* 对于多播套接字接收到信息还需要回复发送者,使用单播套接字 */
    /* 响应用户的多播请求 */
    m_pSend->writeDatagram(m_host.hostName().toUtf8(), addr, SINGLE_PORT);
}

void FindTerminal::slot_recv_single_msg()
{
    this->__Recv_Msg(m_pSingleRecv, nullptr);
}

/* 信息接受函数,address值返回参数 */
void FindTerminal::__Recv_Msg(QUdpSocket *socket, QHostAddress *address)
{
    bool isnew = false;
    while (socket->hasPendingDatagrams())
    {
        /* 获取数据大小 */
        qint64 size = socket->pendingDatagramSize();
        if (size < 0)
        {
            return;
        }
        char buffer[256];
        QHostAddress addr;
        qint64 ret = socket->readDatagram(buffer, 256, &addr);
        if (address)
            *address = addr;
        /* 忽略自己发送的多播信息 */
        foreach (QHostAddress a, m_host.addresses()) {
            if (a.toString() == addr.toString())
            {
                qDebug() << "Equal local host";
                return;
            }
        }
        buffer[ret] = '\0';

        /* 判断本机是否已存有该多播发起者的地址信息 */
        int ip = addr.toString().toInt();
        if (m_hostmap.contains(ip))
        {/* MAP已包含此用户信息 */
            return;
        }
        isnew = true;
        /* 添加一个新用户到用户列表 */
        chat_host_t host;
        host.address = addr.toString();
        host.hostname = QString(buffer);
        m_hostmap[ip] = host;
    }
    if (isnew && m_pWidget && dynamic_cast<MyListWidget *>(m_pWidget))
        /* 通知好友列表更新 */
        m_pWidget->Update(this);
}

