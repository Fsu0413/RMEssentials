#ifndef __UNCOMPRESSER_H__
#define __UNCOMPRESSER_H__

#include <QThread>
#include <QStringList>

class Uncompresser : public QThread
{
    Q_OBJECT

public:
    virtual void run();

    QStringList zipNames;
    QStringList fileNames;

signals:
    void signal_file_finished(const QString &filename);
};

#endif