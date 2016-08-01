#ifndef TRANSFERFILE_H
#define TRANSFERFILE_H

#include <QObject>
#include <QFile>


class TransferFile : public QObject
{
    Q_OBJECT
public:
    explicit TransferFile(QObject *parent = 0);

signals:

public slots:

};

#endif // TRANSFERFILE_H
