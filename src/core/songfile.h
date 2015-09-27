#ifndef __SONGFILE_H__
#define __SONGFILE_H__

class QIODevice;

namespace RMSong
{
    struct SongClientItemStruct;
    struct PapaSongClientItemStruct;

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
        const QByteArray &fileHeader() const;

        int songCount() const;

    private:
        QList<SongClientItemStruct *> m_songsList;

        QByteArray m_fileHeader; // to be re-realized as SongClientHeader

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
        const QByteArray &fileHeader() const;

        int songCount() const;

    private:
        QList<PapaSongClientItemStruct *> m_songsList;

        QByteArray m_fileHeader; // to be re-realized as PapaSongClientHeader
        void cleanup();
    };

}

#endif