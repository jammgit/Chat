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
    delete m_pTextChat;
    delete m_pFindTerminal;
    delete m_pShowTimer;
}

void MainWindow::__Init()
{

    {
        /* 初始化控件 */

        /* set border */
        //ui->LABEL_SELF->setFrameShape (QFrame::Box);
        //ui->LABEL_OTHER->setFrameShape (QFrame::Box);
        QPalette palabel;
        palabel.setColor(QPalette::WindowText,QColor(51,204,255));
        ui->label->setPalette(palabel);

        ui->LABEL_SELF->setStyleSheet("border: 1px solid  #000000");
        ui->LABEL_OTHER->setStyleSheet("border: 1px solid  #000000");

        /* set event for key */
        ui->TEXT_MSG_SEND->installEventFilter(this);
        ui->TEXT_MSG_RECORD->setEnabled(false);
        ui->TEXT_MSG_SEND->setEnabled(false);
        ui->BTN_SEND->setEnabled(false);
        ui->BTN_SEND_PIC->setEnabled(false);
        ui->BTN_SESSION_CLOSE->setEnabled(false);
        ui->BTN_SEND_EMOJI->setEnabled(false);
        ui->BTN_SHAKE->setEnabled(false);
        ui->BTN_FILE->setEnabled(false);
        ui->COMBO_DOWN_FILE_LIST->setEditable(false);

        QPalette pa;
        pa.setColor(QPalette::WindowText,QColor(0,180,180));
        ui->LABEL_CHAT_WITH_WHO->setPalette(pa);

        /* 设置透明textedit */
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

        /* init vertical scroll style */
        QFile file("./qss/vscroll.qss");
        file.open(QFile::ReadOnly);
        QString str(file.readAll());
        file.close();
        QScrollBar *sroll = ui->TEXT_MSG_RECORD->verticalScrollBar();
        sroll->setStyleSheet(str);
        sroll->setMinimum(0);
        sroll->setMaximum(100);
        sroll = ui->TEXT_MSG_SEND->verticalScrollBar();
        sroll->setStyleSheet(str);

        /* set emoji table widget */
        m_is_show_emoji_table = true;
        ui->TABLE_EMOJI->hide();
        ui->TABLE_EMOJI->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->TABLE_EMOJI->verticalHeader()->setHidden(true);
        ui->TABLE_EMOJI->horizontalHeader()->setHidden(true);
        ui->TABLE_EMOJI->verticalScrollBar()->setStyleSheet(str);
        QDir dir("./emoji");
        QStringList filters;
        filters << "*.gif";
        dir.setNameFilters(filters);
        int count = dir.count();
        int rows = count/9+1;
        ui->TABLE_EMOJI->setRowCount(rows);
        ui->TABLE_EMOJI->setColumnCount(9);
        for (int i = 0; i < 9; i++)
            ui->TABLE_EMOJI->setColumnWidth(i,60);
        for (int j = 0; j < rows; j++)
            ui->TABLE_EMOJI->setRowHeight(j,60);
        int idx=0;
        while (idx < count)
        {
            QLabel *label = new QLabel;
            label->setStyleSheet("QLabel:hover{border: 1px solid  #000000}");
            label->setScaledContents(true);
            QMovie *movie = new QMovie(QString("./emoji/%1.gif").arg(idx+1));
            label->setMovie(movie);
            movie->start();
            ui->TABLE_EMOJI->setCellWidget(idx/9,idx%9,label);
            ++idx;
        }


        /* 设置阴影 */
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


        /* set button style */
        QFile f("./qss/pushbutton.qss");
        f.open(QFile::ReadOnly);
        str = f.readAll();
        f.close();
        ui->BTN_REFRESH->setStyleSheet(str);
        ui->BTN_SEND->setStyleSheet(str);
        ui->BTN_SESSION_CLOSE->setStyleSheet(str);
        ui->BTN_MIN->setStyleSheet(str);
        ui->BTN_WINDOW_CLOSE->setStyleSheet(str);
        ui->BTN_SEND_PIC->setStyleSheet(str);
        ui->BTN_VIDEO->setStyleSheet(str);
        ui->BTN_SEND_EMOJI->setStyleSheet(str);
        ui->BTN_FILE->setStyleSheet(str);
        ui->BTN_SHAKE->setStyleSheet(str);

        QFile fcom("./qss/combobox.qss");
        fcom.open(QFile::ReadOnly);
        str = fcom.readAll();
        fcom.close();
        ui->COMBO_DOWN_FILE_LIST->setStyleSheet("QComboBox {background-color:transparent;} "
                                                " QToolTip {background-color:white;color:black;} ");
        ui->COMBO_DOWN_FILE_LIST->addItem(QString("asdvcbvcbvcbvcbcad"));
        ui->COMBO_DOWN_FILE_LIST->addItem(QString("asdsaad"));
        ui->COMBO_DOWN_FILE_LIST->addItem(QString("哈哈"));

        ui->COMBO_DOWN_FILE_LIST->setLineEdit(ui->LINE_COMBO);
        ui->COMBO_DOWN_FILE_LIST->lineEdit()->setStyleSheet("QLineEdit {background-color:transparent;} ");


        this->setWindowFlags(Qt::FramelessWindowHint );//无边框
        /* 设置阴影必须带上这一句 */
        //this->setAttribute(Qt::WA_TranslucentBackground);
        /* 直接使用资源文件的资源，路径则为... */
        this->setStyleSheet("QMainWindow{background-image: url(:/src/bg_4.png); border: 1px solid  #1e90ff}");
        //ui->pushButton->setStyleSheet("QPushButton{border-radius:5px;border-width:0px;}");           设置透明
    }
    /* 没TIME_DISPLAY_SPACE秒显示一次时间 */
    m_pShowTimer = new QTimer;
    connect(m_pShowTimer, SIGNAL(timeout()), this, SLOT(slot_show_time()));
    m_is_show_time = true;


    /* 初始化多播、文本聊天接口 */
    m_pFindTerminal = new FindTerminal;
    m_pFindTerminal->AddBrowser(ui->LIST_HOST);
    m_pTextChat = new TextChat;
    m_pTextChat->SetFindTerminal(m_pFindTerminal);


    /* 文件服务开始监听 */
    m_pFileServer = new MyFileThread_Server(this);
    m_pFileServer->start();
    m_pFileClient = nullptr;
    /* 图片服务开始监听 */
    m_pPicServer = new MyPictureThread_Server(this);
    m_pPicServer->start();
    m_pPicClient = nullptr;

    /* 视频:发送(服务端) */
    QVideoWidget *w = new QVideoWidget(ui->LABEL_SELF);
    w->setGeometry(1,1,ui->LABEL_SELF->width()-2,ui->LABEL_SELF->height()-2);
    m_pVideoSend = new MyVideo_Send_Thread(w,this);
    m_pVideoSend->start();
    m_pVideoRecv = nullptr;
    /* 视频:接受是主动连接端，在主动连接时再创建 */


    /* 初始化文本聊天相关的connect */
    connect(m_pTextChat, SIGNAL(signal_request_result(bool, const chat_host_t&)),
            this, SLOT(slot_request_result(bool, const chat_host_t&)));
    connect(m_pTextChat, SIGNAL(signal_request_arrive(QString,QMessageBox::StandardButton&)),
            this, SLOT(slot_request_arrive(QString,QMessageBox::StandardButton&)));
    connect(m_pTextChat, SIGNAL(signal_recv_msg(QList<QString>&, QList<QString>&)),
            this, SLOT(slot_recv_text_msg(QList<QString>&, QList<QString>&)));
    connect(m_pTextChat, SIGNAL(signal_peer_close()), this, SLOT(slot_peer_close()));
    connect(m_pTextChat, SIGNAL(signal_send_error()), this, SLOT(slot_send_error()));
    connect(m_pTextChat, SIGNAL(signal_shake_window()), this, SLOT(slot_shake_window()));

}

/* 设置聊天环境 */
void MainWindow::__Set_Session(bool yes)
{
    if (yes)
    {
        ui->TEXT_MSG_RECORD->setEnabled(true);
        ui->TEXT_MSG_SEND->setEnabled(true);
        ui->BTN_SEND->setEnabled(true);
        ui->BTN_SEND_PIC->setEnabled(true);
        ui->BTN_SESSION_CLOSE->setEnabled(true);
        ui->BTN_SEND_EMOJI->setEnabled(true);
        ui->BTN_SHAKE->setEnabled(true);
        ui->BTN_FILE->setEnabled(true);
        ui->COMBO_DOWN_FILE_LIST->setEnabled(true);
        ui->LABEL_CHAT_WITH_WHO->setText(m_peerhost.hostname);
    }
    else
    {
        ui->TEXT_MSG_RECORD->clear();
        ui->TEXT_MSG_SEND->clear();
        ui->COMBO_DOWN_FILE_LIST->clear();
        ui->COMBO_DOWN_FILE_LIST->setEditable(false);
        ui->TEXT_MSG_RECORD->setEnabled(false);
        ui->TEXT_MSG_SEND->setEnabled(false);
        ui->BTN_SEND->setEnabled(false);
        ui->BTN_SEND_PIC->setEnabled(false);
        ui->BTN_SESSION_CLOSE->setEnabled(false);
        ui->BTN_SEND_EMOJI->setEnabled(false);
        ui->BTN_SHAKE->setEnabled(false);
        ui->BTN_FILE->setEnabled(false);
        ui->LABEL_CHAT_WITH_WHO->setText("");
    }
}

/* 窗口移动 */
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_position = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

/* 窗口移动 */
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        move(event->globalPos() - m_position);
        event->accept();
    }
}

/* 文本框快捷键设置 */
bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    Q_ASSERT(obj == ui->TEXT_MSG_SEND);
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent *event = static_cast<QKeyEvent*>(e);
        /* Key_Enter is in small key(number)*/
        if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && (event->modifiers() & Qt::ShiftModifier))
        {/* 换行 */

        }
        else if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        {/* 发送 */
            this->on_BTN_SEND_clicked();
            return true;
        }
    }
    return false;
}

/* 画阴影 */
void MainWindow::paintEvent(QPaintEvent *)
{
//    event = event;
//    QPainterPath path;
//    path.setFillRule(Qt::WindingFill);
//    path.addRect(10, 10, this->width()-20, this->height()-20);

//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing, true);
//    /* 此图像应放在build文件夹的子文件夹src里 */
//    painter.fillPath(path, QBrush(QPixmap("src/bg_4.png")));

//    QColor color(0, 0, 0, 50);
//    for(int i=0; i<10; i++)
//    {
//        QPainterPath path;
//        path.setFillRule(Qt::WindingFill);
//        path.addRect(10-i, 10-i, this->width()-(10-i)*2, this->height()-(10-i)*2);
//        color.setAlpha(150 - qSqrt(i)*50);
//        painter.setPen(color);
//        painter.drawPath(path);
//    }
}


//////////////////////////////////////////////////////////////////////////
/// 窗口控件槽函数
//////////////////////////////////////////////////////////////////////////

/* 更新频率的限制要考虑 */
void MainWindow::on_BTN_REFRESH_clicked()
{
    m_pFindTerminal->RefreshHostList();
}

/* 请求聊天 */
void MainWindow::on_LIST_HOST_doubleClicked(const QModelIndex &index)
{
    ui->LIST_HOST->setEnabled(false);
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
    /* create file , picture socket */
    //emit this->signal_create_socket(QHostAddress(QString(ip)));

    if (!m_pFileClient)
    {
        m_pFileClient = new MyFileThread_Client(this, QHostAddress(QString(ip)));
    }
    else
    {
        delete m_pFileClient;
        m_pFileClient = new MyFileThread_Client(this, QHostAddress(QString(ip)));
    }
    m_pFileClient->start();

    if (!m_pPicClient)
    {
        m_pPicClient = new MyPictureThread_Client(this, QHostAddress(QString(ip)));
    }
    else
    {
        delete m_pPicClient;
        m_pPicClient = new MyPictureThread_Client(this, QHostAddress(QString(ip)));
    }
    m_pPicClient->start();

    if (!m_pVideoRecv)
    {
        m_pVideoRecv = new MyVideo_Recv_Thread(this,QHostAddress(QString(ip)));
    }
    else
    {
        delete m_pVideoRecv;
        m_pVideoRecv = new MyVideo_Recv_Thread(this,QHostAddress(QString(ip)));
    }
    m_pVideoRecv->start();
    while (!m_pVideoRecv->GetVideoDisplay());
    connect(m_pVideoRecv->GetVideoDisplay(), SIGNAL(signal_get_image(QImage)),
            this, SLOT(slot_get_image(QImage)));


    /* 建立三个必须的连接连接 */
    bool b = m_pTextChat->ConnectHost(QHostAddress(QString(ip)));
    if (!b)
    {
        QMessageBox::information(nullptr, "网络错误", "建立网络连接出现错误，请重试");
        ui->LIST_HOST->setEnabled(true);
        return;
    }

}

/* 发送文本消息,每10秒以上间隔才显示一次时间 */
void MainWindow::on_BTN_SEND_clicked()
{
    if (ui->TEXT_MSG_SEND->toPlainText().size() == 0)
        return ;

    if (m_is_show_time)
    {
        ui->TEXT_MSG_RECORD->setHtml(
                    ui->TEXT_MSG_RECORD->toHtml()
                    + TEXT_FRONT.arg(CENTER, FONT, TIME_COLOR, FONT_SIZE) + QDateTime::currentDateTime().toString() + TEXT_BACK
                    );
        m_pShowTimer->start(TIME_DISPLAY_SPACE);
        m_is_show_time = false;
    }
    QString tmp(ui->TEXT_MSG_SEND->toHtml());
    QString htmltext = m_pTextChat->SendMsg(MSG_TEXT, tmp);

    ui->TEXT_MSG_RECORD->setHtml(
                ui->TEXT_MSG_RECORD->toHtml()
                + htmltext);

    ui->TEXT_MSG_SEND->clear();
    /* 设置滚动条置底 */
    ui->TEXT_MSG_RECORD->verticalScrollBar()->setValue(32767);
}

/* 结束聊天 */
void MainWindow::on_BTN_SESSION_CLOSE_clicked()
{
    m_pTextChat->Close();

    if (m_pFileClient)
    {
        m_pFileClient->exit();
        delete m_pFileClient;
        m_pFileClient = nullptr;
    }
    if (m_pFileServer)
    {
        m_pFileServer->exit();
        m_pFileServer->start();
    }

    if (m_pPicClient)
    {
        m_pPicClient->exit();
        delete m_pPicClient;
        m_pPicClient = nullptr;
    }
    if (m_pPicServer)
    {
        m_pPicServer->exit();
        m_pPicServer->start();
    }

//    m_pFileChat->stop();
//    m_pPicChat->stop();

    QMessageBox::information(this, "提示", "聊天结束");
    this->__Set_Session(false);
    ui->LIST_HOST->setEnabled(true);
}

/* 发送图片 */
void MainWindow::on_BTN_SEND_PIC_clicked()
{
    if (m_is_show_time)
    {
        ui->TEXT_MSG_RECORD->setHtml(
                    ui->TEXT_MSG_RECORD->toHtml()
                    + TEXT_FRONT.arg(CENTER, FONT, TIME_COLOR, FONT_SIZE) + QDateTime::currentDateTime().toString() + TEXT_BACK
                    );
        m_pShowTimer->start(TIME_DISPLAY_SPACE);
        m_is_show_time = false;
    }
    QStringList   fileNameList;
    QFileDialog* fd = new QFileDialog(this);        //创建对话框
    fd->resize(240,320);                            //设置显示的大小
    fd->setNameFilter(PICTURE_NAME_FILTER);         //设置图片过滤器
    fd->setFileMode(QFileDialog::ExistingFiles);
    fd->setViewMode(QFileDialog::List);             //设置浏览模式，有 列表（list） 模式和 详细信息（detail）两种方式
    if ( fd->exec() == QDialog::Accepted )          //如果成功的执行
    {
        fileNameList = fd->selectedFiles();         //返回文件列表的名称
    }
    else
    {
        fd->close();
        return;
    }
    fd->close();
    qDebug() << fileNameList;

    QString html;
    foreach (QString path, fileNameList) {
        emit this->signal_append_picture_task(path);
        QImage image(path);
        html += PIC_HTML_STRING.arg(
                RIGHT,
                path,
                QString::number(image.height()/(image.width()/200)),
                QString::number(200));
    }

    ui->TEXT_MSG_RECORD->setHtml(
                ui->TEXT_MSG_RECORD->toHtml()
                + html);
    /* 设置滚动条置底 */
    ui->TEXT_MSG_RECORD->verticalScrollBar()->setValue(32767);
}


void MainWindow::on_BTN_FILE_clicked()
{
    if (m_is_show_time)
    {
        ui->TEXT_MSG_RECORD->setHtml(
                    ui->TEXT_MSG_RECORD->toHtml()
                    + TEXT_FRONT.arg(CENTER, FONT, TIME_COLOR, FONT_SIZE) + QDateTime::currentDateTime().toString() + TEXT_BACK
                    );
        m_pShowTimer->start(TIME_DISPLAY_SPACE);
        m_is_show_time = false;
    }
    QStringList   fileNameList;
    QFileDialog* fd = new QFileDialog(this);        //创建对话框
    fd->resize(240,320);                            //设置显示的大小
    fd->setFileMode(QFileDialog::ExistingFiles);
    fd->setViewMode(QFileDialog::List);             //设置浏览模式，有 列表（list） 模式和 详细信息（detail）两种方式
    if ( fd->exec() == QDialog::Accepted )          //如果成功的执行
    {
        fileNameList = fd->selectedFiles();         //返回文件列表的名称
    }
    else
    {
        fd->close();
        return;
    }
    fd->close();
    qDebug() << fileNameList;


    QString html;
    foreach (QString path, fileNameList) {
        emit this->signal_append_file_task(path);
        html += TEXT_FRONT.arg(RIGHT,FONT,TEXT_COLOR_3,FONT_SIZE)+"我发送了文件["+ path+"]"+TEXT_BACK;
    }
    ui->TEXT_MSG_RECORD->setHtml(
                ui->TEXT_MSG_RECORD->toHtml()
                + html);
    /* 设置滚动条置底 */
    ui->TEXT_MSG_RECORD->verticalScrollBar()->setValue(32767);
}

/*  */
void MainWindow::on_COMBO_DOWN_FILE_LIST_currentIndexChanged(const QString &)
{

}


/* 关闭程序 */
void MainWindow::on_BTN_WINDOW_CLOSE_clicked()
{
    m_pTextChat->Close();
    m_pVideoSend->exit();
    if (m_pFileServer)
    {
        m_pFileServer->exit();
        delete m_pFileServer;
        m_pFileServer = nullptr;
    }
    if (m_pFileClient)
    {
        m_pFileClient->exit();
        delete m_pFileClient;
        m_pFileClient = nullptr;
    }
//    m_pFileChat->stop();
//    m_pPicChat->stop();
    this->close();
}

/* 最小化 */
void MainWindow::on_BTN_MIN_clicked()
{
    this->showMinimized();
}


/* 关闭，开启摄像头 */
void MainWindow::on_BTN_VIDEO_clicked()
{


}


void MainWindow::on_BTN_SEND_EMOJI_clicked()
{

    if (m_is_show_emoji_table)
        ui->TABLE_EMOJI->show();
    else
        ui->TABLE_EMOJI->hide();
    m_is_show_emoji_table = !m_is_show_emoji_table;
}


void MainWindow::on_TABLE_EMOJI_clicked(const QModelIndex &index)
{
    if (m_is_show_time)
    {
        ui->TEXT_MSG_RECORD->setHtml(
                    ui->TEXT_MSG_RECORD->toHtml()
                    + TEXT_FRONT.arg(CENTER, FONT, TIME_COLOR, FONT_SIZE) + QDateTime::currentDateTime().toString() + TEXT_BACK
                    );
        m_pShowTimer->start(TIME_DISPLAY_SPACE);
        m_is_show_time = false;
    }
    ui->TABLE_EMOJI->hide();
    m_is_show_emoji_table = true;
    int row = index.row();
    int column = index.column();
    int idx = row*9+column+1;
    QString emoji = QString("./emoji/%1.gif").arg(idx);

    QString html = m_pTextChat->SendMsg(MSG_EMOJI, emoji);

    ui->TEXT_MSG_RECORD->setHtml(
                ui->TEXT_MSG_RECORD->toHtml()
                + html);

    /* QUrli's QString is html variable's src ,just a tag*/
    ui->TEXT_MSG_RECORD->AddAnimation(QUrl(emoji), emoji);  //添加一个动画.

    /* 设置滚动条置底 */
    ui->TEXT_MSG_RECORD->verticalScrollBar()->setValue(32767);
}


/* frequency of shaking window is five seconds */

void MainWindow::on_BTN_SHAKE_clicked()
{
    if (m_is_show_time)
    {
        ui->TEXT_MSG_RECORD->setHtml(
                    ui->TEXT_MSG_RECORD->toHtml()
                    + TEXT_FRONT.arg(CENTER, FONT, TIME_COLOR, FONT_SIZE) + QDateTime::currentDateTime().toString() + TEXT_BACK
                    );
        m_pShowTimer->start(TIME_DISPLAY_SPACE);
        m_is_show_time = false;
    }
    static bool isbegin = true;
    static QDateTime s = QDateTime::currentDateTime();
    QDateTime e = QDateTime::currentDateTime();
    if(isbegin || (e.toTime_t() - s.toTime_t() > 5))
    {
        s = e;
        QString tmp("你抖动了窗口");
        QString html = m_pTextChat->SendMsg(MSG_SHAKE, tmp);
        ui->TEXT_MSG_RECORD->setHtml(
                    ui->TEXT_MSG_RECORD->toHtml()
                    + html);
        isbegin = false;
    }
    /* 设置滚动条置底 */
    ui->TEXT_MSG_RECORD->verticalScrollBar()->setValue(32767);
}



////////////////////////////////////////////////////////////////////////
/// 文本消息槽函数
////////////////////////////////////////////////////////////////////////

/* 对端关闭连接 */
void MainWindow::slot_peer_close()
{
//    m_pPicChat->stop();
//    m_pFileChat->stop();

    if (m_pFileClient)
    {
        m_pFileClient->exit();
        delete m_pFileClient;
        m_pFileClient = nullptr;
    }
    if (m_pFileServer)
    {
        m_pFileServer->exit();
        m_pFileServer->start();
    }

    QMessageBox::information(nullptr, "聊天关闭", "对方结束了聊天");
    this->__Set_Session(false);
    ui->LIST_HOST->setEnabled(true);
    ui->COMBO_DOWN_FILE_LIST->clear();
}



/* 接受消息 */

void MainWindow::slot_recv_text_msg(QList<QString>& text, QList<QString>& emojis)
{
    /* 设置滚动条置底 */
    ui->TEXT_MSG_RECORD->verticalScrollBar()->setValue(0);

    if (m_is_show_time)
    {
        ui->TEXT_MSG_RECORD->setHtml(
                    ui->TEXT_MSG_RECORD->toHtml()
                    + text.front());
        text.pop_front();
        m_pShowTimer->start(TIME_DISPLAY_SPACE);
        m_is_show_time = false;
    }
    else
        text.pop_front();

    QString t = ui->TEXT_MSG_RECORD->toHtml();
    while (!text.empty())
    {
        t += text.front();
        text.pop_front();
    }
    ui->TEXT_MSG_RECORD->setHtml(t);
    /* QUrli's QString is html variable's src ,just a tag*/
    foreach (QString emoji, emojis) {
        ui->TEXT_MSG_RECORD->AddAnimation(QUrl(emoji), emoji);  //添加一个动画.
    }
    /* 设置滚动条置底 */
    ui->TEXT_MSG_RECORD->verticalScrollBar()->setValue(32767);
}


/* 聊天请求到达 */

void MainWindow::slot_request_arrive(QString text, QMessageBox::StandardButton &btn)
{
    QMessageBox::StandardButton b = QMessageBox::information(this, "聊天请求", text,
                                                             QMessageBox::StandardButton::Yes
                                                             | QMessageBox::StandardButton::No);
    btn = b;

    if (b == QMessageBox::StandardButton::No)
    {
        if (m_pFileServer)
        {
            m_pFileServer->exit();
            m_pFileServer->start();
        }
    }
}


/* 请求结果 */

void MainWindow::slot_request_result(bool ret, const chat_host_t& peerhost)
{
    if (ret)
    {
        /* 获得对端信息 */
        m_peerhost = peerhost;
        /* 聊天请求被接受 */
        QMessageBox::information(this, "请求成功", "现在可以开始聊天");
        this->__Set_Session(true);
        ui->LIST_HOST->setEnabled(false);
    }
    else
    {/* 聊天请求被拒绝 */
        QMessageBox::information(this, "请求失败", "对方已拒绝聊天请求");
        ui->LIST_HOST->setEnabled(false);
//        m_pFileChat->stop();
//        m_pPicChat->stop();

        if (m_pFileClient)
        {
            m_pFileClient->exit();
            delete m_pFileClient;
            m_pFileClient = nullptr;
        }
    }
}


/* 发送消息失败 */

void MainWindow::slot_send_error()
{
    this->__Set_Session(false);
//    m_pFileChat->stop();
//    m_pPicChat->stop();
    ui->LIST_HOST->setEnabled(true);
    ui->COMBO_DOWN_FILE_LIST->clear();
    if (m_pFileClient)
    {
        m_pFileClient->exit();
        delete m_pFileClient;
        m_pFileClient = nullptr;
    }
    if (m_pFileServer)
    {
        m_pFileServer->exit();
        m_pFileServer->start();
    }
}


void MainWindow::slot_show_time()
{
    m_is_show_time = true;

}


void MainWindow::slot_shake_window()
{
    /* 设置窗口置顶 */
//    Qt::WindowFlags flags = this->windowFlags();
//    flags |= Qt::WindowStaysOnTopHint;
//    this->setWindowFlags(flags);
//    this->show();
//    qDebug() << "show window";

    QPoint point = this->pos();
    int x = point.x();
    int y = point.y();

    int i = 6;
    bool b = true;
    while (!this->isHidden() && i>0)
    {
        if (b)
        {
            this->move(x-20, y-20);
        }
        else
        {
            this->move(x+20, y+20);
        }
        b = !b;
        i--;
        /* debug引起延时才能看到窗口移动的效果，也可以做一个小循环，qt没提供跨平台的睡眠函数 */
        qDebug() << "move window";
    }
    this->move(point);
    /* 取消置顶 */
//    this->setWindowFlags(flags & ~Qt::WindowStaysOnTopHint);
//    this->show();

}


/* */
void MainWindow::slot_recv_file_success(const QString& file)
{
    ui->COMBO_DOWN_FILE_LIST->addItem(file);
    QString html;
    html += TEXT_FRONT.arg(LEFT,FONT,TEXT_COLOR_3,FONT_SIZE)+"对方发送了文件["+ file+"]"+TEXT_BACK;

    ui->TEXT_MSG_RECORD->setHtml(
                ui->TEXT_MSG_RECORD->toHtml()
                + html);
}

void MainWindow::slot_recv_picture_success(const QString& file)
{
    ui->COMBO_DOWN_FILE_LIST->addItem(file);

    QImage image(QString("./tmp/") + file);

    ui->TEXT_MSG_RECORD->setHtml(
                ui->TEXT_MSG_RECORD->toHtml()
                + PIC_HTML_STRING.arg(
                  LEFT,
                  QString("./tmp/%1").arg(file),
                  QString::number(image.height()/(image.width()/200)),
                  QString::number(200))
                );

}

void MainWindow::slot_get_image(const QImage &image)
{
    ui->label->setPixmap(QPixmap::fromImage(image));
}










