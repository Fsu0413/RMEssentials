#include "rmesongfile.h"
#include "rmesongstruct.h"

using namespace RmeSong;

#include <QIODevice>

class RmeSong::RmeSongClientFilePrivate
{
public:
    QList<RmeSongClientItemStruct *> m_songsList;
    RmeSongClientHeaderStruct *m_header;
    void cleanup();
};

RmeSong::RmeSongClientFile::RmeSongClientFile()
    : d_ptr(new RmeSongClientFilePrivate)
{
    Q_D(RmeSongClientFile);
    d->m_header = nullptr;
}

RmeSong::RmeSongClientFile::~RmeSongClientFile()
{
    Q_D(RmeSongClientFile);
    qDeleteAll(d->m_songsList);
    delete d->m_header;
    delete d;
}

void RmeSong::RmeSongClientFilePrivate::cleanup()
{
    qDeleteAll(m_songsList);
    m_songsList.clear();
}

bool RmeSong::RmeSongClientFile::readInfoFromDevice(QIODevice *input, RmeFileFormat format)
{
    Q_D(RmeSongClientFile);
    // treat the unknown format
    if (format == Unknown || input == nullptr)
        return false;
    else if (format == BinFormat) {
        if ((input->isOpen() && input->isReadable()) || input->open(QIODevice::ReadOnly)) {
            d->cleanup();
            QByteArray ba = input->readAll();
            if (ba.size() % 0x33e == 0x88) {
                if (d->m_header == nullptr)
                    d->m_header = new RmeSongClientHeaderStruct;
                QByteArray fh = ba.mid(0, 0x88);
                ByteArray2Header(fh, *d->m_header);
                for (int i = 0x88; i < ba.size(); i += 0x33e) {
                    QByteArray sp = ba.mid(i, 0x33e);
                    RmeSongClientItemStruct *ss = new RmeSongClientItemStruct;
                    ByteArray2Song(sp, *ss);
                    d->m_songsList << ss;
                }
                input->close();
                return true;
            }
            input->close();
        }
    } else if (format == XmlFormat) {
        // to be realized
        Q_UNIMPLEMENTED();
        Q_UNUSED(d);
    }
    return false;
}

bool RmeSong::RmeSongClientFile::saveInfoToDevice(QIODevice *output, RmeFileFormat format) const
{
    Q_D(const RmeSongClientFile);
    if (format == Unknown || output == nullptr || d->m_header == nullptr)
        return false;
    else if (format == BinFormat) {
        if (output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QByteArray fh;
            Header2ByteArray(*d->m_header, fh);
            output->write(fh, 0x88);
            foreach (RmeSongClientItemStruct *s, d->m_songsList) {
                QByteArray arr;
                Song2ByteArray(*s, arr);
                output->write(arr.constData(), 0x33e);
            }
            output->close();
            return true;
        }
    } else if (format == XmlFormat) {
        // to be realized
        Q_UNUSED(d);
    }

    return false;
}

RmeSong::RmeSongClientItemStruct *RmeSong::RmeSongClientFile::song(int n)
{
    Q_D(RmeSongClientFile);
    return d->m_songsList.value(n);
}

const RmeSong::RmeSongClientItemStruct *RmeSong::RmeSongClientFile::song(int n) const
{
    Q_D(const RmeSongClientFile);
    return d->m_songsList.value(n);
}

const RmeSong::RmeSongClientHeaderStruct &RmeSong::RmeSongClientFile::fileHeader() const
{
    Q_D(const RmeSongClientFile);
    return *d->m_header;
}

QList<int> RmeSong::RmeSongClientFile::search(const QString &cond) const
{
    Q_D(const RmeSongClientFile);
    QList<int> r;
    bool matchId = false;
    int preferedId = cond.toInt(&matchId);
    int i = 0;
    foreach (const RmeSongClientItemStruct *s, d->m_songsList) {
        bool flag = false;
        if (matchId && s->m_ushSongID == preferedId)
            flag = true;
        else if (s->m_szSongName.toLower().contains(cond.toLower()))
            flag = true;
        else if (s->m_szPath.toLower().contains(cond.toLower()))
            flag = true;
        else if (s->m_szArtist.toLower().contains(cond.toLower()))
            flag = true;

        if (flag)
            r << i;

        ++i;
    }

    return r;
}

int RmeSong::RmeSongClientFile::songCount() const
{
    Q_D(const RmeSongClientFile);
    return d->m_songsList.length();
}

void RmeSong::RmeSongClientFile::mergeSongList(RmeSongClientFile *file2)
{
    if (file2 == nullptr)
        return;

    Q_D(RmeSongClientFile);
    delete d->m_header;
    d->m_header = file2->d_ptr->m_header;
    file2->d_ptr->m_header = nullptr;

    QList<RmeSongClientItemStruct *> s = d->m_songsList;
    d->m_songsList = file2->d_ptr->m_songsList;
    file2->d_ptr->m_songsList.clear();
    delete file2;

    int j = 0;
    for (int i = 0; i < d->m_songsList.length(); ++i) {
        if (s.value(j)->m_ushSongID == d->m_songsList.value(i)->m_ushSongID) {
            delete d->m_songsList.value(i);
            d->m_songsList[i] = s.value(j);
            s[j] = nullptr;
            ++j;
            if (j >= s.length())
                break;
        }
    }

    qDeleteAll(s);
}

class RmeSong::RmePapaSongClientFilePrivate
{
public:
    QList<RmePapaSongClientItemStruct *> m_songsList;
    RmeSongClientHeaderStruct *m_header;
    void cleanup();
};

RmeSong::RmePapaSongClientFile::RmePapaSongClientFile()
    : d_ptr(new RmePapaSongClientFilePrivate)
{
    Q_D(RmePapaSongClientFile);
    d->m_header = nullptr;
}

RmeSong::RmePapaSongClientFile::~RmePapaSongClientFile()
{
    Q_D(RmePapaSongClientFile);
    qDeleteAll(d->m_songsList);
    delete d->m_header;
    delete d;
}

void RmeSong::RmePapaSongClientFilePrivate::cleanup()
{
    qDeleteAll(m_songsList);
    m_songsList.clear();
}

bool RmeSong::RmePapaSongClientFile::readInfoFromDevice(QIODevice *input, RmeFileFormat format)
{
    Q_D(RmePapaSongClientFile);
    // treat the unknown format
    if (format == Unknown || input == nullptr)
        return false;
    else if (format == BinFormat) {
        if ((input->isOpen() && input->isReadable()) || input->open(QIODevice::ReadOnly)) {
            d->cleanup();
            QByteArray ba = input->readAll();
            if (ba.size() % 0x169 == 0x88) {
                if (d->m_header == nullptr)
                    d->m_header = new RmeSongClientHeaderStruct;
                QByteArray fh = ba.mid(0, 0x88);
                ByteArray2Header(fh, *d->m_header);
                for (int i = 0x88; i < ba.size(); i += 0x169) {
                    QByteArray sp = ba.mid(i, 0x169);
                    RmePapaSongClientItemStruct *ss = new RmePapaSongClientItemStruct;
                    ByteArray2Song(sp, *ss);
                    d->m_songsList << ss;
                }
                input->close();
                return true;
            }
            input->close();
        }
    } else if (format == XmlFormat) {
        // to be realized
        Q_UNUSED(d);
    }
    return false;
}

bool RmeSong::RmePapaSongClientFile::saveInfoToDevice(QIODevice *output, RmeFileFormat format) const
{
    Q_D(const RmePapaSongClientFile);
    if (format == Unknown || output == nullptr || d->m_header == nullptr)
        return false;
    else if (format == BinFormat) {
        if (output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QByteArray fh;
            Header2ByteArray(*d->m_header, fh);
            output->write(fh, 0x88);
            foreach (RmePapaSongClientItemStruct *s, d->m_songsList) {
                QByteArray arr;
                Song2ByteArray(*s, arr);
                output->write(arr.constData(), 0x169);
            }
            output->close();
            return true;
        }
    } else if (format == XmlFormat) {
        // to be realized
        Q_UNUSED(d);
    }

    return false;
}

RmeSong::RmePapaSongClientItemStruct *RmeSong::RmePapaSongClientFile::song(int n)
{
    Q_D(RmePapaSongClientFile);
    return d->m_songsList.value(n);
}

const RmeSong::RmePapaSongClientItemStruct *RmeSong::RmePapaSongClientFile::song(int n) const
{
    Q_D(const RmePapaSongClientFile);
    return d->m_songsList.value(n);
}

const RmeSong::RmeSongClientHeaderStruct &RmeSong::RmePapaSongClientFile::fileHeader() const
{
    Q_D(const RmePapaSongClientFile);
    return *d->m_header;
}

int RmeSong::RmePapaSongClientFile::songCount() const
{
    Q_D(const RmePapaSongClientFile);
    return d->m_songsList.length();
}

QList<int> RmeSong::RmePapaSongClientFile::search(const QString &cond) const
{
    Q_D(const RmePapaSongClientFile);
    QList<int> r;
    bool matchId = false;
    int preferedId = cond.toInt(&matchId);
    int i = 0;
    foreach (const RmePapaSongClientItemStruct *s, d->m_songsList) {
        bool flag = false;
        if (matchId && s->m_ushSongID == preferedId)
            flag = true;
        else if (s->m_szSongName.toLower().contains(cond.toLower()))
            flag = true;
        else if (s->m_szPath.toLower().contains(cond.toLower()))
            flag = true;
        else if (s->m_szArtist.toLower().contains(cond.toLower()))
            flag = true;

        if (flag)
            r << i;

        ++i;
    }

    return r;
}
