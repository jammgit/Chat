#ifndef MYEXTEXTEDIT_H
#define MYEXTEXTEDIT_H

#include <QObject>
#include <QTextEdit>
#include <QUrl>
#include <QString>
#include <QMovie>
#include <QList>
#include <QFile>
#include <QDebug>
#include <QMap>
#include <QHash>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>

class MyExTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    MyExTextEdit(QWidget *parent = nullptr);
    void AddAnimation(const QUrl& url, const QString& fileName);

private slots:
    void slot_animate(int a);

protected:
    void contextMenuEvent(QContextMenuEvent *e);

private:
    QList<QMovie *>         m_lstMovie;
    QList<QUrl>             m_lstUrl;
    QHash<QMovie*, QUrl>    m_urls;
};


#endif // MYEXTEXTEDIT_H
