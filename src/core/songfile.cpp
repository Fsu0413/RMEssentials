#include "songfile.h"
#include "songstruct.h"


RMSong::SongClientFile::SongClientFile() : m_header(NULL)
{
}

RMSong::SongClientFile::~SongClientFile()
{
    if (!m_songsList.isEmpty()) {
        foreach (SongClientItemStruct *s, m_songsList)
            delete s;
    }

    delete m_header;
}

void RMSong::SongClientFile::cleanup()
{
    if (!m_songsList.isEmpty()) {
        foreach (SongClientItemStruct *s, m_songsList)
            delete s;
    }

    m_songsList.clear();
}

bool RMSong::SongClientFile::readInfoFromDevice(QIODevice *input, FileFormat format)
{
    // treat the unknown format
    if (format == Unknown || input == NULL)
        return false;
    else if (format == BinFormat) {
        if (input->open(QIODevice::ReadOnly)) {
            cleanup();
            QByteArray ba = input->readAll();
            if (ba.size() % 0x33e == 0x88) {
                if (m_header == NULL)
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
    if (format == Unknown || output == NULL || m_header == NULL)
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

RMSong::PapaSongClientFile::PapaSongClientFile() : m_header(NULL)
{
}

RMSong::PapaSongClientFile::~PapaSongClientFile()
{
    if (!m_songsList.isEmpty()) {
        foreach (PapaSongClientItemStruct *s, m_songsList)
            delete s;
    }

    delete m_header;
}

void RMSong::PapaSongClientFile::cleanup()
{
    if (!m_songsList.isEmpty()) {
        foreach (PapaSongClientItemStruct *s, m_songsList)
            delete s;
    }

    m_songsList.clear();
}

bool RMSong::PapaSongClientFile::readInfoFromDevice(QIODevice *input, FileFormat format)
{
    // treat the unknown format
    if (format == Unknown || input == NULL)
        return false;
    else if (format == BinFormat) {
        if (input->open(QIODevice::ReadOnly)) {
            cleanup();
            QByteArray ba = input->readAll();
            if (ba.size() % 0x169 == 0x88) {
                if (m_header == NULL)
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
    if (format == Unknown || output == NULL || m_header == NULL)
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