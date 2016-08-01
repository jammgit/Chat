#include "myextextedit.h"


MyExTextEdit::MyExTextEdit(QWidget *parent)
    : QTextEdit(parent)
{

}

void MyExTextEdit::AddAnimation(const QUrl& url, const QString& fileName)
{
    QFile *file =new QFile(fileName);  //读取gif文件
    if(!file->open(QIODevice::ReadOnly)){
        qDebug()<<" open err";
    }

     if(lstUrl.contains(url)){ //同一个gif 使用同一个movie
         return;
     }else{
        lstUrl.append(url);
     }

    QMovie* movie = new QMovie(this);
    movie->setFileName(fileName);
    movie->setCacheMode(QMovie::CacheNone);

    lstMovie.append(movie);   //获取该movie,以便删除
    urls.insert(movie, url);   //urls 一个hash

    //换帧时刷新
    connect(movie, SIGNAL(frameChanged(int)), this, SLOT(slot_animate(int)));
    movie->start();
    file->close();
    delete file;
}

void MyExTextEdit::slot_animate(int a)
{
    a = !a;
    if (QMovie* movie = qobject_cast<QMovie*>(sender()))
    {
        document()->addResource(QTextDocument::ImageResource,   //替换图片为当前帧
                                urls.value(movie), movie->currentPixmap());

        setLineWrapColumnOrWidth(lineWrapColumnOrWidth()); // ..刷新显示
    }
}


