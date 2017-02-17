#include "libunrarplugin.h"

#include <QDir>
#include <QJsonObject>
#include <QLibrary>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

enum
{
    ERAR_SUCCESS = 0,
    ERAR_END_ARCHIVE = 10,
    ERAR_NO_MEMORY,
    ERAR_BAD_DATA,
    ERAR_BAD_ARCHIVE,
    ERAR_UNKNOWN_FORMAT,
    ERAR_EOPEN,
    ERAR_ECREATE,
    ERAR_ECLOSE,
    ERAR_EREAD,
    ERAR_EWRITE,
    ERAR_SMALL_BUF,
    ERAR_UNKNOWN,
    ERAR_MISSING_PASSWORD,
    ERAR_EREFERENCE,
    ERAR_BAD_PASSWORD
};

enum
{
    RAR_OM_LIST = 0,
    RAR_OM_EXTRACT,
    RAR_OM_LIST_INCSPLIT
};

enum
{
    RAR_SKIP = 0,
    RAR_TEST,
    RAR_EXTRACT
};

#ifndef Q_OS_WIN
#define CALLBACK
#define PASCAL
typedef long LONG;
typedef void *HANDLE;
typedef long LPARAM;
typedef unsigned int UINT;
#endif

#pragma pack(push)
#pragma pack(1)

struct RARHeaderData
{
    char ArcName[260];
    char FileName[260];
    unsigned int Flags;
    unsigned int PackSize;
    unsigned int UnpSize;
    unsigned int HostOS;
    unsigned int FileCRC;
    unsigned int FileTime;
    unsigned int UnpVer;
    unsigned int Method;
    unsigned int FileAttr;
    char *CmtBuf;
    unsigned int CmtBufSize;
    unsigned int CmtSize;
    unsigned int CmtState;
};

struct RAROpenArchiveData
{
    char *ArcName;
    unsigned int OpenMode;
    unsigned int OpenResult;
    char *CmtBuf;
    unsigned int CmtBufSize;
    unsigned int CmtSize;
    unsigned int CmtState;
};

#pragma pack(pop)

typedef HANDLE(PASCAL *RAROpenArchive)(RAROpenArchiveData *ArchiveData);
typedef int(PASCAL *RARCloseArchive)(HANDLE hArcData);
typedef int(PASCAL *RARReadHeader)(HANDLE hArcData, RARHeaderData *HeaderData);
typedef int(PASCAL *RARProcessFile)(HANDLE hArcData, int Operation, char *DestPath, char *DestName);
typedef void(PASCAL *RARSetPassword)(HANDLE hArcData, char *Password);
typedef int(PASCAL *RARGetDllVersion)();

RmeUncompLibUnrarPlugin::RmeUncompLibUnrarPlugin(QObject *parent)
    : QObject(parent)
    , m_libunrar(nullptr)
{
}

bool RmeUncompLibUnrarPlugin::isReadyForUse() const
{
    if (const_cast<RmeUncompLibUnrarPlugin *>(this)->loadLibUnrar())
        return true;

    return false;
}

RmeUncompresserFormats RmeUncompLibUnrarPlugin::supportedFormats() const
{
    if (const_cast<RmeUncompLibUnrarPlugin *>(this)->loadLibUnrar())
        return RmeUncompRar;

    return RmeUncompresserFormats();
}

RmeUncompresserResult RmeUncompLibUnrarPlugin::openArchive(const QString &fileName)
{
    if (!loadLibUnrar())
        return RmeUncompNotReadyForUse;

    RAROpenArchive rarOpenArchive = reinterpret_cast<RAROpenArchive>(m_libunrar->resolve("RAROpenArchive"));
    if (rarOpenArchive == nullptr)
        return RmeUncompUnknownError;
    RARCloseArchive rarCloseArchive = reinterpret_cast<RARCloseArchive>(m_libunrar->resolve("RARCloseArchive"));
    if (rarCloseArchive == nullptr)
        return RmeUncompUnknownError;

    RAROpenArchiveData data;
    memset(&data, 0, sizeof(RAROpenArchiveData));
    data.ArcName = qstrdup(fileName.toLocal8Bit().constData());
    data.OpenMode = RAR_OM_LIST;
    HANDLE handle = (*rarOpenArchive)(&data);

    switch (data.OpenResult) {
    case ERAR_SUCCESS:
        break;
    case ERAR_BAD_DATA:
    case ERAR_BAD_ARCHIVE:
        delete[] data.ArcName;
        return RmeUncompUnknownFileFormat;
    case ERAR_EOPEN:
        delete[] data.ArcName;
        return RmeUncompFileNotExist;
    default:
        delete[] data.ArcName;
        return RmeUncompUnknownError;
    }

    rarCloseArchive(handle);
    delete[] data.ArcName;

    m_fileName = fileName;
    return RmeUncompSuccess;
}

QStringList RmeUncompLibUnrarPlugin::listFiles()
{
    if (!loadLibUnrar())
        return QStringList();

    RAROpenArchive rarOpenArchive = reinterpret_cast<RAROpenArchive>(m_libunrar->resolve("RAROpenArchive"));
    if (rarOpenArchive == nullptr)
        return QStringList();
    RARCloseArchive rarCloseArchive = reinterpret_cast<RARCloseArchive>(m_libunrar->resolve("RARCloseArchive"));
    if (rarCloseArchive == nullptr)
        return QStringList();
    RARSetPassword rarSetPassword = reinterpret_cast<RARSetPassword>(m_libunrar->resolve("RARSetPassword"));
    if (rarSetPassword == nullptr)
        return QStringList();
    RARReadHeader rarReadHeader = reinterpret_cast<RARReadHeader>(m_libunrar->resolve("RARReadHeader"));
    if (rarReadHeader == nullptr)
        return QStringList();
    RARProcessFile rarProcessFile = reinterpret_cast<RARProcessFile>(m_libunrar->resolve("RARProcessFile"));
    if (rarProcessFile == nullptr)
        return QStringList();

    RAROpenArchiveData data;
    memset(&data, 0, sizeof(RAROpenArchiveData));
    data.ArcName = qstrdup(m_fileName.toLocal8Bit().constData());
    data.OpenMode = RAR_OM_LIST;
    HANDLE handle = (*rarOpenArchive)(&data);

    switch (data.OpenResult) {
    case ERAR_SUCCESS:
        break;
    default:
        delete[] data.ArcName;
        return QStringList();
    }
    char *pass = qstrdup(password().toLocal8Bit().constData());
    (*rarSetPassword)(handle, pass);

    RARHeaderData headerData;
    memset(&headerData, 0, sizeof(RARHeaderData));
    int result;
    QStringList l;
    while ((result = (*rarReadHeader)(handle, &headerData)) == ERAR_SUCCESS) {
        l << QString::fromLocal8Bit(headerData.FileName);
        int result2 = (*rarProcessFile)(handle, RAR_SKIP, nullptr, nullptr);
        if (result2 != ERAR_SUCCESS) {
            rarCloseArchive(handle);
            delete[] data.ArcName;
            delete[] pass;
            return QStringList();
        }
    }
    (*rarCloseArchive)(handle);
    delete[] data.ArcName;
    delete[] pass;
    if (result != ERAR_END_ARCHIVE)
        return QStringList();

    return l;
}

RmeUncompresserResult RmeUncompLibUnrarPlugin::uncompressAllFiles(const QDir &targetDir)
{
    if (!loadLibUnrar())
        return RmeUncompNotReadyForUse;

    RAROpenArchive rarOpenArchive = reinterpret_cast<RAROpenArchive>(m_libunrar->resolve("RAROpenArchive"));
    if (rarOpenArchive == nullptr)
        return RmeUncompUnknownError;
    RARCloseArchive rarCloseArchive = reinterpret_cast<RARCloseArchive>(m_libunrar->resolve("RARCloseArchive"));
    if (rarCloseArchive == nullptr)
        return RmeUncompUnknownError;
    RARSetPassword rarSetPassword = reinterpret_cast<RARSetPassword>(m_libunrar->resolve("RARSetPassword"));
    if (rarSetPassword == nullptr)
        return RmeUncompUnknownError;
    RARReadHeader rarReadHeader = reinterpret_cast<RARReadHeader>(m_libunrar->resolve("RARReadHeader"));
    if (rarReadHeader == nullptr)
        return RmeUncompUnknownError;
    RARProcessFile rarProcessFile = reinterpret_cast<RARProcessFile>(m_libunrar->resolve("RARProcessFile"));
    if (rarProcessFile == nullptr)
        return RmeUncompUnknownError;

    RAROpenArchiveData data;
    memset(&data, 0, sizeof(RAROpenArchiveData));
    data.ArcName = qstrdup(m_fileName.toLocal8Bit().constData());
    data.OpenMode = RAR_OM_EXTRACT;
    HANDLE handle = (*rarOpenArchive)(&data);

    switch (data.OpenResult) {
    case ERAR_SUCCESS:
        break;
    case ERAR_BAD_DATA:
    case ERAR_BAD_ARCHIVE:
        delete[] data.ArcName;
        return RmeUncompUnknownFileFormat;
    case ERAR_EOPEN:
        delete[] data.ArcName;
        return RmeUncompFileNotExist;
    default:
        delete[] data.ArcName;
        return RmeUncompUnknownError;
    }
    char *pass = qstrdup(password().toLocal8Bit().constData());
    (*rarSetPassword)(handle, pass);

    RARHeaderData headerData;
    memset(&headerData, 0, sizeof(RARHeaderData));
    int result;
    char *destpath = qstrdup(QDir::toNativeSeparators(targetDir.absolutePath()).toLocal8Bit().constData());

    while ((result = (*rarReadHeader)(handle, &headerData)) == ERAR_SUCCESS) {
        int result2 = (*rarProcessFile)(handle, RAR_EXTRACT, nullptr, destpath);
        if (result2 != ERAR_SUCCESS) {
            rarCloseArchive(handle);
            delete[] data.ArcName;
            delete[] pass;
            delete[] destpath;
            switch (result2) {
            case ERAR_BAD_DATA:
                return RmeUncompChecksumError;
            case ERAR_BAD_ARCHIVE:
            case ERAR_UNKNOWN_FORMAT:
                return RmeUncompUnknownFileFormat;
            default:
                return RmeUncompUnknownError;
            }
        }
    }
    (*rarCloseArchive)(handle);
    delete[] data.ArcName;
    delete[] pass;
    delete[] destpath;
    if (result != ERAR_END_ARCHIVE)
        return RmeUncompUnknownError;

    return RmeUncompSuccess;
}

RmeUncompresserResult RmeUncompLibUnrarPlugin::uncompressOneFile(const QDir &targetDir, const QString &fileName)
{
    if (!loadLibUnrar())
        return RmeUncompNotReadyForUse;

    RAROpenArchive rarOpenArchive = reinterpret_cast<RAROpenArchive>(m_libunrar->resolve("RAROpenArchive"));
    if (rarOpenArchive == nullptr)
        return RmeUncompUnknownError;
    RARCloseArchive rarCloseArchive = reinterpret_cast<RARCloseArchive>(m_libunrar->resolve("RARCloseArchive"));
    if (rarCloseArchive == nullptr)
        return RmeUncompUnknownError;
    RARSetPassword rarSetPassword = reinterpret_cast<RARSetPassword>(m_libunrar->resolve("RARSetPassword"));
    if (rarSetPassword == nullptr)
        return RmeUncompUnknownError;
    RARReadHeader rarReadHeader = reinterpret_cast<RARReadHeader>(m_libunrar->resolve("RARReadHeader"));
    if (rarReadHeader == nullptr)
        return RmeUncompUnknownError;
    RARProcessFile rarProcessFile = reinterpret_cast<RARProcessFile>(m_libunrar->resolve("RARProcessFile"));
    if (rarProcessFile == nullptr)
        return RmeUncompUnknownError;

    RAROpenArchiveData data;
    memset(&data, 0, sizeof(RAROpenArchiveData));
    data.ArcName = qstrdup(m_fileName.toLocal8Bit().constData());
    data.OpenMode = RAR_OM_EXTRACT;
    HANDLE handle = (*rarOpenArchive)(&data);

    switch (data.OpenResult) {
    case ERAR_SUCCESS:
        break;
    case ERAR_BAD_DATA:
    case ERAR_BAD_ARCHIVE:
        delete[] data.ArcName;
        return RmeUncompUnknownFileFormat;
    case ERAR_EOPEN:
        delete[] data.ArcName;
        return RmeUncompFileNotExist;
    default:
        delete[] data.ArcName;
        return RmeUncompUnknownError;
    }
    char *pass = qstrdup(password().toLocal8Bit().constData());
    (*rarSetPassword)(handle, pass);

    RARHeaderData headerData;
    memset(&headerData, 0, sizeof(RARHeaderData));
    int result;
    char *destpath = qstrdup(QDir::toNativeSeparators(targetDir.absoluteFilePath(fileName)).toLocal8Bit().constData());
    bool extracted = false;
    while ((result = (*rarReadHeader)(handle, &headerData)) == ERAR_SUCCESS) {
        int result2 = ERAR_SUCCESS;
        if (QDir::fromNativeSeparators(QString::fromLocal8Bit(headerData.FileName)) == fileName) {
            result2 = (*rarProcessFile)(handle, RAR_EXTRACT, destpath, nullptr);
            extracted = true;
        } else
            result2 = (*rarProcessFile)(handle, RAR_SKIP, nullptr, nullptr);
        if (result2 != ERAR_SUCCESS) {
            rarCloseArchive(handle);
            delete[] data.ArcName;
            delete[] pass;
            delete[] destpath;
            switch (result2) {
            case ERAR_BAD_DATA:
                return RmeUncompChecksumError;
            case ERAR_BAD_ARCHIVE:
            case ERAR_UNKNOWN_FORMAT:
                return RmeUncompUnknownFileFormat;
            default:
                return RmeUncompUnknownError;
            }
        } else if (extracted)
            break;
    }
    (*rarCloseArchive)(handle);
    delete[] data.ArcName;
    delete[] pass;
    delete[] destpath;
    if (result == ERAR_END_ARCHIVE)
        return RmeUncompFileNotExist;
    else if (result != ERAR_SUCCESS)
        return RmeUncompUnknownError;

    return RmeUncompSuccess;
}

void RmeUncompLibUnrarPlugin::setPassword(const QString &password)
{
    m_password = password;
}

QJsonObject RmeUncompLibUnrarPlugin::pluginSettings() const
{
    QJsonObject ob;
    ob[QStringLiteral("libraryPath")] = libraryPath();
    return ob;
}

bool RmeUncompLibUnrarPlugin::setPluginSetting(const QString &key, const QJsonValue &value)
{
    if (key == QStringLiteral("libraryPath") && value.isString()) {
        m_libraryPath = value.toString();
        return true;
    }
    return false;
}

bool RmeUncompLibUnrarPlugin::setPluginSettings(const QJsonObject &json)
{
    bool result = true;
    foreach (const QString &key, json.keys())
        result &= setPluginSetting(key, json.value(key));
    return result;
}

bool RmeUncompLibUnrarPlugin::loadLibUnrar()
{
    if (m_libunrar != nullptr)
        return true;

    m_libunrar = new QLibrary(libraryPath(), this);
    m_libunrar->setLoadHints(QLibrary::ResolveAllSymbolsHint);
    if (!m_libunrar->load()) {
        delete m_libunrar;
        return false;
    }

    RARGetDllVersion rarGetDllVersion = reinterpret_cast<RARGetDllVersion>(m_libunrar->resolve("RARGetDllVersion"));
    if (rarGetDllVersion == nullptr) {
        m_libunrar->unload();
        delete m_libunrar;
        return false;
    }

    int dllversion = (*rarGetDllVersion)();
    if (dllversion < 7) {
        m_libunrar->unload();
        delete m_libunrar;
        return false;
    }

    return true;
}

QString RmeUncompLibUnrarPlugin::password() const
{
    return m_password.isEmpty() ? QStringLiteral("-") : m_password;
}

QString RmeUncompLibUnrarPlugin::libraryPath() const
{
    return m_libraryPath.isEmpty() ? QStringLiteral("unrar") : m_libraryPath;
}
