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
    //ui->TEXT_MSG_RECORD->setHtml(QString("<p align=\"right\"><img src=\"src/bg_1.png\" height=\"100\" width=\"100\"><p>"));

    /* set border */
    //ui->LABEL_SELF->setFrameShape (QFrame::Box);
    //ui->LABEL_OTHER->setFrameShape (QFrame::Box);
    ui->LABEL_SELF->setStyleSheet("border: 1px solid  #000000");
    ui->LABEL_OTHER->setStyleSheet("border: 1px solid  #000000");

    ui->TEXT_MSG_SEND->installEventFilter(this);
    ui->TEXT_MSG_RECORD->setEnabled(false);
    ui->TEXT_MSG_SEND->setEnabled(false);
    ui->BTN_SEND->setEnabled(false);

    QPalette pa;
    pa.setColor(QPalette::WindowText,QColor(0,180,180));
    ui->LABEL_CHAT_WITH_WHO->setPalette(pa);

    ui->TEXT_MSG_RECORD->setReadOnly(true);
    QPalette pl = ui->TEXT_MSG_RECORD->palette();
    pl.setBrush(QPalette::Base,QBrush(QColor(230,230,230,100)));
    ui->TEXT_MSG_RECORD->setPalette(pl);
    pl = ui->TEXT_MSG_SEND->palette();
    pl.setBrush(QPalette::Base,QBrush(QColor(230,230,230,100)));
    ui->TEXT_MSG_SEND->setPalette(pl);
    pl = ui->LIST_HOST->palette();
    pl.setBrush(QPalette::Base,QBrush(QColor(230,230,230,100)));
    ui->LIST_HOST->setPalette(pl);

    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(-5, 5);
    shadow_effect->setColor(Qt::black);
    shadow_effect->setBlurRadius(8);
    QGraphicsDropShadowEffect *shadow_effect1 = new QGraphicsDropShadowEffect(this);
    shadow_effect1->setOffset(-5, 5);
    shadow_effect1->setColor(Qt::black);
    shadow_effect1->setBlurRadius(8);
    QGraphicsDropShadowEffect *shadow_effect2 = new QGraphicsDropShadowEffect(this);
    shadow_effect2->setOffset(-5, 5);
    shadow_effect2->setColor(Qt::black);
    shadow_effect2->setBlurRadius(8);
    ui->TEXT_MSG_RECORD->setGraphicsEffect(shadow_effect);
    ui->TEXT_MSG_SEND->setGraphicsEffect(shadow_effect1);
    ui->LIST_HOST->setGraphicsEffect(shadow_effect2);

    ui->BTN_REFRESH->setStyleSheet("QPushButton{color: white;   border-radius: 15px; border-style: outset;}"
                                   "QPushButton:hover{background-color:white; color: black;}"
                                   "QPushButton:pressed{background-color:rgb(85, 170, 255);border-style:inset;}");
    ui->BTN_SEND->setStyleSheet("QPushButton{color: white;   border-radius: 15px; border-style: outset;}"
                                   "QPushButton:hover{background-color:white; color: black;}"
                                   "QPushButton:pressed{background-color:rgb(85, 170, 255);border-style:inset;}");
    ui->BTN_SESSION_CLOSE->setStyleSheet("QPushButton{color: white;   border-radius: 15px; border-style: outset;}"
                                   "QPushButton:hover{background-color:white; color: black;}"
                                   "QPushButton:pressed{background-color:rgb(85, 170, 255);border-style:inset;}");
    ui->BTN_MIN->setStyleSheet("QPushButton{color: white;   border-radius: 15px; border-style: outset;}"
                                   "QPushButton:hover{background-color:white; color: black;}"
                                   "QPushButton:pressed{background-color:rgb(85, 170, 255);border-style:inset;}");
    ui->BTN_WINDOW_CLOSE->setStyleSheet("QPushButton{color: white;   border-radius: 15px; border-style: outset;}"
                                   "QPushButton:hover{background-color:white; color: black;}"
                                   "QPushButton:pressed{background-color:rgb(85, 170, 255);border-style:inset;}");
    ui->BTN_SEND_PIC->setStyleSheet("QPushButton{color: white;   border-radius: 15px; border-style: outset;}"
                                   "QPushButton:hover{background-color:white; color: black;}"
                                   "QPushButton:pressed{background-color:rgb(85, 170, 255);border-style:inset;}");
    this->setWindowFlags(Qt::FramelessWindowHint);//无边框
    /* 设置阴影必须带上这一句 */
    this->setAttribute(Qt::WA_TranslucentBackground);
    /* 直接使用资源文件的资源，路径则为... */
    this->setStyleSheet("QMainWindow{background-image: url(:/src/bg_4.png)}");
    //ui->pushButton->setStyleSheet("QPushButton{border-radius:5px;border-width:0px;}");           设置透明
    }

    m_pFindTerminal = new FindTerminal;
    m_pFindTerminal->AddBrowser(ui->LIST_HOST);
    m_pTextChat = new TextChat;
    m_pTextChat->SetFindTerminal(m_pFindTerminal);
    /* 初始化文本聊天相关的connect */
    connect(m_pTextChat, SIGNAL(signal_request_result(bool, const chat_host_t&)),
            this, SLOT(slot_request_result(bool, const chat_host_t&)));
    connect(m_pTextChat, SIGNAL(signal_request_arrive(QString,QMessageBox::StandardButton&)),
            this, SLOT(slot_request_arrive(QString,QMessageBox::StandardButton&)));
    connect(m_pTextChat, SIGNAL(signal_recv_msg(QString)), this, SLOT(slot_recv_text_msg(QString)));
    connect(m_pTextChat, SIGNAL(signal_peer_close()), this, SLOT(slot_peer_close()));
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_position = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        move(event->globalPos() - m_position);
        event->accept();
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    Q_ASSERT(obj == ui->TEXT_MSG_SEND);
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent *event = static_cast<QKeyEvent*>(e);
        /* Key_Enter is in small key(number)*/
        if (event->key() == Qt::Key_Return && (event->modifiers() & Qt::ControlModifier))
        {
            this->on_BTN_SEND_clicked();
            return true;
        }
    }
    return false;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    event = event;
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRect(10, 10, this->width()-20, this->height()-20);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    /* 此图像应放在build文件夹的子文件夹src里 */
    painter.fillPath(path, QBrush(QPixmap("src/bg_4.png")));

    QColor color(0, 0, 0, 50);
    for(int i=0; i<10; i++)
    {
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        path.addRect(10-i, 10-i, this->width()-(10-i)*2, this->height()-(10-i)*2);
        color.setAlpha(150 - qSqrt(i)*50);
        painter.setPen(color);
        painter.drawPath(path);
    }
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
    {
        p++;
    }
    p++;
    while (*p!=')')
    {
        ip[iplen++] = *p;
        ++p;
    }
    ip[iplen] = '\0';
    qDebug() << QString(ip);
    /* 建立连接 */
    m_pTextChat->ConnectHost(QHostAddress(QString(ip)));
}

/* */
void MainWindow::on_BTN_SEND_clicked()
{
    m_pTextChat->SendMsg(ui->TEXT_MSG_SEND->toPlainText());
    ui->TEXT_MSG_RECORD->setHtml(
                ui->TEXT_MSG_RECORD->toHtml()
                + TIME_FRONT_SELF + QHostInfo::localHostName() + TEXT_BACK
                + TEXT_FRONT_SELF + ui->TEXT_MSG_SEND->toPlainText() + TEXT_BACK);
    ui->TEXT_MSG_SEND->clear();
}

void MainWindow::on_BTN_SESSION_CLOSE_clicked()
{
    m_pTextChat->Close();
    ui->LABEL_CHAT_WITH_WHO->setText("");
    QMessageBox::information(this, "提示", "聊天结束");
    ui->TEXT_MSG_RECORD->clear();
    ui->TEXT_MSG_SEND->clear();
    ui->TEXT_MSG_RECORD->setEnabled(false);
    ui->TEXT_MSG_SEND->setEnabled(false);
    ui->BTN_SEND->setEnabled(false);
}
/* */
void MainWindow::on_BTN_SEND_PIC_clicked()
{
    QStringList   fileNameList;
    QFileDialog* fd = new QFileDialog(this);        //创建对话框
    fd->resize(240,320);                            //设置显示的大小
    fd->setNameFilter("Image Files(*.png *.jpg)");  //设置文件过滤器
    fd->setFileMode(QFileDialog::ExistingFiles);
    fd->setViewMode(QFileDialog::List);             //设置浏览模式，有 列表（list） 模式和 详细信息（detail）两种方式
    if ( fd->exec() == QDialog::Accepted )          //如果成功的执行
    {
        fileNameList = fd->selectedFiles();         //返回文件列表的名称
    }
    else
        fd->close();
    qDebug() << fileNameList;
}

void MainWindow::on_BTN_WINDOW_CLOSE_clicked()
{
    m_pTextChat->Close();
    this->close();
}
void MainWindow::on_BTN_MIN_clicked()
{
    this->showMinimized();
}

////////////////////////////////////////////////////////////////////////
/// 文本消息槽函数
////////////////////////////////////////////////////////////////////////
void MainWindow::slot_peer_close()
{
    QMessageBox::information(this, "聊天关闭", "对方结束了聊天");
    ui->TEXT_MSG_RECORD->clear();
    ui->TEXT_MSG_SEND->clear();
    ui->TEXT_MSG_RECORD->setEnabled(false);
    ui->TEXT_MSG_SEND->setEnabled(false);
    ui->BTN_SEND->setEnabled(false);
    ui->LABEL_CHAT_WITH_WHO->setText("");
}

void MainWindow::slot_recv_text_msg(QString text)
{
    ui->TEXT_MSG_RECORD->setHtml(
                ui->TEXT_MSG_RECORD->toHtml()
                + TIME_FRONT_OTHER + m_peerhost.hostname + TEXT_BACK
                + TEXT_FRONT_OTHER + text + TEXT_BACK);
}

void MainWindow::slot_request_arrive(QString text, QMessageBox::StandardButton &btn)
{
    QMessageBox::StandardButton b = QMessageBox::information(this, "聊天请求", text,
                                                             QMessageBox::StandardButton::Yes
                                                             | QMessageBox::StandardButton::No);
    btn = b;
}

void MainWindow::slot_request_result(bool ret, const chat_host_t& peerhost)
{
    if (ret)
    {/* 聊天请求被接受 */
        QMessageBox::information(this, "请求成功", "现在可以开始聊天");
        ui->TEXT_MSG_RECORD->setEnabled(true);
        ui->TEXT_MSG_SEND->setEnabled(true);
        ui->BTN_SEND->setEnabled(true);
        ui->LABEL_CHAT_WITH_WHO->setText(peerhost.hostname);
        /* 获得对端信息 */
        m_peerhost = peerhost;
    }
    else
    {/* 聊天请求被拒绝 */
        QMessageBox::information(this, "请求失败", "对方已拒绝聊天请求");
    }
}










