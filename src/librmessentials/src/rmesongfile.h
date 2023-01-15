#ifndef RMESONGFILE_H__INCLUDED
#define RMESONGFILE_H__INCLUDED

#include "rmeglobal.h"

#include <QList>

class QIODevice;

namespace RmeSong {
struct RmeSongClientItemStruct;
struct RmePapaSongClientItemStruct;
struct RmeSongClientHeaderStruct;

enum RmeFileFormat
{
    UnknownFormat,
    BinFormat,
    XmlFormat,
    JsonFormat, // Leaked Rhythm Master Remastered internal data
};

class RmeSongClientFilePrivate;

class LIBRMESSENTIALS_EXPORT RmeSongClientFile
{
public:
    RmeSongClientFile();
    ~RmeSongClientFile();

    bool readInfoFromDevice(QIODevice *input, RmeFileFormat format); // the device should be opened READABLE or CLOSED when calling this func
    bool saveInfoToDevice(QIODevice *output, RmeFileFormat format) const; // the device must be CLOSED when calling this func

    RmeSongClientItemStruct *song(int n);
    const RmeSongClientItemStruct *song(int n) const;

    const RmeSongClientHeaderStruct &fileHeader() const;
    QList<int> search(const QString &cond) const;

    bool savePatchToDevice(QIODevice *output, const RmeSongClientFile &orig) const;
    bool applyPatchFromDevice(QIODevice *input);

    int songCount() const;

    void prepareForUserMadeNotes();
    bool isUserMadeMode() const;

private:
    Q_DISABLE_COPY(RmeSongClientFile)
    Q_DECLARE_PRIVATE(RmeSongClientFile)
    RmeSongClientFilePrivate *d_ptr;
};

class RmePapaSongClientFilePrivate;

class LIBRMESSENTIALS_EXPORT RmePapaSongClientFile
{
public:
    RmePapaSongClientFile();
    ~RmePapaSongClientFile();

    bool readInfoFromDevice(QIODevice *input, RmeFileFormat format); // the device should be opened READABLE or CLOSED when calling this func
    bool saveInfoToDevice(QIODevice *output, RmeFileFormat format) const; // the device must be CLOSED when calling this func

    RmePapaSongClientItemStruct *song(int n);
    const RmePapaSongClientItemStruct *song(int n) const;

    const RmeSongClientHeaderStruct &fileHeader() const;
    QList<int> search(const QString &cond) const;

    bool savePatchToDevice(QIODevice *output, const RmePapaSongClientFile &orig) const;
    bool applyPatchFromDevice(QIODevice *input);

    int songCount() const;

private:
    Q_DISABLE_COPY(RmePapaSongClientFile)
    Q_DECLARE_PRIVATE(RmePapaSongClientFile)
    RmePapaSongClientFilePrivate *d_ptr;
};
}

#endif
