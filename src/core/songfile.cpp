#include "songfile.h"
#include "songstruct.h"


RMSong::SongClientFile::SongClientFile()
{
}

RMSong::SongClientFile::~SongClientFile()
{
    if (!m_songsList.isEmpty()) {
        foreach (SongClientItemStruct *s, m_songsList)
            delete s;
    }
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
                m_fileHeader = ba.mid(0, 0x88);
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
    if (format == Unknown || output == NULL)
        return false;
    else if (format == BinFormat) {
        if (output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            output->write(m_fileHeader, 0x88);
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

const QByteArray &RMSong::SongClientFile::fileHeader() const
{
    return m_fileHeader;
}

int RMSong::SongClientFile::songCount() const
{
    return m_songsList.length();
}

RMSong::PapaSongClientFile::PapaSongClientFile()
{
}

RMSong::PapaSongClientFile::~PapaSongClientFile()
{
    if (!m_songsList.isEmpty()) {
        foreach (PapaSongClientItemStruct *s, m_songsList)
            delete s;
    }
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
                m_fileHeader = ba.mid(0, 0x88);
                for (int i = 0x88; i < ba.size(); i += 0x169) {
                    QByteArray sp = ba.mid(i, 0x33e);
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
    if (format == Unknown || output == NULL)
        return false;
    else if (format == BinFormat) {
        if (output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            output->write(m_fileHeader, 0x88);
            foreach (PapaSongClientItemStruct *s, m_songsList) {
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

RMSong::PapaSongClientItemStruct *RMSong::PapaSongClientFile::song(int n)
{
    return m_songsList.value(n);
}

const RMSong::PapaSongClientItemStruct *RMSong::PapaSongClientFile::song(int n) const
{
    return m_songsList.value(n);
}

const QByteArray &RMSong::PapaSongClientFile::fileHeader() const
{
    return m_fileHeader;
}

int RMSong::PapaSongClientFile::songCount() const
{
    return m_songsList.length();
}
