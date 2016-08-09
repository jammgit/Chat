#include "myextextedit.h"


MyExTextEdit::MyExTextEdit(QWidget *parent)
    : QTextEdit(parent)
{

}

void MyExTextEdit::AddAnimation(const QUrl& url, const QString& fileName)
{
    QFile *file =new QFile(fileName);
    if(!file->open(QIODevice::ReadOnly)){
        qDebug()<<" open err";
    }

    /* 同一个gif 使用同一个movie */
     if(m_lstUrl.contains(url)){
         return;
     }else{
        m_lstUrl.append(url);
     }

    QMovie* movie = new QMovie(this);
    movie->setFileName(fileName);
    movie->setCacheMode(QMovie::CacheNone);

    m_lstMovie.append(movie);
    m_urls.insert(movie, url);

    /* 换帧时刷新 */
    connect(movie, SIGNAL(frameChanged(int)), this, SLOT(slot_animate(int)));
    movie->start();
    file->close();
    delete file;
}

void MyExTextEdit::slot_animate(int a)
{
    a = !a;
    if (QMovie* movie = qobject_cast<QMovie*>(sender()))
    {/* 替换图片为当前帧 */
        document()->addResource(QTextDocument::ImageResource,
                                m_urls.value(movie), movie->currentPixmap());
        /* 刷新显示 */
        setLineWrapColumnOrWidth(lineWrapColumnOrWidth());
    }
}

void MyExTextEdit::contextMenuEvent(QContextMenuEvent *)
{
}


