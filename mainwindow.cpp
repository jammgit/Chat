#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->__Init();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::__Init()
{

    {
    /* 初始化控件 */
    ui->TEXT_MSG_RECORD->setReadOnly(true);
    QPalette pl = ui->TEXT_MSG_RECORD->palette();
    pl.setBrush(QPalette::Base,QBrush(QColor(180,180,180,150)));
    ui->TEXT_MSG_RECORD->setPalette(pl);
    pl = ui->TEXT_MSG_SEND->palette();
    pl.setBrush(QPalette::Base,QBrush(QColor(180,180,180,150)));
    ui->TEXT_MSG_SEND->setPalette(pl);
    pl = ui->LIST_HOST->palette();
    pl.setBrush(QPalette::Base,QBrush(QColor(180,180,180,150)));
    ui->LIST_HOST->setPalette(pl);
    pl = ui->LABEL_OTHER->palette();
    pl.setBrush(QPalette::Base,QBrush(QColor(180,180,180,150)));
    ui->LABEL_OTHER->setPalette(pl);
    pl = ui->LABEL_SELF->palette();
    pl.setBrush(QPalette::Base,QBrush(QColor(180,180,180,150)));
    ui->LABEL_SELF->setPalette(pl);
    ui->BTN_REFRESH->setStyleSheet("QPushButton{background-color:black;\
                                                color: white;   border-radius: 10px;  border: 2px groove gray;\
                                                border-style: outset;}"
                                                "QPushButton:hover{background-color:white; color: black;}"
                                                "QPushButton:pressed{background-color:rgb(85, 170, 255);\
                                                 border-style: inset; }"
                                               );
    ui->BTN_SEND->setStyleSheet("QPushButton{background-color:black;\
                                            color: white;   border-radius: 10px;  border: 2px groove gray;\
                                            border-style: outset;}"
                                            "QPushButton:hover{background-color:white; color: black;}"
                                            "QPushButton:pressed{background-color:rgb(85, 170, 255);\
                                             border-style: inset; }"
                                           );
    this->setStyleSheet("QMainWindow{background-image: url(:/src/bg1.png)}");
    }

    m_pFindTerminal = new FindTerminal;
    m_pFindTerminal->AddBrowser(ui->LIST_HOST);
    m_pTextChat = new TextChat;
    /* 初始化文本聊天相关的connect */
    connect(m_pTextChat, SIGNAL(signal_request_result(bool)), this, SLOT(slot_request_result(bool)));
    connect(m_pTextChat, SIGNAL(signal_request_arrive(QString,QMessageBox::StandardButton&)),
            this, SLOT(slot_request_arrive(QString,QMessageBox::StandardButton&)));
    connect(m_pTextChat, SIGNAL(signal_recv_msg(QString)), this, SLOT(slot_recv_text_msg(QString)));
    connect(m_pTextChat, SIGNAL(signal_peer_close()), this, SLOT(slot_peer_close()));
}

//////////////////////////////////////////////////////////////////////////
/// 窗口控件槽函数
//////////////////////////////////////////////////////////////////////////
/* 更新频率的限制要考虑 */
void MainWindow::on_BTN_REFRESH_clicked()
{
    m_pFindTerminal->RefreshHostList();
}

void MainWindow::on_LIST_HOST_doubleClicked(const QModelIndex &index)
{
    /* 分离出ip */
    QString str = (ui->LIST_HOST->item(index.row()))->text();
    std::string stdstr = str.toStdString();
    char ip[256];
    int iplen=0;
    const char *p = stdstr.c_str();
    while (*p!='(')
        p++;
    p++;
    while (*p!=')')
    {
        ip[iplen] = *p;
        ++p;
        iplen++;
    }
    ip[iplen] = '\0';
    qDebug() << QString(ip);
    /* 建立连接 */
    m_pTextChat->ConnectHost(QHostAddress(QString(ip)));
}

void MainWindow::on_BTN_SEND_clicked()
{
    m_pTextChat->SendMsg(ui->TEXT_MSG_SEND->toPlainText());
    ui->TEXT_MSG_SEND->clear();
    ui->TEXT_MSG_RECORD->setText(ui->TEXT_MSG_RECORD->toPlainText()+'\n'+
                                 ui->TEXT_MSG_SEND->toPlainText());
}

////////////////////////////////////////////////////////////////////////
/// 文本消息槽函数
////////////////////////////////////////////////////////////////////////
void MainWindow::slot_peer_close()
{
    QMessageBox::information(this, "聊天关闭", "对方结束了聊天");
    ui->TEXT_MSG_RECORD->clear();
    ui->TEXT_MSG_SEND->clear();
}

void MainWindow::slot_recv_text_msg(QString text)
{
    ui->TEXT_MSG_RECORD->setText(ui->TEXT_MSG_RECORD->toPlainText()+'\n'+text);
}

void MainWindow::slot_request_arrive(QString text, QMessageBox::StandardButton &btn)
{
    QMessageBox::StandardButton b = QMessageBox::information(this, "聊天请求", text,
                                                             QMessageBox::StandardButton::Yes
                                                             | QMessageBox::StandardButton::No);
    btn = b;
}

void MainWindow::slot_request_result(bool ret)
{
    if (ret)
    {/* 聊天请求被接受 */
        QMessageBox::information(this, "请求成功", "对方已接受聊天请求，现在可以开始聊天");
    }
    else
    {/* 聊天请求被拒绝 */
        QMessageBox::information(this, "请求失败", "对方已拒绝聊天请求");
    }
}


