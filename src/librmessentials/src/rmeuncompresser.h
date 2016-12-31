#ifndef __UNCOMPRESSER_H__
#define __UNCOMPRESSER_H__

#include "rmeglobal.h"

#include <QStringList>
#include <QThread>

class LIBRMESSENTIALS_EXPORT RmeUncompresser : public QThread
{
    Q_OBJECT

public:
    void run() override;

    QStringList zipNames;
    QStringList fileNames;

signals:
    void signalFileFinished(const QString &filename);
};

#endif
