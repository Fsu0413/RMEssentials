
#include "rmeuncompresser.h"

#ifdef RME_USE_QUAZIP
#include "rmedownloader.h"
#include <QDebug>
#include <QDir>
#include <QMutex>
#include <quazip/QuaZipFile>

class RmeUncompresserPrivate
{
public:
    QMutex m;
    QStringList zipNames;
    QStringList fileNames;
    QStringList extractedFileNames;
};

RmeUncompresser::RmeUncompresser(QObject *parent)
    : QThread(parent)
    , d_ptr(new RmeUncompresserPrivate)
{
}

RmeUncompresser::~RmeUncompresser()
{
    Q_D(RmeUncompresser);
    if (d->m.tryLock(0))
        d->m.unlock();
    else
        qWarning() << "destroying RmeUncompresser when holding the mutex";

    delete d;
}

void RmeUncompresser::addFile(const QString &zipName, const QString &fileName)
{
    addFile(zipName, fileName, fileName);
}

void RmeUncompresser::addFile(const QString &zipName, const QString &fileName, const QString &extractedFileName)
{
    Q_D(RmeUncompresser);
    QMutexLocker locker(&d->m);
    Q_UNUSED(locker);
    d->zipNames << zipName;
    d->fileNames << fileName;
    d->extractedFileNames << extractedFileName;
}

void RmeUncompresser::run()
{
    Q_D(RmeUncompresser);
    d->m.lock();
    if ((d->zipNames.length() != d->fileNames.length()) || (d->zipNames.length() != d->extractedFileNames.length())) {
        qWarning() << "the length of these lists does not match";
        d->zipNames.clear();
        d->fileNames.clear();
        d->extractedFileNames.clear();
        d->m.unlock();
        return;
    }
    while (!d->zipNames.isEmpty()) {
        QString zipName = d->zipNames.takeFirst();
        QString fileName = d->fileNames.takeFirst();
        QString extractedFileName = d->extractedFileNames.takeFirst();
        d->m.unlock();

        QuaZipFile f(zipName, fileName);
        if (f.open(QIODevice::ReadOnly)) {
            QDir dir(QFileInfo(zipName).absolutePath());
            QFile output(dir.absoluteFilePath(extractedFileName));
            output.open(QIODevice::WriteOnly);
            output.write(f.readAll());
            output.close();
            f.close();
            emit signalFileFinished(extractedFileName);
        }
        d->m.lock();
    }
    d->m.unlock();
}

#else

RmeUncompresser::RmeUncompresser(QObject *parent)
    : QThread(parent)
    , d_ptr(nullptr)
{
    Q_UNIMPLEMENTED();
}

RmeUncompresser::~RmeUncompresser()
{
}

void RmeUncompresser::addFile(const QString &, const QString &)
{
    Q_UNIMPLEMENTED();
}

void RmeUncompresser::run()
{
    Q_UNIMPLEMENTED();
}

#endif
