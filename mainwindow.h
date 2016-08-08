#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QFileDialog>
#include <QLineEdit>
#include <QScrollBar>
#include <QtMath>
#include <QGraphicsDropShadowEffect>
#include <string>
#include <QMovie>
#include <QPoint>
#include <QTimer>
#include <QDateTime>
#include <QTextEdit>
#include <QImage>
#include <QCamera>
#include <QVideoWidget>

#include "findterminal.h"
#include "textchat.h"
#include "mylistwidget.h"
#include "myextextedit.h"
#include "transferfile.h"
#include "transferpic.h"
#include "videodisplay.h"

/*  SIGNAL/SLOT理解：
 *      signal（int a, char &b, string * c）
 *      {
 *            slot(a, b, c);
 *      }
 *
*/

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *);
    bool eventFilter(QObject *obj, QEvent *e);

private:
    /* 初始化控件，信号 */
    void __Init();
    /* 设置聊天框环境 */
    void __Set_Session(bool yes);

private slots:  /* ------------窗口控件槽函数---------------- */
    /* 刷新好友列表 */
    void on_BTN_REFRESH_clicked();
    /* 双击请求聊天 */
    void on_LIST_HOST_doubleClicked(const QModelIndex &index);
    /* 发送消息 */
    void on_BTN_SEND_clicked();
    /* 关闭程序 */
    void on_BTN_WINDOW_CLOSE_clicked();
    /* 最小化 */
    void on_BTN_MIN_clicked();
    /* 关闭会话 */
    void on_BTN_SESSION_CLOSE_clicked();
    /* 发送图片 */
    void on_BTN_SEND_PIC_clicked();

    void on_BTN_VIDEO_clicked();

    void on_BTN_SEND_EMOJI_clicked();

    void on_TABLE_EMOJI_clicked(const QModelIndex &index);

    void on_BTN_SHAKE_clicked();

    void on_BTN_FILE_clicked();

    void on_COMBO_DOWN_FILE_LIST_currentIndexChanged(const QString &arg1);


public slots: /* --------------文本消息槽函数---------------- */
    /* 请求聊天的结果，被接受（true）或者拒绝（false）*/
    void slot_request_result(bool ret, const chat_host_t& peerhost);
    /* 请求聊天消息到达,btn返回用户点击的按钮 */
    void slot_request_arrive(QString text, QMessageBox::StandardButton &btn);
    /* 文本消息到达，通知窗口更新 */
    void slot_recv_text_msg(QList<QString>& text, QList<QString>& emojis);
    /* 关闭连接信号函数 */
    void slot_peer_close();
    /* 通信出错 */
    void slot_send_error();
    /* 显示时间 */
    void slot_show_time();

    void slot_shake_window();
    /* */
    void slot_recv_file_success(const QString& file);
    void slot_recv_picture_success(const QString& file);

    void slot_get_image(const QImage& image);

signals:
    /* 主线程通知 */
    void signal_append_file_task(const QString& filepath);
    void signal_append_picture_task(const QString& filepath);

public slots: /* --------------视频信息槽函数---------------- */


private:
    Ui::MainWindow              * ui;
    /* 用于文本框显示时间的计时 */
    QTimer                      * m_pShowTimer;
    bool                          m_is_show_time;

    bool                          m_is_show_emoji_table;

    /* 正在聊天的对端用户的信息 */
    chat_host_t                   m_peerhost;
    /* window move position */
    QPoint                        m_position;
    /* 多播终端发现接口 */
    FindTerminal                * m_pFindTerminal;
    /* 文本聊天接口 */
    TextChat                    * m_pTextChat;
    /* 文件服务 */
    MyFileThread_Client         * m_pFileClient;
    MyFileThread_Server         * m_pFileServer;

    /**/
    MyPictureThread_Client      * m_pPicClient;
    MyPictureThread_Server      * m_pPicServer;

    MyVideo_Send_Thread         * m_pVideoSend;
    MyVideo_Recv_Thread         * m_pVideoRecv;
    VideoDisplay_Recv           * m_pRecvDisplay;

};

#endif // MAINWINDOW_H
