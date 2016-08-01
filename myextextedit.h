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

class MyExTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    MyExTextEdit(QWidget *parent = nullptr);
    void AddAnimation(const QUrl& url, const QString& fileName);

private slots:
    void slot_animate(int a);

private:
    QList<QMovie *> lstMovie;
    QList<QUrl> lstUrl;
    QHash<QMovie*, QUrl> urls;
};


#endif // MYEXTEXTEDIT_H
