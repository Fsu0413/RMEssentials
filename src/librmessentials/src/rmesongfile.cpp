#include "rmesongfile.h"
#include "rmesongstruct.h"

#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>
#include <QXmlStreamWriter>

using namespace RmeSong;

namespace {

class RmeXmlReader
{
public:
    explicit RmeXmlReader(QIODevice *device);
    explicit RmeXmlReader(const QByteArray &array);
    explicit RmeXmlReader(const QString &str);
    explicit RmeXmlReader(const char *str);

    bool isPapa() const;
    bool error() const;
    const RmeSongClientHeaderStruct &header() const;

    RmeSongClientItemStruct *readItem();
    RmePapaSongClientItemStruct *readPapaItem();

private:
    bool readHeader();

    bool m_isPapa;
    bool m_isError;

    QXmlStreamReader m_reader;
    RmeSongClientHeaderStruct m_header;

    Q_DISABLE_COPY(RmeXmlReader)
    RmeXmlReader() = delete;
};

class RmeXmlWriter
{
public:
    explicit RmeXmlWriter(QIODevice *device, bool isPapa);
    explicit RmeXmlWriter(QByteArray *array, bool isPapa);
    explicit RmeXmlWriter(QString *string, bool isPapa);
    ~RmeXmlWriter();

    bool writeHeader(const RmeSongClientHeaderStruct *header);
    bool writeClient(const RmeSongClientItemStruct *item);
    bool writeClient(const RmePapaSongClientItemStruct *item);

    bool error() const;

private:
    bool startWriting(bool isPapa);

    QXmlStreamWriter m_writer;

    Q_DISABLE_COPY(RmeXmlWriter)
    RmeXmlWriter() = delete;
};

RmeXmlReader::RmeXmlReader(QIODevice *device)
    : m_isPapa(false)
    , m_isError(false)
    , m_reader(device)
{
    m_isError = !readHeader();
}

RmeXmlReader::RmeXmlReader(const QByteArray &array)
    : m_isPapa(false)
    , m_isError(false)
    , m_reader(array)
{
    m_isError = !readHeader();
}

RmeXmlReader::RmeXmlReader(const QString &str)
    : m_isPapa(false)
    , m_isError(false)
    , m_reader(str)
{
    m_isError = !readHeader();
}

RmeXmlReader::RmeXmlReader(const char *str)
    : m_isPapa(false)
    , m_isError(false)
    , m_reader(str)
{
    m_isError = !readHeader();
}

bool RmeXmlReader::isPapa() const
{
    return m_isPapa;
}

bool RmeXmlReader::error() const
{
    return m_isError || (m_reader.hasError());
}

const RmeSongClientHeaderStruct &RmeXmlReader::header() const
{
    return m_header;
}

#define READNEXT(retvalue)       \
    do {                         \
        m_reader.readNext();     \
        if (m_reader.hasError()) \
            return retvalue;     \
    } while (m_reader.isWhitespace())

RmeSongClientItemStruct *RmeXmlReader::readItem()
{
    if (m_isPapa)
        return nullptr;
    if (error())
        return nullptr;

    READNEXT(nullptr);

    if (!m_reader.isStartElement()) {
        m_isError = true;
        return nullptr;
    }
    if (m_reader.name() != QStringLiteral("SongConfig_Client")) {
        m_isError = true;
        return nullptr;
    }

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
    return item;
}

RmePapaSongClientItemStruct *RmeXmlReader::readPapaItem()
{
    if (!m_isPapa)
        return nullptr;
    if (error())
        return nullptr;

    READNEXT(nullptr);

    if (!m_reader.isStartElement()) {
        m_isError = true;
        return nullptr;
    }
    if (m_reader.name() != QStringLiteral("PapaSongConfig_Client_Tab")) {
        m_isError = true;
        return nullptr;
    }

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
    return item;
}

bool RmeXmlReader::readHeader()
{
    READNEXT(false);

    if (!m_reader.isStartDocument())
        return false;

    READNEXT(false);

    if (!m_reader.isStartElement())
        return false;
    if (!m_reader.name().endsWith(QStringLiteral("SongConfig_Client_Tab")))
        return false;
    if (m_reader.name().startsWith(QStringLiteral("Papa")))
        m_isPapa = true;

    READNEXT(false);

    if (!m_reader.isStartElement())
        return false;
    if (m_reader.name() != QStringLiteral("TResHeadAll"))
        return false;
    READNEXT(false);

    if (!m_reader.isStartElement())
        return false;
    if (m_reader.name() != QStringLiteral("resHead"))
        return false;

    READNEXT(false);

    QVariantMap map;
    while (m_reader.isStartElement()) {
        map[m_reader.name().toString()] = m_reader.readElementText();
        if (m_reader.hasError())
            return false;

        READNEXT(false);
    }

    if (!m_reader.isEndElement())
        return false;
    if (m_reader.name() != QStringLiteral("resHead"))
        return false;

    READNEXT(false);

    if (!m_reader.isStartElement())
        return false;
    if (m_reader.name() != QStringLiteral("resHeadExt"))
        return false;

    READNEXT(false);

    while (m_reader.isStartElement()) {
        map[m_reader.name().toString()] = m_reader.readElementText();
        if (m_reader.hasError())
            return false;

        READNEXT(false);
    }

    if (!m_reader.isEndElement())
        return false;
    if (m_reader.name() != QStringLiteral("resHeadExt"))
        return false;

    READNEXT(false);

    if (!m_reader.isEndElement())
        return false;
    if (m_reader.name() != QStringLiteral("TResHeadAll"))
        return false;

    m_header.parseMap(map);
    return true;
}

#undef READNEXT

RmeXmlWriter::RmeXmlWriter(QIODevice *device, bool isPapa)
    : m_writer(device)
{
    startWriting(isPapa);
}

RmeXmlWriter::RmeXmlWriter(QByteArray *array, bool isPapa)
    : m_writer(array)
{
    startWriting(isPapa);
}

RmeXmlWriter::RmeXmlWriter(QString *string, bool isPapa)
    : m_writer(string)
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

bool RmeXmlWriter::error() const
{
    return m_writer.hasError();
}

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

class RmeJsonReader
{
public:
    explicit RmeJsonReader(QIODevice *device);
    explicit RmeJsonReader(const QByteArray &array);
    explicit RmeJsonReader(const char *str);

    bool error() const;
    const RmeSongClientHeaderStruct &header() const;

    RmeSongClientItemStruct *readItem();
    RmePapaSongClientItemStruct *readPapaItem();

private:
    bool load(const QByteArray &data);

    bool m_isPapa;
    bool m_isError;

    QJsonArray m_jsonArray;
    QJsonParseError m_jsonError;

    RmeSongClientHeaderStruct m_header;

    Q_DISABLE_COPY(RmeJsonReader)
    RmeJsonReader() = delete;
};

// This class is different by design from RmeXmlWriter since all data are written to Json Array
// the IODevice or Array can only be updated in destructor
class RmeJsonWriter
{
public:
    explicit RmeJsonWriter(QIODevice *device, bool isPapa);
    explicit RmeJsonWriter(QByteArray *array, bool isPapa);
    ~RmeJsonWriter();

    bool writeHeader(const RmeSongClientHeaderStruct *header);
    bool writeClient(const RmeSongClientItemStruct *item);
    bool writeClient(const RmePapaSongClientItemStruct *item);

    bool error() const;

private:
    QJsonArray m_jsonArray;

    QIODevice *m_device;
    QByteArray *m_byteArray;

    Q_DISABLE_COPY(RmeJsonWriter)
    RmeJsonWriter() = delete;
};

RmeJsonReader::RmeJsonReader(QIODevice *device)
    : m_isPapa(false)
    , m_isError(false)
{
    if ((device->isOpen() && device->isReadable()) || device->open(QIODevice::ReadOnly)) {
        QByteArray arr = device->readAll();
        load(arr);
    }
}

RmeJsonReader::RmeJsonReader(const QByteArray &array)
    : m_isPapa(false)
    , m_isError(false)
{
    load(array);
}

RmeJsonReader::RmeJsonReader(const char *str)
    : m_isPapa(false)
    , m_isError(false)
{
    QByteArray arr = QByteArray::fromRawData(str, (int)strlen(str));
    load(arr);
}

bool RmeJsonReader::error() const
{
    return m_isError;
}

const RmeSongClientHeaderStruct &RmeJsonReader::header() const
{
    return m_header;
}

RmeSongClientItemStruct *RmeJsonReader::readItem()
{
    if (m_isPapa)
        return nullptr;
    if (error())
        return nullptr;
    if (m_jsonArray.isEmpty()) {
        m_isError = true;
        return nullptr;
    }

    QJsonValue v = m_jsonArray.takeAt(0);

    if (!v.isObject()) {
        m_isError = true;
        return nullptr;
    }

    QJsonObject ob = v.toObject();

    // idk what if we use toVariantMap
    QVariantMap map;
    for (auto it = ob.begin(); it != ob.end(); ++it) {
        QString value;
        if (it.value().isString()) {
            value = it.value().toString();
        } else if (!it.value().isNull()) {
            m_isError = true;
            return nullptr;
        }

        map[it.key()] = value;
    }

    RmeSongClientItemStruct *item = new RmeSongClientItemStruct;
    item->parseMap(map);
    return item;
}

RmePapaSongClientItemStruct *RmeJsonReader::readPapaItem()
{
    if (!m_isPapa)
        return nullptr;
    if (error())
        return nullptr;
    if (m_jsonArray.isEmpty()) {
        m_isError = true;
        return nullptr;
    }

    QJsonValue v = m_jsonArray.takeAt(0);

    if (!v.isObject()) {
        m_isError = true;
        return nullptr;
    }

    QJsonObject ob = v.toObject();

    // idk what if we use toVariantMap
    QVariantMap map;
    for (auto it = ob.begin(); it != ob.end(); ++it) {
        QString value;
        if (it.value().isString()) {
            value = it.value().toString();
        } else if (!it.value().isNull()) {
            m_isError = true;
            return nullptr;
        }

        map[it.key()] = value;
    }

    RmePapaSongClientItemStruct *item = new RmePapaSongClientItemStruct;
    item->parseMap(map);
    return item;
}

bool RmeJsonReader::load(const QByteArray &data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data, &m_jsonError);
    if (m_jsonError.error != QJsonParseError::NoError) {
        m_isError = true;
        return false;
    }

    if (!doc.isArray()) {
        m_isError = true;
        return false;
    }

    m_jsonArray = doc.array();

    // distinguish it is Papa struct or not
    // Papa struct has an "m_cLevel" in each element while normal mode does not
    QJsonValue v = m_jsonArray.at(0);
    if (v.isObject() && v.toObject().contains(QStringLiteral("m_cLevel"))) // assuming it is papa struct
        m_isPapa = true;

    return true;
}

RmeJsonWriter::RmeJsonWriter(QIODevice *device, bool)
    : m_device(device)
{
    m_byteArray = new QByteArray;
}

RmeJsonWriter::RmeJsonWriter(QByteArray *array, bool)
    : m_byteArray(array)
{
}

RmeJsonWriter::~RmeJsonWriter()
{
    QJsonDocument doc(m_jsonArray);
    *m_byteArray = doc.toJson(QJsonDocument::Indented);

    if (m_device != nullptr) {
        if ((m_device->isOpen() && m_device->isWritable()) || m_device->open(QIODevice::WriteOnly)) {
            int64_t written = m_device->write(*m_byteArray);
            if (written < m_byteArray->length()) {
                // what to do....
            }
        }

        delete m_byteArray;
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

bool RmeJsonWriter::error() const
{
    return false;
}

}

class RmeSong::RmeSongClientFilePrivate
{
public:
    QMap<int16_t, RmeSongClientItemStruct *> m_songsList;
    QList<int16_t> m_songKeys;
    RmeSongClientHeaderStruct *m_header;
    bool m_isUserMade;
    void cleanup();
};

RmeSong::RmeSongClientFile::RmeSongClientFile()
    : d_ptr(new RmeSongClientFilePrivate)
{
    Q_D(RmeSongClientFile);
    d->m_header = nullptr;
    d->m_isUserMade = false;
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
    m_songKeys.clear();
}

bool RmeSong::RmeSongClientFile::readInfoFromDevice(QIODevice *input, RmeFileFormat format)
{
    Q_D(RmeSongClientFile);
    // treat the unknown format
    if (format == UnknownFormat || input == nullptr)
        return false;

    if ((input->isOpen() && input->isReadable()) || input->open(QIODevice::ReadOnly)) {
        d->cleanup();
        QByteArray ba = input->readAll();
        input->close();
        if (format == BinFormat) {
            if (ba.size() % 0x33e == 0x88) {
                delete d->m_header;
                d->m_header = new RmeSongClientHeaderStruct;
                QByteArray fh = ba.mid(0, 0x88);
                d->m_header->parseByteArray(fh);
                for (int i = 0x88; i < ba.size(); i += 0x33e) {
                    QByteArray sp = ba.mid(i, 0x33e);
                    RmeSongClientItemStruct *ss = new RmeSongClientItemStruct;
                    ss->parseByteArray(sp);
                    d->m_songsList[ss->m_ushSongID] = ss;
                }
                d->m_songKeys = d->m_songsList.keys();
                return true;
            }
        } else if (format == XmlFormat) {
            RmeXmlReader reader(ba);
            delete d->m_header;
            d->m_header = new RmeSongClientHeaderStruct(reader.header());
            for (int32_t i = 0; i < d->m_header->Count; ++i) {
                RmeSongClientItemStruct *ss = reader.readItem();
                if (ss == nullptr)
                    continue;
                d->m_songsList[ss->m_ushSongID] = ss;
            }
            d->m_songKeys = d->m_songsList.keys();
            return true;
        } else if (format == JsonFormat) {
            // Json song client file have no header.
            RmeJsonReader reader(ba);
            delete d->m_header;
            d->m_header = new RmeSongClientHeaderStruct;
            RmeSongClientItemStruct *ss = reader.readItem();
            while (ss != nullptr) {
                d->m_songsList[ss->m_ushSongID] = ss;
                ss = reader.readItem();
            }
            d->m_header->Count = d->m_songsList.count();

            d->m_songKeys = d->m_songsList.keys();
            return true;
        }
    }
    return false;
}

bool RmeSong::RmeSongClientFile::saveInfoToDevice(QIODevice *output, RmeFileFormat format) const
{
    Q_D(const RmeSongClientFile);
    if (format == UnknownFormat || output == nullptr || d->m_header == nullptr)
        return false;

    if (output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (format == BinFormat) {
            output->write(d->m_header->toByteArray(), 0x88);
            foreach (RmeSongClientItemStruct *s, d->m_songsList)
                output->write(s->toByteArray(), 0x33e);
        } else if (format == XmlFormat) {
            RmeXmlWriter writer(output, false);
            writer.writeHeader(d->m_header);
            foreach (RmeSongClientItemStruct *s, d->m_songsList)
                writer.writeClient(s);
        } else if (format == JsonFormat) {
            RmeJsonWriter writer(output, false);
            writer.writeHeader(d->m_header);
            foreach (RmeSongClientItemStruct *s, d->m_songsList)
                writer.writeClient(s);
        }

        output->close();
        return true;
    }

    return false;
}

RmeSong::RmeSongClientItemStruct *RmeSong::RmeSongClientFile::song(int n)
{
    Q_D(RmeSongClientFile);

    int16_t id = d->m_songKeys.value(n, 1);
    return d->m_songsList.value(id, nullptr);
}

const RmeSong::RmeSongClientItemStruct *RmeSong::RmeSongClientFile::song(int n) const
{
    Q_D(const RmeSongClientFile);

    int16_t id = d->m_songKeys.value(n, 1);
    return d->m_songsList.value(id, nullptr);
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
!|type
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

            QString type;
            if (song->m_ushSongID >= 251 && song->m_ushSongID <= 262)
                type = QStringLiteral("level");
            else if (song->m_bIsHide && (song->m_ucIsOpen || song->m_bIsFree))
                type = QStringLiteral("hide");
            else if (song->m_bIsReward && (song->m_ucIsOpen || song->m_bIsFree))
                type = QStringLiteral("reward");
            else if (song->m_ucIsOpen && song->m_bIsFree)
                type = QStringLiteral("free");
            else if (song->m_ucCanBuy)
                type = QStringLiteral("buy");

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
                                                  "|%7||%1||%2||%3||%4||%5||%6\n"))
                               .arg(song->m_szSongName, song->m_szArtist, gameTime, song->m_szBPM, difficultyLine, type, QString::number(song->m_ushSongID));
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
            QJsonObject ob1 = d->m_songsList.value(id)->createPatch(*orig.d_ptr->m_songsList.value(id), d->m_isUserMade);
            if (!ob1.isEmpty())
                arr << ob1;
        }
    }

    if (arr.isEmpty())
        return false;

    if (output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QJsonObject ob;
        ob[QStringLiteral("userMadePatch")] = QJsonValue(d->m_isUserMade);
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
                if (ob.contains(QStringLiteral("userMadePatch")) && ob.value(QStringLiteral("userMadePatch")).toBool(false)) {
                    prepareForUserMadeNotes();
                    // find next free song, and patch it
                    auto listit = d->m_songsList.end();
                    for (auto arrit = arr.constBegin(), arre = arr.constEnd(); listit != d->m_songsList.begin() && arrit != arre; ++arrit) {
                        --listit;
                        // find a free song
                        while (listit != d->m_songsList.begin() && !(*listit)->isFree())
                            --listit;
                        if (!(*listit)->isFree())
                            break;
                        // *listit is free!!
                        (*listit)->applyPatch(arrit->toObject(), true);
                    }
                } else {
                    for (auto it = arr.constBegin(), e = arr.constEnd(); it != e; ++it) {
                        QJsonObject item = it->toObject();
                        int16_t id = item.value(QStringLiteral("ushSongID")).toInt(0);
                        if (id != 0 && d->m_songsList.contains(id))
                            d->m_songsList[id]->applyPatch(item);
                    }
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

int RmeSong::RmeSongClientFile::songCount() const
{
    Q_D(const RmeSongClientFile);
    return d->m_songsList.size();
}

void RmeSongClientFile::prepareForUserMadeNotes()
{
    Q_D(RmeSongClientFile);
    d->m_isUserMade = true;
    int orderIndex = 0;
    for (auto it = d->m_songsList.end(), b = d->m_songsList.begin(); it != b;) {
        --it;
        if ((*it)->isFree()) {
            (*it)->m_szComposer = QStringLiteral("Offical Free Song");
            (*it)->m_iOrderIndex = ++orderIndex;
        } else {
            (*it)->m_szComposer = QStringLiteral("Offical Non-free Song");
            (*it)->m_iOrderIndex = 0;
        }
    }
}

bool RmeSongClientFile::isUserMadeMode() const
{
    Q_D(const RmeSongClientFile);
    return d->m_isUserMade;
}

class RmeSong::RmePapaSongClientFilePrivate
{
public:
    QMap<int32_t, RmePapaSongClientItemStruct *> m_songsList;
    QList<int32_t> m_songKeys;
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
    m_songKeys.clear();
}

bool RmeSong::RmePapaSongClientFile::readInfoFromDevice(QIODevice *input, RmeFileFormat format)
{
    Q_D(RmePapaSongClientFile);
    // treat the unknown format
    if (format == UnknownFormat || input == nullptr)
        return false;

    if ((input->isOpen() && input->isReadable()) || input->open(QIODevice::ReadOnly)) {
        d->cleanup();
        QByteArray ba = input->readAll();
        input->close();
        if (format == BinFormat) {
            if (ba.size() % 0x169 == 0x88) {
                delete d->m_header;
                d->m_header = new RmeSongClientHeaderStruct;
                QByteArray fh = ba.mid(0, 0x88);
                d->m_header->parseByteArray(fh);
                for (int i = 0x88; i < ba.size(); i += 0x169) {
                    QByteArray sp = ba.mid(i, 0x169);
                    RmePapaSongClientItemStruct *ss = new RmePapaSongClientItemStruct;
                    ss->parseByteArray(sp);
                    d->m_songsList[(int32_t)(ss->m_ushSongID) * 10 + ss->m_cLevel] = ss;
                }

                d->m_songKeys = d->m_songsList.keys();
                return true;
            }
        } else if (format == XmlFormat) {
            RmeXmlReader reader(ba);
            delete d->m_header;
            d->m_header = new RmeSongClientHeaderStruct(reader.header());
            for (int32_t i = 0; i < d->m_header->Count; ++i) {
                RmePapaSongClientItemStruct *ss = reader.readPapaItem();
                if (ss == nullptr)
                    continue;
                d->m_songsList[(int32_t)(ss->m_ushSongID) * 10 + ss->m_cLevel] = ss;
            }

            d->m_songKeys = d->m_songsList.keys();
            return true;
        } else if (format == JsonFormat) {
            RmeJsonReader reader(ba);
            delete d->m_header;
            d->m_header = new RmeSongClientHeaderStruct;
            RmePapaSongClientItemStruct *ss = reader.readPapaItem();
            while (ss != nullptr) {
                d->m_songsList[(int32_t)(ss->m_ushSongID) * 10 + ss->m_cLevel] = ss;
                ss = reader.readPapaItem();
            }
            d->m_header->Count = d->m_songsList.count();

            d->m_songKeys = d->m_songsList.keys();
            return true;
        }
    }
    return false;
}

bool RmeSong::RmePapaSongClientFile::saveInfoToDevice(QIODevice *output, RmeFileFormat format) const
{
    Q_D(const RmePapaSongClientFile);
    if (format == UnknownFormat || output == nullptr || d->m_header == nullptr)
        return false;

    if (output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (format == BinFormat) {
            output->write(d->m_header->toByteArray(), 0x88);
            foreach (RmePapaSongClientItemStruct *s, d->m_songsList)
                output->write(s->toByteArray(), 0x169);
        } else if (format == XmlFormat) {
            RmeXmlWriter writer(output, true);
            writer.writeHeader(d->m_header);
            foreach (RmePapaSongClientItemStruct *s, d->m_songsList)
                writer.writeClient(s);
        } else if (format == JsonFormat) {
            RmeJsonWriter writer(output, true);
            writer.writeHeader(d->m_header);
            foreach (RmePapaSongClientItemStruct *s, d->m_songsList)
                writer.writeClient(s);
        }

        output->close();
        return true;
    }

    return false;
}

RmeSong::RmePapaSongClientItemStruct *RmeSong::RmePapaSongClientFile::song(int n)
{
    Q_D(RmePapaSongClientFile);

    int32_t id = d->m_songKeys.value(n, 1);
    return d->m_songsList.value(id, nullptr);
}

const RmeSong::RmePapaSongClientItemStruct *RmeSong::RmePapaSongClientFile::song(int n) const
{
    Q_D(const RmePapaSongClientFile);

    int32_t id = d->m_songKeys.value(n, 1);
    return d->m_songsList.value(id, nullptr);
}

const RmeSong::RmeSongClientHeaderStruct &RmeSong::RmePapaSongClientFile::fileHeader() const
{
    Q_D(const RmePapaSongClientFile);
    return *d->m_header;
}

QList<int> RmeSong::RmePapaSongClientFile::search(const QString &cond) const
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
            QString secondStr = QString::number(second);
            if (secondStr.length() == 1)
                secondStr.prepend(QStringLiteral("0"));

            QString gameTime = QString(QStringLiteral("%1:%2")).arg(minute).arg(secondStr);

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

int RmeSong::RmePapaSongClientFile::songCount() const
{
    Q_D(const RmePapaSongClientFile);
    return d->m_songsList.size();
}
