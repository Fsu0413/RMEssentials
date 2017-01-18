#include "rmesongfile.h"
#include "rmesongstruct.h"

#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMap>

using namespace RmeSong;

class RmeSong::RmeSongClientFilePrivate
{
public:
    QMap<int16_t, RmeSongClientItemStruct *> m_songsList;
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
            input->close();
            if (ba.size() % 0x33e == 0x88) {
                if (d->m_header == nullptr)
                    d->m_header = new RmeSongClientHeaderStruct;
                QByteArray fh = ba.mid(0, 0x88);
                d->m_header->parseByteArray(fh);
                for (int i = 0x88; i < ba.size(); i += 0x33e) {
                    QByteArray sp = ba.mid(i, 0x33e);
                    RmeSongClientItemStruct *ss = new RmeSongClientItemStruct;
                    ss->parseByteArray(sp);
                    d->m_songsList[ss->m_ushSongID] = ss;
                }
                return true;
            }
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
            QByteArray fh = d->m_header->toByteArray();
            output->write(fh, 0x88);
            foreach (RmeSongClientItemStruct *s, d->m_songsList) {
                QByteArray arr = s->toByteArray();
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
    return d->m_songsList.value(d->m_songsList.keys().value(n, 0), nullptr);
}

const RmeSong::RmeSongClientItemStruct *RmeSong::RmeSongClientFile::song(int n) const
{
    Q_D(const RmeSongClientFile);
    return d->m_songsList.value(d->m_songsList.keys().value(n, 0), nullptr);
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
        ob[QStringLiteral("userMadePatch")] = QJsonValue(false);
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
        } else
            return false;
    } else
        return false;
    return true;
}

int RmeSong::RmeSongClientFile::songCount() const
{
    Q_D(const RmeSongClientFile);
    return d->m_songsList.size();
}

void RmeSong::RmeSongClientFile::mergeSongList(RmeSongClientFile *file2)
{
    if (file2 == nullptr)
        return;

    Q_D(RmeSongClientFile);
    delete d->m_header;
    d->m_header = file2->d_ptr->m_header;
    file2->d_ptr->m_header = nullptr;

    QMap<int16_t, RmeSongClientItemStruct *> s = d->m_songsList;
    d->m_songsList = file2->d_ptr->m_songsList;
    file2->d_ptr->m_songsList.clear();
    delete file2;

    foreach (int16_t id, d->m_songsList.keys()) {
        if (s.contains(id)) {
            delete d->m_songsList.value(id, nullptr);
            d->m_songsList[id] = s.value(id);
            s.remove(id);
        }
    }

    qDeleteAll(s);
}

class RmeSong::RmePapaSongClientFilePrivate
{
public:
    QMap<int16_t, RmePapaSongClientItemStruct *> m_songsList;
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
            input->close();
            if (ba.size() % 0x169 == 0x88) {
                if (d->m_header == nullptr)
                    d->m_header = new RmeSongClientHeaderStruct;
                QByteArray fh = ba.mid(0, 0x88);
                d->m_header->parseByteArray(fh);
                for (int i = 0x88; i < ba.size(); i += 0x169) {
                    QByteArray sp = ba.mid(i, 0x169);
                    RmePapaSongClientItemStruct *ss = new RmePapaSongClientItemStruct;
                    ss->parseByteArray(sp);
                    d->m_songsList[ss->m_ushSongID] = ss;
                }
                return true;
            }
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
            QByteArray fh = d->m_header->toByteArray();
            output->write(fh, 0x88);
            foreach (RmePapaSongClientItemStruct *s, d->m_songsList) {
                QByteArray arr = s->toByteArray();
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
    return d->m_songsList.value(d->m_songsList.keys().value(n, 0), nullptr);
}

const RmeSong::RmePapaSongClientItemStruct *RmeSong::RmePapaSongClientFile::song(int n) const
{
    Q_D(const RmePapaSongClientFile);
    return d->m_songsList.value(d->m_songsList.keys().value(n, 0), nullptr);
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
        ob[QStringLiteral("userMadePatch")] = QJsonValue(false);
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
                if (ob.contains(QStringLiteral("userMadePatch")) && ob.value(QStringLiteral("userMadePatch")).toBool(false)) {
                    // find next free song, and patch it
                    auto listit = d->m_songsList.end();
                    for (auto arrit = arr.constBegin(), arre = arr.constEnd(); listit != d->m_songsList.begin() && arrit != arre; ++arrit) {
                        --listit;
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
