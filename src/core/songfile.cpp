#include "songfile.h"
#include "songstruct.h"

#include <QIODevice>

RMSong::SongClientFile::SongClientFile()
    : m_header(nullptr)
{
}

RMSong::SongClientFile::~SongClientFile()
{
    qDeleteAll(m_songsList);
    delete m_header;
}

void RMSong::SongClientFile::cleanup()
{
    qDeleteAll(m_songsList);
    m_songsList.clear();
}

bool RMSong::SongClientFile::readInfoFromDevice(QIODevice *input, FileFormat format)
{
    // treat the unknown format
    if (format == Unknown || input == nullptr)
        return false;
    else if (format == BinFormat) {
        if ((input->isOpen() && input->isReadable()) || input->open(QIODevice::ReadOnly)) {
            cleanup();
            QByteArray ba = input->readAll();
            if (ba.size() % 0x33e == 0x88) {
                if (m_header == nullptr)
                    m_header = new SongClientHeaderStruct;
                QByteArray fh = ba.mid(0, 0x88);
                ByteArray2Header(fh, *m_header);
                for (int i = 0x88; i < ba.size(); i += 0x33e) {
                    QByteArray sp = ba.mid(i, 0x33e);
                    SongClientItemStruct *ss = new SongClientItemStruct;
                    ByteArray2Song(sp, *ss);
                    m_songsList << ss;
                }
                input->close();
                return true;
            }
            input->close();
        }
    } else if (format == XmlFormat) {
        // to be realized
    }
    return false;
}

bool RMSong::SongClientFile::saveInfoToDevice(QIODevice *output, FileFormat format) const
{
    if (format == Unknown || output == nullptr || m_header == nullptr)
        return false;
    else if (format == BinFormat) {
        if (output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QByteArray fh;
            Header2ByteArray(*m_header, fh);
            output->write(fh, 0x88);
            foreach (SongClientItemStruct *s, m_songsList) {
                QByteArray arr;
                Song2ByteArray(*s, arr);
                output->write(arr.constData(), 0x33e);
            }
            output->close();
            return true;
        }
    } else if (format == XmlFormat) {
        // to be realized
    }

    return false;
}

RMSong::SongClientItemStruct *RMSong::SongClientFile::song(int n)
{
    return m_songsList.value(n);
}

const RMSong::SongClientItemStruct *RMSong::SongClientFile::song(int n) const
{
    return m_songsList.value(n);
}

const RMSong::SongClientHeaderStruct &RMSong::SongClientFile::fileHeader() const
{
    return *m_header;
}

QList<int> RMSong::SongClientFile::search(const QString &cond) const
{
    QList<int> r;
    bool matchId = false;
    int preferedId = cond.toInt(&matchId);
    int i = 0;
    foreach (const SongClientItemStruct *s, m_songsList) {
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

int RMSong::SongClientFile::songCount() const
{
    return m_songsList.length();
}

void RMSong::SongClientFile::mergeSongList(SongClientFile *file2)
{
    if (file2 == nullptr)
        return;

    delete m_header;
    m_header = file2->m_header;
    file2->m_header = nullptr;

    QList<SongClientItemStruct *> s = m_songsList;
    m_songsList = file2->m_songsList;
    file2->m_songsList.clear();
    delete file2;

    int j = 0;
    for (int i = 0; i < m_songsList.length(); ++i) {
        if (s.value(j)->m_ushSongID == m_songsList.value(i)->m_ushSongID) {
            delete m_songsList.value(i);
            m_songsList[i] = s.value(j);
            s[j] = nullptr;
            ++j;
            if (j >= s.length())
                break;
        }
    }

    qDeleteAll(s);
}

RMSong::PapaSongClientFile::PapaSongClientFile()
    : m_header(nullptr)
{
}

RMSong::PapaSongClientFile::~PapaSongClientFile()
{
    qDeleteAll(m_songsList);
    delete m_header;
}

void RMSong::PapaSongClientFile::cleanup()
{
    qDeleteAll(m_songsList);
    m_songsList.clear();
}

bool RMSong::PapaSongClientFile::readInfoFromDevice(QIODevice *input, FileFormat format)
{
    // treat the unknown format
    if (format == Unknown || input == nullptr)
        return false;
    else if (format == BinFormat) {
        if ((input->isOpen() && input->isReadable()) || input->open(QIODevice::ReadOnly)) {
            cleanup();
            QByteArray ba = input->readAll();
            if (ba.size() % 0x169 == 0x88) {
                if (m_header == nullptr)
                    m_header = new SongClientHeaderStruct;
                QByteArray fh = ba.mid(0, 0x88);
                ByteArray2Header(fh, *m_header);
                for (int i = 0x88; i < ba.size(); i += 0x169) {
                    QByteArray sp = ba.mid(i, 0x169);
                    PapaSongClientItemStruct *ss = new PapaSongClientItemStruct;
                    ByteArray2Song(sp, *ss);
                    m_songsList << ss;
                }
                input->close();
                return true;
            }
            input->close();
        }
    } else if (format == XmlFormat) {
        // to be realized
    }
    return false;
}

bool RMSong::PapaSongClientFile::saveInfoToDevice(QIODevice *output, FileFormat format) const
{
    if (format == Unknown || output == nullptr || m_header == nullptr)
        return false;
    else if (format == BinFormat) {
        if (output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QByteArray fh;
            Header2ByteArray(*m_header, fh);
            output->write(fh, 0x88);
            foreach (PapaSongClientItemStruct *s, m_songsList) {
                QByteArray arr;
                Song2ByteArray(*s, arr);
                output->write(arr.constData(), 0x169);
            }
            output->close();
            return true;
        }
    } else if (format == XmlFormat) {
        // to be realized
    }

    return false;
}

RMSong::PapaSongClientItemStruct *RMSong::PapaSongClientFile::song(int n)
{
    return m_songsList.value(n);
}

const RMSong::PapaSongClientItemStruct *RMSong::PapaSongClientFile::song(int n) const
{
    return m_songsList.value(n);
}

const RMSong::SongClientHeaderStruct &RMSong::PapaSongClientFile::fileHeader() const
{
    return *m_header;
}

int RMSong::PapaSongClientFile::songCount() const
{
    return m_songsList.length();
}

QList<int> RMSong::PapaSongClientFile::search(const QString &cond) const
{
    QList<int> r;
    bool matchId = false;
    int preferedId = cond.toInt(&matchId);
    int i = 0;
    foreach (const PapaSongClientItemStruct *s, m_songsList) {
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
