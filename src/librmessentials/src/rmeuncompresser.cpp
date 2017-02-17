
#include "rmeuncompresser.h"

#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>

namespace {

void addUncompresserLibraryPath()
{
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + QStringLiteral("/plugins/uncompressers"));
}
}

Q_COREAPP_STARTUP_FUNCTION(addUncompresserLibraryPath)

QStringList RmeUncompresser::pluginNames()
{
    QDir pluginDir(QCoreApplication::applicationDirPath() + QStringLiteral("/plugins/uncompressers"));
    QStringList r;
    foreach (const QFileInfo &info, pluginDir.entryInfoList()) {
        if (QLibrary::isLibrary(info.absoluteFilePath())) {
            QString name = info.completeBaseName();
#ifndef Q_OS_WIN
            if (name.startsWith(QStringLiteral("lib")))
                name = name.mid(3);
#endif
            if (QLibrary::isLibrary(name))
                r << name;
        }
    }
    return r;
}

RmeUncompresserPlugin *RmeUncompresser::plugin(const QString &key)
{
    QPluginLoader loader(key);
    return qobject_cast<RmeUncompresserPlugin *>(loader.instance());
}

//#ifdef RME_USE_QUAZIP
//#include "rmedownloader.h"
//#include <QDebug>
//#include <QDir>
//#include <QMutex>
//#include <quazip/QuaZipFile>

//class RmeUncompresserPrivate
//{
//public:
//    QMutex m;
//    QStringList zipNames;
//    QStringList fileNames;
//};

//RmeUncompresser::RmeUncompresser(QObject *parent)
//    : QThread(parent)
//    , d_ptr(new RmeUncompresserPrivate)
//{
//}

//RmeUncompresser::~RmeUncompresser()
//{
//    Q_D(RmeUncompresser);
//    if (d->m.tryLock(0))
//        d->m.unlock();
//    else
//        qWarning() << "destroying RmeUncompresser when holding the mutex";

//    delete d;
//}

//void RmeUncompresser::addFile(const QString &zipName, const QString &fileName)
//{
//    Q_D(RmeUncompresser);
//    QMutexLocker locker(&d->m);
//    Q_UNUSED(locker);
//    d->zipNames << zipName;
//    d->fileNames << fileName;
//}

//void RmeUncompresser::run()
//{
//    Q_D(RmeUncompresser);
//    d->m.lock();
//    if (d->zipNames.length() != d->fileNames.length()) {
//        qWarning() << "the length of these lists does not match";
//        d->zipNames.clear();
//        d->fileNames.clear();
//        d->m.unlock();
//        return;
//    }
//    while (!d->zipNames.isEmpty()) {
//        QString zipName = d->zipNames.takeFirst();
//        QString fileName = d->fileNames.takeFirst();
//        d->m.unlock();

//        QuaZipFile f(zipName, fileName);
//        if (f.open(QIODevice::ReadOnly)) {
//            QDir dir(QFileInfo(zipName).absolutePath());
//            QFile output(dir.absoluteFilePath(fileName));
//            output.open(QIODevice::WriteOnly);
//            output.write(f.readAll());
//            output.close();
//            f.close();
//            emit signalFileFinished(fileName);
//        }
//        d->m.lock();
//    }
//    d->m.unlock();
//}

//#else

//RmeUncompresser::RmeUncompresser(QObject *parent)
//    : QThread(parent)
//    , d_ptr(nullptr)
//{
//    Q_UNIMPLEMENTED();
//}

//RmeUncompresser::~RmeUncompresser()
//{
//}

//void RmeUncompresser::addFile(const QString &, const QString &)
//{
//    Q_UNIMPLEMENTED();
//}

//void RmeUncompresser::run()
//{
//    Q_UNIMPLEMENTED();
//}

//#endif

RmeUncompresserPlugin::RmeUncompresserPlugin()
{
}

RmeUncompresserPlugin::~RmeUncompresserPlugin()
{
}
