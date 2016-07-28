#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QtMath>
#include <QGraphicsDropShadowEffect>
#include <string>
#include <QPoint>
#include <QTextEdit>
#include "findterminal.h"
#include "textchat.h"
#include "mylistwidget.h"

/*  SIGNAL/SLOT理解：
 *      signal（int a, char &b, string * c）
 *      {
 *            slot(a, b, c);
 *      }
 *
*/

#define TEXT_FRONT_SELF  QString("<p align=\"right\"><font style=\"font-family:微软雅黑\" color=\"#0099FF\" size=\"3\">")
#define TIME_FRONT_SELF  QString("<p align=\"right\"><font style=\"font-family:微软雅黑\" color=\"#0099FF\" size=\"2\">")
#define TEXT_FRONT_OTHER QString("<p align=\"left\"><font style=\"font-family:微软雅黑\" color=\"#303030\" size=\"3\">")
#define TIME_FRONT_OTHER QString("<p align=\"right\"><font style=\"font-family:微软雅黑\" color=\"#0099FF\" size=\"2\">")
#define TEXT_BACK        QString("</font></p>")


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
    void __Init();

private slots:  /* ------------窗口控件槽函数---------------- */
    /* 刷新好友列表 */
    void on_BTN_REFRESH_clicked();
    /* 双击请求聊天 */
    void on_LIST_HOST_doubleClicked(const QModelIndex &index);

    void on_BTN_SEND_clicked();

    void on_BTN_WINDOW_CLOSE_clicked();

    void on_BTN_MIN_clicked();

    void on_BTN_SESSION_CLOSE_clicked();

public slots: /* --------------文本消息槽函数---------------- */
    /* 请求聊天的结果，被接受（true）或者拒绝（false）*/
    void slot_request_result(bool ret, const chat_host_t& peerhost);
    /* 请求聊天消息到达,btn返回用户点击的按钮 */
    void slot_request_arrive(QString text, QMessageBox::StandardButton &btn);
    /* 文本消息到达，通知窗口更新 */
    void slot_recv_text_msg(QString text);
    /* 关闭连接信号函数 */
    void slot_peer_close();

public slots: /* --------------视频信息槽函数---------------- */


private:
    Ui::MainWindow *ui;
    QPoint m_position;
    /* 多播终端发现接口 */
    FindTerminal *m_pFindTerminal;
    /* 文本聊天接口 */
    TextChat *m_pTextChat;
    /* 正在聊天的对端用户的信息 */
    chat_host_t m_peerhost;
    /* 用于聊天框是否需要打印发送者名和时间的变量，初始值为true(判断依据是是否是连续的消息) */
    bool m_istrue;
};

#endif // MAINWINDOW_H
