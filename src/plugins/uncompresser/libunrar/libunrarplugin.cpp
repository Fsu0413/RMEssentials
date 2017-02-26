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
#ifndef RARDLL
typedef HANDLE(PASCAL *rarOpenArchive)(RAROpenArchiveData *ArchiveData);
typedef int(PASCAL *rarCloseArchive)(HANDLE hArcData);
typedef int(PASCAL *rarReadHeader)(HANDLE hArcData, RARHeaderData *HeaderData);
typedef int(PASCAL *rarProcessFile)(HANDLE hArcData, int Operation, char *DestPath, char *DestName);
typedef void(PASCAL *rarSetPassword)(HANDLE hArcData, char *Password);
typedef int(PASCAL *rarGetDllVersion)();
#else
extern "C" {
#include "src/version.hpp"
HANDLE PASCAL RAROpenArchive(struct RAROpenArchiveData *ArchiveData);
int PASCAL RARCloseArchive(HANDLE hArcData);
int PASCAL RARReadHeader(HANDLE hArcData, struct RARHeaderData *HeaderData);
int PASCAL RARProcessFile(HANDLE hArcData, int Operation, char *DestPath, char *DestName);
void PASCAL RARSetPassword(HANDLE hArcData, char *Password);
int PASCAL RARGetDllVersion();
}
#endif

RmeUncompLibUnrarPlugin::RmeUncompLibUnrarPlugin(QObject *parent)
    : RmeUncompresserPlugin(parent)
#ifndef RARDLL
    , m_libunrar(nullptr)
#endif
{
}

bool RmeUncompLibUnrarPlugin::isReadyForUse() const
{
#ifndef RARDLL
    if (const_cast<RmeUncompLibUnrarPlugin *>(this)->loadLibUnrar())
        return true;

    return false;
#else
    return true;
#endif
}

RmeUncompresserFormats RmeUncompLibUnrarPlugin::supportedFormats() const
{
#ifndef RARDLL
    if (const_cast<RmeUncompLibUnrarPlugin *>(this)->loadLibUnrar())
        return RmeUncompRar;

    return RmeUncompresserFormats();
#else
    return RmeUncompRar;
#endif
}

RmeUncompresserResult RmeUncompLibUnrarPlugin::openArchive(const QString &fileName)
{
#ifndef RARDLL
    if (!loadLibUnrar())
        return RmeUncompNotReadyForUse;

    rarOpenArchive RAROpenArchive = reinterpret_cast<rarOpenArchive>(m_libunrar->resolve("RAROpenArchive"));
    if (RAROpenArchive == nullptr)
        return RmeUncompUnknownError;
    rarCloseArchive RARCloseArchive = reinterpret_cast<rarCloseArchive>(m_libunrar->resolve("RARCloseArchive"));
    if (RARCloseArchive == nullptr)
        return RmeUncompUnknownError;
#endif

    RAROpenArchiveData data;
    memset(&data, 0, sizeof(RAROpenArchiveData));
    data.ArcName = qstrdup(fileName.toLocal8Bit().constData());
    data.OpenMode = RAR_OM_LIST;
    HANDLE handle = RAROpenArchive(&data);

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

    RARCloseArchive(handle);
    delete[] data.ArcName;

    m_fileName = fileName;
    return RmeUncompSuccess;
}

QStringList RmeUncompLibUnrarPlugin::listFiles()
{
#ifndef RARDLL
    if (!loadLibUnrar())
        return QStringList();

    rarOpenArchive RAROpenArchive = reinterpret_cast<rarOpenArchive>(m_libunrar->resolve("RAROpenArchive"));
    if (RAROpenArchive == nullptr)
        return QStringList();
    rarCloseArchive RARCloseArchive = reinterpret_cast<rarCloseArchive>(m_libunrar->resolve("RARCloseArchive"));
    if (RARCloseArchive == nullptr)
        return QStringList();
    rarSetPassword RARSetPassword = reinterpret_cast<rarSetPassword>(m_libunrar->resolve("RARSetPassword"));
    if (RARSetPassword == nullptr)
        return QStringList();
    rarReadHeader RARReadHeader = reinterpret_cast<rarReadHeader>(m_libunrar->resolve("RARReadHeader"));
    if (RARReadHeader == nullptr)
        return QStringList();
    rarProcessFile RARProcessFile = reinterpret_cast<rarProcessFile>(m_libunrar->resolve("RARProcessFile"));
    if (RARProcessFile == nullptr)
        return QStringList();
#endif

    RAROpenArchiveData data;
    memset(&data, 0, sizeof(RAROpenArchiveData));
    data.ArcName = qstrdup(m_fileName.toLocal8Bit().constData());
    data.OpenMode = RAR_OM_LIST;
    HANDLE handle = RAROpenArchive(&data);

    switch (data.OpenResult) {
    case ERAR_SUCCESS:
        break;
    default:
        delete[] data.ArcName;
        return QStringList();
    }
    char *pass = qstrdup(password().toLocal8Bit().constData());
    RARSetPassword(handle, pass);

    RARHeaderData headerData;
    memset(&headerData, 0, sizeof(RARHeaderData));
    int result;
    QStringList l;
    while ((result = RARReadHeader(handle, &headerData)) == ERAR_SUCCESS) {
        l << QString::fromLocal8Bit(headerData.FileName);
        int result2 = RARProcessFile(handle, RAR_SKIP, nullptr, nullptr);
        if (result2 != ERAR_SUCCESS) {
            RARCloseArchive(handle);
            delete[] data.ArcName;
            delete[] pass;
            return QStringList();
        }
    }
    RARCloseArchive(handle);
    delete[] data.ArcName;
    delete[] pass;
    if (result != ERAR_END_ARCHIVE)
        return QStringList();

    return l;
}

RmeUncompresserResult RmeUncompLibUnrarPlugin::uncompressAllFiles(const QDir &targetDir)
{
#ifndef RARDLL
    if (!loadLibUnrar())
        return RmeUncompNotReadyForUse;

    rarOpenArchive RAROpenArchive = reinterpret_cast<rarOpenArchive>(m_libunrar->resolve("RAROpenArchive"));
    if (RAROpenArchive == nullptr)
        return RmeUncompUnknownError;
    rarCloseArchive RARCloseArchive = reinterpret_cast<rarCloseArchive>(m_libunrar->resolve("RARCloseArchive"));
    if (RARCloseArchive == nullptr)
        return RmeUncompUnknownError;
    rarSetPassword RARSetPassword = reinterpret_cast<rarSetPassword>(m_libunrar->resolve("RARSetPassword"));
    if (RARSetPassword == nullptr)
        return RmeUncompUnknownError;
    rarReadHeader RARReadHeader = reinterpret_cast<rarReadHeader>(m_libunrar->resolve("RARReadHeader"));
    if (RARReadHeader == nullptr)
        return RmeUncompUnknownError;
    rarProcessFile RARProcessFile = reinterpret_cast<rarProcessFile>(m_libunrar->resolve("RARProcessFile"));
    if (RARProcessFile == nullptr)
        return RmeUncompUnknownError;
#endif

    RAROpenArchiveData data;
    memset(&data, 0, sizeof(RAROpenArchiveData));
    data.ArcName = qstrdup(m_fileName.toLocal8Bit().constData());
    data.OpenMode = RAR_OM_EXTRACT;
    HANDLE handle = RAROpenArchive(&data);

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
    RARSetPassword(handle, pass);

    RARHeaderData headerData;
    memset(&headerData, 0, sizeof(RARHeaderData));
    int result;
    char *destpath = qstrdup(QDir::toNativeSeparators(targetDir.absolutePath()).toLocal8Bit().constData());

    while ((result = RARReadHeader(handle, &headerData)) == ERAR_SUCCESS) {
        int result2 = RARProcessFile(handle, RAR_EXTRACT, nullptr, destpath);
        if (result2 != ERAR_SUCCESS) {
            RARCloseArchive(handle);
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
    RARCloseArchive(handle);
    delete[] data.ArcName;
    delete[] pass;
    delete[] destpath;
    if (result != ERAR_END_ARCHIVE)
        return RmeUncompUnknownError;

    return RmeUncompSuccess;
}

RmeUncompresserResult RmeUncompLibUnrarPlugin::uncompressOneFile(const QDir &targetDir, const QString &fileName)
{
#ifndef RARDLL
    if (!loadLibUnrar())
        return RmeUncompNotReadyForUse;

    rarOpenArchive RAROpenArchive = reinterpret_cast<rarOpenArchive>(m_libunrar->resolve("RAROpenArchive"));
    if (RAROpenArchive == nullptr)
        return RmeUncompUnknownError;
    rarCloseArchive RARCloseArchive = reinterpret_cast<rarCloseArchive>(m_libunrar->resolve("RARCloseArchive"));
    if (RARCloseArchive == nullptr)
        return RmeUncompUnknownError;
    rarSetPassword RARSetPassword = reinterpret_cast<rarSetPassword>(m_libunrar->resolve("RARSetPassword"));
    if (RARSetPassword == nullptr)
        return RmeUncompUnknownError;
    rarReadHeader RARReadHeader = reinterpret_cast<rarReadHeader>(m_libunrar->resolve("RARReadHeader"));
    if (RARReadHeader == nullptr)
        return RmeUncompUnknownError;
    rarProcessFile RARProcessFile = reinterpret_cast<rarProcessFile>(m_libunrar->resolve("RARProcessFile"));
    if (RARProcessFile == nullptr)
        return RmeUncompUnknownError;
#endif

    RAROpenArchiveData data;
    memset(&data, 0, sizeof(RAROpenArchiveData));
    data.ArcName = qstrdup(m_fileName.toLocal8Bit().constData());
    data.OpenMode = RAR_OM_EXTRACT;
    HANDLE handle = RAROpenArchive(&data);

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
    RARSetPassword(handle, pass);

    RARHeaderData headerData;
    memset(&headerData, 0, sizeof(RARHeaderData));
    int result;
    char *destpath = qstrdup(QDir::toNativeSeparators(targetDir.absoluteFilePath(fileName)).toLocal8Bit().constData());
    bool extracted = false;
    while ((result = RARReadHeader(handle, &headerData)) == ERAR_SUCCESS) {
        int result2 = ERAR_SUCCESS;
        if (QDir::fromNativeSeparators(QString::fromLocal8Bit(headerData.FileName)) == fileName) {
            result2 = RARProcessFile(handle, RAR_EXTRACT, destpath, nullptr);
            extracted = true;
        } else
            result2 = RARProcessFile(handle, RAR_SKIP, nullptr, nullptr);
        if (result2 != ERAR_SUCCESS) {
            RARCloseArchive(handle);
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
    RARCloseArchive(handle);
    delete[] data.ArcName;
    delete[] pass;
    delete[] destpath;
    if (result == ERAR_END_ARCHIVE)
        return RmeUncompFileNotExist;
    else if (result != ERAR_SUCCESS)
        return RmeUncompUnknownError;

    return RmeUncompSuccess;
}

#ifndef RARDLL
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

    rarGetDllVersion RARGetDllVersion = reinterpret_cast<rarGetDllVersion>(m_libunrar->resolve("RARGetDllVersion"));
    if (RARGetDllVersion == nullptr) {
        m_libunrar->unload();
        delete m_libunrar;
        return false;
    }

    int dllversion = RARGetDllVersion();
    if (dllversion < 7) {
        m_libunrar->unload();
        delete m_libunrar;
        return false;
    }

    return true;
}

QString RmeUncompLibUnrarPlugin::libraryPath() const
{
    return m_libraryPath.isEmpty() ? QStringLiteral("unrar") : m_libraryPath;
}
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
QVersionNumber RmeUncompLibUnrarPlugin::versionNumber() const
{
    return QVersionNumber(RARVER_MAJOR, RARVER_MINOR, RARVER_BETA);
}
#endif

const char *RmeUncompLibUnrarPlugin::version() const
{
#ifdef RARDLL
    return QT_STRINGIFY(RARVER_MAJOR) "." QT_STRINGIFY(RARVER_MINOR) "." QT_STRINGIFY(RARVER_BETA);
#else
    return "0";
#endif
}
