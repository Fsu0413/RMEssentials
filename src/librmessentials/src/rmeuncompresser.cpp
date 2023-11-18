
#include "rmeuncompresser.h"
#include <QDebug>
#include <QDir>
#include <QMutex>
#include <tuple>

#ifdef RME_USE_QUAZIP
#include <quazip/QuaZipFile>
#else
// Although QuaZip seems to be a more reliable solution since it is public API
// But since user have requested not to use QuaZip, we are using mechanisms from Qt GuiPrivate for uncompressing files
// I didn't expect it will work but let's try!
#include <private/qzipreader_p.h>
#endif

class RmeUncompresserPrivate
{
public:
    QMutex m;
    QList<std::tuple<QString /* zipName */, QString /* fileName */, QString /*extractedFileName */> > names;
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

void RmeUncompresser::addFile(const QString &zipName, const QString &fileName, const QString &extractedFileName)
{
    Q_D(RmeUncompresser);
    QMutexLocker locker(&d->m);
    Q_UNUSED(locker);
    d->names << std::make_tuple(zipName, fileName, (extractedFileName.isEmpty() ? fileName : extractedFileName));
}

void RmeUncompresser::run()
{
    Q_D(RmeUncompresser);
    d->m.lock();
    while (!d->names.isEmpty()) {
        auto [/* QString */ zipName, /* QString */ fileName, /* QString */ extractedFileName] = d->names.takeFirst();
        d->m.unlock();

#ifdef RME_USE_QUAZIP
        QuaZipFile f(zipName, fileName);
        if (f.open(QIODevice::ReadOnly)) {
#else
        QZipReader reader(zipName);
        if (reader.status() == QZipReader::NoError) {
#endif
            QDir dir(QFileInfo(zipName).absolutePath());
            QFile output(dir.absoluteFilePath(extractedFileName));
            output.open(QIODevice::WriteOnly);
#ifdef RME_USE_QUAZIP
            output.write(f.readAll());
            f.close();
#else
            output.write(reader.fileData(fileName));
#endif
            output.close();
            emit signalFileFinished(extractedFileName);
        }
        d->m.lock();
    }
    d->m.unlock();
}
