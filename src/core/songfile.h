#ifndef __SONGFILE_H__
#define __SONGFILE_H__

class QIODevice;

#include <QList>

namespace RMSong
{
    struct SongClientItemStruct;
    struct PapaSongClientItemStruct;
    struct SongClientHeaderStruct;

    enum FileFormat
    {
        Unknown,
        BinFormat,
        XmlFormat
    };

    class SongClientFile
    {
    public:
        SongClientFile();
        ~SongClientFile();
        bool readInfoFromDevice(QIODevice *input, FileFormat format); // the device should be opened READABLE or CLOSED when calling this func
        bool saveInfoToDevice(QIODevice *output, FileFormat format) const; // the device must be CLOSED when calling this func
        SongClientItemStruct *song(int n);
        const SongClientItemStruct *song(int n) const;
        const SongClientHeaderStruct &fileHeader() const;
        QList<int> search(const QString &cond) const;

        // Attention!! after merging, the file2 is not available anymore, thus been deleted, please do not get access to it after calling this function!!!
        void mergeSongList(SongClientFile *file2);

        int songCount() const;

    private:
        QList<SongClientItemStruct *> m_songsList;

        SongClientHeaderStruct *m_header;

        void cleanup();
    };

    class PapaSongClientFile
    {
    public:
        PapaSongClientFile();
        ~PapaSongClientFile();

        bool readInfoFromDevice(QIODevice *input, FileFormat format); // the device should be opened READABLE or CLOSED when calling this func
        bool saveInfoToDevice(QIODevice *output, FileFormat format) const; // the device must be CLOSED when calling this func
        PapaSongClientItemStruct *song(int n);
        const PapaSongClientItemStruct *song(int n) const;
        const SongClientHeaderStruct &fileHeader() const;
        QList<int> search(const QString &cond) const;

        int songCount() const;

    private:
        QList<PapaSongClientItemStruct *> m_songsList;

        SongClientHeaderStruct *m_header;

        void cleanup();
    };

}

#endif
