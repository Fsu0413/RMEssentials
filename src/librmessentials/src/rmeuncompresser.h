#ifndef RMEUNCOMPRESSER_H__INCLUDED
#define RMEUNCOMPRESSER_H__INCLUDED

#include "rmeglobal.h"

#include <QStringList>
#include <QThread>

class RmeUncompresserPrivate;

class LIBRMESSENTIALS_EXPORT RmeUncompresser : public QThread
{
    Q_OBJECT

public:
    explicit RmeUncompresser(QObject *parent = nullptr);
    ~RmeUncompresser() override;

    void addFile(const QString &zipName, const QString &fileName);

    void run() override;

signals:
    void signalFileFinished(const QString &filename);

private:
    Q_DISABLE_COPY(RmeUncompresser)
    Q_DECLARE_PRIVATE(RmeUncompresser)
    RmeUncompresserPrivate *d_ptr;
};

#endif
