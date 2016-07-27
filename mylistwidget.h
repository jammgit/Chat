#ifndef MYLISTWIDGET_H
#define MYLISTWIDGET_H

#include <QListWidget>
#include <QMap>
#include <QList>
#include "findterminal.h"

class FindTerminal;

class MyListWidget : public QListWidget
{
public:
    MyListWidget(QWidget *parent = nullptr);
    ~MyListWidget();
    /* 好友列表视图更新函数 */
    void Update(FindTerminal *r);

};

#endif // MYLISTWIDGET_H
