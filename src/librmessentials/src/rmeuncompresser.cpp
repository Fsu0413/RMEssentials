
#include <QtGlobal>

// QtConcurrent is not -Wzero-as-null-pointer-constant clean yet
QT_WARNING_DISABLE_GCC("-Wzero-as-null-pointer-constant")

#include "rmeuncompresser.h"

#include <QCoreApplication>
#include <QDir>
#include <QFuture>
#include <QFutureWatcher>
#include <QMetaProperty>
#include <QPluginLoader>
#include <QThread>
#include <QtConcurrent>

namespace {
void addUncompresserLibraryPath()
{
    QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + QStringLiteral("/plugins/uncompressers"));
}

void registerMetaType()
{
    qRegisterMetaType<RmeUncompresserResult>();
    qRegisterMetaType<QDir>();
}
}

Q_COREAPP_STARTUP_FUNCTION(addUncompresserLibraryPath)
Q_COREAPP_STARTUP_FUNCTION(registerMetaType)

class RmeUncompresserPrivate
    : public QObject
{
    Q_OBJECT

public:
    RmeUncompresserPrivate(RmeUncompresser *uncompresser)
        : QObject(uncompresser)
        , q(uncompresser)
        , m_currentUsingPlugin(nullptr)
    {
    }

    RmeUncompresser *q;

    QString m_password;
    QMap<RmeUncompresserFormat, RmeUncompresserPlugin *> m_pluginMap;
    RmeUncompresserPlugin *m_currentUsingPlugin;

    static RmeUncompresserFormat guessFileFormat(const QString fileName)
    {
        QFileInfo info(fileName);
        if (info.suffix() == QStringLiteral("7z"))
            return RmeUncomp7z;
        else if (info.suffix() == QStringLiteral("rar") || info.suffix() == QStringLiteral("rar5"))
            return RmeUncompRar;
        else if (info.suffix() == QStringLiteral("zip"))
            return RmeUncompZip;
        else
            return RmeUncompUnknownFormat;
    }

public slots:
    void uncompressAllFilesComplete()
    {
        // qobject_cast will cause nullptr since QFutureWatcher does not use Q_OBJECT Macro
        auto resultWatcher = dynamic_cast<QFutureWatcher<RmeUncompresserResult> *>(sender());
        if (resultWatcher == nullptr) // wtf?
            return;

        emit q->uncompressAllFilesCompleted(resultWatcher->result());

        resultWatcher->deleteLater();
    }

    void uncompressOneFileComplete()
    {
        auto resultWatcher = dynamic_cast<QFutureWatcher<RmeUncompresserResult> *>(sender());
        if (resultWatcher == nullptr)
            return;

        emit q->uncompressOneFileCompleted(resultWatcher->result());

        resultWatcher->deleteLater();
    }
};

RmeUncompresser::RmeUncompresser()
    : d_ptr(new RmeUncompresserPrivate(this))
{
}

RmeUncompresser::~RmeUncompresser()
{
}

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

bool RmeUncompresser::setPlugin(RmeUncompresserFormat format, const QString &pluginName)
{
    RmeUncompresserPlugin *p = plugin(pluginName);
    if (p != nullptr && (p->supportedFormats() & format) && p->isReadyForUse()) {
        Q_D(RmeUncompresser);
        d->m_pluginMap[format] = p;
        return true;
    }
    return false;
}

RmeUncompresserResult RmeUncompresser::openArchive(const QString &fileName)
{
    Q_D(RmeUncompresser);

    RmeUncompresserFormat format = d->guessFileFormat(fileName);
    if (format == RmeUncompUnknownFormat)
        return RmeUncompUnknownFileFormat;

    RmeUncompresserPlugin *p = d->m_pluginMap.value(format, nullptr);
    if (p == nullptr) {
        foreach (const QString &pn, pluginNames()) {
            if (setPlugin(format, pn)) {
                p = d->m_pluginMap.value(format, nullptr);
                break;
            }
        }
    }
    if (p == nullptr)
        return RmeUncompNoAvailablePlugin;

    d->m_currentUsingPlugin = p;

    return p->openArchive(fileName);
}

QStringList RmeUncompresser::listFiles()
{
    Q_D(RmeUncompresser);
    if (d->m_currentUsingPlugin == nullptr)
        return QStringList();

    return d->m_currentUsingPlugin->listFiles();
}

bool RmeUncompresser::uncompressAllFiles(const QDir &targetDir)
{
    Q_D(RmeUncompresser);
    if (d->m_currentUsingPlugin == nullptr)
        return false;

    QFutureWatcher<RmeUncompresserResult> *resultWatcher = new QFutureWatcher<RmeUncompresserResult>();
    connect(resultWatcher, &QFutureWatcher<RmeUncompresserResult>::finished, d, &RmeUncompresserPrivate::uncompressAllFilesComplete);
    QFuture<RmeUncompresserResult> result = QtConcurrent::run([d, targetDir]() { return d->m_currentUsingPlugin->uncompressAllFiles(targetDir); });
    resultWatcher->setFuture(result);

    return true;
}

bool RmeUncompresser::uncompressOneFile(const QDir &targetDir, const QString &fileName)
{
    Q_D(RmeUncompresser);
    if (d->m_currentUsingPlugin == nullptr)
        return false;

    QFutureWatcher<RmeUncompresserResult> *resultWatcher = new QFutureWatcher<RmeUncompresserResult>();
    connect(resultWatcher, &QFutureWatcher<RmeUncompresserResult>::finished, d, &RmeUncompresserPrivate::uncompressOneFileComplete);
    QFuture<RmeUncompresserResult> result = QtConcurrent::run([d, targetDir, fileName]() { return d->m_currentUsingPlugin->uncompressOneFile(targetDir, fileName); });
    resultWatcher->setFuture(result);

    return true;
}

void RmeUncompresser::setPassword(const QString &password)
{
    Q_D(RmeUncompresser);
    d->m_password = password;
}

QString RmeUncompresser::password() const
{
    Q_D(const RmeUncompresser);
    return d->m_password;
}

class RmeUncompresserPluginPrivate
{
public:
    RmeUncompresserPluginPrivate()
        : m_uncompresser(nullptr)
    {
    }

    RmeUncompresser *m_uncompresser;

private:
    Q_DISABLE_COPY(RmeUncompresserPluginPrivate)
};

RmeUncompresserPlugin::RmeUncompresserPlugin(QObject *parent)
    : QObject(parent)
    , d_ptr(new RmeUncompresserPluginPrivate)
{
}

RmeUncompresserPlugin::~RmeUncompresserPlugin()
{
    Q_D(RmeUncompresserPlugin);
    delete d;
}

QStringList RmeUncompresserPlugin::propertyList() const
{
    QStringList r;
    for (int i = 0; i < metaObject()->propertyCount(); ++i) {
        QMetaProperty p = metaObject()->property(i);
        if (qstrcmp(p.name(), "objectName") == 0)
            continue;
        r << QString::fromUtf8(p.name());
    }
    return r;
}

void RmeUncompresserPlugin::setUncompresser(RmeUncompresser *uncompresser)
{
    Q_D(RmeUncompresserPlugin);
    d->m_uncompresser = uncompresser;
}

QString RmeUncompresserPlugin::password() const
{
    QString p;
    Q_D(const RmeUncompresserPlugin);
    if (d->m_uncompresser != nullptr)
        p = d->m_uncompresser->password();

    if (p.isEmpty() || p.isNull())
        p = QStringLiteral("-");
    return p;
}

#include "rmeuncompresser.moc"
