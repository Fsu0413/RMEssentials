#include "rmesongfile.h"
#include "rmesongstruct.h"

#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QXmlStreamWriter>

#include <cstring>

namespace {

class RmeFileReader
{
public:
    static RmeFileReader *createFileReader(RmeFileFormat format, const QByteArray &arr);

    constexpr RmeFileReader()
        : m_isPapa(false)
        , m_isError(false)
    {
    }
    virtual ~RmeFileReader()
    {
    }

    constexpr inline bool isPapa() const
    {
        return m_isPapa;
    }
    constexpr inline void setIsPapa(bool isPapa)
    {
        m_isPapa = isPapa;
    }

    constexpr inline bool isError() const
    {
        return m_isError;
    }
    constexpr inline void setIsError(bool isError)
    {
        m_isError = isError;
    }

    virtual RmeSongClientHeaderStruct *readHeader() = 0;
    virtual RmeSongClientItemStruct *readItem() = 0;
    virtual RmePapaSongClientItemStruct *readPapaItem() = 0;

private:
    bool m_isPapa;
    bool m_isError;

    Q_DISABLE_COPY_MOVE(RmeFileReader)
};

class RmeFileWriter
{
public:
    static RmeFileWriter *createFileWriter(RmeFileFormat format, bool isPapa, QIODevice *device);

    RmeFileWriter() = delete;
    constexpr explicit RmeFileWriter(bool isPapa)
        : m_isPapa(isPapa)
    {
    }
    virtual ~RmeFileWriter()
    {
    }

    constexpr inline bool isPapa() const
    {
        return m_isPapa;
    }
    constexpr inline void setIsPapa(bool isPapa)
    {
        m_isPapa = isPapa;
    }

    virtual bool writeHeader(const RmeSongClientHeaderStruct *header) = 0;
    virtual bool writeClient(const RmeSongClientItemStruct *item) = 0;
    virtual bool writeClient(const RmePapaSongClientItemStruct *item) = 0;

private:
    bool m_isPapa;

    Q_DISABLE_COPY_MOVE(RmeFileWriter)
};

class RmeBinReader : public RmeFileReader
{
public:
    explicit RmeBinReader(const QByteArray &array);
    ~RmeBinReader() override = default;

    RmeSongClientHeaderStruct *readHeader() override;
    RmeSongClientItemStruct *readItem() override;
    RmePapaSongClientItemStruct *readPapaItem() override;

private:
    QByteArray arr;
    bool isPapaSet;
};

class RmeBinWriter : public RmeFileWriter
{
public:
    explicit RmeBinWriter(QIODevice *device, bool isPapa);
    ~RmeBinWriter() override = default;

    bool writeHeader(const RmeSongClientHeaderStruct *header) override;
    bool writeClient(const RmeSongClientItemStruct *item) override;
    bool writeClient(const RmePapaSongClientItemStruct *item) override;

private:
    QIODevice *m_device;

    qint64 writeByteArray(const QByteArray &arr);
};

RmeBinReader::RmeBinReader(const QByteArray &array)
    : arr(array)
    , isPapaSet(false)
{
    setIsError(arr.size() < 0x88);
}

RmeSongClientHeaderStruct *RmeBinReader::readHeader()
{
    if (isError())
        return nullptr;

    setIsError(true);

    RmeSongClientHeaderStruct *header = new RmeSongClientHeaderStruct;
    QByteArray fh = arr.left(0x88);
    header->parseByteArray(fh);
    arr = arr.mid(0x88);

    if (arr.size() % 0x33e == 0) {
        if (arr.size() % 0x169 != 0) {
            isPapaSet = true;
            setIsPapa(false);
        }
    } else if (arr.size() % 0x169 == 0) {
        isPapaSet = true;
        setIsPapa(true);
    } else {
        delete header;
        return nullptr;
    }

    setIsError(false);

    return header;
}

RmeSongClientItemStruct *RmeBinReader::readItem()
{
    setIsError(arr.size() % 0x33e != 0);
    if (isError())
        return nullptr;

    setIsError(true);

    if (!isPapaSet) {
        setIsPapa(false);
        isPapaSet = true;
    }
    if (isPapa())
        return nullptr;

    if (arr.isEmpty())
        return nullptr;

    QByteArray sp = arr.left(0x33e);
    arr = arr.mid(0x33e);

    RmeSongClientItemStruct *ss = new RmeSongClientItemStruct;
    ss->parseByteArray(sp);

    setIsError(false);

    return ss;
}

RmePapaSongClientItemStruct *RmeBinReader::readPapaItem()
{
    setIsError(arr.size() % 0x169 != 0);
    if (isError())
        return nullptr;

    setIsError(true);

    if (!isPapaSet) {
        setIsPapa(true);
        isPapaSet = true;
    }
    if (!isPapa())
        return nullptr;

    if (arr.isEmpty())
        return nullptr;

    QByteArray sp = arr.left(0x169);
    arr = arr.mid(0x169);

    RmePapaSongClientItemStruct *ss = new RmePapaSongClientItemStruct;
    ss->parseByteArray(sp);

    setIsError(false);

    return ss;
}

RmeBinWriter::RmeBinWriter(QIODevice *device, bool isPapa)
    : RmeFileWriter(isPapa)
    , m_device(device)
{
}

bool RmeBinWriter::writeHeader(const RmeSongClientHeaderStruct *header)
{
    return writeByteArray(header->toByteArray()) == 0x88;
}

bool RmeBinWriter::writeClient(const RmeSongClientItemStruct *item)
{
    return writeByteArray(item->toByteArray()) == 0x33e;
}

bool RmeBinWriter::writeClient(const RmePapaSongClientItemStruct *item)
{
    return writeByteArray(item->toByteArray()) == 0x169;
}

qint64 RmeBinWriter::writeByteArray(const QByteArray &arr)
{
    const char *data = arr.constData();
    int64_t length = arr.length();
    int64_t total = 0;
    int64_t written = m_device->write(data, length);
    while (written > 0) {
        total += written;
        length -= written;
        if (length <= 0)
            return total;

        // Unreachable for QFile, but probably reachable for other QIODevice -s.
        written = m_device->write(data + total, length);
    }

    return written;
}

class RmeXmlReader : public RmeFileReader
{
public:
    explicit RmeXmlReader(const QByteArray &array);
    ~RmeXmlReader() override = default;

    RmeSongClientHeaderStruct *readHeader() override;
    RmeSongClientItemStruct *readItem() override;
    RmePapaSongClientItemStruct *readPapaItem() override;

private:
    QXmlStreamReader m_reader;
};

class RmeXmlWriter : public RmeFileWriter
{
public:
    explicit RmeXmlWriter(QIODevice *device, bool isPapa);
    ~RmeXmlWriter() override;

    bool writeHeader(const RmeSongClientHeaderStruct *header) override;
    bool writeClient(const RmeSongClientItemStruct *item) override;
    bool writeClient(const RmePapaSongClientItemStruct *item) override;

private:
    bool startWriting(bool isPapa);

    QXmlStreamWriter m_writer;
};

RmeXmlReader::RmeXmlReader(const QByteArray &array)
    : m_reader(array)
{
}

#define READNEXT(retvalue)       \
    do {                         \
        m_reader.readNext();     \
        if (m_reader.hasError()) \
            return retvalue;     \
    } while (m_reader.isWhitespace())

RmeSongClientHeaderStruct *RmeXmlReader::readHeader()
{
    if (isError())
        return nullptr;

    setIsError(true);

    READNEXT(nullptr);

    if (!m_reader.isStartDocument())
        return nullptr;

    READNEXT(nullptr);

    if (!m_reader.isStartElement())
        return nullptr;
    if (!m_reader.name().endsWith(QStringLiteral("SongConfig_Client_Tab")))
        return nullptr;
    if (m_reader.name().startsWith(QStringLiteral("Papa")))
        setIsPapa(true);

    READNEXT(nullptr);

    if (!m_reader.isStartElement())
        return nullptr;
    if (m_reader.name() != QStringLiteral("TResHeadAll"))
        return nullptr;
    READNEXT(nullptr);

    if (!m_reader.isStartElement())
        return nullptr;
    if (m_reader.name() != QStringLiteral("resHead"))
        return nullptr;

    READNEXT(nullptr);

    QVariantMap map;
    while (m_reader.isStartElement()) {
        map[m_reader.name().toString()] = m_reader.readElementText();
        if (m_reader.hasError())
            return nullptr;

        READNEXT(nullptr);
    }

    if (!m_reader.isEndElement())
        return nullptr;
    if (m_reader.name() != QStringLiteral("resHead"))
        return nullptr;

    READNEXT(nullptr);

    if (!m_reader.isStartElement())
        return nullptr;
    if (m_reader.name() != QStringLiteral("resHeadExt"))
        return nullptr;

    READNEXT(nullptr);

    while (m_reader.isStartElement()) {
        map[m_reader.name().toString()] = m_reader.readElementText();
        if (m_reader.hasError())
            return nullptr;

        READNEXT(nullptr);
    }

    if (!m_reader.isEndElement())
        return nullptr;
    if (m_reader.name() != QStringLiteral("resHeadExt"))
        return nullptr;

    READNEXT(nullptr);

    if (!m_reader.isEndElement())
        return nullptr;
    if (m_reader.name() != QStringLiteral("TResHeadAll"))
        return nullptr;

    RmeSongClientHeaderStruct *header = new RmeSongClientHeaderStruct;
    header->parseMap(map);

    setIsError(false);
    return header;
}

RmeSongClientItemStruct *RmeXmlReader::readItem()
{
    if (isError())
        return nullptr;

    setIsError(true);

    if (isPapa())
        return nullptr;

    READNEXT(nullptr);

    if (!m_reader.isStartElement())
        return nullptr;

    if (m_reader.name() != QStringLiteral("SongConfig_Client"))
        return nullptr;

    READNEXT(nullptr);

    QVariantMap map;
    while (m_reader.isStartElement()) {
        map[m_reader.name().toString()] = m_reader.readElementText();
        if (m_reader.hasError())
            return nullptr;

        READNEXT(nullptr);
    }

    if (!m_reader.isEndElement())
        return nullptr;
    if (m_reader.name() != QStringLiteral("SongConfig_Client"))
        return nullptr;

    RmeSongClientItemStruct *item = new RmeSongClientItemStruct;
    item->parseMap(map);

    setIsError(false);
    return item;
}

RmePapaSongClientItemStruct *RmeXmlReader::readPapaItem()
{
    if (isError())
        return nullptr;

    setIsError(true);

    if (!isPapa())
        return nullptr;

    READNEXT(nullptr);

    if (!m_reader.isStartElement())
        return nullptr;

    if (m_reader.name() != QStringLiteral("PapaSongConfig_Client_Tab"))
        return nullptr;

    READNEXT(nullptr);

    QVariantMap map;
    while (m_reader.isStartElement()) {
        map[m_reader.name().toString()] = m_reader.readElementText();
        if (m_reader.hasError())
            return nullptr;

        READNEXT(nullptr);
    }

    if (!m_reader.isEndElement())
        return nullptr;
    if (m_reader.name() != QStringLiteral("PapaSongConfig_Client_Tab"))
        return nullptr;

    RmePapaSongClientItemStruct *item = new RmePapaSongClientItemStruct;
    item->parseMap(map);

    setIsError(false);

    return item;
}

#undef READNEXT

RmeXmlWriter::RmeXmlWriter(QIODevice *device, bool isPapa)
    : RmeFileWriter(isPapa)
    , m_writer(device)
{
    startWriting(isPapa);
}

RmeXmlWriter::~RmeXmlWriter()
{
    m_writer.writeEndElement();
    m_writer.writeEndDocument();
}

#define WRITEELEMENT(x) m_writer.writeTextElement(QStringLiteral(#x), map.value(QStringLiteral(#x)).toString())

bool RmeXmlWriter::writeHeader(const RmeSongClientHeaderStruct *header)
{
    QVariantMap map = header->toMap();

    m_writer.writeStartElement(QStringLiteral("TResHeadAll"));
    m_writer.writeAttribute(QStringLiteral("version"), QStringLiteral("5"));
    m_writer.writeStartElement(QStringLiteral("resHead"));
    m_writer.writeAttribute(QStringLiteral("type"), QStringLiteral("TResHead"));

    WRITEELEMENT(Magic);
    WRITEELEMENT(Version);
    WRITEELEMENT(Unit);
    WRITEELEMENT(Count);
    WRITEELEMENT(MetalibHash);
    WRITEELEMENT(ResVersion);
    WRITEELEMENT(CreateTime);
    WRITEELEMENT(ResEncording);
    WRITEELEMENT(ContentHash);

    m_writer.writeEndElement();
    m_writer.writeStartElement(QStringLiteral("resHeadExt"));
    m_writer.writeAttribute(QStringLiteral("type"), QStringLiteral("TResHeadExt"));

    WRITEELEMENT(DataOffset);

    m_writer.writeEndElement();
    m_writer.writeEndElement();

    return !m_writer.hasError();
}

bool RmeXmlWriter::writeClient(const RmeSongClientItemStruct *item)
{
    QVariantMap map = item->toMap();
    m_writer.writeStartElement(QStringLiteral("SongConfig_Client"));
    m_writer.writeAttribute(QStringLiteral("version"), QStringLiteral("1"));

    WRITEELEMENT(m_ushSongID);
    WRITEELEMENT(m_iVersion);
    WRITEELEMENT(m_szSongName);
    WRITEELEMENT(m_szPath);
    WRITEELEMENT(m_szArtist);
    WRITEELEMENT(m_szComposer);
    WRITEELEMENT(m_szSongTime);
    WRITEELEMENT(m_iGameTime);
    WRITEELEMENT(m_iRegion);
    WRITEELEMENT(m_iStyle);
    WRITEELEMENT(m_ucIsNew);
    WRITEELEMENT(m_ucIsHot);
    WRITEELEMENT(m_ucIsRecommend);
    WRITEELEMENT(m_szBPM);
    WRITEELEMENT(m_ucIsOpen);
    WRITEELEMENT(m_ucCanBuy);
    WRITEELEMENT(m_iOrderIndex);
    WRITEELEMENT(m_bIsFree);
    WRITEELEMENT(m_bSongPkg);
    WRITEELEMENT(m_szFreeBeginTime);
    WRITEELEMENT(m_szFreeEndTime);
    WRITEELEMENT(m_ush4KeyEasy);
    WRITEELEMENT(m_ush4KeyNormal);
    WRITEELEMENT(m_ush4KeyHard);
    WRITEELEMENT(m_ush5KeyEasy);
    WRITEELEMENT(m_ush5KeyNormal);
    WRITEELEMENT(m_ush5KeyHard);
    WRITEELEMENT(m_ush6KeyEasy);
    WRITEELEMENT(m_ush6KeyNormal);
    WRITEELEMENT(m_ush6KeyHard);
    WRITEELEMENT(m_iPrice);
    WRITEELEMENT(m_szNoteNumber);
    WRITEELEMENT(m_szProductID);
    WRITEELEMENT(m_iVipFlag);
    WRITEELEMENT(m_bIsHide);
    WRITEELEMENT(m_bIsReward);
    WRITEELEMENT(m_bIsLevelReward);

    m_writer.writeEndElement();

    return !m_writer.hasError();
}

bool RmeXmlWriter::writeClient(const RmePapaSongClientItemStruct *item)
{
    QVariantMap map = item->toMap();
    m_writer.writeStartElement(QStringLiteral("PapaSongConfig_Client"));
    m_writer.writeAttribute(QStringLiteral("version"), QStringLiteral("1"));

    WRITEELEMENT(m_ushSongID);
    WRITEELEMENT(m_iVersion);
    WRITEELEMENT(m_szSongName);
    WRITEELEMENT(m_cDifficulty);
    WRITEELEMENT(m_cLevel);
    WRITEELEMENT(m_szPath);
    WRITEELEMENT(m_szArtist);
    WRITEELEMENT(m_szSongTime);
    WRITEELEMENT(m_iGameTime);
    WRITEELEMENT(m_szRegion);
    WRITEELEMENT(m_szStyle);
    WRITEELEMENT(m_szBPM);
    WRITEELEMENT(m_szNoteNumber);
    WRITEELEMENT(m_iOrderIndex);
    WRITEELEMENT(m_ucIsOpen);
    WRITEELEMENT(m_ucIsFree);
    WRITEELEMENT(m_ucIsHide);
    WRITEELEMENT(m_ucIsReward);
    WRITEELEMENT(m_ucIsLevelReward);
    WRITEELEMENT(m_iSongType);

    m_writer.writeEndElement();

    return !m_writer.hasError();
}

#undef WRITEELEMENT

bool RmeXmlWriter::startWriting(bool isPapa)
{
    // Todo: find a way to control indent, Auto formating is.......
    m_writer.setAutoFormattingIndent(4);
    m_writer.setAutoFormatting(true);
#if QT_VERSION_MAJOR == 5
    m_writer.setCodec("UTF-8");
#endif
    m_writer.writeStartDocument(QStringLiteral("1.0"), true);
    if (isPapa)
        m_writer.writeStartElement(QStringLiteral("PapaSongConfig_Client_Tab"));
    else
        m_writer.writeStartElement(QStringLiteral("SongConfig_Client_Tab"));

    return !m_writer.hasError();
}

class RmeJsonReader : public RmeFileReader
{
public:
    explicit RmeJsonReader(const QByteArray &array);
    ~RmeJsonReader() override = default;

    RmeSongClientHeaderStruct *readHeader() override;
    RmeSongClientItemStruct *readItem() override;
    RmePapaSongClientItemStruct *readPapaItem() override;

private:
    QJsonArray m_jsonArray;
    QJsonParseError m_jsonError;

    bool load(const QByteArray &data);
};

class RmeJsonWriter : public RmeFileWriter
{
public:
    explicit RmeJsonWriter(QIODevice *device, bool isPapa);
    ~RmeJsonWriter() override;

    bool writeHeader(const RmeSongClientHeaderStruct *header) override;
    bool writeClient(const RmeSongClientItemStruct *item) override;
    bool writeClient(const RmePapaSongClientItemStruct *item) override;

private:
    QJsonArray m_jsonArray;

    QIODevice *m_device;

    Q_DISABLE_COPY(RmeJsonWriter)
    RmeJsonWriter() = delete;
};

RmeJsonReader::RmeJsonReader(const QByteArray &array)
{
    load(array);
}

RmeSongClientHeaderStruct *RmeJsonReader::readHeader()
{
    RmeSongClientHeaderStruct *r = new RmeSongClientHeaderStruct;
    r->Count = m_jsonArray.size();
    return r;
}

RmeSongClientItemStruct *RmeJsonReader::readItem()
{
    if (isError())
        return nullptr;

    setIsError(true);

    if (isPapa())
        return nullptr;

    if (m_jsonArray.isEmpty())
        return nullptr;

    QJsonValue v = m_jsonArray.takeAt(0);

    if (!v.isObject())
        return nullptr;

    QJsonObject ob = v.toObject();

    // idk what if we use toVariantMap
    QVariantMap map;
    for (auto it = ob.begin(); it != ob.end(); ++it) {
        QString value;
        if (it.value().isString())
            value = it.value().toString();
        else if (!it.value().isNull())
            return nullptr;

        map[it.key()] = value;
    }

    RmeSongClientItemStruct *item = new RmeSongClientItemStruct;
    item->parseMap(map);

    setIsError(false);

    return item;
}

RmePapaSongClientItemStruct *RmeJsonReader::readPapaItem()
{
    if (isError())
        return nullptr;

    setIsError(true);

    if (!isPapa())
        return nullptr;

    if (m_jsonArray.isEmpty())
        return nullptr;

    QJsonValue v = m_jsonArray.takeAt(0);

    if (!v.isObject())
        return nullptr;

    QJsonObject ob = v.toObject();

    // idk what if we use toVariantMap
    QVariantMap map;
    for (auto it = ob.begin(); it != ob.end(); ++it) {
        QString value;
        if (it.value().isString())
            value = it.value().toString();
        else if (!it.value().isNull())
            return nullptr;

        map[it.key()] = value;
    }

    RmePapaSongClientItemStruct *item = new RmePapaSongClientItemStruct;
    item->parseMap(map);

    setIsError(false);

    return item;
}

bool RmeJsonReader::load(const QByteArray &data)
{
    if (isError())
        return false;

    setIsError(true);

    QJsonDocument doc = QJsonDocument::fromJson(data, &m_jsonError);
    if (m_jsonError.error != QJsonParseError::NoError)
        return false;

    if (!doc.isArray())
        return false;

    m_jsonArray = doc.array();

    if (m_jsonArray.isEmpty())
        return false;

    // distinguish it is Papa struct or not
    // Papa struct has an "m_cLevel" in each element while normal mode does not
    QJsonValue v = m_jsonArray.at(0);
    if (v.isObject() && v.toObject().contains(QStringLiteral("m_cLevel"))) // assuming it is papa struct
        setIsPapa(true);

    setIsError(false);

    return true;
}

RmeJsonWriter::RmeJsonWriter(QIODevice *device, bool isPapa)
    : RmeFileWriter(isPapa)
    , m_device(device)
{
}

RmeJsonWriter::~RmeJsonWriter()
{
    QJsonDocument doc(m_jsonArray);
    QByteArray byteArray = doc.toJson(QJsonDocument::Indented);

    if ((m_device->isOpen() && m_device->isWritable()) || m_device->open(QIODevice::WriteOnly)) {
        int64_t written = m_device->write(byteArray);
        if (written < byteArray.length()) {
            // what to do....
            // I don't want to copy RmeBinWriter code here
        }
    }
}

bool RmeJsonWriter::writeHeader(const RmeSongClientHeaderStruct *)
{
    // nothing is needed to do
    return true;
}

#define WRITEELEMENT(x)                                         \
    do {                                                        \
        QString str = map.value(QStringLiteral(#x)).toString(); \
        if (str.isEmpty())                                      \
            ob[QStringLiteral(#x)] = QJsonValue::Null;          \
        else                                                    \
            ob[QStringLiteral(#x)] = str.trimmed();             \
    } while (0)

bool RmeJsonWriter::writeClient(const RmeSongClientItemStruct *item)
{
    QVariantMap map = item->toMap();

    QJsonObject ob;
    ob[QStringLiteral("@version")] = QStringLiteral("1");

    WRITEELEMENT(m_ushSongID);
    WRITEELEMENT(m_iVersion);
    WRITEELEMENT(m_szSongName);
    WRITEELEMENT(m_szPath);
    WRITEELEMENT(m_szArtist);
    WRITEELEMENT(m_szComposer);
    WRITEELEMENT(m_szSongTime);
    WRITEELEMENT(m_iGameTime);
    WRITEELEMENT(m_iRegion);
    WRITEELEMENT(m_iStyle);
    WRITEELEMENT(m_ucIsNew);
    WRITEELEMENT(m_ucIsHot);
    WRITEELEMENT(m_ucIsRecommend);
    WRITEELEMENT(m_szBPM);
    WRITEELEMENT(m_ucIsOpen);
    WRITEELEMENT(m_ucCanBuy);
    WRITEELEMENT(m_iOrderIndex);
    WRITEELEMENT(m_bIsFree);
    WRITEELEMENT(m_bSongPkg);
    WRITEELEMENT(m_szFreeBeginTime);
    WRITEELEMENT(m_szFreeEndTime);
    WRITEELEMENT(m_ush4KeyEasy);
    WRITEELEMENT(m_ush4KeyNormal);
    WRITEELEMENT(m_ush4KeyHard);
    WRITEELEMENT(m_ush5KeyEasy);
    WRITEELEMENT(m_ush5KeyNormal);
    WRITEELEMENT(m_ush5KeyHard);
    WRITEELEMENT(m_ush6KeyEasy);
    WRITEELEMENT(m_ush6KeyNormal);
    WRITEELEMENT(m_ush6KeyHard);
    WRITEELEMENT(m_iPrice);
    WRITEELEMENT(m_szNoteNumber);
    WRITEELEMENT(m_szProductID);
    WRITEELEMENT(m_iVipFlag);
    WRITEELEMENT(m_bIsHide);
    WRITEELEMENT(m_bIsReward);
    WRITEELEMENT(m_bIsLevelReward);
    WRITEELEMENT(m_ushTypeMark);

    m_jsonArray.append(ob);

    return true;
}

bool RmeJsonWriter::writeClient(const RmePapaSongClientItemStruct *item)
{
    QVariantMap map = item->toMap();

    QJsonObject ob;
    ob[QStringLiteral("@version")] = QStringLiteral("1");

    WRITEELEMENT(m_ushSongID);
    WRITEELEMENT(m_iVersion);
    WRITEELEMENT(m_szSongName);
    WRITEELEMENT(m_cDifficulty);
    WRITEELEMENT(m_cLevel);
    WRITEELEMENT(m_szPath);
    WRITEELEMENT(m_szArtist);
    WRITEELEMENT(m_szSongTime);
    WRITEELEMENT(m_iGameTime);
    WRITEELEMENT(m_szRegion);
    WRITEELEMENT(m_szStyle);
    WRITEELEMENT(m_szBPM);
    WRITEELEMENT(m_szNoteNumber);
    WRITEELEMENT(m_iOrderIndex);
    WRITEELEMENT(m_ucIsOpen);
    WRITEELEMENT(m_ucIsFree);
    WRITEELEMENT(m_ucIsHide);
    WRITEELEMENT(m_ucIsReward);
    WRITEELEMENT(m_ucIsLevelReward);
    WRITEELEMENT(m_iSongType);

    m_jsonArray.append(ob);

    return true;
}

#undef WRITEELEMENT

RmeFileReader *RmeFileReader::createFileReader(RmeFileFormat format, const QByteArray &arr)
{
    switch (format) {
    case BinFormat:
        return new RmeBinReader(arr);
    case XmlFormat:
        return new RmeXmlReader(arr);
    case JsonFormat:
        return new RmeJsonReader(arr);
    }

    return nullptr;
}

RmeFileWriter *RmeFileWriter::createFileWriter(RmeFileFormat format, bool isPapa, QIODevice *device)
{
    switch (format) {
    case BinFormat:
        return new RmeBinWriter(device, isPapa);
    case XmlFormat:
        return new RmeXmlWriter(device, isPapa);
    case JsonFormat:
        return new RmeJsonWriter(device, isPapa);
    }

    return nullptr;
}

}

class RmeSongClientFilePrivate
{
public:
    QMap<int16_t, RmeSongClientItemStruct *> m_songsList;
    QList<int16_t> m_songKeys;
    RmeSongClientHeaderStruct *m_header;
    void cleanup();
};

RmeSongClientFile::RmeSongClientFile()
    : d_ptr(new RmeSongClientFilePrivate)
{
    Q_D(RmeSongClientFile);
    d->m_header = nullptr;
}

RmeSongClientFile::~RmeSongClientFile()
{
    Q_D(RmeSongClientFile);
    qDeleteAll(d->m_songsList);
    delete d->m_header;
    delete d;
}

void RmeSongClientFilePrivate::cleanup()
{
    qDeleteAll(m_songsList);
    m_songsList.clear();
    m_songKeys.clear();
}

bool RmeSongClientFile::readInfoFromDevice(QIODevice *input, RmeFileFormat format)
{
    Q_D(RmeSongClientFile);
    // treat the unknown format
    if (format == UnknownFormat || input == nullptr)
        return false;

    if ((input->isOpen() && input->isReadable()) || input->open(QIODevice::ReadOnly)) {
        d->cleanup();
        QByteArray ba = input->readAll();
        input->close();

        std::unique_ptr<RmeFileReader> reader(RmeFileReader::createFileReader(format, ba));
        if (reader == nullptr)
            return false;

        d->m_header = reader->readHeader();
        for (int32_t i = 0; i < d->m_header->Count; ++i) {
            RmeSongClientItemStruct *ss = reader->readItem();
            if (ss == nullptr)
                return false;
            d->m_songsList[ss->m_ushSongID] = ss;
        }
        d->m_songKeys = d->m_songsList.keys();
        return true;
    }
    return false;
}

bool RmeSongClientFile::saveInfoToDevice(QIODevice *output, RmeFileFormat format) const
{
    Q_D(const RmeSongClientFile);
    if (format == UnknownFormat || output == nullptr || d->m_header == nullptr)
        return false;

    if (output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        {
            // All the writer writes their data during destructor so make the writer out of scope when writing done
            std::unique_ptr<RmeFileWriter> writer(RmeFileWriter::createFileWriter(format, false, output));
            if (!writer->writeHeader(d->m_header))
                return false;
            foreach (RmeSongClientItemStruct *s, d->m_songsList) {
                if (!writer->writeClient(s))
                    return false;
            }
        }
        output->close();
        return true;
    }

    return false;
}

RmeSongClientItemStruct *RmeSongClientFile::song(int n)
{
    Q_D(RmeSongClientFile);

    int16_t id = d->m_songKeys.value(n, 1);
    return d->m_songsList.value(id, nullptr);
}

const RmeSongClientItemStruct *RmeSongClientFile::song(int n) const
{
    Q_D(const RmeSongClientFile);

    int16_t id = d->m_songKeys.value(n, 1);
    return d->m_songsList.value(id, nullptr);
}

const RmeSongClientHeaderStruct &RmeSongClientFile::fileHeader() const
{
    Q_D(const RmeSongClientFile);
    return *d->m_header;
}

QList<int> RmeSongClientFile::search(const QString &cond) const
{
    Q_D(const RmeSongClientFile);
    QList<int> r;
    bool isNum = false;
    int num = cond.toInt(&isNum);
    int i = 0;
    foreach (const RmeSongClientItemStruct *s, d->m_songsList) {
        bool flag = false;
        if (s->m_szSongName.contains(cond, Qt::CaseInsensitive))
            flag = true;
        else if (s->m_szPath.contains(cond, Qt::CaseInsensitive))
            flag = true;
        else if (s->m_szArtist.contains(cond, Qt::CaseInsensitive))
            flag = true;
        else if (isNum) {
            if (s->m_ushSongID == num)
                flag = true;
            else {
                QStringList l = s->m_szNoteNumber.split(QLatin1Char(','));
                foreach (const QString &nn, l) {
                    bool isNumNn;
                    int numNn = nn.toInt(&isNumNn);
                    if (isNumNn && numNn == num) {
                        flag = true;
                        break;
                    }
                }
            }
        }

        if (flag)
            r << i;

        ++i;
    }

    return r;
}

bool RmeSongClientFile::saveWikiTable(QIODevice *output) const
{
    Q_D(const RmeSongClientFile);
    if (output == nullptr || d->m_header == nullptr)
        return false;

    if (output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        // title
        QString titleline(QStringLiteral(R"r({| class="wikitable-green" width="100%"
|-
!|id
!|songname
!|artist
!|time
!|BPM
!|d4e
!|d4m
!|d4h
!|d5e
!|d5m
!|d5h
!|d6e
!|d6m
!|d6h
!|appendix
)r"));

        output->write(titleline.toUtf8().constData());
        foreach (RmeSongClientItemStruct *song, d->m_songsList) {
            // 251 - 262
            if (song->m_bIsLevelReward)
                continue;

            // deal with game time
            int32_t second = song->m_iGameTime % 60;
            int32_t minute = song->m_iGameTime / 60;
            QString gameTime = QString(QStringLiteral("%1:%2")).arg(minute).arg(second);
            if (second < 10)
                gameTime = QString(QStringLiteral("%1:0%2")).arg(minute).arg(second);

            QString difficultyLine = QString(QStringLiteral("%1||%2||%3||%4||%5||%6||%7||%8||%9"))
                                         .arg((song->m_ush4KeyEasy == 0 ? QStringLiteral("{{N/A}}") : QString::number(song->m_ush4KeyEasy)),
                                              (song->m_ush4KeyNormal == 0 ? QStringLiteral("{{N/A}}") : QString::number(song->m_ush4KeyNormal)),
                                              (song->m_ush4KeyHard == 0 ? QStringLiteral("{{N/A}}") : QString::number(song->m_ush4KeyHard)),
                                              (song->m_ush5KeyEasy == 0 ? QStringLiteral("{{N/A}}") : QString::number(song->m_ush5KeyEasy)),
                                              (song->m_ush5KeyNormal == 0 ? QStringLiteral("{{N/A}}") : QString::number(song->m_ush5KeyNormal)),
                                              (song->m_ush5KeyHard == 0 ? QStringLiteral("{{N/A}}") : QString::number(song->m_ush5KeyHard)),
                                              (song->m_ush6KeyEasy == 0 ? QStringLiteral("{{N/A}}") : QString::number(song->m_ush6KeyEasy)),
                                              (song->m_ush6KeyNormal == 0 ? QStringLiteral("{{N/A}}") : QString::number(song->m_ush6KeyNormal)),
                                              (song->m_ush6KeyHard == 0 ? QStringLiteral("{{N/A}}") : QString::number(song->m_ush6KeyHard)));

            QString line = QString(QStringLiteral("|-\n"
                                                  "|%6||%1||%2||%3||%4||%5||\n"))
                               .arg(song->m_szSongName, song->m_szArtist, gameTime, song->m_szBPM, difficultyLine, QString::number(song->m_ushSongID));
            output->write(line.toUtf8().constData());
        }
        QString tailLine = QStringLiteral("|}");
        output->write(tailLine.toUtf8().constData());

        return true;
    }
    return false;
}

bool RmeSongClientFile::savePatchToDevice(QIODevice *output, const RmeSongClientFile &orig) const
{
    Q_D(const RmeSongClientFile);
    if (output == nullptr || d->m_header == nullptr || orig.d_ptr->m_header == nullptr)
        return false;

    QJsonArray arr;
    auto key = d->m_songsList.keys();
    for (auto it = key.crbegin(), e = key.crend(); it != e; ++it) {
        int16_t id = *it;
        if (orig.d_ptr->m_songsList.contains(id)) {
            QJsonObject ob1 = d->m_songsList.value(id)->createPatch(*orig.d_ptr->m_songsList.value(id));
            if (!ob1.isEmpty())
                arr << ob1;
        }
    }

    if (arr.isEmpty())
        return false;

    if (output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QJsonObject ob;
        ob[QStringLiteral("data")] = arr;
        QJsonDocument doc(ob);
        output->write(doc.toJson(QJsonDocument::Indented));
        output->close();
    } else
        return false;

    return true;
}

bool RmeSongClientFile::applyPatchFromDevice(QIODevice *input)
{
    Q_D(RmeSongClientFile);
    if (input == nullptr || d->m_header == nullptr)
        return false;
    if ((input->isOpen() && input->isReadable()) || input->open(QIODevice::ReadOnly)) {
        QByteArray arr = input->readAll();
        input->close();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(arr, &err);
        if (err.error == QJsonParseError::NoError) {
            // Todo: one of these solutions must be procedured:
            // 1. check the patch if it is valid, notify user if it may not be applyed
            // 2. deep copy the original song list and try to apply the patch, if succeeded, create a new RmeSongClientFilePrivate and swap d_ptr
            if (doc.isObject()) {
                QJsonObject ob = doc.object();
                QJsonArray arr = ob.value(QStringLiteral("data")).toArray();
                for (auto it = arr.constBegin(), e = arr.constEnd(); it != e; ++it) {
                    QJsonObject item = it->toObject();
                    int16_t id = item.value(QStringLiteral("ushSongID")).toInt(0);
                    if (id != 0 && d->m_songsList.contains(id))
                        d->m_songsList[id]->applyPatch(item);
                }
            } else
                return false;
        } else {
            qDebug() << err.error << err.offset;
            qDebug() << err.errorString();
            return false;
        }
    } else
        return false;
    return true;
}

int RmeSongClientFile::songCount() const
{
    Q_D(const RmeSongClientFile);
    return d->m_songsList.size();
}

class RmePapaSongClientFilePrivate
{
public:
    QMap<int32_t, RmePapaSongClientItemStruct *> m_songsList;
    QList<int32_t> m_songKeys;
    RmeSongClientHeaderStruct *m_header;
    void cleanup();
};

RmePapaSongClientFile::RmePapaSongClientFile()
    : d_ptr(new RmePapaSongClientFilePrivate)
{
    Q_D(RmePapaSongClientFile);
    d->m_header = nullptr;
}

RmePapaSongClientFile::~RmePapaSongClientFile()
{
    Q_D(RmePapaSongClientFile);
    qDeleteAll(d->m_songsList);
    delete d->m_header;
    delete d;
}

void RmePapaSongClientFilePrivate::cleanup()
{
    qDeleteAll(m_songsList);
    m_songsList.clear();
    m_songKeys.clear();
}

bool RmePapaSongClientFile::readInfoFromDevice(QIODevice *input, RmeFileFormat format)
{
    Q_D(RmePapaSongClientFile);
    // treat the unknown format
    if (format == UnknownFormat || input == nullptr)
        return false;

    if ((input->isOpen() && input->isReadable()) || input->open(QIODevice::ReadOnly)) {
        d->cleanup();
        QByteArray ba = input->readAll();
        input->close();

        std::unique_ptr<RmeFileReader> reader(RmeFileReader::createFileReader(format, ba));
        if (reader == nullptr)
            return false;

        d->m_header = reader->readHeader();
        for (int32_t i = 0; i < d->m_header->Count; ++i) {
            RmePapaSongClientItemStruct *ss = reader->readPapaItem();
            if (ss == nullptr)
                return false;
            d->m_songsList[(int32_t)(ss->m_ushSongID) * 10 + ss->m_cLevel] = ss;
        }
        d->m_songKeys = d->m_songsList.keys();
        return true;
    }
    return false;
}

bool RmePapaSongClientFile::saveInfoToDevice(QIODevice *output, RmeFileFormat format) const
{
    Q_D(const RmePapaSongClientFile);
    if (format == UnknownFormat || output == nullptr || d->m_header == nullptr)
        return false;

    if (output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        {
            // All the writer writes their data during destructor so make the writer out of scope when writing done
            std::unique_ptr<RmeFileWriter> writer(RmeFileWriter::createFileWriter(format, true, output));
            if (!writer->writeHeader(d->m_header))
                return false;
            foreach (RmePapaSongClientItemStruct *s, d->m_songsList) {
                if (!writer->writeClient(s))
                    return false;
            }
        }
        output->close();
        return true;
    }

    return false;
}

RmePapaSongClientItemStruct *RmePapaSongClientFile::song(int n)
{
    Q_D(RmePapaSongClientFile);

    int32_t id = d->m_songKeys.value(n, 1);
    return d->m_songsList.value(id, nullptr);
}

const RmePapaSongClientItemStruct *RmePapaSongClientFile::song(int n) const
{
    Q_D(const RmePapaSongClientFile);

    int32_t id = d->m_songKeys.value(n, 1);
    return d->m_songsList.value(id, nullptr);
}

const RmeSongClientHeaderStruct &RmePapaSongClientFile::fileHeader() const
{
    Q_D(const RmePapaSongClientFile);
    return *d->m_header;
}

QList<int> RmePapaSongClientFile::search(const QString &cond) const
{
    Q_D(const RmePapaSongClientFile);
    QList<int> r;
    bool isNum = false;
    int num = cond.toInt(&isNum);
    int i = 0;
    foreach (const RmePapaSongClientItemStruct *s, d->m_songsList) {
        bool flag = false;
        if (s->m_szSongName.contains(cond, Qt::CaseInsensitive))
            flag = true;
        else if (s->m_szPath.contains(cond, Qt::CaseInsensitive))
            flag = true;
        else if (s->m_szArtist.contains(cond, Qt::CaseInsensitive))
            flag = true;
        else if (isNum) {
            if (s->m_ushSongID == num)
                flag = true;
            else {
                bool isNumNn;
                int numNn = s->m_szNoteNumber.toInt(&isNumNn);
                if (isNumNn && numNn == num)
                    flag = true;
            }
        }

        if (flag)
            r << i;

        ++i;
    }

    return r;
}

bool RmePapaSongClientFile::saveWikiTable(QIODevice *output) const
{
    Q_D(const RmePapaSongClientFile);
    if (output == nullptr || d->m_header == nullptr)
        return false;

    if (output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        // title
        QString titleline(QStringLiteral(R"r({| class="wikitable-green" width="100%"
|-
!|songid
!|songname
!|artist
!|time
!|BPM
!|difficulty
!|level
!|appendix
)r"));

        output->write(titleline.toUtf8().constData());
        foreach (RmePapaSongClientItemStruct *song, d->m_songsList) {
            // deal with game time
            int32_t second = song->m_iGameTime % 60;
            int32_t minute = song->m_iGameTime / 60;
            QString gameTime = QString(QStringLiteral("%1:%2")).arg(minute).arg(second);
            if (second < 10)
                gameTime = QString(QStringLiteral("%1:0%2")).arg(minute).arg(second);

            QString difficulty = QStringLiteral("hd");
            if (song->m_cDifficulty == 2)
                difficulty = QStringLiteral("nm");
            else if (song->m_cDifficulty == 1)
                difficulty = QStringLiteral("ez");

            QString line = QString(QStringLiteral("|-\n"
                                                  "|%1||%2||%3||%4||%5||%6||%7||\n"))
                               .arg(QString::number(song->m_ushSongID), song->m_szSongName, song->m_szArtist, gameTime, song->m_szBPM, difficulty, QString::number(song->m_cLevel));
            output->write(line.toUtf8().constData());
        }
        QString tailLine = QStringLiteral("|}");
        output->write(tailLine.toUtf8().constData());

        return true;
    }
    return false;
}

bool RmePapaSongClientFile::savePatchToDevice(QIODevice *output, const RmePapaSongClientFile &orig) const
{
    Q_D(const RmePapaSongClientFile);
    if (output == nullptr || d->m_header == nullptr || orig.d_ptr->m_header == nullptr)
        return false;

    QJsonArray arr;
    auto key = d->m_songsList.keys();
    for (auto it = key.crbegin(), e = key.crend(); it != e; ++it) {
        int16_t id = *it;
        if (orig.d_ptr->m_songsList.contains(id)) {
            QJsonObject ob1 = d->m_songsList.value(id)->createPatch(*orig.d_ptr->m_songsList.value(id));
            if (!ob1.isEmpty())
                arr << ob1;
        }
    }

    if (arr.isEmpty())
        return false;

    if (output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QJsonObject ob;
        ob[QStringLiteral("data")] = arr;
        QJsonDocument doc(ob);
        output->write(doc.toJson(QJsonDocument::Indented));
        output->close();
    } else
        return false;

    return true;
}

bool RmePapaSongClientFile::applyPatchFromDevice(QIODevice *input)
{
    Q_D(RmePapaSongClientFile);
    if (input == nullptr || d->m_header == nullptr)
        return false;
    if ((input->isOpen() && input->isReadable()) || input->open(QIODevice::ReadOnly)) {
        QByteArray arr = input->readAll();
        input->close();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(arr, &err);
        if (err.error == QJsonParseError::NoError) {
            // Todo: one of these solutions must be procedured:
            // 1. check the patch if it is valid, notify user if it may not be applyed
            // 2. deep copy the original song list and try to apply the patch, if succeeded, create a new RmeSongClientFilePrivate and swap d_ptr
            if (doc.isObject()) {
                QJsonObject ob = doc.object();
                QJsonArray arr = ob.value(QStringLiteral("data")).toArray();
                for (auto it = arr.constBegin(), e = arr.constEnd(); it != e; ++it) {
                    QJsonObject item = it->toObject();
                    int16_t id = item.value(QStringLiteral("ushSongID")).toInt(0);
                    if (id != 0 && d->m_songsList.contains(id))
                        d->m_songsList[id]->applyPatch(item);
                }
            } else
                return false;
        } else
            return false;
    } else
        return false;
    return true;
}

int RmePapaSongClientFile::songCount() const
{
    Q_D(const RmePapaSongClientFile);
    return d->m_songsList.size();
}
