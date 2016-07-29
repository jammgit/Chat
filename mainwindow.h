#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QFileDialog>
#include <QScrollBar>
#include <QtMath>
#include <QGraphicsDropShadowEffect>
#include <string>
#include <QPoint>
#include <QTimer>
#include <QDateTime>
#include <QTextEdit>
#include "findterminal.h"
#include "textchat.h"
#include "mylistwidget.h"
#include "videodisplay.h"

/*  SIGNAL/SLOT理解：
 *      signal（int a, char &b, string * c）
 *      {
 *            slot(a, b, c);
 *      }
 *
*/

/* 文本框格式宏 */
#define TEXT_FRONT          QString("<p align=\"%1\"><font style=\"font-family:%2\" color=\"%3\" size=\"%4\">")
#define TEXT_BACK           QString("</font></p>")
#define PIC_HTML_STRING     QString("<p align=\"%1\"><img src=\"%2\" height=\"%3\" width=\"%4\"><p>")
#define RIGHT               "right"
#define LEFT                "left"
#define CENTER              "center"
#define TEXT_COLOR          "#505050"
#define TEXT_COLOR_2        "#000000"
#define NAME_COLOR          "#0099FF"
#define TIME_COLOR          "#123456"
#define FONT_SIZE           "2"
#define FONT                "微软雅黑"
#define TIME_DISPLAY_SPACE  10000

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

public slots: /* --------------文本消息槽函数---------------- */
    /* 请求聊天的结果，被接受（true）或者拒绝（false）*/
    void slot_request_result(bool ret, const chat_host_t& peerhost);
    /* 请求聊天消息到达,btn返回用户点击的按钮 */
    void slot_request_arrive(QString text, QMessageBox::StandardButton &btn);
    /* 文本消息到达，通知窗口更新 */
    void slot_recv_text_msg(QList<QString>& text);
    /* 关闭连接信号函数 */
    void slot_peer_close();
    /* 通信出错 */
    void slot_send_error();
    /* 提升视频窗口 */
    void slot_raise_video();
    /* 显示时间 */
    void slot_show_time();

public slots: /* --------------视频信息槽函数---------------- */


private:
    Ui::MainWindow *ui;
    /* 用于文本框显示时间的计时 */
    QTimer *m_pTimeSpace;
    bool m_isshow;

    /* 执行视频窗口的的raise */
    QTimer *m_pTimer;

    QPoint m_position;
    /* 多播终端发现接口 */
    FindTerminal *m_pFindTerminal;
    /* 文本聊天接口 */
    TextChat *m_pTextChat;
    /* 摄像头接口 */
    VideoDisplay *m_pVideo;
    /* 正在聊天的对端用户的信息 */
    chat_host_t m_peerhost;
    /* 本机地址信息 */
//    QHostInfo m_host;
};

#endif // MAINWINDOW_H
