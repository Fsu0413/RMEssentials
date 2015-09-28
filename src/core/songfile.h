#ifndef __SONGFILE_H__
#define __SONGFILE_H__

class QIODevice;

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
        bool readInfoFromDevice(QIODevice *input, FileFormat format);
        bool saveInfoToDevice(QIODevice *output, FileFormat format) const;
        SongClientItemStruct *song(int n);
        const SongClientItemStruct *song(int n) const;
        const SongClientHeaderStruct &fileHeader() const;

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

        bool readInfoFromDevice(QIODevice *input, FileFormat format);
        bool saveInfoToDevice(QIODevice *output, FileFormat format) const;
        PapaSongClientItemStruct *song(int n);
        const PapaSongClientItemStruct *song(int n) const;
        const SongClientHeaderStruct &fileHeader() const;

        int songCount() const;

    private:
        QList<PapaSongClientItemStruct *> m_songsList;

        SongClientHeaderStruct *m_header;

        void cleanup();
    };

}

#endif