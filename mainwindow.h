#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
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


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void __Init();

private slots:  /* ------------窗口控件槽函数---------------- */
    /* 刷新好友列表 */
    void on_BTN_REFRESH_clicked();
    /* 双击请求聊天 */
    void on_LIST_HOST_doubleClicked(const QModelIndex &index);

    void on_BTN_SEND_clicked();

public slots: /* --------------文本消息槽函数---------------- */
    /* 请求聊天的结果，被接受（true）或者拒绝（false）*/
    void slot_request_result(bool ret);
    /* 请求聊天消息到达,btn返回用户点击的按钮 */
    void slot_request_arrive(QString text, QMessageBox::StandardButton &btn);
    /* 文本消息到达，通知窗口更新 */
    void slot_recv_text_msg(QString text);
    /* 关闭连接信号函数 */
    void slot_peer_close();

public slots: /* --------------视频信息槽函数---------------- */


private:
    Ui::MainWindow *ui;
    /* 多播终端发现接口 */
    FindTerminal *m_pFindTerminal;
    /* 文本聊天接口 */
    TextChat *m_pTextChat;
};

#endif // MAINWINDOW_H
